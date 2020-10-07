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

	while (!isExit) {
		if (!XVideoThread::isStart) {
			msleep(10);
			continue;
		}
		// 获取出来的空余音频缓存大小不够写一帧音频数据
		if (XAudioPlay::get()->get_free_buffer_size() < 7680)
		{
			cout << "aaaaaaaaaaaaaaa" << endl;
			msleep(1);
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
			if (pkt->stream_index == XFFmpeg::get()->audioStream)
			{
				XFFmpeg::get()->decode(pkt);
				av_packet_unref(pkt);
				int len = XFFmpeg::get()->audio_convert((uint8_t* const)aout);
				XAudioPlay::get()->write(aout, len);
				continue;
			}
			else if (pkt->stream_index == XFFmpeg::get()->videoStream) {
				XFFmpeg::get()->decode(pkt);
				av_packet_unref(pkt);
			}
			else{
				cout << "[THREAD] Unknown stream ID: " << pkt->stream_index << endl;
				av_packet_unref(pkt);
			}
		}
		
		if (XFFmpeg::get()->get_video_fps() > 0) {
			msleep(1000 / XFFmpeg::get()->get_video_fps());
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