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

#ifndef GWEATHER_DIALOG_H
#define GWEATHER_DIALOG_H

#include "gweather-applet.h"

G_BEGIN_DECLS

#define GWEATHER_TYPE_DIALOG (gweather_dialog_get_type ())
G_DECLARE_FINAL_TYPE (GWeatherDialog, gweather_dialog, GWEATHER, DIALOG, GtkDialog)

GtkWidget *gweather_dialog_new    (GWeatherApplet *applet);
void       gweather_dialog_update (GWeatherDialog *dialog);

G_END_DECLS

#endif
