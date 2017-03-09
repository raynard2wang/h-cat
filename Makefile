#
TOPDIR=$(shell pwd)

#CROSS_PLATFORM=$(SDKROOT)/bin/arm-linux-

CC=$(CROSS_PLATFORM)gcc
LD=$(CROSS_PLATFORM)ld
AR=$(CROSS_PLATFORM)ar
aS=$(CROSS_PLATFORM)as

#flags
CFLAGS=-I$(SYSROOT)/usr/include
LDFLAGS=-L$(SYSROOT)/lib

CFLAGS+=-I$(TOPDIR)
CFLAGS+= -O3 -Wall -fmessage-length=0


#src files 
SRCS= $(wildcard *.c )
OBJS=$(patsubst %.c,%.o,$(SRCS))

#TARGET=val_v0063
TARGET=h-cat

all: $(OBJS)
	echo $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) --sysroot=$(SYSROOT) -o $(TARGET)
	#test -f $(TARGET).tar.gz && rm -fr $(TARGET).tar.gz 
	#rm -fr $(TARGET).tar.gz 
	#test -f $(TARGET) && ./file_patch $(TARGET)
	test -f $(TARGET) && tar czvf $(TARGET).tar.gz $(TARGET)

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@	

clean:
	rm -fr $(OBJS) 
	rm -f $(TARGET)
	rm -f $(TARGET).tar.gz 

.PHONY:clean release
