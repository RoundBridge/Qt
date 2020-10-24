#include "XVideoThread.h"
#include "XDistributeThread.h"
#include "XFFmpeg.h"
#include "xplay1.h"
#include <iostream>

using std::cout;
using std::endl;

bool XVideoThread::isExit = true;
bool XVideoThread::isStart = false;
bool XVideoThread::bReset = false;

void XVideoThread::run() {  // 重写QT线程函数(在调用start之后会在线程中运行这个函数)	
	int vPts = -1;
	int aPts = -1;

	while (!isExit) {
		if (bReset) {
			cout << "[VIDEO THREAD] ------ Reset video thread! ------" << endl;
			break;
		}
		if (!XVideoThread::isStart) {
			msleep(10);
			continue;
		}
		XPlay1::rlock();
		if (XDistributeThread::get()->get_video_list()->size() > 0)
		{
			//cout << "[VIDEO THREAD] ------ Video thread running! ------" << endl;
			AVPacket* pktv = XDistributeThread::get()->get_video_list()->front();
			vPts = XFFmpeg::get()->get_current_video_pts(pktv);
			aPts = XFFmpeg::get()->get_current_audio_pts();
			// 情况1：最早的视频帧时间戳都大于当前音频帧时间戳，说明最早的视频帧都在当前音频帧的后面，那就
			//        不播放视频，让视频在队列中再等待一会儿，等音频帧的时间戳增大到能匹配视频帧时间戳
			// 情况2：最早的视频帧时间戳小于等于当前音频帧的时间戳时，播放该视频帧
			while (vPts > aPts && !XPlay1::bSeek && !bReset && isStart && !isExit) {
				/*
					此处用音频来同步视频，也即将视频按照音频时间戳来进行播放
					因为音频在采样率、通道数、样本点位宽确定的情况下，播放速度是固定
					的，而视频帧与帧之间需要有间隔，这个间隔如果通过sleep来实现就不准
					确，且偏差会累积，所以用音频来同步视频
				*/
				//cout << "[VIDEO THREAD] ------ vPts " << vPts << "ms, aPts " << aPts << " ms ------" << endl;
				msleep(5);
				aPts = XFFmpeg::get()->get_current_audio_pts();
			}
			if (isExit || bReset) {
				XPlay1::unlock();
				break;
			}
			if (!isStart || XPlay1::bSeek) {
				XPlay1::unlock();
				continue;
			}
			XFFmpeg::get()->decode(pktv);
			av_packet_unref(pktv);
			if (pktv) av_packet_free(&pktv);
			XDistributeThread::get()->get_video_list()->pop_front();
		}
		else {
			msleep(1);
		}
		//对于解码出来的AVFrame类型的yuv。注意，linesize对应的是一行的长度，比如测试视频是1280x720，且
		//format是AV_PIX_FMT_YUV420P，则linesize [0]/[1]/[2]分别表示一行的yuv长度，为1280/640/640.
	    //对于音频，则是一帧音频的字节数，如果音频是float型（AV_SAMPLE_FMT_S32P）且是双通道的，则
	    //linesize = 4 * 2 * as->codecpar->frame_size(as->codecpar->frame_size表示一帧数据，单通道样本数)
		XPlay1::unlock();
	}
	isExit = true;
	cout << "[VIDEO THREAD] ------ Video thread exit! ------" << endl;
}

XVideoThread:: ~XVideoThread() {

}

XVideoThread::XVideoThread() {  // 不让外部创建线程实例

}