#include "XDemux.h"
#include <iostream>
using namespace std;

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
}

#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"avcodec.lib")

bool XDemux::isFirst = true;

static double r2d(AVRational r)
{
	return r.den == 0 ? 0 : (double)r.num / (double)r.den;
}

XDemux::XDemux()
{
	if (isFirst)
	{
		//初始化封装库
		av_register_all();

		//初始化网络库 （可以打开rtsp rtmp http 协议的流媒体视频）
		avformat_network_init();

		//注册所有所有解码器

		isFirst = false;
	}
}
XDemux::~XDemux(){ }

void XDemux::Close()
{
	if (!formatCtx) return;

	//清除读取格式上下文
	avformat_close_input(&formatCtx);
	totalTime = 0;
}

void XDemux::Clear()
{
	//清除缓存
	if (!formatCtx) return;

	avformat_flush(formatCtx);
}

bool XDemux::Open(const char *url)
{
	//清除缓存
	Close();
	//参数设置
	AVDictionary *opts = NULL;
	//设置rtsp流已tcp协议打开
	av_dict_set(&opts, "rtsp_transport", "tcp", 0);

	//网络延时时间
	av_dict_set(&opts, "max_delay", "500", 0);

	int re = avformat_open_input(&formatCtx, url, 0, &opts);
	if (re != 0)
	{
		//错误信息
		char buf[1024] = { 0 };
		av_strerror(re, buf, sizeof(buf) - 1);
		cout << "open: " << url << " failed! :" << buf << endl;
		return false;
	}
	cout << "Open:" << url << " Success" << endl;

	//获取流信息 针对不同类型的视频
	re = avformat_find_stream_info(formatCtx, 0);

	//总时长  AV_TIME_BASE/1000 为毫秒
	totalTime = formatCtx->duration / (AV_TIME_BASE / 1000);
	cout << "video totalTime: " << totalTime << endl;

	//打印视频流信息
	av_dump_format(formatCtx, 0, url, 0);

	//找到视频流,音频流
	videoStream = av_find_best_stream(formatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	audioStream = av_find_best_stream(formatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);

	//打印视频信息
	AVStream *stream = formatCtx->streams[videoStream];
	width = stream->codecpar->width;
	height = stream->codecpar->height;

	cout << videoStream << "*************视频信息**************" << endl;
	cout << "width: " << stream->codecpar->width << endl;
	cout << "height: " << stream->codecpar->height << endl;
	//帧率 fps
	cout << "video fps:" << r2d(stream->avg_frame_rate) << endl;

	//打印音频信息
	stream = formatCtx->streams[audioStream];

	this->sampleRate = stream->codecpar->sample_rate;
	this->channels = stream->codecpar->channels;

	cout << audioStream << "**************音频信息**************" << endl;
	cout << "sample rate: " << stream->codecpar->sample_rate << endl;
	cout << "format: " << stream->codecpar->format << endl;
	cout << "channles: " << stream->codecpar->channels << endl;
	cout << "audio codec_id: " << stream->codecpar->codec_id << endl;

	return true;
}

//拷贝音视频参数
AVCodecParameters *XDemux::CopyVideoParameters()
{
	if (!formatCtx) return false;

	AVCodecParameters *videoParameters = avcodec_parameters_alloc();
	avcodec_parameters_copy(videoParameters, formatCtx->streams[videoStream]->codecpar);
	return videoParameters;
}
AVCodecParameters *XDemux::CopyAudioParameters()
{
	if (!formatCtx) return false;

	AVCodecParameters *audioParameters = avcodec_parameters_alloc();
	avcodec_parameters_copy(audioParameters, formatCtx->streams[audioStream]->codecpar);
	return audioParameters;
}

//读取一帧
AVPacket* XDemux::Read()
{
	if (!formatCtx)
	{
		return NULL;
	}
	
	AVPacket * packet = av_packet_alloc();

	int re = av_read_frame(formatCtx, packet);
	if (re != 0)
	{
		av_packet_free(&packet);
		return NULL;
	}

	//pts dts转换成毫秒
	packet->pts = packet->pts * (r2d(formatCtx->streams[packet->stream_index]->time_base) * 1000);
	packet->dts = packet->dts * (r2d(formatCtx->streams[packet->stream_index]->time_base) * 1000);

	return packet;
}
//判断一帧是否为音频帧
bool XDemux::IsAudio(AVPacket *pkt)
{
	if (!pkt) return false;

	if (pkt->stream_index == audioStream) return true;

	return false;
}
//快进快退
bool XDemux::Seek(double pos)
{
	if (!formatCtx) return false;

	//清除缓冲
	avformat_flush(formatCtx);

	long long seekPos = 0;
	//总时长*传入pos 就是移动到哪里
	seekPos = formatCtx->streams[videoStream]->duration * pos;
	//seek只是往后跳到关键帧，跳到实际的位置由外部解码来进行做。
	int re = av_seek_frame(formatCtx, videoStream, seekPos, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);

	if (re < 0) return false;
	return true;
}