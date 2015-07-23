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

#include <gdk/gdk.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <libwnck/libwnck.h>
#include <string.h>

#include "task-list.h"
#include "wp-about-dialog.h"
#include "wp-applet.h"
#include "wp-preferences-dialog.h"
#include "wp-task-title.h"

#define SETTINGS_SCHEMA "org.gnome.gnome-applets.window-picker-applet"
#define GRESOURCE "/org/gnome/gnome-applets/window-picker/"
#define TITLE_BUTTON_SPACE 6

struct _WpApplet
{
  PanelApplet  parent;

  GSettings   *settings;

  GtkWidget   *about_dialog;
  GtkWidget   *preferences_dialog;

  gboolean     show_all_windows;
  gboolean     icons_greyscale;
  gboolean     expand_task_list;

  GtkWidget   *container;
  GtkWidget   *tasks;
  GtkWidget   *title;
};

enum
{
  PROP_0,
  PROP_SHOW_ALL_WINDOWS,
  PROP_ICONS_GREYSCALE,
  PROP_EXPAND_TASK_LIST,
  LAST_PROP
};

static GParamSpec *properties[LAST_PROP] = { NULL };

G_DEFINE_TYPE (WpApplet, wp_applet, PANEL_TYPE_APPLET)

static void
wp_about_dialog_response_cb (GtkDialog *dialog,
                             gint       response_id,
                             gpointer   user_data)
{
  WpApplet *applet;

  applet = WP_APPLET (user_data);

  if (applet->about_dialog == NULL)
    return;

  gtk_widget_destroy (applet->about_dialog);
  applet->about_dialog = NULL;
}

static void
display_about_dialog (GSimpleAction *action,
                      GVariant      *parameter,
                      gpointer       user_data)
{
  WpApplet *applet;

  applet = WP_APPLET (user_data);

  if (applet->about_dialog == NULL)
    {
      applet->about_dialog = wp_about_dialog_new ();

      g_signal_connect (applet->about_dialog, "response",
                        G_CALLBACK (wp_about_dialog_response_cb), applet);
    }

  gtk_window_present (GTK_WINDOW (applet->about_dialog));
}

static void
wp_preferences_dialog_response_cb (GtkDialog *dialog,
                                   gint       response_id,
                                   gpointer   user_data)
{
  WpApplet *applet;

  applet = WP_APPLET (user_data);

  if (applet->preferences_dialog == NULL)
    return;

  gtk_widget_destroy (applet->preferences_dialog);
  applet->preferences_dialog = NULL;
}

static void
display_prefs_dialog (GSimpleAction *action,
                      GVariant      *parameter,
                      gpointer       user_data)
{
  WpApplet *applet;
  GSettings *settings;

  applet = WP_APPLET (user_data);
  settings = applet->settings;

  if (applet->preferences_dialog == NULL)
    {
      applet->preferences_dialog = wp_preferences_dialog_new (settings);

      g_signal_connect (applet->preferences_dialog, "response",
                        G_CALLBACK (wp_preferences_dialog_response_cb), applet);
    }

  gtk_window_present (GTK_WINDOW (applet->preferences_dialog));
}

static const GActionEntry menu_actions[] = {
  { "preferences", display_prefs_dialog },
  { "about",       display_about_dialog }
};

static void
wp_applet_setup_menu (PanelApplet *applet)
{
  GSimpleActionGroup *action_group;
  const gchar *resource_name;

  action_group = g_simple_action_group_new ();
  resource_name = GRESOURCE "wp-menu.xml";

  g_action_map_add_action_entries (G_ACTION_MAP (action_group), menu_actions,
                                   G_N_ELEMENTS (menu_actions), applet);

  panel_applet_setup_menu_from_resource (applet, resource_name, action_group,
                                         GETTEXT_PACKAGE);

  gtk_widget_insert_action_group (GTK_WIDGET (applet), "window-picker-applet",
                                  G_ACTION_GROUP (action_group));

  g_object_unref (action_group);
}

static void
wp_applet_setup_list (WpApplet *applet)
{
  applet->tasks = task_list_new (applet);

  gtk_box_pack_start (GTK_BOX (applet->container), applet->tasks,
                      FALSE, FALSE, 0);
}

static void
wp_applet_setup_title (WpApplet *applet)
{
  PanelApplet *panel_applet;

  panel_applet = PANEL_APPLET (applet);

  applet->title = wp_task_title_new (TITLE_BUTTON_SPACE);

  g_object_bind_property (applet->container, "orientation",
                          applet->title, "orientation",
                          G_BINDING_DEFAULT);

  g_object_bind_property (panel_applet, "orient",
                          applet->title, "orient",
                          G_BINDING_DEFAULT);

  gtk_box_pack_start (GTK_BOX (applet->container), applet->title,
                      FALSE, FALSE, 0);
}

static void
wp_applet_load (PanelApplet *panel_applet)
{
  WpApplet *applet;

  applet = WP_APPLET (panel_applet);

  applet->settings = panel_applet_settings_new (panel_applet, SETTINGS_SCHEMA);

  g_settings_bind (applet->settings, KEY_SHOW_ALL_WINDOWS,
                   applet, KEY_SHOW_ALL_WINDOWS, G_SETTINGS_BIND_GET);

  g_settings_bind (applet->settings, KEY_SHOW_APPLICATION_TITLE,
                   applet->title, KEY_SHOW_APPLICATION_TITLE,
                   G_SETTINGS_BIND_GET);

  g_settings_bind (applet->settings, KEY_SHOW_HOME_TITLE,
                   applet->title, KEY_SHOW_HOME_TITLE,
                   G_SETTINGS_BIND_GET);

  g_settings_bind (applet->settings, KEY_ICONS_GREYSCALE,
                   applet, KEY_ICONS_GREYSCALE, G_SETTINGS_BIND_GET);

  g_settings_bind (applet->settings, KEY_EXPAND_TASK_LIST,
                   applet, KEY_EXPAND_TASK_LIST, G_SETTINGS_BIND_GET);

  gtk_widget_show_all (GTK_WIDGET (applet));
}

static gboolean
wp_applet_factory (PanelApplet *applet,
                   const gchar *iid,
                   gpointer     data)
{
  static gboolean client_type_registered = FALSE;

  if (client_type_registered == FALSE)
    {
      wnck_set_client_type (WNCK_CLIENT_TYPE_PAGER);
      client_type_registered = TRUE;
    }

  if (g_strcmp0 (iid, "WindowPicker") != 0)
    return FALSE;

  wp_applet_load (applet);

  return TRUE;
}

static void
wp_applet_dispose (GObject *object)
{
  WpApplet *applet;

  applet = WP_APPLET (object);

  g_clear_object (&applet->settings);

  if (applet->about_dialog != NULL)
    {
      gtk_widget_destroy (applet->about_dialog);
      applet->about_dialog = NULL;
    }

  if (applet->preferences_dialog != NULL)
    {
      gtk_widget_destroy (applet->preferences_dialog);
      applet->preferences_dialog = NULL;
    }

  G_OBJECT_CLASS (wp_applet_parent_class)->dispose (object);
}

static void
wp_applet_set_property (GObject      *object,
                        guint         property_id,
                        const GValue *value,
                        GParamSpec   *pspec)
{
  WpApplet *applet;
  gboolean expand_task_list;

  applet = WP_APPLET (object);

  switch (property_id)
    {
      case PROP_SHOW_ALL_WINDOWS:
        applet->show_all_windows = g_value_get_boolean (value);
        break;

      case PROP_ICONS_GREYSCALE:
        applet->icons_greyscale = g_value_get_boolean (value);
        break;

      case PROP_EXPAND_TASK_LIST:
        expand_task_list = g_value_get_boolean (value);

        if (applet->expand_task_list != expand_task_list)
          {
            PanelApplet *panel_applet;
            PanelAppletFlags flags;

            panel_applet = PANEL_APPLET (applet);
            flags = panel_applet_get_flags (panel_applet);

            if (expand_task_list == TRUE)
              flags |= PANEL_APPLET_EXPAND_MAJOR;
            else
              flags &= ~PANEL_APPLET_EXPAND_MAJOR;

            panel_applet_set_flags (panel_applet, flags);

            applet->expand_task_list = expand_task_list;

            gtk_widget_queue_resize (GTK_WIDGET (applet));
          }
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}

static void
wp_applet_get_property (GObject    *object,
                        guint       property_id,
                        GValue     *value,
                        GParamSpec *pspec)
{
  WpApplet *applet;

  applet = WP_APPLET (object);

  switch (property_id)
    {
      case PROP_SHOW_ALL_WINDOWS:
        g_value_set_boolean (value, applet->show_all_windows);
        break;

      case PROP_ICONS_GREYSCALE:
        g_value_set_boolean (value, applet->icons_greyscale);
        break;

      case PROP_EXPAND_TASK_LIST:
        g_value_set_boolean (value, applet->expand_task_list);
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}

static void
wp_applet_change_orient (PanelApplet       *panel_applet,
                         PanelAppletOrient  orient)
{
  WpApplet *applet;
  GtkOrientation orientation;

  applet = WP_APPLET (panel_applet);
  orientation = panel_applet_get_gtk_orientation (panel_applet);

  gtk_orientable_set_orientation (GTK_ORIENTABLE (applet->container),
                                  orientation);

  if (orientation == GTK_ORIENTATION_HORIZONTAL)
    {
      gtk_widget_set_halign (applet->container, GTK_ALIGN_START);
      gtk_widget_set_valign (applet->container, GTK_ALIGN_FILL);
    }
  else
    {
      gtk_widget_set_halign (applet->container, GTK_ALIGN_FILL);
      gtk_widget_set_valign (applet->container, GTK_ALIGN_START);
    }

  gtk_widget_queue_resize (GTK_WIDGET (applet));
}

static void
wp_applet_class_init (WpAppletClass *applet_class)
{
  GObjectClass *object_class;
  PanelAppletClass *panel_applet_class;

  object_class = G_OBJECT_CLASS (applet_class);
  panel_applet_class = PANEL_APPLET_CLASS (applet_class);

  object_class->dispose = wp_applet_dispose;
  object_class->set_property = wp_applet_set_property;
  object_class->get_property = wp_applet_get_property;

  panel_applet_class->change_orient = wp_applet_change_orient;

  properties[PROP_SHOW_ALL_WINDOWS] =
    g_param_spec_boolean ("show-all-windows",
                          "Show All Windows",
                          "Show windows from all workspaces",
                          TRUE,
                          G_PARAM_READWRITE);

  properties[PROP_ICONS_GREYSCALE] =
    g_param_spec_boolean ("icons-greyscale",
                          "Icons Greyscale",
                          "All icons except the current active window icon are greyed out",
                          FALSE,
                          G_PARAM_READWRITE);

  properties[PROP_EXPAND_TASK_LIST] =
    g_param_spec_boolean ("expand-task-list",
                          "Expand Task List",
                          "Whether the task list will expand automatically and use all available space",
                          FALSE,
                          G_PARAM_READWRITE);

  g_object_class_install_properties (object_class, LAST_PROP, properties);
}

static void
wp_applet_init (WpApplet *applet)
{
  PanelApplet *panel_applet;
  PanelAppletFlags flags;
  GtkOrientation orientation;

  panel_applet = PANEL_APPLET (applet);

  flags = PANEL_APPLET_EXPAND_MINOR | PANEL_APPLET_HAS_HANDLE;
  orientation = panel_applet_get_gtk_orientation (panel_applet);

  panel_applet_set_flags (panel_applet, flags);

  applet->container = gtk_box_new (orientation, 10);
  gtk_container_add (GTK_CONTAINER (applet), applet->container);

  wp_applet_setup_list (applet);
  wp_applet_setup_title (applet);

  wp_applet_setup_menu (panel_applet);
}

GtkWidget *
wp_applet_get_tasks (WpApplet *applet)
{
  return applet->tasks;
}

gboolean
wp_applet_get_show_all_windows(WpApplet *applet)
{
  return applet->show_all_windows;
}

gboolean
wp_applet_get_icons_greyscale (WpApplet *applet)
{
  return applet->icons_greyscale;
}

gboolean
wp_applet_get_expand_task_list (WpApplet *applet)
{
  return applet->expand_task_list;
}

PANEL_APPLET_IN_PROCESS_FACTORY ("WindowPickerFactory", WP_TYPE_APPLET,
                                 wp_applet_factory, NULL);
