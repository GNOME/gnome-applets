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

#include <xmms/xmmsctrl.h>

#define GUMMA_PLUGIN
#include <gumma/gumma.h>

static gpointer   xmms_init (void);
static void       xmms_denit (gpointer data);

static void       xmms_do_verb (GummaVerb verb, gpointer data);
static void       xmms_data_dropped (GtkSelectionData *selection, 
				   gpointer data);

static GummaState xmms_get_state (gpointer data);
static void       xmms_get_time (GummaTimeInfo *tinfo, gpointer data);

static void       xmms_about (gpointer data);
static GtkWidget *xmms_get_config_page (gpointer data);

GummaPlugin xmms_plugin = {
	xmms_init,
	xmms_denit,
	xmms_do_verb,
	xmms_data_dropped,
	xmms_get_state,
	xmms_get_time,
	xmms_about,
	xmms_get_config_page
};

GummaPlugin *
get_plugin () {
	return &xmms_plugin;
}

typedef struct {
	int session;
} XmmsData;

typedef void (*xmms_func) (gint);
static xmms_func cmds[] = { xmms_remote_play,
			    xmms_remote_pause,
			    xmms_remote_stop,
			    xmms_remote_eject,
			    xmms_remote_playlist_next,
			    xmms_remote_playlist_prev,
			    NULL,
			    NULL };

static void
xmms_do_verb (GummaVerb verb, gpointer data)
{
	XmmsData *xmms = data;
	gint time;
	switch (verb) {
	case GUMMA_VERB_FORWARD:
		time = xmms_remote_get_output_time (xmms->session);
		xmms_remote_jump_to_time (xmms->session, time + 1000);
		break;
	case GUMMA_VERB_REWIND:
		time = xmms_remote_get_output_time (xmms->session);
		xmms_remote_jump_to_time (xmms->session, time - 1000);
		break;
	default:
		if (!cmds[verb]) return;
		cmds[verb] (xmms->session);
		break;
	}
}

static GtkWidget *
xmms_get_config_page (gpointer data)
{
	return NULL;
}

static gpointer
xmms_init ()
{
	XmmsData *xmms = g_new0 (XmmsData, 1);
	return xmms;
}

static void
xmms_denit (gpointer data) { 
	XmmsData *xmms = data;
	g_free (xmms);
}

static GummaState
xmms_get_state (gpointer data)
{
	XmmsData *xmms = data;
	if (!xmms_remote_is_running (xmms->session))
		return GUMMA_STATE_ERROR;
	else if (xmms_remote_is_playing (xmms->session))
		return GUMMA_STATE_PLAYING;
	else if (xmms_remote_is_paused (xmms->session))
		return GUMMA_STATE_PAUSED;
	else
		return GUMMA_STATE_STOPPED;
}

static void
xmms_data_dropped (GtkSelectionData *selection, gpointer data)
{
	return;
}

static void
xmms_get_time (GummaTimeInfo *tinfo, gpointer data)
{
	XmmsData *xmms = data;
	gint t;
	g_return_if_fail (tinfo);
	t = xmms_remote_get_output_time (xmms->session) / 1000;
	tinfo->minutes = t / 60;
	tinfo->seconds = t % 60;
	tinfo->track = 0; /*xmms_remote_get_playlist_time (xmms_session);*/
}

static void
xmms_about (gpointer data)
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

	about_box = gnome_about_new("XMMS plugin for GUMMA", "0.1",
				    "Copyright (C) 1999 The Free Software Foundation",
				    authors,
				    "Plugin for controlling xmms via GUMMA\n\n"
				    "Originally based on gnomexmms by Anders Carlsson\n"
				    "Which is baased on wmxmms by Mikael Alm.",
				    NULL);

	gtk_widget_show(about_box);
}
