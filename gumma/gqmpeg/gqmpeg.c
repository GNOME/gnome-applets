/*
 * GNOME Universal MultiMedia Applet
 * (C) 1999 The Free Software Foundation
 *
 * Authors: Jacob Berkman  <jberkman@andrew.cmu.edu>
 *
 * Plugin to control gqmpeg
 */

#include <config.h>

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <gnome.h>

#define GUMMA_PLUGIN
#include <gumma/gumma.h>

static gpointer   gqmpeg_init (void);
static void       gqmpeg_denit (gpointer data);

static void       gqmpeg_do_verb (GummaVerb verb, gpointer data);
static void       gqmpeg_data_dropped (GtkSelectionData *selection, 
				   gpointer data);

static GummaState gqmpeg_get_state (gpointer data);
static void       gqmpeg_get_time (GummaTimeInfo *tinfo, gpointer data);

static void       gqmpeg_about (gpointer data);
static GtkWidget *gqmpeg_get_config_page (gpointer data);

GummaPlugin gqmpeg_plugin = {
	gqmpeg_init,
	gqmpeg_denit,
	gqmpeg_do_verb,
	gqmpeg_data_dropped,
	gqmpeg_get_state,
	gqmpeg_get_time,
	gqmpeg_about,
	gqmpeg_get_config_page
};

GummaPlugin *
get_plugin ()
{
	return &gqmpeg_plugin;
}

static void parse_input (gpointer data, gint source,
			 GdkInputCondition condition);

typedef struct {
	char *ifname;
	char *ofname;

	gint ifd;
	FILE *ifp;
	gint gdk_input;

	GummaState state;
	gint seconds;
	gint track;
	gboolean connected;
} GqmpegData;

static gchar *cmds[] = { "play",
			 "pause",
			 "stop",
			 "show_playlist",
			 "next",
			 "prev",
			 "seek +1",
			 "seek -1" };

static GtkWidget *
gqmpeg_get_config_page (gpointer data)
{
	return NULL;
}

static void
close_infile (GqmpegData *gq)
{
	g_message ("closing \"%s\"", gq->ifname);
	gq->connected = FALSE;
	if (gq->gdk_input)
		gdk_input_remove (gq->gdk_input);
	if (gq->ifp)
		fclose (gq->ifp);
	if (gq->ifname)
		unlink (gq->ifname);
}


static gboolean
is_connected (GqmpegData *gq)
{
	char *s;
	int ofd;
	FILE *ofp;

	if (gq->connected) return TRUE;
	
	if (mkfifo (gq->ifname, S_IRUSR | S_IWUSR) != 0) {
		perror ("couldn't make fifo");
		return FALSE;
	}

	if ( (gq->ifd = open (gq->ifname, O_RDONLY | O_NONBLOCK)) == -1) {
		perror ("open failed");
		return FALSE;
	}

	gq->ifp = fdopen (gq->ifd, "r");

	if ( (ofd = open (gq->ofname, O_WRONLY | O_NONBLOCK)) == -1) {
		/*perror ("cannot open gqmpeg command file");*/
		fclose (gq->ifp);
		gq->ifp = NULL;
		unlink (gq->ifname);
		return FALSE;
	}

	ofp = fdopen (ofd, "w");

	s = g_strdup_printf ("status_add \"gumma\" \"%s\"", gq->ifname);
	fputs (s, ofp);
	fclose (ofp);
	
	gq->gdk_input = gdk_input_add (gq->ifd, GDK_INPUT_READ | GDK_INPUT_EXCEPTION, 
				       (GdkInputFunction) parse_input,
				       gq);	
	return (gq->connected = TRUE);
}

static void
open_infile (GqmpegData *gq)
{
	gq->ifp = NULL;
	gq->ifd = 0;
	gq->gdk_input = 0;

	if (g_file_exists (gq->ifname)) {
		GtkWidget *dialog;
		gint reply;
		dialog = gnome_message_box_new (_("~/.gnome/.gumma-gqmpeg-ctl already exists.\n\n"
						  "This file is created by GUMMA to get feedback\n"
						  "from gqmpeg.  If GUMMA has recently crashed,\n"
						  "then deleting this file is ok.  Otherwise,\n" 
						  "this plugin may not function correctly.\n\n"
						  "Delete this file?"),
						GNOME_MESSAGE_BOX_QUESTION,
						GNOME_STOCK_BUTTON_YES,
						GNOME_STOCK_BUTTON_NO,
						NULL);
		reply = gnome_dialog_run_and_close (GNOME_DIALOG (dialog));
		if (reply == GNOME_YES)
			unlink (gq->ifname);
		else
			return;
	}
	
	is_connected (gq);
}

static void
gqmpeg_do_verb (GummaVerb verb, gpointer data)
{
	FILE *ofp;
	int ofd;
	GqmpegData *gq = data;
	g_return_if_fail (gq->ofname);

	if (!is_connected (gq)) return;
	if (!cmds[verb]) return;
	
	if ( (ofd = open (gq->ofname, O_WRONLY | O_NONBLOCK)) == -1) {
		perror ("cannot open gqmpeg command file");
		return;
	}

	ofp = fdopen (ofd, "w");

	fputs (cmds[verb], ofp);
	fclose (ofp);
}

#define BUFLEN 2048
static char buf[BUFLEN];

static void
parse_input (gpointer data, gint source,
	     GdkInputCondition condition)
{
	GqmpegData *gq = data;
	char *p;

	if (condition == GDK_INPUT_EXCEPTION) {
		g_message ("exception!!");
		return;
	}
	
	p  = fgets (buf, BUFLEN-1, gq->ifp);
	if (!p) {
		g_message ("no input!!");
		return;
	}

	g_message ("got string: %s", buf);

	if (!strncmp (buf, "time: ", 6)) {
		p = buf+6;
		if (!strncmp (p, "play ", 5)) {
			gq->state = GUMMA_STATE_PLAYING;
			p += 5;
		} else if (!strncmp (buf+6, "stop ", 5)) {
			gq->state = GUMMA_STATE_STOPPED;
			p += 5;
		} else if (!strncmp (buf+6, "pause ", 6)) {
			gq->state = GUMMA_STATE_PAUSED;
			p += 6;
		}
		
		gq->seconds = atoi (p);
	} else if (!strncmp (buf, "track: ", 7)) {
		p = buf + 7;
		gq->track = atoi (p);
	} else if (!strncmp (buf, "close", 5) ||
		   !strncmp (buf, "exit", 4)) {
		gq->state = GUMMA_STATE_ERROR;
		close_infile (gq);
		open_infile (gq);
	}
#if 0
	else if (!strcmp (buf, "name:", 6)) {
		
	}
#endif
}

static gpointer
gqmpeg_init ()
{
	GqmpegData *gq = g_new0 (GqmpegData, 1);
	
	gq->ofname = gnome_util_prepend_user_home (".gqmpeg/command");
	gq->ifname = gnome_util_prepend_user_home (".gnome/.gumma-gqmpeg-ctl");

	open_infile (gq);

	return gq;
}

static void
gqmpeg_denit (gpointer data)
{
	GqmpegData *gq = data;
	int ofd;
	FILE *ofp;

	if ( (ofd = open (gq->ofname, O_WRONLY | O_NONBLOCK)) == -1) {
		perror ("cannot open gqmpeg command file");
		goto cleanup;
	}

	ofp = fdopen (ofd, "w");
	fputs ("status_rm \"gumma\"", ofp);
	fclose (ofp);	

 cleanup:
	close_infile (gq);

	g_free (gq->ofname);
	g_free (gq->ifname);
}

static GummaState
gqmpeg_get_state (gpointer data)
{
	GqmpegData *gq = data;
	return is_connected (gq) ? gq->state : GUMMA_STATE_ERROR;
}

static void
gqmpeg_data_dropped (GtkSelectionData *selection, gpointer data)
{
	return;
}

static void
gqmpeg_get_time (GummaTimeInfo *tinfo, gpointer data)
{
	GqmpegData *gq = data;
	g_return_if_fail (tinfo);
	if (is_connected (gq)) {
		tinfo->minutes = gq->seconds / 60;
		tinfo->seconds = gq->seconds % 60;
		tinfo->track = gq->track;
	} else {
		tinfo->minutes = tinfo->seconds = tinfo->track = 0;
	}
}

static void
gqmpeg_about (gpointer data)
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
	
	about_box = gnome_about_new ("gqmpeg GUMMA Plugin", VERSION,
				     "Copyright (C) 1999 The Free Software Foundation",
				     authors,
				     "Plugin for controlling gqmpeg via GUMMA",
				     NULL);
	
	gtk_widget_show (about_box);
}
