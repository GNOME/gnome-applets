/*
 * Copyright (C) 2008 Canonical Ltd
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
 *     Neil Jagdish Patel <neil.patel@canonical.com>
 */

#ifndef WP_TASK_TITLE_H
#define WP_TASK_TITLE_H

#include <gtk/gtk.h>

#include "wp-applet.h"

G_BEGIN_DECLS

#define WP_TYPE_TASK_TITLE wp_task_title_get_type ()
G_DECLARE_FINAL_TYPE (WpTaskTitle, wp_task_title, WP, TASK_TITLE, GtkBox)

GtkWidget *wp_task_title_new (gint      spacing,
                              WpApplet *applet);

G_END_DECLS

#endif
