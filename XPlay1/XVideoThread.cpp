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

void XVideoThread::run() {  // 重写QT线程函数(在调用start之后会在线程中运行这个函数)
	char aout[MAXAUDIOSWRLEN] = { 0 };  // 用于存放audio_convert出来的音频数据
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

		// 获取出来的空余音频缓存大小不够写一帧音频数据
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
				// 情况1：最早的视频帧时间戳都大于当前音频帧时间戳，说明最早的视频帧都在当前音频帧的后面，那就
				//        不播放视频，让视频在队列中再等待一会儿，等音频帧的时间戳增大到能匹配视频帧时间戳
				// 情况2：最早的视频帧时间戳小于等于当前音频帧的时间戳时，播放该视频帧
				if (vPts > aPts)  // 情况1，跳出循环，不播放视频
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
				av_packet_ref(pkt2, pkt); // pkt2共享同一个数据缓存空间...
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
		//对于解码出来的AVFrame类型的yuv。注意，linesize对应的是一行的长度，比如测试视频是1280x720，且
		//format是AV_PIX_FMT_YUV420P，则linesize [0]/[1]/[2]分别表示一行的yuv长度，为1280/640/640.
	    //对于音频，则是一帧音频的字节数，如果音频是float型（AV_SAMPLE_FMT_S32P）且是双通道的，则
	    //linesize = 4 * 2 * as->codecpar->frame_size(as->codecpar->frame_size表示一帧数据，单通道样本数)
	}
	XFFmpeg::get()->close();
	cout << "[THREAD] ------ Video thread exit! ------" << endl;
}

XVideoThread:: ~XVideoThread() {

}

XVideoThread::XVideoThread() {  // 不让外部创建线程实例

}