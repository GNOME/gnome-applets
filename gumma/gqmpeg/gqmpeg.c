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

static void parse_input (gpointer data, gint source,
			 GdkInputCondition condition);

struct {
	char *fname;
	FILE *fp;
	gint fd;
	gint gdk;
} outfile, infile;

static GummaState state;
static gint seconds;
static gint track;
static gboolean connected = FALSE;

static gchar *cmds[] = { "play",
			 "pause",
			 "stop",
			 NULL,
			 "next",
			 "prev" };

GtkWidget *
gp_get_config_page ()
{
	return NULL;
}

static void
close_infile ()
{
	g_message ("closing \"%s\"", infile.fname);
	connected = FALSE;
	gdk_input_remove (infile.gdk);
	fclose (infile.fp);
	close (infile.fd);
	unlink (infile.fname);
}


static gboolean
is_connected ()
{
	char *s;

	if (connected) return TRUE;
	
	if (mkfifo (infile.fname, S_IRUSR | S_IWUSR) != 0) {
		perror ("couldn't make fifo");
		return FALSE;
	}

	if ( (infile.fd = open (infile.fname, O_RDONLY | O_NONBLOCK)) == -1) {
		perror ("open failed");
		return FALSE;
	}

	infile.fp = fdopen (infile.fd, "r");

	if ( (outfile.fd = open (outfile.fname, O_WRONLY | O_NONBLOCK)) == -1) {
		/*perror ("cannot open gqmpeg command file");*/
		fclose (infile.fp);
		unlink (infile.fname);
		return FALSE;
	}

	outfile.fp = fdopen (outfile.fd, "w");

	s = g_strdup_printf ("status_add \"gumma\" \"%s\"", infile.fname);
	fputs (s, outfile.fp);
	fclose (outfile.fp);
	
	infile.gdk = gdk_input_add (infile.fd, GDK_INPUT_READ | GDK_INPUT_EXCEPTION, 
				    (GdkInputFunction) parse_input,
				    infile.fp);	
	return (connected = TRUE);
}

static void
open_infile ()
{
	outfile.fp = infile.fp = NULL;
	outfile.fd = infile.fd = 0;
	outfile.gdk = infile.gdk = 0;

	if (g_file_exists (infile.fname)) {
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
			unlink (infile.fname);
		else
			return;
	}
	
	is_connected ();
}

void
gp_do_verb (GummaVerb verb)
{
	g_return_if_fail (outfile.fname);

	if (!is_connected ()) return;
	if (!cmds[verb]) return;

	if ( !(outfile.fp = fopen (outfile.fname, "w")) ) {
		perror ("cannot open gqmpeg command file");
		return;
	}
	fputs (cmds[verb], outfile.fp);
	fclose (outfile.fp);
}

#define BUFLEN 2048
static char buf[BUFLEN];

static void
parse_input (gpointer data, gint source,
	     GdkInputCondition condition)
{
	char *p;
	FILE *fp = data;

	if (condition == GDK_INPUT_EXCEPTION) {
		g_message ("exception!!");
		return;
	}
	
	p  = fgets (buf, BUFLEN-1, fp);
	if (!p) {
		g_message ("no input!!");
	}

	g_message ("got string: %s", buf);

	if (!strncmp (buf, "time: ", 6)) {
		p = buf+6;
		if (!strncmp (p, "play ", 5)) {
			state = GUMMA_STATE_PLAYING;
			p += 5;
		} else if (!strncmp (buf+6, "stop ", 5)) {
			state = GUMMA_STATE_STOPPED;
			p += 5;
		} else if (!strncmp (buf+6, "pause ", 6)) {
			state = GUMMA_STATE_PAUSED;
			p += 6;
		}
		
		seconds = atoi (p);
	} else if (!strncmp (buf, "track: ", 7)) {
		p = buf + 7;
		track = atoi (p);
	} else if (!strncmp (buf, "close", 5) ||
		   !strncmp (buf, "exit", 4)) {
		state = GUMMA_STATE_ERROR;
		close_infile ();
		open_infile ();
	}
#if 0
	else if (!strcmp (buf, "name:", 6)) {
		
	}
#endif
}

void 
gp_init ()
{
	outfile.fname = gnome_util_prepend_user_home (".gqmpeg/command");

	infile.fname = gnome_util_prepend_user_home (".gnome/.gumma-gqmpeg-ctl");

	open_infile ();
	return;
}

void
gp_denit ()
{
	if ( !(outfile.fp = fopen (outfile.fname, "w")) ) {
		perror ("cannot open gqmpeg command file");
		return;
	}
	fputs ("status_rm \"gumma\"", outfile.fp);
	fclose (outfile.fp);	

	close_infile ();

	g_free (outfile.fname);
	g_free (infile.fname);
}

GummaState
gp_get_state ()
{
	return is_connected () ? state : GUMMA_STATE_ERROR;
}

void
gp_data_dropped (GtkSelectionData *data)
{
	return;
}

void
gp_get_time (GummaTimeInfo *tinfo)
{
	g_return_if_fail (tinfo);
	if (is_connected ()) {
		tinfo->minutes = seconds / 60;
		tinfo->seconds = seconds % 60;
		tinfo->track = track;
	} else {
		tinfo->minutes = tinfo->seconds = tinfo->track = 0;
	}
}

void
gp_about ()
{
	static const char *authors[] = {
		"Jacob Berkman  <jberkman@andrew.cmu.edu>",
		NULL
	};
	GtkWidget *about_box;

	about_box = gnome_about_new("gqmpeg GUMMA Plugin", "0.1",
				    "Copyright (C) 1999 The Free Software Foundation",
				    authors,
				    "Plugin for controlling gqmpeg via GUMMA",
				    NULL);

	gtk_widget_show(about_box);
}
