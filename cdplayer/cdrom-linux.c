#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <glib.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <linux/types.h>
#include <linux/cdrom.h>
#include "cdrom-interface.h"


#define ASSIGN_MSF(dest, src) \
{ \
  (dest).minute = (src).minute; \
  (dest).second = (src).second; \
  (dest).frame = (src).frame; \
}
void
cdrom_unuse(cdrom_device_t cdp)
{
	g_return_if_fail (cdp->use_count > 0);
	cdp->use_count--;
	if (cdp->use_count == 0) {
		close(cdp->device);
		cdp->device = -1;
	}
}
gboolean
cdrom_use(cdrom_device_t cdp, int *errcode)
{
	if (cdp->use_count == 0) {
		cdp->device = open(cdp->device_name, O_RDONLY|O_NONBLOCK);
		if (cdp->device == -1) {
			*errcode = errno;
			return FALSE;
		}
	}

	cdp->use_count++;
	return TRUE;
}

int
cdrom_play(cdrom_device_t cdp, int start, int stop)
{
	struct cdrom_ti ti;
	int result = DISC_NO_ERROR;

	if (!cdrom_use (cdp, &cdp->my_errno))
		return DISC_IO_ERROR;

	ti.cdti_trk0 = start;
	ti.cdti_ind0 = 1;
	ti.cdti_trk1 = stop;
	ti.cdti_ind1 = 1;
	if (ioctl(cdp->device, CDROMPLAYTRKIND, &ti) == -1) {
		cdp->my_errno = errno;
		result = DISC_IO_ERROR;
	}

	cdrom_unuse (cdp);
	return result;
}

int
cdrom_play_msf(cdrom_device_t cdp, cdrom_msf_t * start)
{
	struct cdrom_msf msf;
	int result = DISC_NO_ERROR;

	if (!cdrom_use (cdp, &cdp->my_errno))
		return DISC_IO_ERROR;

	if (cdrom_read_track_info(cdp) == DISC_IO_ERROR) {
		result = DISC_IO_ERROR;
		goto out;
	}
	msf.cdmsf_min0 = start->minute;
	msf.cdmsf_sec0 = start->second;
	msf.cdmsf_frame0 = start->frame;

	msf.cdmsf_min1 = cdp->track_info[cdp->nr_track].address.minute;
	msf.cdmsf_sec1 = cdp->track_info[cdp->nr_track].address.second;
	msf.cdmsf_frame1 = cdp->track_info[cdp->nr_track].address.frame;

	if (ioctl(cdp->device, CDROMPLAYMSF, &msf) == -1) {
		cdp->my_errno = errno;
		result = DISC_IO_ERROR;
	}

 out:
	cdrom_unuse (cdp);
	return result;
}

int
cdrom_pause(cdrom_device_t cdp)
{
	int result = DISC_NO_ERROR;
	
	if (!cdrom_use (cdp, &cdp->my_errno))
		return DISC_IO_ERROR;

	if (ioctl(cdp->device, CDROMPAUSE, 0) == -1) {
		result = DISC_IO_ERROR;
		cdp->my_errno = errno;
	}

	cdrom_unuse (cdp);
	return result;
}

int
cdrom_resume(cdrom_device_t cdp)
{
	int result = DISC_NO_ERROR;
	
	if (!cdrom_use (cdp, &cdp->my_errno))
		return DISC_IO_ERROR;

	if (ioctl(cdp->device, CDROMRESUME, 0) == -1) {
		cdp->my_errno = errno;
		result = DISC_IO_ERROR;
	}

	cdrom_unuse (cdp);
	return result;
}


int
cdrom_stop(cdrom_device_t cdp)
{
	int result = DISC_NO_ERROR;
	
	if (!cdrom_use (cdp, &cdp->my_errno))
		return DISC_IO_ERROR;

	if (ioctl(cdp->device, CDROMSTOP, 0) == -1) {
		cdp->my_errno = errno;
		result = DISC_IO_ERROR;
	}

	cdrom_unuse (cdp);
	return result;
}

int
cdrom_read_track_info(cdrom_device_t cdp)
{
	struct cdrom_tochdr toc;
	struct cdrom_tocentry tocentry;

	int i, j, nr_track;
	int result = DISC_NO_ERROR;

	if (!cdrom_use (cdp, &cdp->my_errno))
		return DISC_IO_ERROR;

	if (ioctl(cdp->device, CDROMREADTOCHDR, &toc) == -1) {
		cdp->my_errno = errno;
		result = DISC_IO_ERROR;
		goto out;
	}
	cdp->track0 = toc.cdth_trk0;
	cdp->track1 = toc.cdth_trk1;
	nr_track = toc.cdth_trk1 - toc.cdth_trk0 + 1;
	if (nr_track <= 0) {
		result = DISC_IO_ERROR;
		goto out;
	}

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
			result = DISC_IO_ERROR;
			goto out;
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
		result = DISC_IO_ERROR;
		goto out;
	}
	cdp->track_info[i].track = j;
	cdp->track_info[i].audio_track = 0;
	ASSIGN_MSF(cdp->track_info[i].address, tocentry.cdte_addr.msf);

 out:
	cdrom_unuse (cdp);
	return result;
}

int
cdrom_get_status(cdrom_device_t cdp, cdrom_device_status_t * stat)
{
	struct cdrom_subchnl subchnl;
	int status;
	int result = DISC_NO_ERROR;

	if (!cdrom_use (cdp, &cdp->my_errno))
		return DISC_IO_ERROR;

/*if we have the uniform cdrom api, which I thought was on 2.0 kernels,
  but it doesn't seem to be*/
#ifdef CDROM_DRIVE_STATUS
	status = ioctl(cdp->device, CDROM_DRIVE_STATUS, CDSL_CURRENT);

	/*if the status is -1, then it could mean that we just don't support
	  the CDROM_DRIVE_STATUS ioctl*/
	if (status != -1) {
		switch(status) {
		case CDS_DISC_OK:
			break;
		case CDS_TRAY_OPEN:
			result = DISC_TRAY_OPEN;
			goto out;
		case CDS_DRIVE_NOT_READY:
			result = DISC_DRIVE_NOT_READY;
			goto out;
		default:
			break;
		}
	}
#endif

	subchnl.cdsc_format = CDROM_MSF;
	if (ioctl(cdp->device, CDROMSUBCHNL, &subchnl) == -1) {
		cdp->my_errno = errno;
		result = DISC_IO_ERROR;
		goto out;
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
	stat->track = subchnl.cdsc_trk;
	ASSIGN_MSF(stat->relative_address, subchnl.cdsc_reladdr.msf);
	ASSIGN_MSF(stat->absolute_address, subchnl.cdsc_absaddr.msf);

 out:
	cdrom_unuse (cdp);

	return result;
}

cdrom_device_t
cdrom_open(char *device, int *errcode)
{
	cdrom_device_t cdp;

	cdp = g_malloc(sizeof(struct cdrom_device));
	cdp->device_name = g_strdup (device);
	cdp->use_count = 0;

	if (!cdrom_use (cdp, errcode)) {
		g_free(cdp);
		return NULL;
	}
	cdrom_unuse (cdp);
	cdp->nr_track = 0;
	cdp->track_info = NULL;
	return cdp;
}

void
cdrom_close(cdrom_device_t cdp)
{
	g_assert (cdp->use_count == 0);
	
	if (cdp->nr_track)
		g_free(cdp->track_info);
	g_free(cdp->device_name);
	g_free(cdp);
}

int
cdrom_load(cdrom_device_t cdp)
{
	int result = DISC_NO_ERROR;
	
	if (!cdrom_use (cdp, &cdp->my_errno))
		return DISC_IO_ERROR;
	
	if (ioctl(cdp->device, CDROMCLOSETRAY, 0) == -1) {
		cdp->my_errno = errno;
		result = DISC_IO_ERROR;
	}

	cdrom_unuse (cdp);
	return result;
}

int
cdrom_eject(cdrom_device_t cdp)
{
	int result = DISC_NO_ERROR;
	
	if (!cdrom_use (cdp, &cdp->my_errno))
		return DISC_IO_ERROR;
	
	if (ioctl(cdp->device, CDROMEJECT, 0) == -1) {
		cdp->my_errno = errno;
		result = DISC_IO_ERROR;
	}

	cdrom_unuse (cdp);
	return result;
}


int 
cdrom_next(cdrom_device_t cdp)
{
	cdrom_device_status_t stat;
	int track;
	int result = DISC_NO_ERROR;

	if (!cdrom_use (cdp, &cdp->my_errno))
		return DISC_IO_ERROR;
	
	if ((cdrom_read_track_info(cdp) == DISC_IO_ERROR) ||
	    (cdrom_get_status(cdp, &stat) == DISC_IO_ERROR)) {
		result = DISC_IO_ERROR;
		goto out;
	}
	track = stat.track + 1;
	result = cdrom_play(cdp, stat.track + 1, cdp->track1);

 out:
	cdrom_unuse (cdp);
	return result;
}

int 
cdrom_prev(cdrom_device_t cdp)
{
	cdrom_device_status_t stat;
	int track;
	int result = DISC_NO_ERROR;

	if (!cdrom_use (cdp, &cdp->my_errno))
		return DISC_IO_ERROR;
	
	if ((cdrom_read_track_info(cdp) == DISC_IO_ERROR) ||
	    (cdrom_get_status(cdp, &stat) == DISC_IO_ERROR)) {
		result = DISC_IO_ERROR;
		goto out;
	}
	track = stat.track - 1;
	result = cdrom_play(cdp, stat.track - 1, cdp->track1);

 out:
	cdrom_unuse (cdp);
	return result;
}

int
cdrom_rewind(cdrom_device_t cdp)
{
	cdrom_device_status_t stat;
	int result = DISC_NO_ERROR;
	/* int track; */

	if (!cdrom_use (cdp, &cdp->my_errno))
		return DISC_IO_ERROR;
	
	if ((cdrom_read_track_info(cdp) == DISC_IO_ERROR) ||
	    (cdrom_get_status(cdp, &stat) == DISC_IO_ERROR)) {
		result = DISC_IO_ERROR;
		goto out;
	}
	if (stat.absolute_address.second != 0)
		stat.absolute_address.second--;
	else {
		stat.absolute_address.second = 0;
		if (stat.absolute_address.minute > 0)
			stat.absolute_address.minute--;
	}
	stat.absolute_address.frame = 0;
	result = cdrom_play_msf(cdp, &stat.absolute_address);

 out:
	cdrom_unuse (cdp);
	return result;
}

int
cdrom_ff(cdrom_device_t cdp)
{
	cdrom_device_status_t stat;
	int result = DISC_NO_ERROR;
	/* int track; */

	if (!cdrom_use (cdp, &cdp->my_errno))
		return DISC_IO_ERROR;
	
	if ((cdrom_read_track_info(cdp) == DISC_IO_ERROR) ||
	    (cdrom_get_status(cdp, &stat) == DISC_IO_ERROR)) {
		result = DISC_IO_ERROR;
		goto out;
	}
	stat.absolute_address.second++;
	if (stat.absolute_address.second >= 60) {
		stat.absolute_address.minute++;
		stat.absolute_address.second = 0;
	}
	stat.absolute_address.frame = 0;
	result = cdrom_play_msf(cdp, &stat.absolute_address);

 out:
	cdrom_unuse (cdp);
	return result;
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

	return 0;
}
