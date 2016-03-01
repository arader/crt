#
##
#

AS=as
CPP=cpp
CC=cc

ASFLAGS=
CFLAGS=-Wall -Werror -ggdb -fPIC
LDFLAGS=-ldl

bmp.so: bmp.c
	$(CC) $(CFLAGS) -shared -o bmp.so bmp.c

clean:
	rm -f *.so
