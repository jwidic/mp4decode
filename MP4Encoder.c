/********************************************************************
filename:   MP4Encoder.cpp
created:    2013-04-16
author:     firehood
purpose:    MP4编码器，基于开源库mp4v2实现（https://code.google.com/p/mp4v2/）。
*********************************************************************/
#include "MP4Encoder.h"
#include <string.h>

#define BUFFER_SIZE  (1024*1024)



void MP4Encoder_init(void)
{
	F_H264_MP4ENCODER.m_videoId =NULL;
	F_H264_MP4ENCODER.m_nWidth  =0;
	F_H264_MP4ENCODER.m_nHeight =0;
	F_H264_MP4ENCODER.m_nTimeScale =0;
	F_H264_MP4ENCODER.m_nFrameRate =0;
	F_H264_MP4ENCODER.hMp4file=NULL;
}


bool CreateMP4File(const char *pFileName, int width, int height, int timeScale/* = 90000*/, int frameRate/* = 25*/)
{
	if (pFileName == NULL)
	{
		return false;
	}
	// create mp4 file
	F_H264_MP4ENCODER.hMp4file = MP4Create(pFileName,0);
	if (F_H264_MP4ENCODER.hMp4file == MP4_INVALID_FILE_HANDLE)
	{
		printf("ERROR:Open file fialed.\n");
		return false;
	}
	F_H264_MP4ENCODER.m_nWidth = width;
	F_H264_MP4ENCODER.m_nHeight = height;
	F_H264_MP4ENCODER.m_nTimeScale = 1200000;
	F_H264_MP4ENCODER.m_nFrameRate = 25;
	MP4SetTimeScale(F_H264_MP4ENCODER.hMp4file, F_H264_MP4ENCODER.m_nTimeScale);
	return true;
}

bool Write264Metadata(MP4FileHandle hMp4File, LPMP4ENC_Metadata lpMetadata)
{
	F_H264_MP4ENCODER.m_videoId = MP4AddH264VideoTrack
	(hMp4File,
		F_H264_MP4ENCODER.m_nTimeScale,
		F_H264_MP4ENCODER.m_nTimeScale /F_H264_MP4ENCODER.m_nFrameRate,
		F_H264_MP4ENCODER.m_nWidth, // width
		F_H264_MP4ENCODER.m_nHeight,// height
		lpMetadata->Sps[1], // sps[1] AVCProfileIndication
		lpMetadata->Sps[2], // sps[2] profile_compat
		lpMetadata->Sps[3], // sps[3] AVCLevelIndication
		3);           // 4 bytes length before each NAL unit
	if (F_H264_MP4ENCODER.m_videoId == MP4_INVALID_TRACK_ID)
	{
		printf("add video track failed.\n");
		return false;
	}
	MP4SetVideoProfileLevel(hMp4File, 0x01); //  Simple Profile @ Level 3

											 // write sps
	MP4AddH264SequenceParameterSet(hMp4File, F_H264_MP4ENCODER.m_videoId, lpMetadata->Sps, lpMetadata->nSpsLen);

	// write pps
	MP4AddH264PictureParameterSet(hMp4File, F_H264_MP4ENCODER.m_videoId, lpMetadata->Pps, lpMetadata->nPpsLen);

	return true;
}

int WriteH264Data(MP4FileHandle hMp4File, const unsigned char* pData, int size)
{
    unsigned char data[BUFFER_SIZE] =0;
	if (hMp4File == NULL)
	{
		return -1;
	}
	if (pData == NULL)
	{
		return -1;
	}
	MP4ENC_NaluUnit nalu;
	int pos = 0, len = 0;
	while (len = ReadOneNaluFromBuf(pData, size, pos, nalu))
	{
		if (nalu.type == 0x07) // sps
		{
			// 添加h264 track
			F_H264_MP4ENCODER.m_videoId = MP4AddH264VideoTrack
			(hMp4File,
				F_H264_MP4ENCODER.m_nTimeScale,
				F_H264_MP4ENCODER.m_nTimeScale / F_H264_MP4ENCODER.m_nFrameRate,
				F_H264_MP4ENCODER.m_nWidth,     // width
				F_H264_MP4ENCODER.m_nHeight,    // height
				nalu.data[1], // sps[1] AVCProfileIndication
				nalu.data[2], // sps[2] profile_compat
				nalu.data[3], // sps[3] AVCLevelIndication
				3);           // 4 bytes length before each NAL unit
			if (F_H264_MP4ENCODER.m_videoId == MP4_INVALID_TRACK_ID)
			{
				printf("add video track failed.\n");
				return 0;
			}
			MP4SetVideoProfileLevel(hMp4File, 1); //  Simple Profile @ Level 3

			MP4AddH264SequenceParameterSet(hMp4File, F_H264_MP4ENCODER.m_videoId, nalu.data, nalu.size);
		}
		else if (nalu.type == 0x08) // pps
		{
			MP4AddH264PictureParameterSet(hMp4File, F_H264_MP4ENCODER.m_videoId, nalu.data, nalu.size);
		}
		else
		{
			int datalen = nalu.size + 4;
			// MP4 Nalu前四个字节表示Nalu长度
			data[0] = nalu.size >> 24;
			data[1] = nalu.size >> 16;
			data[2] = nalu.size >> 8;
			data[3] = nalu.size & 0xff;
			memcpy(data + 4, nalu.data, nalu.size);
			if (!MP4WriteSample(hMp4File, F_H264_MP4ENCODER.m_videoId, data, datalen, MP4_INVALID_DURATION, 0, 1))
			{
				return 0;
			}
			delete[] data;
		}

		pos += len;
	}
	return pos;
}

int ReadOneNaluFromBuf(const unsigned char *buffer, unsigned int nBufferSize, unsigned int offSet, MP4ENC_NaluUnit &nalu)
{
	int i = offSet;
	while (i<nBufferSize)
	{
		if (buffer[i++] == 0x00 &&
			buffer[i++] == 0x00 &&
			buffer[i++] == 0x00 &&
			buffer[i++] == 0x01
			)
		{
			int pos = i;
			while (pos<nBufferSize)
			{
				if (buffer[pos++] == 0x00 &&
					buffer[pos++] == 0x00 &&
					buffer[pos++] == 0x00 &&
					buffer[pos++] == 0x01
					)
				{
					break;
				}
			}
			if (pos == nBufferSize)
			{
				nalu.size = pos - i;
			}
			else
			{
				nalu.size = (pos - 4) - i;
			}

			nalu.type = buffer[i] & 0x1f;
			nalu.data = (unsigned char*)&buffer[i];
			return (nalu.size + i - offSet);
		}
	}
	return 0;
}

void CloseMP4File(MP4FileHandle hMp4File)
{
	if (hMp4File)
	{
		MP4Close(hMp4File);
		hMp4File = NULL;
	}
}

bool WriteH264File(const char* pFile264, const char* pFileMp4)
{
	if (pFile264 == NULL || pFileMp4 == NULL)
	{
		return false;
	}

	MP4FileHandle hMp4File = CreateMP4File(pFileMp4, 352, 288);

	if (hMp4File == NULL)
	{
		printf("ERROR:Create file failed!");
		return false;
	}

	FILE *fp = fopen(pFile264, "rb");
	if (!fp)
	{
		printf("ERROR:open file failed!");
		return false;
	}
	fseek(fp, 0, SEEK_SET);

	unsigned char *buffer = new unsigned char[BUFFER_SIZE];
	int pos = 0;
	while (1)
	{
		int readlen = fread(buffer + pos, sizeof(unsigned char), BUFFER_SIZE - pos, fp);


		if (readlen <= 0)
		{
			break;
		}

		readlen += pos;

		int writelen = 0;
		for (int i = readlen - 1; i >= 0; i--)
		{
			if (buffer[i--] == 0x01 &&
				buffer[i--] == 0x00 &&
				buffer[i--] == 0x00 &&
				buffer[i--] == 0x00
				)
			{
				writelen = i + 5;
				break;
			}
		}

		writelen = WriteH264Data(hMp4File, buffer, writelen);
		if (writelen <= 0)
		{
			break;
		}
//		memcpy(buffer, buffer + writelen, readlen - writelen + 1);
		pos = readlen - writelen + 1;
	}
	fclose(fp);

	delete[] buffer;
	CloseMP4File(hMp4File);

	return true;
}

bool PraseMetadata(const unsigned char* pData, int size, MP4ENC_Metadata &metadata)
{
	if (pData == NULL || size<4)
	{
		return false;
	}
	MP4ENC_NaluUnit nalu;
	int pos = 0;
	bool bRet1 = false, bRet2 = false;
	while (int len = ReadOneNaluFromBuf(pData, size, pos, nalu))
	{
		if (nalu.type == 0x07)
		{
			memcpy(metadata.Sps, nalu.data, nalu.size);
			metadata.nSpsLen = nalu.size;
			bRet1 = true;
		}
		else if ((nalu.type == 0x08))
		{
			memcpy(metadata.Pps, nalu.data, nalu.size);
			metadata.nPpsLen = nalu.size;
			bRet2 = true;
		}
		pos += len;
	}
	if (bRet1 && bRet2)
	{
		return true;
	}
	return false;
}
