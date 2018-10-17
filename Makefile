# set the compile chain
#CC = gcc -g
#CCX = g++ -g
#CFLAGS = -Wall


#for Arm linux
CC = arm-linux-gnueabihf-gcc -g
CCX = arm-linux-gnueabihf-g++ -g
#CFLAGS = -Wall
CFLAGS = -D_GNU_SOURCE -mfpu=neon -fPIC -fno-strict-aliasing -fno-optimize-sibling-calls -Wall -Wno-missing-braces

#target
TARGET=mp4cencode

# h file seach path
INC = -I. -I./include/mp4v2 -I./include

# lib file and path
DLIBS = -lmp4v2
LDFLAGS = -L ./lib

# running lib path
RPATH = -Wl,-rpath=./lib_arm

# SRC
SRCS = main.c MP4Encoder.c
# Obj file
OBJS = $(SRCS:.c=.o)
#OBJS = main.o MP4Encoder.o
$(TARGET):$(OBJS)
#	$(CC) -o $@ $^ $(LDFLAGS) $(DLIBS) $(RPATH)
	$(CCX) -o $@ $^ $(LDFLAGS) $(DLIBS) $(RPATH)
	
clean:
	rm -rf $(TARGET) $(OBJS)
	

# continue :1 clean 2 compile 3.run
exec:clean $(TARGET)
	@echo  run the app
	./$(TARGET)
	@echo  finish!!!
# compile rules
%.o:%.c
	$(CC) $(CFLAGS) $(INC) -o $@ -c $<
		
	

