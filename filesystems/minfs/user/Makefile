CFLAGS = -Wall -g 
LDFLAGS = -static 

.PHONY: all clean

all: mkfs.minfs

mkfs.minfs: mkfs.minfs.o

mkfs.minfs.o: mkfs.minfs.c ../kernel/minfs.h

clean:
	-rm -f *~ *.o mkfs.minfs
