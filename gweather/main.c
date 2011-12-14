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
#include <gio/gio.h>

#define GWEATHER_I_KNOW_THIS_IS_UNSTABLE

#include "gweather.h"
#include "gweather-pref.h"
#include "gweather-dialog.h"
#include "gweather-applet.h"


static gboolean
gweather_applet_new(PanelApplet *applet, const gchar *iid, gpointer data)
{
	GWeatherApplet *gw_applet;

	char *prefs_key = panel_applet_get_preferences_key(applet);

	gw_applet = g_new0(GWeatherApplet, 1);   
	
	gw_applet->applet = applet;
	gw_applet->gweather_info = NULL;
	gw_applet->lib_settings = g_settings_new("org.gnome.GWeather");
	gw_applet->applet_settings = panel_applet_settings_new(applet, "org.gnome.applets.GWeatherApplet");
	g_free (prefs_key);
    	gweather_applet_create(gw_applet);

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
