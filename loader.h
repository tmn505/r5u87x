/*
 * Ricoh 5U87x series USB firmware loader.
 * Copyright (c) 2008 Alexander Hixon <alex@alexhixon.com>
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

#ifndef _LOADER_H_
#define _LOADER_H_

#define TIMEOUT 1000

#define loader_msg(args...)  printf (args)
#define loader_warn(args...)  printf (args)
#define loader_error(args...)  ({printf (args); exit (-1); })

struct device_info {
    int usb_vendor;
    int usb_product;
};

struct usb_device * find_device (const struct device_info devices[]);

/*
 * Please try to keep this in sync with docs/model_matrix.txt!
 */
static const struct device_info device_table[] = {
    { 0x05ca, 0x183a },
    { }
};

#endif
