/*
 * Copyright (C) Papadimitriou Spiros <spapadim+@cs.cmu.edu>
 * Copyright (C) todd kulesza <fflewddur@dropline.net>
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

#ifndef __GWEATHER_APPLET_H_
#define __GWEATHER_APPLET_H_

#include <glib/gi18n-lib.h>

#include <libgweather/gweather.h>

#include <libgnome-panel/gp-applet.h>

G_BEGIN_DECLS

#define GWEATHER_TYPE_APPLET (gweather_applet_get_type ())
G_DECLARE_FINAL_TYPE (GWeatherApplet, gweather_applet, GWEATHER, APPLET, GpApplet)

struct _GWeatherApplet
{
  GpApplet           parent;
  GWeatherInfo      *gweather_info;

  GSettings         *lib_settings;
  GSettings         *applet_settings;

  GtkWidget         *container;
  GtkWidget         *box;
  GtkWidget         *label;
  GtkWidget         *image;

  gint               size;
  gint               timeout_tag;
  gint               suncalc_timeout_tag;

  GtkWidget         *pref_dialog;

  GtkWidget         *details_dialog;
};

void gweather_applet_create(GWeatherApplet *gw_applet);
gboolean timeout_cb (gpointer data);
gboolean suncalc_timeout_cb (gpointer data);
void gweather_update (GWeatherApplet *applet);

void gweather_applet_setup_about (GtkAboutDialog *dialog);

G_END_DECLS

#endif /* __GWEATHER_APPLET_H_ */
