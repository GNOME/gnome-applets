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


static void
make_new_applet (const gchar *goad_id)
{
	if (!strcmp (goad_id, "multiload_memload_applet"))
		make_memload_applet (goad_id);
	else if (!strcmp (goad_id, "multiload_swapload_applet"))
		make_swapload_applet (goad_id);
	else
		make_cpuload_applet (goad_id);
}

/*when we get a command to start a new widget*/
static void
applet_start_new_applet (const gchar *goad_id, gpointer data)
{
	make_new_applet (goad_id);
}

int
main (int argc, char **argv)
{
	GList *list = NULL;
	char *goad_id;

	/* Initialize the i18n stuff */
        bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain (PACKAGE);


	list = g_list_prepend(list,"multiload_cpuload_applet");
	list = g_list_prepend(list,"multiload_memload_applet");
	list = g_list_prepend(list,"multiload_swapload_applet");
	applet_widget_init ("multiload_applet", VERSION, argc, argv, NULL, 0,
			    NULL, TRUE, list, applet_start_new_applet, NULL);
	g_list_free(list);

	goad_id = goad_server_activation_id();
	if(!goad_id)
		return 0;
	make_new_applet(goad_id);

	applet_widget_gtk_main ();
	
	return 0;
}
