#include "XVideoThread.h"
#include "XFFmpeg.h"
#include "xplay1.h"
#include <iostream>
#include <list>
#include "XAudioPlay.h"

using std::cout;
using std::endl;
using std::list;

bool XVideoThread::isExit = false;
bool XVideoThread::isStart = false;

void XVideoThread::run() {  // ��дQT�̺߳���(�ڵ���start֮������߳��������������)
	char aout[MAXAUDIOSWRLEN] = { 0 };  // ���ڴ��audio_convert��������Ƶ����
	int free = 0;
	int vPts = -1;
	int aPts = -1;
	list<AVPacket*> videoPackets;
	AVPacket* pkt2 = NULL;

	while (!isExit) {
		if (!XVideoThread::isStart) {
			msleep(10);
			continue;
		}

		// ��ȡ�����Ŀ�����Ƶ�����С����дһ֡��Ƶ����
		free = XAudioPlay::get()->get_free_buffer_size();
		if (free < 4096)
		{
			msleep(1);
			continue;
		}

		if (XFFmpeg::get()->send_flush_packet()) {
			XFFmpeg::get()->get_buffered_frames();			
		}
		else {
			while (videoPackets.size() > 0)
			{
				AVPacket* pktv = videoPackets.front();
				vPts = XFFmpeg::get()->get_current_video_pts(pktv);
				// ���1���������Ƶ֡ʱ��������ڵ�ǰ��Ƶ֡ʱ�����˵���������Ƶ֡���ڵ�ǰ��Ƶ֡�ĺ��棬�Ǿ�
				//        ��������Ƶ������Ƶ�ڶ������ٵȴ�һ���������Ƶ֡��ʱ���������ƥ����Ƶ֡ʱ���
				// ���2���������Ƶ֡ʱ���С�ڵ��ڵ�ǰ��Ƶ֡��ʱ���ʱ�����Ÿ���Ƶ֡
				if (vPts > aPts)  // ���1������ѭ������������Ƶ
				{
					break;
				}
				XFFmpeg::get()->decode(pktv);
				cout << "^^^^^^ Play Vpts: " << XFFmpeg::get()->get_current_video_pts(pktv) << endl;
				av_packet_unref(pktv);
				if (pktv) av_packet_free(&pktv);
				videoPackets.pop_front();
			}
			AVPacket* pkt = XFFmpeg::get()->read();
			if (pkt == NULL || pkt->size <= 0) {
				msleep(1);
				continue;
			}
			if (pkt->stream_index == XFFmpeg::get()->audioStream)
			{
				XFFmpeg::get()->decode(pkt);
				aPts = XFFmpeg::get()->get_current_audio_pts();
				cout << "Apts: " << aPts << endl;
				av_packet_unref(pkt);
				int len = XFFmpeg::get()->audio_convert((uint8_t* const)aout);
				if (len > 0) {
					cout << "write len " << len << endl;
					XAudioPlay::get()->write(aout, len);
				}
			}
			else if (pkt->stream_index == XFFmpeg::get()->videoStream) {
				pkt2 = av_packet_alloc();
				av_packet_ref(pkt2, pkt); // pkt2����ͬһ�����ݻ���ռ�...
				av_packet_unref(pkt);
				videoPackets.push_back(pkt2);
				cout << "###### Push Vpts: " << XFFmpeg::get()->get_current_video_pts(pkt2) << endl;
			}
			else{
				cout << "[THREAD] Unknown stream ID: " << pkt->stream_index << endl;
				av_packet_unref(pkt);
			}
		}
		
		if (XFFmpeg::get()->get_video_fps() > 0) {
			//msleep(1000 / XFFmpeg::get()->get_video_fps());
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