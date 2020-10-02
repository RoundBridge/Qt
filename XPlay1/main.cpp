#include "xplay1.h"
#include <QtWidgets/QApplication>
#include "XFFmpeg.h"
#include <iostream>
using std::cout;
using std::endl;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //int re = 0;
    //uint8_t* out = NULL;
    //int out_w = 800, out_h = 600;
    //AVPixelFormat pixfmt = AV_PIX_FMT_YUV422P;
    //AVPacket* pkt = NULL;
    //AVFrame* yuv = NULL;
    //const char* path = "1024.mp4";

    //if (pixfmt == AV_PIX_FMT_ARGB
    //    || pixfmt == AV_PIX_FMT_RGBA
    //    || pixfmt == AV_PIX_FMT_ABGR
    //    || pixfmt == AV_PIX_FMT_BGRA) {
    //    out = new uint8_t[out_w * out_h * 4];
    //}
    //else if (pixfmt == AV_PIX_FMT_YUV422P) {
    //    out = new uint8_t[out_w * out_h * 2];
    //}

    //re = XFFmpeg::get()->open(path);
    //if (0 != re) {
    //    cout << "open " << path << " failed: " << XFFmpeg::get()->get_error(re) << endl;
    //    return -1;
    //}
    //else {
    //    cout << "open " << path << " success! The total length is " << XFFmpeg::get() ->get_duration_ms() << " ms." << endl;
    //}

    //while (1) {
    //    /* 读视频中的每一个包 */
    //    pkt = XFFmpeg::get()->read();
    //    if (pkt == NULL || pkt->size == 0)break;
    //    cout << "[READ] - PTS = " << pkt->pts << endl;
    //    cout << "[READ] - SIZE = " << pkt->size << endl;

    //    // 处理视频
    //    if (XFFmpeg::get()->videoStream == pkt->stream_index) {
    //        /* 对读到的包解码成YUV图像 */
    //        yuv = XFFmpeg::get()->decode(pkt);
    //        /*
    //            注意，这里不能这么写：
    //                if(yuv == NULL) break;
    //            因为正常情况下，前三次av_read_frame得到的pkt送去解码也会失败的，显示“Resource temporarily unavailable”,
    //            所以直接break就会中断解码过程，因此要向下面这么判断
    //        */
    //        if (yuv != NULL) {
    //            cout << "[DECODE] -- W/H = " << yuv->width << "/" << yuv->height << endl;
    //            //注意，linesize对应的是一行的长度，比如测试视频是1280x720，且format是AV_PIX_FMT_YUV420P，
    //            //则linesize [0]/[1]/[2]分别表示一行的yuv长度，为1280/640/640
    //            //对于音频，则是一帧音频的字节数，如果音频是float型（AV_SAMPLE_FMT_S32P）且是双通道的，则
    //            //linesize = 4 * 2 * as->codecpar->frame_size(as->codecpar->frame_size表示一帧数据，单通道样本数)
    //            cout << "[DECODE] -- format = " << yuv->format << ", linesize [0]/[1]/[2] = " << yuv->linesize[0] << "/" << yuv->linesize[1] << "/" << yuv->linesize[2] << endl;
    //            re = XFFmpeg::get()->video_convert(yuv, out, out_w, out_h, pixfmt);
    //            if (re == true) {
    //                cout << "[CONVERT] --- video_convert OK!" << endl;
    //            }
    //            else {
    //                cout << "[CONVERT] --- video_convert FAILED!" << endl;
    //            }                
    //        }
    //        av_packet_unref(pkt);
    //    }
    //    else {
    //        av_packet_unref(pkt);
    //    }
    //}
    //delete[] out;
    //XFFmpeg::get()->close();
    XPlay1 w;
    w.show();
    return a.exec();
}