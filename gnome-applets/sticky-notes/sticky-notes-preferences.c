/*
 * Copyright (C) 2020 Alberts Muktupāvels
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
 */

#include "config.h"
#include "sticky-notes-preferences.h"

#include "gsettings.h"

struct _StickyNotesPreferences
{
  GtkDialog  parent;

  GtkWidget *width_label;
  GtkWidget *width_spin;

  GtkWidget *height_label;
  GtkWidget *height_spin;

  GtkWidget *sys_color_check;

  GtkWidget *prefs_font_color_label;
  GtkWidget *prefs_font_color;

  GtkWidget *prefs_color_label;
  GtkWidget *default_color;

  GtkWidget *sys_font_check;

  GtkWidget *prefs_font_label;
  GtkWidget *default_font;

  GtkWidget *sticky_check;

  GtkWidget *force_default_check;

  GtkWidget *desktop_hide_check;

  GSettings *settings;
};

enum
{
  PROP_0,

  PROP_SETTINGS,

  LAST_PROP
};

static GParamSpec *preferences_properties[LAST_PROP] = { NULL };

G_DEFINE_TYPE (StickyNotesPreferences, sticky_notes_preferences, GTK_TYPE_DIALOG)

static gboolean
int_to_dobule (GValue   *value,
               GVariant *variant,
               gpointer  user_data)
{
  int int_value;

  g_variant_get (variant, "i", &int_value);
  g_value_set_double (value, int_value);

  return TRUE;
}

static GVariant *
double_to_int (const GValue       *value,
               const GVariantType *expected_type,
               gpointer            user_data)
{
  double double_value;

  double_value = g_value_get_double (value);

  return g_variant_new_int32 (double_value);
}

static void
use_system_color_changed_cb (GSettings              *settings,
                             const char             *key,
                             StickyNotesPreferences *self)
{
  gboolean use_system_color;
  gboolean sensitive;

  use_system_color = g_settings_get_boolean (settings, key);

  sensitive = !use_system_color &&
              g_settings_is_writable (settings, KEY_DEFAULT_FONT_COLOR);

  gtk_widget_set_sensitive (self->prefs_font_color_label, sensitive);
  gtk_widget_set_sensitive (self->prefs_font_color, sensitive);

  sensitive = !use_system_color &&
              g_settings_is_writable (settings, KEY_DEFAULT_COLOR);

  gtk_widget_set_sensitive (self->prefs_color_label, sensitive);
  gtk_widget_set_sensitive (self->default_color, sensitive);
}

static void
use_system_font_changed_cb (GSettings              *settings,
                            const char             *key,
                            StickyNotesPreferences *self)
{
  gboolean use_system_font;
  gboolean sensitive;

  use_system_font = g_settings_get_boolean (settings, key);

  sensitive = !use_system_font &&
              g_settings_is_writable (settings, KEY_DEFAULT_FONT);

  gtk_widget_set_sensitive (self->prefs_font_label, sensitive);
  gtk_widget_set_sensitive (self->default_font, sensitive);
}

static gboolean
string_to_rgba (GValue   *value,
                GVariant *variant,
                gpointer  user_data)
{
  const char *color;
  GdkRGBA rgba;

  g_variant_get (variant, "&s", &color);

  if (!gdk_rgba_parse (&rgba, color))
    return FALSE;

  g_value_set_boxed (value, &rgba);

  return TRUE;
}

static GVariant *
rgba_to_string (const GValue       *value,
                const GVariantType *expected_type,
                gpointer            user_data)
{
  GdkRGBA *rgba;
  char *color;
  GVariant *variant;

  rgba = g_value_get_boxed (value);

  if (rgba == NULL)
    return NULL;

  color = gdk_rgba_to_string (rgba);
  variant = g_variant_new_string (color);
  g_free (color);

  return variant;
}

static gboolean
string_to_font (GValue   *value,
                GVariant *variant,
                gpointer  user_data)
{
  const char *color;

  g_variant_get (variant, "&s", &color);

  if (*color == '\0')
    return FALSE;

  g_value_set_string (value, color);

  return TRUE;
}

static void
sticky_notes_preferences_constructed (GObject *object)
{
  StickyNotesPreferences *self;

  self = STICKY_NOTES_PREFERENCES (object);

  G_OBJECT_CLASS (sticky_notes_preferences_parent_class)->constructed (object);

  g_settings_bind_writable (self->settings,
                            KEY_DEFAULT_WIDTH,
                            self->width_label,
                            "sensitive",
                            FALSE);

  g_settings_bind_with_mapping (self->settings,
                                KEY_DEFAULT_WIDTH,
                                self->width_spin,
                                "value",
                                G_SETTINGS_BIND_DEFAULT,
                                int_to_dobule,
                                double_to_int,
                                NULL,
                                NULL);

  g_settings_bind_writable (self->settings,
                            KEY_DEFAULT_HEIGHT,
                            self->height_label,
                            "sensitive",
                            FALSE);

  g_settings_bind_with_mapping (self->settings,
                                KEY_DEFAULT_HEIGHT,
                                self->height_spin,
                                "value",
                                G_SETTINGS_BIND_DEFAULT,
                                int_to_dobule,
                                double_to_int,
                                NULL,
                                NULL);

  g_settings_bind (self->settings,
                   KEY_USE_SYSTEM_COLOR,
                   self->sys_color_check,
                   "active",
                   G_SETTINGS_BIND_DEFAULT);

  g_signal_connect (self->settings,
                    "changed::" KEY_USE_SYSTEM_COLOR,
                    G_CALLBACK (use_system_color_changed_cb),
                    self);

  g_settings_bind_with_mapping (self->settings,
                                KEY_DEFAULT_FONT_COLOR,
                                self->prefs_font_color,
                                "rgba",
                                G_SETTINGS_BIND_DEFAULT |
                                G_SETTINGS_BIND_NO_SENSITIVITY,
                                string_to_rgba,
                                rgba_to_string,
                                NULL,
                                NULL);

  g_settings_bind_with_mapping (self->settings,
                                KEY_DEFAULT_COLOR,
                                self->default_color,
                                "rgba",
                                G_SETTINGS_BIND_DEFAULT |
                                G_SETTINGS_BIND_NO_SENSITIVITY,
                                string_to_rgba,
                                rgba_to_string,
                                NULL,
                                NULL);

  g_settings_bind (self->settings,
                   KEY_USE_SYSTEM_FONT,
                   self->sys_font_check,
                   "active",
                   G_SETTINGS_BIND_DEFAULT);

  g_signal_connect (self->settings,
                    "changed::" KEY_USE_SYSTEM_FONT,
                    G_CALLBACK (use_system_font_changed_cb),
                    self);

  g_settings_bind_with_mapping (self->settings,
                                KEY_DEFAULT_FONT,
                                self->default_font,
                                "font",
                                G_SETTINGS_BIND_DEFAULT |
                                G_SETTINGS_BIND_NO_SENSITIVITY,
                                string_to_font,
                                NULL,
                                NULL,
                                NULL);

  g_settings_bind (self->settings,
                   KEY_STICKY,
                   self->sticky_check,
                   "active",
                   G_SETTINGS_BIND_DEFAULT);

  g_settings_bind (self->settings,
                   KEY_FORCE_DEFAULT,
                   self->force_default_check,
                   "active",
                   G_SETTINGS_BIND_DEFAULT);

  g_settings_bind (self->settings,
                   KEY_DESKTOP_HIDE,
                   self->desktop_hide_check,
                   "active",
                   G_SETTINGS_BIND_DEFAULT);

  use_system_color_changed_cb (self->settings, KEY_USE_SYSTEM_COLOR, self);
  use_system_font_changed_cb (self->settings, KEY_USE_SYSTEM_FONT, self);
}

static void
sticky_notes_preferences_dispose (GObject *object)
{
  StickyNotesPreferences *self;

  self = STICKY_NOTES_PREFERENCES (object);

  g_clear_object (&self->settings);

  G_OBJECT_CLASS (sticky_notes_preferences_parent_class)->dispose (object);
}

static void
sticky_notes_preferences_set_property (GObject      *object,
                                       guint         property_id,
                                       const GValue *value,
                                       GParamSpec   *pspec)
{
  StickyNotesPreferences *self;

  self = STICKY_NOTES_PREFERENCES (object);

  switch (property_id)
    {
      case PROP_SETTINGS:
        g_assert (self->settings == NULL);
        self->settings = g_value_dup_object (value);
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}

static void
install_properties (GObjectClass *object_class)
{
  preferences_properties[PROP_SETTINGS] =
    g_param_spec_object ("settings",
                         "settings",
                         "settings",
                         G_TYPE_SETTINGS,
                         G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY |
                         G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class,
                                     LAST_PROP,
                                     preferences_properties);
}

static void
sticky_notes_preferences_class_init (StickyNotesPreferencesClass *self_class)
{
  GObjectClass *object_class;
  GtkWidgetClass *widget_class;
  const char *resource_name;

  object_class = G_OBJECT_CLASS (self_class);
  widget_class = GTK_WIDGET_CLASS (self_class);

  object_class->constructed = sticky_notes_preferences_constructed;
  object_class->dispose = sticky_notes_preferences_dispose;
  object_class->set_property = sticky_notes_preferences_set_property;

  install_properties (object_class);

  resource_name = GRESOURCE_PREFIX "/ui/sticky-notes-preferences.ui";
  gtk_widget_class_set_template_from_resource (widget_class, resource_name);

  gtk_widget_class_bind_template_child (widget_class, StickyNotesPreferences, width_label);
  gtk_widget_class_bind_template_child (widget_class, StickyNotesPreferences, width_spin);

  gtk_widget_class_bind_template_child (widget_class, StickyNotesPreferences, height_label);
  gtk_widget_class_bind_template_child (widget_class, StickyNotesPreferences, height_spin);

  gtk_widget_class_bind_template_child (widget_class, StickyNotesPreferences, sys_color_check);

  gtk_widget_class_bind_template_child (widget_class, StickyNotesPreferences, prefs_font_color_label);
  gtk_widget_class_bind_template_child (widget_class, StickyNotesPreferences, prefs_font_color);

  gtk_widget_class_bind_template_child (widget_class, StickyNotesPreferences, prefs_color_label);
  gtk_widget_class_bind_template_child (widget_class, StickyNotesPreferences, default_color);

  gtk_widget_class_bind_template_child (widget_class, StickyNotesPreferences, sys_font_check);

  gtk_widget_class_bind_template_child (widget_class, StickyNotesPreferences, prefs_font_label);
  gtk_widget_class_bind_template_child (widget_class, StickyNotesPreferences, default_font);

  gtk_widget_class_bind_template_child (widget_class, StickyNotesPreferences, sticky_check);

  gtk_widget_class_bind_template_child (widget_class, StickyNotesPreferences, force_default_check);

  gtk_widget_class_bind_template_child (widget_class, StickyNotesPreferences, desktop_hide_check);
}

static void
sticky_notes_preferences_init (StickyNotesPreferences *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}

GtkWidget *
sticky_notes_preferences_new (GSettings *settings)
{
  return g_object_new (STICKY_NOTES_TYPE_PREFERENCES,
                       "settings", settings,
                       NULL);
}
