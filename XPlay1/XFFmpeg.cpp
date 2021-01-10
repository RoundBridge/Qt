#include "XFFmpeg.h"
#include "XVideoThread.h"
#include "XAudioThread.h"
#include <iostream>
using std::cout;
using std::endl;

#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "swscale.lib")
#pragma comment(lib, "swresample.lib")

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
	if (0 <= re) {	// 0��ʾ�ɹ���������ʾʧ�ܣ�������ʱ�᷵�����������������������ж� 0<=re
		re = create_decoder(ic);		
		compute_duration_ms();
		compute_video_fps();
	}
	mutex.unlock();

	return re; // ���ҽ�re>=0��ʾ�ɹ�
}

AVPacket* XFFmpeg::read() {
	int re = 0;

	mutex.lock();
	// read�������治�����packet���ͷŲ�������Ҫ�ⲿ�ͷ�
	if (packet == NULL) {
		packet = av_packet_alloc();
	}
	if (!ic) {
		mutex.unlock();
		cout << "[PACKET] Please open first!" << endl;
		return NULL;
	}	
	re = av_read_frame(ic, packet);
	if (0 != re) {
		mutex.unlock();
		cout << "[PACKET] read frame error(" << re << "): " << get_error(re) << endl;
		if (re == -541478725) {  // -541478725����ֵ�ǲ��Գ����ı�ʾ�����ļ�ĩβ
			set_send_flush_packet(true);
		}
		return NULL;
	}
	else {
		//cout <<"stream id: "<< packet->stream_index << ", " << (uint)packet <<", " << (uint)packet->data << endl;
		if (packet->stream_index == videoStream) {
			if (packet->flags & AV_PKT_FLAG_KEY) {
				//cout << "[PACKET] V --- read KEY packet! ---" << endl;
			}
			// ��Щ��Ƶ��������packet�е�pts��δ�����
			if (packet->pts == AV_NOPTS_VALUE) {  // AV_NOPTS_VALUE ((int64_t)UINT64_C(0x8000000000000000))(-9223372036854775808)
				//cout << "[PACKET] V packet pts undefined timestamp value: " << AV_NOPTS_VALUE << endl;
			}
			else {
				//cout << "[PACKET] V packet pts " << packet->pts << endl;  // AV_NOPTS_VALUE
			}
		}
		else if (packet->stream_index == audioStream) {
			if (packet->flags & AV_PKT_FLAG_KEY) {
				//cout << "[PACKET] A --- read KEY packet! ---" << endl;
			}
			// ��Щ��Ƶ��������packet�е�pts��δ�����
			if (packet->pts == AV_NOPTS_VALUE) {  // AV_NOPTS_VALUE ((int64_t)UINT64_C(0x8000000000000000))(-9223372036854775808)
				//cout << "[PACKET] A packet pts undefined timestamp value: " << AV_NOPTS_VALUE << endl;
			}
			else {
				//cout << "[PACKET] A packet pts " << packet->pts << endl;  // AV_NOPTS_VALUE
			}
		}
	}
	mutex.unlock();
	return packet;
}

bool XFFmpeg::decode(const AVPacket* pkt) {
	AVCodecContext* cc = NULL;
	AVFrame* frame = NULL;

	if (!pkt || pkt->size <= 0 || !pkt->data) {
		return false;
	}

	mutex.lock();
	if (yuv == NULL) {
		yuv = av_frame_alloc();
	}
	if (pcm == NULL) {
		pcm = av_frame_alloc();
	}

	if (pkt->stream_index == videoStream) { 
		cc = vc; 
		frame = yuv; 
	}
	else if (pkt->stream_index == audioStream) {
		cc = ac; 
		frame = pcm; 
	}
	else { 
		mutex.unlock();
		cout << "[DECODE] stream index wrong: " << pkt->stream_index << endl; 
		return false; 
	}

	// �������Ƶ����pkt�е�stream_indexֱ��ָ����Ƶ����idx����Ƶ��ͬ��
	// ����packet�������̣߳�ֻ�ǽ�pkt�Ӹ������̣߳�������ռ��CPU
	// send NULL����ö��receive����ȡ�����л���֡
	int re = avcodec_send_packet(cc, pkt);
	if (0 != re) {
		mutex.unlock();
		cout << "[DECODE] avcodec_send_packet failed: " << get_error(re) << endl;
		return false;
	}
	// ֻ�Ǵӽ����߳��л�ȡ��������������ռ��CPU��һ��send��һ�ο��ܷ��Ͷ����Ƶ֡�����ܶ�Ӧ���receive
	// Ϊʲô����ֻ��ȡһ�Σ��ѵ���Ӧ�ö��ȡ���Σ��������֡ȫ����ȡ������
	// ����avcodec_receive_frame�ĵ�һ������������ic->streams[pkt->stream_index]->codec�ˣ���Ϊ���Ѿ���
	// ���������ˣ������Ļ�����������ʧ��
	re = avcodec_receive_frame(cc, frame);	// �����receive�������send�Ƿ���Էֵ������ط���������
	if (0 != re) {
		mutex.unlock();
		cout << "[DECODE] avcodec_receive_frame failed: " << get_error(re) << endl;
		return false;
	}
	mutex.unlock();
	compute_current_pts(frame, pkt->stream_index);
	return true;
}

AVFrame* XFFmpeg::get_buffered_frames() {
	int re = 0;
	static bool flag = true;
	static int bufferedFramesCnt = 0;
		
	if (yuv == NULL) {
		cout << "[GET BUFFERED FRAMES] YUV NULL!"<< endl;
		return NULL;
	}
	mutex.lock();
	if (flag) {
		avcodec_send_packet(vc, NULL);
		flag = false;
	}
	re = avcodec_receive_frame(vc, yuv);
	if (0 != re) {
		mutex.unlock();
		// the decoder has been fully flushed, and there will be no more output frames
		if (AVERROR_EOF == re) {
			XVideoThread::isStart = false;
			XVideoThread::isExit = true;
			XAudioThread::isStart = false;
			XAudioThread::isExit = true;
			set_send_flush_packet(false);
			flag = true;
		}
		cout << "[GET BUFFERED FRAMES] " << get_error(re) << ", bufferedFramesCnt: " << bufferedFramesCnt << endl;
		bufferedFramesCnt = 0;
		return NULL;
	}
	mutex.unlock();
	compute_current_pts(yuv, videoStream);
	bufferedFramesCnt++;
	return yuv;
}

bool XFFmpeg::video_convert(uint8_t* const out, int out_w, int out_h, AVPixelFormat out_pixfmt) {
	int re = 0;
	uint8_t* data[AV_NUM_DATA_POINTERS] = { 0 };
	int lines[AV_NUM_DATA_POINTERS] = { 0 };
	const AVFrame* frame = yuv;

	if (yuv == NULL ||
		yuv->width == 0 ||
		yuv->height == 0) {
		return false;
	}
	
	if (out_pixfmt == AV_PIX_FMT_ARGB
		|| out_pixfmt == AV_PIX_FMT_RGBA
		|| out_pixfmt == AV_PIX_FMT_ABGR
		|| out_pixfmt == AV_PIX_FMT_BGRA) {
		data[0] = out;
		lines[0] = out_w * 4;
	}
	else if (out_pixfmt == AV_PIX_FMT_RGB24
		|| out_pixfmt == AV_PIX_FMT_BGR24) {
		data[0] = out;
		lines[0] = out_w * 3;
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
		vSwsCtx,  //��NULL���´���������Ƚϸ������ĺ�������Щ������Ҫ�����������������Ƿ�һ�£�
				  //��һ����ֱ�ӷ��ظ������ģ���һ���򴴽��µ�������
		frame->width, frame->height,	//����Ŀ��
		(AVPixelFormat)frame->format,	//����ĸ�ʽ��YUV420p
		out_w, out_h,					//����Ŀ��
		out_pixfmt,						//�����ʽ
		SWS_BILINEAR,					//�ߴ�ת�����㷨
		0, 0, 0
	);
	//cout << "[CONVERT] the w/h of the input is: " << frame->width << "/" << frame->height << endl;
	if (vSwsCtx) {
		// sws_scale�����Ŀ����ܴ�
		re = sws_scale(			//����ֵ the height of the output slice
			vSwsCtx,
			frame->data,		//�������ݵ�ַ
			frame->linesize,	//�����п��
			0,					//the position in the source image of the slice to process, 
								//that is the number(counted starting from
								// zero) in the image of the first row of the slice
			frame->height,		//����߶�
			data,				//������ݵ�ַ
			lines				//����п��
		);
		if (re != out_h) {
			cout << "[VIDEO CONVERT] the height of the output is: " << re << ", the needed height is " << out_h << endl;
			mutex.unlock();
			return false;
		}			
	}
	mutex.unlock();
	return true;
}

int XFFmpeg::audio_convert(uint8_t* const out) {
	uint8_t* dataOut[1];
	int re = 0;

	mutex.lock();
	if (!ic || !ac || !out || !pcm) {
		mutex.unlock();
		cout << "[AUDIO CONVERT] NULL PTR!" << endl;
		return 0;
	}
	
	if (!aSwrCtx) {
		aSwrCtx = swr_alloc();
		if (NULL == swr_alloc_set_opts(aSwrCtx,
			ac->channel_layout,  // �ز������������ģʽ���䣬�õľ������������ģʽ������ģʽ��������Ϊ�Ǳ��絥������˫��������2.1������5.1����֮��ģ��ȵ�
			AV_SAMPLE_FMT_S16,   // �ز�������̶���16λ
			ac->sample_rate,	 // �ز�������Ĳ��������ﲻ�仯
			ac->channel_layout,	 // ���������ģʽ
			ac->sample_fmt,		 // �����������ʽ
			ac->sample_rate,	 // ����Ĳ�����
			0,
			NULL)) 
		{
			mutex.unlock();
			cout << "[AUDIO CONVERT] swr_alloc_set_opts failed!" << endl;
			return 0;
		}
		swr_init(aSwrCtx);
	}
	dataOut[0] = out;
	re = swr_convert(aSwrCtx, dataOut, MAXAUDIOSWRLEN, (const uint8_t**)pcm->data, pcm->nb_samples); // ����������ֻҪ��֤����һ֡��Ƶ���ݵ��ز��������С���ɣ�һ�㲻�ᳬ��10000
	if (re <= 0) {
		mutex.unlock();
		cout << "[AUDIO CONVERT] swr_convert error, " << get_error(re) << endl;
		return re;
	}
	// Get the required buffer size for the given audio parameters(�ز��������������Ҫ�Ŀռ��С).
	re = av_samples_get_buffer_size(NULL, ac->channels, pcm->nb_samples, AV_SAMPLE_FMT_S16, 0);
	mutex.unlock();
	return re;
}

void XFFmpeg::compute_current_pts(AVFrame* frame, int streamId) {
	double rate = 0.0;
	int* temp = NULL;

	if (videoStream == streamId) { temp = &currentVPtsMs; }
	else if (audioStream == streamId) { temp = &currentAPtsMs; }
	else { return; }

	if (frame == NULL)
	{
		cout << "[CURRENT PTS] NULL PTR! StreamId: " << streamId << endl;
		*temp = 0;
		return;
	}
	mutex.lock();
	if (ic) {
		//time base  �ҵ������һ����λ��ռ�õ�������������Է��ֲ�������48000Hz����Ƶ����
		//time base num/den�ֱ���1/48000��Ҳ����һ��������ռ��ʱ��1/48000s
		//��pts���ҵ�����Ǹ�ʱ���ۻ��ĵ�λ�������ʱ�̵��������� pts * timebase
		if (ic->streams[streamId]->time_base.den == 0) {
			rate = 0.0;
		}
		else {
			rate = (double)ic->streams[streamId]->time_base.num / (double)ic->streams[streamId]->time_base.den;
		}
	}
	else {
		rate = 0.0;
	}
	/**
	 *pts: Presentation timestamp in time_base units (time when frame should be shown to user).
	 */
	//*temp = frame->pts * rate * 1000;  // ��ʱ������pts��AV_NOPTS_VALUE�������������best_effort_timestamp
	/**
	 * frame timestamp estimated using various heuristics, in stream time base
	 * - encoding: unused
	 * - decoding: set by libavcodec, read by user.
	 */
	*temp = frame->best_effort_timestamp * rate * 1000;
	mutex.unlock();
	if (videoStream == streamId) {
		//cout << "[CURRENT PTS] V ------ best_effort_timestamp: " << frame->best_effort_timestamp << endl;
		//cout << "[CURRENT PTS] V frame->pts/temp: " << frame->pts << "/" << *temp << endl;
	}
	else if (audioStream == streamId) {
		//cout << "[CURRENT PTS] A ------ best_effort_timestamp: " << frame->best_effort_timestamp << endl;
		//cout << "[CURRENT PTS] A frame->pts/temp: " << frame->pts << "/" << *temp << endl;
	}
	else {
	
	}
	return;
}

void XFFmpeg::compute_duration_ms() {
	if (ic) {
		if (videoStream >= 0) {
			if (ic->streams[videoStream]->duration == AV_NOPTS_VALUE) {
				totalVms = ic->duration / (AV_TIME_BASE / 1000);  // AV_TIME_BASE��ʾ1����AV_TIME_BASE����λ
			}
			else {
				totalVms = 1000 * ic->streams[videoStream]->duration * ic->streams[videoStream]->time_base.num / ic->streams[videoStream]->time_base.den;
			}
		}
		if (audioStream >= 0) {
			if (ic->streams[audioStream]->duration == AV_NOPTS_VALUE) {
				totalAms = ic->duration / (AV_TIME_BASE / 1000);  // AV_TIME_BASE��ʾ1����AV_TIME_BASE����λ
			}
			else {
				totalAms = 1000 * ic->streams[audioStream]->duration * ic->streams[audioStream]->time_base.num / ic->streams[audioStream]->time_base.den;
			}
		}
	}
	else {
		totalVms = 0;
		totalAms = 0;
	}
	cout << "[DURATION] There ara " << ic->nb_streams << " streams: " << endl;
	cout << "[DURATION] TotalVms: " << totalVms << " ms" << endl;
	cout << "[DURATION] TotalAms: " << totalAms << " ms" << endl;
	if (videoStream >= 0)cout << "[DURATION] In stream, Vduration/num/den: " << ic->streams[videoStream]->duration << "/" << ic->streams[videoStream]->time_base.num << "/" << ic->streams[videoStream]->time_base.den << endl;
	if (audioStream >= 0)cout << "[DURATION] In stream, Aduration/num/den: " << ic->streams[audioStream]->duration << "/" << ic->streams[audioStream]->time_base.num << "/" << ic->streams[audioStream]->time_base.den << endl;
	cout << "[DURATION] In ic,	   duration " << ic->duration << endl; 

	return;
}

void XFFmpeg::compute_video_fps() {
	if (ic) {
		if (ic->streams[videoStream]->avg_frame_rate.den == 0) {
			fps = 0; 
		}
		else {
			fps = ic->streams[videoStream]->avg_frame_rate.num / ic->streams[videoStream]->avg_frame_rate.den;
		}
	}
	else {
		fps = 0;
	}
	cout << "[FPS] " << fps << " fps" << endl;
	return;
}

bool XFFmpeg::seek(float pos) {
	int64_t stamp = 0;
	int re = 0;

	mutex.lock();
	if (!ic) {
		mutex.unlock();
		return false;
	}
	/*
	 * The set of streams, the detected duration, stream parameters and codecs do
	 * not change when calling this function. If you want a complete reset, it's
	 * better to open a new AVFormatContext.
	 */
	avformat_flush(ic);
	stamp = (float)ic->streams[videoStream]->duration * pos;	
	re = av_seek_frame(ic, videoStream, stamp, AVSEEK_FLAG_FRAME|AVSEEK_FLAG_BACKWARD);
	//avcodec_flush_buffers(ic->streams[videoStream]->codec);  // codec�Ƿ������ԣ�ʹ�û���������
	/*
		���sliderRelease����û�м�XDistributeThread::get()->lock()�������ͬʱ���������д�����ע�͵��ˣ�
		��������ţ������л���������Ϊ�����������pkt������������ñ������ڽ�����buffer�е�֡�ֲ��ˣ�
		���Բ��Ứ�������ǻ�����ʱ���������������ȷ�ķ�ʽӦ������sliderRelease�����XDistributeThread::get()->lock()
		�������ͬʱ�����������д��롣
	*/
	avcodec_flush_buffers(vc);  
	
	if (re >= 0)
	{
		currentVPtsMs = totalVms * pos;  // �ȶ��϶������Ƶpts���и���һ�£���ֹslider bar����
		mutex.unlock();
		cout << "[SEEK] vPts after seek: " << currentVPtsMs << endl;
		return true;
	}
	else
	{
		mutex.unlock();
		cout << "[SEEK] av_seek_frame error: " << get_error(re) << endl;
		return false;
	}	
}

bool XFFmpeg::create_decoder(AVFormatContext* ic) {
	int re = 0;

	if (!ic) {
		cout << "[CREATE DECODER] Please open first!" << endl;
		return false;
	}
	cout << "[CREATE DECODER] Total stream numbers: " << ic->nb_streams << endl;
#if 0
	//��ȡ����Ƶ����Ϣ��������������ȡ��
	for (int i = 0; i < ic->nb_streams; i++) {
		AVStream* as = ic->streams[i];

		if (as->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
			audioStream = i;
			cout << i << " ��Ƶ��Ϣ" << endl;
			cout << "  - sample rate: " << as->codecpar->sample_rate << endl;
			cout << "  - channels: " << as->codecpar->channels << endl;
			cout << "  - format: " << as->codecpar->format << endl;  // ��Ƶ AVSampleFormat ö�����ͣ�����unsigned 8 bits planar��signed 16 bits planar��float planar�ȵ�
			cout << "  - codec_id: " << as->codecpar->codec_id << endl;  // ����AV_CODEC_ID_AAC��AV_CODEC_ID_MP3�ȵ�
			cout << "  - audio frame size: " << as->codecpar->frame_size << endl;  //һ֡����  ��ͨ��������������Ϊ1024��
			//�����˫ͨ����as->codecpar->channels == 2������һ֡����frame size��as->codecpar->frame_size * 2 * byte per sample = 1024*2*2(����Ϊ16λ)=4096�ֽ�
			//����Ƶ֡�� fps=as->codecpar->sample_rate*2/frame size = 48000*2/4096 = 23.4375������Ƶ֡�ʲ�һ�����
		}
		else if (as->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoStream = i;
			cout << i << " ��Ƶ��Ϣ" << endl;
			cout << "  - width: " << as->codecpar->width << endl;
			cout << "  - height: " << as->codecpar->height << endl;
			cout << "  - format: " << as->codecpar->format << endl;  // ��Ƶ AVPixelFormat ö������
			cout << "  - codec_id: " << as->codecpar->codec_id << endl;  // ����AV_CODEC_ID_H264�ȵ�
			cout << "  - video frame size: " << as->codecpar->frame_size << endl;  // ��Ƶ���ֵΪ0
		}
	}
#else
	//������ȡvideoStream��audioStream��Ȼ����뵽ic->streams[i]�л�ȡ����������Ϣ���Ͳ��ñ�����
	videoStream = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);  // �����ڶ���������NULL����Ϊ��װ�ͽ�����뿪����ϣ�
	audioStream = av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
#endif

	cout << "[CREATE DECODER] videoStream = " << videoStream << endl;
	cout << "[CREATE DECODER] audioStream = " << audioStream << endl;

	if (videoStream >= 0) {
		/************************************ ��Ƶ���������� ************************************/
		//�ҵ���Ƶ������
		AVCodec* vcodec = avcodec_find_decoder(ic->streams[videoStream]->codecpar->codec_id);
		if (!vcodec) {
			cout << "[CREATE DECODER] Can't find the video codec id " << ic->streams[videoStream]->codecpar->codec_id << endl;
			return false;
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
			avcodec_free_context(&vc);
			cout << "[CREATE DECODER] avcodec_open2 failed: " << get_error(re) << endl;
			return false;
		}
		else {
			cout << "[CREATE DECODER] video - avcodec_open2 success!" << endl;
		}
	}

	if (audioStream >= 0) {
		/************************************ ��Ƶ���������� ************************************/
		//�ҵ���Ƶ������
		AVCodec* acodec = avcodec_find_decoder(ic->streams[audioStream]->codecpar->codec_id);
		if (!acodec) {
			cout << "[CREATE DECODER] Can't find the audio codec id " << ic->streams[audioStream]->codecpar->codec_id << endl;
			return false;
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
			avcodec_free_context(&ac);
			cout << "[CREATE DECODER] avcodec_open2 failed: " << get_error(re) << endl;
			return false;
		}
		else {
			cout << "[CREATE DECODER] audio - avcodec_open2 success!" << endl;
			sampleRate = ac->sample_rate;
			channel = ac->channels;
			switch (ac->sample_fmt) {
			case AV_SAMPLE_FMT_S16:
				sampleSize = 16;
				break;
			case AV_SAMPLE_FMT_S32:
			case AV_SAMPLE_FMT_FLTP:
				sampleSize = 32;
				break;
			default:
				cout << "[CREATE DECODER] Error sample fmt " << ac->sample_fmt << endl;
				return false;
			}
			cout << "[CREATE DECODER] sampleRate/channel/sampleSize " << sampleRate << "/" << channel << "/" << sampleSize << endl;
		}
	}
	return true;
}

/*
	���pkt�ǿյģ����ص�ǰ�Ѿ������������Ƶ֡��pts�����򷵻�pkt����ָ��Ƶ֡��pts
*/
int XFFmpeg::get_current_video_pts(AVPacket* pkt) {
	mutex.lock();
	int pts = 0;
	double rate = 0.0;

	if (NULL == pkt) {
		pts = currentVPtsMs;
	}
	else {		
		if (!ic)
		{
			mutex.unlock();
			return -1;
		}
		if (ic->streams[videoStream]->time_base.den == 0) {
			rate = 0.0;
		}
		else {
			rate = (double)ic->streams[videoStream]->time_base.num / (double)ic->streams[videoStream]->time_base.den;
		}
		pts = pkt->pts * rate * 1000;
	}
	mutex.unlock();
	return pts;
}

int XFFmpeg::get_current_audio_pts() {
	return currentAPtsMs;
}

int XFFmpeg::get_duration_ms(int streamId) {
	return (streamId == videoStream ? totalVms : totalAms);
}

int XFFmpeg::get_video_fps() {
	return fps;
}

bool XFFmpeg::send_flush_packet() {
	return bSendFlushPacket;
}

void XFFmpeg::set_send_flush_packet(bool flag) {
	bSendFlushPacket = flag;
}

void XFFmpeg::clean() {
	totalAms = 0;
	totalVms = 0;
	currentAPtsMs = 0;
	currentVPtsMs = 0;
	fps = 0;
	bSendFlushPacket = false;
	videoStream = -1;
	audioStream = -1;
	error_buf[0] = '\0';
}

void XFFmpeg::close() {
	mutex.lock();
	if (ic) avformat_close_input(&ic);
	if (vc) { 
		avcodec_close(vc);
		avcodec_free_context(&vc); 
	}
	if (ac) { 
		avcodec_close(ac);
		avcodec_free_context(&ac);
	}
	if (vSwsCtx) { 
		sws_freeContext(vSwsCtx); 
		vSwsCtx = NULL;
	}
	if (aSwrCtx) { swr_free(&aSwrCtx); }
	if (packet) av_packet_free(&packet);
	if (yuv) av_frame_free(&yuv);
	if (pcm) av_frame_free(&pcm);
	clean();
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
	error_buf[0] = '\0';
	//��ʼ����װ�⣨��ʵ���°汾��������ѱ����������û����������Ϊ�ѷ���Ĵ���
	av_register_all();
	//��ʼ������⣬���Դ�rtsp��rtmp��http��Э�����ý����Ƶ
	avformat_network_init();
}

XFFmpeg::~XFFmpeg() {

}