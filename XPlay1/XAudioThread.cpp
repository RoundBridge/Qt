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
	char aout[MAXAUDIOSWRLEN] = { 0 };  // ���ڴ��audio_convert��������Ƶ����

	while (!isExit) {
		if (bReset) {
			cout << "[AUDIO THREAD] ------ Reset audio thread! ------" << endl;
			break;
		}
		if (!XAudioThread::isStart) {
			msleep(10);
			continue;
		}
		// ��ȡ�����Ŀ�����Ƶ�����С����дһ֡��Ƶ����
		free = XAudioPlay::get()->get_free_buffer_size();
		if (free < 23040)
		{
			// ���Է�����Ƶ���д����Ĵ�С��38400��ÿ����ʮ��֮һ�Ļ����С��Ϊʹ�õ�λ������һ��д��4096�ֽڵĻ�����ȥ��3840*2�Ŀռ�
			// Ϊ�˷�ֹ������������Ϊ���ܸ�׼ȷ��ͬ����Ƶʱ������������ٵĻ�����Ƶ֡����������Ԥ��һ֡���ã������Ҫ�������ռ䲻С��
			// ��38400 - 3840*4 = 23040��ʱ��д�롣���⣺Ϊʲô��Ƶ���д����Ĵ�С��38400����Ӳ�����޹�ϵ����ϵͳ���޹�ϵ��ͬ��������
			// Ҳ������ÿ����ʮ��֮һ�Ļ����С��Ϊʹ�õ�λ��һ���ϣ�������������Ǿ���ֵ��
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
				//cout << "Output audio data size " << len << endl;	// ���Է��֣�һ֡��4096�ֽڣ���42.7ms
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
