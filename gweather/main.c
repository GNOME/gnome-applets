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

#  include <config.h>

#include <gnome.h>
#include <panel-applet.h>

#include "gweather.h"
#include "gweather-pref.h"
#include "gweather-dialog.h"
#include "gweather-applet.h"

static void
initialize_info (WeatherInfo *info)
{
	info->main_handle = NULL;
	info->success = FALSE;
	info->xml = NULL;
	info->forecasts = NULL;
	
}

gboolean
gweather_applet_new(PanelApplet *applet, const gchar *iid, gpointer data)
{
	GWeatherApplet *gw_applet;

	panel_applet_add_preferences(applet, "/schemas/apps/gweather/prefs", NULL);
	
	gw_applet = g_new0(GWeatherApplet, 1);   
	
	gw_applet->applet = applet;
	gw_applet->gweather_info = g_new0 (WeatherInfo, 1);
	initialize_info (gw_applet->gweather_info);
	
    	gweather_pref_load(gw_applet);
	gweather_applet_create (gw_applet);
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

PANEL_APPLET_BONOBO_FACTORY("OAFIID:GNOME_GWeatherApplet_Factory",
			PANEL_TYPE_APPLET,
			"gweather",
			"0",
			gweather_applet_factory,
			NULL);
