/* diskload
 * (c) 2005, Davyd Madeley <davyd@madeley.id.au>
 *
 * Authors: Davyd Madeley
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
diskload_applet_new(PanelApplet *applet, gpointer data)
{
	LoadGraph *g;
	gint speed, size;
	GError *error = NULL;

	g_print ("diskload_applet_new()\n");
	
	speed = panel_applet_gconf_get_int(applet, "speed", &error);
	if (error) {
		g_print ("%s \n", error->message);
		g_error_free (error);
		error = NULL;
	}
	speed = MAX (speed, 50);
	size = panel_applet_gconf_get_int(applet, "size", NULL);
	size = CLAMP (size, 10, 400); 
	g = load_graph_new(applet, 3, N_("Disk Load"), 
					speed, 
					size, 
					panel_applet_gconf_get_bool (applet,
						"view_diskload", NULL),
					"diskload", 
					GetDiskLoad);
	
	return g;
}

