/* GNOME cpuload/memload panel applet
 * (C) 1997 The Free Software Foundation
 *
 * Authors: Tim P. Gerla
 *          Martin Baulig
 *          Todd Kulesza
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

#include <gtk/gtk.h>
#include <libgnomeui/libgnomeui.h>
#include <libgnome/libgnome.h>
#include <libgnomeui/gnome-window-icon.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <libart_lgpl/libart.h>

#include "global.h"

static const gchar *swap_texts [2] = {
    N_("Used"), N_("Free")
};

/* start a new instance of the swapload applet */
LoadGraph *
swapload_applet_new(PanelApplet *applet, gpointer data)
{
	LoadGraph *g;
	
	g = load_graph_new(applet, 2, N_("Swap Load"), 
					panel_applet_gconf_get_int(applet, "swapload_speed", NULL), 
					panel_applet_gconf_get_int(applet, "swapload_size", NULL), 
					panel_applet_gconf_get_bool(applet, "view_swapload", NULL), 
					"swapload", 
					GetSwap);
	
	return g;
}
