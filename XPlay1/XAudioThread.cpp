#include "XAudioThread.h"
#include "XDistributeThread.h"
#include "XAudioPlay.h"
#include "XFFmpeg.h"
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
		if (free < 4096)
		{
			msleep(1);
			continue;
		}

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
				//cout << "Output audio data size " << len << endl;
			}
		}
		else {
			msleep(1);
		}
	}
	isExit = true;
	cout << "[AUDIO THREAD] ------ Audio thread exit! ------" << endl;
}

XAudioThread::XAudioThread() {

}

XAudioThread::~XAudioThread() {

}
