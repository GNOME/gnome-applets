/* GNOME multiload panel applet
 * (C) 1997 The Free Software Foundation
 *
 * Authors: Tim P. Gerla
 *          Martin Baulig
 *
 * With code from wmload.c, v0.9.2, apparently by Ryan Land, rland@bc1.com.
 *
 */

#include <stdio.h>
#include <config.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <config.h>
#include <gnome.h>
#include <gdk/gdkx.h>
#include <applet-widget.h>

void make_cpuload_applet  (const gchar *);
void make_memload_applet  (const gchar *);
void make_swapload_applet (const gchar *);

/* These are the arguments that our application supports.  */
static struct argp_option arguments[] =
{
	{ "cpu",  -1, NULL, 0, N_("Start in CPU mode"), 1 },
	{ "mem",  -1, NULL, 0, N_("Start in memory mode"), 1 },
	{ "swap", -1, NULL, 0, N_("Start in swapload mode"), 1 },
	{ NULL, 0, NULL, 0, NULL, 0 }
};

/* Forward declaration of the function that gets called when one of
   our arguments is recognized.  */
/* we ignore the arguments */
static error_t
parse_an_arg (int key, char *arg, struct argp_state *state)
{
	return 0;
}

/* This structure defines our parser.  It can be used to specify some
   options for how our parsing function should be called.  */
static struct argp parser =
{
	arguments,			/* Options.  */
	parse_an_arg,			/* The parser function.  */
	NULL,				/* Some docs.  */
	NULL,				/* Some more docs.  */
	NULL,				/* Child arguments -- gnome_init fills
					   this in for us.  */
	NULL,				/* Help filter.  */
	NULL				/* Translation domain; for the app it
					   can always be NULL.  */
};

static void
make_new_applet (const gchar *param)
{
	if (!strcmp (param, "--mem"))
		make_memload_applet (param);
	else if (!strcmp (param, "--swap"))
		make_swapload_applet (param);
	else
		make_cpuload_applet (param);
}

/*when we get a command to start a new widget*/
static void
applet_start_new_applet (const gchar *param, gpointer data)
{
	make_new_applet (param);
}

int
main (int argc, char **argv)
{
	gchar *param;

	/* Initialize the i18n stuff */
        bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain (PACKAGE);

	/*we make a param string, instead of first parsing the params with
	  argp, we will allways use a string, since the panel will only
	  give us a string */
	param = make_param_string (argc,argv);

	applet_widget_init ("multiload_applet", &parser, argc, argv, 0, NULL,
			    argv[0], TRUE, TRUE, applet_start_new_applet, NULL);

	make_new_applet (param);
	g_free (param);

	applet_widget_gtk_main ();
	
	return 0;
}
