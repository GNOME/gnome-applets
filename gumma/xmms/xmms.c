/*
 * GNOME Universal MultiMedia Applet
 * (C) 1999 The Free Software Foundation
 *
 * Authors: Jacob Berkman  <jberkman@andrew.cmu.edu>
 *
 * Plugin to control xmms
 */

#include <config.h>

#include <gnome.h>

#define GUMMA_PLUGIN
#include <gumma/gumma.h>

#include <xmms/xmmsctrl.h>

static int xmms_session=0;

typedef void (*xmms_func) (gint);

static xmms_func cmds[] = { xmms_remote_play,
			    xmms_remote_pause,
			    xmms_remote_stop,
			    xmms_remote_eject,
			    xmms_remote_playlist_next,
			    xmms_remote_playlist_prev };

void
gp_do_verb (GummaVerb verb)
{
	if (!cmds[verb]) return;
	cmds[verb] (xmms_session);
}

GtkWidget *
gp_get_config_page ()
{
	return NULL;
}

void 
gp_init ()
{
	xmms_session = 0;
}

void
gp_denit () { }

GummaState
gp_get_state ()
{
	if (!xmms_remote_is_running (xmms_session))
		return GUMMA_STATE_ERROR;
	else if (xmms_remote_is_playing (xmms_session))
		return GUMMA_STATE_PLAYING;
	else if (xmms_remote_is_paused (xmms_session))
		return GUMMA_STATE_PAUSED;
	else
		return GUMMA_STATE_STOPPED;
}

void
gp_data_dropped (GtkSelectionData *data)
{
	return;
}

void
gp_get_time (GummaTimeInfo *tinfo)
{
	gint t;
	g_return_if_fail (tinfo);
	t = xmms_remote_get_output_time (xmms_session) / 1000;
	tinfo->minutes = t / 60;
	tinfo->seconds = t % 60;
	tinfo->track = 0; /*xmms_remote_get_playlist_time (xmms_session);*/
}

void
gp_about ()
{
	static const char *authors[] = {
		"Jacob Berkman  <jberkman@andrew.cmu.edu>",
		NULL
	};
	GtkWidget *about_box;

	about_box = gnome_about_new("XMMS plugin for GUMMA", "0.1",
				    "Copyright (C) 1999 The Free Software Foundation",
				    authors,
				    "Plugin for controlling xmms via GUMMA\n\n"
				    "Originally based on gnomexmms by Anders Carlsson\n"
				    "Which is baased on wmxmms by Mikael Alm.",
				    NULL);

	gtk_widget_show(about_box);
}
