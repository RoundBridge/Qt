#include "XFFmpeg.h"
#include <iostream>
using std::cout;
using std::endl;

#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")

bool XFFmpeg::open(const char* path) {	
	//先关闭现有的资源
	close();
	//参数设置
	AVDictionary* opts = NULL;
	//用av_dict_set添加属性
	av_dict_set(&opts, "rtsp_transport", "tcp", 0);  //设置rtsp流以tcp打开
	av_dict_set(&opts, "max_delay", "500", 0);  //设置网络延时
	mutex.lock();
	int re = avformat_open_input(
		&ic,
		path,
		0,  //0表示自动选择解封装器
		&opts  //参数设置，比如rtsp的延时时间，传NULL表示默认设置
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
	std::string re = this->error_buf;  // 把一个char*赋值给string对象时会复制一份，用一个复制的空间避免多线程访问出现异常
	mutex.unlock();
	return re;
}
XFFmpeg::XFFmpeg() {
	error_buf[error_len] = '\0';
	//初始化封装库（其实最新版本这个函数已被废弃，调用会产生被声明为已否决的错误）
	av_register_all();
	//初始化网络库，可以打开rtsp、rtmp、http等协议的流媒体视频
	avformat_network_init();
}

XFFmpeg::~XFFmpeg() {

}