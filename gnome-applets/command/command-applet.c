/*
 * Copyright (C) 2013-2014 Stefano Karapetsas
 *
 * This file is part of GNOME Applets.
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Authors:
 *      Stefano Karapetsas <stefano@karapetsas.com>
 */

#include "config.h"
#include "command-applet.h"

#include <glib.h>
#include <glib/gi18n-lib.h>
#include <gio/gio.h>
#include <gtk/gtk.h>
#include <libgnome-panel/gp-utils.h>

#include "ga-command.h"

/* Applet constants */
#define APPLET_ICON    "utilities-terminal"
#define ERROR_OUTPUT   "#"

/* GSettings constants */
#define COMMAND_SCHEMA "org.gnome.gnome-applets.command"
#define COMMAND_KEY    "command"
#define INTERVAL_KEY   "interval"
#define SHOW_ICON_KEY  "show-icon"
#define WIDTH_KEY      "width"

/* GKeyFile constants */
#define GK_COMMAND_GROUP   "Command"
#define GK_COMMAND_OUTPUT  "Output"
#define GK_COMMAND_ICON    "Icon"

struct _CommandApplet
{
  GpApplet     parent;

  GSettings   *settings;

  GtkLabel    *label;
  GtkImage    *image;
  GtkBox      *box;
  GtkEntry    *command_entry;

  GtkWidget   *preferences_dialog;

  guint        width;

  GaCommand   *command;
};

G_DEFINE_TYPE (CommandApplet, command_applet, GP_TYPE_APPLET)

static void command_about_callback (GSimpleAction *action, GVariant *parameter, gpointer data);
static void command_settings_callback (GSimpleAction *action, GVariant *parameter, gpointer data);

static const GActionEntry applet_menu_actions [] = {
    {"preferences", command_settings_callback, NULL, NULL, NULL},
    {"about", command_about_callback, NULL, NULL, NULL},
    {NULL}
};

static void
command_about_callback (GSimpleAction *action,
                        GVariant      *parameter,
                        gpointer       user_data)
{
  gp_applet_show_about (GP_APPLET (user_data));
}

static void
apply_command_callback (GtkButton *button,
                        gpointer   data)
{
  CommandApplet *command_applet;

  command_applet = (CommandApplet *) data;

  g_settings_set_string (command_applet->settings, COMMAND_KEY, gtk_entry_get_text (command_applet->command_entry));
}

static void
command_preferences_dialog_response_cb (GtkDialog *dialog,
                                        gint       response_id,
                                        gpointer   data)
{
    CommandApplet *command_applet;

    command_applet = (CommandApplet *) data;

    gtk_widget_destroy (command_applet->preferences_dialog);
    command_applet->preferences_dialog = NULL;
}

/* Show the preferences dialog */
static void
command_settings_callback (GSimpleAction *action, GVariant *parameter, gpointer data)
{
    CommandApplet *command_applet;
    GtkGrid *grid;
    GtkWidget *widget;
    GtkWidget *button;
    GtkWidget *interval;
    GtkWidget *width;
    GtkWidget *showicon;

    command_applet = (CommandApplet *) data;

    if (command_applet->preferences_dialog != NULL) {
        gtk_window_present (GTK_WINDOW (command_applet->preferences_dialog));
        return;
    }

    command_applet->preferences_dialog = gtk_dialog_new_with_buttons(_("Command Applet Preferences"),
                                                     NULL,
                                                     0,
                                                     _("_Close"),
                                                     GTK_RESPONSE_CLOSE,
                                                     NULL);
    grid = GTK_GRID (gtk_grid_new ());
    gtk_grid_set_row_spacing (grid, 12);
    gtk_grid_set_column_spacing (grid, 12);

    gtk_window_set_default_size (GTK_WINDOW (command_applet->preferences_dialog), 350, 150);
    gtk_container_set_border_width (GTK_CONTAINER (command_applet->preferences_dialog), 10);

    widget = gtk_label_new (_("Command:"));
    gtk_label_set_xalign (GTK_LABEL (widget), 1.0);
    gtk_label_set_yalign (GTK_LABEL (widget), 0.5);
    gtk_grid_attach (grid, widget, 1, 0, 1, 1);

    command_applet->command_entry = GTK_ENTRY (gtk_entry_new ());
    gtk_grid_attach (grid, GTK_WIDGET (command_applet->command_entry), 2, 0, 1, 1);

    button = gtk_button_new_with_mnemonic (_("_Apply"));
    gtk_widget_set_tooltip_text (button, _("Click to apply the new command."));
    gtk_grid_attach (grid, button, 3, 0, 1, 1);

    widget = gtk_label_new (_("Interval (seconds):"));
    gtk_label_set_xalign (GTK_LABEL (widget), 1.0);
    gtk_label_set_yalign (GTK_LABEL (widget), 0.5);
    gtk_grid_attach (grid, widget, 1, 1, 1, 1);

    interval = gtk_spin_button_new_with_range (1.0, 600.0, 1.0);
    gtk_grid_attach (grid, interval, 2, 1, 2, 1);

    widget = gtk_label_new (_("Maximum width (chars):"));
    gtk_label_set_xalign (GTK_LABEL (widget), 1.0);
    gtk_label_set_yalign (GTK_LABEL (widget), 0.5);
    gtk_grid_attach (grid, widget, 1, 2, 1, 1);

    width = gtk_spin_button_new_with_range(1.0, 100.0, 1.0);
    gtk_grid_attach (grid, width, 2, 2, 2, 1);

    showicon = gtk_check_button_new_with_label (_("Show icon"));
    gtk_grid_attach (grid, showicon, 2, 3, 2, 1);

    gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (command_applet->preferences_dialog))), GTK_WIDGET (grid), TRUE, TRUE, 0);

    gtk_entry_set_text (command_applet->command_entry, g_settings_get_string (command_applet->settings, COMMAND_KEY));

    g_signal_connect (button, "clicked", G_CALLBACK (apply_command_callback), command_applet);
    g_signal_connect (command_applet->preferences_dialog, "response", G_CALLBACK (command_preferences_dialog_response_cb), command_applet);

    /* use g_settings_bind to manage settings */
    g_settings_bind (command_applet->settings, INTERVAL_KEY, interval, "value", G_SETTINGS_BIND_DEFAULT);
    g_settings_bind (command_applet->settings, WIDTH_KEY, width, "value", G_SETTINGS_BIND_DEFAULT);
    g_settings_bind (command_applet->settings, SHOW_ICON_KEY, showicon, "active", G_SETTINGS_BIND_DEFAULT);

    gtk_widget_show_all (command_applet->preferences_dialog);
}

static void
output_cb (GaCommand     *command,
           const char    *output,
           CommandApplet *self)
{
  if (output == NULL || *output == '\0')
    {
      gtk_label_set_text (self->label, ERROR_OUTPUT);
      return;
    }

  if (g_str_has_prefix (output, "[Command]"))
    {
      GKeyFile *file;

      file = g_key_file_new ();

      if (g_key_file_load_from_data (file, output, -1, G_KEY_FILE_NONE, NULL))
        {
          char *markup;
          char *icon;

          markup = g_key_file_get_string (file,
                                          GK_COMMAND_GROUP,
                                          GK_COMMAND_OUTPUT,
                                          NULL);
          icon = g_key_file_get_string (file,
                                        GK_COMMAND_GROUP,
                                        GK_COMMAND_ICON,
                                        NULL);

          if (markup)
            {
              gtk_label_set_use_markup (self->label, TRUE);
              gtk_label_set_markup (self->label, markup);
            }

          if (icon)
            {
              gtk_image_set_from_icon_name (self->image,
                                            icon,
                                            GTK_ICON_SIZE_LARGE_TOOLBAR);
            }

          g_free (markup);
          g_free (icon);
        }
      else
        {
          gtk_label_set_text (self->label, ERROR_OUTPUT);
        }

      g_key_file_free (file);
    }
  else
    {
      char *tmp;

      if (strlen (output) > self->width)
        {
          GString *strip_output;

          strip_output = g_string_new_len (output, self->width);
          tmp = g_string_free (strip_output, FALSE);
        }
      else
        {
          tmp = g_strdup (output);
        }

      if (g_str_has_suffix (tmp, "\n"))
        tmp[strlen (tmp) - 1] = 0;

      gtk_label_set_text (self->label, tmp);
      g_free (tmp);
    }
}

static void
error_cb (GaCommand     *command,
          GError        *error,
          CommandApplet *self)
{
  gtk_label_set_text (self->label, ERROR_OUTPUT);
}

static void
create_command (CommandApplet *self)
{
  char *command;
  unsigned int interval;
  GError *error;

  command = g_settings_get_string (self->settings, COMMAND_KEY);
  interval = g_settings_get_uint (self->settings, INTERVAL_KEY);
  error = NULL;

  g_clear_object (&self->command);
  self->command = ga_command_new (command, interval, &error);

  gtk_widget_set_tooltip_text (GTK_WIDGET (self->label), command);
  g_free (command);

  if (error != NULL)
    {
      gtk_label_set_text (self->label, ERROR_OUTPUT);

      g_warning ("%s", error->message);
      g_error_free (error);
      return;
    }

  g_signal_connect (self->command, "output", G_CALLBACK (output_cb), self);
  g_signal_connect (self->command, "error", G_CALLBACK (error_cb), self);

  ga_command_start (self->command);
}

/* GSettings signal callbacks */
static void
settings_command_changed (GSettings     *settings,
                          const char    *key,
                          CommandApplet *self)
{
  create_command (self);
}

static void
settings_width_changed (GSettings     *settings,
                        const char    *key,
                        CommandApplet *self)
{
  self->width = g_settings_get_uint (self->settings, WIDTH_KEY);

  if (self->command == NULL)
    return;

  ga_command_restart (self->command);
}

static void
settings_interval_changed (GSettings     *settings,
                           const char    *key,
                           CommandApplet *self)
{
  if (self->command == NULL)
    return;

  ga_command_set_interval (self->command,
                           g_settings_get_uint (self->settings, INTERVAL_KEY));
}

static void
command_applet_fill (CommandApplet *command_applet)
{
    const char *menu_resource;

    command_applet->settings = gp_applet_settings_new (GP_APPLET (command_applet),
                                                       COMMAND_SCHEMA);

    command_applet->width = g_settings_get_uint (command_applet->settings, WIDTH_KEY);

    command_applet->box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
    command_applet->image = GTK_IMAGE (gtk_image_new_from_icon_name (APPLET_ICON, GTK_ICON_SIZE_LARGE_TOOLBAR));
    command_applet->label = GTK_LABEL (gtk_label_new (ERROR_OUTPUT));

    /* we add the Gtk label into the applet */
    gtk_box_pack_start (command_applet->box,
                        GTK_WIDGET (command_applet->image),
                        TRUE, TRUE, 0);
    gtk_box_pack_start (command_applet->box,
                        GTK_WIDGET (command_applet->label),
                        TRUE, TRUE, 0);

    gp_add_text_color_class (GTK_WIDGET (command_applet->label));

    gtk_container_add (GTK_CONTAINER (command_applet),
                       GTK_WIDGET (command_applet->box));

    gtk_widget_show_all (GTK_WIDGET (command_applet));

    /* GSettings signals */
    g_signal_connect(command_applet->settings,
                     "changed::" COMMAND_KEY,
                     G_CALLBACK (settings_command_changed),
                     command_applet);
    g_signal_connect(command_applet->settings,
                     "changed::" INTERVAL_KEY,
                     G_CALLBACK (settings_interval_changed),
                     command_applet);
    g_signal_connect(command_applet->settings,
                     "changed::" WIDTH_KEY,
                     G_CALLBACK (settings_width_changed),
                     command_applet);
    g_settings_bind (command_applet->settings,
                     SHOW_ICON_KEY,
                     command_applet->image,
                     "visible",
                     G_SETTINGS_BIND_DEFAULT);

    /* set up context menu */
    menu_resource = GRESOURCE_PREFIX "/ui/command-applet-menu.ui";
    gp_applet_setup_menu_from_resource (GP_APPLET (command_applet),
                                        menu_resource,
                                        applet_menu_actions);

    /* first command execution */
    create_command (command_applet);
}

static void
command_applet_constructed (GObject *object)
{
  G_OBJECT_CLASS (command_applet_parent_class)->constructed (object);
  command_applet_fill (COMMAND_APPLET (object));
}

static void
command_applet_dispose (GObject *object)
{
  CommandApplet *self;

  self = COMMAND_APPLET (object);

  g_clear_object (&self->settings);
  g_clear_object (&self->command);

  g_clear_pointer (&self->preferences_dialog, gtk_widget_destroy);

  G_OBJECT_CLASS (command_applet_parent_class)->dispose (object);
}

static void
command_applet_class_init (CommandAppletClass *self_class)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (self_class);

  object_class->constructed = command_applet_constructed;
  object_class->dispose = command_applet_dispose;
}

static void
command_applet_init (CommandApplet *self)
{
  gp_applet_set_flags (GP_APPLET (self), GP_APPLET_FLAGS_EXPAND_MINOR);
}

void
command_applet_setup_about (GtkAboutDialog *dialog)
{
  const char **authors;
  const char *copyright;

  authors = (const char *[])
    {
      "Stefano Karapetsas <stefano@karapetsas.com>",
      NULL
    };

  copyright = "Copyright © 2013-2014 Stefano Karapetsas";

  gtk_about_dialog_set_authors (dialog, authors);
  gtk_about_dialog_set_translator_credits (dialog, _("translator-credits"));
  gtk_about_dialog_set_copyright (dialog, copyright);
}
