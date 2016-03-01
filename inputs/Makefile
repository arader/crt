#
##
#

AS=as
CPP=cpp
CC=cc

ASFLAGS=
CFLAGS=-Wall -Werror -ggdb -I/usr/local/include -fPIC
LDFLAGS=-ldl

xml.so: xml.c
	$(CC) $(CFLAGS) -shared -o xml.so xml.c

clean:
	rm -f *.so
