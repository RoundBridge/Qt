#include "XAudioThread.h"
#include "XDistributeThread.h"
#include "XAudioPlay.h"
#include "XFFmpeg.h"
#include "xplay1.h"
#include <iostream>
#include <QTime>

using std::cout;
using std::endl;

bool XAudioThread::isExit = true;
bool XAudioThread::isStart= false;
bool XAudioThread::bReset = false;

void XAudioThread::run() {
	int free = 0;
	char aout[MAXAUDIOSWRLEN] = { 0 };  // 用于存放audio_convert出来的音频数据

	while (!isExit) {
		if (bReset) {
			cout << "[AUDIO THREAD] ------ Reset audio thread! ------" << endl;
			break;
		}
		if (!XAudioThread::isStart) {
			msleep(10);
			continue;
		}
		// 获取出来的空余音频缓存大小不够写一帧音频数据
		free = XAudioPlay::get()->get_free_buffer_size();
		if (free < 23040)
		{
			// 测试发现音频输出写缓存的大小是38400，每次以十分之一的缓存大小作为使用单位，所以一次写入4096字节的话，用去了3840*2的空间
			// 为了防止声音断续，且为了能更准确地同步视频时间戳（尽可能少的缓存音频帧），缓存中预留一帧备用，因此需要满足空余空间不小于
			// （38400 - 3840*4 = 23040）时才写入。问题：为什么音频输出写缓存的大小是38400？跟硬件有无关系？跟系统有无关系？同样的疑问
			// 也适用于每次以十分之一的缓存大小作为使用单位这一点上！！！因此这里是经验值！
			//cout << "audio free buffer size " << free << ", total size " << XAudioPlay::get()->get_buffer_size() << endl;
			msleep(1);
			continue;
		}
		else
		{
			//cout << "--------------------------------------------free " << free << endl;
		}

		XPlay1::rlock();
		if (XDistributeThread::get()->get_audio_list()->size() > 0)
		{
			//cout << "[AUDIO THREAD] ------ Audio thread running! ------" << endl;
			AVPacket* pkta = XDistributeThread::get()->get_audio_list()->front();
			XFFmpeg::get()->decode(pkta);
			av_packet_unref(pkta);
			if (pkta) av_packet_free(&pkta);
			XDistributeThread::get()->get_audio_list()->pop_front();
			int len = XFFmpeg::get()->audio_convert((uint8_t* const)aout);
			if (len > 0) {				
				XAudioPlay::get()->write(aout, len);
				//cout << "Output audio data size " << len << endl;	// 测试发现，一帧是4096字节，即42.7ms
			}
		}
		else {
			msleep(5);
		}
		XPlay1::unlock();
	}
	isExit = true;
	cout << "[AUDIO THREAD] ------ Audio thread exit! ------" << endl;
}

XAudioThread::XAudioThread() {

}

XAudioThread::~XAudioThread() {

}
