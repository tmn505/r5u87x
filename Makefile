# For build target -----------------------------------------------------------|
LIBS=`pkg-config --libs glib-2.0 libusb`
INCS=`pkg-config --cflags glib-2.0 libusb`

# For install target ---------------------------------------------------------|
# App to run to install files, usually in path; set otherwise
INSTALL=install

# FW data
FIRMWARE="r5u87x-05ca-1803.fw" "r5u87x-05ca-1810.fw" "r5u87x-05ca-1812.fw" "r5u87x-05ca-1830.fw" "r5u87x-05ca-1832.fw" "r5u87x-05ca-1833.fw" "r5u87x-05ca-1834.fw" "r5u87x-05ca-1835.fw" "r5u87x-05ca-1836.fw" "r5u87x-05ca-1837.fw" "r5u87x-05ca-1839.fw" "r5u87x-05ca-183a.fw" "r5u87x-05ca-183b.fw" "r5u87x-05ca-183e.fw" "r5u87x-05ca-1841.fw" "r5u87x-05ca-1870_1.fw" "r5u87x-05ca-1870.fw"

# Modification to filenames
LOADER_INSTALL=r5u87x-loader

# Directories
PREFIX=/usr
INSTALL_PATH=$(DESTDIR)/$(PREFIX)
bindir=/bin
libdir=/lib
FIRMWARE_DIR=$(INSTALL_PATH)$(libdir)/r5u87x/ucode/

# Automake targets -----------------------------------------------------------|

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
	./mkinstalldirs $(INSTALL_PATH)$(bindir)
	$(INSTALL) -m 0755 loader $(INSTALL_PATH)$(bindir)/$(LOADER_INSTALL)
	./mkinstalldirs $(FIRMWARE_DIR)
	@for fw in $(FIRMWARE); do \
	    echo "$(INSTALL) -m 0666 ucode/$$fw $(FIRMWARE_DIR)$$fw" ; \
	    $(INSTALL) -m 0666 ucode/$$fw $(FIRMWARE_DIR)$$fw || exit 1 ; \
    done

uninstall:
	rm -fv $(INSTALL_PATH)$(bindir)/$(LOADER_INSTALL)
	rm -rfv $(INSTALL_PATH)$(libdir)/r5u87x
