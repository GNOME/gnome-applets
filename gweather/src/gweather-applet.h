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

#ifndef __GWEATHER_APPLET_H_
#define __GWEATHER_APPLET_H_

#define GWEATHER_I_KNOW_THIS_IS_UNSTABLE

#include "gweather.h"

G_BEGIN_DECLS

void gweather_applet_create(GWeatherApplet *gw_applet);
gboolean timeout_cb (gpointer data);
gboolean suncalc_timeout_cb (gpointer data);
void gweather_update (GWeatherApplet *applet);

G_END_DECLS

#endif /* __GWEATHER_APPLET_H_ */

