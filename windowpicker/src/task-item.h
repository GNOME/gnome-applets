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

#define TASK_TYPE_ITEM            (task_item_get_type ())
#define TASK_ITEM(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TASK_TYPE_ITEM, TaskItem))
#define TASK_ITEM_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  TASK_TYPE_ITEM, TaskItemClass))
#define TASK_IS_ITEM(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TASK_TYPE_ITEM))
#define TASK_IS_ITEM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  TASK_TYPE_ITEM))
#define TASK_ITEM_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  TASK_TYPE_ITEM, TaskItemClass))

typedef struct _TaskItem        TaskItem;
typedef struct _TaskItemClass   TaskItemClass;
typedef struct _TaskItemPrivate TaskItemPrivate;

struct _TaskItem {
    GtkEventBox     parent;
    TaskItemPrivate *priv;
};

struct _TaskItemClass {
    GtkEventBoxClass   parent_class;
    void (* itemclosed) (TaskItem *item);
};

GType task_item_get_type (void) G_GNUC_CONST;
GtkWidget * task_item_new (WpApplet *windowPickerApplet, WnckWindow *window);

gint        task_item_get_monitor (TaskItem *item);

#endif /* _TASK_ITEM_H_ */
