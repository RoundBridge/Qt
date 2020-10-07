#include "XAudioPlay.h"

XAudioPlay::~XAudioPlay() {

}

XAudioPlay::XAudioPlay() {

}

AudioPlay::AudioPlay():XAudioPlay() {

}

AudioPlay::~AudioPlay() {

}

// get�������ڳ�����XAudioPlay����������һ����������ָ�루ת���ɸ���ָ�룩
// ����AudioPlay�Ĺ��캯��������public�ģ���Ȼ���롰static AudioPlay ap;������
// �������Ĵ���д�ɡ�XAudioPlay* AudioPlay::get(){...} �������ʾ��������
// ���¶���һ��get��������������¶���һ�������������ø����get��������ô��
// �������ǡ�XAudioPlay::��,����̳и���ĺ����������ڸ���ģ�����Ҫ�ø���
// ��������
XAudioPlay* XAudioPlay::get() {
	static AudioPlay ap;
	return &ap;
}

bool AudioPlay::start() {
	QAudioFormat fmt;

	// ÿ�δ�һ���µ���Ƶ�ļ���stop���������ϴδ򿪵���Ƶ�ļ�
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
	io = output->start();  // start()�᷵��һ��QIODevice����
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
	���Ż�����ͣ����
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
	�����ж��ٻ���ռ����
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