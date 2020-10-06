#include "XAudioPlay.h"
#include <QAudioOutput>
#include <QIODevice>

XAudioPlay::~XAudioPlay() {

}

XAudioPlay::XAudioPlay() {

}

class AudioPlay : public XAudioPlay
{
public:
	QAudioOutput* output = NULL;
	QIODevice* io = NULL;
	static AudioPlay* get();
	virtual ~AudioPlay();
	bool start();
	bool stop();
	bool play(bool isPlay);
	int get_free_buffer_size();
	bool write(const char* data, int dataSize);
private:
	AudioPlay();
};

AudioPlay::AudioPlay():XAudioPlay() {

}

AudioPlay::~AudioPlay() {

}

AudioPlay* AudioPlay::get() {
	static AudioPlay ap;
	return &ap;
}

bool AudioPlay::start() {
	QAudioFormat fmt;

	// 每次打开一个新的音频文件先stop，清理下上次打开的音频文件
	if (true != stop()) {
		return false;
	}
	fmt.setSampleRate(48000);
	fmt.setSampleSize(16);
	fmt.setChannelCount(2);
	fmt.setCodec("audio/pcm");
	fmt.setByteOrder(QAudioFormat::LittleEndian);
	fmt.setSampleType(QAudioFormat::UnSignedInt);
	output = new QAudioOutput(fmt);
	io = output->start();  // start()会返回一个QIODevice对象

	return true;
}

bool AudioPlay::stop() {
	if (output) {
		output->stop();
		delete output;
		output = NULL;
		io = NULL;
	}
	return true;
}

/*
	播放还是暂停函数
*/
bool AudioPlay::play(bool isPlay) {
	if (!output) {
		return false;
	}
	if (isPlay) {
		output->resume();
	}
	else {
		output->suspend();
	}
	return true;
}
/*
	返回有多少缓存空间可用
*/
int AudioPlay::get_free_buffer_size() {
	if (!output)
	{
		return 0;
	}
	else {
		return output->bytesFree();
	}
}

bool AudioPlay::write(const char* data, int dataSize) {
	if (!io) {
		return false;
	}
	io->write(data, dataSize);
	return true;
}