LIBS=`pkg-config --libs glib-2.0 libusb`
INCS=`pkg-config --cflags glib-2.0 libusb`

.c.o:
	$(CC) -g -Wall $(CFLAGS) $(INCS) -c $*.c

all: loader

loader: loader.o
	$(CC) -g -Wall $(CFLAGS) -o $@ loader.o $(LIBS)

clean:
	rm -fr *.o loader
