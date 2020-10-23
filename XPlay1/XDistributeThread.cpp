#include "XDistributeThread.h"
#include "XAudioThread.h"
#include "XVideoThread.h"
#include <iostream>

using std::cout;
using std::endl;

bool XDistributeThread::isExit = true;
bool XDistributeThread::bReset = false;

list<AVPacket*>* XDistributeThread::get_video_list() {
	return &videolist;
}

list<AVPacket*>* XDistributeThread::get_audio_list() {
	return &audiolist;
}

void XDistributeThread::lock() {
	mutex.lock();
}
void XDistributeThread::unlock() {
	mutex.unlock();
}

/*
	���߳�ר�����ڴ�FFMpeg�ж�ȡ����Ƶ���ݰ���
	Ȼ�����Ƿ�����Ƶ����Ƶ����list�У���Ƶ
	����Ƶ�����̴߳�������list�ж�ȡ���Ե���
	�ݽ��д���
*/
void XDistributeThread::run() {
	AVPacket* pkt2 = NULL;
	int numV = 0, numA = 0;

	while (!isExit) {
		//cout << "[DISTRIBUTE THREAD] ------ Distribute thread running! ------" << endl;
		XDistributeThread::get()->lock();
		if (bReset) {
			XDistributeThread::get()->unlock();
			cout << "[DISTRIBUTE THREAD] ------ Reset distribute thread! ------" << endl;
			break;
		}
		/* �������Ƶ���ܶ�û�п����������ַ� */
		if (!XAudioThread::isStart && !XVideoThread::isStart) {
			XDistributeThread::get()->unlock();
			msleep(1);
			continue;
		}
		/* ��Ҫ�ö�������̫��İ������� */
		if (videolist.size() >= 5 && audiolist.size() >= 5) {
			XDistributeThread::get()->unlock();
			msleep(10);
			continue;
		}
		if (XFFmpeg::get()->send_flush_packet()) {
			if (videolist.size() > 0) {
				XDistributeThread::get()->unlock();
				msleep(1);  // �Ƚ������е���Ƶ֡����
				continue;
			}
			else {
				XFFmpeg::get()->get_buffered_frames();
			}
		}
		else {
			AVPacket* pkt = XFFmpeg::get()->read();
			if (pkt == NULL || pkt->size <= 0) {
				XDistributeThread::get()->unlock();
				msleep(1);
				continue;
			}
			pkt2 = av_packet_alloc();
			av_packet_ref(pkt2, pkt); // pkt2����ͬһ�����ݻ���ռ�...
			if (pkt->stream_index == XFFmpeg::get()->videoStream) {
				cout << "[DISTRIBUTE THREAD] Push V " << numV << ", pts " << XFFmpeg::get()->get_current_video_pts(pkt2) << ", buffered vPkt " << videolist.size() << endl;
				videolist.push_back(pkt2);
				numV++;				
			}
			else if (pkt->stream_index == XFFmpeg::get()->audioStream) {
				cout << "[DISTRIBUTE THREAD] Push A " << numA << ", pts " << XFFmpeg::get()->get_current_video_pts(pkt2) << ", buffered aPkt " << audiolist.size() << endl;
				audiolist.push_back(pkt2);
				numA++;				
			}
			else {
				cout << "[DISTRIBUTE THREAD] Unknown stream ID: " << pkt->stream_index << endl;
				av_packet_unref(pkt2);
			}
			av_packet_unref(pkt);
			//av_packet_free(&pkt);  // ֱ��free��ҵ�
		}
		XDistributeThread::get()->unlock();
	}
	/*
	���һ���ļ�û�в����������´���һ���ļ�����Ҫ������Ƶlist���
	*/
	close();
	isExit = true;
	cout << "[DISTRIBUTE THREAD] ------ Distribute thread exit! ------" << endl;
}

void XDistributeThread::clear_packet_list(list<AVPacket*>* list) {
	AVPacket* pkt;

	if (!list) {
		return;
	}
	
	while (list->size() > 0) {
		pkt = list->front();
		av_packet_unref(pkt);
		av_packet_free(&pkt);
		list->pop_front();
	}
	
	return;
}

void XDistributeThread::close() {
	clear_packet_list(&videolist);
	clear_packet_list(&audiolist);
	return;
}

XDistributeThread::XDistributeThread() {

}

XDistributeThread::~XDistributeThread() {

}
