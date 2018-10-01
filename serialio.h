/* serialio.h */
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

#ifndef SERIALIO_H_
#define SERIALIO_H_

#include <termios.h>
#include <stdint.h>
#include <stddef.h>

/**
 * @brief Open a serial connection to the MSR device.
 *
 * @param path The path to the serial device.
 * @param fd The int pointer to store the file descriptor in.
 * @param blocking The blocking flag (e.g., ::MSR_BLOCKING)
 * @param baud The baud rate of the serial device (e.g., ::MSR_BAUD)
 * @return ::LIBMSR_ERR_OK on success
 * @return ::LIBMSR_ERR_SERIAL on failure
 */
extern int msr_serial_open(const char *path, int *fd, int blocking, speed_t baud);

/**
 * @brief Close a serial connection to the MSR device.
 *
 * @param fd The file descriptor to close.
 * @return ::LIBMSR_ERR_OK
 */
extern int msr_serial_close(int fd);

/**
 * @brief Read a single character from the MSR device.
 *
 * @param fd The file descriptor to read from.
 * @param c A pointer to write the character into.
 *
 * @return The character read
 */
extern int msr_serial_readchar(int fd, uint8_t *c);

/**
 * @brief Write a series of bytes to the MSR device.
 *
 * @param fd The file descriptor to write to.
 * @param buf The buffer to write.
 * @param len The length of the buffer.
 * @return The number of bytes written, or -1 on error.
 */
extern int msr_serial_write(int fd, const void *buf, size_t len);

/**
 * @brief Read a series of bytes from the MSR device.
 *
 * @param fd The file descriptor to read from.
 * @param buf The buffer to read into.
 * @param len The length of the buffer.
 * @return ::LIBMSR_ERR_OK
 */
extern int msr_serial_read(int fd, void *buf, size_t len);

#endif /* SERIALIO_H_ */
