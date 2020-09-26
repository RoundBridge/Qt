#include "xplay1.h"
#include <QtWidgets/QApplication>
#include "XFFmpeg.h"
#include <iostream>
using std::cout;
using std::endl;


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    AVPacket* pkt = NULL;
    AVFrame* yuv = NULL;
    const char* path = "1024.mp4";

    int re = XFFmpeg::get()->open(path);
    if (0 != re) {
        cout << "open " << path << " failed: " << XFFmpeg::get()->get_error(re) << endl;
        return -1;
    }
    else {
        cout << "open " << path << " success! The total length is " << XFFmpeg::get() ->get_duration_ms() << " ms." << endl;
    }

    while (1) {
        /* ����Ƶ�е�ÿһ���� */
        pkt = XFFmpeg::get()->read();
        if (pkt == NULL || pkt->size == 0)break;
        cout << "[READ] - PTS = " << pkt->pts << endl;
        cout << "[READ] - SIZE = " << pkt->size << endl;

        /* �Զ����İ������YUVͼ�� */
        yuv = XFFmpeg::get()->decode(pkt);
        /*
            ע�⣬���ﲻ����ôд��
                if(yuv == NULL) break;
            ��Ϊ��������£�ǰ����av_read_frame�õ���pkt��ȥ����Ҳ��ʧ�ܵģ���ʾ��Resource temporarily unavailable��,
            ����ֱ��break�ͻ��жϽ�����̣����Ҫ��������ô�ж�
        */
        if (yuv != NULL) {
            cout << "[DECODE] -- W/H = " << yuv->width << "/" << yuv->height << endl;
            //ע�⣬linesize��Ӧ����һ�еĳ��ȣ����������Ƶ��1280x720����format��AV_PIX_FMT_YUV420P��
            //��linesize [0]/[1]/[2]�ֱ��ʾһ�е�yuv���ȣ�Ϊ1280/640/640
            //������Ƶ������һ֡��Ƶ���ֽ����������Ƶ��float�ͣ�AV_SAMPLE_FMT_S32P������˫ͨ���ģ���
            //linesize = 4 * 2 * as->codecpar->frame_size(as->codecpar->frame_size��ʾһ֡���ݣ���ͨ��������)
            cout << "[DECODE] -- format = " << yuv->format << ", linesize [0]/[1]/[2] = " << yuv->linesize[0] << "/" << yuv->linesize[1] << "/" << yuv->linesize[2] << endl;
        }
        av_packet_unref(pkt);
    }

    XFFmpeg::get()->close();
    XPlay1 w;
    w.show();
    return a.exec();
}