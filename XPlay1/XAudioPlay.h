#pragma once
class XAudioPlay
{
public:
	virtual ~XAudioPlay();
	virtual bool start() = 0;
	virtual bool stop() = 0;
	virtual bool play(bool isPlay) = 0;
	virtual int get_free_buffer_size() = 0;
	virtual bool write(const char* data, int dataSize) = 0;

protected:
	// 采样率
	int sampleRate = 48000;
	// 样本点比特数
	int sampleSize = 16;
	// 通道数
	int channel = 2;
	XAudioPlay();
};

