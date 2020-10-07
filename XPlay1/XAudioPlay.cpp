#include "XAudioPlay.h"

XAudioPlay::~XAudioPlay() {

}

XAudioPlay::XAudioPlay() {

}

AudioPlay::AudioPlay():XAudioPlay() {

}

AudioPlay::~AudioPlay() {

}

// get函数属于抽象类XAudioPlay，但它返回一个子类对象的指针（转换成父类指针）
// 子类AudioPlay的构造函数必须是public的，不然代码“static AudioPlay ap;”出错
// 如果下面的代码写成“XAudioPlay* AudioPlay::get(){...} ”，则表示在子类中
// 重新定义一个get函数，如果不重新定义一个函数，而是用父类的get函数，那么域
// 作用域还是“XAudioPlay::”,子类继承父类的函数还是属于父类的，所以要用父类
// 的作用域。
XAudioPlay* XAudioPlay::get() {
	static AudioPlay ap;
	return &ap;
}

bool AudioPlay::start() {
	QAudioFormat fmt;

	// 每次打开一个新的音频文件先stop，清理下上次打开的音频文件
	if (true != stop()) {
		return false;
	}
	mutex.lock();
	fmt.setSampleRate(48000);
	fmt.setSampleSize(16);
	fmt.setChannelCount(2);
	fmt.setCodec("audio/pcm");
	fmt.setByteOrder(QAudioFormat::LittleEndian);
	fmt.setSampleType(QAudioFormat::UnSignedInt);
	output = new QAudioOutput(fmt);
	io = output->start();  // start()会返回一个QIODevice对象
	mutex.unlock();

	return true;
}

bool AudioPlay::stop() {
	mutex.lock();
	if (output) {
		output->stop();
		delete output;
		output = NULL;
		io = NULL;
	}
	mutex.unlock();
	return true;
}

/*
	播放还是暂停函数
*/
bool AudioPlay::play(bool isPlay) {
	mutex.lock();
	if (!output) {
		mutex.unlock();
		return false;
	}
	if (isPlay) {
		output->resume();
	}
	else {
		output->suspend();
	}
	mutex.unlock();
	return true;
}
/*
	返回有多少缓存空间可用
*/
int AudioPlay::get_free_buffer_size() {
	mutex.lock();
	if (!output)
	{
		mutex.unlock();
		return 0;
	}
	else {
		int free = output->bytesFree();
		mutex.unlock();
		return free;
	}
}

bool AudioPlay::write(const char* data, int dataSize) {
	mutex.lock();
	if (!io) {
		mutex.unlock();
		return false;
	}
	io->write(data, dataSize);
	mutex.unlock();
	return true;
}