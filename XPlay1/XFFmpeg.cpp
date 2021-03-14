#include "XFFmpeg.h"
#include "XVideoThread.h"
#include "XAudioThread.h"
#include <iostream>
#include <ctime>
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
	if (0 <= re) {	// 0表示成功，负数表示失败，但是有时会返回正数，所以这里暂且用判断 0<=re
		re = create_decoder(ic);
		init_yuv_pool();
		compute_duration_ms();
		compute_video_fps();
	}
	mutex.unlock();

	return re; // 暂且将re>=0表示成功
}

AVPacket* XFFmpeg::read() {
	int re = 0;

	mutex.lock();
	// read函数里面不负责对packet的释放操作，需要外部释放
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
		if (re == -541478725) {  // -541478725返回值是测试出来的表示读到文件末尾
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
			// 有些视频读出来的packet中的pts是未定义的
			if (packet->pts == AV_NOPTS_VALUE) {  // AV_NOPTS_VALUE ((int64_t)UINT64_C(0x8000000000000000))(-9223372036854775808)
				//cout << "[PACKET] V packet pts undefined timestamp value: " << AV_NOPTS_VALUE << endl;
			}
			// AVPacket的dts是解码顺序时间戳，也即编码顺序时间戳，如果存在B帧，就会先编码后面一帧，然后再编码前面一帧，导致显示时间戳pts出现回跳现象。
			// ffprobe -show_frames 出来的结果会按照显示时间戳从小到大排列，但是它的coded_picture_number则会回跳。
			else {
				//cout << "[PACKET] V packet display pts " << packet->pts << ", decode pts " << packet->dts << endl;  // AV_NOPTS_VALUE
			}
		}
		else if (packet->stream_index == audioStream) {
			if (packet->flags & AV_PKT_FLAG_KEY) {
				//cout << "[PACKET] A --- read KEY packet! ---" << endl;
			}
			// 有些音频读出来的packet中的pts是未定义的
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
	bool isAPkt = false;

	if (!pkt || pkt->size <= 0 || !pkt->data) {
		return false;
	}

	mutex.lock();
	if (pcm == NULL) {
		pcm = av_frame_alloc();
	}

	if (pkt->stream_index == videoStream) {
		cc = vc;
		if (nullptr == (frame = get_yuv_frame())) {
			mutex.unlock();
			//cout << "[DECODE] get_yuv_frame failed!" << endl;
			return false;
		}
		else
		{
			//cout << "[DECODE] get_yuv_frame OK!" << endl;
		}
	}
	else if (pkt->stream_index == audioStream) {
		cc = ac;
		frame = pcm;
		isAPkt = true;
	}
	else {
		mutex.unlock();
		cout << "[DECODE] stream index wrong: " << pkt->stream_index << endl;
		return false;
	}

	// 如果是视频包，pkt中的stream_index直接指明视频流的idx，音频包同理
	// 发送packet到解码线程，只是将pkt扔给解码线程，几乎不占用CPU
	// send NULL后调用多次receive可以取出所有缓冲帧
	int re = avcodec_send_packet(cc, pkt);
	if (0 != re) {
		mutex.unlock();
		release_yuv_frame(frame);
		cout << "[DECODE] avcodec_send_packet failed: " << get_error(re) << endl;
		return false;
	}
	// 只是从解码线程中获取解码结果，几乎不占用CPU，一次send（一次可能发送多个音频帧）可能对应多次receive
	// 为什么这里只获取一次？难道不应该多获取几次，把里面的帧全部获取出来？
	// 下面avcodec_receive_frame的第一个参数不能用ic->streams[pkt->stream_index]->codec了，因为它已经是
	// 废弃属性了，用它的话，函数调用失败
	re = avcodec_receive_frame(cc, frame);
	if (re == 0) {
		mutex.unlock();
	}
	else if (re == AVERROR(EAGAIN)) { // output is not available in this state, user must try to send new input
		mutex.unlock();
		release_yuv_frame(frame);
		cout << "[DECODE] avcodec_receive_frame EAGAIN: " << get_error(re) << endl;
		return true;
	}
	else {
		mutex.unlock();
		release_yuv_frame(frame);
		cout << "[DECODE] avcodec_receive_frame failed: " << get_error(re) << endl;
		return false;
	}

	if (isAPkt) //视频当前的pts等到convert显示时再计算
		compute_current_pts(frame, pkt->stream_index);

	return true;
}

AVFrame* XFFmpeg::get_buffered_frames() {
	int re = 0;
	AVFrame* frame = NULL;
	static bool flag = true;
	static int bufferedFramesCnt = 0;

	if (nullptr == (frame = get_yuv_frame())) {
		cout << "[DECODE] get_yuv_frame failed!" << endl;
		return frame;
	}

	mutex.lock();
	if (flag) {
		avcodec_send_packet(vc, NULL);
		flag = false;
	}
	re = avcodec_receive_frame(vc, frame);
	if (0 != re) {
		mutex.unlock();
		release_yuv_frame(frame);
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
	//compute_current_pts(frame, videoStream);  //视频当前的pts等到convert显示时再计算
	bufferedFramesCnt++;
	return frame;
}

bool XFFmpeg::video_convert(uint8_t* const out, int out_w, int out_h, AVPixelFormat out_pixfmt) {
	int re = 0;
	uint8_t* data[AV_NUM_DATA_POINTERS] = { 0 };
	int lines[AV_NUM_DATA_POINTERS] = { 0 };
	AVFrame* frame = nullptr;

	frame = get_yuv_frame_decode();
	if (frame == nullptr ||
		frame->width == 0 ||
		frame->height == 0) {
		release_yuv_frame(frame);
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
			0,					//the position in the source image of the slice to process,
								//that is the number(counted starting from
								// zero) in the image of the first row of the slice
			frame->height,		//输入高度
			data,				//输出数据地址
			lines				//输出行跨度
		);
		if (re != out_h) {
			cout << "[VIDEO CONVERT] the height of sws_scale output is: " << re << ", the needed height is " << out_h << endl;
			mutex.unlock();
			return false;
		}
		// 这次转换没成功，下次再做一遍
		if (!release_yuv_frame(frame)) {
			cout << "[VIDEO CONVERT] release_yuv_frame_buffer failed!" << endl;
		}
		mutex.unlock();
		return true;
	}
	else {
		cout << "[VIDEO CONVERT] NO vSwsCtx available!" << endl;
		mutex.unlock();
		return false;
	}
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
	dataOut[0] = out;
	re = swr_convert(aSwrCtx, dataOut, MAXAUDIOSWRLEN, (const uint8_t**)pcm->data, pcm->nb_samples); // 第三个参数只要保证大于一帧音频数据的重采样输出大小即可，一般不会超过10000
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
    int64_t stamp = -1;
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
    /**
    *pts: Presentation timestamp in time_base units (time when frame should be shown to user).
    */
    /**
    * frame timestamp estimated using various heuristics, in stream time base
    * - encoding: unused
    * - decoding: set by libavcodec, read by user.
    */
    // 有时best_effort_timestamp和frame->pts读到的值是AV_NOPTS_VALUE，所以这里判断一下

    if (frame->best_effort_timestamp != AV_NOPTS_VALUE) {
        stamp = frame->best_effort_timestamp;
    }
    else {
        if (frame->pts != AV_NOPTS_VALUE) {
            stamp = frame->pts;
        }
        else {
            // 注意这个条件判断是进得来的，这说明有个无数据的包（测试时发现音频经常有这种情况，难道是静音包！？）
			if (!frame->buf[0]) {
				cout << "[CURRENT PTS] AV_NOPTS_VALUE timestamp!!! With no ref buf!!! " << endl;
			}
            mutex.unlock();
            return;
        }
    }

	*temp = stamp * rate * 1000;
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
				totalVms = ic->duration / (AV_TIME_BASE / 1000);  // AV_TIME_BASE表示1秒有AV_TIME_BASE个单位
			}
			else {
				totalVms = 1000 * ic->streams[videoStream]->duration * ic->streams[videoStream]->time_base.num / ic->streams[videoStream]->time_base.den;
			}
		}
		if (audioStream >= 0) {
			if (ic->streams[audioStream]->duration == AV_NOPTS_VALUE) {
				totalAms = ic->duration / (AV_TIME_BASE / 1000);  // AV_TIME_BASE表示1秒有AV_TIME_BASE个单位
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
			finterval = -1.0;
		}
		else {
			fps = ic->streams[videoStream]->avg_frame_rate.num / ic->streams[videoStream]->avg_frame_rate.den;
			finterval = 1000 * 1.0 / fps;
		}
	}
	else {
		fps = 0;
		finterval = -1.0;
	}
	cout << "[FPS] " << fps << " fps(" << finterval << ")" << endl;
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
	re = av_seek_frame(ic, videoStream, stamp, AVSEEK_FLAG_FRAME | AVSEEK_FLAG_BACKWARD);
	//avcodec_flush_buffers(ic->streams[videoStream]->codec);  // codec是废弃属性，使用会引发错误
	/*
		如果sliderRelease里面没有加XDistributeThread::get()->lock()这把锁，同时呢下面这行代码又注释掉了，
		则歪打正着，不会有花屏现象，因为队列中清掉的pkt所起的作用正好被缓存在解码器buffer中的帧弥补了，
		所以不会花屏，但是会引起时间戳回跳，所以正确的方式应该是在sliderRelease里面加XDistributeThread::get()->lock()
		这把锁，同时运行下面这行代码。
	*/
	avcodec_flush_buffers(vc);

	if (re >= 0)
	{
		currentVPtsMs = totalVms * pos;  // 先对拖动后的视频pts进行更新一下，防止slider bar回跳
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
	//获取音视频流信息（遍历、函数获取）
	for (int i = 0; i < ic->nb_streams; i++) {
		AVStream* as = ic->streams[i];

		if (as->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
			audioStream = i;
			cout << i << " 音频信息" << endl;
			cout << "  - sample rate: " << as->codecpar->sample_rate << endl;
			cout << "  - channels: " << as->codecpar->channels << endl;
			cout << "  - format: " << as->codecpar->format << endl;  // 音频 AVSampleFormat 枚举类型，比如unsigned 8 bits planar，signed 16 bits planar，float planar等等
			cout << "  - codec_id: " << as->codecpar->codec_id << endl;  // 比如AV_CODEC_ID_AAC，AV_CODEC_ID_MP3等等
			cout << "  - audio frame size: " << as->codecpar->frame_size << endl;  //一帧数据  单通道样本数（假设为1024）
			//如果是双通道（as->codecpar->channels == 2），则一帧数据frame size是as->codecpar->frame_size * 2 * byte per sample = 1024*2*2(假设为16位)=4096字节
			//则音频帧率 fps=as->codecpar->sample_rate*2/frame size = 48000*2/4096 = 23.4375，跟视频帧率不一定相等
		}
		else if (as->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoStream = i;
			cout << i << " 视频信息" << endl;
			cout << "  - width: " << as->codecpar->width << endl;
			cout << "  - height: " << as->codecpar->height << endl;
			cout << "  - format: " << as->codecpar->format << endl;  // 视频 AVPixelFormat 枚举类型
			cout << "  - codec_id: " << as->codecpar->codec_id << endl;  // 比如AV_CODEC_ID_H264等等
			cout << "  - video frame size: " << as->codecpar->frame_size << endl;  // 视频这个值为0
		}
	}
#else
	//函数获取videoStream和audioStream，然后代入到ic->streams[i]中获取各个流的信息，就不用遍历了
	videoStream = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);  // 倒数第二个参数传NULL是因为封装和解码隔离开不耦合？
	audioStream = av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
#endif

	cout << "[CREATE DECODER] videoStream = " << videoStream << endl;
	cout << "[CREATE DECODER] audioStream = " << audioStream << endl;

	if (videoStream >= 0) {
		/************************************ 视频解码器部分 ************************************/
		//找到视频解码器
		AVCodec* vcodec = avcodec_find_decoder(ic->streams[videoStream]->codecpar->codec_id);
		if (!vcodec) {
			cout << "[CREATE DECODER] Can't find the video codec id " << ic->streams[videoStream]->codecpar->codec_id << endl;
			return false;
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
			avcodec_free_context(&vc);
			cout << "[CREATE DECODER] avcodec_open2 failed: " << get_error(re) << endl;
			return false;
		}
		else {
			cout << "[CREATE DECODER] video - avcodec_open2 success!" << endl;
		}
	}

	if (audioStream >= 0) {
		/************************************ 音频解码器部分 ************************************/
		//找到音频解码器
		AVCodec* acodec = avcodec_find_decoder(ic->streams[audioStream]->codecpar->codec_id);
		if (!acodec) {
			cout << "[CREATE DECODER] Can't find the audio codec id " << ic->streams[audioStream]->codecpar->codec_id << endl;
			return false;
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
	如果pkt是空的，返回当前已经解码的最新视频帧的pts，否则返回pkt包所指视频帧的pts
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
	finterval = -1.0;
	bSendFlushPacket = false;
	videoStream = -1;
	audioStream = -1;
	error_buf[0] = '\0';
}

/*
	初始化一个AVFrame对象的缓存池，这些对象用于存放解码后得到的YUV图像，
	池的大小是max_buffered_yuv_frame_num，0表示这个AVFrame对象    是空闲的，
	可以存放新进来的解码帧。
*/
bool XFFmpeg::init_yuv_pool() {
	for (int j = 0; j < max_buffered_yuv_frame_num; j++) {
		AVFrame* frame = av_frame_alloc();
		yuvPool[0][j] = 0;
		yuvPool[1][j] = (unsigned int)frame;
	}
	return true;
}

AVFrame* XFFmpeg::get_yuv_frame_decode() {
	static clock_t last = 0;
    int idx = -1;
	int64_t vPts = -1;
	int64_t aPts = get_current_audio_pts();
	AVFrame* ret = nullptr;
    clock_t cur = clock();

	get_minimum_frame_pts(&idx);

	if (idx == -1) {
		cout << "@@@ Get yuv frame to decode failed! aPts= " << aPts << endl;
		return nullptr;
	}
	ret = (AVFrame*)yuvPool[1][idx];

	compute_current_pts(ret, videoStream);  //计算视频当前最小的pts，也即最快要播放的那一帧的pts
	vPts = get_current_video_pts(NULL);
	cout << "!! 1 Get yuv to decode! idx= " << idx << ", aPts= " << aPts << ", vPts=" << vPts << endl;

	// 如果媒体文件中的音频是在几分钟之后才出现时(aPts<=0表示还没有出现音频包)，前面几分钟的视频要能够播放
	if (aPts <= 0) {
		cout << "[" << 1000.0 * (cur - last) / (double)CLOCKS_PER_SEC << "]!! 2-1 Get yuv to decode OK! idx= " << idx << ", aPts= " << aPts << ", vPts=" << vPts << endl;
        last = cur;
		return ret;
	}
	else {
		// 利用音频同步上了视频
		if (vPts <= aPts) {
			cout << "[" << 1000.0 * (cur - last) / (double)CLOCKS_PER_SEC << "]!! 2-2 Get yuv to decode OK! idx= " << idx << ", aPts= " << aPts << ", vPts=" << vPts << endl;
            last = cur;
            return ret;
		}
		else {
			// 如果上一个音频包播放完毕后没有再出现音频包，则这个条件满足时继续播放视频
			if (finterval > 0 && 1000.0 * (cur - last) / (double)CLOCKS_PER_SEC >= finterval) {
				cout << "[" << 1000.0 * (cur - last) / (double)CLOCKS_PER_SEC << "]!! 2-3 Get yuv to decode OK! idx= " << idx << ", aPts= " << aPts << ", vPts=" << vPts << endl;
                last = cur;
                return ret;
			}
			else
			{
				cout << "!! 2-4 Getting yuv to decode... finterval " << finterval << endl;
				return nullptr;
			}
		}
	}
}

/*
	从AVFrame对象缓存池中获取一个空闲的AVFrame对象，用来存放当前解码出来
	的YUV数据，同时标记该AVFrame对象为非空闲，失败返回空指针。
*/
AVFrame* XFFmpeg::get_yuv_frame() {
	AVFrame* ret = nullptr;

	for (int j = 0; j < max_buffered_yuv_frame_num; j++) {
		poolmutex.lock();
		if (yuvPool[0][j] == 0) {
			yuvPool[0][j] = 1;
			ret = (AVFrame*)yuvPool[1][j];
			poolmutex.unlock();
			cout << "------>get_yuv_frame OK, idx= " << j << endl;
			break;
		}
		poolmutex.unlock();
	}

	return ret;
}

/*
	释放占有的AVFrame对象(注意：AVFrame对象中的buf空间不负责释放)
*/
bool XFFmpeg::release_yuv_frame(AVFrame* frame) {
	for (int j = 0; j < max_buffered_yuv_frame_num; j++) {
		poolmutex.lock();
		if (yuvPool[0][j] == 1 && yuvPool[1][j] == (unsigned int)frame) {
			yuvPool[0][j] = 0;
			poolmutex.unlock();
			cout << "<------release_yuv_frame, index = " << j << endl;
			return true;
		}
		poolmutex.unlock();
	}

	return false;
}

/*
	获取当前AVFrame缓存池中最小的pts
	index返回最小pts对应的AVFrame在缓存池中的下标
*/
int64_t XFFmpeg::get_minimum_frame_pts(int* index) {
	int64_t min = 0x7fffffffffffffff;
	AVFrame* frame = nullptr;
	int idx = -1;

	for (int j = 0; j < max_buffered_yuv_frame_num; j++) {
		poolmutex.lock();
		frame = (AVFrame*)yuvPool[1][j];
		if (yuvPool[0][j] == 1 && frame->best_effort_timestamp < min) {
			min = frame->best_effort_timestamp;
			idx = j;
		}
		poolmutex.unlock();
	}
	if (index) *index = idx;
	return min;
}

bool XFFmpeg::clear_yuv_pool() {
	poolmutex.lock();
	for (int j = 0; j < max_buffered_yuv_frame_num; j++) {
		AVFrame* frame = (AVFrame*)yuvPool[1][j];
		av_frame_free(&frame);
		yuvPool[0][j] = 0;
		yuvPool[1][j] = 0;
	}
	poolmutex.unlock();
	return true;
}

void XFFmpeg::reset_on_seek() {
	poolmutex.lock();
	for (int j = 0; j < max_buffered_yuv_frame_num; j++) {
		AVFrame* frame = (AVFrame*)yuvPool[1][j];
		yuvPool[0][j] = 0;
		av_frame_unref(frame);
	}
	poolmutex.unlock();
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
	clear_yuv_pool();
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
