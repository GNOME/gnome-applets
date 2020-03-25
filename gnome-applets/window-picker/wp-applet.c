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
#define CONTAINER_SPACING 10

struct _WpApplet
{
  GpApplet     parent;

  GSettings   *settings;

  GtkWidget   *about_dialog;
  GtkWidget   *preferences_dialog;

  gboolean     show_all_windows;
  gboolean     icons_greyscale;

  GtkWidget   *container;
  GtkWidget   *tasks;
  GtkWidget   *title;
};

enum
{
  PROP_0,
  PROP_SHOW_ALL_WINDOWS,
  PROP_ICONS_GREYSCALE,
  LAST_PROP
};

static GParamSpec *properties[LAST_PROP] = { NULL };

G_DEFINE_TYPE (WpApplet, wp_applet, GP_TYPE_APPLET)

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
  { "about",       display_about_dialog },
  { NULL }
};

static void
wp_applet_setup_menu (GpApplet *applet)
{
  const gchar *resource_name;

  resource_name = GRESOURCE "wp-menu.xml";

  gp_applet_setup_menu_from_resource (applet, resource_name, menu_actions);
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
  GpApplet *panel_applet;

  panel_applet = GP_APPLET (applet);

  applet->title = wp_task_title_new (TITLE_BUTTON_SPACE);

  g_object_bind_property (applet->container, "orientation",
                          applet->title, "orientation",
                          G_BINDING_DEFAULT);

  g_object_bind_property (panel_applet, "orientation",
                          applet->title, "orient",
                          G_BINDING_DEFAULT);

  gtk_box_pack_start (GTK_BOX (applet->container), applet->title,
                      FALSE, FALSE, 0);
}

static void
wp_applet_contructed (GObject *object)
{
  WpApplet *applet;
  GpApplet *gp_applet;

  G_OBJECT_CLASS (wp_applet_parent_class)->constructed (object);

  applet = WP_APPLET (object);
  gp_applet = GP_APPLET (object);

  wp_applet_setup_list (applet);
  wp_applet_setup_title (applet);

  wp_applet_setup_menu (gp_applet);

  applet->settings = gp_applet_settings_new (gp_applet, SETTINGS_SCHEMA);

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

  gtk_widget_show_all (GTK_WIDGET (applet));
}

static void
wp_applet_size_allocate (GtkWidget     *widget,
                         GtkAllocation *allocation)
{
  WpApplet *applet;
  GpApplet *panel_applet;
  GtkOrientation orientation;
  gint size_hints[2];
  gint size;

  GTK_WIDGET_CLASS (wp_applet_parent_class)->size_allocate (widget, allocation);

  applet = WP_APPLET (widget);
  panel_applet = GP_APPLET (widget);

  orientation = gp_applet_get_orientation (panel_applet);

  if (orientation == GTK_ORIENTATION_HORIZONTAL)
    gtk_widget_get_preferred_width_for_height (applet->tasks, allocation->height, NULL, &size);
  else
    gtk_widget_get_preferred_height_for_width (applet->tasks, allocation->width, NULL, &size);

  size_hints[0] = size;
  size_hints[1] = 0;

  if (gtk_widget_is_visible (applet->title))
    {
      if (orientation == GTK_ORIENTATION_HORIZONTAL)
        gtk_widget_get_preferred_width (applet->title, NULL, &size);
      else
        gtk_widget_get_preferred_height (applet->title, NULL, &size);

      if (size != 0)
        size_hints[0] += CONTAINER_SPACING;

      size_hints[0] += size;
    }

  gp_applet_set_size_hints (panel_applet, size_hints, 2, 0);
}

static void
wp_applet_dispose (GObject *object)
{
  WpApplet *applet;

  applet = WP_APPLET (object);

  g_clear_object (&applet->settings);
  g_clear_pointer (&applet->about_dialog, gtk_widget_destroy);
  g_clear_pointer (&applet->preferences_dialog, gtk_widget_destroy);

  G_OBJECT_CLASS (wp_applet_parent_class)->dispose (object);
}

static void
wp_applet_set_property (GObject      *object,
                        guint         property_id,
                        const GValue *value,
                        GParamSpec   *pspec)
{
  WpApplet *applet;

  applet = WP_APPLET (object);

  switch (property_id)
    {
      case PROP_SHOW_ALL_WINDOWS:
        applet->show_all_windows = g_value_get_boolean (value);
        break;

      case PROP_ICONS_GREYSCALE:
        applet->icons_greyscale = g_value_get_boolean (value);
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

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}

static void
wp_applet_placement_changed (GpApplet        *panel_applet,
                             GtkOrientation   orientation,
                             GtkPositionType  position)
{
  WpApplet *applet;

  applet = WP_APPLET (panel_applet);

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
  GtkWidgetClass *widget_class;
  GpAppletClass *panel_applet_class;

  object_class = G_OBJECT_CLASS (applet_class);
  widget_class = GTK_WIDGET_CLASS (applet_class);
  panel_applet_class = GP_APPLET_CLASS (applet_class);

  widget_class->size_allocate = wp_applet_size_allocate;

  object_class->dispose = wp_applet_dispose;
  object_class->set_property = wp_applet_set_property;
  object_class->get_property = wp_applet_get_property;
  object_class->constructed = wp_applet_contructed;

  panel_applet_class->placement_changed = wp_applet_placement_changed;

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

  g_object_class_install_properties (object_class, LAST_PROP, properties);
}

static void
wp_applet_init (WpApplet *applet)
{
  GpApplet *panel_applet;
  GpAppletFlags flags;
  GtkOrientation orientation;

  panel_applet = GP_APPLET (applet);

  flags = GP_APPLET_FLAGS_EXPAND_MINOR | GP_APPLET_FLAGS_HAS_HANDLE |
          GP_APPLET_FLAGS_EXPAND_MAJOR;
  orientation = gp_applet_get_orientation (panel_applet);

  gp_applet_set_flags (panel_applet, flags);

  applet->container = gtk_box_new (orientation, CONTAINER_SPACING);
  gtk_container_add (GTK_CONTAINER (applet), applet->container);
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
