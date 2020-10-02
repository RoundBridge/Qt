#pragma once
#include <QThread>
class XVideoThread:public QThread
{
public:
	static bool isExit;
	static XVideoThread *get() {
		static XVideoThread vt;
		return &vt;
	}
	void run();  // 重写QT线程函数(在调用start之后会在线程中运行这个函数)
	virtual ~XVideoThread();

private:
	XVideoThread();  // 不让外部创建线程实例
};

