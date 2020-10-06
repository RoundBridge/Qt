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
		a. ��̬��Ա�������������,���Ƕ����; 
		b. ��̬��Ա�����ĵ��÷�ʽ�;�̬��Ա�����ķ������ơ� 
		c. ��̬��Ա�������ܹ�������ͨ�ĳ�Ա��������ͨ�ĳ�Ա����,��Ϊ��̬��Ա����������,
		��֪����ͨ�ĳ�Ա���������ĸ�����,ֻ�ܵ��þ�̬�������Դ����̬��Ա��������������
		��this�Ա��������ԣ������޷������Լ���ķǾ�̬��Ա����
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
	bool decode(const AVPacket* pkt);
	AVFrame *get_buffered_frames();  // ��ȡ��������׶λ����ڽ������е�ͼ������
	bool video_convert(uint8_t* const out, int out_w, int out_h, AVPixelFormat out_pixfmt);
	int audio_convert(uint8_t* const out);
	bool send_flush_packet();
	void set_send_flush_packet(bool flag);
	void close();
	std::string get_error(int error_num);
	int get_duration_ms(int streamId);
	int get_video_fps();
	int get_current_video_pts();
	bool seek(float pos); // pos���϶���������ʱ�����ϵ�λ�ðٷֱȣ�ȡֵ��Χ0~1
	int videoStream = 0;
	int audioStream = 0;

protected:
	void compute_duration_ms();  // �����ļ��ܹ��Ĳ���ʱ�����Ժ���Ϊ��λ
	void compute_video_fps();	 // �����ļ���֡��
	void compute_current_pts(AVFrame*, int);  // ���㵱ǰ�Ѿ����ŵ���ʱ�����Ժ���Ϊ��λ
	bool create_decoder(AVFormatContext* ic);  // ���������������ڽ��루�ڲ��ã�
	void clean();
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
	int totalVms = 0;	// ��Ƶ��ʱ��������Ϊ��λ
	int totalAms = 0;	// ��Ƶ��ʱ��������Ϊ��λ
	int currentVPtsMs = 0;  // ��ǰ�Ѳ��ŵ���Ƶ��ʱ��������Ϊ��λ
	int currentAPtsMs = 0;  // ��ǰ�Ѳ��ŵ���Ƶ��ʱ��������Ϊ��λ
	int fps = 0;	// ��Ƶ֡��
	bool bSendFlushPacket = false;  // ��ȡ��Ƶ������������Ƿ���������֡

	// ������
	int sampleRate = 48000;
	// �����������
	int sampleSize = 16;
	// ͨ����
	int channel = 2;
	// �洢�������Ƶ���ݵ�ָ֡��
	AVFrame* pcm = NULL;
	// ��Ƶ�ز���ת��������
	SwrContext* aSwrCtx = NULL;
};

