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
		a. ��̬��Ա����ʱ�������,���Ƕ����; 
		b. ��̬��Ա�����ĵ��÷�ʽ�;�̬��Ա�����ķ������ơ� 
		c. ��̬��Ա�������ܹ�������ͨ�ĳ�Ա��������ͨ�ĳ�Ա����,��Ϊ��̬��Ա����������,
		��֪����ͨ�ĳ�Ա���������ĸ�����,ֻ�ܵ��þ�̬�������Դ��
		*/
		static XFFmpeg f_obj;
		/*
		��һ����ĳ�Ա˵��Ϊstaticʱ,�������۴������ٸ�����,��Щ���󶼹������static��Ա; 
		��̬��Ա����������,�����ڶ���; ���徲̬��Ա������ʱ��,��������ⲿ��
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
	void compute_duration_ms();  // �����ļ��ܹ��Ĳ���ʱ�����Ժ���Ϊ��λ
	bool create_decoder(AVFormatContext* ic);  // ���������������ڽ��루�ڲ��ã�
	XFFmpeg();	// �ⲿ�������ɶ����ˣ��ⲿ��������ʧ��
	char error_buf[error_len];
	QMutex mutex;
	//���װ������ 
	//NULL��ʾ��ffmpeg����ռ䣬�����Ҳ��ffmpeg�ڲ��ͷ�
	//����û�����ռ䣬��������û��Լ���ɿռ��ͷŹ���
	//����û��ͷ�ffmpeg����Ŀռ�������⣬��Ϊffmpeg��
	//�ڶ�̬��������ģ��û���һ����delete��
	AVFormatContext* ic = NULL;  // C++11���п���ֱ�ӶԳ�Ա������ֵ
	AVCodecContext* ac = NULL;  // ��Ƶ������������
	AVCodecContext* vc = NULL;  // ��Ƶ������������		
	SwsContext* vSwsCtx = NULL;  // ��Ƶ���سߴ缰��ʽת��������
	AVPacket *packet = NULL;
	AVFrame *yuv = NULL;
	int total_ms = 0;	// �ļ���ʱ��������Ϊ��λ
};

