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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>
#include <panel-applet.h>

#include "http.h"

#include "gweather.h"
#include "gweather-pref.h"
#include "gweather-dialog.h"
#include "gweather-applet.h"


gboolean
gweather_applet_new(PanelApplet *applet, const gchar *iid, gpointer data)
{
	GWeatherApplet *gw_applet;

	gw_applet = g_new0(GWeatherApplet, 1);
		
    bindtextdomain (PACKAGE, GNOMELOCALEDIR);
    textdomain (PACKAGE);

#if 0
    g_thread_init(NULL);
    http_init();
#endif
	
	gw_applet->applet = applet;

    gweather_applet_create(gw_applet);

    gweather_pref_load("test_path", gw_applet);
    gweather_info_load("test_path", gw_applet);
    
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
			"GWeather Applet Factory",
			"0",
			gweather_applet_factory,
			NULL);
