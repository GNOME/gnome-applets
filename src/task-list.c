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
#include <panel-applet.h>

struct _TaskListPrivate {
    WnckScreen *screen;
    GHashTable *win_table;
    guint timer;
    guint counter;
    gboolean show_all_windows;
    WindowPickerApplet *windowPickerApplet;
};

G_DEFINE_TYPE_WITH_PRIVATE (TaskList, task_list, GTK_TYPE_BOX);

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
