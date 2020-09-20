#pragma once
#include <string>
#include <qmutex.h>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
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
	bool open(const char* path);
	void close();
	std::string get_error(int error_num);
	int total_ms = 0;	// �ļ���ʱ��������Ϊ��λ
protected:
	char error_buf[error_len];
	QMutex mutex;
	//���װ������ 
	//NULL��ʾ��ffmpeg����ռ䣬�����Ҳ��ffmpeg�ڲ��ͷ�
	//����û�����ռ䣬��������û��Լ���ɿռ��ͷŹ���
	//����û��ͷ�ffmpeg����Ŀռ�������⣬��Ϊffmpeg��
	//�ڶ�̬��������ģ��û���һ����delete��
	AVFormatContext* ic = NULL;  // C++11���п���ֱ�ӶԳ�Ա������ֵ
	XFFmpeg();	// �ⲿ�������ɶ����ˣ��ⲿ��������ʧ��

};

