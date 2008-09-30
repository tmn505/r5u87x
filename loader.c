/*
 * Ricoh R5U87x series USB firmware loader.
 * Copyright (c) 2008 Alexander Hixon <alex@alexhixon.com>
 * 
 * Loading routines based off those used in the original r5u870 kernel
 * driver, written by Sam Revitch <samr7@cs.washington.edu>. See README
 * for additional credits.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA
 */

#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <usb.h>

#include "loader.h"

static gchar    *firmware       = "ucode/r5u87x-%vid%-%pid%.fw";
static gboolean force_clear     = FALSE;
static gboolean no_load         = FALSE;

static GOptionEntry entries[] = {
    { "firmware", 'f', 0,
        G_OPTION_ARG_FILENAME, &firmware,
        "Path to microcode. %vid% and %pid% are substituted in.", NULL
    },
    { "force-clear", 0, 0,
        G_OPTION_ARG_NONE, &force_clear,
        "Forcefully clear the device before uploading microcode.", NULL
    },
    { "pretend", 0, 0,
        G_OPTION_ARG_NONE, &no_load,
        "Don't actually load any microcode on to the device.", NULL
    },
};

gchar *
replace_str(gchar *str, gchar *orig, gchar *rep)
{
  static char buffer[4096];
  char *p;

  if(!(p = strstr(str, orig)))  // Is 'orig' even in 'str'?
    return str;

  strncpy(buffer, str, p-str); // Copy characters from 'str' start to 'orig'
  buffer[p-str] = '\0';

  sprintf(buffer+(p-str), "%s%s", rep, p+strlen(orig));

  return buffer;
}

struct usb_device *
find_device (const struct device_info devices[], gint *version) {
    gint i;
    struct usb_bus *busses;
    struct usb_bus *bus;
    
    busses = usb_get_busses();
    for (bus = busses; bus; bus = bus->next) {
        struct usb_device *dev;
        
        for (dev = bus->devices; dev; dev = dev->next) {
            for (i = 0; i < sizeof (devices); i++) {
                if (dev->descriptor.idVendor == devices[i].usb_vendor &&
                    dev->descriptor.idProduct == devices[i].usb_product) {
                    
                    *version = devices[i].ucode_version;
                    return dev;
                }
            }
        }
    }
    
    return NULL;
}

gint
r5u87x_ucode_upload (gint firmware, struct usb_dev_handle *handle, gint size) {
    gint length, remaining, address, index, res, requesttype = 0;
    unsigned char header[3], payload[1024];
    
    index = 0;
    remaining = size;
    
    loader_msg ("Sending microcode to camera...\n");
    
    while (remaining > 0) {
        if (remaining < 3) {
            loader_warn ("Microcode file is incomplete; message %d\n", index);
            return -1;
        }
        
        // load in packet header
        if ((length = read (firmware, header, 3)) != 3) {
            if (length == 0) {
                return 0;
            } else {
                loader_warn ("Reading firmware header chunk failed\n");
                return -2;
            }
        }
        
        length = header[0];
        address = header[1] | (header[2] << 8);
        remaining -= 3;
        
        // Read the payload
        if (length < 0 || length >= 1024) {
            loader_warn ("Bad payload length %d (%i).\n", length, index);
            return -3;
        } else if (read (firmware, payload, length) != length) {
            loader_warn ("Failed to read firmware data.\n");
        }
        
        if (no_load == FALSE) {
            res = usb_control_msg (handle, USB_SEND, 0xa0, address, 0,
                payload, length, TIMEOUT);
            
            if (res < 0) {
                loader_warn ("Command failed. Msg: %d; res: %d\n", index, res);
                return res;
            }
            
            if (res != length) {
                loader_warn ("Command failed. Msg: %d; res: %d; exp: %d\n", 
                    index, res, length);
                return res;
            }
        }
        
        remaining -= length;
        index++;
    }
    
    return 0;
}

r5u87x_ucode_status (struct usb_dev_handle *handle) {
    gchar buf[1];
    gint res;
    
    res = usb_control_msg (handle, USB_RECV, 0xa4, 0, 0, buf, 1, TIMEOUT);
    if (res != 1) {
        loader_warn ("Failed to get microcode status.\n");
        return res;
    }
    
    loader_msg ("Camera reports %s microcode state.\n",
        buf[0] == 1 ? "positive" : "negative");
    
    return buf[0] == 1 ? TRUE : FALSE;
}

r5u87x_ucode_version (struct usb_dev_handle *handle, gint *version) {
    gchar buf[2];
    gint res;
    
    res = usb_control_msg (handle, USB_RECV, 0xc3, 0, 0x0e, buf, 2, TIMEOUT);
    if (res != 2) {
        loader_warn ("Failed to get microcode version.\n");
        return res;
    }
    
    // FIXME: This is kinda bad. What about endianness?
    res = buf[1] | (buf[0] << 4);
    
    loader_msg ("Camera reports microcode version 0x%04x.\n", res);
    *version = res;
    
    return 0;
}

gint
r5u87x_ucode_enable (struct usb_dev_handle *handle) {
    gchar buf[1];
    gint res;
    buf[0] = 0;
    
    res = usb_control_msg (handle, USB_SEND, 0xa1, 0, 0, buf, 1, TIMEOUT);
    if (res != 1) {
        loader_warn ("Failed to enable microcode.\n");
        return res;
    }
    
    loader_msg ("Enabled microcode.\n");
    
    return 0;
}

gint
r5u87x_ucode_clear (struct usb_dev_handle *handle) {
    gchar buf[1];
    gint res;
    buf[0] = 1;
    res = usb_control_msg (handle, USB_SEND, 0xa6, 0, 0, buf, 1, TIMEOUT);
    if (!res) {
        sleep (200);
    }
    
    if (res < 0) {
        loader_warn ("Failed to reset microcode.\n");
        return res;
    }
    
    loader_msg ("Reset microcode.\n");
    
    return res;
}

gint
load_firmware (struct usb_device *dev, gchar* firmware_tmpl,
    const gint ucode_version) {
    
    gchar* fw_name;
    
    gint fd, res, dev_version;
    usb_dev_handle *handle;
    struct stat infobuf;

    dev_version = 0;
    
    // Convert the template firmware name into one we're going to use for this
    // device.
    fw_name = replace_str (firmware_tmpl, "%pid%",
        g_strdup_printf ("%04x", dev->descriptor.idProduct));
    fw_name = replace_str (fw_name, "%vid%",
        g_strdup_printf ("%04x", dev->descriptor.idVendor));
    
    loader_msg ("Found camera   : %04x:%04x\n", dev->descriptor.idVendor,
        dev->descriptor.idProduct);
    loader_msg ("Firmware       : %s\n\n", fw_name);
    
    // Open the firmware file
    if ((fd = g_open (fw_name, O_RDONLY)) == -1) {
        loader_error ("Failed to open %s. Does it exist?\n", fw_name);
    }
    
    // Possibly not the best way to do this, but hey, it's certainly easy
    // (without loading everything into memory, and compared to seeking around)
    if (stat (fw_name, &infobuf) == -1) {
        loader_error ("Failed to get filesize of firmware.\n");
    }
    
    // Try the USB device too.
    if (!(handle = usb_open (dev))) {
        loader_warn ("Failed to open USB device.\n");
    }
    
    // Force reset if asked.
    if (force_clear) {
        res = r5u87x_ucode_clear (handle);
        if (res < 0) {
            return res;
        }
    }
    
    // Check to see if there's already stuff on there.
    res = r5u87x_ucode_status (handle);
    if (res < 0) {
        return res;
    } else if (res == 1) {
        // Microcode already uploaded.
        res = r5u87x_ucode_version (handle, &dev_version);
        if (res < 0) {
            return res;
        } else if (dev_version != ucode_version) {
            // Clear it out - ucode version and device version don't match.
            res = r5u87x_ucode_clear (handle);
            if (res < 0) {
                return res;
            }
            
            res = r5u87x_ucode_status (handle);
            if (res < 0) {
                return res;
            } else if (res == 1) {
                loader_warn ("Camera still has microcode even though we"
                    "cleared it.\n");
            }
        } else {
            // Camera already has microcode - it's all good.
            loader_msg ("Not doing anything - camera already setup.\n");
            return 0;
        }
    }
    
    // Upload the microcode!
    res = r5u87x_ucode_upload (fd, handle, infobuf.st_size);
    if (res < 0) {
        return res;
    }
    
    // Enable it after we do a successful upload.
    if (!no_load) {
        res = r5u87x_ucode_enable (handle);
        if (res < 0) {
            return res;
        }
        
        // Check versions to see if all is OK.
        dev_version = 0;
        res = r5u87x_ucode_version (handle, &dev_version);
        if (res < 0) {
            return res;
        } else if (dev_version != ucode_version) {
            loader_warn ("Camera returned unexpected ucode version 0x%04x - "
                "expected 0x%04x\n", dev_version, ucode_version);
            return -EPROTO;
        }
    } else {
        loader_warn ("Skipping enabling of microcode and version checks; in "
            "pretend mode.\n");
    }
    
    return 0;
}

gint
main (gint argc, gchar *argv []) {
    GOptionContext *context;
    GError *error = NULL;
    struct usb_device *dev;
    gint version = 0;
    
    g_set_prgname ("loader");
    context = g_option_context_new("- Ricoh R5U87x series firmware loader");
    g_option_context_add_main_entries (context, entries, NULL);
    
    if (!g_option_context_parse(context, &argc, &argv, &error)) {
        loader_error ("%s\n", error->message);
    }
    
    usb_init ();
    if (usb_find_busses () == 0) {
        loader_error ("Failed to find any USB busses.\n");
    } else if (usb_find_devices () == 0) {
        loader_error ("Failed to find any USB devices.\n");
    }
    
    loader_msg ("Searching for device...\n");
    
    dev = find_device (device_table, &version);
    
    if (dev == NULL) {
        loader_error ("Failed to find any supported webcams.\n");
    }
    
    int res = load_firmware (dev, firmware, version);
    if (res < 0) {
        loader_error ("Failed to upload firmware to device: %s (code %d).\n",
            strerror (errno), res);
    } else {
        loader_msg ("\nSuccessfully uploaded firmware to device %04x:%04x!\n",
            dev->descriptor.idVendor, dev->descriptor.idProduct);
    }
    
    return 0;
}
