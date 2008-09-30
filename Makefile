LIBS=`pkg-config --libs glib-2.0 libusb`
INCS=`pkg-config --cflags glib-2.0 libusb`

.c.o:   loader.h
	$(CC) -g -Wall $(CFLAGS) $(INCS) -c $*.c $*.h

all: loader

loader: loader.o
	$(CC) -g -Wall $(CFLAGS) -o $@ loader.o $(LIBS)

clean:
	rm -fr *.o loader
