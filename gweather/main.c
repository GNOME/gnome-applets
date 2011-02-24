/* $Id$ */

/*
 *  Papadimitriou Spiros <spapadim+@cs.cmu.edu>
 *
 *  This code released under the GNU GPL.
 *  Read the file COPYING for more information.
 *
 *  Main applet widget
 *
 */

#include <glib.h>
#include <config.h>
#include <gtk/gtk.h>
#include <panel-applet.h>

#define GWEATHER_I_KNOW_THIS_IS_UNSTABLE

#include <libgweather/gweather-gconf.h>
#include <libgweather/gweather-prefs.h>

#include "gweather.h"
#include "gweather-pref.h"
#include "gweather-dialog.h"
#include "gweather-applet.h"


static gboolean
gweather_applet_new(PanelApplet *applet, const gchar *iid, gpointer data)
{
	GWeatherApplet *gw_applet;

	char *prefs_key = panel_applet_get_preferences_key(applet);

	panel_applet_add_preferences(applet, "/schemas/apps/gweather/prefs", NULL);
	
	gw_applet = g_new0(GWeatherApplet, 1);   
	
	gw_applet->applet = applet;
	gw_applet->gweather_info = NULL;
	gw_applet->gconf = gweather_gconf_new(prefs_key);
	g_free (prefs_key);
    	gweather_applet_create(gw_applet);

    	gweather_prefs_load(&gw_applet->gweather_pref, gw_applet->gconf);
    
    	gweather_update(gw_applet);

    	return TRUE;
}

static gboolean
gweather_applet_factory(PanelApplet *applet,
							const gchar *iid,
							gpointer data)
{
	gboolean retval = FALSE;
	
	retval = gweather_applet_new(applet, iid, data);
	
	return retval;
}

PANEL_APPLET_OUT_PROCESS_FACTORY ("GWeatherAppletFactory",
				  PANEL_TYPE_APPLET,
				  gweather_applet_factory,
				  NULL)
