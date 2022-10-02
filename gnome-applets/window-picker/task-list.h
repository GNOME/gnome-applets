/*
 * Copyright (C) 2008 Canonical Ltd
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by Neil Jagdish Patel <neil.patel@canonical.com>
 */

#ifndef _TASK_LIST_H_
#define _TASK_LIST_H_

#include "wp-applet-private.h"

#include <glib-object.h>
#include <gtk/gtk.h>

#define TASK_TYPE_LIST (task_list_get_type ())

G_DECLARE_FINAL_TYPE (TaskList, task_list, TASK, LIST, GtkBox)

GtkWidget  *task_list_new         (WpApplet *applet);
GdkMonitor *task_list_get_monitor (TaskList *list);

#endif /* _TASK_LIST_H_ */
