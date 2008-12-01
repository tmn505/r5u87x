# For build target
LIBS=`pkg-config --libs glib-2.0 libusb`
INCS=`pkg-config --cflags glib-2.0 libusb`

# For install target
FIRMWARE="r5u87x-05ca-1803.fw" "r5u87x-05ca-1810.fw" "r5u87x-05ca-1812.fw" "r5u87x-05ca-1830.fw" "r5u87x-05ca-1832.fw" "r5u87x-05ca-1833.fw" "r5u87x-05ca-1834.fw" "r5u87x-05ca-1835.fw" "r5u87x-05ca-1836.fw" "r5u87x-05ca-1837.fw" "r5u87x-05ca-1839.fw" "r5u87x-05ca-183a.fw" "r5u87x-05ca-183b.fw" "r5u87x-05ca-183e.fw" "r5u87x-05ca-1841.fw" "r5u87x-05ca-1870_1.fw" "r5u87x-05ca-1870.fw"
INSTALL=install
DESTDIR=/usr
bindir=/bin
libdir=/lib

.c.o:
	$(CC) -g -Wall -DHAVE_CONFIG_H $(CFLAGS) $(INCS) -c $*.c $*.h

all: loader

loader: loader.o
	$(CC) -g -Wall $(CFLAGS) -o $@ loader.o $(LIBS)

clean:
	rm -fr *.o loader
	rm -fr *.gch loader
	#rm *~

install:	all
	./mkinstalldirs $(DESTDIR)$(bindir)
	$(INSTALL) -m 0755 loader $(DESTDIR)$(bindir)/r5u87x-loader
	./mkinstalldirs $(DESTDIR)$(libdir)/r5u87x/ucode
	@for fw in $(FIRMWARE); do \
	    echo "$(INSTALL) -m 0644 ucode/$$fw $(DESTDIR)$(libdir)/r5u87x/ucode/$$fw" ; \
	    $(INSTALL) -m 0666 ucode/$$fw $(DESTDIR)$(libdir)/r5u87x/ucode/$$fw || exit 1 ; \
    done

uninstall:
	rm -fv $(DESTDIR)$(bindir)/r5u87x-loader
	rm -rfv $(DESTDIR)$(libdir)/r5u87x
