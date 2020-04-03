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

#ifndef GEYES_APPLET_H
#define GEYES_APPLET_H

#include <libgnome-panel/gp-applet.h>

G_BEGIN_DECLS

#define EYES_TYPE_APPLET (eyes_applet_get_type ())
G_DECLARE_FINAL_TYPE (EyesApplet, eyes_applet, EYES, APPLET, GpApplet)

void eyes_applet_setup_about (GtkAboutDialog *dialog);

G_END_DECLS

#endif
