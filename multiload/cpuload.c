/* GNOME cpuload/memload panel applet
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

#include "global.h"

static gint
applet_save_session (GtkWidget *widget, char *privcfgpath,
		     char *globcfgpath, gpointer data)
{
#if 0
	save_cpu_properties (privcfgpath, &cpu_props);
#endif
	return FALSE;
}

/* start a new instance of the cpuload applet */
GtkWidget *
make_cpuload_applet (const gchar *goad_id)
{
    GtkWidget *applet;
    LoadGraph *g;
    
    /* create a new applet_widget */
    applet = applet_widget_new(goad_id);
    /* in the rare case that the communication with the panel
       failed, error out */
    if (!applet)
	g_error ("Can't create applet!\n");

#if 0
    load_cpu_properties (APPLET_WIDGET(applet)->privcfgpath, &cpu_props);
#endif

    g = load_graph_new (4, N_("CPU Load"), &multiload_properties.cpuload,
			500, 40, 40, GetLoad);

    applet_widget_add (APPLET_WIDGET(applet), g->frame);
    gtk_widget_show (applet);

    load_graph_start (g);

    gtk_signal_connect (GTK_OBJECT(applet),"save_session",
			GTK_SIGNAL_FUNC(applet_save_session),
			NULL);

    applet_widget_register_stock_callback (APPLET_WIDGET(applet),
					   "properties",
					   GNOME_STOCK_MENU_PROP,
					   _("Properties..."),
					   multiload_properties_cb,
					   g);

    return applet;
}
