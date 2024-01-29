/*
 * Copyright (C) 2014 Stefano Karapetsas
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
#include "timer-applet.h"

#include <glib.h>
#include <glib/gi18n-lib.h>
#include <gio/gio.h>
#include <gtk/gtk.h>

#include <libnotify/notify.h>
#include <libgnome-panel/gp-utils.h>

/* Applet constants */
#define APPLET_ICON  "gnome-panel-clock"
#define STEP         100

/* GSettings constants */
#define TIMER_SCHEMA            "org.gnome.gnome-applets.timer"
#define NAME_KEY                "name"
#define DURATION_KEY            "duration"
#define SHOW_NOTIFICATION_KEY   "show-notification"
#define SHOW_DIALOG_KEY         "show-dialog"

struct _TimerApplet
{
    GpApplet            parent;

    GSettings          *settings;

    GtkLabel           *label;
    GtkImage           *image;
    GtkImage           *pause_image;
    GtkBox             *box;

    GtkSpinButton      *hours;
    GtkSpinButton      *minutes;
    GtkSpinButton      *seconds;

    GtkWidget          *preferences_dialog;

    gboolean            active;
    gboolean            pause;
    gint                elapsed;

    guint               timeout_id;
};

G_DEFINE_TYPE (TimerApplet, timer_applet, GP_TYPE_APPLET)

static void timer_start_callback (GSimpleAction *action, GVariant *parameter, gpointer data);
static void timer_pause_callback (GSimpleAction *action, GVariant *parameter, gpointer data);
static void timer_stop_callback (GSimpleAction *action, GVariant *parameter, gpointer data);
static void timer_about_callback (GSimpleAction *action, GVariant *parameter, gpointer data);
static void timer_preferences_callback (GSimpleAction *action, GVariant *parameter, gpointer data);

static const GActionEntry applet_menu_actions [] = {
    {"start", timer_start_callback, NULL, NULL, NULL},
    {"pause", timer_pause_callback, NULL, NULL, NULL},
    {"stop", timer_stop_callback, NULL, NULL, NULL},
    {"preferences", timer_preferences_callback, NULL, NULL, NULL},
    {"about", timer_about_callback, NULL, NULL, NULL},
    {NULL}
};

/* timer management */
static gboolean
timer_callback (TimerApplet *applet)
{
    gboolean retval = TRUE;
    gchar *label;
    gchar *name;
    gchar *tooltip;
    gint hours, minutes, seconds, duration, remaining;

    label = NULL;
    tooltip = NULL;

    name = g_settings_get_string (applet->settings, NAME_KEY);

    if (!applet->active)
    {
        gtk_label_set_text (applet->label, name);
        gtk_widget_set_tooltip_text (GTK_WIDGET (applet->label), "");
        gtk_widget_hide (GTK_WIDGET (applet->pause_image));
    }
    else
    {
        if (applet->active && !applet->pause)
            applet->elapsed += STEP;

        duration = g_settings_get_int (applet->settings, DURATION_KEY);

        remaining = duration - (applet->elapsed / 1000);

        if (remaining <= 0)
        {
            applet->active = FALSE;
            gtk_label_set_text (applet->label, _("Finished"));
            gtk_widget_set_tooltip_text (GTK_WIDGET (applet->label), name);
            gtk_widget_hide (GTK_WIDGET (applet->pause_image));

            if (g_settings_get_boolean (applet->settings, SHOW_NOTIFICATION_KEY))
            {
                NotifyNotification *n;
                n = notify_notification_new (name, _("Timer finished!"), APPLET_ICON);
                notify_notification_set_timeout (n, 30000);
                notify_notification_show (n, NULL);
                g_object_unref (G_OBJECT (n));
            }

            if (g_settings_get_boolean (applet->settings, SHOW_DIALOG_KEY))
            {
                GtkWidget *dialog = gtk_message_dialog_new_with_markup (NULL,
                                                                        GTK_DIALOG_MODAL,
                                                                        GTK_MESSAGE_INFO,
                                                                        GTK_BUTTONS_OK,
                                                                        "<b>%s</b>\n\n%s", name, _("Timer finished!"));
                gtk_dialog_run (GTK_DIALOG (dialog));
                gtk_widget_destroy (dialog);
            }

            /* stop further calls */
            retval = FALSE;
        }
        else
        {
            hours = remaining / 60 / 60;
            minutes = remaining / 60 % 60;
            seconds = remaining % 60;

            if (hours > 0)
                label = g_strdup_printf ("%02d:%02d:%02d", hours, minutes, seconds);
            else
                label = g_strdup_printf ("%02d:%02d", minutes, seconds);

            hours = duration / 60 / 60;
            minutes = duration / 60 % 60;
            seconds = duration % 60;

            if (hours > 0)
                tooltip = g_strdup_printf ("%s (%02d:%02d:%02d)", name, hours, minutes, seconds);
            else
                tooltip = g_strdup_printf ("%s (%02d:%02d)", name, minutes, seconds);

            gtk_label_set_text (applet->label, label);
            gtk_widget_set_tooltip_text (GTK_WIDGET (applet->label), tooltip);
            gtk_widget_set_visible (GTK_WIDGET (applet->pause_image), applet->pause);
        }

        g_free (label);
        g_free (tooltip);
    }

    /* update actions sensitiveness */
    g_simple_action_set_enabled (G_SIMPLE_ACTION (gp_applet_menu_lookup_action (GP_APPLET (applet), "start")), !applet->active || applet->pause);
    g_simple_action_set_enabled (G_SIMPLE_ACTION (gp_applet_menu_lookup_action (GP_APPLET (applet), "pause")), applet->active && !applet->pause);
    g_simple_action_set_enabled (G_SIMPLE_ACTION (gp_applet_menu_lookup_action (GP_APPLET (applet), "stop")), applet->active);
    g_simple_action_set_enabled (G_SIMPLE_ACTION (gp_applet_menu_lookup_action (GP_APPLET (applet), "preferences")), !applet->active && !applet->pause);

    g_free (name);

    return retval;
}

/* start action */
static void
timer_start_callback (GSimpleAction *action, GVariant *parameter, gpointer data)
{
    TimerApplet *applet;

    applet = (TimerApplet *) data;

    applet->active = TRUE;
    if (applet->pause)
        applet->pause = FALSE;
    else
        applet->elapsed = 0;
    applet->timeout_id = g_timeout_add (STEP, (GSourceFunc) timer_callback, applet);
}

/* pause action */
static void
timer_pause_callback (GSimpleAction *action, GVariant *parameter, gpointer data)
{
    TimerApplet *applet;

    applet = (TimerApplet *) data;

    applet->pause = TRUE;
    if (applet->timeout_id != 0)
    {
        g_source_remove(applet->timeout_id);
        applet->timeout_id = 0;
    }
    timer_callback (applet);
}

/* stop action */
static void
timer_stop_callback (GSimpleAction *action, GVariant *parameter, gpointer data)
{
    TimerApplet *applet;

    applet = (TimerApplet *) data;

    applet->active = FALSE;
    applet->pause = FALSE;
    if (applet->timeout_id != 0)
    {
        g_source_remove(applet->timeout_id);
        applet->timeout_id = 0;
    }
    timer_callback (applet);
}

static void
timer_about_callback (GSimpleAction *action,
                      GVariant      *parameter,
                      gpointer       user_data)
{
  gp_applet_show_about (GP_APPLET (user_data));
}

/* calculate duration and save in GSettings */
static void
timer_spin_button_value_changed (GtkSpinButton *spinbutton, TimerApplet *applet)
{
    gint duration = 0;

    duration += gtk_spin_button_get_value (applet->hours) * 60 * 60;
    duration += gtk_spin_button_get_value (applet->minutes) * 60;
    duration += gtk_spin_button_get_value (applet->seconds);

    g_settings_set_int (applet->settings, DURATION_KEY, duration);
}

static void
timer_preferences_dialog_response_cb (GtkDialog *dialog,
                                      gint       response_id,
                                      gpointer   data)
{
    TimerApplet *applet;

    applet = (TimerApplet *) data;

    gtk_widget_destroy (GTK_WIDGET (applet->preferences_dialog));
    applet->preferences_dialog = NULL;
}

/* Show the preferences dialog */
static void
timer_preferences_callback (GSimpleAction *action, GVariant *parameter, gpointer data)
{
    TimerApplet *applet;
    GtkGrid *grid;
    GtkWidget *widget;
    gint duration, hours, minutes, seconds;

    applet = (TimerApplet *) data;

    if (applet->preferences_dialog != NULL) {
        gtk_window_present (GTK_WINDOW (applet->preferences_dialog));
        return;
    }

    duration = g_settings_get_int (applet->settings, DURATION_KEY);
    hours = duration / 60 / 60;
    minutes = duration / 60 % 60;
    seconds = duration % 60;

    applet->preferences_dialog = gtk_dialog_new_with_buttons(_("Timer Applet Preferences"),
                                                     NULL,
                                                     0,
                                                     _("_Close"),
                                                     GTK_RESPONSE_CLOSE,
                                                     NULL);
    grid = GTK_GRID (gtk_grid_new ());
    gtk_grid_set_row_spacing (grid, 12);
    gtk_grid_set_column_spacing (grid, 12);

    gtk_window_set_default_size (GTK_WINDOW (applet->preferences_dialog), 350, 150);
    gtk_container_set_border_width (GTK_CONTAINER (applet->preferences_dialog), 10);

    widget = gtk_label_new (_("Name:"));
    gtk_label_set_xalign (GTK_LABEL (widget), 1.0);
    gtk_label_set_yalign (GTK_LABEL (widget), 0.5);
    gtk_grid_attach (grid, widget, 1, 0, 1, 1);

    widget = gtk_entry_new ();
    gtk_grid_attach (grid, widget, 2, 0, 1, 1);
    g_settings_bind (applet->settings, NAME_KEY, widget, "text", G_SETTINGS_BIND_DEFAULT);

    widget = gtk_label_new (_("Hours:"));
    gtk_label_set_xalign (GTK_LABEL (widget), 1.0);
    gtk_label_set_yalign (GTK_LABEL (widget), 0.5);
    gtk_grid_attach (grid, widget, 1, 1, 1, 1);

    widget = gtk_spin_button_new_with_range (0.0, 100.0, 1.0);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (widget), hours);
    gtk_grid_attach (grid, widget, 2, 1, 1, 1);
    applet->hours = GTK_SPIN_BUTTON (widget);
    g_signal_connect (widget, "value-changed", G_CALLBACK (timer_spin_button_value_changed), applet);

    widget = gtk_label_new (_("Minutes:"));
    gtk_label_set_xalign (GTK_LABEL (widget), 1.0);
    gtk_label_set_yalign (GTK_LABEL (widget), 0.5);
    gtk_grid_attach (grid, widget, 1, 2, 1, 1);

    widget = gtk_spin_button_new_with_range (0.0, 59.0, 1.0);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (widget), minutes);
    gtk_grid_attach (grid, widget, 2, 2, 1, 1);
    applet->minutes = GTK_SPIN_BUTTON (widget);
    g_signal_connect (widget, "value-changed", G_CALLBACK (timer_spin_button_value_changed), applet);

    widget = gtk_label_new (_("Seconds:"));
    gtk_label_set_xalign (GTK_LABEL (widget), 1.0);
    gtk_label_set_yalign (GTK_LABEL (widget), 0.5);
    gtk_grid_attach (grid, widget, 1, 3, 1, 1);

    widget = gtk_spin_button_new_with_range (0.0, 59.0, 1.0);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (widget), seconds);
    gtk_grid_attach (grid, widget, 2, 3, 1, 1);
    applet->seconds = GTK_SPIN_BUTTON (widget);
    g_signal_connect (widget, "value-changed", G_CALLBACK (timer_spin_button_value_changed), applet);

    widget = gtk_check_button_new_with_label (_("Show notification popup"));
    gtk_grid_attach (grid, widget, 2, 4, 1, 1);
    g_settings_bind (applet->settings, SHOW_NOTIFICATION_KEY, widget, "active", G_SETTINGS_BIND_DEFAULT);

    widget = gtk_check_button_new_with_label (_("Show dialog"));
    gtk_grid_attach (grid, widget, 2, 5, 1, 1);
    g_settings_bind (applet->settings, SHOW_DIALOG_KEY, widget, "active", G_SETTINGS_BIND_DEFAULT);

    gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (applet->preferences_dialog))), GTK_WIDGET (grid), TRUE, TRUE, 0);

    g_signal_connect (applet->preferences_dialog, "response", G_CALLBACK (timer_preferences_dialog_response_cb), applet);

    gtk_widget_show_all (applet->preferences_dialog);
}

static void
timer_settings_changed (GSettings *settings, gchar *key, TimerApplet *applet)
{
    timer_callback (applet);
}

static void
timer_applet_fill (TimerApplet *applet)
{
    const char *menu_resource;

    applet->settings = gp_applet_settings_new (GP_APPLET (applet), TIMER_SCHEMA);
    applet->timeout_id = 0;
    applet->active = FALSE;
    applet->pause = FALSE;

    applet->box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
    applet->image = GTK_IMAGE (gtk_image_new_from_icon_name (APPLET_ICON, GTK_ICON_SIZE_BUTTON));
    applet->pause_image = GTK_IMAGE (gtk_image_new_from_icon_name ("media-playback-pause", GTK_ICON_SIZE_BUTTON));
    applet->label = GTK_LABEL (gtk_label_new (""));

    gp_add_text_color_class (GTK_WIDGET (applet->label));

    /* we add the Gtk label into the applet */
    gtk_box_pack_start (applet->box,
                        GTK_WIDGET (applet->image),
                        TRUE, TRUE, 0);
    gtk_box_pack_start (applet->box,
                        GTK_WIDGET (applet->pause_image),
                        TRUE, TRUE, 0);
    gtk_box_pack_start (applet->box,
                        GTK_WIDGET (applet->label),
                        TRUE, TRUE, 3);

    gtk_container_add (GTK_CONTAINER (applet),
                       GTK_WIDGET (applet->box));

    gtk_widget_show_all (GTK_WIDGET (applet));
    gtk_widget_hide (GTK_WIDGET (applet->pause_image));

    /* set up context menu */
    menu_resource = GRESOURCE_PREFIX "/ui/timer-applet-menu.ui";
    gp_applet_setup_menu_from_resource (GP_APPLET (applet),
                                        menu_resource,
                                        applet_menu_actions);

    /* execute callback to set actions sensitiveness */
    timer_callback (applet);

    /* GSettings callback */
    g_signal_connect (G_OBJECT (applet->settings), "changed",
                      G_CALLBACK (timer_settings_changed), applet);
}

static void
timer_applet_constructed (GObject *object)
{
  G_OBJECT_CLASS (timer_applet_parent_class)->constructed (object);
  timer_applet_fill (TIMER_APPLET (object));
}

static void
timer_applet_finalize (GObject *object)
{
  TimerApplet *self;

  self = TIMER_APPLET (object);

  if (self->timeout_id != 0)
    {
      g_source_remove (self->timeout_id);
      self->timeout_id = 0;
    }

  g_object_unref (self->settings);

  g_clear_pointer (&self->preferences_dialog, gtk_widget_destroy);

  notify_uninit ();

  G_OBJECT_CLASS (timer_applet_parent_class)->finalize (object);
}

static void
timer_applet_class_init (TimerAppletClass *self_class)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (self_class);

  object_class->constructed = timer_applet_constructed;
  object_class->finalize = timer_applet_finalize;
}

static void
timer_applet_init (TimerApplet *self)
{
  GpApplet *applet;

  applet = GP_APPLET (self);

  if (!notify_is_initted ())
    notify_init ("timer-applet");

  gp_applet_set_flags (applet, GP_APPLET_FLAGS_EXPAND_MINOR);
}

void
timer_applet_setup_about (GtkAboutDialog *dialog)
{
  const char **authors;
  const char *copyright;

  authors = (const char *[])
    {
      "Stefano Karapetsas <stefano@karapetsas.com>",
      NULL
    };

  copyright = "Copyright © 2014 Stefano Karapetsas";

  gtk_about_dialog_set_authors (dialog, authors);
  gtk_about_dialog_set_translator_credits (dialog, _("translator-credits"));
  gtk_about_dialog_set_copyright (dialog, copyright);
}
