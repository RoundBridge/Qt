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
	void run();  // ��дQT�̺߳���(�ڵ���start֮������߳��������������)
	virtual ~XVideoThread();

private:
	XVideoThread();  // �����ⲿ�����߳�ʵ��
};

