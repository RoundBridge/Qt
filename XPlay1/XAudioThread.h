#pragma once
#include <QThread>
class XAudioThread:public QThread
{
public:
	static bool isExit;
	static bool isStart;
	static bool bReset;		// 线程是否需要退出一下
	static XAudioThread* get() {
		static XAudioThread at;
		return &at;
	}
	void run();  // 重写QT线程函数(在调用start之后会在线程中运行这个函数)
	virtual ~XAudioThread();

private:
	XAudioThread();  // 不让外部创建线程实例
};

