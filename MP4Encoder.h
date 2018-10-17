/********************************************************************
filename:   MP4Encoder.h
created:    2013-04-16
author:     firehood
purpose:    MP4编码器，基于开源库mp4v2实现（https://code.google.com/p/mp4v2/）。
*********************************************************************/
#include "include/mp4v2/mp4v2.h"

// NALU单元
typedef struct _MP4ENC_NaluUnit
{
	int type;
	int size;
	unsigned char *data;
}MP4ENC_NaluUnit;

typedef struct _MP4ENC_Metadata
{
	// video, must be h264 type
	unsigned int    nSpsLen;
	unsigned char   Sps[1024];
	unsigned int    nPpsLen;
	unsigned char   Pps[1024];

} MP4ENC_Metadata, *LPMP4ENC_Metadata;

struct H264_MP4ENCODER
{
	int m_nWidth;
	int m_nHeight;
	int m_nFrameRate;
	int m_nTimeScale;
	MP4TrackId m_videoId;
	MP4FileHandle hMp4file;
    unsigned char *data;

}F_H264_MP4ENCODER;
    //init basic struct variable
    void MP4Encoder_init(void);
	// open or creat a mp4 file.
	bool CreateMP4File(const char *fileName, int width, int height, int timeScale, int frameRate);
	// wirte 264 metadata in mp4 file.
	bool Write264Metadata(MP4FileHandle hMp4File, LPMP4ENC_Metadata lpMetadata);
	// wirte 264 data, data can contain  multiple frame.
	int WriteH264Data(MP4FileHandle hMp4File, const unsigned char* pData, int size);
	// close mp4 file.
	void CloseMP4File(MP4FileHandle hMp4File);
	// convert H264 file to mp4 file.
	// no need to call CreateMP4File and CloseMP4File,it will create/close mp4 file automaticly.
	bool WriteH264File(const char* pFile264, const char* pFileMp4);
	// Prase H264 metamata from H264 data frame
	bool PraseMetadata(const unsigned char* pData, int size, MP4ENC_Metadata *metadata);
	// read one nalu from H264 data buffer
	int ReadOneNaluFromBuf(const unsigned char *buffer, unsigned int nBufferSize, unsigned int offSet, MP4ENC_NaluUnit *nalu);


