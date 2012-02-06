/*
 * Copyright (C) 2008 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
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
 *
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "task-title.h"

#include <libwnck/libwnck.h>
#include <panel-applet.h>
#include <panel-applet-gconf.h>
#include <glib/gi18n-lib.h>

#include <gconf/gconf.h>
#include <gconf/gconf-client.h>

#include "common.h"
#include "task-list.h"

G_DEFINE_TYPE (TaskTitle, task_title, GTK_TYPE_EVENT_BOX);

#define TASK_TITLE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj),\
  TASK_TYPE_TITLE, \
  TaskTitlePrivate))

#define SHOW_HOME_TITLE_KEY "show-home-title"

struct _TaskTitlePrivate
{
    WnckScreen *screen;
    WnckWindow *window;
    GtkWidget *align;
    GtkWidget *grid;
    GtkWidget *label;
    GtkWidget *button;
    GtkWidget *button_image;
    GdkPixbuf *quit_icon;
    gboolean show_home_title;
    gboolean mouse_in_close_button;
};

static void disconnect_window (TaskTitle *title);

/**
 * The event handler for the close icon. This closes the window and terminates
 * the program.
 */
static gboolean on_close_clicked (GtkButton *button,
    GdkEventButton *event,
    TaskTitle *title)
{
    g_return_val_if_fail (TASK_IS_TITLE (title), FALSE);
    TaskTitlePrivate *priv = title->priv;
    if (event->button != 1 || !priv->mouse_in_close_button)
        return FALSE;
    WnckWindow *window = wnck_screen_get_active_window (priv->screen);
    if (!WNCK_IS_WINDOW (window)
        || wnck_window_get_window_type (window) == WNCK_WINDOW_DESKTOP)
    {
        fprintf(stdout, "Error\n");
        fflush(stdout);
    } else {
        if (priv->window == window)
            disconnect_window (title);
        wnck_window_close (window, GDK_CURRENT_TIME);
    }
    gtk_widget_queue_draw (GTK_WIDGET (title));
    return TRUE;
}

static gboolean on_enter_notify (GtkWidget *widget,
     GdkEventCrossing *event,
     TaskTitle *title)
{
    g_return_val_if_fail (TASK_IS_TITLE (title), FALSE);
    title->priv->mouse_in_close_button = TRUE;
    gtk_widget_queue_draw (widget);  
    return FALSE;
}

static gboolean on_leave_notify (GtkWidget *widget,
        GdkEventCrossing *event,
        TaskTitle *title)
{
    g_return_val_if_fail (TASK_IS_TITLE (title), FALSE);
    title->priv->mouse_in_close_button = FALSE;
    gtk_widget_queue_draw (widget);
    return FALSE;
}

static gboolean on_button_draw (GtkWidget *widget,
                cairo_t *cr,
                gpointer userdata)
{
    TaskTitle *title = (TaskTitle*) title;
    g_return_val_if_fail (TASK_IS_TITLE (title), FALSE);
    TaskTitlePrivate *priv = title->priv;
    if (priv->mouse_in_close_button) {
        GtkStyle *style = gtk_widget_get_style (widget);
        gtk_paint_box (
            style,
            cr,
            GTK_STATE_PRELIGHT,
            GTK_SHADOW_NONE,
            widget,
            NULL,
            0,
            2,
            gtk_widget_get_allocated_width(widget),
            gtk_widget_get_allocated_height(widget) - 4
        );
    }
    return FALSE;
}

static void on_name_changed (WnckWindow *window, TaskTitle *title) {
    g_return_if_fail (TASK_IS_TITLE (title));
    g_return_if_fail (WNCK_IS_WINDOW (window));
    TaskTitlePrivate *priv = title->priv;
    if (priv->window != window)
        return;
    gtk_label_set_text (GTK_LABEL (title->priv->label),
        wnck_window_get_name (window));
    gtk_widget_set_tooltip_text (GTK_WIDGET (title),
        wnck_window_get_name (window));
    gtk_widget_queue_draw (GTK_WIDGET (title));
}

/**
 * Depending on whether the window is maximized the task title is shown
 * or hidden.
 */
static void on_state_changed (WnckWindow *window,
        WnckWindowState changed_mask,
        WnckWindowState new_state,
        TaskTitle *title)
{
    TaskTitlePrivate *priv = title->priv;
    g_return_if_fail (TASK_IS_TITLE (title));
    g_return_if_fail (WNCK_IS_WINDOW (window));
    if (priv->window != window)
        return;
    if (wnck_window_is_maximized (window)) {
        gtk_widget_set_state (GTK_WIDGET (title), GTK_STATE_ACTIVE);
        gtk_widget_show (priv->grid);
    } else {
        gtk_widget_set_state (GTK_WIDGET (title), GTK_STATE_NORMAL);
        gtk_widget_hide (priv->grid);
    }
}

/**
 * Disconnects the event handlers and the window object from the task title.
 */
static void disconnect_window (TaskTitle *title) {
    TaskTitlePrivate *priv = title->priv;
    if (!priv->window)
        return;
    g_signal_handlers_disconnect_by_func (priv->window, on_name_changed, title);
    g_signal_handlers_disconnect_by_func (priv->window, on_state_changed, title);
    priv->window = NULL;
}

static void on_active_window_changed (WnckScreen *screen,
        WnckWindow *old_window,
        TaskTitle   *title)
{
    g_return_if_fail (TASK_IS_TITLE (title));
    WnckWindow *act_window = wnck_screen_get_active_window (screen);
    WnckWindowType type = WNCK_WINDOW_NORMAL;
    TaskTitlePrivate *priv = title->priv;

    if (act_window)
        type = wnck_window_get_window_type (act_window);

    if (WNCK_IS_WINDOW (act_window)
        && wnck_window_is_skip_tasklist (act_window)
        && type != WNCK_WINDOW_DESKTOP)
    {
        return;
    }

    if (type == WNCK_WINDOW_DOCK
        || type == WNCK_WINDOW_SPLASHSCREEN
        || type == WNCK_WINDOW_MENU)
    {
        return;
    }

    disconnect_window (title);

    if (!WNCK_IS_WINDOW (act_window)
        || wnck_window_get_window_type (act_window) == WNCK_WINDOW_DESKTOP)
    { //there is no active window or we are on the desktop
        if (priv->show_home_title) {
            gtk_label_set_text (GTK_LABEL (priv->label), _("Home"));
            gtk_image_set_from_pixbuf (GTK_IMAGE (priv->button_image),
                priv->quit_icon);
            gtk_widget_set_tooltip_text (priv->button,
                _("Log off, switch user, lock screen or power "
                "down the computer"));
            gtk_widget_set_tooltip_text (GTK_WIDGET (title),
                _("Home"));
            gtk_widget_show (priv->grid);
        } else { //reset the task title and hide it
            gtk_widget_set_state (GTK_WIDGET (title), GTK_STATE_NORMAL);
            gtk_widget_set_tooltip_text (priv->button, NULL);
            gtk_widget_set_tooltip_text (GTK_WIDGET (title), NULL);
            gtk_widget_hide (priv->grid);
        }
    } else { //the new active window is a regular window
        gtk_label_set_text (GTK_LABEL (priv->label),
            wnck_window_get_name (act_window));
        gtk_image_set_from_stock (GTK_IMAGE (priv->button_image),
            GTK_STOCK_CLOSE, GTK_ICON_SIZE_MENU);
        gtk_widget_set_tooltip_text (GTK_WIDGET (title),
             wnck_window_get_name (act_window));
        gtk_widget_set_tooltip_text (priv->button, _("Close window"));
        g_signal_connect (act_window, "name-changed",
            G_CALLBACK (on_name_changed), title);
        g_signal_connect (act_window, "icon-changed",
            G_CALLBACK (on_icon_changed), title);
        g_signal_connect_after (act_window, "state-changed",
            G_CALLBACK (on_state_changed), title);
        gtk_widget_show (priv->grid);
        priv->window = act_window;
    }

    if (WNCK_IS_WINDOW (act_window)
        && !wnck_window_is_maximized (act_window)
        && (priv->show_home_title ? type != WNCK_WINDOW_DESKTOP : 1))
    {
        gtk_widget_set_state (GTK_WIDGET (title), GTK_STATE_NORMAL);
        gtk_widget_hide (priv->grid);
    } else if (!WNCK_IS_WINDOW (act_window)) {
        if (task_list_get_desktop_visible (TASK_LIST (task_list_get_default ()))
            && priv->show_home_title)
        {
            gtk_label_set_text (GTK_LABEL (priv->label), _("Home"));
            gtk_image_set_from_pixbuf (GTK_IMAGE (priv->button_image),
                 priv->quit_icon);
            gtk_widget_set_tooltip_text (priv->button,
                _("Log off, switch user, lock screen or power "
                "down the computer"));
            gtk_widget_set_tooltip_text (GTK_WIDGET (title),
                _("Home"));
            gtk_widget_show (priv->grid);
        } else {
            gtk_widget_set_state (GTK_WIDGET (title), GTK_STATE_NORMAL);
            gtk_widget_set_tooltip_text (priv->button, NULL);
            gtk_widget_set_tooltip_text (GTK_WIDGET (title), NULL);
            gtk_widget_hide (priv->grid);
        }
    } else gtk_widget_set_state (GTK_WIDGET (title), GTK_STATE_ACTIVE);
    gtk_widget_queue_draw (GTK_WIDGET (title));
}

/**
 * Event handler for clicking on the title (not the close button)
 * On double click unmaximized the window
 * On right click it shows the context menu for the current window
 */
static gboolean on_button_press (GtkWidget *title, GdkEventButton *event) {
    g_return_val_if_fail (TASK_IS_TITLE (title), FALSE);
    TaskTitlePrivate *priv = TASK_TITLE_GET_PRIVATE (title);
    WnckWindow *window = wnck_screen_get_active_window (priv->screen);
    g_return_val_if_fail (WNCK_IS_WINDOW (window), FALSE);

    if (event->button == 3) { //right click
        if (wnck_window_get_window_type (window) != WNCK_WINDOW_DESKTOP) {
            GtkWidget *menu = wnck_action_menu_new (window);
            gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL,
                event->button, event->time);
            return TRUE;
        }
    } else if (event->button == 1) { //left button double click
        if (event->type == GDK_2BUTTON_PRESS
            && wnck_window_is_maximized (window)) {
                wnck_window_unmaximize (window);
        }
    }
    return FALSE;
}

/**
 * This draws the task title. Drawing is only effective if the window for this
 * task title is maximized
 */
static gboolean on_draw (
    GtkWidget *widget,
    cairo_t *cr,
    gpointer userdata)
{
    if (gtk_widget_get_state(widget) == GTK_STATE_ACTIVE) {
        //window is either maximized or we are on the desktop
        GtkStyleContext *context = gtk_widget_get_style_context (widget);
        gtk_render_frame (
            context,
            cr,
            0, 0,
            gtk_widget_get_allocated_width(widget),
            gtk_widget_get_allocated_height(widget)
        );
    }
    gtk_container_propagate_draw (GTK_CONTAINER (widget),
          gtk_bin_get_child (GTK_BIN (widget)),
          cr
    );
    return TRUE;
}


static GtkWidget *getTitleLabel() {
    GtkWidget *label = gtk_label_new (_("Home"));
    gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_END);
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
    PangoAttrList *attr_list = pango_attr_list_new ();
    PangoAttribute *attr = pango_attr_weight_new (PANGO_WEIGHT_BOLD);
    pango_attr_list_insert (attr_list, attr);
    gtk_label_set_attributes (GTK_LABEL (label), attr_list);
    gtk_widget_set_hexpand (label, TRUE);
    gtk_widget_set_vexpand (label, TRUE);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
    return label;
}

static GtkWidget *getCloseButton(TaskTitle* title) {
    //Create the button
    GtkWidget *button = g_object_new (
        GTK_TYPE_EVENT_BOX,
        "visible-window", FALSE,
        "above-child", TRUE,
        NULL
    );

    //Add accessibility
    AtkObject *atk = gtk_widget_get_accessible (button);
    atk_object_set_name (atk, _("Close"));
    atk_object_set_description (atk, _("Close current window."));
    atk_object_set_role (atk, ATK_ROLE_PUSH_BUTTON);

    //Connect the signals
    g_signal_connect (button, "button-release-event",
        G_CALLBACK (on_close_clicked), title);
    g_signal_connect (button, "enter-notify-event",
        G_CALLBACK (on_enter_notify), title);
    g_signal_connect (button, "leave-notify-event",
        G_CALLBACK (on_leave_notify), title);
    g_signal_connect (button, "draw",
        G_CALLBACK (on_button_draw), title);

    return button;
}

/* The following methods contain the GObject code for the class lifecycle */
static void task_title_init (TaskTitle *title) {
    GSettings *gsettings = mainapp->settings;
    int width, height;
    TaskTitlePrivate *priv = title->priv = TASK_TITLE_GET_PRIVATE (title);
    priv->screen = wnck_screen_get_default ();
    priv->window = NULL;
    priv->show_home_title = g_settings_get_boolean (
        gsettings, SHOW_HOME_TITLE_KEY
    );
    gtk_widget_add_events (GTK_WIDGET (title), GDK_ALL_EVENTS_MASK);
    priv->align = gtk_alignment_new (0.0, 0.0, 1.0, 1.0);
    gtk_alignment_set_padding (GTK_ALIGNMENT (priv->align),
        0, 0, 6, 6);
    gtk_container_add (GTK_CONTAINER (title), priv->align);
    priv->grid = gtk_grid_new();
    gtk_grid_set_row_spacing (GTK_GRID(priv->grid), 2);
    gtk_container_add (GTK_CONTAINER (priv->align), priv->grid);
    gtk_widget_set_no_show_all (priv->grid, TRUE);
    gtk_widget_show (priv->grid);

    //Prepare and show the title label
    priv->label = getTitleLabel();
    gtk_grid_attach (GTK_GRID(priv->grid), priv->label, 0, 0, 1, 1);
    gtk_widget_show (priv->label);

    //Create the close button
    priv->button = getCloseButton (title);
    gtk_grid_attach (GTK_GRID(priv->grid), priv->button, 1, 0, 1, 1);
    gtk_widget_show (priv->button);

    g_signal_connect (GTK_WIDGET(title), "draw",
        G_CALLBACK (on_draw), title);

    // Prepare and add the logoff icon to the title
    GdkScreen *gdkscreen = gtk_widget_get_screen (GTK_WIDGET (title));
    GtkIconTheme *theme = gtk_icon_theme_get_for_screen (gdkscreen);
    GtkSettings *settings = gtk_settings_get_for_screen (gdkscreen);
    gtk_icon_size_lookup_for_settings (
        settings,
        GTK_ICON_SIZE_MENU,
        &width, &height
    );
    //this shows a little green exit icon, like the ones on emergency exits
    priv->quit_icon = gtk_icon_theme_load_icon (
        theme, "gnome-logout", width, 0, NULL
    );
    priv->button_image = gtk_image_new_from_pixbuf (priv->quit_icon);
    gtk_container_add (GTK_CONTAINER (priv->button), priv->button_image);
    gtk_widget_show (priv->button_image);
    gtk_widget_set_tooltip_text (priv->button,
        _("Log off, switch user, lock screen or "
        "power down the computer")
    );
    gtk_widget_set_tooltip_text (GTK_WIDGET (title), _("Home"));
    if (priv->show_home_title)
        gtk_widget_set_state (GTK_WIDGET (title), GTK_STATE_ACTIVE);
    else
        gtk_widget_hide (priv->grid);
    gtk_widget_add_events (GTK_WIDGET (title), GDK_ALL_EVENTS_MASK);
    g_signal_connect (priv->screen, "active-window-changed",
        G_CALLBACK (on_active_window_changed), title);
    g_signal_connect (title, "button-press-event",
        G_CALLBACK (on_button_press), NULL);
}

/* Destructor for the task title*/
static void task_title_finalize (GObject *object) {
    TaskTitlePrivate *priv;
    priv = TASK_TITLE_GET_PRIVATE (object);
    disconnect_window (TASK_TITLE (object));
    g_object_unref (G_OBJECT (priv->quit_icon));
    G_OBJECT_CLASS (task_title_parent_class)->finalize (object);
}

/* Class initialization */
static void task_title_class_init (TaskTitleClass *klass) {
    GObjectClass        *obj_class = G_OBJECT_CLASS (klass);
    obj_class->finalize = task_title_finalize;
    g_type_class_add_private (obj_class, sizeof (TaskTitlePrivate));
}

/* Constructor for our task title, creates a new TaskTitle object */
GtkWidget *task_title_new (void) {
    GtkWidget *title = g_object_new (TASK_TYPE_TITLE,
        "border-width", 0,
        "name", "tasklist-button",
        "visible-window", FALSE,
        NULL
    );
    return title;
}
