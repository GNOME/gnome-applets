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

#include "wp-preferences-dialog.h"

struct _WpPreferencesDialog
{
  GtkDialog  parent;

  GSettings *settings;

  GtkWidget *check_show_all_windows;
  GtkWidget *check_show_application_title;
  GtkWidget *check_show_home_title;
  GtkWidget *check_icons_greyscale;
};

enum
{
  PROP_0,
  PROP_SETTINGS,
  LAST_PROP
};

static GParamSpec *properties[LAST_PROP] = { NULL };

G_DEFINE_TYPE (WpPreferencesDialog, wp_preferences_dialog, GTK_TYPE_DIALOG)

static void
wp_preferences_dialog_constructed (GObject *object)
{
  WpPreferencesDialog *dialog;

  dialog = WP_PREFERENCES_DIALOG (object);

  G_OBJECT_CLASS (wp_preferences_dialog_parent_class)->constructed (object);

  g_settings_bind (dialog->settings, KEY_SHOW_ALL_WINDOWS,
                   dialog->check_show_all_windows, "active",
                   G_SETTINGS_BIND_DEFAULT);

  g_settings_bind (dialog->settings, KEY_SHOW_APPLICATION_TITLE,
                   dialog->check_show_application_title, "active",
                   G_SETTINGS_BIND_DEFAULT);

  g_settings_bind (dialog->settings, KEY_SHOW_HOME_TITLE,
                   dialog->check_show_home_title, "active",
                   G_SETTINGS_BIND_DEFAULT);

  g_settings_bind (dialog->settings, KEY_ICONS_GREYSCALE,
                   dialog->check_icons_greyscale, "active",
                   G_SETTINGS_BIND_DEFAULT);
}

static void
wp_preferences_dialog_set_property (GObject      *object,
                                    guint         property_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
  WpPreferencesDialog *dialog;

  dialog = WP_PREFERENCES_DIALOG (object);

  switch (property_id)
    {
      case PROP_SETTINGS:
        dialog->settings = g_value_dup_object (value);
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}

static void
wp_preferences_dialog_dispose (GObject *object)
{
  WpPreferencesDialog *dialog;

  dialog = WP_PREFERENCES_DIALOG (object);

  g_clear_object (&dialog->settings);

  G_OBJECT_CLASS (wp_preferences_dialog_parent_class)->dispose (object);
}

static void
wp_preferences_dialog_class_init (WpPreferencesDialogClass *dialog_class)
{
  GObjectClass *object_class;
  GtkWidgetClass *widget_class;
  const gchar *resource_name;

  object_class = G_OBJECT_CLASS (dialog_class);
  widget_class = GTK_WIDGET_CLASS (dialog_class);

  object_class->constructed = wp_preferences_dialog_constructed;
  object_class->set_property = wp_preferences_dialog_set_property;
  object_class->dispose = wp_preferences_dialog_dispose;

  properties[PROP_SETTINGS] =
    g_param_spec_object ("settings",
                         "Settings",
                         "Settings",
                         G_TYPE_SETTINGS,
                         G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY);

  g_object_class_install_properties (object_class, LAST_PROP, properties);

  resource_name = GRESOURCE_PREFIX "/ui/wp-preferences-dialog.ui";
  gtk_widget_class_set_template_from_resource (widget_class, resource_name);

  gtk_widget_class_bind_template_child (widget_class, WpPreferencesDialog,
                                        check_show_all_windows);
  gtk_widget_class_bind_template_child (widget_class, WpPreferencesDialog,
                                        check_show_application_title);
  gtk_widget_class_bind_template_child (widget_class, WpPreferencesDialog,
                                        check_show_home_title);
  gtk_widget_class_bind_template_child (widget_class, WpPreferencesDialog,
                                        check_icons_greyscale);
}

static void
wp_preferences_dialog_init (WpPreferencesDialog *dialog)
{
  gtk_widget_init_template (GTK_WIDGET (dialog));
}

GtkWidget *
wp_preferences_dialog_new (GSettings *settings)
{
  return g_object_new (WP_TYPE_PREFERENCES_DIALOG,
                       "settings", settings,
                       NULL);
}
