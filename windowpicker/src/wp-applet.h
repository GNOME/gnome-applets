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

#include <panel-applet.h>

G_BEGIN_DECLS

#define WP_TYPE_APPLET         (wp_applet_get_type ())
#define WP_APPLET(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), WP_TYPE_APPLET, WpApplet))
#define WP_APPLET_CLASS(c)     (G_TYPE_CHECK_CLASS_CAST ((c),    WP_TYPE_APPLET, WpAppletClass))
#define WP_IS_APPLET(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), WP_TYPE_APPLET))
#define WP_IS_APPLET_CLASS(c)  (G_TYPE_CHECK_CLASS_TYPE ((c),    WP_TYPE_APPLET))
#define WP_APPLET_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS((o),   WP_TYPE_APPLET, WpAppletClass))

typedef struct _WpApplet      WpApplet;
typedef struct _WpAppletClass WpAppletClass;

struct _WpAppletClass
{
  PanelAppletClass parent_class;
};

GType      wp_applet_get_type             (void) G_GNUC_CONST;

GtkWidget *wp_applet_get_tasks            (WpApplet *applet);
gboolean   wp_applet_get_show_all_windows (WpApplet *applet);
gboolean   wp_applet_get_icons_greyscale  (WpApplet *applet);

G_END_DECLS

#endif
