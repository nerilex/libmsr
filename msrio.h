/* msrio.h */
/*
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

#ifndef MSRIO_H_
#define MSRIO_H_

#include <termios.h>
#include <stdint.h>
#include <stddef.h>

typedef int dev_handle_t;

typedef int(*device_open_fpt)(const char* path, dev_handle_t *dev, int blocking, speed_t speed);
typedef int(*device_close_fpt)(dev_handle_t dev);
typedef int(*device_commit_fpt)(dev_handle_t dev);
typedef int(*device_readchar_fpt)(dev_handle_t dev, uint8_t *b);
typedef int(*device_read_fpt)(dev_handle_t dev, void *data, size_t size);
typedef int(*device_write_fpt)(dev_handle_t dev, const void *data, size_t size);

extern int msr_io_open (const char *path, int *dev, int blocking, speed_t speed);
extern int msr_io_close (int dev);
extern int msr_io_readchar (int dev, uint8_t *b);
extern int msr_io_write (int dev, const void *data, size_t size);
extern int msr_io_read (int dev, void *data, size_t size);
extern int msr_io_commit (int dev);

struct device_functions {
    device_open_fpt open;
    device_close_fpt close;
    device_commit_fpt commit;
    device_readchar_fpt readchar;
    device_read_fpt read;
    device_write_fpt write;
};

struct device {
    dev_handle_t handle;
    struct device_functions *func;
};


#endif /* MSRIO_H_ */
