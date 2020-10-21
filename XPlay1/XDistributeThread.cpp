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
	该线程专门用于从FFMpeg中读取音视频数据包，
	然后将它们放入音频和视频两个list中，音频
	和视频两个线程从这两个list中读取各自的数
	据进行处理
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
		/* 如果音视频功能都没有开启，不做分发 */
		if (!XAudioThread::isStart && !XVideoThread::isStart) {
			XDistributeThread::get()->unlock();
			msleep(1);
			continue;
		}
		/* 不要让队列里有太多的包缓存着 */
		if (videolist.size() >= 5 && audiolist.size() >= 5) {
			XDistributeThread::get()->unlock();
			msleep(5);
			continue;
		}
		if (XFFmpeg::get()->send_flush_packet()) {
			if (videolist.size() > 0) {
				XDistributeThread::get()->unlock();
				msleep(1);  // 先将队列中的视频帧放完
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
			av_packet_ref(pkt2, pkt); // pkt2共享同一个数据缓存空间...
			if (pkt->stream_index == XFFmpeg::get()->videoStream) {
				videolist.push_back(pkt2);
				numV++;
				cout << "[DISTRIBUTE THREAD] Push V " << numV << ", pts " << XFFmpeg::get()->get_current_video_pts(pkt2) << endl;
			}
			else if (pkt->stream_index == XFFmpeg::get()->audioStream) {
				audiolist.push_back(pkt2);
				numA++;
				cout << "[DISTRIBUTE THREAD] Push A " << numA << ", pts " << XFFmpeg::get()->get_current_video_pts(pkt2) << endl;
			}
			else {
				cout << "[DISTRIBUTE THREAD] Unknown stream ID: " << pkt->stream_index << endl;
				av_packet_unref(pkt2);
			}
			av_packet_unref(pkt);
			//av_packet_free(&pkt);  // 直接free会挂掉
		}
		XDistributeThread::get()->unlock();
	}
	/*
	如果一个文件没有播放完又重新打开另一个文件，则要把音视频list清空
	*/
	close();
	isExit = true;
	cout << "[DISTRIBUTE THREAD] ------ Distribute thread exit! ------" << endl;
}

void XDistributeThread::close() {
	AVPacket* pktv;
	AVPacket* pkta;

	while (videolist.size() > 0) {
		pktv = videolist.front();
		av_packet_unref(pktv);
		if (pktv) av_packet_free(&pktv);
		videolist.pop_front();
	}
	while (audiolist.size() > 0) {
		pkta = audiolist.front();
		av_packet_unref(pkta);
		if (pkta) av_packet_free(&pkta);
		audiolist.pop_front();
	}
	return;
}

XDistributeThread::XDistributeThread() {

}

XDistributeThread::~XDistributeThread() {

}
