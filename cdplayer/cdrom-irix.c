/* Time-stamp: "Thu 15 Mar 2001, 18:45:10 EST by drk@sgi.com (David Kaelbling)"
 *
 * SGI IRIX 6.x cdrom player guts.
 *
 * Known bugs:
 * - Output is through the CD headphone jack only.  Output via libaudio
 *	isn't hard (see "man cdintro" for an example), but that's not
 *	how this program behaves on other systems.
 * - You'll need the Development Libraries (dev.sw.lib) to link.
 * - The CD device must be deprotected in /etc/ioperms with ioconfig.
 *	Making cdplayer_applet suid causes problems in ~/.gnome_private.
 */

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <glib.h>
#include <sys/types.h>
#include <unistd.h>

#include <dmedia/cdaudio.h>
#include "cdrom-interface.h"


/* Play tracks start .. stop */
int
cdrom_play(cdrom_device_t cdp, int start, int stop)
{
  int status;

  /*
   * IRIX doesn't support playing a sub-range of tracks.  Fortunately
   * this only seems to be used to play the remainder of a disk, which
   * IRIX does support.  Special case playing one track just in case...
   */
  if (start + 1 >= stop)
    status = CDplaytrack(cdp->device, start, 1);
  else
    status = CDplay(cdp->device, start, 1);

  return (status == 0) ? DISC_IO_ERROR : DISC_NO_ERROR;
}

/* Play starting in the middle of a track. */
int
cdrom_play_msf(cdrom_device_t cdp, cdrom_msf_t * start)
{
  int status;

  status = CDplayabs(cdp->device, start->minute, start->second, start->frame, 1);

  return (status == 0) ? DISC_IO_ERROR : DISC_NO_ERROR;
}

/* Pause the player. */
int
cdrom_pause(cdrom_device_t cdp)
{
  int status;
  CDSTATUS drive_status;

  /* Find out if we're playing before we toggle pause. */
  status = CDgetstatus(cdp->device, &drive_status);
  if (status && drive_status.state == CD_PLAYING)
    status = CDtogglepause(cdp->device);

  return (status == 0) ? DISC_IO_ERROR : DISC_NO_ERROR;
}

/* Unpause the player. */
int
cdrom_resume(cdrom_device_t cdp)
{
  int status;
  CDSTATUS drive_status;

  /* Find out if we're paused before we resume playing. */
  status = CDgetstatus(cdp->device, &drive_status);
  if (status && drive_status.state == CD_PAUSED)
    status = CDtogglepause(cdp->device);

  return (status == 0) ? DISC_IO_ERROR : DISC_NO_ERROR;
}

/* Stop the player. */
int
cdrom_stop(cdrom_device_t cdp)
{
  int status;

  status = CDstop(cdp->device);

  return (status == 0) ? DISC_IO_ERROR : DISC_NO_ERROR;
}

/* Query track information. */
int
cdrom_read_track_info(cdrom_device_t cdp)
{
  int old_nr_track = cdp->nr_track;
  CDSTATUS drive_status;
  CDTRACKINFO info;
  int status;
  int i;

  /* Get the disc stats */
  if (! (status = CDgetstatus(cdp->device, &drive_status)) ||
      drive_status.state == CD_ERROR ||
      drive_status.state == CD_NODISC ||
      drive_status.state == CD_CDROM ||
      drive_status.last < drive_status.first) {
    return DISC_IO_ERROR;
  }
  cdp->track0 = drive_status.first;
  cdp->track1 = drive_status.last;
  cdp->nr_track = cdp->track1 - cdp->track0 + 1;

  /* Realloc the track_info structure.  Leave an extra entry for leadout. */
  if (old_nr_track != cdp->nr_track) {
    if (cdp->track_info)
      g_free(cdp->track_info);
    cdp->track_info = g_malloc((cdp->nr_track + 1) * sizeof(track_info_t));
  }

  /* Fill in the track info data. */
  for (i = 0; i < cdp->nr_track; i++) {
    if (! (status = CDgettrackinfo(cdp->device, i + cdp->track0, &info)))
      return DISC_IO_ERROR;

    cdp->track_info[i].track = i + cdp->track0;
    cdp->track_info[i].audio_track = (drive_status.state != CD_CDROM);
    cdp->track_info[i].address.minute = info.start_min;
    cdp->track_info[i].address.second = info.start_sec;
    cdp->track_info[i].address.frame = info.start_frame;
    cdp->track_info[i].length.minute = info.total_min;
    cdp->track_info[i].length.second = info.total_sec;
    cdp->track_info[i].length.frame = info.total_frame;
  }

  /* Create an entry for the leadout. */
  cdp->track_info[cdp->nr_track].track = cdp->track1 + 1;
  cdp->track_info[cdp->nr_track].audio_track = 0;
  cdp->track_info[cdp->nr_track].address.minute = drive_status.total_min;
  cdp->track_info[cdp->nr_track].address.second = drive_status.total_sec;
  cdp->track_info[cdp->nr_track].address.frame = drive_status.total_frame;
  cdp->track_info[cdp->nr_track].length.minute = 0;
  cdp->track_info[cdp->nr_track].length.second = 0;
  cdp->track_info[cdp->nr_track].length.frame = 0;

  return DISC_NO_ERROR;
}

/* Query player status. */
int
cdrom_get_status(cdrom_device_t cdp, cdrom_device_status_t * stat)
{
  CDSTATUS drive_status;
  int status;

  /* Query the drive status. */
  if (! (status = CDgetstatus(cdp->device, &drive_status))) {
    cdp->my_errno = -1;
    return DISC_IO_ERROR;
  }
  cdp->my_errno = drive_status.state;

  /* Copy out the obvious fields. */
  stat->track = drive_status.track;
  stat->relative_address.minute = drive_status.min;
  stat->relative_address.second = drive_status.sec;
  stat->relative_address.frame = drive_status.frame;
  stat->absolute_address.minute = drive_status.abs_min;
  stat->absolute_address.second = drive_status.abs_sec;
  stat->absolute_address.frame = drive_status.abs_frame;

  /* Translate the audio status. */
  switch (drive_status.state) {
  case CD_CDROM:		/* This is not an audio disc. */
  case CD_ERROR:
    stat->audio_status = DISC_ERROR;
    return DISC_IO_ERROR;
  case CD_NODISC:
    stat->audio_status = DISC_ERROR;
    return DISC_DRIVE_NOT_READY;
  case CD_READY:
    stat->audio_status = DISC_COMPLETED;
    break;
  case CD_PLAYING:
    stat->audio_status = DISC_PLAY;
    break;
  case CD_PAUSED:
    stat->audio_status = DISC_PAUSED;
    break;
  case CD_STILL:
  default:
    stat->audio_status = DISC_STOP;
    break;
  }

  return DISC_NO_ERROR;
}

/* Open a player device. */
cdrom_device_t
cdrom_open(char *device, int *errcode)
{
  cdrom_device_t cdp;

  cdp = g_malloc(sizeof(struct cdrom_device));

  cdp->device = CDopen(device, "r");
  if (cdp->device == NULL) {
    *errcode = errno;
    g_free(cdp);
    return NULL;
  }

  cdp->nr_track = 0;
  cdp->track_info = NULL;
  return cdp;
}

/* Close a player device. */
void
cdrom_close(cdrom_device_t cdp)
{
  if (cdp->nr_track)
    g_free(cdp->track_info);
  CDclose(cdp->device);
  g_free(cdp);
}

/* Close player's tray */
int
cdrom_load(cdrom_device_t cdp)
{
  int status;

  /* There is no CDload API on IRIX, so balk playing. */
  status = CDplay(cdp->device, cdp->track0, 0) && CDstop(cdp->device);

  return (status == 0) ? DISC_IO_ERROR : DISC_NO_ERROR;
}

/* Eject the tray. */
int
cdrom_eject(cdrom_device_t cdp)
{
  int status;

  status = CDeject(cdp->device);

  return (status == 0) ? DISC_IO_ERROR : DISC_NO_ERROR;
}

/* Skip forward to the next track and play. */
int 
cdrom_next(cdrom_device_t cdp)
{
  cdrom_device_status_t stat;

  if ((cdrom_read_track_info(cdp) == DISC_IO_ERROR) ||
      (cdrom_get_status(cdp, &stat) == DISC_IO_ERROR)) {
    return DISC_IO_ERROR;
  }

  return cdrom_play(cdp, stat.track + 1, cdp->track1);
}

/* Skip backward to the previous track and play. */
int 
cdrom_prev(cdrom_device_t cdp)
{
  cdrom_device_status_t stat;

  if ((cdrom_read_track_info(cdp) == DISC_IO_ERROR) ||
      (cdrom_get_status(cdp, &stat) == DISC_IO_ERROR)) {
    return DISC_IO_ERROR;
  }

  return cdrom_play(cdp, stat.track - 1, cdp->track1);
}

/* Backup a second. */
int
cdrom_rewind(cdrom_device_t cdp)
{
  cdrom_device_status_t stat;

  if ((cdrom_read_track_info(cdp) == DISC_IO_ERROR) ||
      (cdrom_get_status(cdp, &stat) == DISC_IO_ERROR)) {
    return DISC_IO_ERROR;
  }

  if (stat.absolute_address.second != 0) {
    stat.absolute_address.second--;
  } else {
    stat.absolute_address.second = 0;
    if (stat.absolute_address.minute > 0)
      stat.absolute_address.minute--;
  }

  stat.absolute_address.frame = 0;
  return cdrom_play_msf(cdp, &stat.absolute_address);
}

/* Advance a second. */
int
cdrom_ff(cdrom_device_t cdp)
{
  cdrom_device_status_t stat;

  if ((cdrom_read_track_info(cdp) == DISC_IO_ERROR) ||
      (cdrom_get_status(cdp, &stat) == DISC_IO_ERROR)) {
    return DISC_IO_ERROR;
  }

  stat.absolute_address.second++;
  if (stat.absolute_address.second >= 60) {
    stat.absolute_address.minute++;
    stat.absolute_address.second = 0;
  }

  stat.absolute_address.frame = 0;
  return cdrom_play_msf(cdp, &stat.absolute_address);
}

/* Return the length of a track. */
int
cdrom_track_length(cdrom_device_t cdp, int track, cdrom_msf_t * length)
{
  if ((track < cdp->track0) || (track > cdp->track1)) {
    return DISC_INDEX_OUT_OF_RANGE;
  }

  length->minute = cdp->track_info[track - cdp->track0].length.minute;
  length->second = cdp->track_info[track - cdp->track0].length.second;
  length->frame = cdp->track_info[track - cdp->track0].length.frame;

  return DISC_NO_ERROR;
}
