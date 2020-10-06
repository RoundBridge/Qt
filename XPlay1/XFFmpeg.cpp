#include "XFFmpeg.h"
#include "XVideoThread.h"
#include <iostream>
using std::cout;
using std::endl;

#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "swscale.lib")
#pragma comment(lib, "swresample.lib")

int XFFmpeg::open(const char* path) {	
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
	// 打开成功的情况下，把关于这个文件的相关信息都更新好，便于外部使用
	if (0 == re) {		
		re = create_decoder(ic);		
		compute_duration_ms();
		compute_video_fps();
	}
	mutex.unlock();

	return re;
}

AVPacket* XFFmpeg::read() {
	int re = 0;
	// read函数里面不负责对packet的释放操作，需要外部释放
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
		cout << "[PACKET] read frame error(" << re << "): " << get_error(re) << endl;
		if (re == -541478725) {  // -541478725返回值是测试出来的表示读到文件末尾
			set_send_flush_packet(true);
		}
		return NULL;
	}
	else {
		if (packet->stream_index == videoStream) {
			if (packet->flags & AV_PKT_FLAG_KEY) {
				//cout << "[PACKET] --- read KEY packet! ---" << endl;
			}
			// 有些视频读出来的packet中的pts是未定义的
			if (packet->pts == AV_NOPTS_VALUE) {  // AV_NOPTS_VALUE ((int64_t)UINT64_C(0x8000000000000000))(-9223372036854775808)
				//cout << "[PACKET] packet pts undefined timestamp value: " << AV_NOPTS_VALUE << endl;
			}
			//cout << "[PACKET] packet pts " << packet->pts << endl;  // AV_NOPTS_VALUE

		}
	}
	mutex.unlock();
	return packet;
}

bool XFFmpeg::decode(const AVPacket* pkt) {
	AVCodecContext* cc = NULL;
	AVFrame* frame = NULL;

	if (yuv == NULL) {
		yuv = av_frame_alloc();
	}
	if (pcm == NULL) {
		pcm = av_frame_alloc();
	}

	if (pkt->stream_index == videoStream) { cc = vc; frame = yuv; }
	else if (pkt->stream_index == audioStream) { cc = ac; frame = pcm; }
	else { cout << "[DECODE] stream index wrong: " << pkt->stream_index << endl; return false; }

	mutex.lock();
	// 如果是视频包，pkt中的stream_index直接指明视频流的idx，音频包同理
	// 发送packet到解码线程，只是将pkt扔给解码线程，几乎不占用CPU
	// send NULL后调用多次receive可以取出所有缓冲帧
	int re = avcodec_send_packet(cc, pkt);
	if (0 != re) {
		mutex.unlock();
		cout << "[DECODE] avcodec_send_packet failed: " << get_error(re) << endl;
		return false;
	}
	// 只是从解码线程中获取解码结果，几乎不占用CPU，一次send（一次可能发送多个音频帧）可能对应多次receive
	// 为什么这里只获取一次？难道不应该多获取几次，把里面的帧全部获取出来？
	// 下面avcodec_receive_frame的第一个参数不能用ic->streams[pkt->stream_index]->codec了，因为它已经是
	// 废弃属性了，用它的话，函数调用失败
	re = avcodec_receive_frame(cc, frame);
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
		if (AVERROR_EOF == re) {
			XVideoThread::isStart = false;
			XVideoThread::isExit = true;
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
		vSwsCtx,  //传NULL会新创建，否则比较该上下文和下面这些参数将要创建出来的上下文是否一致，
				  //若一致则直接返回该上下文，不一致则创建新的上下文
		frame->width, frame->height,	//输入的宽高
		(AVPixelFormat)frame->format,	//输入的格式，YUV420p
		out_w, out_h,					//输出的宽高
		out_pixfmt,						//输出格式
		SWS_BILINEAR,					//尺寸转换的算法
		0, 0, 0
	);
	//cout << "[CONVERT] the w/h of the input is: " << frame->width << "/" << frame->height << endl;
	if (vSwsCtx) {
		// sws_scale函数的开销很大
		re = sws_scale(			//返回值 the height of the output slice
			vSwsCtx,
			frame->data,		//输入数据地址
			frame->linesize,	//输入行跨度
			0,					//the position in the source image of the slice to *process, 
								//that is the number(counted starting from
								// zero) in the image of the first row of the slice
			frame->height,		//输入高度
			data,				//输出数据地址
			lines				//输出行跨度
		);
		//cout << "[CONVERT] the height of the output is: " << re << endl;		
	}
	mutex.unlock();
	return true;
}

int XFFmpeg::audio_convert(uint8_t* const out) {
	uint8_t** dataOut;
	int re = 0;

	if (!ic || !out || !pcm) {
		cout << "[AUDIO CONVERT] NULL PTR!" << endl;
		return 0;
	}
	mutex.lock();
	if (!aSwrCtx) {
		aSwrCtx = swr_alloc();
		if (NULL == swr_alloc_set_opts(aSwrCtx,
			ac->channel_layout,  // 重采样输出的声道模式不变，用的就是输入的声道模式。声道模式，可以认为是比如单声道、双声道或者2.1声道、5.1声道之类的，等等
			AV_SAMPLE_FMT_S16,   // 重采样输出固定成16位
			ac->sample_rate,	 // 重采样输出的采样率这里不变化
			ac->channel_layout,	 // 输入的声道模式
			ac->sample_fmt,		 // 输入的样本格式
			ac->sample_rate,	 // 输入的采样率
			0,
			NULL)) 
		{
			mutex.unlock();
			cout << "[AUDIO CONVERT] swr_alloc_set_opts failed!" << endl;
			return 0;
		}
		swr_init(aSwrCtx);
	}
	*dataOut = out;
	re = swr_convert(aSwrCtx, dataOut, 10000, (const uint8_t**)pcm->data, pcm->nb_samples); // 第三个参数只要保证大于一帧音频数据的重采样输出大小即可，一般不会超过10000
	if (re <= 0) {
		mutex.unlock();
		cout << "[AUDIO CONVERT] swr_convert error, " << get_error(re) << endl;
		return re;
	}
	// Get the required buffer size for the given audio parameters(重采样后输出数据需要的空间大小).
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
	
	if (ic) {
		//time base  我的理解是一个单位所占用的秒数，比如测试发现采样率是48000Hz的音频，其
		//time base num/den分别是1/48000，也就是一个采样点占用时间1/48000s
		//而pts，我的理解是该时刻累积的单位数，则该时刻的秒数就是 pts * timebase
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
	mutex.lock();
	/**
	 *pts: Presentation timestamp in time_base units (time when frame should be shown to user).
	 */
	//*temp = frame->pts * rate * 1000;  // 有时读到的pts是AV_NOPTS_VALUE，所以这里采用best_effort_timestamp
	/**
	 * frame timestamp estimated using various heuristics, in stream time base
	 * - encoding: unused
	 * - decoding: set by libavcodec, read by user.
	 */
	*temp = frame->best_effort_timestamp * rate * 1000;
	mutex.unlock();
	//if (videoStream == streamId) {
	//	cout << "[CURRENT PTS] ------ best_effort_timestamp: " << frame->best_effort_timestamp << endl;
	//	cout << "[CURRENT PTS] frame->pts/temp: " << frame->pts << "/" << *temp << endl;
	//}
	return;
}

void XFFmpeg::compute_duration_ms() {
	if (ic) {
		if (ic->streams[videoStream]->duration == AV_NOPTS_VALUE) {
			totalVms = ic->duration / (AV_TIME_BASE / 1000);  // AV_TIME_BASE表示1秒有AV_TIME_BASE个单位
		}
		else {
			totalVms = 1000 * ic->streams[videoStream]->duration * ic->streams[videoStream]->time_base.num / ic->streams[videoStream]->time_base.den;
		}
		// todo: 后续需加入对音频的支持
	}
	else {
		totalVms = 0;
		totalAms = 0;
	}
	cout << "[DURATION] In stream, duration/num/den: " << ic->streams[videoStream]->duration << "/" << ic->streams[videoStream]->time_base.num << "/" << ic->streams[videoStream]->time_base.den << endl;
	cout << "[DURATION] In ic,	   duration " << ic->duration << endl; 
	cout << "[DURATION] totalVms: " << totalVms << " ms" << endl;
	cout << "[DURATION] totalAms: " << totalAms << " ms" << endl;
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
	
	if (!ic) {
		return false;
	}
	mutex.lock();
	stamp = (float)ic->streams[videoStream]->duration * pos;	
	re = av_seek_frame(ic, videoStream, stamp, AVSEEK_FLAG_FRAME|AVSEEK_FLAG_BACKWARD);
	//avcodec_flush_buffers(ic->streams[videoStream]->codec);  // codec是废弃属性，使用会引发错误
	avcodec_flush_buffers(vc);
	mutex.unlock();
	if (re >= 0)
	{
		currentVPtsMs = totalVms * pos;  // 先对拖动后的视频pts进行更新一下，防止slider bar回跳
		return true;
	}
	else
	{
		cout << "[SEEK] av_seek_frame error: " << get_error(re) << endl;
		return false;
	}	
}

bool XFFmpeg::create_decoder(AVFormatContext* ic) {
	int re = 0;

	if (!ic) {
		cout << "[CREATE DECODER] Please open first!" << endl;
		return -5;
	}

	//函数获取videoStream和audioStream，然后代入到ic->streams[i]中获取各个流的信息，就不用遍历了
	videoStream = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);  // 倒数第二个参数传NULL是因为封装和解码隔离开不耦合？
	audioStream = av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
	cout << "[CREATE DECODER] videoStream = " << videoStream << endl;
	cout << "[CREATE DECODER] audioStream = " << audioStream << endl;

	/************************************ 视频解码器部分 ************************************/
	//找到视频解码器
	AVCodec* vcodec = avcodec_find_decoder(ic->streams[videoStream]->codecpar->codec_id);
	if (!vcodec) {
		cout << "[CREATE DECODER] Can't find the video codec id " << ic->streams[videoStream]->codecpar->codec_id << endl;
		return -1;
	}
	else {
		cout << "[CREATE DECODER] Find the vcodec " << ic->streams[videoStream]->codecpar->codec_id << endl;
	}

	//创建解码器上下文
	if (vc == NULL) { vc = avcodec_alloc_context3(vcodec); }	

	//配置解码器上下文参数
	//新版本已经将AVStream结构体中的AVCodecContext字段定义为废弃属性。
	//因此无法像旧的版本直接通过AVFormatContext获取到AVCodecContext结构体参数
	//当前版本保存视音频流信息的结构体AVCodecParameters，FFmpeg提供了函数avcodec_parameters_to_context
	//将音视频流信息拷贝到新的AVCodecContext结构体中
	avcodec_parameters_to_context(vc, ic->streams[videoStream]->codecpar);

	//4线程解码
	//也可以先通过linux或者Windows API来获取CPU个数进而决定解码线程数
	vc->thread_count = 4;

	//打开解码器上下文
	re = avcodec_open2(vc, 0, 0);
	if (0 != re) {
		cout << "[CREATE DECODER] avcodec_open2 failed: " << get_error(re) << endl;
		return -2;
	}
	else {
		cout << "[CREATE DECODER] video - avcodec_open2 success!" << endl;
	}

	/************************************ 音频解码器部分 ************************************/
	//找到音频解码器
	AVCodec* acodec = avcodec_find_decoder(ic->streams[audioStream]->codecpar->codec_id);
	if (!acodec) {
		cout << "[CREATE DECODER] Can't find the audio codec id " << ic->streams[audioStream]->codecpar->codec_id << endl;
		return -3;
	}
	else {
		cout << "[CREATE DECODER] Find the acodec " << ic->streams[audioStream]->codecpar->codec_id << endl;
	}

	//创建解码器上下文
	if (ac == NULL) { ac = avcodec_alloc_context3(acodec); }

	//配置解码器上下文参数
	avcodec_parameters_to_context(ac, ic->streams[audioStream]->codecpar);

	//4线程解码
	ac->thread_count = 4;

	//打开解码器上下文
	/*
		对于avcodec_open2函数的第二个参数，If a non-NULL codec has been previously passed to 
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
				return -5;
		}
		cout << "[CREATE DECODER] sampleRate/channel/sampleSize " << sampleRate << "/" << channel << "/" << sampleSize << endl;
	}

	return 0;
}

int XFFmpeg::get_current_video_pts() {
	return currentVPtsMs;
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
	videoStream = 0;
	audioStream = 0;
	error_buf[0] = '\0';
}

void XFFmpeg::close() {
	mutex.lock();
	if (ic) avformat_close_input(&ic);
	if (vc) avcodec_free_context(&vc);
	if (ac) avcodec_free_context(&ac);
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
	std::string re = this->error_buf;  // 把一个char*赋值给string对象时会复制一份，用一个复制的空间避免多线程访问出现异常
	mutex.unlock();
	return re;
}

XFFmpeg::XFFmpeg() {
	//错误信息缓存初始化
	error_buf[0] = '\0';
	//初始化封装库（其实最新版本这个函数已被废弃，调用会产生被声明为已否决的错误）
	av_register_all();
	//初始化网络库，可以打开rtsp、rtmp、http等协议的流媒体视频
	avformat_network_init();
}

XFFmpeg::~XFFmpeg() {

}