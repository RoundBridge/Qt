#include "XVideoThread.h"
#include "XFFmpeg.h"

bool XVideoThread::isExit = false;

void XVideoThread::run() {  // ��дQT�̺߳���(�ڵ���start֮������߳��������������)
	while (!isExit) {
		AVPacket *pkt = XFFmpeg::get()->read();
		if (pkt == NULL || pkt->size <= 0) {
			msleep(10);
			continue;
		}
		if (pkt->stream_index != XFFmpeg::get()->videoStream) {
			av_packet_unref(pkt);
			continue;
		}
		AVFrame* yuv = XFFmpeg::get()->decode(pkt);		
		av_packet_unref(pkt);
		if (XFFmpeg::get()->get_video_fps() > 0) {
			msleep(1000 / XFFmpeg::get()->get_video_fps());
		}
	}
}

XVideoThread:: ~XVideoThread() {

}

XVideoThread::XVideoThread() {  // �����ⲿ�����߳�ʵ��

}