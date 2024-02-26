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
 *     Sebastian Geiger <sbastig@gmx.net>
 */

#include "config.h"

#include <glib/gi18n-lib.h>
#include <gio/gio.h>
#include <libwnck/libwnck.h>
#include <libgnome-panel/gp-applet.h>

#include "wp-task-title.h"

#define CLOSE_WINDOW_ICON "window-close"
#define LOGOUT_ICON "system-log-out"

struct _WpTaskTitle
{
  GtkBox          parent;

  WpApplet       *applet;

  GtkWidget      *label;
  GtkWidget      *button;
  GtkWidget      *image;

  gboolean        show_application_title;
  gboolean        show_home_title;
  GtkOrientation  orientation;

  WnckWindow     *active_window;
  GDBusProxy     *session_proxy;
};

enum
{
  PROP_0,
  PROP_SHOW_APPLICATION_TITLE,
  PROP_SHOW_HOME_TITLE,
  PROP_APPLET_ORIENT,
  LAST_PROP
};

static GParamSpec *properties[LAST_PROP] = { NULL };

G_DEFINE_TYPE (WpTaskTitle, wp_task_title, GTK_TYPE_BOX)

static void disconnect_active_window (WpTaskTitle *title);

static void
logout_ready_callback (GObject      *source_object,
                       GAsyncResult *res,
                       WpTaskTitle  *title)
{
  GError *error;
  GVariant *result;

  error = NULL;
  result = g_dbus_proxy_call_finish (title->session_proxy, res, &error);

  if (result)
    g_variant_unref (result);

  if (error)
    {
      g_warning ("Could not ask session manager to log out: %s",
                 error->message);
      g_error_free (error);
    }
}

static gboolean
button_press_event_cb (GtkButton      *button,
                       GdkEventButton *event,
                       gpointer        user_data)
{
  if (event->button != 1)
    return GDK_EVENT_STOP;

  return GDK_EVENT_PROPAGATE;
}

static gboolean
button_clicked_cb (GtkButton *button,
                   gpointer   user_data)
{
  WpTaskTitle *title;
  const gchar *icon;

  title = WP_TASK_TITLE (user_data);

  gtk_image_get_icon_name (GTK_IMAGE (title->image), &icon, NULL);

  if (g_strcmp0 (icon, CLOSE_WINDOW_ICON) == 0)
    {
      WnckScreen *screen;
      WnckWindow *active_window;

      screen = wp_applet_get_default_screen (title->applet);
      active_window = wnck_screen_get_active_window (screen);

      if (WNCK_IS_WINDOW (active_window) == FALSE)
        return FALSE;

      if (title->active_window != active_window)
        return FALSE;

      disconnect_active_window (title);
      wnck_window_close (active_window, gtk_get_current_event_time ());

      return GDK_EVENT_STOP;
    }
  else if (g_strcmp0 (icon, LOGOUT_ICON) == 0)
    {
      g_dbus_proxy_call (title->session_proxy, "Logout",
                         g_variant_new ("(u)", 0), G_DBUS_CALL_FLAGS_NONE,
                         -1, NULL, (GAsyncReadyCallback) logout_ready_callback,
                         title);

      return GDK_EVENT_STOP;
    }
  else
    {
      g_assert_not_reached ();
    }

  return GDK_EVENT_PROPAGATE;
}

static gboolean
is_desktop_visible (WpTaskTitle *title)
{
  WnckScreen *screen;
  GList *windows;
  GList *w;

  screen = wp_applet_get_default_screen (title->applet);
  windows = wnck_screen_get_windows (screen);

  for (w = windows; w; w = w->next)
    {
      WnckWindow *window;
      WnckWindowType type;

      window = WNCK_WINDOW (w->data);

      if (WNCK_IS_WINDOW (window) == FALSE)
        continue;

      type = wnck_window_get_window_type (window);

      if (type == WNCK_WINDOW_DOCK || type == WNCK_WINDOW_SPLASHSCREEN ||
          type == WNCK_WINDOW_MENU || type == WNCK_WINDOW_DESKTOP)
        continue;

      if (wnck_window_is_minimized (window) == FALSE)
        return FALSE;
    }

  return TRUE;
}

static void
show_home_title (WpTaskTitle *title)
{
  const gchar *name;
  const gchar *tooltip;

  name = _("Home");
  tooltip = _("Log off, switch user, lock screen or power down the computer");

  gtk_label_set_text (GTK_LABEL (title->label), name);
  gtk_widget_set_tooltip_text (GTK_WIDGET (title), name);

  gtk_image_set_from_icon_name (GTK_IMAGE (title->image), LOGOUT_ICON,
                                GTK_ICON_SIZE_MENU);

  gtk_widget_set_tooltip_text (title->button, tooltip);
}

static void
show_application_title (WpTaskTitle *title)
{
  const gchar *name;
  const gchar *tooltip;

  name = wnck_window_get_name (title->active_window);
  tooltip = _("Close window");

  gtk_label_set_text (GTK_LABEL (title->label), name);
  gtk_widget_set_tooltip_text (GTK_WIDGET (title), name);

  gtk_image_set_from_icon_name (GTK_IMAGE (title->image), CLOSE_WINDOW_ICON,
                                GTK_ICON_SIZE_MENU);

  gtk_widget_set_tooltip_text (title->button, tooltip);
}

static void
update_title_visibility (WpTaskTitle *title)
{
  GtkWidget *widget;
  WnckWindowType type;

  widget = GTK_WIDGET (title);
  type = WNCK_WINDOW_NORMAL;

  gtk_widget_hide (widget);

  if (title->show_application_title == FALSE && title->show_home_title == FALSE)
    return;

  if (title->active_window)
    type = wnck_window_get_window_type (title->active_window);

  if (title->active_window == NULL || type == WNCK_WINDOW_DESKTOP)
    {
      if (title->show_home_title == FALSE)
        return;

      if (is_desktop_visible (title) == FALSE)
        return;

      if (title->session_proxy == NULL)
        return;

      show_home_title (title);
    }
  else
    {
      if (title->show_application_title == FALSE)
        return;

      show_application_title (title);
    }

  gtk_widget_show (widget);
}

static void
name_changed_cb (WnckWindow *window,
                 gpointer    user_data)
{
  WpTaskTitle *title;

  title = WP_TASK_TITLE (user_data);

  update_title_visibility (title);
}

static void
state_changed_cb (WnckWindow      *window,
                  WnckWindowState  changed_mask,
                  WnckWindowState  new_state,
                  gpointer         user_data)
{
  WpTaskTitle *title;

  title = WP_TASK_TITLE (user_data);

  update_title_visibility (title);
}

static void
disconnect_active_window (WpTaskTitle *title)
{
  if (title->active_window == NULL)
    return;

  g_signal_handlers_disconnect_by_func (title->active_window,
                                        name_changed_cb, title);
  g_signal_handlers_disconnect_by_func (title->active_window,
                                        state_changed_cb, title);

  title->active_window = NULL;
}

static void
active_window_changed_cb (WnckScreen *screen,
                          WnckWindow *previously_active_window,
                          gpointer    user_data)
{
  WpTaskTitle *title;
  WnckWindow *active_window;
  WnckWindowType type;

  title = WP_TASK_TITLE (user_data);

  active_window = wnck_screen_get_active_window (screen);
  type = WNCK_WINDOW_NORMAL;

  if (WNCK_IS_WINDOW (active_window) == FALSE)
    {
      disconnect_active_window (title);
      update_title_visibility (title);
      return;
    }

  if (active_window)
    type = wnck_window_get_window_type (active_window);

  if (wnck_window_is_skip_tasklist (active_window) &&
      type != WNCK_WINDOW_DESKTOP)
    return;

  if (type == WNCK_WINDOW_DOCK || type == WNCK_WINDOW_SPLASHSCREEN ||
      type == WNCK_WINDOW_MENU)
    return;

  disconnect_active_window (title);

  g_signal_connect_object (active_window, "name-changed",
                           G_CALLBACK (name_changed_cb), title,
                           G_CONNECT_AFTER);
  g_signal_connect_object (active_window, "state-changed",
                           G_CALLBACK (state_changed_cb), title,
                           G_CONNECT_AFTER);

  title->active_window = active_window;
  update_title_visibility (title);
}

static void
update_label_rotation (WpTaskTitle *title)
{
  gdouble angle;

  if (title->orientation == GTK_ORIENTATION_VERTICAL)
    angle = 270;
  else
    angle = 0;

  gtk_label_set_angle (GTK_LABEL (title->label), angle);
}

static void
proxy_ready_cb (GObject      *source_object,
                GAsyncResult *result,
                WpTaskTitle  *title)
{
  GError *error;

  error = NULL;
  title->session_proxy = g_dbus_proxy_new_for_bus_finish (result, &error);

  if (error)
    {
      g_warning ("[windowpicker] Could not connect to session manager: %s",
                 error->message);
      g_error_free (error);
    }
}

static void
wp_task_title_set_property (GObject      *object,
                            guint         property_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
  WpTaskTitle *title;
  gboolean show_application_title;
  gboolean show_home_title;
  GtkOrientation orientation;

  title = WP_TASK_TITLE (object);

  switch (property_id)
    {
      case PROP_SHOW_APPLICATION_TITLE:
        show_application_title = g_value_get_boolean (value);

        if (title->show_application_title != show_application_title)
          {
            title->show_application_title = show_application_title;
            update_title_visibility (title);
          }
        break;

      case PROP_SHOW_HOME_TITLE:
        show_home_title = g_value_get_boolean (value);

        if (title->show_home_title != show_home_title)
          {
            title->show_home_title = show_home_title;
            update_title_visibility (title);
          }
        break;

      case PROP_APPLET_ORIENT:
        orientation = g_value_get_enum (value);

        if (title->orientation != orientation)
          {
            title->orientation = orientation;
            update_label_rotation (title);
          }
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}

static void
wp_task_title_dispose (GObject *object)
{
  WpTaskTitle *title;

  title = WP_TASK_TITLE (object);

  g_clear_object (&title->session_proxy);

  G_OBJECT_CLASS (wp_task_title_parent_class)->dispose (object);
}

static void
wp_task_title_class_init (WpTaskTitleClass *title_class)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (title_class);

  object_class->set_property = wp_task_title_set_property;
  object_class->dispose = wp_task_title_dispose;

  properties[PROP_SHOW_APPLICATION_TITLE] =
    g_param_spec_boolean ("show-application-title",
                          "Show Application Title",
                          "Show the application title",
                          FALSE,
                          G_PARAM_WRITABLE);

  properties[PROP_SHOW_HOME_TITLE] =
    g_param_spec_boolean ("show-home-title",
                          "Show Home Title",
                          "Show the home title and logout button",
                          FALSE,
                          G_PARAM_WRITABLE);

  properties[PROP_APPLET_ORIENT] =
    g_param_spec_enum ("orient",
                       "Orient",
                       "Panel Applet Orientation",
                       GTK_TYPE_ORIENTATION,
                       GTK_ORIENTATION_HORIZONTAL,
                       G_PARAM_WRITABLE);

  g_object_class_install_properties (object_class, LAST_PROP, properties);
}

static gboolean
label_button_press_event_cb (GtkWidget      *widget,
                             GdkEventButton *event,
                             gpointer        user_data)
{
  WpTaskTitle *title;

  title = WP_TASK_TITLE (user_data);

  if (event->button == 3)
    {
      WnckWindowType type;

      type = wnck_window_get_window_type (title->active_window);

      if (type != WNCK_WINDOW_DESKTOP)
        {
          GtkWidget *menu;

          menu = wnck_action_menu_new (title->active_window);

          gp_applet_popup_menu_at_widget (GP_APPLET (title->applet),
                                          GTK_MENU (menu), GTK_WIDGET (title),
                                          (GdkEvent *) event);

          return TRUE;
        }
    }
  else if (event->button == 1 && event->type == GDK_2BUTTON_PRESS)
    {
      if (wnck_window_is_maximized (title->active_window))
        wnck_window_unmaximize (title->active_window);
    }

  return FALSE;
}

static void
wp_task_title_setup_label (WpTaskTitle *title)
{
  GtkWidget *event_box;
  PangoAttrList *attr_list;
  PangoAttribute *attr;

  event_box = gtk_event_box_new ();

  gtk_widget_add_events (GTK_WIDGET (event_box), GDK_BUTTON_PRESS_MASK);

  g_signal_connect (event_box, "button-press-event",
                    G_CALLBACK (label_button_press_event_cb), title);

  gtk_box_pack_start (GTK_BOX (title), event_box, FALSE, FALSE, 0);
  gtk_widget_show (event_box);

  title->label = gtk_label_new (NULL);
  attr_list = pango_attr_list_new ();
  attr = pango_attr_weight_new (PANGO_WEIGHT_BOLD);

  pango_attr_list_insert (attr_list, attr);

  gtk_label_set_ellipsize (GTK_LABEL (title->label), PANGO_ELLIPSIZE_END);
  gtk_label_set_attributes (GTK_LABEL (title->label), attr_list);
  pango_attr_list_unref (attr_list);

  gtk_container_add (GTK_CONTAINER (event_box), title->label);
  gtk_widget_show (title->label);
}

static void
wp_task_title_setup_button (WpTaskTitle *title)
{
  GtkWidget *widget;

  title->button = gtk_button_new ();
  title->image = gtk_image_new ();

  widget = GTK_WIDGET (title->button);

  gtk_widget_set_halign (widget, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (widget, GTK_ALIGN_CENTER);

  gtk_button_set_image (GTK_BUTTON (widget), title->image);

  gtk_box_pack_start (GTK_BOX (title), title->button, FALSE, FALSE, 0);
  gtk_widget_show (title->button);

  g_signal_connect (title->button, "clicked",
                    G_CALLBACK (button_clicked_cb), title);

  g_signal_connect (title->button, "button-press-event",
                    G_CALLBACK (button_press_event_cb), title);
}

static void
wp_task_title_setup_wnck (WpTaskTitle *title)
{
  WnckScreen *screen;

  screen = wp_applet_get_default_screen (title->applet);

  g_signal_connect_object (screen, "active-window-changed",
                           G_CALLBACK (active_window_changed_cb), title,
                           G_CONNECT_AFTER);

  active_window_changed_cb (screen, NULL, title);
}

static void
wp_task_title_init (WpTaskTitle *title)
{
  g_dbus_proxy_new_for_bus (G_BUS_TYPE_SESSION, G_DBUS_PROXY_FLAGS_NONE, NULL,
                            "org.gnome.SessionManager",
                            "/org/gnome/SessionManager",
                            "org.gnome.SessionManager",
                            NULL, (GAsyncReadyCallback) proxy_ready_cb, title);
}

GtkWidget *
wp_task_title_new (gint      spacing,
                   WpApplet *applet)
{
  WpTaskTitle *title;

  title = g_object_new (WP_TYPE_TASK_TITLE,
                        "spacing", spacing,
                         NULL);

  title->applet = applet;

  wp_task_title_setup_label (title);
  wp_task_title_setup_button (title);
  wp_task_title_setup_wnck (title);

  return GTK_WIDGET (title);
}
