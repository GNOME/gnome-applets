#if !defined(__CDROM_INTERFACE_H__)
#define __CDROM_INTERFACE_H__


#include <errno.h>

enum {
	DISC_NO_ERROR = 0,
	DISC_IO_ERROR,
	DISC_INDEX_OUT_OF_RANGE,
	DISC_TRAY_OPEN,
	DISC_DRIVE_NOT_READY
};

enum {
	DISC_PLAY,
	DISC_PAUSED,
	DISC_STOP,
	DISC_COMPLETED,
	DISC_ERROR
};

typedef struct {
	int minute;
	int second;
	int frame;
} cdrom_msf_t;

typedef struct {
	char *name;
	unsigned char track;
	unsigned int audio_track:1;
	cdrom_msf_t address;
	cdrom_msf_t length;
} track_info_t;

typedef struct {
	int audio_status;
	unsigned char track;
	cdrom_msf_t relative_address;
	cdrom_msf_t absolute_address;
} cdrom_device_status_t;

typedef struct cdrom_device {
#ifdef __sgi
	struct cdplayer *device;
#else
	int device;
#endif
	int nr_track;
	unsigned char track0, track1;
	track_info_t *track_info;
#if 0
	cdrom_msf_t leadout;
#endif

	int my_errno;
} *cdrom_device_t;

cdrom_device_t cdrom_open(char *path, /* out */ int *errcode);
void cdrom_close(cdrom_device_t cdp);

int cdrom_load(cdrom_device_t cdp);
int cdrom_eject(cdrom_device_t cdp);
int cdrom_read_track_info(cdrom_device_t cdp);
int cdrom_play(cdrom_device_t cdp, int start, int stop);
int cdrom_play_msf(cdrom_device_t cdp, cdrom_msf_t * start);
int cdrom_pause(cdrom_device_t cdp);
int cdrom_resume(cdrom_device_t cdp);
int cdrom_stop(cdrom_device_t cdp);
int cdrom_next(cdrom_device_t cdp);
int cdrom_prev(cdrom_device_t cdp);
int cdrom_ff(cdrom_device_t cdp);
int cdrom_rewind(cdrom_device_t cdp);

int cdrom_track_length(cdrom_device_t cdp, int track,
		       /* out */ cdrom_msf_t * length);
int cdrom_get_status(cdrom_device_t cdp, cdrom_device_status_t * stat);
#endif
