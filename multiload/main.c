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

extern GtkWidget* make_cpuload_applet  (const gchar *);
extern GtkWidget* make_memload_applet  (const gchar *);
extern GtkWidget* make_swapload_applet (const gchar *);


static GtkWidget *
make_new_applet (const gchar *goad_id)
{
  if (!strcmp (goad_id, "multiload_memload_applet"))
    return make_memload_applet (goad_id);
  else if (!strcmp (goad_id, "multiload_swapload_applet"))
    return make_swapload_applet (goad_id);
  else if(!strcmp(goad_id, "multiload_cpuload_applet"))
    return make_cpuload_applet (goad_id);
}

int
main (int argc, char **argv)
{
	const char *goad_id;

	/* Initialize the i18n stuff */
        bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain (PACKAGE);

	applet_widget_init ("multiload_applet", VERSION, argc, argv, NULL, 0,
			    NULL);

	applet_factory_new("multiload_applet", NULL,
			   (AppletFactoryActivator)make_cpuload_applet);

	goad_id = goad_server_activation_id();
	if(!goad_id)
		make_new_applet("multiload_cpuload_applet");
	else
		make_new_applet(goad_id);

	applet_widget_gtk_main ();
	
	return 0;
}
