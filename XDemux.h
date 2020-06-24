#pragma once
#include <iostream>
struct AVFormatContext;
struct AVPacket;
struct AVCodecParameters;
struct AVPacket;
class XDemux
{
public:
	//打开文件
	virtual bool Open(const char* url);
	//清理读取缓存
	virtual void Clear();
	virtual void Close();
	//读取一帧（音频/视频)
	virtual AVPacket* Read();	
	//判断一帧是否为音频
	virtual bool IsAudio(AVPacket*);

	//拷贝音视频参数
	AVCodecParameters *CopyVideoParameters();
	AVCodecParameters *CopyAudioParameters();

	//快进快退操作
	bool Seek(double pos);

	
	XDemux();
	~XDemux();
protected:
	//解封装上下文
	AVFormatContext *formatCtx = NULL;
	//音视频流下标
	int videoStream = 0;
	int audioStream = 1;

public:
	int height = 0;					//视频的高度
	int width = 0;					//视频的宽度
	int sampleRate = 0;				//采样率
	int channels = 0;			    //声道数
	int totalTime = 0;				//视频的总时长
	static bool isFirst;
};

