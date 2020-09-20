#include "XFFmpeg.h"
#include <iostream>
using std::cout;
using std::endl;

#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")

int XFFmpeg::open(const char* path) {	
	//�ȹر����е���Դ
	close();
	//��������
	AVDictionary* opts = NULL;
	//��av_dict_set�������
	av_dict_set(&opts, "rtsp_transport", "tcp", 0);  //����rtsp����tcp��
	av_dict_set(&opts, "max_delay", "500", 0);  //����������ʱ

	mutex.lock();
	int re = avformat_open_input(
		&ic,
		path,
		0,  //0��ʾ�Զ�ѡ����װ��
		&opts  //�������ã�����rtsp����ʱʱ�䣬��NULL��ʾĬ������
	);
	// �򿪳ɹ�������£��ѹ�������ļ��������Ϣ�����ºã������ⲿʹ��
	if (0 == re) {
		compute_duration_ms();
	}
	mutex.unlock();

	return re;
}
void XFFmpeg::compute_duration_ms() {
	if (ic) {
		total_ms = ic->duration / (AV_TIME_BASE / 1000);
	}
	else {
		total_ms = 0;
	}
	return;
}
int XFFmpeg::get_duration_ms() {
	return total_ms;
}
void XFFmpeg::close() {
	mutex.lock();
	if (ic) avformat_close_input(&ic);
	mutex.unlock();
}
std::string XFFmpeg::get_error(int error_num) {	
	mutex.lock();
	av_strerror(error_num, error_buf, error_len - 1);
	std::string re = this->error_buf;  // ��һ��char*��ֵ��string����ʱ�Ḵ��һ�ݣ���һ�����ƵĿռ������̷߳��ʳ����쳣
	mutex.unlock();
	return re;
}
XFFmpeg::XFFmpeg() {
	//������Ϣ�����ʼ��
	error_buf[error_len] = '\0';
	//��ʼ����װ�⣨��ʵ���°汾��������ѱ����������û����������Ϊ�ѷ���Ĵ���
	av_register_all();
	//��ʼ������⣬���Դ�rtsp��rtmp��http��Э�����ý����Ƶ
	avformat_network_init();
}

XFFmpeg::~XFFmpeg() {

}