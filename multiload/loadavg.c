/* GNOME cpuload/memload panel applet
 * (C) 1997 The Free Software Foundation
 *
 * Authors: Tim P. Gerla
 *          Martin Baulig
 *		  Todd Kulesza
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
#include <panel-applet.h>
#include <panel-applet-gconf.h>

#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "global.h"

/*
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
*/

/* start a new instance of the loadavg applet */
LoadGraph *
loadavg_applet_new(PanelApplet *applet, gpointer data)
{
	LoadGraph *g;
	gint speed, size;
	GError *error = NULL;
	
	speed = panel_applet_gconf_get_int(applet, "speed", &error);
	if (error) {
		g_print ("%s \n", error->message);
		g_error_free (error);
		error = NULL;
	}
	speed = MAX (speed, 50);
	size = panel_applet_gconf_get_int(applet, "size", NULL);
	
	size = CLAMP (size, 10, 400);
	
	g = load_graph_new(applet, 2, N_("Load Average"), speed, size, 
			   panel_applet_gconf_get_bool(applet, "view_loadavg", NULL), 
			   "loadavg", GetLoadAvg);
	
	return g;
}
