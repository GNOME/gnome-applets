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
#include <gnome.h>
#include <panel-applet.h>
#include <panel-applet-gconf.h>

#include <gtk/gtk.h>
#include <libgnomeui/libgnomeui.h>
#include <libgnome/libgnome.h>
#include <libgnomeui/gnome-window-icon.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <libart_lgpl/libart.h>

#include "global.h"

LoadGraph *
cpuload_applet_new(PanelApplet *applet, gpointer data)
{
	LoadGraph *g;
	
	g = load_graph_new(applet, 4, N_("CPU Load"), 
					panel_applet_gconf_get_int(applet, "speed", NULL), 
					panel_applet_gconf_get_int(applet, "size", NULL), 
					panel_applet_gconf_get_bool(applet, "view_cpuload", NULL), 
					"cpuload", 
					GetLoad);
	
	return g;
}

