#include "XFFmpeg.h"
#include <iostream>
using std::cout;
using std::endl;

#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")

bool XFFmpeg::open(const char* path) {	
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
	if (0 != re) {
		mutex.unlock();
		cout << "open " << path << " failed: " << get_error(re) << endl;
		return false;
	}
	mutex.unlock();
	total_ms = ic->duration / (AV_TIME_BASE / 1000);
	cout << "open "<< path << " success! The total length is " << total_ms <<" ms." << endl;
	return true;
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
	error_buf[error_len] = '\0';
	//��ʼ����װ�⣨��ʵ���°汾��������ѱ����������û����������Ϊ�ѷ���Ĵ���
	av_register_all();
	//��ʼ������⣬���Դ�rtsp��rtmp��http��Э�����ý����Ƶ
	avformat_network_init();
}

XFFmpeg::~XFFmpeg() {

}