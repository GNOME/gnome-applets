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
#include <libgnomeui/gnome-window-icon.h>

#include "global.h"

static void
about_cb (AppletWidget *widget, gpointer data)
{
    static GtkWidget *about = NULL;
    const gchar *authors[8];

    if (about != NULL)
	{
	    gdk_window_show(about->window);
	    gdk_window_raise(about->window);
	    return;
	}

    authors[0] = "Martin Baulig <martin@home-of-linux.org>";
    authors[1] = NULL;

    about = gnome_about_new
	(_("Swap Load Applet"), VERSION,
	 "(C) 1999",
	 authors,
	 _("Released under the GNU general public license.\n\n"
	   "Swap Load Meter Applet."),
	 NULL);
    gnome_window_icon_set_from_file (GTK_WINDOW (about),
				     GNOME_ICONDIR"/gnome-mem.png");

    gtk_signal_connect (GTK_OBJECT (about), "destroy",
			GTK_SIGNAL_FUNC (gtk_widget_destroyed), &about);

    gtk_widget_show (about);
}

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
	g_error (_("Can't create applet!\n"));

    prop_data = g_memdup (&multiload_properties.swapload,
			  sizeof (LoadGraphProperties));

    g = load_graph_new (APPLET_WIDGET (applet), 2, N_("Swap Load"),
			&multiload_properties.swapload, prop_data,
			multiload_properties.swapload.adj_data[0],
			multiload_properties.swapload.adj_data[1], GetSwap,
			"index.html#SWAPLOAD-PROPERTIES");

    applet_widget_add (APPLET_WIDGET(applet), g->main_widget);
    gtk_widget_show (applet);

    load_graph_start (g);

    applet_widget_register_stock_callback (APPLET_WIDGET(applet),
					   "properties",
					   GNOME_STOCK_MENU_PROP,
					   _("Default Properties..."),
					   multiload_properties_cb,
					   g);

    applet_widget_register_stock_callback (APPLET_WIDGET(applet),
					   "local_properties",
					   GNOME_STOCK_MENU_PROP,
					   _("Properties..."),
					   multiload_local_properties_cb,
					   g);

    applet_widget_register_stock_callback (APPLET_WIDGET(applet),
					   "run_gtop",
					   GNOME_STOCK_MENU_INDEX,
					   _("Run gtop..."),
					   start_gtop_cb, NULL);
    
    applet_widget_register_stock_callback (APPLET_WIDGET(applet),
					   "help",
					   GNOME_STOCK_PIXMAP_HELP,
					   _("Help"),
					   multiload_help_cb,
					   "swapload_applet");

    applet_widget_register_stock_callback (APPLET_WIDGET(applet),
					   "about",
					   GNOME_STOCK_MENU_ABOUT,
					   _("About..."),
					   about_cb, NULL);

    applet_widget_set_tooltip(APPLET_WIDGET(applet), _("Swap Load")); 
    
    return applet;
}
