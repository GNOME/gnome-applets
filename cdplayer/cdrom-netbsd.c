
/*
 * Hacked up from cdrom-solaris.c
 *
 * jgw 24/02/2000
 *
 */


#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <glib.h>

#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h>
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
       struct ioc_play_track ipt;

       ipt.start_track = start;
       ipt.start_index = 1;
       ipt.end_track = stop;
       ipt.end_index = 1;
       if (ioctl(cdp->device, CDIOCPLAYTRACKS, &ipt) == -1) {
               cdp->my_errno = errno;
               return DISC_IO_ERROR;
       }
       return DISC_NO_ERROR;
}

int
cdrom_play_msf(cdrom_device_t cdp, cdrom_msf_t * start)
{
       struct ioc_play_msf msf;

       if (cdrom_read_track_info(cdp) == DISC_IO_ERROR)
               return DISC_IO_ERROR;
       msf.start_m = start->minute;
       msf.start_s = start->second;
       msf.start_f = start->frame;

       msf.end_m = cdp->track_info[cdp->nr_track].address.minute;
       msf.end_s = cdp->track_info[cdp->nr_track].address.second;
       msf.end_f = cdp->track_info[cdp->nr_track].address.frame;

       if (ioctl(cdp->device, CDIOCPLAYMSF, &msf) == -1) {
               cdp->my_errno = errno;
               return DISC_IO_ERROR;
       }
       return DISC_NO_ERROR;
}

int
cdrom_pause(cdrom_device_t cdp)
{
       if (ioctl(cdp->device, CDIOCPAUSE, 0) == -1) {
               return DISC_IO_ERROR;
               cdp->my_errno = errno;
       }
       return DISC_IO_ERROR;
}

int
cdrom_resume(cdrom_device_t cdp)
{
       if (ioctl(cdp->device, CDIOCRESUME, 0) == -1) {
               cdp->my_errno = errno;
               return DISC_IO_ERROR;
       }
       return DISC_NO_ERROR;
}


int
cdrom_stop(cdrom_device_t cdp)
{
       if (ioctl(cdp->device, CDIOCSTOP, 0) == -1) {
               cdp->my_errno = errno;
               return DISC_IO_ERROR;
       }
       return DISC_NO_ERROR;
}

int
cdrom_read_track_info(cdrom_device_t cdp)
{
       struct ioc_toc_header toc;
       struct ioc_read_toc_entry tocentry;

       int i, j, nr_track;

       if (ioctl(cdp->device, CDIOREADTOCHEADER, &toc) == -1) {
               cdp->my_errno = errno;
               return DISC_IO_ERROR;
       }
       cdp->track0 = toc.starting_track;
       cdp->track1 = toc.ending_track;
       nr_track = toc.ending_track - toc.starting_track + 1;
       if (nr_track <= 0)
               return DISC_IO_ERROR;

       if (nr_track != cdp->nr_track) {
               if (cdp->track_info)
                       g_free(cdp->track_info);
               cdp->nr_track = nr_track;
               cdp->track_info = g_malloc((cdp->nr_track + 1) * sizeof(track_info_t));
       }
       for (i = 0, j = cdp->track0; i < cdp->nr_track; i++, j++) {
               tocentry.starting_track = j;
               tocentry.address_format = CD_MSF_FORMAT;
               tocentry.data_len = sizeof(struct cd_toc_entry);
               tocentry.data = malloc(sizeof(struct cd_toc_entry));
              
               if (ioctl(cdp->device, CDIOREADTOCENTRIES, &tocentry) == -1) {
                       cdp->my_errno = errno;
                       free(tocentry.data);
                       return DISC_IO_ERROR;
               }
               /* fill the trackinfo field */
               cdp->track_info[i].track = j;

               /* XXX these should also be in <sys/cdio.h> */
#define CD_TYPE_AUDIO       1   /* Audio track */
#define CD_TYPE_DATA        2   /* Data track */

               cdp->track_info[i].audio_track = tocentry.data->control != CD_TYPE_DATA ? 1 : 0;
               cdp->track_info[i].audio_track = 0;
               ASSIGN_MSF(cdp->track_info[i].address, tocentry.data->addr.msf);
               free(tocentry.data);
       }
#define CD_LEADOUT   0xaa /* XXX this should be in <sys/cdio.h> (?) */

       tocentry.starting_track = CD_LEADOUT;
       tocentry.address_format = CD_MSF_FORMAT;
       if (ioctl(cdp->device, CDIOREADTOCENTRIES, &tocentry) == -1) {
               cdp->my_errno = errno;
               return DISC_IO_ERROR;
       }
       cdp->track_info[i].track = j;
       cdp->track_info[i].audio_track = 0;
       ASSIGN_MSF(cdp->track_info[i].address, tocentry.data->addr.msf);

       return DISC_NO_ERROR;
}

int
cdrom_reset(cdrom_device_t cdp)
{
       if (ioctl(cdp->device, CDIOCRESET, 0) == -1) {
               cdp->my_errno = errno;
               /* perror("cdrom_reset"); */ /* XXX don't spam the user with errors. */
               return DISC_IO_ERROR;
       }
       return DISC_NO_ERROR;
}

int
cdrom_get_status(cdrom_device_t cdp, cdrom_device_status_t * stat)
{
       struct ioc_read_subchannel subchnl;
       int errcode;

       memset(&subchnl, 0, sizeof(struct ioc_read_subchannel));

       subchnl.address_format = CD_MSF_FORMAT;
       subchnl.data_format = CD_CURRENT_POSITION;
      
       subchnl.data_len = sizeof(struct cd_sub_channel_info);
       subchnl.data = malloc(sizeof(struct cd_sub_channel_info));

       if (ioctl(cdp->device, CDIOCREADSUBCHANNEL, &subchnl) == -1) {
               cdp->my_errno = errno;
               free(subchnl.data);
               if (cdrom_reset(cdp) == DISC_NO_ERROR)
               {
                       /* XXX ICK, if the user has ejected a cdrom, and then loaded new one,
                               we need to close/open the device before we can play cd's again.
                              
                               Unfortunalty we gets loads of errors spammed to syslog as we wait for the drive to
                               spin up.
                              
                               we need a CDIOCDRIVESTATUS ioctl which will tell us:
                              
                               drive empty
                               drive open
                               drive becoming ready
                               drive ready
                              
                               */
                       cdrom_close(cdp);
                       errcode = 0;
                       cdp = cdrom_open(NULL,&errcode);
                       if (errcode == 0)
                               return(cdrom_get_status(cdp, stat));
               }
               /* unfortunatly we spam the user with errors here */
               perror("You may ignore these errors if your in the process of changing a cd : ");
               printf("\n");
               return DISC_IO_ERROR;
       }
       switch (subchnl.data->header.audio_status) {
       case CD_AS_PLAY_IN_PROGRESS:
               stat->audio_status = DISC_PLAY;
               break;
       case CD_AS_PLAY_PAUSED:
               stat->audio_status = DISC_PAUSED;
               break;
       case CD_AS_PLAY_COMPLETED:
               stat->audio_status = DISC_COMPLETED;
               break;
       case CD_AS_AUDIO_INVALID:
       case CD_AS_NO_STATUS:
       case CD_AS_PLAY_ERROR:
               stat->audio_status = DISC_STOP;
               break;
       default:
               stat->audio_status = DISC_ERROR;
       }
       stat->track = subchnl.data->what.position.track_number;
       ASSIGN_MSF(stat->relative_address, subchnl.data->what.position.reladdr.msf);
       ASSIGN_MSF(stat->absolute_address, subchnl.data->what.position.absaddr.msf);
       free(subchnl.data);
       return DISC_NO_ERROR;
}

cdrom_device_t
cdrom_open(char *device, int *errcode)
{
       cdrom_device_t cdp;
       static char * cddev = NULL;

       if (cddev == NULL && device != NULL)
       {
               cddev = device;
       }

       cdp = g_malloc(sizeof(struct cdrom_device));

       if (device == NULL)
               device = cddev; /* remember which device to use */

       cdp->device = open(device, O_RDONLY);
       if (cdp->device == -1) {
               *errcode = errno;
               perror("cdrom_open");
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
       if (ioctl(cdp->device, CDIOCCLOSE, 0) == -1) {
               cdp->my_errno = errno;
               return DISC_IO_ERROR;
       }
       return DISC_NO_ERROR;
}

int
cdrom_eject(cdrom_device_t cdp)
{
       cdrom_device_status_t stat;

       printf("cdrom_eject: %d", cdrom_get_status(cdp, &stat));
      
       if (stat.audio_status == DISC_PLAY)
       {
               /* stop before we eject */
               cdrom_stop(cdp);
       }
      
       if (ioctl(cdp->device, CDIOCALLOW, 0) == -1) {
               cdp->my_errno = errno;
               perror("cdrom_eject: allow");
       }

       if (ioctl(cdp->device, CDIOCEJECT, 0) == -1) {
               cdp->my_errno = errno;
               perror("cdrom_eject");
               return DISC_IO_ERROR;
       }
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
       return cdrom_play(cdp, stat.track + 1, cdp->track1);
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
       return cdrom_play(cdp, stat.track - 1, cdp->track1);
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

       return 0;
}
