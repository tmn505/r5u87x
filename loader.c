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
#include <glib.h>
#include <glib/gstdio.h>
#include <usb.h>

#include "loader.h"

static gchar    *firmware   = "r5u87x-%vid%-%pid%.fw";

static GOptionEntry entries[] = {
    { "firmware", 'f', 0,
        G_OPTION_ARG_FILENAME, &firmware,
        "Path to the firmware.", NULL
    },
};

gchar *
replace_str(gchar *str, gchar *orig, gchar *rep)
{
  static char buffer[4096];
  char *p;

  if(!(p = strstr(str, orig)))  // Is 'orig' even in 'str'?
    return str;

  strncpy(buffer, str, p-str); // Copy characters from 'str' start to 'orig' st$
  buffer[p-str] = '\0';

  sprintf(buffer+(p-str), "%s%s", rep, p+strlen(orig));

  return buffer;
}

struct usb_device *
find_device (const struct device_info devices[]) {
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
                    
                    return dev;
                }
            }
        }
    }
    
    return NULL;
}

gint
load_firmware (struct usb_device *dev, gchar* firmware_tmpl) {
    gchar* pid_str;
    gchar* vid_str;
    gchar* fw_name;

    // Create string representations of our USB IDs
    sprintf (pid_str, "%04x", dev->descriptor.idProduct);
    sprintf (vid_str, "%04x", dev->descriptor.idVendor);
    
    // Convert the template firmware name into one we're going to use for this
    // device.
    fw_name = replace_str (firmware_tmpl, "%pid%", pid_str);
    fw_name = replace_str (fw_name, "%vid%", vid_str);
    
    loader_msg ("Found camera       : %04x:%04x.\n", dev->descriptor.idVendor, dev->descriptor.idProduct);
    loader_msg ("Using firmware file: %s\n", fw_name);
    return -1;
}

gint
main (gint argc, gchar *argv []) {
    struct usb_device *dev;
    
    usb_init ();
    if (usb_find_busses () == 0) {
        loader_error ("Failed to find any USB busses.\n");
    } else if (usb_find_devices () == 0) {
        loader_error ("Failed to find any USB devices.\n");
    }
    
    dev = find_device (device_table);
    
    if (dev == NULL) {
        loader_error ("Failed to find any supported webcams.\n");
    }
    
    int res = load_firmware (dev, firmware);
    if (res < 0) {
        loader_error ("Failed to upload firmware to device successfully. Error: %d\n", res);
    } else {
        loader_msg ("Successfully uploaded firmware to device %x:%x!\n", dev->descriptor.idVendor, dev->descriptor.idProduct);
    }
    
    return 0;
}
