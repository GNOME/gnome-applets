/* GNOME cpuload/memload panel applet
 * (C) 1997 The Free Software Foundation
 *
 * Authors: Tim P. Gerla
 *          Martin Baulig
 *
 * With code from wmload.c, v0.9.2, apparently by Ryan Land, rland@bc1.com.
 *
 */

#include <config.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <glib.h>
#include <gnome.h>
#include <gdk/gdkx.h>
#include <applet-widget.h>

#include "global.h"

static gint
applet_save_session (GtkWidget *widget, char *privcfgpath,
		     char *globcfgpath, gpointer data)
{
	return FALSE;
}

/* start a new instance of the pageload applet */
GtkWidget *
make_pageload_applet (const gchar *goad_id)
{
    GtkWidget *applet;
    LoadGraphProperties *prop_data;
    LoadGraph *g;

    /* create a new applet_widget */
    applet = applet_widget_new(goad_id);
    /* in the rare case that the communication with the panel
       failed, error out */
    if (!applet)
	g_error ("Can't create applet!\n");

    prop_data = g_memdup (&multiload_properties.pageload,
			   sizeof(LoadGraphProperties));
			   
    g = load_graph_new (APPLET_WIDGET (applet), 3, N_("PAGE Load"),
			&multiload_properties.pageload, prop_data,
			multiload_properties.pageload.adj_data[0],
			multiload_properties.pageload.adj_data[1],
			multiload_properties.pageload.adj_data[2], GetPage);

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

    applet_widget_register_callback (APPLET_WIDGET(applet),
				     "run_gtop",
				     _("Run gtop..."),
				     start_gtop_cb,
				     NULL);

    return applet;
}
