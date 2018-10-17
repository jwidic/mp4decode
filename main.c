#include <stdio.h>
#include <stdlib.h>
#include "MP4Encoder.h"


int main()
{
 MP4Encoder_init();
 printf("Convert opening !!! \n");
	// convert H264 file to mp4 file
  WriteH264File("test.264", "test.mp4");
  printf("Convert completed !!! \n");
return 0;
}
