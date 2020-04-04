/*
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
 *     Jörgen Scheibengruber <mfcn@gmx.de>
 */

#include "config.h"
#include "preferences.h"

#include <glib/gi18n-lib.h>

#include "backend.h"

struct _NetspeedPreferences
{
  GtkDialog       parent;

  NetspeedApplet *netspeed;
  GSettings      *settings;
};

G_DEFINE_TYPE (NetspeedPreferences, netspeed_preferences, GTK_TYPE_DIALOG)

static void
device_change_cb (GtkComboBox *combo_box,
                  gpointer     user_data)
{
  NetspeedPreferences *preferences;
  gboolean auto_change_device;
  gint active;
  gint i;

  preferences = NETSPEED_PREFERENCES (user_data);

  auto_change_device = g_settings_get_boolean (preferences->settings, "auto-change-device");
  active = gtk_combo_box_get_active (combo_box);

  if (active == 0) {
    if (auto_change_device)
      return;

    auto_change_device = TRUE;

    g_settings_set_string (preferences->settings, "device", "");
  } else {
    GList *devices;

    auto_change_device = FALSE;
    devices = g_object_get_data (G_OBJECT (combo_box), "devices");

    for (i = 1; i < active; i++)
      devices = devices->next;

    g_settings_set_string (preferences->settings, "device", devices->data);
  }

  g_settings_set_boolean (preferences->settings, "auto-change-device", auto_change_device);
}

static void
auto_change_device_settings_changed (GSettings   *settings,
                                     const gchar *key,
                                     gpointer     user_data)
{
  gboolean auto_change_device;
  gchar *device;

  auto_change_device = g_settings_get_boolean (settings, "auto-change-device");
  device = g_settings_get_string (settings, "device");

  if (auto_change_device) {
    if (g_strcmp0 (device, "") != 0)
      g_settings_set_string (settings, "device", "");
  } else {
    if (g_strcmp0 (device, "") == 0) {
      gchar *tmp_device;

      tmp_device = netspeed_applet_get_auto_device_name ();
      g_settings_set_string (settings, "device", tmp_device);
      g_free (tmp_device);
    }
  }

  g_free (device);
}

static void
device_settings_changed (GSettings   *settings,
                         const gchar *key,
                         gpointer     user_data)
{
  GtkComboBox *combo_box;
  GList *devices;
  GList *ptr;
  gchar *device;
  gint active;
  gint i;

  if (g_strcmp0 (key, "device") != 0)
    return;

  combo_box = GTK_COMBO_BOX (user_data);
  device = g_settings_get_string (settings, "device");
  ptr = devices = get_available_devices ();

  active = 0;
  for (i = 1; ptr; ptr = ptr->next) {
    if (g_str_equal (ptr->data, device)) {
      active = i;
      break;
    }

    i++;
  }

  gtk_combo_box_set_active (combo_box, active);

  if (active == 0 && g_strcmp0 (device, "") != 0) {
    g_settings_set_string (settings, "device", "");
    g_settings_set_boolean (settings, "auto-change-device", TRUE);
  }

  g_list_free_full (devices, g_free);
  g_free (device);
}

static void
free_devices (gpointer data)
{
  g_list_free_full (data, g_free);
}

static GtkWidget *
create_network_hbox (NetspeedPreferences *preferences)
{
  GtkWidget *network_device_hbox;
  GtkWidget *network_device_label;
  GtkWidget *network_device_combo;
  GList *ptr;
  GList *devices;
  gint i;
  gint active;

  network_device_hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);

  network_device_label = gtk_label_new_with_mnemonic (_("Network _device:"));
  gtk_label_set_justify (GTK_LABEL (network_device_label), GTK_JUSTIFY_LEFT);
  gtk_label_set_xalign (GTK_LABEL (network_device_label), 0.0);
  gtk_box_pack_start (GTK_BOX (network_device_hbox), network_device_label, FALSE, FALSE, 0);

  network_device_combo = gtk_combo_box_text_new ();
  gtk_label_set_mnemonic_widget (GTK_LABEL (network_device_label), network_device_combo);
  gtk_box_pack_start (GTK_BOX (network_device_hbox), network_device_combo, TRUE, TRUE, 0);

  /* Default means device with default route set */
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (network_device_combo), _("Default"));

  active = 0;
  ptr = devices = get_available_devices ();
  for (i = 1; ptr; ptr = ptr->next) {
    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (network_device_combo), ptr->data);
    if (g_str_equal (ptr->data, netspeed_applet_get_current_device_name (preferences->netspeed)))
      active = i;
    i++;
  }

  if (g_settings_get_boolean (preferences->settings, "auto-change-device"))
    active = 0;

  gtk_combo_box_set_active (GTK_COMBO_BOX (network_device_combo), active);

  g_object_set_data_full (G_OBJECT (network_device_combo),
                          "devices", devices,
                          free_devices);

  g_signal_connect (network_device_combo, "changed",
                    G_CALLBACK (device_change_cb), preferences);
  g_signal_connect (preferences->settings, "changed::auto-change-device",
                    G_CALLBACK (auto_change_device_settings_changed), network_device_combo);
  g_signal_connect (preferences->settings, "changed::device",
                    G_CALLBACK (device_settings_changed), network_device_combo);

  return network_device_hbox;
}

static void
netspeed_preferences_setup (NetspeedPreferences *preferences)
{
  GtkDialog *dialog;
  GtkWidget *vbox;
  GtkWidget *categories;
  GtkWidget *category;
  gchar *header_title;
  GtkWidget *category_label;
  GtkWidget *hbox;
  GtkWidget *indent_label;
  GtkWidget *controls;
  GtkWidget *show_sum_checkbutton;
  GtkWidget *show_bits_checkbutton;
  GtkWidget *change_icon_checkbutton;

  dialog = GTK_DIALOG (preferences);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 10);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 12);

  categories = gtk_box_new (GTK_ORIENTATION_VERTICAL, 18);
  gtk_box_pack_start (GTK_BOX (vbox), categories, TRUE, TRUE, 0);

  category = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
  gtk_box_pack_start (GTK_BOX (categories), category, TRUE, TRUE, 0);

  header_title = g_strconcat ("<span weight=\"bold\">", _("General Settings"), "</span>", NULL);
  category_label = gtk_label_new (header_title);
  g_free (header_title);

  gtk_label_set_use_markup (GTK_LABEL (category_label), TRUE);
  gtk_label_set_justify (GTK_LABEL (category_label), GTK_JUSTIFY_LEFT);
  gtk_label_set_xalign (GTK_LABEL (category_label), 0.0);
  gtk_box_pack_start (GTK_BOX (category), category_label, FALSE, FALSE, 0);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_pack_start (GTK_BOX (category), hbox, TRUE, TRUE, 0);

  indent_label = gtk_label_new ("    ");
  gtk_label_set_justify (GTK_LABEL (indent_label), GTK_JUSTIFY_LEFT);
  gtk_box_pack_start (GTK_BOX (hbox), indent_label, FALSE, FALSE, 0);

  controls = gtk_box_new (GTK_ORIENTATION_VERTICAL, 10);
  gtk_box_pack_start (GTK_BOX (hbox), controls, TRUE, TRUE, 0);

  gtk_box_pack_start (GTK_BOX (controls), create_network_hbox (preferences), TRUE, TRUE, 0);

  show_sum_checkbutton = gtk_check_button_new_with_mnemonic (_("Show _sum instead of in & out"));
  gtk_box_pack_start (GTK_BOX (controls), show_sum_checkbutton, FALSE, FALSE, 0);
  g_settings_bind (preferences->settings, "show-sum",
                   show_sum_checkbutton, "active",
                   G_SETTINGS_BIND_DEFAULT);

  show_bits_checkbutton = gtk_check_button_new_with_mnemonic (_("Show _bits instead of bytes"));
  gtk_box_pack_start (GTK_BOX (controls), show_bits_checkbutton, FALSE, FALSE, 0);
  g_settings_bind (preferences->settings, "show-bits",
                   show_bits_checkbutton, "active",
                   G_SETTINGS_BIND_DEFAULT);

  change_icon_checkbutton = gtk_check_button_new_with_mnemonic (_("Change _icon according to the selected device"));
  gtk_box_pack_start (GTK_BOX (controls), change_icon_checkbutton, FALSE, FALSE, 0);
  g_settings_bind (preferences->settings, "change-icon",
                   change_icon_checkbutton, "active",
                   G_SETTINGS_BIND_DEFAULT);

  gtk_container_add (GTK_CONTAINER (gtk_dialog_get_content_area (dialog)), vbox);
}

static void
netspeed_preferences_finalize (GObject *object)
{
  G_OBJECT_CLASS (netspeed_preferences_parent_class)->finalize (object);
}

static void
netspeed_preferences_response (GtkDialog *dialog,
                               gint       response_id)
{
  NetspeedPreferences *preferences;

  preferences = NETSPEED_PREFERENCES (dialog);

  switch (response_id) {
    case GTK_RESPONSE_HELP:
      gp_applet_show_help (GP_APPLET (preferences->netspeed),
                           "netspeed_applet-settings");
      break;
    default:
      gtk_widget_destroy (GTK_WIDGET (preferences));
      break;
  }
}

static void
netspeed_preferences_class_init (NetspeedPreferencesClass *preferences_class)
{
  GObjectClass *object_class;
  GtkDialogClass *dialog_class;

  object_class = G_OBJECT_CLASS (preferences_class);
  dialog_class = GTK_DIALOG_CLASS (preferences_class);

  object_class->finalize = netspeed_preferences_finalize;

  dialog_class->response = netspeed_preferences_response;
}

static void
netspeed_preferences_init (NetspeedPreferences *preferences)
{
}

GtkWidget *
netspeed_preferences_new (NetspeedApplet *netspeed)
{
  NetspeedPreferences *preferences;
  GtkDialog *dialog;
  GtkWidget *widget;

  preferences = g_object_new (NETSPEED_TYPE_PREFERENCES,
                              "title", _("Netspeed Preferences"),
                              "screen", gtk_widget_get_screen (GTK_WIDGET (netspeed)),
                              "resizable", FALSE,
                              NULL);

  preferences->netspeed = netspeed;
  preferences->settings = netspeed_applet_get_settings (netspeed);

  dialog = GTK_DIALOG (preferences);
  widget = GTK_WIDGET (dialog);

  gtk_dialog_add_buttons (dialog,
                          _("_Help"), GTK_RESPONSE_HELP,
                          _("_Close"), GTK_RESPONSE_ACCEPT,
                          NULL);
  gtk_dialog_set_default_response (dialog, GTK_RESPONSE_CLOSE);

  gtk_window_set_destroy_with_parent (GTK_WINDOW (dialog), TRUE);

  netspeed_preferences_setup (preferences);

  gtk_widget_show_all (widget);

  return widget;
}
