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

#ifndef _USBIO_H_
#define _USBIO_H_

#include <termios.h>
#include <stdint.h>
#include <stddef.h>

/**
 * @brief Open a USB connection to the MSR device.
 *
 * @param path The path is currently ignored but might be used to indicate
 *             a specific device in the futuree.
 * @param fd The int pointer to store a file descriptor in.
 * @param blocking The blocking flag is currently ignored
 * @param baud The baud rate is currently ignored
 * @return ::LIBMSR_ERR_OK on success
 * @return ::LIBMSR_ERR_SERIAL on failure
 */
extern int msr_usb_open(const char *path, int *fd, int blocking, speed_t baud);

/**
 * @brief Close a USB connection to the MSR device.
 *
 * @param fd The file descriptor to close.
 * @return ::LIBMSR_ERR_OK
 */
extern int msr_usb_close(int fd);


/**
 * @brief Send bufferd command data to the MSR device.
 *
 * @param fd The file descriptor for the device.
 * @return ::LIBMSR_ERR_OK
 */
extern int msr_usb_commit(int fd);

/**
 * @brief Read a single character from the MSR device.
 *
 * @param fd The file descriptor for the device.
 * @param c A pointer to write the character into.
 *
 * @return The character read
 */
extern int msr_usb_readchar(int fd, uint8_t *c);

/**
 * @brief Write a series of bytes to the MSR device.
 *
 * @param fd The file descriptor for the device.
 * @param buf The buffer to write.
 * @param len The length of the buffer.
 * @return The number of bytes written, or -1 on error.
 */
extern int msr_usb_write(int fd, const void *buf, size_t len);

/**
 * @brief Read a series of bytes from the MSR device.
 *
 * @param fd The file descriptor for the device.
 * @param buf The buffer to read into.
 * @param len The length of the buffer.
 * @return ::LIBMSR_ERR_OK
 */
extern int msr_usb_read(int fd, void *buf, size_t len);

#endif /* _USBIO_H_ */
