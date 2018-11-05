/********************************************************************
filename:   MP4Encoder.h
created:    2013-04-16
author:     firehood
purpose:    MP4编码器，基于开源库mp4v2实现（https://code.google.com/p/mp4v2/）。
*********************************************************************/
#include "include/mp4v2/mp4v2.h"
const static char HEAD[75]={0x00,0x00,0x00,0x01,0x27,0x64,0x00,0x28,
                            0xad,0x84,0x05,0x45,0x62,0xb8,0xac,0x54,
                            0x71,0x08,0x0a,0x8a,0xc5,0x71,0x58,0xa8,
                            0xe2,0x10,0x24,0x85,0x21,0x39,0x3c,0x9f,
                            0x27,0xe4,0xfe,0x4f,0xc9,0xf2,0x79,0xb9,
                            0xb3,0x4d,0x08,0x12,0x42,0x90,0x9c,0x9e,
                            0x4f,0x93,0xf2,0x7f,0x27,0xe4,0xf9,0x3c,
                            0xdc,0xd9,0xa6,0x17,0x2a,0x01,0x40,0x16,
                            0xe4,0x00,0x00,0x00,0x01,0x28,0xfe,0x01,
                            0xae,0x2c};
const static char NAL[5]={0x00,0x00,0x00,0x01}; //h264 frame head
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

}F_H264_MP4ENCODER;

struct H264_MP4DECODER
{
    unsigned int nSize;
    unsigned char *pData;
    MP4Timestamp pStartTime;
    MP4Duration  pDuration;
    MP4Duration  pRenderingOffset;
    unsigned int VnumSamples;
	MP4TrackId m_videoId;
	MP4FileHandle hMp4file;

}F_H264_MP4DECODER;

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

    int closemp4file( MP4FileHandle *oMp4File );
    int openmp4file(char *sMp4file,MP4FileHandle *oMp4File);
