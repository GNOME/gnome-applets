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

static gpointer   cd_init (void);
static void       cd_denit (gpointer data);

static void       cd_do_verb (GummaVerb verb, gpointer data);
static void       cd_data_dropped (GtkSelectionData *selection, 
				   gpointer data);

static GummaState cd_get_state (gpointer data);
static void       cd_get_time (GummaTimeInfo *tinfo, gpointer data);

static void       cd_about (gpointer data);
static GtkWidget *cd_get_config_page (gpointer data);

GummaPlugin cd_plugin = {
	cd_init,
	cd_denit,
	cd_do_verb,
	cd_data_dropped,
	cd_get_state,
	cd_get_time,
	cd_about,
	cd_get_config_page
};

GummaPlugin *
get_plugin ()
{
	return &cd_plugin;
}

typedef struct {
	char *devpath;
	cdrom_device_t device;
} CDData;

static gboolean
cd_try_open (CDData *cd)
{
	int err;
	if(!cd->device) {
		cd->device = cdrom_open(cd->devpath, &err);
		return cd->device != NULL;
	}
	return TRUE;
}

static GummaState
cd_get_state (gpointer data)
{
	CDData *cd = data;
	cdrom_device_status_t stat;

	if (cd_try_open (cd) &&
	    cdrom_get_status (cd->device, &stat) == DISC_NO_ERROR) {
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


static void
cd_do_verb (GummaVerb verb, gpointer data)
{
	CDData *cd = data;
	cdrom_device_status_t stat;
	GummaState state;

	if(!cd_try_open (cd))
		return;

	state = cd_get_state (cd);

	switch (verb) {
	case GUMMA_VERB_PLAY:
		switch (state) {
		case GUMMA_STATE_PLAYING:
			break;
		case GUMMA_STATE_PAUSED:
			cdrom_resume (cd->device);
			break;
		default:
			if (cdrom_get_status (cd->device, &stat) == DISC_TRAY_OPEN)
				cdrom_load (cd->device);
			cdrom_read_track_info (cd->device);
			cdrom_play (cd->device, cd->device->track0,
				    cd->device->track1);
			break;
		}
	case GUMMA_VERB_PAUSE:
		if (state == GUMMA_STATE_PLAYING)
			cdrom_pause (cd->device);
		break;
	case GUMMA_VERB_STOP:
		cdrom_stop (cd->device);
		break;
	case GUMMA_VERB_PREV:
		cdrom_prev (cd->device);
		break;
	case GUMMA_VERB_NEXT:
		cdrom_next (cd->device);
		break;
	case GUMMA_VERB_FORWARD:
		cdrom_ff (cd->device);
		break;
	case GUMMA_VERB_REWIND:
		cdrom_rewind (cd->device);
		break;
	case GUMMA_VERB_EJECT:
#if 0
		if (cdrom_get_status (cd->device, &stat)==DISC_TRAY_OPEN)
			/*FIXME: if there is no disc, we get TRAY_OPEN even
			  if the tray is closed, is this a kernel bug?? or
			  is this the inteded behaviour, we don't support this
			  on solaris anyway, but unless we have a good way to
			  find out if the tray is actually open, we can't do this*/
			cdrom_load(device);
#endif
		cdrom_eject (cd->device);
		break;
	}
	return;
}

static GtkWidget *
cd_get_config_page (gpointer data)
{
	return NULL;
	data = NULL;
}

static gpointer 
cd_init ()
{
	int err;
	CDData *cd = g_new0 (CDData, 1);
	cd->devpath = gnome_config_get_string("GUMMA/cd/devpath=/dev/cdrom");
	cd->device = cdrom_open (cd->devpath, &err);
	return cd;
}

static void
cd_denit (gpointer data)
{
	CDData *cd = data;
	if (cd->device)
		cdrom_close (cd->device);
	g_free (cd->devpath);
	g_free (cd);
}

static void
cd_data_dropped (GtkSelectionData *selection, gpointer data)
{
	return;
	selection = NULL;
	data = NULL;
}

static void
cd_get_time (GummaTimeInfo *tinfo, gpointer data)
{
	CDData *cd = data;
	cdrom_device_status_t stat;
	
	g_return_if_fail (tinfo);

	if (cd_try_open (cd) &&
            cdrom_get_status (cd->device, &stat) == DISC_NO_ERROR) {
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

static void
cd_about (gpointer data)
{
	const char *authors[] = {
		"Jacob Berkman  <jberkman@andrew.cmu.edu>",
		NULL
	};
	static GtkWidget *about_box = NULL;

	if (about_box) {
		gdk_window_show (about_box->window);
		gdk_window_raise (about_box->window);
		return;
	}

	about_box = gnome_about_new ("Compact Disc GUMMA Plugin", VERSION,
				     "Copyright (C) 1999 The Free Software Foundation",
				     authors,
				     "Plugin for playing CD's via GUMMA\n\n"
				     "Based on the GNOME cdplayer applet by Ching Hui.",
				     NULL);

	gtk_widget_show (about_box);
	return;
	data = NULL;
}
