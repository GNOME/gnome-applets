#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <glib.h>

#include <sys/types.h>
#include <sys/cdio.h>
#include "cdrom-interface.h"


#define ASSIGN_MSF(dest, src) \
{ \
  (dest).minute = (src).minute; \
  (dest).second = (src).second; \
  (dest).frame = (src).frame; \
}
int
cdrom_play(cdrom_device_t cdp, int start, int stop)
{
	struct cdrom_ti ti;
 	struct cdrom_msf msf;

 	/* Set up CDROMPLAYMSF call. If this fails try CDROMPLAYTRKIND */
 	msf.cdmsf_min0 = cdp->track_info[start-1].address.minute;
 	msf.cdmsf_sec0 = cdp->track_info[start-1].address.second;
 	msf.cdmsf_frame0 = cdp->track_info[start-1].address.frame;
 
 	msf.cdmsf_min1 = cdp->track_info[stop].address.minute;
 	msf.cdmsf_sec1 = cdp->track_info[stop].address.second;
 	msf.cdmsf_frame1 = cdp->track_info[stop].address.frame - 1;
 
 	msf.cdmsf_min1 += (msf.cdmsf_sec1 / 60);
 	msf.cdmsf_sec1 %= 60;

 	if (ioctl(cdp->device, CDROMPLAYMSF, &msf) == -1) {
	  ti.cdti_trk0 = start;
	  ti.cdti_ind0 = 1;
	  ti.cdti_trk1 = stop;
	  ti.cdti_ind1 = 1;
	  if (ioctl(cdp->device, CDROMPLAYTRKIND, &ti) == -1) {
	    cdp->my_errno = errno;
	    return DISC_IO_ERROR;
	  }
	}
	return DISC_NO_ERROR;
}

int
cdrom_play_msf(cdrom_device_t cdp, cdrom_msf_t * start)
{
	struct cdrom_msf msf;

	if (cdrom_read_track_info(cdp) == DISC_IO_ERROR)
		return DISC_IO_ERROR;
	msf.cdmsf_min0 = start->minute;
	msf.cdmsf_sec0 = start->second;
	msf.cdmsf_frame0 = start->frame;

	msf.cdmsf_min1 = cdp->track_info[cdp->nr_track].address.minute;
	msf.cdmsf_sec1 = cdp->track_info[cdp->nr_track].address.second;
	msf.cdmsf_frame1 = cdp->track_info[cdp->nr_track].address.frame;

	if (ioctl(cdp->device, CDROMPLAYMSF, &msf) == -1) {
		cdp->my_errno = errno;
		return DISC_IO_ERROR;
	}
	return DISC_NO_ERROR;
}

int
cdrom_pause(cdrom_device_t cdp)
{
	if (ioctl(cdp->device, CDROMPAUSE, 0) == -1) {
		return DISC_IO_ERROR;
		cdp->my_errno = errno;
	}
	return DISC_IO_ERROR;
}

int
cdrom_resume(cdrom_device_t cdp)
{
	if (ioctl(cdp->device, CDROMRESUME, 0) == -1) {
		cdp->my_errno = errno;
		return DISC_IO_ERROR;
	}
	return DISC_NO_ERROR;
}


int
cdrom_stop(cdrom_device_t cdp)
{
	if (ioctl(cdp->device, CDROMSTOP, 0) == -1) {
		cdp->my_errno = errno;
		return DISC_IO_ERROR;
	}
	return DISC_NO_ERROR;
}

int
cdrom_read_track_info(cdrom_device_t cdp)
{
	struct cdrom_tochdr toc;
	struct cdrom_tocentry tocentry;

	int i, j, nr_track;

	if (ioctl(cdp->device, CDROMREADTOCHDR, &toc) == -1) {
		cdp->my_errno = errno;
		return DISC_IO_ERROR;
	}
	cdp->track0 = toc.cdth_trk0;
	cdp->track1 = toc.cdth_trk1;
	nr_track = toc.cdth_trk1 - toc.cdth_trk0 + 1;
	if (nr_track <= 0)
		return DISC_IO_ERROR;

	if (nr_track != cdp->nr_track) {
		if (cdp->track_info)
			g_free(cdp->track_info);
		cdp->nr_track = nr_track;
		cdp->track_info = g_malloc((cdp->nr_track + 1) * sizeof(track_info_t));
	}
	for (i = 0, j = cdp->track0; i < cdp->nr_track; i++, j++) {
		tocentry.cdte_track = j;
		tocentry.cdte_format = CDROM_MSF;

		if (ioctl(cdp->device, CDROMREADTOCENTRY, &tocentry) == -1) {
			cdp->my_errno = errno;
			return DISC_IO_ERROR;
		}
		/* fill the trackinfo field */
		cdp->track_info[i].track = j;
		cdp->track_info[i].audio_track = tocentry.cdte_ctrl !=
		    CDROM_DATA_TRACK ? 1 : 0;
		ASSIGN_MSF(cdp->track_info[i].address, tocentry.cdte_addr.msf);
	}

	tocentry.cdte_track = CDROM_LEADOUT;
	tocentry.cdte_format = CDROM_MSF;
	if (ioctl(cdp->device, CDROMREADTOCENTRY, &tocentry) == -1) {
		cdp->my_errno = errno;
		return DISC_IO_ERROR;
	}
	cdp->track_info[i].track = j;
	cdp->track_info[i].audio_track = 0;
	ASSIGN_MSF(cdp->track_info[i].address, tocentry.cdte_addr.msf);

	return DISC_NO_ERROR;
}

int
cdrom_get_status(cdrom_device_t cdp, cdrom_device_status_t * stat)
{
	struct cdrom_subchnl subchnl;

	subchnl.cdsc_format = CDROM_MSF;
	if (ioctl(cdp->device, CDROMSUBCHNL, &subchnl) == -1) {
		cdp->my_errno = errno;
		return DISC_IO_ERROR;
	}
	switch (subchnl.cdsc_audiostatus) {
	case CDROM_AUDIO_PLAY:
		stat->audio_status = DISC_PLAY;
		break;
	case CDROM_AUDIO_PAUSED:
		stat->audio_status = DISC_PAUSED;
		break;
	case CDROM_AUDIO_COMPLETED:
		stat->audio_status = DISC_COMPLETED;
		break;
	case CDROM_AUDIO_INVALID:
	case CDROM_AUDIO_NO_STATUS:
	case CDROM_AUDIO_ERROR:
		stat->audio_status = DISC_STOP;
		break;
	default:
		stat->audio_status = DISC_ERROR;
	}
 	/* If stopped we're on track zero */
 	if (stat->audio_status != DISC_STOP)
 	  stat->track = subchnl.cdsc_trk;
 	else
 	  stat->track = 0;
	ASSIGN_MSF(stat->relative_address, subchnl.cdsc_reladdr.msf);
	ASSIGN_MSF(stat->absolute_address, subchnl.cdsc_absaddr.msf);
	return DISC_NO_ERROR;
}

cdrom_device_t
cdrom_open(char *device, int *errcode)
{
	cdrom_device_t cdp;

	cdp = g_malloc(sizeof(struct cdrom_device));

	cdp->device = open(device, O_RDONLY);
	if (cdp->device == -1) {
		*errcode = errno;
		g_free(cdp);
		return NULL;
	}
	cdp->nr_track = 0;
	cdp->track_info = NULL;
	return cdp;
}

void
cdrom_close(cdrom_device_t cdp)
{
	if (cdp->nr_track)
		g_free(cdp->track_info);
	close(cdp->device);
	g_free(cdp);
}

int
cdrom_load(cdrom_device_t cdp)
{
	/*if (ioctl(cdp->device, CDROMCLOSETRAY, 0) == -1) {
		cdp->my_errno = errno;
		return DISC_IO_ERROR;
	};*/
	/*XXX: is this supported under solaris??*/
	return DISC_NO_ERROR;
}

int
cdrom_eject(cdrom_device_t cdp)
{
	if (ioctl(cdp->device, CDROMEJECT, 0) == -1) {
		cdp->my_errno = errno;
		return DISC_IO_ERROR;
	};
	return DISC_NO_ERROR;
}


int 
cdrom_next(cdrom_device_t cdp)
{
	cdrom_device_status_t stat;
	int track;

	if ((cdrom_read_track_info(cdp) == DISC_IO_ERROR) ||
	    (cdrom_get_status(cdp, &stat) == DISC_IO_ERROR))
		return DISC_IO_ERROR;
	track = stat.track + 1;
	/* Don't advance if on the last track */
	if (track <= cdp->track1)
	  return cdrom_play(cdp, stat.track + 1, cdp->track1);
	return DISC_NO_ERROR;
}

int 
cdrom_prev(cdrom_device_t cdp)
{
	cdrom_device_status_t stat;
	int track;

	if ((cdrom_read_track_info(cdp) == DISC_IO_ERROR) ||
	    (cdrom_get_status(cdp, &stat) == DISC_IO_ERROR))
		return DISC_IO_ERROR;
	track = stat.track - 1;
	/* Don't go back if stopped or on the first track */
	if (track > 0)
	  return cdrom_play(cdp, stat.track - 1, cdp->track1);
	return DISC_NO_ERROR;
}

int
cdrom_rewind(cdrom_device_t cdp)
{
	cdrom_device_status_t stat;
	int track;

	if ((cdrom_read_track_info(cdp) == DISC_IO_ERROR) ||
	    (cdrom_get_status(cdp, &stat) == DISC_IO_ERROR))
		return DISC_IO_ERROR;
	if (stat.absolute_address.second != 0)
		stat.absolute_address.second--;
	else {
		stat.absolute_address.second = 0;
		if (stat.absolute_address.minute > 0)
			stat.absolute_address.minute--;
	}
	stat.absolute_address.frame = 0;
	return cdrom_play_msf(cdp, &stat.absolute_address);
}

int
cdrom_ff(cdrom_device_t cdp)
{
	cdrom_device_status_t stat;
	int track;

	if ((cdrom_read_track_info(cdp) == DISC_IO_ERROR) ||
	    (cdrom_get_status(cdp, &stat) == DISC_IO_ERROR))
		return DISC_IO_ERROR;
	stat.absolute_address.second++;
	if (stat.absolute_address.second >= 60) {
		stat.absolute_address.minute++;
		stat.absolute_address.second = 0;
	}
	stat.absolute_address.frame = 0;
	return cdrom_play_msf(cdp, &stat.absolute_address);
}

int
cdrom_track_length(cdrom_device_t cdp, int track, cdrom_msf_t * length)
{
	int index, s1, s2, i;

	if ((track < cdp->track0) || (track > cdp->track1))
		return DISC_INDEX_OUT_OF_RANGE;
	index = track - cdp->track0;

	s1 = cdp->track_info[index + 1].address.second;
	s2 = cdp->track_info[index].address.second;
	length->second = s1 = s1 - s2;
	if (s1 < 0) {
		i = 1;
		length->second = s1 + 60;
	} else
		i = 0;

	length->minute = cdp->track_info[index + 1].address.minute -
	    cdp->track_info[index].address.minute - i;
}
