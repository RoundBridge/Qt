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
		如果下面的sample等成员变量是私有的，则即使是公有继承，子类中的start之类的函数也不能访问，想要访问
		父类的私有成员变量，则可以在父类中定义共有的成员函数，这些父类的公有成员函数调用父类的私有成员变量，
		然后公有继承的子类调用父类的公有成员函数，进而间接调用父类的私有成员变量。
	*/
public:
	// 采样率
	int sampleRate = 48000;
	// 样本点比特数
	int sampleSize = 16;
	// 通道数
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
