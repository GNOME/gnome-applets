/* GNOME multiload panel applet
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
#include <bonobo/bonobo-shlib-factory.h>

#include "global.h"

/* run gtop (better location for this function?) */
void 
start_gtop_cb (BonoboUIComponent *widget, gpointer data, const gchar *name)
{
    gnome_execute_shell(NULL, "gtop");
    return;
    widget = NULL;
    data = NULL;
}

void
start_procman_cb (BonoboUIComponent *uic, gpointer data, const gchar *name)
{
	gnome_execute_shell(NULL, "procman");
	return;
}
              
/* show help for the applet */

void
multiload_help_cb (BonoboUIComponent *uic, gpointer data, const gchar *name)
{
    gnome_help_display(data, NULL, NULL);
    return;
}


static gboolean
multiload_factory (PanelApplet *applet,
				const gchar *iid,
				gpointer data)
{
	gboolean retval = FALSE;

	g_print ("multiload_factory: %s\n", iid);
	
	if (!strcmp (iid, "OAFIID:GNOME_CPULoadApplet"))
		retval = cpuload_applet_new(applet);
	if (!strcmp (iid, "OAFIID:GNOME_MemoryLoadApplet"))
		retval = memload_applet_new(applet);
	if (!strcmp (iid, "OAFIID:GNOME_NetLoadApplet"))
		retval = netload_applet_new(applet);
	if (!strcmp (iid, "OAFIID:GNOME_SwapLoadApplet"))
		retval = swapload_applet_new(applet);
	if (!strcmp (iid, "OAFIID:GNOME_AvgLoadApplet"))
		retval = loadavg_applet_new(applet);

	return retval;
}

PANEL_APPLET_BONOBO_SHLIB_FACTORY ("OAFIID:GNOME_MultiLoadApplet_Factory",
				   "MultiLoad Applet factory",
				   multiload_factory, NULL);