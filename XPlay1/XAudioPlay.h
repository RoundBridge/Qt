#pragma once
#include <QAudioOutput>
#include <QIODevice>
#include <qmutex.h>

class XAudioPlay
{
public:
	static XAudioPlay* get();
	virtual ~XAudioPlay();
	virtual bool start() = 0;
	virtual bool stop() = 0;
	virtual bool play(bool isPlay) = 0;
	virtual int get_free_buffer_size() = 0;
	virtual int get_buffer_size() = 0;
	virtual bool write(const char* data, int dataSize) = 0;

	/*
		��������sample�ȳ�Ա������˽�еģ���ʹ�ǹ��м̳У������е�start֮��ĺ���Ҳ���ܷ��ʣ���Ҫ����
		�����˽�г�Ա������������ڸ����ж��干�еĳ�Ա��������Щ����Ĺ��г�Ա�������ø����˽�г�Ա������
		Ȼ���м̳е�������ø���Ĺ��г�Ա������������ӵ��ø����˽�г�Ա������
	*/
public:
	// ������
	int sampleRate = 48000;
	// �����������
	int sampleSize = 16;
	// ͨ����
	int channel = 2;
protected:
	XAudioPlay();
};

class AudioPlay : public XAudioPlay
{
public:
	QAudioOutput* output = NULL;
	QIODevice* io = NULL;
	QMutex mutex;

	virtual ~AudioPlay();
	bool start();
	bool stop();
	bool play(bool isPlay);
	int get_free_buffer_size();
	int get_buffer_size();
	bool write(const char* data, int dataSize);
	AudioPlay();
};
