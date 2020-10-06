#pragma once
#include <string>
#include <qmutex.h>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libswresample/swresample.h>
}

const int error_len = 1024;

class XFFmpeg
{
public:
	static XFFmpeg* get() {
		/*
		a. 静态成员函数是属于类的,不是对象的; 
		b. 静态成员函数的调用方式和静态成员变量的方法类似。 
		c. 静态成员函数不能够调用普通的成员函数和普通的成员变量,因为静态成员函数属于类,
		不知道普通的成员属性属于哪个对象,只能调用静态的类的资源（静态成员函数不接受隐含
		的this自变量。所以，它就无法访问自己类的非静态成员）。
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
	bool decode(const AVPacket* pkt);
	AVFrame *get_buffered_frames();  // 获取解码结束阶段缓存在解码器中的图像数据
	bool video_convert(uint8_t* const out, int out_w, int out_h, AVPixelFormat out_pixfmt);
	int audio_convert(uint8_t* const out);
	bool send_flush_packet();
	void set_send_flush_packet(bool flag);
	void close();
	std::string get_error(int error_num);
	int get_duration_ms(int streamId);
	int get_video_fps();
	int get_current_video_pts();
	bool seek(float pos); // pos是拖动滑动条在时间轴上的位置百分比，取值范围0~1
	int videoStream = 0;
	int audioStream = 0;

protected:
	void compute_duration_ms();  // 计算文件总共的播放时长，以毫秒为单位
	void compute_video_fps();	 // 计算文件的帧率
	void compute_current_pts(AVFrame*, int);  // 计算当前已经播放的总时长，以毫秒为单位
	bool create_decoder(AVFormatContext* ic);  // 创建解码器，用于解码（内部用）
	void clean();
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
	int totalVms = 0;	// 视频总时长，毫秒为单位
	int totalAms = 0;	// 音频总时长，毫秒为单位
	int currentVPtsMs = 0;  // 当前已播放的视频总时长，毫秒为单位
	int currentAPtsMs = 0;  // 当前已播放的音频总时长，毫秒为单位
	int fps = 0;	// 视频帧率
	bool bSendFlushPacket = false;  // 读取视频解码包结束后是否发送清理缓存帧

	// 采样率
	int sampleRate = 48000;
	// 样本点比特数
	int sampleSize = 16;
	// 通道数
	int channel = 2;
	// 存储解码后音频数据的帧指针
	AVFrame* pcm = NULL;
	// 音频重采样转换上下文
	SwrContext* aSwrCtx = NULL;
};

