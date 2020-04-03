/*
 * Copyright (C) 2014 Sebastian Geiger
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
 *     Sebastian Geiger <sbastig@gmx.net>
 */

#ifndef WP_APPLET_H
#define WP_APPLET_H

#include <glib-object.h>
#include <libgnome-panel/gp-applet.h>

G_BEGIN_DECLS

#define WP_TYPE_APPLET (wp_applet_get_type ())

G_DECLARE_FINAL_TYPE (WpApplet, wp_applet, WP, APPLET, GpApplet)

GtkWidget *wp_applet_get_tasks            (WpApplet *applet);
gboolean   wp_applet_get_show_all_windows (WpApplet *applet);
gboolean   wp_applet_get_icons_greyscale  (WpApplet *applet);

void       wp_applet_setup_about          (GtkAboutDialog *dialog);

G_END_DECLS

#endif
