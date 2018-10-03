#include <sys/types.h>
#include <sys/fcntl.h>

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <termios.h>
#include <err.h>
#include <string.h>

#include <libusb-1.0/libusb.h>
#include "libmsr.h"
#include "usbio.h"

#define MAX_SUPPORTED_DEVICES 32
#define ARRAY_ELEMENTS(a) (sizeof(a) / sizeof((a)[0]))

#define IF_DEBUG(x)

struct {
    uint16_t vid, pid;
} supported_devices[] = {
        {.vid = 0x0801, .pid = 0x0003} /* officially assigned ID */
};

libusb_device_handle *devh[MAX_SUPPORTED_DEVICES];

static
libusb_device_handle *open_device(void)
{
    size_t i;
    libusb_device_handle *ret = NULL;
    for (i = 0; ret == NULL &&
                i < sizeof(supported_devices) / sizeof(supported_devices[0]);
         ++i) {
        ret = libusb_open_device_with_vid_pid(NULL, supported_devices[i].vid,
                                              supported_devices[i].pid);
    }
    return ret;
}

#define USB_MAX_PACKET_SIZE 64
#define USB_MAX_PAYLOAD_SIZE 63

struct ring_buf {
    uint8_t *buf;
    size_t rx_index; /* buf[rx_index] is the next byte to return on read */
    size_t tx_index; /* buf[tx_index] is the next byte to write on append */
                     /* rx_index == tx_index when buffer is empty */
                     /* rx_index == (tx_index + 1) % size when buffer is full */
   size_t size;
};



#define BUFFER_FULL(rb) ((rb)->rx_index == ((rb)->tx_index + 1) % (rb)->size)
#define BUFFER_EMPTY(rb) ((rb)->rx_index == ((rb)->tx_index))
#define BUFFER_FILL(rb) (((rb)->tx_index - ((rb)->rx_index + (rb)->size)) % (rb)->size)
#define BUFFER_CLEAR(rb) do {(rb)->rx_index = (rb)->tx_index = 0; } while(0)

static
struct usb_if_state {
    struct ring_buf tx;
    struct ring_buf rx;
    uint8_t tx_buf_data [2 * (USB_MAX_PAYLOAD_SIZE + 1)];
    uint8_t rx_buf_data [USB_MAX_PAYLOAD_SIZE + 1];
} usb_if_state[MAX_SUPPORTED_DEVICES];

static int active_devices;

static
size_t ring_buf_append(struct ring_buf *rb, const void *data, size_t len) {
    size_t ret = 0;
    const uint8_t *d = (const uint8_t*)data;
    while (!BUFFER_FULL(rb) && len > ret) {
        rb->buf[rb->tx_index] = *d++;
        rb->tx_index = (rb->tx_index + 1) % rb->size;
        ++ret;
    }
    return ret;
}

static
size_t ring_buf_fetch(struct ring_buf *rb, void *data, size_t len) {
    size_t ret = 0;
    uint8_t *d = (uint8_t*)data;
    while (!BUFFER_EMPTY(rb) && len > ret) {
        *d++ = rb->buf[rb->rx_index];
        rb->rx_index = (rb->rx_index + 1) % rb->size;
        ++ret;
    }
    return ret;
}

/*
 * Send out what is in the send buffer and clear the receive buffer
 */

int
msr_usb_commit (int fd)
{
    if (!BUFFER_EMPTY(&usb_if_state[fd].tx)) {
        size_t len;
        uint8_t tmp[USB_MAX_PACKET_SIZE];
        tmp[0] = 0x80;
        do {
            len =
                    ring_buf_fetch(&usb_if_state[fd].tx, &tmp[1], USB_MAX_PAYLOAD_SIZE);
            if (BUFFER_EMPTY(&usb_if_state[fd].tx)) {
                tmp[0] |= 0x40;
            }
            tmp[0] |= len;
            libusb_control_transfer(devh[fd], LIBUSB_REQUEST_TYPE_CLASS
                    | LIBUSB_RECIPIENT_INTERFACE | LIBUSB_ENDPOINT_OUT,
                    9, 0x300, 0, tmp, sizeof(tmp), 0);
        } while (!BUFFER_EMPTY(&usb_if_state[fd].tx));
        BUFFER_CLEAR(&usb_if_state[fd].rx);
    }
    return 0;
}
/*
 * Read a character from the serial port. Note that this
 * routine will block until a valid character is read.
 */

int
msr_usb_readchar (int fd, uint8_t * c)
{
	uint8_t		b;
    uint8_t tmp[USB_MAX_PACKET_SIZE];
	msr_usb_commit(fd);
	while (BUFFER_EMPTY(&usb_if_state[fd].rx)){
	    int actual_length;
	    libusb_interrupt_transfer(devh[fd], LIBUSB_ENDPOINT_IN | 1, tmp, sizeof(tmp), &actual_length, 0);
	    if (actual_length != sizeof(tmp)) {
	        printf("Error :-(\n");
	    }
	    ring_buf_append(&usb_if_state[fd].rx, &tmp[1], tmp[0] & 63);
	}

	ring_buf_fetch(&usb_if_state[fd].rx, &b, 1);
	if (c) {
	    *c = b;
	}

	return (int)b;
}

/*
 * Read a series of characters from the serial port. This
 * routine will block until the desired number of characters
 * is read.
 */

int
msr_usb_read (int fd, void * buf, size_t len)
{
	size_t i;
	uint8_t b, *p;

	p = buf;

#ifdef SERIAL_DEBUG
	printf("[RX %.3d]", len);
#endif
	for (i = 0; i < len; i++) {
		msr_usb_readchar (fd, &b);
#ifdef SERIAL_DEBUG
		printf(" %.2x", b);
#endif
		p[i] = b;
	}
#ifdef SERIAL_DEBUG
	printf("\n");
#endif

	return (0);
}

int
msr_usb_write (int fd, const void * buf, size_t len)
{
	return ring_buf_append(&usb_if_state[fd].tx, buf, len);
}

int
msr_usb_open(const char *path, int * fd, int blocking, speed_t baud)
{
    int r;
    int idx = 0;
    (void)path;
    (void)fd;
    (void)blocking;
    (void)baud;

    for (idx = 0; idx < ARRAY_ELEMENTS(devh) && devh[idx] != NULL; ++idx)
        ;
    if (idx >= ARRAY_ELEMENTS(devh)) {
        return -1;
    }

    memset(&usb_if_state, 0, sizeof(usb_if_state));
    usb_if_state[idx].rx.buf = usb_if_state[idx].rx_buf_data;
    usb_if_state[idx].rx.size = sizeof(usb_if_state[idx].rx_buf_data);
    usb_if_state[idx].tx.buf = usb_if_state[idx].tx_buf_data;
    usb_if_state[idx].tx.size = sizeof(usb_if_state[idx].tx_buf_data);

    r = libusb_init(NULL);
/*        libusb_set_debug(NULL, LIBUSB_LOG_LEVEL_DEBUG); */
    if (r < 0) {
        fprintf(stderr, "failed to initialise libusb\n");
        exit(1);
    }

    devh[idx] = open_device();
    if (devh[idx] == NULL) {
        fprintf(stderr, "Could not find/open device\n");
        goto error;
    }

    r = libusb_set_auto_detach_kernel_driver(devh[idx], 1);
    if (r < 0) {
        fprintf(stderr, "libusb_set_auto_detach_kernel_driver error %d (%s)\n", r, libusb_error_name(r));
        goto error;
    }

    r = libusb_claim_interface(devh[idx], 0);
    if (r < 0) {
        fprintf(stderr, "usb_claim_interface error %d (%s)\n", r, libusb_error_name(r));
    }
    IF_DEBUG(printf("claimed interface\n");)
    active_devices += 1;
    *fd = idx;
	return LIBMSR_ERR_OK;
error:
    if (devh[idx] != NULL) {
        libusb_reset_device(devh[idx]);
    }
    libusb_close(devh[idx]);
    if (active_devices == 0) {
        libusb_exit(NULL);
    }
    return LIBMSR_ERR_INTERFACE;
}

int
msr_usb_close(int fd)
{
    libusb_device_handle *h;
    if (active_devices > 0) {
        active_devices -= 1;
    }
    h = devh[fd];
    devh[fd] = NULL;
    libusb_release_interface(h, 0);
    libusb_close(h);
    if (active_devices == 0) {
        libusb_exit(NULL);
    }
    return LIBMSR_ERR_OK;
}

