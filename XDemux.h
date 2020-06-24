#pragma once
#include <iostream>
struct AVFormatContext;
struct AVPacket;
struct AVCodecParameters;
struct AVPacket;
class XDemux
{
public:
	//���ļ�
	virtual bool Open(const char* url);
	//�����ȡ����
	virtual void Clear();
	virtual void Close();
	//��ȡһ֡����Ƶ/��Ƶ)
	virtual AVPacket* Read();	
	//�ж�һ֡�Ƿ�Ϊ��Ƶ
	virtual bool IsAudio(AVPacket*);

	//��������Ƶ����
	AVCodecParameters *CopyVideoParameters();
	AVCodecParameters *CopyAudioParameters();

	//������˲���
	bool Seek(double pos);

	
	XDemux();
	~XDemux();
protected:
	//���װ������
	AVFormatContext *formatCtx = NULL;
	//����Ƶ���±�
	int videoStream = 0;
	int audioStream = 1;

public:
	int height = 0;					//��Ƶ�ĸ߶�
	int width = 0;					//��Ƶ�Ŀ��
	int sampleRate = 0;				//������
	int channels = 0;			    //������
	int totalTime = 0;				//��Ƶ����ʱ��
	static bool isFirst;
};

