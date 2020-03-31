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

#ifndef ACCESSX_STATUS_APPLET_H
#define ACCESSX_STATUS_APPLET_H

#include <libgnome-panel/gp-applet.h>

G_BEGIN_DECLS

#define ACCESSX_STATUS_TYPE_APPLET (accessx_status_applet_get_type ())
G_DECLARE_FINAL_TYPE (AccessxStatusApplet, accessx_status_applet,
                      ACCESSX_STATUS, APPLET, GpApplet)

void accessx_status_applet_setup_about (GtkAboutDialog *dialog);

G_END_DECLS

#endif
