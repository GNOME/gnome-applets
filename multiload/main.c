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

static int start_cpuload = 0, start_memload = 0, start_swapload = 0;

static const struct poptOption options[] = {
  {"cpu", '\0', POPT_ARG_NONE, &start_cpuload, 0, N_("CPU load graph")},
  {"mem", '\0', POPT_ARG_NONE, &start_memload, 0, N_("Memory usage graph")},
  {"swap", '\0', POPT_ARG_NONE, &start_swapload, 0, N_("Swap load graph")},
  {NULL, '\0', 0, NULL, 0}
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
	gchar *param = NULL, **args;
	poptContext ctx;

	/* Initialize the i18n stuff */
        bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain (PACKAGE);

	applet_widget_init ("multiload_applet", VERSION, argc, argv, options, 0,
			    NULL, TRUE, TRUE, applet_start_new_applet, NULL);

	/*we make a param string, instead of first parsing the params with
	  argp, we will allways use a string, since the panel will only
	  give us a string */
	param = g_strjoinv(" ", argv+1);

	if(start_cpuload)
	  make_cpuload_applet(param);

	if(start_memload)
	  make_memload_applet(param);

	if(start_swapload)
	  make_swapload_applet(param);

	g_free (param);

	applet_widget_gtk_main ();
	
	return 0;
}
