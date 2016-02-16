CC = gcc
CFLAGS = -D DEBUG -std=gnu11 -g

PROG = JALACompiler
SRCS = JALACompiler.c Stack.c StringOps.c StringList.c
HDRS = Stack.h StringOps.h StringList.h
OBJS = $(SRCS:.c=.o)

$(PROG) : $(OBJS)
	$(CC) $(OBJS) -o $(PROG)

JALACompiler.o : JALACompiler.c $(HDRS)
	$(CC) $(CFLAGS) -c JALACompiler.c

Stack.o : Stack.c Stack.h
	$(CC) $(CFLAGS) -c Stack.c

StringOps.o : StringOps.c StringOps.h
	$(CC) $(CFLAGS) -c StringOps.c

StringList.o : StringList.c StringList.h
	$(CC) $(CFLAGS) -c StringList.c
