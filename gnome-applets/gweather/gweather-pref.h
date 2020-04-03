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

#ifndef __GWEATHER_PREF_H_
#define __GWEATHER_PREF_H_

#include "gweather-applet.h"

G_BEGIN_DECLS

#define GWEATHER_TYPE_PREF (gweather_pref_get_type ())
G_DECLARE_FINAL_TYPE (GWeatherPref, gweather_pref, GWEATHER, PREF, GtkDialog)

GtkWidget	*gweather_pref_new	(GWeatherApplet *applet);

void set_access_namedesc (GtkWidget *widget, const gchar *name, const gchar *desc);

G_END_DECLS

#endif /* __GWEATHER_PREF_H */
