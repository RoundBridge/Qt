#pragma once
#include <string>
#include <qmutex.h>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
}

const int error_len = 1024;

class XFFmpeg
{
public:
	static XFFmpeg* get() {
		/*
		a. 静态成员函数时属于类的,不是对象的; 
		b. 静态成员函数的调用方式和静态成员变量的方法类似。 
		c. 静态成员函数不能够调用普通的成员函数和普通的成员变量,因为静态成员函数属于类,
		不知道普通的成员属性属于哪个对象,只能调用静态的类的资源。
		*/
		static XFFmpeg f_obj;
		/*
		把一个类的成员说明为static时,该类无论创建多少个对象,这些对象都共享这个static成员; 
		静态成员变量属于类,不属于对象; 定义静态成员变量的时候,是在类的外部。
		*/
		return &f_obj;
	}
	virtual ~XFFmpeg();
	int open(const char* path);
	AVPacket* read();
	AVFrame *decode(const AVPacket* pkt);
	bool video_convert(const AVFrame *frame, uint8_t* const out, int out_w, int out_h, AVPixelFormat out_pixfmt);
	void close();
	std::string get_error(int error_num);
	int get_duration_ms();
	int videoStream = 0;
	int audioStream = 0;

protected:
	void compute_duration_ms();  // 计算文件总共的播放时长，以毫秒为单位
	bool create_decoder(AVFormatContext* ic);  // 创建解码器，用于解码（内部用）
	XFFmpeg();	// 外部不能生成对象了，外部定义对象会失败
	char error_buf[error_len];
	QMutex mutex;
	//解封装上下文 
	//NULL表示由ffmpeg申请空间，用完后也由ffmpeg内部释放
	//如果用户申请空间，则用完后用户自己完成空间释放工作
	//如果用户释放ffmpeg申请的空间会有问题，因为ffmpeg是
	//在动态库里申请的，用户不一定能delete掉
	AVFormatContext* ic = NULL;  // C++11类中可以直接对成员变量赋值
	AVCodecContext* ac = NULL;  // 音频解码器上下文
	AVCodecContext* vc = NULL;  // 视频解码器上下文		
	SwsContext* vSwsCtx = NULL;  // 视频像素尺寸及格式转换上下文
	AVPacket *packet = NULL;
	AVFrame *yuv = NULL;
	int total_ms = 0;	// 文件总时长，毫秒为单位
};

