#pragma once
#include <QThread>
class XAudioThread:public QThread
{
public:
	static bool isExit;
	static bool isStart;
	static bool bReset;		// �߳��Ƿ���Ҫ�˳�һ��
	static XAudioThread* get() {
		static XAudioThread at;
		return &at;
	}
	void run();  // ��дQT�̺߳���(�ڵ���start֮������߳��������������)
	virtual ~XAudioThread();

private:
	XAudioThread();  // �����ⲿ�����߳�ʵ��
};

