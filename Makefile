# For build target -----------------------------------------------------------|
LIBS=`pkg-config --libs glib-2.0 libusb`
INCS=`pkg-config --cflags glib-2.0 libusb`

# For install target ---------------------------------------------------------|
# App to run to install files, usually in path; set otherwise
INSTALL=install

# FW data
FIRMWARE_NAMESPEC="r5u87x-%vid%-%pid%.fw"
FIRMWARE=`ls ucode | xargs`

# Install names
LOADER_INSTALL=r5u87x-loader
UDEV_INSTALL=/etc/udev/rules.d/

# Directories
PREFIX=/usr
INSTALL_PATH=$(DESTDIR)$(PREFIX)
sbindir=/sbin
libdir=/lib
FIRMWARE_DIR=$(INSTALL_PATH)$(libdir)/r5u87x/ucode

# For rules and make targets -------------------------------------------------|
RULESFILE=contrib/90-r5u87x-loader.rules

# Automake targets -----------------------------------------------------------|

.c.o:
	$(CC) -g -Wall -DHAVE_CONFIG_H -DUCODE_PATH=\"$(FIRMWARE_DIR)/$(FIRMWARE_NAMESPEC)\" $(CFLAGS) $(INCS) -c $*.c $*.h

all: loader

loader: loader.o
	$(CC) -g -Wall $(CFLAGS) -o $@ loader.o $(LIBS)

clean:
	rm -fr *.o loader
	rm -fr *.gch loader
	if [ -f $(RULESFILE) ]; then \
	    rm -f $(RULESFILE); \
    fi

install: all
	$(INSTALL) -d $(INSTALL_PATH)$(bindir)
	$(INSTALL) -m 0755 loader $(INSTALL_PATH)$(sbindir)/$(LOADER_INSTALL)
	$(INSTALL) -d $(FIRMWARE_DIR)
	@for fw in $(FIRMWARE); do \
		echo "$(INSTALL) -m 0644 ucode/$$fw $(FIRMWARE_DIR)/$$fw" ; \
		$(INSTALL) -m 0644 ucode/$$fw $(FIRMWARE_DIR)/$$fw || exit 1 ; \
	done
	
	## If we have the rules file generated, install it while we're here
	if [ -f $(RULESFILE) ]; then \
		$(INSTALL) -m 0644 $(RULESFILE) $(UDEV_INSTALL); \
	fi

uninstall:
	rm -fv $(INSTALL_PATH)$(sbindir)/$(LOADER_INSTALL)
	rm -rfv $(INSTALL_PATH)$(libdir)/r5u87x

$(RULESFILE):
	cat $(RULESFILE).in | awk 'BEGIN{P=1;}/^###BEGINTEMPLATE###/{P=0;} {if (P) print;}' | grep -v '^###' >$@
	for sedline in `ls ucode | sed 's/^r5u87x-\([0-9a-zA-Z]\+\)-\([0-9a-zA-Z]\+\)\.fw$$/s\/#VENDORID#\/\1\/g;s\/#PRODUCTID#\/\2\/g/p;d'`; do \
		cat $(RULESFILE).in | awk 'BEGIN{P=0;}/^###BEGINTEMPLATE###/{P=1;}/^###ENDTEMPLATE###/{P=0;} {if (P) print;}' | grep -v '^###' | sed "$$sedline" >>$@; \
		done >>$@
	cat $(RULESFILE).in | awk 'BEGIN{P=0;}/^###ENDTEMPLATE###/{P=1;} {if (P) print;}' | grep -v '^###' >>$@

rules: $(RULESFILE)
