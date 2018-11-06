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
void get_loop()
{
;;;;
}
static int gene_filename(char *pFileName,const char *pFileType )
{
	time_t curTime = time(NULL);
	struct tm *sTm = localtime(&curTime);
	sprintf(pFileName,"%02d%02d%02d%02d%02d%02d",(sTm->tm_year-100),(sTm->tm_mon+1),
                                    sTm->tm_mday,sTm->tm_hour,sTm->tm_min,sTm->tm_sec);
	strcat(pFileName,pFileType);
	printf("pFileName %s size %d \n",pFileName, sizeof(pFileName));

	return 0;
}

void get_h264()
{
        int video_frame =0;
		unsigned char *pData=NULL;
		unsigned char *p =NULL;

		static unsigned int nSize =0;
		// addition
		int frameLength = 0;

		MP4Timestamp pStartTime;
		MP4Duration  pDuration;
		MP4Duration  pRenderingOffset;
		unsigned long framesize=0x00000000;
		bool pIsSyncSample =0;
		static int nReadIndex =0;
		static bool IsMp4Open = FALSE;
		static bool FirstIdx = FALSE;
        static FILE *pFile = NULL;

		if(!IsMp4Open){
            // add pFile


			openmp4file("cbbRec.mp4", &F_H264_MP4DECODER.hMp4file);
			FirstIdx = TRUE;
			IsMp4Open = TRUE;
            nReadIndex=0;

			//add write the h264

            static char pFileName[20];
            gene_filename(pFileName,".h264");
            pFile = fopen(pFileName,"wb");

		}

        if(nReadIndex >= F_H264_MP4DECODER.VnumSamples){
            if( IsMp4Open ){
                // add
                fflush(pFile);
                fclose(pFile);

                //add write the h264
                char pFileName[20];
                gene_filename(pFileName,".h264");
                pFile = fopen(pFileName,"wb");

                IsMp4Open = FALSE;
                closemp4file(&F_H264_MP4DECODER.hMp4file);
                openmp4file("cbbRec.mp4", &F_H264_MP4DECODER.hMp4file);
                IsMp4Open = TRUE;
                FirstIdx = TRUE;
                nReadIndex=0;
                if(!F_H264_MP4DECODER.hMp4file){
                    printf("error open MP4 file \n");
                }
            }else{

                printf("File is not exist");

            }
        }


		if(!F_H264_MP4DECODER.hMp4file){
			printf("error open MP4 file \n");
		}

        if(nReadIndex >= F_H264_MP4DECODER.VnumSamples){
			IsMp4Open = FALSE;
			closemp4file(&F_H264_MP4DECODER.hMp4file);
			openmp4file("cbbRec.mp4", &F_H264_MP4DECODER.hMp4file);
			IsMp4Open = TRUE;
			FirstIdx = TRUE;
			nReadIndex=0;
			if(!F_H264_MP4DECODER.hMp4file){
				printf("error open MP4 file \n");
			}

        }
		if(nReadIndex<F_H264_MP4DECODER.VnumSamples){
			if(nSize <= 0){
				nReadIndex++;
//				printf("nReadIndex:%d\n",nReadIndex);
				MP4ReadSample(F_H264_MP4DECODER.hMp4file,F_H264_MP4DECODER.m_videoId,
							  nReadIndex,&pData,&nSize,&pStartTime,
							  &pDuration,&pRenderingOffset,&pIsSyncSample);

                p = pData;
			}
			if(nSize > 0){
				framesize = 0x0;
				framesize |= *pData<<24;
				framesize |= *(pData+1)<<16;
				framesize |= *(pData+2)<<8;
				framesize |= *(pData+3);
//            printf("nSize=%d\n",nSize);
//              printf("framesize=%x,nSize=%d\n",framesize,nSize);
				video_frame = nSize;
				nSize = nSize-framesize-4;
				pData = pData+4;

				if(nSize > 0)
					pData=pData+framesize;
			}
//			 printf("nReadIndex %d, framesize=%d,nSize=%d\n",nReadIndex,framesize,nSize);


		}
        frameLength = video_frame;

				if( FirstIdx == TRUE ){
					//memcpy((char *)bitstreamBuf->bufAddr,(char *)HEAD, 74);
					//memcpy((char *)bitstreamBuf->bufAddr+74,(char *)NAL, 4);
					//memcpy((char *)bitstreamBuf->bufAddr+78,(char *)pDatatemp, framesize);
                    fwrite(HEAD,1,74,pFile);
                    fwrite(NAL,1,4,pFile);
                    fwrite(pData,1,framesize,pFile);
					if( ( 78 + framesize) == (frameLength+74) ){
                           printf("%d \n",(frameLength+74));
					//	bitstreamBuf->fillLength = frameLength;
					}else{
						printf("write_error \n");
					}
					FirstIdx = FALSE;
					//bitstreamBuf->fillLength = fread(bitstreamBuf->bufAddr, 1U,
					//								 frameLength, pObj->fpDataStream[channelId]);
					//bitstreamBuf->fillLength = fread(bitstreamBuf->bufAddr, 1U,
					//								 frameLength, pObj->fpDataStream[channelId]);
				}else{
					//memcpy((char *)bitstreamBuf->bufAddr,(char *)NAL, 4);
					//memcpy((char *)bitstreamBuf->bufAddr+4,(char *)pDatatemp, framesize);
                    fwrite(NAL,1,4,pFile);
                    fwrite(pData,1,framesize,pFile);
					if( ( 4 + framesize) == frameLength){
                        printf("%d \n",(frameLength));
					//	bitstreamBuf->fillLength = frameLength;
					}else{
						printf("write_error \n");

					}

				}
//				printf( "framelength: %d  video_frame %d framesize %d \n", frameLength,video_frame, framesize);
                free(p);
                pData = NULL;
                p = NULL;
}






int main(int argc,char **argv){
    while(1){
          get_loop();

            get_h264();
    }


  //  openmp4file("cbbRec.mp4", &F_H264_MP4DECODER.hMp4file);

    //if(F_H264_MP4DECODER.m_videoId >= 0){
    //   get264stream(F_H264_MP4DECODER.hMp4file,F_H264_MP4DECODER.m_videoId,F_H264_MP4DECODER.VnumSamples,"test.h264");
 // int fget264stream(MP4FileHandle oMp4File,int VTrackId,int totalFrame)
   // }

    //closemp4file(&F_H264_MP4DECODER.hMp4file);
    return 0;
}

