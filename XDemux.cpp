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
		//��ʼ����װ��
		av_register_all();

		//��ʼ������� �����Դ�rtsp rtmp http Э�����ý����Ƶ��
		avformat_network_init();

		//ע���������н�����

		isFirst = false;
	}
}
XDemux::~XDemux(){ }

void XDemux::Close()
{
	if (!formatCtx) return;

	//�����ȡ��ʽ������
	avformat_close_input(&formatCtx);
	totalTime = 0;
}

void XDemux::Clear()
{
	//�������
	if (!formatCtx) return;

	avformat_flush(formatCtx);
}

bool XDemux::Open(const char *url)
{
	//�������
	Close();
	//��������
	AVDictionary *opts = NULL;
	//����rtsp����tcpЭ���
	av_dict_set(&opts, "rtsp_transport", "tcp", 0);

	//������ʱʱ��
	av_dict_set(&opts, "max_delay", "500", 0);

	int re = avformat_open_input(&formatCtx, url, 0, &opts);
	if (re != 0)
	{
		//������Ϣ
		char buf[1024] = { 0 };
		av_strerror(re, buf, sizeof(buf) - 1);
		cout << "open: " << url << " failed! :" << buf << endl;
		return false;
	}
	cout << "Open:" << url << " Success" << endl;

	//��ȡ����Ϣ ��Բ�ͬ���͵���Ƶ
	re = avformat_find_stream_info(formatCtx, 0);

	//��ʱ��  AV_TIME_BASE/1000 Ϊ����
	totalTime = formatCtx->duration / (AV_TIME_BASE / 1000);
	cout << "video totalTime: " << totalTime << endl;

	//��ӡ��Ƶ����Ϣ
	av_dump_format(formatCtx, 0, url, 0);

	//�ҵ���Ƶ��,��Ƶ��
	videoStream = av_find_best_stream(formatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	audioStream = av_find_best_stream(formatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);

	//��ӡ��Ƶ��Ϣ
	AVStream *stream = formatCtx->streams[videoStream];
	width = stream->codecpar->width;
	height = stream->codecpar->height;

	cout << videoStream << "*************��Ƶ��Ϣ**************" << endl;
	cout << "width: " << stream->codecpar->width << endl;
	cout << "height: " << stream->codecpar->height << endl;
	//֡�� fps
	cout << "video fps:" << r2d(stream->avg_frame_rate) << endl;

	//��ӡ��Ƶ��Ϣ
	stream = formatCtx->streams[audioStream];

	this->sampleRate = stream->codecpar->sample_rate;
	this->channels = stream->codecpar->channels;

	cout << audioStream << "**************��Ƶ��Ϣ**************" << endl;
	cout << "sample rate: " << stream->codecpar->sample_rate << endl;
	cout << "format: " << stream->codecpar->format << endl;
	cout << "channles: " << stream->codecpar->channels << endl;
	cout << "audio codec_id: " << stream->codecpar->codec_id << endl;

	return true;
}

//��������Ƶ����
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

//��ȡһ֡
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

	//pts dtsת���ɺ���
	packet->pts = packet->pts * (r2d(formatCtx->streams[packet->stream_index]->time_base) * 1000);
	packet->dts = packet->dts * (r2d(formatCtx->streams[packet->stream_index]->time_base) * 1000);

	return packet;
}
//�ж�һ֡�Ƿ�Ϊ��Ƶ֡
bool XDemux::IsAudio(AVPacket *pkt)
{
	if (!pkt) return false;

	if (pkt->stream_index == audioStream) return true;

	return false;
}
//�������
bool XDemux::Seek(double pos)
{
	if (!formatCtx) return false;

	//�������
	avformat_flush(formatCtx);

	long long seekPos = 0;
	//��ʱ��*����pos �����ƶ�������
	seekPos = formatCtx->streams[videoStream]->duration * pos;
	//seekֻ�����������ؼ�֡������ʵ�ʵ�λ�����ⲿ��������������
	int re = av_seek_frame(formatCtx, videoStream, seekPos, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);

	if (re < 0) return false;
	return true;
}