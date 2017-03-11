/* Everyone must include libmsr.h or they're doing it wrong! */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdint.h>

/* Error codes, to be returned from library functions. */

#define LIBMSR_ERR_OK 0x0 /* no error */
#define LIBMSR_ERR_GENERIC 0x1000 /* generic errors */
#define LIBMSR_ERR_ISO 0x1100 /* errors with ISO formatted cards */
#define LIBMSR_ERR_DEVICE 0x2000 /* errors in device control */
#define LIBMSR_ERR_SERIAL 0x4000 /* errors in serial I/O */

/*
 * Track lengths when doing raw accesses can be at most 256 byte
 * in size, since the size field is only 8 bits wide. So we use this
 * as our maximum size.
 */

#define MSR_MAX_TRACK_LEN 255
#define MSR_MAX_TRACKS 3
#define MSR_BLOCKING O_NONBLOCK
#define MSR_BAUD B9600

/* MSR206 definitions, moved from msr206.h. */

/* ESC is frequently used as a start delimiter character */

#define MSR_ESC	0x1B /* Escape character */

/* ASCII file separator character is used to separate track data */

#define MSR_FS 0x1C /* File separator */

#define MSR_STS_OK 0x30 /* Ok */
#define MSR_STS_ERR 0x41 /* General error */

typedef struct msr_cmd {
	uint8_t msr_esc;
	uint8_t msr_cmd;
} msr_cmd_t;

/* Read/write commands */

#define MSR_CMD_READ 0x72 /* Formatted read */
#define MSR_CMD_WRITE 0x77 /* Formatted write */
#define MSR_CMD_RAW_READ 0x6D /* Raw read */
#define MSR_CMD_RAW_WRITE 0x6E /* Raw write */

/* Status byte values from read/write commands */

#define MSR_STS_RW_ERR 0x31	/* Read/write error */
#define MSR_STS_RW_CMDFMT_ERR 0x32 /* Command format error */
#define MSR_STS_RW_CMDBAD_ERR 0x34 /* Invalid command */
#define MSR_STS_RW_SWIPEBAD_ERR 0x39 /* Invalid card swipe in write mode */

typedef struct msr_end {
	uint8_t msr_enddelim;
	uint8_t msr_fs;
	uint8_t msr_esc;
	uint8_t msr_sts;
} msr_end_t;

/*
 * Read/write start and end delimiters.
 * The empty delimiter occurs when reading a track with no data.
 */

#define MSR_RW_START 0x73 /* 's' */
#define MSR_RW_END 0x3F /* '?' */
#define MSR_RW_BAD 0x2A /* '*' */
#define MSR_RW_EMPTY 0x2B /* '+' */

/*
 * Serial port communications test
 * If serial communications are working properly, the device
 * should respond with a 'y' command.
 */

#define MSR_CMD_DIAG_COMM 0x65 /* Communications test */
#define MSR_STS_COMM_OK 0x79

/*
 * Sensor diagnostic command. Will respond with MSR_STS_OK once
 * a card swipe is detected. Can be interrupted by a reset.
 */

#define MSR_CMD_DIAG_SENSOR 0x86 /* Card sensor test */
#define MSR_STS_SENSOR_OK MSR_STS_OK

/*
 * RAM diagnostic command. Will return MSR_STS_OK if RAM checks
 * good, otherwise MSR_STS_ERR.
 */

#define MSR_CMD_DIAG_RAM 0x87 /* RAM test */
#define MSR_STS_RAM_OK MSR_STS_OK
#define MSR_STS_RAM_ERR MSR_STS_ERR

/*
 * Set leading zero count. Responds with MSR_STS_OK if values
 * set ok, otherwise MSR_STS_ERR
 */

#define MSR_CMD_SLZ	0x7A /* Set leading zeros */
#define MSR_STS_SLZ_OK MSR_STS_OK
#define MSR_STS_SLZ_ERR	MSR_STS_ERR

/*
 * Get leading zero count. Returns leading zero counts for
 * track 1/3 and 2.
 */

#define MSR_CMD_CLZ 0x6C /* Check leading zeros */

typedef struct msr_lz {
	uint8_t msr_esc;
	uint8_t msr_lz_tk1_3;
	uint8_t msr_lz_tk2;
} msr_lz_t;

/*
 * Erase card tracks. Returns MSR_STS_OK on success or
 * MSR_STS_ERR.
 */

#define MSR_CMD_ERASE 0x63 /* Erase card tracks */
#define MSR_STS_ERASE_OK MSR_STS_OK
#define MSR_STS_ERASE_ERR MSR_STS_ERR

#define MSR_ERASE_TK1 0x00
#define MSR_ERASE_TK2 0x02
#define MSR_ERASE_TK3 0x04
#define MSR_ERASE_TK1_TK2 0x03
#define MSR_ERASE_TK1_TK3 0x05
#define MSR_ERASE_TK2_TK3 0x06
#define MSR_ERASE_ALL 0x07

/*
 * Set bits per inch. Returns MSR_STS_OK on success or
 * MSR_STS_ERR.
 */

#define MSR_CMD_SETBPI 0x62 /* Set bits per inch */
#define MSR_STS_BPI_OK MSR_STS_OK
#define MSR_STS_BPI_ERR MSR_STS_ERR

/*
 * Get device model number. Returns a value indicating a model
 * number, plus an 'S'.
 */

#define MSR_CMD_MODEL 0x74 /* Read model */
#define MSR_STS_MODEL_OK 0x53

#define MSR_MODEL_MSR206_1 0x31
#define MSR_MODEL_MSR206_2 0x32
#define MSR_MODEL_MSR206_3 0x33
#define MSR_MODEL_MSR206_5 0x35

typedef struct msr_model {
	uint8_t msr_esc;
	uint8_t msr_model;
	uint8_t msr_s;
} msr_model_t;

/*
 * Get firmware revision. Response is a string in
 * the form of "REV?X.XX" where X.XX is the firmware
 * rev, and ? can be:
 *
 * MSR206: '0'
 * MSR206HC: 'H'
 * MSR206HC: 'L'
 */

#define MSR_CMD_FWREV 0x76 /* Read firmware revision */
#define MSR_FWREV_FMT "REV?X.XX"

/*
 * Set bits per character. Returns MSR_STS_OK on success, accompanied
 * by resulting per-track BPC settings.
 */

#define MSR_CMD_SETBPC 0x6F /* Set bits per character */
#define MSR_STS_BPC_OK MSR_STS_OK
#define MSR_STS_BPC_ERR MSR_STS_ERR

typedef struct msr_bpc {
	uint8_t msr_bpctk1;
	uint8_t msr_bpctk2;
	uint8_t msr_bpctk3;
} msr_bpc_t;

/*
 * Set coercivity high or low. Returns MSR_STS_OK on success.
 */

#define MSR_CMD_SETCO_HI 0x78 /* Set coercivity high */
#define MSR_CMD_SETCO_LO 0x79 /* Set coercivity low */
#define MSR_STS_CO_OK MSR_STS_OK
#define MSR_STS_CO_ERR MSR_STS_ERR

/*
 * Get coercivity. Returns 'h' for high coercivity, 'l' for low.
 * NOTE: The user manual lies here, the returns are lower case!
 */

#define MSR_CMD_GETCO 0x64 /* Read coercivity setting */
#define MSR_CO_HI 0x68
#define MSR_CO_LO 0x6C

/* The following commands have no response codes */

#define MSR_CMD_RESET 0x61 /* Reset device */
#define MSR_CMD_LED_OFF 0x81 /* All LEDs off */
#define MSR_CMD_LED_ON 0x82 /* All LEDs on */
#define MSR_CMD_LED_GRN_ON 0x83 /* Green LED on */
#define MSR_CMD_LED_YLW_ON 0x84 /* Yellow LED on */
#define MSR_CMD_LED_RED_ON 0x85 /* Red LED on */

typedef struct msr_track {
	uint8_t msr_tk_data[MSR_MAX_TRACK_LEN];
	uint8_t msr_tk_len;
} msr_track_t;

typedef struct msr_tracks {
	msr_track_t	msr_tracks[MSR_MAX_TRACKS];
} msr_tracks_t;

/*
 * Serial I/O functions, moved from serialio.h and prefixed with msr_.
 */
extern int msr_serial_open (char *, int *,  int, speed_t);
extern int msr_serial_close (int);
extern int msr_serial_readchar (int, uint8_t *);
extern int msr_serial_write (int, void *, size_t);
extern int msr_serial_read (int, void *, size_t);

extern int msr_zeros (int);
extern int msr_commtest (int);
extern int msr_init (int);
extern int msr_fwrev (int, uint8_t *);
extern int msr_model (int, uint8_t *);
extern int msr_sensor_test (int);
extern int msr_ram_test (int);
extern int msr_get_co(int);
extern int msr_set_hi_co (int);
extern int msr_set_lo_co (int);
extern int msr_reset(int);
extern int msr_iso_read (int, msr_tracks_t *);
extern int msr_iso_write (int, msr_tracks_t *);
extern int msr_raw_read (int, msr_tracks_t *);
extern int msr_raw_write (int, msr_tracks_t *);
extern int msr_erase (int, uint8_t);
extern int msr_flash_led (int, uint8_t);
extern int msr_set_bpi (int, uint8_t);
extern int msr_set_bpc (int, uint8_t, uint8_t, uint8_t);

extern int msr_dumpbits (uint8_t *, int);
extern int msr_getbit (uint8_t *, uint8_t, int);
extern int msr_setbit (uint8_t *, uint8_t, int, int);
extern int msr_decode (uint8_t *, uint8_t, uint8_t *, uint8_t *, int);

extern int msr_reverse_tracks (msr_tracks_t *);
extern int msr_reverse_track (msr_track_t *);

extern void msr_pretty_output_hex (int fd, msr_tracks_t tracks);
extern void msr_pretty_output_string (int fd, msr_tracks_t tracks);
extern void msr_pretty_output_bits (int fd, msr_tracks_t tracks);

extern void msr_pretty_printer_hex (msr_tracks_t tracks);
extern void msr_pretty_printer_string (msr_tracks_t tracks);
extern void msr_pretty_printer_bits (msr_tracks_t tracks);

extern const unsigned char msr_reverse_byte (const unsigned char);
