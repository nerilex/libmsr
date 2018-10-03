/* msrio.c */
/*
 This file is part of libmsr.
 Copyright (C) 2018 bg nerilex (bg@nerilex.org)

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "msrio.h"
#include "serialio.h"
#include "usbio.h"
#include "libmsr.h"
#include <stdio.h>

#define MAX_SUPPORTED_DEVICES 32
#define ARRAY_ELEMENTS(a) (sizeof(a) / sizeof((a)[0]))

#if DEBUG
#define DBG_PRINTF(...) printf(__VA_ARGS__)
#else
#define DBG_PRINTF(...)
#endif
struct device_functions drivers[] = {
        {
                .open = msr_usb_open,
                .close = msr_usb_close,
                .commit = msr_usb_commit,
                .readchar = msr_usb_readchar,
                .read = msr_usb_read,
                .write = msr_usb_write,
        },
        {
                .open = msr_serial_open,
                .close = msr_serial_close,
                .commit = NULL,
                .readchar = msr_serial_readchar,
                .read = msr_serial_read,
                .write = msr_serial_write,
        }
};

struct device devices[MAX_SUPPORTED_DEVICES];

int msr_io_open(const char *path, int *dev, int blocking, speed_t speed)
{
    int r = LIBMSR_ERR_INTERFACE;
    int d;
    int fd;
    size_t i;

   DBG_PRINTF("DBG: %s(path %s, fd %p, blocking %d, speed %d) <%s %s %d>\n", __func__, path, (void*)dev, blocking, speed, __FILE__, __func__, __LINE__);
   for (d = 0; d < ARRAY_ELEMENTS(devices) && devices[d].func != NULL; ++d)
        ;
    if (d >= ARRAY_ELEMENTS(devices)) {
        /* all device slots occupied */
        DBG_PRINTF("DBG: Error: all device slots occupied <%s %s %d>\n", __FILE__, __func__, __LINE__);
        return r;
    }

    for (i = 0; i < ARRAY_ELEMENTS(drivers) && r != LIBMSR_ERR_OK; ++i) {
        r = drivers[i].open(path, &fd, blocking, speed);
    }

    if (r == LIBMSR_ERR_OK) {
        --i;
        devices[d].handle = fd;
        devices[d].func = &drivers[i];
        *dev = d;
        DBG_PRINTF("DBG: dev = %d; driver = %d\n", d, i);
    }
    DBG_PRINTF("DBG: %s() return : %d <%s %s %d>\n", __func__, r, __FILE__, __func__, __LINE__);
    return r;
}

#define XSTR(a) #a
#define STR(a) XSTR(a)

#define DELEGATE_A(name) \
        int ret = LIBMSR_ERR_INTERFACE;                                                    \
        if (dev >= 0 && dev < MAX_SUPPORTED_DEVICES) {                   \
            if (devices[dev].func != NULL) {                             \
                ret = 0;                                                 \
                if (devices[dev].func->name) {                         \
                    ret = devices[dev].func->name                      \
                        (devices[dev].handle);   \
                } else { DBG_PRINTF("DBG: %s not defined for device %d <%s %s %d>\n", STR(name), dev, __FILE__, __func__, __LINE__);}                                                       \
            }                                                            \
        }                                                                \

#define DELEGATE(name, ...) \
        int ret = -1;                                                    \
        if (dev >= 0 && dev < MAX_SUPPORTED_DEVICES) {                   \
            if (devices[dev].func != NULL) {                             \
                ret = 0;                                                 \
                if (devices[dev].func->name) {                         \
                    ret = devices[dev].func->name                      \
                        (devices[dev].handle , __VA_ARGS__);   \
                } else { DBG_PRINTF("DBG: %s not defined for device %d <%s %s %d>\n", STR(name), dev, __FILE__, __func__, __LINE__);}                                                       \
            }                                                            \
        }                                                                \

int msr_io_close(int dev)
{
    DBG_PRINTF("DBG: %s(device %d) <%s %s %d>\n", __func__, dev, __FILE__, __func__, __LINE__);
    DELEGATE_A(close)
    if (dev >= 0 && dev < MAX_SUPPORTED_DEVICES) {
        devices[dev].func = NULL;
        devices[dev].handle = 0;
    }
    return ret;
}

int msr_io_readchar(int dev, uint8_t *b)
{
    DBG_PRINTF("DBG: %s(device %d, b %p) <%s %s %d>\n", __func__, dev, b, __FILE__, __func__, __LINE__);
    DELEGATE(readchar, b)
    return ret;
}

int msr_io_write(int dev, const void *data, size_t size)
{
    DBG_PRINTF("DBG: %s(device %d, data %p, size %zu) <%s %s %d>\n", __func__, dev, data, size, __FILE__, __func__, __LINE__);
    DELEGATE(write, data, size)
    return ret;
}
int msr_io_read(int dev, void *data, size_t size)
{
    DBG_PRINTF("DBG: %s(device %d, data %p, size %zu) <%s %s %d>\n", __func__, dev, data, size, __FILE__, __func__, __LINE__);
    DELEGATE(read, data, size)
    return ret;
}

int msr_io_commit(int dev)
{
    DBG_PRINTF("DBG: %s(device %d) <%s %s %d>\n", __func__, dev, __FILE__, __func__, __LINE__);
    DELEGATE_A(commit)
    return ret;
}
