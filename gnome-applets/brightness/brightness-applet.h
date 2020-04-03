/*
 * Copyright (C) 2020 Alberts Muktupāvels
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef BRIGHTNESS_APPLET_H
#define BRIGHTNESS_APPLET_H

#include <libgnome-panel/gp-applet.h>

G_BEGIN_DECLS

#define GPM_TYPE_BRIGHTNESS_APPLET (gpm_brightness_applet_get_type ())
G_DECLARE_FINAL_TYPE (GpmBrightnessApplet, gpm_brightness_applet,
                      GPM, BRIGHTNESS_APPLET, GpApplet)

void gpm_brightness_applet_setup_about (GtkAboutDialog *dialog);

G_END_DECLS

#endif
