#
##
#

PROG=crt
SOURCES=crt.c vector.c objects.c
OBJS=$(SOURCES:.c=.o)

AS=as
CPP=cpp
CC=cc

ASFLAGS=
CFLAGS=-Wall -Werror -ggdb
LDFLAGS=-L/usr/local/lib -rdynamic -lm -lexpat

$(PROG): $(SOURCES) outputs inputs
	$(CC) $(CFLAGS) $(SOURCES) -o $(PROG) $(LDFLAGS)

outputs: force_look
	make -C outputs

inputs: force_look
	make -C inputs

clean:
	rm -f *.o $(PROG)
	make clean -C outputs
	make clean -C inputs

force_look:
	true
