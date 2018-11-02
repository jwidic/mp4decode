#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h> 
#include <sys/time.h>
#include "include/mp4v2/mp4v2.h"



int get264stream(MP4FileHandle oMp4File,int VTrackId,int totalFrame,char *name)
{

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
    unsigned char *pData=NULL;
    unsigned char *p =NULL;
    unsigned int i=0;
    unsigned int nSize =0;

    MP4Timestamp pStartTime;
    MP4Duration  pDuration;
    MP4Duration  pRenderingOffset;
    unsigned long framesize=0x00000000;
    bool pIsSyncSample =0;
    int nReadIndex =0;
    FILE *pFile = NULL;

    static bool FirstIdx = FALSE;
    if(!oMp4File)
		return -1;
    pFile = fopen(name,"wb");
    // fredy add file head!
    fwrite(HEAD,1,74,pFile);
    FirstIdx = TRUE;

    while(nReadIndex<totalFrame){
        nReadIndex++;
        //printf("nReadIndex:%d\n",nReadIndex);
        MP4ReadSample(oMp4File,VTrackId,nReadIndex,&pData,&nSize,&pStartTime,&pDuration,&pRenderingOffset,&pIsSyncSample);
        p = pData;
        while(nSize > 0){
            fwrite(NAL,4,1,pFile);
            framesize = 0x0;
            framesize |= *pData<<24;
            framesize |= *(pData+1)<<16;
            framesize |= *(pData+2)<<8;
            framesize |= *(pData+3);
//            printf("nSize=%d\n",nSize);
            if( FirstIdx == TRUE ){
                printf("%d\n",nSize+74);
            }else{
                printf("%d\n",nSize);
            }
            nSize = nSize-framesize-4;
//            printf("framesize=%x,nSize=%d\n",framesize,nSize);
            pData = pData+4;
            fwrite(pData,framesize,1,pFile);
            if(nSize > 0)
                pData=pData+framesize;
        }
            //如果传入MP4ReadSample的视频pData是null
            // 它内部就会new 一个内存
            //如果传入的是已知的内存区域
            //则需要保证空间bigger then max frames size.
            free(p);
            pData = NULL;
            p = NULL;
    }
    fflush(pFile);
    fclose(pFile);
    return 0;
}

int openmp4file(char *sMp4file,char *Vname,char *Aname,MP4FileHandle *oMp4File)
{
    int i;

    //unsigned int oStreamDuration;
    unsigned int VFrameCount;
    unsigned int AFrameCount;

    *oMp4File = MP4Read(sMp4file);
    int videoindex = -1,audioindex = -1;
    uint32_t VnumSamples;
    uint32_t AnumSamples;
    //uint32_t timescale;
    //uint64_t duration;        

    if (!*oMp4File)
    {
        printf("Read error....%s\r\n",sMp4file);
        return -1;
    }

    MP4TrackId trackId = MP4_INVALID_TRACK_ID;
    uint32_t numTracks = MP4GetNumberOfTracks(*oMp4File,NULL,0);//获取音视频轨道数 2 
    //  printf("numTracks:%d\n",numTracks);

    for (i = 0; i < numTracks; i++)
    {
        trackId = MP4FindTrackId(*oMp4File, i,NULL,0); //获取trackId，判断获取数据类型 1 获取视频数据，2 获取音频数据
//        printf("trackId:%d\n",trackId);
        const char* trackType = MP4GetTrackType(*oMp4File, trackId);
        if (MP4_IS_VIDEO_TRACK_TYPE(trackType)){
        //printf("[%s %d] trackId:%d\r\n",__FUNCTION__,__LINE__,trackId);
        videoindex= trackId;

         //duration = MP4GetTrackDuration(oMp4File, trackId );
        VnumSamples = MP4GetTrackNumberOfSamples(*oMp4File, trackId);
        //timescale = MP4GetTrackTimeScale(oMp4File, trackId);
        //oStreamDuration = duration/(timescale/1000);
        VFrameCount = VnumSamples;

        }else if (MP4_IS_AUDIO_TRACK_TYPE(trackType)){
            audioindex = trackId;
            AnumSamples = MP4GetTrackNumberOfSamples(*oMp4File, trackId);
            AFrameCount = AnumSamples;
            //  printf("audioindex:%d\n",audioindex);
        }
    }
    //解析完了mp4，主要是为了获取sps pps 还有video的trackID
    if(videoindex >= 0){
        get264stream(*oMp4File,videoindex,VFrameCount,Vname);
    }else if(audioindex >= 0){
    //GetAacStream(oMp4File,aduindex,AFrameCount,Aname);
    }

    //需要mp4close 否则在嵌入式设备打开mp4上多了会内存泄露挂掉.
    MP4Close(*oMp4File,0);
    return 0;
}

int main(int argc,char **argv)
{
    MP4FileHandle oMp4File;
    openmp4file("cbbRec.mp4", "test.h264","test.26", &oMp4File);
    return 0;
}

