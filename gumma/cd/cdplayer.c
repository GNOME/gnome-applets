/*
 * GUMMA - Gnome MultiMedia Applet
 *
 * (C) 1999 The Free Software Foundation
 *
 * Authors: Jacob Berkman  <jberkman@andrew.cmu.edu>
 *
 * CD plugin - taken from gnome-applets/cdplayer/
 * (Originally by Ching Hui?)
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <config.h>
#include <gnome.h>
#include <gdk/gdkx.h>

#define GUMMA_PLUGIN
#include <gumma/gumma.h>

#include <gumma/cd/cdrom-interface.h>

static char *devpath;
static cdrom_device_t device;

static int
cd_try_open ()
{
	int err;
	if(!device) {
		device = cdrom_open(devpath, &err);
		return device != NULL;
	}
	return TRUE;
}

GummaState
gp_get_state ()
{
	cdrom_device_status_t stat;

	if (cd_try_open () &&
	    cdrom_get_status (device, &stat) == DISC_NO_ERROR) {
		switch (stat.audio_status) {
		case DISC_PLAY:
			return GUMMA_STATE_PLAYING;
		case DISC_PAUSED:
		        return GUMMA_STATE_PAUSED;
		case DISC_COMPLETED:
		case DISC_STOP:
		case DISC_ERROR:
			return GUMMA_STATE_STOPPED;
		}
	}
	return GUMMA_STATE_ERROR;
}


void
gp_do_verb (GummaVerb verb)
{
	cdrom_device_status_t stat;
	GummaState state;

	if(!cd_try_open ())
		return;

	state = gp_get_state ();

	switch (verb) {
	case GUMMA_VERB_PLAY:
		switch (state) {
		case GUMMA_STATE_PLAYING:
			break;
		case GUMMA_STATE_PAUSED:
			cdrom_resume (device);
			break;
		default:
			if (cdrom_get_status (device, &stat) == DISC_TRAY_OPEN)
				cdrom_load (device);
			cdrom_read_track_info (device);
			cdrom_play (device, device->track0,
				    device->track1);
			break;
		}
	case GUMMA_VERB_PAUSE:
		if (state == GUMMA_STATE_PLAYING)
			cdrom_pause (device);
		break;
	case GUMMA_VERB_STOP:
		cdrom_stop (device);
		break;
	case GUMMA_VERB_PREV:
		cdrom_prev (device);
		break;
	case GUMMA_VERB_NEXT:
		cdrom_next (device);
		break;
	case GUMMA_VERB_EJECT:
#if 0
		if (cdrom_get_status (device, &stat)==DISC_TRAY_OPEN)
			/*FIXME: if there is no disc, we get TRAY_OPEN even
			  if the tray is closed, is this a kernel bug?? or
			  is this the inteded behaviour, we don't support this
			  on solaris anyway, but unless we have a good way to
			  find out if the tray is actually open, we can't do this*/
			cdrom_load(device);
#endif
		cdrom_eject (device);
		break;
	}
	return;
}

GtkWidget *
gp_get_config_page ()
{
	return NULL;
}

void
gp_init ()
{
	int err;
	devpath = gnome_config_get_string("GUMMA/cd/devpath=/dev/cdrom");
	device = cdrom_open (devpath, &err);
	/*g_free (cd);*/
}

void
gp_denit ()
{
	if (device)
		cdrom_close (device);
}

void
gp_data_dropped (GtkSelectionData *data)
{
	return;
}

void
gp_get_time (GummaTimeInfo *tinfo)
{
	cdrom_device_status_t stat;
	
	g_return_if_fail (tinfo);

	if (cd_try_open () &&
            cdrom_get_status (device, &stat) == DISC_NO_ERROR) {
                switch (stat.audio_status) {
                case DISC_PLAY:
                case DISC_PAUSED:
			tinfo->minutes = stat.relative_address.minute;
			tinfo->seconds = stat.relative_address.second;
			tinfo->track = stat.track;
			return;
		}
	}
	tinfo->minutes =
		tinfo->seconds =
		tinfo->track = 0;
}

void
gp_about ()
{
	static const char *authors[] = {
		"Jacob Berkman  <jberkman@andrew.cmu.edu>",
		NULL
	};
	GtkWidget *about_box;

	about_box = gnome_about_new("Compact Disc GUMMA Plugin", "0.1",
				    "Copyright (C) 1999 The Free Software Foundation",
				    authors,
				    "Plugin for playing CD's via GUMMA\n\n"
				    "Based on the GNOME cdplayer applet.",
				    NULL);

	gtk_widget_show(about_box);
}
