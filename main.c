#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h> 
#include <sys/time.h>
#include "include/mp4v2/mp4v2.h"
#include "MP4Encoder.h"


//#define CBB_REC_FILENAME                        "/mnt/cbb/cbbRec.h264"

int get264stream(MP4FileHandle oMp4File,int VTrackId,int totalFrame,char *name)
{
    unsigned char *pData=NULL;
    unsigned char *p =NULL;
    unsigned int i=0;
    unsigned int nSize =0;

    MP4Timestamp pStartTime;
    MP4Duration  pDuration;
    MP4Duration  pRenderingOffset;
    unsigned long framesize=0x00000000;
    bool pIsSyncSample =0;
    static int nReadIndex =0;
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
                FirstIdx = FALSE;
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







int main(int argc,char **argv){

    openmp4file("cbbRec.mp4", &F_H264_MP4DECODER.hMp4file);


    if(F_H264_MP4DECODER.m_videoId >= 0){
       get264stream(F_H264_MP4DECODER.hMp4file,F_H264_MP4DECODER.m_videoId,F_H264_MP4DECODER.VnumSamples,"test.h264");
 // int fget264stream(MP4FileHandle oMp4File,int VTrackId,int totalFrame)
    }

    closemp4file(&F_H264_MP4DECODER.hMp4file);
    return 0;
}

