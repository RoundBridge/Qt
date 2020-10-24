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

void XVideoThread::run() {  // ��дQT�̺߳���(�ڵ���start֮������߳��������������)	
	int vPts = -1;
	int aPts = -1;

	while (!isExit) {
		if (bReset) {
			cout << "[VIDEO THREAD] ------ Reset video thread! ------" << endl;
			break;
		}
		if (!XVideoThread::isStart) {
			msleep(10);
			continue;
		}
		XPlay1::rlock();
		if (XDistributeThread::get()->get_video_list()->size() > 0)
		{
			//cout << "[VIDEO THREAD] ------ Video thread running! ------" << endl;
			AVPacket* pktv = XDistributeThread::get()->get_video_list()->front();
			vPts = XFFmpeg::get()->get_current_video_pts(pktv);
			aPts = XFFmpeg::get()->get_current_audio_pts();
			// ���1���������Ƶ֡ʱ��������ڵ�ǰ��Ƶ֡ʱ�����˵���������Ƶ֡���ڵ�ǰ��Ƶ֡�ĺ��棬�Ǿ�
			//        ��������Ƶ������Ƶ�ڶ������ٵȴ�һ���������Ƶ֡��ʱ���������ƥ����Ƶ֡ʱ���
			// ���2���������Ƶ֡ʱ���С�ڵ��ڵ�ǰ��Ƶ֡��ʱ���ʱ�����Ÿ���Ƶ֡
			while (vPts > aPts && !XPlay1::bSeek && !bReset && isStart && !isExit) {
				/*
					�˴�����Ƶ��ͬ����Ƶ��Ҳ������Ƶ������Ƶʱ��������в���
					��Ϊ��Ƶ�ڲ����ʡ�ͨ������������λ��ȷ��������£������ٶ��ǹ̶�
					�ģ�����Ƶ֡��֮֡����Ҫ�м�������������ͨ��sleep��ʵ�־Ͳ�׼
					ȷ����ƫ����ۻ�����������Ƶ��ͬ����Ƶ
				*/
				//cout << "[VIDEO THREAD] ------ vPts " << vPts << "ms, aPts " << aPts << " ms ------" << endl;
				msleep(5);
				aPts = XFFmpeg::get()->get_current_audio_pts();
			}
			if (isExit || bReset) {
				XPlay1::unlock();
				break;
			}
			if (!isStart || XPlay1::bSeek) {
				XPlay1::unlock();
				continue;
			}
			XFFmpeg::get()->decode(pktv);
			av_packet_unref(pktv);
			if (pktv) av_packet_free(&pktv);
			XDistributeThread::get()->get_video_list()->pop_front();
		}
		else {
			msleep(1);
		}
		//���ڽ��������AVFrame���͵�yuv��ע�⣬linesize��Ӧ����һ�еĳ��ȣ����������Ƶ��1280x720����
		//format��AV_PIX_FMT_YUV420P����linesize [0]/[1]/[2]�ֱ��ʾһ�е�yuv���ȣ�Ϊ1280/640/640.
	    //������Ƶ������һ֡��Ƶ���ֽ����������Ƶ��float�ͣ�AV_SAMPLE_FMT_S32P������˫ͨ���ģ���
	    //linesize = 4 * 2 * as->codecpar->frame_size(as->codecpar->frame_size��ʾһ֡���ݣ���ͨ��������)
		XPlay1::unlock();
	}
	isExit = true;
	cout << "[VIDEO THREAD] ------ Video thread exit! ------" << endl;
}

XVideoThread:: ~XVideoThread() {

}

XVideoThread::XVideoThread() {  // �����ⲿ�����߳�ʵ��

}