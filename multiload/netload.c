/* GNOME cpuload/memload panel applet
 * (C) 1997 The Free Software Foundation
 *
 * Authors: Eric S. Raymond
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

#include <gtk/gtk.h>
#include <libgnomeui/libgnomeui.h>
#include <libgnome/libgnome.h>
#include <libgnomeui/gnome-window-icon.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <libart_lgpl/libart.h>

#include "global.h"

static const gchar *net_texts [4] = {
    N_("SLIP"), N_("PPP"), N_("ETH"), N_("Other"),
};

/* start a new instance of the netload applet */
LoadGraph *
netload_applet_new(PanelApplet *applet, gpointer data)
{
	LoadGraph *g;
	
	g = load_graph_new(applet, 4, N_("Net Load"), 
					panel_applet_gconf_get_int(applet, "netload_speed", NULL), 
					panel_applet_gconf_get_int(applet, "netload_size", NULL), 
					panel_applet_gconf_get_bool(applet, "view_netload", NULL), 
					"netload", 
					GetNet);
	
	return g;
}

