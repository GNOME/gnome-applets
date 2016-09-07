/*
 * Copyright (C) Papadimitriou Spiros <spapadim+@cs.cmu.edu>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
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
	GWeatherApplet *gw_applet = g_new0(GWeatherApplet, 1);

	gw_applet->applet = applet;
	gw_applet->gweather_info = NULL;
	gw_applet->lib_settings = g_settings_new("org.gnome.GWeather");
	gw_applet->applet_settings = panel_applet_settings_new(applet, "org.gnome.gnome-applets.gweather");

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

PANEL_APPLET_IN_PROCESS_FACTORY ("GWeatherAppletFactory",
                                 PANEL_TYPE_APPLET,
                                 gweather_applet_factory,
                                 NULL)
