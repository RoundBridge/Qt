#pragma once
#include <list>
#include <QThread>
#include "XFFmpeg.h"

using std::list;

class XDistributeThread:public QThread
{
public:
	static bool isExit;
	static bool bReset;		// 分发线程是否需要退出一下
	static XDistributeThread* get() {
		static XDistributeThread dt;
		return &dt;
	}
	void run();  // 重写QT线程函数(在调用start之后会在线程中运行这个函数)
	list<AVPacket*>* get_video_list();
	list<AVPacket*>* get_audio_list();
	void close();
	void lock();
	void unlock();
	virtual ~XDistributeThread();

private:
	XDistributeThread();  // 不让外部创建线程实例
	list<AVPacket*> videolist;
	list<AVPacket*> audiolist;
	QMutex mutex;
};

