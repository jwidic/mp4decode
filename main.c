#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h> 
#include <sys/time.h>
#include "include/mp4v2/mp4v2.h"

unsigned char sps[64],pps[64];
int spslen = 0,ppslen = 0;

int get264stream(MP4FileHandle oMp4File,int VTrackId,int totalFrame,char *name)
{

    char NAL[5]={0x00,0x00,0x00,0x01};
    unsigned char *pData=NULL;
    unsigned char *p =NULL;
    unsigned int i=0;
    unsigned int nSize =0;

    MP4Timestamp pStartTime;
    MP4Duration pDuration;
    MP4Duration pRenderingOffset;
    unsigned long framesize=0x00000000;
    bool pIsSyncSample =0;
    int nReadIndex =0;
    FILE *pFile = NULL;

    if(!oMp4File)
		return -1;
    pFile = fopen(name,"wb");


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
            printf("nSize=%d\n",nSize);
            nSize = nSize-framesize-4;
            printf("framesize=%x,nSize=%d\n",framesize,nSize);
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
#if 0
int GetAacStream(MP4FileHandle oMp4File,int ATrackId,int totalFrame,char *name)
{
    if(!oMp4File) return -1;
    char NAL[5] = {0x00,0x00,0x00,0x01};    
    unsigned char *pData = NULL;
    unsigned char *p = NULL;
    unsigned int i=0;
    unsigned int nSize = 0;
    MP4Timestamp pStartTime;
    MP4Duration pDuration;
    MP4Duration pRenderingOffset;
    unsigned long framesize=0x00000000;

    int nReadIndex = 0;
    FILE *pFile = NULL;
    pFile = fopen(name,"wb"); 

    while(nReadIndex < totalFrame)
    {   
        nReadIndex ++;
  
        MP4ReadSample(oMp4File,ATrackId,nReadIndex,&pData,&nSize,&pStartTime,&pDuration,&pRenderingOffset,&pIsSyncSample);
        p = pData;
        fwrite(pData,nsize,1,pFile);
        
        
        //如果传入MP4ReadSample的视频pData是null
        // 它内部就会new 一个内存
        //如果传入的是已知的内存区域，
        //则需要保证空间bigger then max frames size.
        free(p);
        pData = NULL;
        p = NULL;
    }       
    fflush(pFile);
    fclose(pFile);  

  
    return 0;
}
#endif
int openmp4file(char *sMp4file,char *Vname,char *Aname)
{
    MP4FileHandle oMp4File;
    int i;

    //unsigned int oStreamDuration;
    unsigned int VFrameCount;
    unsigned int AFrameCount;

    oMp4File = MP4Read(sMp4file);
    int videoindex = -1,audioindex = -1;
    uint32_t VnumSamples;
    uint32_t AnumSamples;
    //uint32_t timescale;
    //uint64_t duration;        

    if (!oMp4File)
    {
        printf("Read error....%s\r\n",sMp4file);
        return -1;
    }

    MP4TrackId trackId = MP4_INVALID_TRACK_ID;
    uint32_t numTracks = MP4GetNumberOfTracks(oMp4File,NULL,0);//获取音视频轨道数 2 
    printf("numTracks:%d\n",numTracks);

    for (i = 0; i < numTracks; i++)
    {
        trackId = MP4FindTrackId(oMp4File, i,NULL,0); //获取trackId，判断获取数据类型 1 获取视频数据，2 获取音频数据
        printf("trackId:%d\n",trackId);
        const char* trackType = MP4GetTrackType(oMp4File, trackId);
        if (MP4_IS_VIDEO_TRACK_TYPE(trackType)){
        //printf("[%s %d] trackId:%d\r\n",__FUNCTION__,__LINE__,trackId);
        videoindex= trackId;

         //duration = MP4GetTrackDuration(oMp4File, trackId );
        VnumSamples = MP4GetTrackNumberOfSamples(oMp4File, trackId);
        //timescale = MP4GetTrackTimeScale(oMp4File, trackId);          
        //oStreamDuration = duration/(timescale/1000);          
        VFrameCount = VnumSamples;
        #if 0
    // read sps/pps 
    uint8_t **seqheader;
    uint8_t **pictheader;
    uint32_t *pictheadersize;
    uint32_t *seqheadersize;
    uint32_t ix;
    MP4GetTrackH264SeqPictHeaders(oMp4File, trackId, &seqheader, &seqheadersize, &pictheader, &pictheadersize);//读取pps和sps

    for (ix = 0; seqheadersize[ix] != 0; ix++)
    {
    memcpy(sps, seqheader[ix], seqheadersize[ix]);//获取sps
    spslen = seqheadersize[ix];
    free(seqheader[ix]);
    }
    free(seqheader);
    free(seqheadersize);
    for (ix =0;pictheader[ix]!=0;ix++)
    {
    memcpy(pps,pictheader[ix],pictheadersize[ix]); //获取pps
     ppslen=pictheadersize[ix];
                    free(pictheader[ix]);
                }
                    free(pictheader);
                    free(pictheadersize);
        #endif
        }else if (MP4_IS_AUDIO_TRACK_TYPE(trackType)){
            audioindex = trackId;
            AnumSamples = MP4GetTrackNumberOfSamples(oMp4File, trackId);
            AFrameCount = AnumSamples;
            //  printf("audioindex:%d\n",audioindex);
        }
    }
    //解析完了mp4，主要是为了获取sps pps 还有video的trackID
    if(videoindex >= 0){
        get264stream(oMp4File,videoindex,VFrameCount,Vname);
    }else if(audioindex >= 0){
    //GetAacStream(oMp4File,aduindex,AFrameCount,Aname);
    }

    //需要mp4close 否则在嵌入式设备打开mp4上多了会内存泄露挂掉.
    MP4Close(oMp4File,0);
    return 0;
}

int main(int argc,char **argv)
{
    openmp4file("20181029.mp4", "test.h264","test.26");
    return 0;
}

