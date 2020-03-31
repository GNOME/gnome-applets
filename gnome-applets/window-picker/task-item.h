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
 * Authored by: Neil Jagdish Patel <neil.patel@canonical.com>
 *              Jason Smith        <jassmith@gmail.com>
 */

#ifndef _TASK_ITEM_H_
#define _TASK_ITEM_H_

#include "wp-applet.h"

#include <glib.h>
#include <gtk/gtk.h>
#include <libwnck/libwnck.h>

#include "task-list.h"

#define TASK_TYPE_ITEM (task_item_get_type ())
G_DECLARE_FINAL_TYPE (TaskItem, task_item, TASK, ITEM, GtkEventBox)

GtkWidget  *task_item_new           (WpApplet   *windowPickerApplet,
                                     WnckWindow *window,
                                     TaskList   *list);

GdkMonitor *task_item_get_monitor   (TaskItem   *item);

TaskList   *task_item_get_task_list (TaskItem   *item);

void        task_item_set_task_list (TaskItem   *item,
                                     TaskList   *list);

WnckWindow *task_item_get_window    (TaskItem   *item);

#endif /* _TASK_ITEM_H_ */
