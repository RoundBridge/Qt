#pragma once
#include <list>
#include <QThread>
#include "XFFmpeg.h"

using std::list;

class XDistributeThread:public QThread
{
public:
	static bool isExit;
	static bool bReset;		// �ַ��߳��Ƿ���Ҫ�˳�һ��
	static XDistributeThread* get() {
		static XDistributeThread dt;
		return &dt;
	}
	void run();  // ��дQT�̺߳���(�ڵ���start֮������߳��������������)
	list<AVPacket*>* get_video_list();
	list<AVPacket*>* get_audio_list();
	void close();
	void lock();
	void unlock();
	virtual ~XDistributeThread();

private:
	XDistributeThread();  // �����ⲿ�����߳�ʵ��
	list<AVPacket*> videolist;
	list<AVPacket*> audiolist;
	QMutex mutex;
};

