#include "XVideoThread.h"
#include "XFFmpeg.h"
#include "xplay1.h"
#include <iostream>

using std::cout;
using std::endl;

bool XVideoThread::isExit = false;
bool XVideoThread::isStart = false;

void XVideoThread::run() {  // ��дQT�̺߳���(�ڵ���start֮������߳��������������)
	while (!isExit) {
		if (!XVideoThread::isStart) {
			msleep(10);
			continue;
		}

		if (XFFmpeg::get()->send_flush_packet()) {
			XFFmpeg::get()->get_buffered_frames();			
		}
		else {
			AVPacket* pkt = XFFmpeg::get()->read();
			if (pkt == NULL || pkt->size <= 0) {
				msleep(10);
				continue;
			}

			if (pkt->stream_index != XFFmpeg::get()->videoStream) {
				av_packet_unref(pkt);
				continue;
			}
			XFFmpeg::get()->decode(pkt);
			av_packet_unref(pkt);
		}
		
		if (XFFmpeg::get()->get_video_fps() > 0) {
			msleep(1000 / XFFmpeg::get()->get_video_fps());
		}
		//���ڽ��������AVFrame���͵�yuv��ע�⣬linesize��Ӧ����һ�еĳ��ȣ����������Ƶ��1280x720����
		//format��AV_PIX_FMT_YUV420P����linesize [0]/[1]/[2]�ֱ��ʾһ�е�yuv���ȣ�Ϊ1280/640/640.
	    //������Ƶ������һ֡��Ƶ���ֽ����������Ƶ��float�ͣ�AV_SAMPLE_FMT_S32P������˫ͨ���ģ���
	    //linesize = 4 * 2 * as->codecpar->frame_size(as->codecpar->frame_size��ʾһ֡���ݣ���ͨ��������)
	}
	XFFmpeg::get()->close();
	cout << "[THREAD] ------ Video thread exit! ------" << endl;
}

XVideoThread:: ~XVideoThread() {

}

XVideoThread::XVideoThread() {  // �����ⲿ�����߳�ʵ��

}