#
##
#

AS=as
CPP=cpp
CC=gcc

ASFLAGS=
CFLAGS=-Wall -Werror -ggdb
LDFLAGS=-ldl

xml.so: xml.c
	$(CC) $(CFLAGS) -shared -o xml.so xml.c

clean:
	rm -f *.so
