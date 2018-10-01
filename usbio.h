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

#include <termio.h>
#include <stddef.h>

extern int msr_usb_open (const char *, int *,  int, speed_t);
extern int msr_usb_close (int);
extern int msr_usb_readchar (int, uint8_t *);
extern int msr_usb_write (int, const void *, size_t);
extern int msr_usb_read (int, void *, size_t);
extern int msr_usb_commit (int);


#endif /* _USBIO_H_ */
