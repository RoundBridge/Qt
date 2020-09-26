#include "XFFmpeg.h"
#include <iostream>
using std::cout;
using std::endl;

#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "swscale.lib")

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
		compute_duration_ms();
		re = create_decoder(ic);
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
	// 如果是视频包，pkt中的stream_index直接指明视频流的idx，音频包同理
	// 发送packet到解码线程，只是将pkt扔给解码线程，几乎不占用CPU
	// send NULL后调用多次receive可以取出所有缓冲帧
	int re = avcodec_send_packet(cc, pkt);
	if (0 != re) {
		mutex.unlock();
		cout << "[DECODE] avcodec_send_packet failed: " << get_error(re) << endl;
		return NULL;
	}
	// 只是从解码线程中获取解码结果，几乎不占用CPU，一次send（一次可能发送多个音频帧）可能对应多次receive
	// 为什么这里只获取一次？难道不应该多获取几次，把里面的帧全部获取出来？
	// 下面avcodec_receive_frame的第一个参数不能用ic->streams[pkt->stream_index]->codec了，因为它已经是
	// 废弃属性了，用它的话，函数调用失败
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
		vSwsCtx,  //传NULL会新创建，否则比较该上下文和下面这些参数将要创建出来的上下文是否一致，若一致则直接返回该上下文，不一致则创建新的上下文
		frame->width, frame->height,	//输入的宽高
		(AVPixelFormat)frame->format,	//输入的格式，YUV420p
		out_w, out_h,					//输出的宽高
		out_pixfmt,						//输出格式
		SWS_BILINEAR,					//尺寸转换的算法
		0, 0, 0
	);
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
	std::string re = this->error_buf;  // 把一个char*赋值给string对象时会复制一份，用一个复制的空间避免多线程访问出现异常
	mutex.unlock();
	return re;
}

XFFmpeg::XFFmpeg() {
	//错误信息缓存初始化
	error_buf[error_len] = '\0';
	//初始化封装库（其实最新版本这个函数已被废弃，调用会产生被声明为已否决的错误）
	av_register_all();
	//初始化网络库，可以打开rtsp、rtmp、http等协议的流媒体视频
	avformat_network_init();
}

XFFmpeg::~XFFmpeg() {

}