#include "XVideoThread.h"
#include "XDistributeThread.h"
#include "XFFmpeg.h"
#include "xplay1.h"
#include <iostream>

using std::cout;
using std::endl;

bool XVideoThread::isExit = true;
bool XVideoThread::isStart = false;
bool XVideoThread::bReset = false;

void XVideoThread::run() {  // 重写QT线程函数(在调用start之后会在线程中运行这个函数)
	int vPts = -1;
	int aPts = -1;

	while (!isExit) {
		if (bReset) {
			cout << "[VIDEO THREAD] ------ Reset video thread! ------" << endl;
			break;
		}
		if (!XVideoThread::isStart || XPlay1::bSeek) {
			msleep(10);
			continue;
		}
		XPlay1::rlock();
		if (XDistributeThread::get()->get_video_list()->size() > 0)
		{
			//cout << "[VIDEO THREAD] ------ Video thread running! ------" << endl;
			AVPacket* pktv = XDistributeThread::get()->get_video_list()->front();

			while(!XFFmpeg::get()->decode(pktv)) {
                msleep(1);
                if (isExit || bReset || !isStart || XPlay1::bSeek) {
                    goto SKIP;
    			}
            }

			av_packet_unref(pktv);
			if (pktv) av_packet_free(&pktv);
			XDistributeThread::get()->get_video_list()->pop_front();
		}
		else {
			msleep(1);
		}
    SKIP:
		//对于解码出来的AVFrame类型的yuv。注意，linesize对应的是一行的长度，比如测试视频是1280x720，且
		//format是AV_PIX_FMT_YUV420P，则linesize [0]/[1]/[2]分别表示一行的yuv长度，为1280/640/640.
	    //对于音频，则是一帧音频的字节数，如果音频是float型（AV_SAMPLE_FMT_S32P）且是双通道的，则
	    //linesize = 4 * 2 * as->codecpar->frame_size(as->codecpar->frame_size表示一帧数据，单通道样本数)
		XPlay1::unlock();
	}
	isExit = true;
	cout << "[VIDEO THREAD] ------ Video thread exit! ------" << endl;
}

XVideoThread:: ~XVideoThread() {

}

XVideoThread::XVideoThread() {  // 不让外部创建线程实例

}