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
#include <config.h>
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

LoadGraph *
cpuload_applet_new(PanelApplet *applet, gpointer data)
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
	g = load_graph_new(applet, 5, N_("CPU Load"), 
					speed, 
					size, 
					panel_applet_gconf_get_bool(applet, "view_cpuload", NULL), 
					"cpuload", 
					GetLoad);
	
	return g;
}

