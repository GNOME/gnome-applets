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
#include <config.h>
#include <gnome.h>
#include <gdk/gdkx.h>
#include <applet-widget.h>

#include "global.h"

static const char *loadavg_texts [3] = {
    N_("Load average over 1 minute"),
    N_("Load average over 5 minutes"),
    N_("Load average over 15 minutes")
};

static void
loadavg_1_cb (AppletWidget *widget, gpointer data)
{
    LoadGraph *g = (LoadGraph *) data;

    g->prop_data->loadavg_type = LOADAVG_1;

    gnome_property_object_list_walk
	(g->local_prop_data->local_property_object_list,
	 GNOME_PROPERTY_ACTION_UPDATE);

    gnome_property_object_list_walk
	(g->local_prop_data->local_property_object_list,
	 GNOME_PROPERTY_ACTION_SAVE);

    applet_widget_set_tooltip (g->applet, _(loadavg_texts [0]));
}

static void
loadavg_5_cb (AppletWidget *widget, gpointer data)
{
    LoadGraph *g = (LoadGraph *) data;

    g->prop_data->loadavg_type = LOADAVG_5;

    gnome_property_object_list_walk
	(g->local_prop_data->local_property_object_list,
	 GNOME_PROPERTY_ACTION_UPDATE);

    gnome_property_object_list_walk
	(g->local_prop_data->local_property_object_list,
	 GNOME_PROPERTY_ACTION_SAVE);

    applet_widget_set_tooltip (g->applet, _(loadavg_texts [1]));
}

static void
loadavg_15_cb (AppletWidget *widget, gpointer data)
{
    LoadGraph *g = (LoadGraph *) data;

    g->prop_data->loadavg_type = LOADAVG_15;

    gnome_property_object_list_walk
	(g->local_prop_data->local_property_object_list,
	 GNOME_PROPERTY_ACTION_UPDATE);

    gnome_property_object_list_walk
	(g->local_prop_data->local_property_object_list,
	 GNOME_PROPERTY_ACTION_SAVE);

    applet_widget_set_tooltip (g->applet, _(loadavg_texts [2]));
}

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
	(_("Load Average Applet"), VERSION,
	 "(C) 1999",
	 authors,
	 _("Released under the GNU general public license.\n\n"
	   "Load Average Meter Applet."),
	 NULL);

    gtk_signal_connect (GTK_OBJECT (about), "destroy",
			GTK_SIGNAL_FUNC (gtk_widget_destroyed), &about);

    gtk_widget_show (about);
}

/* start a new instance of the loadavg applet */
GtkWidget *
make_loadavg_applet (const gchar *goad_id)
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

    prop_data = g_memdup (&multiload_properties.loadavg,
			  sizeof (LoadGraphProperties));

    g = load_graph_new (APPLET_WIDGET (applet), 2, N_("Load Average"),
			&multiload_properties.loadavg, prop_data,
			multiload_properties.loadavg.adj_data[0],
			multiload_properties.loadavg.adj_data[1], GetLoadAvg,
			"index.html#LOADAVG-PROPERTIES");

    applet_widget_add (APPLET_WIDGET(applet), g->main_widget);
    gtk_widget_show (applet);

    load_graph_start (g);

    applet_widget_register_callback_dir (APPLET_WIDGET (applet),
					 "loadavg/", _("Load Average"));

    applet_widget_register_callback (APPLET_WIDGET (applet),
				     "loadavg/load_1", _(loadavg_texts [0]),
				     loadavg_1_cb, g);

    applet_widget_register_callback (APPLET_WIDGET (applet),
				     "loadavg/load_5", _(loadavg_texts [1]),
				     loadavg_5_cb, g);

    applet_widget_register_callback (APPLET_WIDGET (applet),
				     "loadavg/load_15", _(loadavg_texts [2]),
				     loadavg_15_cb, g);

    applet_widget_register_stock_callback (APPLET_WIDGET (applet),
					   "properties",
					   GNOME_STOCK_MENU_PROP,
					   _("Default Properties..."),
					   multiload_properties_cb,
					   g);

    applet_widget_register_stock_callback (APPLET_WIDGET (applet),
					   "local_properties",
					   GNOME_STOCK_MENU_PROP,
					   _("Properties..."),
					   multiload_local_properties_cb,
					   g);

    applet_widget_register_stock_callback (APPLET_WIDGET (applet), 
					   "run_gtop",
					   GNOME_STOCK_MENU_INDEX,
					   _("Run gtop..."),
					   start_gtop_cb, NULL);

    applet_widget_register_stock_callback (APPLET_WIDGET(applet),
					   "help",
					   GNOME_STOCK_PIXMAP_HELP,
					   _("Help"),
					   multiload_help_cb,
					   "loadavg_applet");

    applet_widget_register_stock_callback (APPLET_WIDGET(applet),
					   "about",
					   GNOME_STOCK_MENU_ABOUT,
					   _("About..."),
					   about_cb, NULL);

    applet_widget_set_tooltip (APPLET_WIDGET (applet),
			       _(loadavg_texts [prop_data->loadavg_type]));

    return applet;
}
