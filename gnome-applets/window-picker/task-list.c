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
 *              Sebastian Geiger <sbastig@gmx.net>
 */

#include "task-list.h"
#include "task-item.h"

#include <libwnck/libwnck.h>
#include <libgnome-panel/gp-applet.h>
#include <X11/Xlib.h>

struct _TaskList {
    GtkBox parent;
    WnckScreen *screen;
    GHashTable *items;
    WpApplet *windowPickerApplet;
    guint init_windows_event_source;
};

G_DEFINE_TYPE (TaskList, task_list, GTK_TYPE_BOX)

static GSList *task_lists;

static gboolean
window_is_special (WnckWindow *window)
{
  WnckWindowType type = wnck_window_get_window_type (window);

  return type == WNCK_WINDOW_DESKTOP
         || type == WNCK_WINDOW_DOCK
         || type == WNCK_WINDOW_SPLASHSCREEN
         || type == WNCK_WINDOW_MENU;
}

static TaskList *
get_task_list_for_monitor (GdkMonitor *monitor)
{
    GSList *list;
    GdkMonitor *list_monitor;
    TaskList *task_list;

    list = task_lists;

    while (list != NULL)
      {
        task_list = list->data;
        list_monitor = task_list_get_monitor (task_list);

        if (list_monitor == monitor)
            return task_list;

        list = list->next;
      }

    return task_lists->data;
}

static GdkMonitor *
window_get_monitor (WnckWindow *window)
{
    GdkDisplay *gdk_display;
    gint x, y, w, h;

    gdk_display = gdk_display_get_default ();

    wnck_window_get_geometry (window, &x, &y, &w, &h);

    return gdk_display_get_monitor_at_point (gdk_display,
                                             x + w / 2,
                                             y + h / 2);
}

static void
on_window_closed (WnckScreen *screen,
                  WnckWindow *window,
                  TaskList   *taskList)
{
  g_hash_table_remove (taskList->items, window);
}

static void
on_task_item_monitor_changed_cb (TaskItem *item,
                                 gpointer  user_data)
{
  GdkMonitor *monitor;
  GdkMonitor *list_monitor;
  TaskList *list;
  TaskList *current_list;
  WnckWindow *window;

  current_list = task_item_get_task_list (item);
  window = task_item_get_window (item);

  monitor = task_item_get_monitor (item);

  list_monitor = task_list_get_monitor (current_list);

  if (monitor == list_monitor)
    return;

  list = get_task_list_for_monitor (monitor);

  g_object_ref (item);

  gtk_container_remove (GTK_CONTAINER (current_list), GTK_WIDGET (item));
  g_hash_table_steal (current_list->items, window);

  gtk_widget_queue_resize (GTK_WIDGET (current_list));

  gtk_container_add (GTK_CONTAINER (list), GTK_WIDGET (item));
  g_hash_table_insert (list->items, window, item);

  task_item_set_task_list (item, list);

  g_object_unref (item);

  gtk_widget_queue_resize (GTK_WIDGET (list));
}

static GtkWidget *
create_task_item (TaskList   *taskList,
                  WnckWindow *window)
{
    GtkWidget *item;
    GdkMonitor *list_monitor;
    GdkMonitor *window_monitor;
    guint num;

    num = g_slist_length (task_lists);

    if (num > 1)
      {
        list_monitor = task_list_get_monitor (taskList);
        window_monitor = window_get_monitor (window);

        if (list_monitor != window_monitor)
            return NULL;
      }

    item = task_item_new (taskList->windowPickerApplet,
                          window, taskList);

    if (item)
      {
        gtk_container_add (GTK_CONTAINER (taskList), item);

        g_signal_connect (TASK_ITEM (item), "monitor-changed",
                          G_CALLBACK (on_task_item_monitor_changed_cb),
                          NULL);
      }

    return item;
}

static void
on_window_type_changed (WnckWindow *window,
                        TaskList   *list)
{
  GtkWidget *item;

  if (window_is_special (window))
    {
      g_hash_table_remove (list->items, window);
    }
  else
    {
      if (g_hash_table_lookup (list->items, window))
        return;

      item = create_task_item (list, window);

      if (item)
        g_hash_table_insert (list->items, window, item);
    }
}

static void
on_task_list_placement_changed (GpApplet        *applet,
                                GtkOrientation   orientation,
                                GtkPositionType  position,
                                GtkBox          *box)
{
    g_return_if_fail(box);

    gtk_orientable_set_orientation (GTK_ORIENTABLE (box), orientation);

    gtk_widget_queue_resize(GTK_WIDGET(box));
}

static void
clear_windows (TaskList *list)
{
  GdkWindow *window;
  GdkMonitor *list_monitor;

  window = gtk_widget_get_window (GTK_WIDGET (list));

  list_monitor = gdk_display_get_monitor_at_window (gdk_display_get_default (),
                                                    window);

  if (task_list_get_monitor (list) == list_monitor)
    g_hash_table_remove_all (list->items);
}

static void
add_window (TaskList *list, WnckWindow *window)
{
  GtkWidget *item;

  g_signal_connect_object (window, "type-changed",
                           G_CALLBACK (on_window_type_changed), list, 0);

  if (window_is_special (window))
    return;

  item = create_task_item (list, window);

  if (item)
    g_hash_table_insert (list->items, window, item);
}

static void
add_windows (TaskList *list)
{
  GList *windows;

  windows = wnck_screen_get_windows (list->screen);

  while (windows != NULL)
  {
    add_window (list, windows->data);

    windows = windows->next;
  }
}

static void
on_window_opened (WnckScreen *screen,
                  WnckWindow *window,
                  TaskList   *taskList)
{
  add_window (taskList, window);
}

static gboolean
init_windows (gpointer user_data)
{
  TaskList *list;

  list = TASK_LIST (user_data);

  clear_windows (list);

  add_windows (list);

  list->init_windows_event_source = 0;

  return G_SOURCE_REMOVE;
}


static GdkFilterReturn
window_filter_function (GdkXEvent *gdk_xevent,
                        GdkEvent  *event,
                        gpointer   user_data)
{
  TaskList *list = user_data;

  XEvent *xevent = (XEvent *) gdk_xevent;

  switch (xevent->type)
    {
      case PropertyNotify:
        {
          XPropertyEvent *propertyEvent;
          Atom WORKAREA_ATOM;

          propertyEvent = (XPropertyEvent *) xevent;

          WORKAREA_ATOM = XInternAtom (propertyEvent->display,
                                       "_NET_WORKAREA", True);

          if (propertyEvent->atom != WORKAREA_ATOM)
            return GDK_FILTER_CONTINUE;

          if (list->init_windows_event_source != 0)
            return GDK_FILTER_CONTINUE;

          list->init_windows_event_source = g_idle_add (init_windows,
                                                        user_data);

          break;
        }
      default:break;
    }

  return GDK_FILTER_CONTINUE;
}

static void
task_list_finalize (GObject *object)
{
    TaskList *taskList;

    taskList = TASK_LIST (object);

    task_lists = g_slist_remove (task_lists, taskList);

    gdk_window_remove_filter (gtk_widget_get_window (GTK_WIDGET (taskList)),
                              window_filter_function,
                              taskList);

    G_OBJECT_CLASS (task_list_parent_class)->finalize (object);
}

static void
task_list_class_init(TaskListClass *class) {
    GObjectClass *obj_class = G_OBJECT_CLASS (class);

    obj_class->finalize = task_list_finalize;
}

static void task_list_init (TaskList *list) {
    list->items = g_hash_table_new_full (NULL, NULL,
                                         NULL, (GDestroyNotify) gtk_widget_destroy);

    gtk_container_set_border_width (GTK_CONTAINER (list), 0);
}

GtkWidget *task_list_new (WpApplet *windowPickerApplet) {

    GtkOrientation orientation;
    TaskList* taskList;

    orientation = gp_applet_get_orientation (GP_APPLET (windowPickerApplet));

    taskList = g_object_new (TASK_TYPE_LIST,
                             "orientation", orientation,
                             NULL);

    task_lists = g_slist_append (task_lists, taskList);

    taskList->screen = wp_applet_get_default_screen (windowPickerApplet);
    taskList->windowPickerApplet = windowPickerApplet;

    g_signal_connect_object (windowPickerApplet, "placement-changed",
                             G_CALLBACK (on_task_list_placement_changed), taskList, 0);

    g_signal_connect_object (taskList->screen, "window-opened",
                             G_CALLBACK (on_window_opened), taskList, 0);

    g_signal_connect_object (taskList->screen, "window-closed",
                             G_CALLBACK (on_window_closed), taskList, 0);

    gdk_window_add_filter (gtk_widget_get_window (GTK_WIDGET (taskList)),
                           window_filter_function,
                           taskList);

    taskList->init_windows_event_source = g_idle_add (init_windows, taskList);

    return (GtkWidget *) taskList;
}

GdkMonitor *
task_list_get_monitor (TaskList *list)
{
    GdkDisplay *gdk_display;
    GdkWindow *gdk_window;

    gdk_display = gdk_display_get_default ();
    gdk_window = gtk_widget_get_window (GTK_WIDGET (list));

    return gdk_display_get_monitor_at_window (gdk_display,
                                              gdk_window);
}
