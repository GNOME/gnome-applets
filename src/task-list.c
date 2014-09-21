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

#include "task-list.h"
#include "task-item.h"

#include <libwnck/libwnck.h>
#include <panel-applet.h>

struct _TaskListPrivate {
    WnckScreen *screen;
    GHashTable *win_table;
    guint timer;
    guint counter;
    gboolean show_all_windows;
    WindowPickerApplet *windowPickerApplet;
};

enum {
  PROP_0,
  PROP_SHOW_ALL_WINDOWS
};

G_DEFINE_TYPE_WITH_PRIVATE (TaskList, task_list, GTK_TYPE_BOX);

static void task_list_set_show_all_windows (
    TaskList *list,
    gboolean show_all_windows)
{
    TaskListPrivate *priv = list->priv;
    priv->show_all_windows = show_all_windows;
    g_debug ("Show all windows: %s", show_all_windows ? "true" : "false");
}

static void on_task_item_closed (
    TaskItem *item,
    TaskList *list)
{
    gtk_container_remove (
        GTK_CONTAINER (list),
        GTK_WIDGET (item)
    );
    gtk_widget_destroy (GTK_WIDGET (item));
}

static void on_window_opened (WnckScreen *screen,
    WnckWindow *window,
    TaskList *taskList)
{
    g_return_if_fail (taskList != NULL);
    WnckWindowType type = wnck_window_get_window_type (window);
    if (type == WNCK_WINDOW_DESKTOP
        || type == WNCK_WINDOW_DOCK
        || type == WNCK_WINDOW_SPLASHSCREEN
        || type == WNCK_WINDOW_MENU)
    {
        return;
    }

    GtkWidget *item = task_item_new (taskList->priv->windowPickerApplet, window);

    if (item) {
        //we add items dynamically to the end of the list
        gtk_container_add(GTK_CONTAINER(taskList), item);
        g_signal_connect (TASK_ITEM(item), "task-item-closed",
                G_CALLBACK(on_task_item_closed), taskList);
    }
}

/* GObject stuff */
static void task_list_finalize (GObject *object) {
    TaskListPrivate *priv = TASK_LIST (object)->priv;
    /* Remove the blink timer */
    if (priv->timer) g_source_remove (priv->timer);

    G_OBJECT_CLASS (task_list_parent_class)->finalize (object);
}

static void task_list_get_property (
    GObject    *object,
    guint       prop_id,
    GValue     *value,
    GParamSpec *pspec)
{
    TaskList *list = TASK_LIST (object);
    TaskListPrivate *priv;
    g_return_if_fail (TASK_IS_LIST (list));
    priv = list->priv;
    switch (prop_id) {
        case PROP_SHOW_ALL_WINDOWS:
            g_value_set_boolean (value, priv->show_all_windows);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void task_list_set_property (
    GObject      *object,
    guint         prop_id,
    const GValue *value,
    GParamSpec   *pspec)
{
    TaskList *list = TASK_LIST (object);
    g_return_if_fail (TASK_IS_LIST (list));
    switch (prop_id) {
        case PROP_SHOW_ALL_WINDOWS:
            task_list_set_show_all_windows (
                list, g_value_get_boolean (value)
            );
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void on_task_list_orient_changed(PanelApplet *applet,
                                        guint orient,
                                        GtkBox *box)
{
    g_return_if_fail(box);
    switch(orient) {
        case PANEL_APPLET_ORIENT_UP:
        case PANEL_APPLET_ORIENT_DOWN:
            gtk_orientable_set_orientation(GTK_ORIENTABLE(box), GTK_ORIENTATION_HORIZONTAL);
            break;
        case PANEL_APPLET_ORIENT_LEFT:
        case PANEL_APPLET_ORIENT_RIGHT:
            gtk_orientable_set_orientation(GTK_ORIENTABLE(box), GTK_ORIENTATION_VERTICAL);
            break;
        default:
            gtk_orientable_set_orientation(GTK_ORIENTABLE(box), GTK_ORIENTATION_HORIZONTAL);
            break;
    }
    gtk_widget_queue_resize(GTK_WIDGET(box));
}

static void
task_list_class_init(TaskListClass *class) {
    GObjectClass *obj_class = G_OBJECT_CLASS (class);

    obj_class->finalize = task_list_finalize;
    obj_class->set_property = task_list_set_property;
    obj_class->get_property = task_list_get_property;
    g_object_class_install_property (
        obj_class,
        PROP_SHOW_ALL_WINDOWS,
        g_param_spec_boolean ("show_all_windows",
            "Show All Windows",
            "Show windows from all workspaces",
            TRUE,
            G_PARAM_READWRITE | G_PARAM_CONSTRUCT
        )
    );
}

static void task_list_init (TaskList *list) {
    list->priv = task_list_get_instance_private (list);
    list->priv->screen = wnck_screen_get_default ();
    /* No blink timer */
    list->priv->timer = 0;
    gtk_container_set_border_width (GTK_CONTAINER (list), 0);
}

GtkWidget *task_list_new (WindowPickerApplet *windowPickerApplet) {
    PanelAppletOrient panel_orientation = panel_applet_get_orient(PANEL_APPLET(windowPickerApplet));
    GtkOrientation orientation;
    switch(panel_orientation) {
        case PANEL_APPLET_ORIENT_UP:
        case PANEL_APPLET_ORIENT_DOWN:
            orientation = GTK_ORIENTATION_HORIZONTAL;
            break;
        case PANEL_APPLET_ORIENT_LEFT:
        case PANEL_APPLET_ORIENT_RIGHT:
            orientation = GTK_ORIENTATION_VERTICAL;
            break;
        default:
            orientation = GTK_ORIENTATION_HORIZONTAL;
    }

    TaskList* taskList = g_object_new (TASK_TYPE_LIST,
                                       "orientation", orientation,
                                       NULL
    );

    taskList->priv->windowPickerApplet = windowPickerApplet;

    g_signal_connect(PANEL_APPLET(windowPickerApplet), "change-orient",
                     G_CALLBACK(on_task_list_orient_changed), taskList);
    g_signal_connect (taskList->priv->screen, "window-opened",
            G_CALLBACK (on_window_opened), taskList);

    GList *windows = wnck_screen_get_windows (taskList->priv->screen);
    while (windows != NULL) {
        on_window_opened (taskList->priv->screen, windows->data, taskList);
        windows = windows->next;
    }
    return (GtkWidget *) taskList;
}

gboolean task_list_get_desktop_visible (TaskList *list) {
    GList *windows, *w;
    gboolean all_minimised = TRUE;
    g_return_val_if_fail (TASK_IS_LIST (list), TRUE);
    windows = wnck_screen_get_windows (list->priv->screen);
    for (w = windows; w; w = w->next) {
        WnckWindow *window = w->data;
        if (WNCK_IS_WINDOW (window) &&
            !wnck_window_is_minimized (window) &&
            !wnck_window_get_window_type (window) !=
                WNCK_WINDOW_DESKTOP)
        {
            all_minimised = FALSE;
        }
    }
    return all_minimised;
}

gboolean task_list_get_show_all_windows (TaskList *list) {
    return list->priv->show_all_windows;
}
