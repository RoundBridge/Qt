#include "XFFmpeg.h"
#include <iostream>
using std::cout;
using std::endl;

#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "swscale.lib")

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
		re = create_decoder(ic);
	}
	mutex.unlock();

	return re;
}

AVPacket* XFFmpeg::read() {
	int re = 0;
	// read�������治�����packet���ͷŲ�������Ҫ�ⲿ�ͷ�
	if (packet == NULL) {
		packet = av_packet_alloc();
	}
	mutex.lock();
	if (!ic) {
		mutex.unlock();
		cout << "[PACKET] Please open first!" << endl;
		return NULL;
	}	
	re = av_read_frame(ic, packet);
	if (0 != re) {
		mutex.unlock();
		cout << "[PACKET] read frame error: " << get_error(re) << endl;
		return NULL;
	}
	mutex.unlock();
	return packet;
}

AVFrame* XFFmpeg::decode(const AVPacket* pkt) {
	AVCodecContext* cc = NULL;

	if (yuv == NULL) {
		yuv = av_frame_alloc();
	}

	if (pkt->stream_index == videoStream) { cc = vc; }
	if (pkt->stream_index == audioStream) { cc = ac; }

	mutex.lock();
	// �������Ƶ����pkt�е�stream_indexֱ��ָ����Ƶ����idx����Ƶ��ͬ��
	// ����packet�������̣߳�ֻ�ǽ�pkt�Ӹ������̣߳�������ռ��CPU
	// send NULL����ö��receive����ȡ�����л���֡
	int re = avcodec_send_packet(cc, pkt);
	if (0 != re) {
		mutex.unlock();
		cout << "[DECODE] avcodec_send_packet failed: " << get_error(re) << endl;
		return NULL;
	}
	// ֻ�Ǵӽ����߳��л�ȡ��������������ռ��CPU��һ��send��һ�ο��ܷ��Ͷ����Ƶ֡�����ܶ�Ӧ���receive
	// Ϊʲô����ֻ��ȡһ�Σ��ѵ���Ӧ�ö��ȡ���Σ��������֡ȫ����ȡ������
	// ����avcodec_receive_frame�ĵ�һ������������ic->streams[pkt->stream_index]->codec�ˣ���Ϊ���Ѿ���
	// ���������ˣ������Ļ�����������ʧ��
	re = avcodec_receive_frame(cc, yuv);
	if (0 != re) {
		mutex.unlock();
		cout << "[DECODE] avcodec_receive_frame failed: " << get_error(re) << endl;
		return NULL;
	}
	mutex.unlock();
	return yuv;
}

bool XFFmpeg::video_convert(const AVFrame* frame, uint8_t* const out, int out_w, int out_h, AVPixelFormat out_pixfmt) {
	int re = 0;
	uint8_t* data[AV_NUM_DATA_POINTERS] = { 0 };
	int lines[AV_NUM_DATA_POINTERS] = { 0 };

	if (out_pixfmt == AV_PIX_FMT_ARGB
		|| out_pixfmt == AV_PIX_FMT_RGBA
		|| out_pixfmt == AV_PIX_FMT_ABGR
		|| out_pixfmt == AV_PIX_FMT_BGRA) {
		data[0] = out;
		lines[0] = out_w * 4;
	}
	else if (out_pixfmt == AV_PIX_FMT_YUV422P) {
		data[0] = out;
		data[1] = data[0] + out_w * out_h;
		data[2] = data[1] + out_w * out_h / 2;
		lines[0] = out_w;
		lines[1] = out_w / 2;
		lines[2] = out_w / 2;
	}

	mutex.lock();
	vSwsCtx = sws_getCachedContext(
		/*
		* If context is NULL, just calls sws_getContext() to get a new
		* context.Otherwise, checks if the parameters are the ones already
		* saved in context.If that is the case, returns the current
		* context.Otherwise, frees context and gets a new context with
		* the new parameters.
		*/
		vSwsCtx,  //��NULL���´���������Ƚϸ������ĺ�������Щ������Ҫ�����������������Ƿ�һ�£���һ����ֱ�ӷ��ظ������ģ���һ���򴴽��µ�������
		frame->width, frame->height,	//����Ŀ��
		(AVPixelFormat)frame->format,	//����ĸ�ʽ��YUV420p
		out_w, out_h,					//����Ŀ��
		out_pixfmt,						//�����ʽ
		SWS_BILINEAR,					//�ߴ�ת�����㷨
		0, 0, 0
	);
	if (vSwsCtx) {
		// sws_scale�����Ŀ����ܴ�
		re = sws_scale(			//����ֵ the height of the output slice
			vSwsCtx,
			frame->data,		//�������ݵ�ַ
			frame->linesize,	//�����п��
			0,					//the position in the source image of the slice to *process, 
								//that is the number(counted starting from
								// zero) in the image of the first row of the slice
			frame->height,		//����߶�
			data,				//������ݵ�ַ
			lines				//����п��
		);
		cout << "[CONVERT] the height of the output is: " << re << endl;
	}
	mutex.unlock();
	return true;
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

bool XFFmpeg::create_decoder(AVFormatContext* ic) {
	int re = 0;

	if (!ic) {
		cout << "[CREATE DECODER] Please open first!" << endl;
		return -5;
	}

	//������ȡvideoStream��audioStream��Ȼ����뵽ic->streams[i]�л�ȡ����������Ϣ���Ͳ��ñ�����
	videoStream = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);  // �����ڶ���������NULL����Ϊ��װ�ͽ�����뿪����ϣ�
	audioStream = av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
	cout << "[CREATE DECODER] videoStream = " << videoStream << endl;
	cout << "[CREATE DECODER] audioStream = " << audioStream << endl;

	/************************************ ��Ƶ���������� ************************************/
	//�ҵ���Ƶ������
	AVCodec* vcodec = avcodec_find_decoder(ic->streams[videoStream]->codecpar->codec_id);
	if (!vcodec) {
		cout << "[CREATE DECODER] Can't find the video codec id " << ic->streams[videoStream]->codecpar->codec_id << endl;
		return -1;
	}
	else {
		cout << "[CREATE DECODER] Find the vcodec " << ic->streams[videoStream]->codecpar->codec_id << endl;
	}

	//����������������
	if (vc == NULL) { vc = avcodec_alloc_context3(vcodec); }	

	//���ý����������Ĳ���
	//�°汾�Ѿ���AVStream�ṹ���е�AVCodecContext�ֶζ���Ϊ�������ԡ�
	//����޷���ɵİ汾ֱ��ͨ��AVFormatContext��ȡ��AVCodecContext�ṹ�����
	//��ǰ�汾��������Ƶ����Ϣ�Ľṹ��AVCodecParameters��FFmpeg�ṩ�˺���avcodec_parameters_to_context
	//������Ƶ����Ϣ�������µ�AVCodecContext�ṹ����
	avcodec_parameters_to_context(vc, ic->streams[videoStream]->codecpar);

	//4�߳̽���
	//Ҳ������ͨ��linux����Windows API����ȡCPU�����������������߳���
	vc->thread_count = 4;

	//�򿪽�����������
	re = avcodec_open2(vc, 0, 0);
	if (0 != re) {
		cout << "[CREATE DECODER] avcodec_open2 failed: " << get_error(re) << endl;
		return -2;
	}
	else {
		cout << "[CREATE DECODER] video - avcodec_open2 success!" << endl;
	}

	/************************************ ��Ƶ���������� ************************************/
	//�ҵ���Ƶ������
	AVCodec* acodec = avcodec_find_decoder(ic->streams[audioStream]->codecpar->codec_id);
	if (!acodec) {
		cout << "[CREATE DECODER] Can't find the audio codec id " << ic->streams[audioStream]->codecpar->codec_id << endl;
		return -3;
	}
	else {
		cout << "[CREATE DECODER] Find the acodec " << ic->streams[audioStream]->codecpar->codec_id << endl;
	}

	//����������������
	if (ac == NULL) { ac = avcodec_alloc_context3(acodec); }

	//���ý����������Ĳ���
	avcodec_parameters_to_context(ac, ic->streams[audioStream]->codecpar);

	//4�߳̽���
	ac->thread_count = 4;

	//�򿪽�����������
	/*
		����avcodec_open2�����ĵڶ���������If a non-NULL codec has been previously passed to 
		avcodec_alloc_context3() or for this context, then this parameter MUST be either NULL 
		or equal to the previously passed codec.
	 */
	re = avcodec_open2(ac, 0, 0);
	if (0 != re) {		
		cout << "[CREATE DECODER] avcodec_open2 failed: " << get_error(re) << endl;
		return -4;
	}
	else {
		cout << "[CREATE DECODER] audio - avcodec_open2 success!" << endl;
	}

	return 0;
}

int XFFmpeg::get_duration_ms() {
	return total_ms;
}

void XFFmpeg::close() {
	mutex.lock();
	if (ic) avformat_close_input(&ic);
	if (vc)avcodec_free_context(&vc);
	if (ac)avcodec_free_context(&ac);
	if (vSwsCtx) { 
		sws_freeContext(vSwsCtx); 
		vSwsCtx = NULL;
	}
	if (packet) av_packet_free(&packet);
	if (yuv) av_frame_free(&yuv);
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