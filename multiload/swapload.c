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

/* start a new instance of the swapload applet */
GtkWidget *
make_swapload_applet (const gchar *goad_id)
{
    GtkWidget *applet;
    LoadGraphProperties *prop_data;
    LoadGraph *g;

    /* create a new applet_widget */
    applet = applet_widget_new (goad_id);
    /* in the rare case that the communication with the panel
       failed, error out */
    if (!applet)
	g_error ("Can't create applet!\n");

    prop_data = g_memdup (&multiload_properties, sizeof (LoadGraphProperties));

    g = load_graph_new (APPLET_WIDGET (applet), 2, N_("Swap Load"),
			&multiload_properties.swapload, prop_data,
			multiload_properties.swapload.adj_data[0],
			multiload_properties.swapload.adj_data[1], GetSwap);

    applet_widget_add (APPLET_WIDGET(applet), g->frame);
    gtk_widget_show (applet);

    load_graph_start (g);

    applet_widget_register_stock_callback (APPLET_WIDGET(applet),
					   "properties",
					   GNOME_STOCK_MENU_PROP,
					   _("Default Properties..."),
					   multiload_properties_cb,
					   g);

    applet_widget_register_stock_callback (APPLET_WIDGET(applet),
					   "run_gtop",
					   GNOME_STOCK_MENU_INDEX,
					   _("Run gtop..."),
					   start_gtop_cb, NULL);
    
    applet_widget_set_tooltip(APPLET_WIDGET(applet), "Swap Load"); 
    
    return applet;
}
