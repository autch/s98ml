
CFLAGS = -g -Wall

s98d: s98d.o s98d_device.o s98d_header.o s98d_dump.o

clean:
	rm -f *.o

