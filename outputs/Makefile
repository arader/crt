#
##
#

AS=as
CPP=cpp
CC=gcc

ASFLAGS=
CFLAGS=-Wall -Werror -ggdb
LDFLAGS=-ldl

bmp.so: bmp.c
	$(CC) $(CFLAGS) -shared -o bmp.so bmp.c

clean:
	rm -f *.so
