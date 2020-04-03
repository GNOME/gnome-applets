/*
 * Copyright (C) 2015 Alberts Muktupāvels
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
 *
 * Authors:
 *     Alberts Muktupāvels <alberts.muktupavels@gmail.com>
 */

#ifndef NETSPEED_APPLET_H
#define NETSPEED_APPLET_H

#include <libgnome-panel/gp-applet.h>

#define NETSPEED_TYPE_APPLET netspeed_applet_get_type ()
G_DECLARE_FINAL_TYPE (NetspeedApplet, netspeed_applet,
                      NETSPEED, APPLET,
                      GpApplet)

GSettings   *netspeed_applet_get_settings            (NetspeedApplet *netspeed);

const gchar *netspeed_applet_get_current_device_name (NetspeedApplet *netspeed);
gchar       *netspeed_applet_get_auto_device_name    (void);

void         netspeed_applet_setup_about             (GtkAboutDialog *dialog);

#endif
