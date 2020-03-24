/*
 * Copyright (C) Papadimitriou Spiros <spapadim+@cs.cmu.edu>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>

#include <panel-applet.h>

#include <gdk/gdkkeysyms.h>

#include <libnotify/notify.h>

#include "gweather-pref.h"
#include "gweather-dialog.h"
#include "gweather-applet.h"

#define MAX_CONSECUTIVE_FAULTS (3)

static void update_finish (GWeatherInfo *info, gpointer data);

static GWeatherLocation*
get_default_location (GWeatherApplet *gw_applet)
{
    GWeatherLocation *world;
    GWeatherLocation *location;
    const gchar *station_code;
    GVariant *default_loc;

    default_loc = g_settings_get_value (gw_applet->lib_settings, "default-location");
    g_variant_get (default_loc, "(&s&sm(dd))", NULL, &station_code, NULL, NULL, NULL);

    world = gweather_location_get_world ();
    location = gweather_location_find_by_station_code (world, station_code);
    g_variant_unref (default_loc);

    return location;
}

static const gchar *authors[] =
  {
    "Todd Kulesza <fflewddur@dropline.net>",
    "Philip Langdale <philipl@mail.utexas.edu>",
    "Ryan Lortie <desrt@desrt.ca>",
    "Davyd Madeley <davyd@madeley.id.au>",
    "Spiros Papadimitriou <spapadim+@cs.cmu.edu>",
    "Kevin Vandersloot <kfv101@psu.edu>",
    NULL
  };

const gchar *documenters[] =
  {
    "Dan Mueth <d-mueth@uchicago.edu>",
    "Spiros Papadimitriou <spapadim+@cs.cmu.edu>",
    "Sun GNOME Documentation Team <gdocteam@sun.com>",
    "Davyd Madeley <davyd@madeley.id.au>",
    NULL
  };

static void
about_cb (GSimpleAction *action,
          GVariant      *parameter,
          gpointer       user_data)
{
  GWeatherApplet *applet;
  GtkAboutDialog *dialog;
  const gchar *text;

  applet = (GWeatherApplet *) user_data;

  if (applet->about_dialog != NULL)
    {
      gtk_window_present (GTK_WINDOW (applet->about_dialog));
      return;
    }

  applet->about_dialog = gtk_about_dialog_new ();
  dialog = GTK_ABOUT_DIALOG (applet->about_dialog);

  gtk_about_dialog_set_version (dialog, VERSION);
  gtk_about_dialog_set_logo_icon_name (dialog, "weather-storm");

  text = _("\xC2\xA9 1999-2005 by S. Papadimitriou and others");
  gtk_about_dialog_set_copyright (dialog, text);

  text = _("A panel application for monitoring local weather conditions.");
  gtk_about_dialog_set_copyright (dialog, text);

  gtk_about_dialog_set_authors (dialog, authors);
  gtk_about_dialog_set_documenters (dialog, documenters);
  gtk_about_dialog_set_translator_credits (dialog, _("translator-credits"));

  g_object_add_weak_pointer (G_OBJECT (applet->about_dialog),
                             (gpointer *) &applet->about_dialog);

  gtk_window_present (GTK_WINDOW (applet->about_dialog));
}

static void help_cb (GSimpleAction *action,
                     GVariant      *parameter,
                     gpointer       user_data)
{
	GWeatherApplet *gw_applet = (GWeatherApplet *) user_data;
    GError *error = NULL;

    gtk_show_uri (gtk_widget_get_screen (GTK_WIDGET (gw_applet->applet)),
		"help:gweather",
		gtk_get_current_event_time (),
		&error);

    if (error) { 
	GtkWidget *dialog = gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
						    _("There was an error displaying help: %s"), error->message);
	g_signal_connect (G_OBJECT (dialog), "response", G_CALLBACK (gtk_widget_destroy), NULL);
	gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
	gtk_window_set_screen (GTK_WINDOW (dialog), gtk_widget_get_screen (GTK_WIDGET (gw_applet->applet)));
	gtk_widget_show (dialog);
        g_error_free (error);
        error = NULL;
    }
}

static void pref_cb (GSimpleAction *action,
                     GVariant      *parameter,
                     gpointer       user_data)
{
   GWeatherApplet *gw_applet = (GWeatherApplet *) user_data;

   if (gw_applet->pref_dialog) {
	gtk_window_present (GTK_WINDOW (gw_applet->pref_dialog));
   } else {
	gw_applet->pref_dialog = gweather_pref_new(gw_applet);
	g_object_add_weak_pointer(G_OBJECT(gw_applet->pref_dialog),
				  (gpointer *)&(gw_applet->pref_dialog));
	gtk_widget_show_all (gw_applet->pref_dialog);
   }
}

static void details_cb (GSimpleAction *action,
                        GVariant      *parameter,
                        gpointer       user_data)
{
   GWeatherApplet *gw_applet = (GWeatherApplet *) user_data;

   if (gw_applet->details_dialog) {
	gtk_window_present (GTK_WINDOW (gw_applet->details_dialog));
   } else {
	gw_applet->details_dialog = gweather_dialog_new(gw_applet);
	g_object_add_weak_pointer(G_OBJECT(gw_applet->details_dialog),
				  (gpointer *)&(gw_applet->details_dialog));
	gweather_dialog_update (GWEATHER_DIALOG (gw_applet->details_dialog));
	gtk_widget_show (gw_applet->details_dialog);
   }
}

static void update_cb (GSimpleAction *action,
                       GVariant      *parameter,
                       gpointer       user_data)
{
	GWeatherApplet *gw_applet = (GWeatherApplet *) user_data;
    gweather_update (gw_applet);
}

static const GActionEntry weather_applet_menu_actions [] = {
	{ "details",     details_cb, NULL, NULL, NULL },
	{ "update",      update_cb,  NULL, NULL, NULL },
	{ "preferences", pref_cb,    NULL, NULL, NULL },
	{ "help",        help_cb,    NULL, NULL, NULL },
	{ "about",       about_cb,   NULL, NULL, NULL }
};

static void place_widgets (GWeatherApplet *gw_applet)
{
    GtkRequisition req;
    int total_size = 0;
    gboolean horizontal = FALSE;
    int panel_size = gw_applet->size;
    const gchar *icon_name;

    switch (gw_applet->orient) {
	case PANEL_APPLET_ORIENT_LEFT:
	case PANEL_APPLET_ORIENT_RIGHT:
	    horizontal = FALSE;
	    break;
	case PANEL_APPLET_ORIENT_UP:
	case PANEL_APPLET_ORIENT_DOWN:
	    horizontal = TRUE;
	    break;
    }

    /* Create the weather icon */
    if (gw_applet->gweather_info) {
        icon_name = gweather_info_get_icon_name (gw_applet->gweather_info);
    } else {
        icon_name = "image-missing";
    }
    gw_applet->image = gtk_image_new_from_icon_name(icon_name, GTK_ICON_SIZE_BUTTON); 

    if (icon_name != NULL) {
        gtk_widget_show (gw_applet->image);
        gtk_widget_get_preferred_size(gw_applet->image, &req, NULL);
        if (horizontal)
            total_size += req.height;
        else
            total_size += req.width;
    }

    /* Create the temperature label */
    gw_applet->label = gtk_label_new("--");
    panel_applet_add_text_class (gw_applet->applet, gw_applet->label);

    /* Update temperature text */
    if (gw_applet->gweather_info) {
        gchar *temp;

        temp = gweather_info_get_temp_summary (gw_applet->gweather_info);

        if (temp) {
            gtk_label_set_text (GTK_LABEL (gw_applet->label), temp);
            g_free (temp);
        }
    }

    /* Check the label size to determine box layout */
    gtk_widget_show (gw_applet->label);
    gtk_widget_get_preferred_size(gw_applet->label, &req, NULL);
    if (horizontal)
        total_size += req.height;
    else
        total_size += req.width;

    /* Pack the box */
    if (gw_applet->box)
        gtk_widget_destroy (gw_applet->box);

    if (horizontal && (total_size <= panel_size))
        gw_applet->box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    else if (horizontal && (total_size > panel_size))
        gw_applet->box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
    else if (!horizontal && (total_size <= panel_size))
        gw_applet->box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
    else 
        gw_applet->box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

    /* Rebuild the applet it's visual area */
    gtk_container_add (GTK_CONTAINER (gw_applet->container), gw_applet->box);
    gtk_box_pack_start (GTK_BOX (gw_applet->box), gw_applet->image, TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (gw_applet->box), gw_applet->label, TRUE, TRUE, 0);

    gtk_widget_show_all (GTK_WIDGET (gw_applet->applet));
}

static void change_orient_cb (PanelApplet *w, PanelAppletOrient o, gpointer data)
{
    GWeatherApplet *gw_applet = (GWeatherApplet *)data;
	
    gw_applet->orient = o;
    place_widgets(gw_applet);
    return;
}

static void size_allocate_cb(PanelApplet *w, GtkAllocation *allocation, gpointer data)
{
    GWeatherApplet *gw_applet = (GWeatherApplet *)data;
	
    if ((gw_applet->orient == PANEL_APPLET_ORIENT_LEFT) || (gw_applet->orient == PANEL_APPLET_ORIENT_RIGHT)) {
      if (gw_applet->size == allocation->width)
	return;
      gw_applet->size = allocation->width;
    } else {
      if (gw_applet->size == allocation->height)
	return;
      gw_applet->size = allocation->height;
    }
	
    place_widgets(gw_applet);
    return;
}

static gboolean clicked_cb (GtkWidget *widget, GdkEventButton *ev, gpointer data)
{
    GWeatherApplet *gw_applet = data;

    if ((ev == NULL) || (ev->button != 1))
        return FALSE;

    if (ev->type == GDK_BUTTON_PRESS) {
	if (!gw_applet->details_dialog)
		details_cb (NULL, NULL, gw_applet);
	else
		gtk_widget_destroy (GTK_WIDGET (gw_applet->details_dialog));
	
	return TRUE;
    }
    
    return FALSE;
}

static gboolean 
key_press_cb (GtkWidget *widget, GdkEventKey *event, GWeatherApplet *gw_applet)
{
	switch (event->keyval) {	
	case GDK_KEY_u:
		if (event->state == GDK_CONTROL_MASK) {
			gweather_update (gw_applet);
			return TRUE;
		}
		break;
	case GDK_KEY_d:
		if (event->state == GDK_CONTROL_MASK) {
			details_cb (NULL, NULL, gw_applet);
			return TRUE;
		}
		break;		
	case GDK_KEY_KP_Enter:
	case GDK_KEY_ISO_Enter:
	case GDK_KEY_3270_Enter:
	case GDK_KEY_Return:
	case GDK_KEY_space:
	case GDK_KEY_KP_Space:
		details_cb (NULL, NULL, gw_applet);
		return TRUE;
	default:
		break;
	}

	return FALSE;

}


static void
network_changed (GNetworkMonitor *monitor, gboolean available, GWeatherApplet *gw_applet)
{
    if (available) {
        gweather_update (gw_applet);
    }
}

static void
applet_destroy (GtkWidget *widget, GWeatherApplet *gw_applet)
{
    GNetworkMonitor *monitor;

    if (gw_applet->pref_dialog)
       gtk_widget_destroy (gw_applet->pref_dialog);

    if (gw_applet->details_dialog)
       gtk_widget_destroy (gw_applet->details_dialog);

    if (gw_applet->timeout_tag > 0) {
       g_source_remove(gw_applet->timeout_tag);
       gw_applet->timeout_tag = 0;
    }
	
    if (gw_applet->suncalc_timeout_tag > 0) {
       g_source_remove(gw_applet->suncalc_timeout_tag);
       gw_applet->suncalc_timeout_tag = 0;
    }
	
    g_clear_object (&gw_applet->lib_settings);
    g_clear_object (&gw_applet->applet_settings);

    monitor = g_network_monitor_get_default ();
    g_signal_handlers_disconnect_by_func (monitor,
                                          G_CALLBACK (network_changed),
                                          gw_applet);

    gweather_info_abort (gw_applet->gweather_info);
}

void gweather_applet_create (GWeatherApplet *gw_applet)
{
    GSimpleActionGroup *action_group;
    GAction *action;
    gchar          *ui_path;
    AtkObject      *atk_obj;
    GNetworkMonitor*monitor;

    panel_applet_set_flags (gw_applet->applet, PANEL_APPLET_EXPAND_MINOR);

    gw_applet->container = gtk_alignment_new (0.5, 0.5, 0, 0);
    gtk_container_add (GTK_CONTAINER (gw_applet->applet), gw_applet->container);

    g_signal_connect (G_OBJECT(gw_applet->applet), "change_orient",
                       G_CALLBACK(change_orient_cb), gw_applet);
    g_signal_connect (G_OBJECT(gw_applet->applet), "size_allocate",
                       G_CALLBACK(size_allocate_cb), gw_applet);
    g_signal_connect (G_OBJECT(gw_applet->applet), "destroy", 
                       G_CALLBACK (applet_destroy), gw_applet);
    g_signal_connect (G_OBJECT(gw_applet->applet), "button_press_event",
                       G_CALLBACK(clicked_cb), gw_applet);
    g_signal_connect (G_OBJECT(gw_applet->applet), "key_press_event",           
			G_CALLBACK(key_press_cb), gw_applet);
                     
    gtk_widget_set_tooltip_text (GTK_WIDGET(gw_applet->applet), _("GNOME Weather"));

    atk_obj = gtk_widget_get_accessible (GTK_WIDGET (gw_applet->applet));
    if (GTK_IS_ACCESSIBLE (atk_obj))
	   atk_object_set_name (atk_obj, _("GNOME Weather"));

    gw_applet->orient = panel_applet_get_orient (gw_applet->applet);

    action_group = g_simple_action_group_new ();
	g_action_map_add_action_entries (G_ACTION_MAP (action_group),
	                                 weather_applet_menu_actions,
	                                 G_N_ELEMENTS (weather_applet_menu_actions),
	                                 gw_applet);
    ui_path = g_build_filename (GWEATHER_MENU_UI_DIR, "gweather-applet-menu.xml", NULL);
    panel_applet_setup_menu_from_file (gw_applet->applet,
				       ui_path, action_group,
				       GETTEXT_PACKAGE);
    g_free (ui_path);

	gtk_widget_insert_action_group (GTK_WIDGET (gw_applet->applet), "gweather",
	                                G_ACTION_GROUP (action_group));

    action = g_action_map_lookup_action (G_ACTION_MAP (action_group), "preferences");
	g_object_bind_property (gw_applet->applet, "locked-down", action, "enabled",
				G_BINDING_DEFAULT|G_BINDING_INVERT_BOOLEAN|G_BINDING_SYNC_CREATE);

    g_object_unref (action_group);

    gw_applet->gweather_info = gweather_info_new (get_default_location (gw_applet));

    gweather_info_set_enabled_providers (gw_applet->gweather_info,
                                         GWEATHER_PROVIDER_ALL);

    g_signal_connect (gw_applet->gweather_info, "updated",
                      G_CALLBACK (update_finish), gw_applet);

    place_widgets(gw_applet);        

    monitor = g_network_monitor_get_default();
    g_signal_connect (monitor, "network-changed",
                      G_CALLBACK (network_changed), gw_applet);
}

gboolean
timeout_cb (gpointer data)
{
    GWeatherApplet *gw_applet = (GWeatherApplet *)data;

    gw_applet->timeout_tag = 0;
    gweather_update(gw_applet);

    /* Do not repeat timeout (will be re-set by gweather_update) */
    return G_SOURCE_REMOVE;
}

static void
update_finish (GWeatherInfo *info, gpointer data)
{
    static int gw_fault_counter = 0;
    char *message, *detail;
    GWeatherApplet *gw_applet = (GWeatherApplet *)data;
    gint nxtSunEvent;
    const gchar *icon_name;

    /* Update timer */
    if (gw_applet->timeout_tag > 0)
        g_source_remove(gw_applet->timeout_tag);
    if (g_settings_get_boolean (gw_applet->applet_settings, "auto-update"))
    {
	gw_applet->timeout_tag =
	  g_timeout_add_seconds (g_settings_get_int (gw_applet->applet_settings, "auto-update-interval"),
				 timeout_cb, gw_applet);

        nxtSunEvent = gweather_info_next_sun_event(gw_applet->gweather_info);
        if (nxtSunEvent >= 0)
            gw_applet->suncalc_timeout_tag =
                        g_timeout_add_seconds (nxtSunEvent,
                                suncalc_timeout_cb, gw_applet);
    }

    if ((TRUE == gweather_info_is_valid (info)) ||
	     (gw_fault_counter >= MAX_CONSECUTIVE_FAULTS))
    {
	    gchar *text;

	    gw_fault_counter = 0;

            icon_name = gweather_info_get_icon_name (gw_applet->gweather_info);
            gtk_image_set_from_icon_name (GTK_IMAGE(gw_applet->image), 
                                          icon_name, GTK_ICON_SIZE_BUTTON);

	    text = gweather_info_get_temp_summary (gw_applet->gweather_info);
	    gtk_label_set_text (GTK_LABEL (gw_applet->label), text);
	    g_free (text);

	    text = gweather_info_get_weather_summary (gw_applet->gweather_info);
	    gtk_widget_set_tooltip_text (GTK_WIDGET (gw_applet->applet), text);
	    g_free (text);

	    /* Update dialog -- if one is present */
	    if (gw_applet->details_dialog) {
	    	gweather_dialog_update (GWEATHER_DIALOG (gw_applet->details_dialog));
	    }

	    /* update applet */
	    place_widgets(gw_applet);

	    if (g_settings_get_boolean (gw_applet->applet_settings, "show-notifications"))
	    {
		    NotifyNotification *n;
	            
		    /* Show notifications if possible */
	            if (!notify_is_initted ())
	                notify_init (_("Weather Forecast"));

		    if (notify_is_initted ())
		    {
			 GError *error = NULL;
                         const char *icon;
                         gchar *location_name;
                         gchar *sky;
                         gchar *temp_summary;

                         location_name = gweather_info_get_location_name (info);
                         sky = gweather_info_get_sky (info);
                         temp_summary = gweather_info_get_temp_summary (info);
			 
	           	 /* Show notification */
	           	 message = g_strdup_printf ("%s: %s",
					 gweather_info_get_location_name (info),
					 gweather_info_get_sky (info));
	           	 detail = g_strdup_printf (
					 _("City: %s\nSky: %s\nTemperature: %s"),
					 location_name,
					 sky,
					 temp_summary);

                         g_free (location_name);
                         g_free (sky);
                         g_free (temp_summary);

			 icon = gweather_info_get_icon_name (gw_applet->gweather_info);
			 if (icon == NULL)
				 icon = "stock-unknown";
	           	 
			 n = notify_notification_new (message, detail, icon);
	
		   	 notify_notification_show (n, &error);
			 if (error)
			 {
				 g_warning ("%s", error->message);
				 g_error_free (error);
			 }
		   	     
		   	 g_free (message);
		   	 g_free (detail);
		    }
	    }
    }
    else
    {
	    /* there has been an error during retrival
	     * just update the fault counter
	     */
	    gw_fault_counter++;
    }
}

gboolean
suncalc_timeout_cb (gpointer data)
{
    GWeatherApplet *gw_applet = (GWeatherApplet *)data;
    GWeatherInfo *info = gw_applet->gweather_info;

    gw_applet->suncalc_timeout_tag = 0;
    update_finish(info, data);

    /* Do not repeat timeout (will be re-set by update_finish) */
    return G_SOURCE_REMOVE;
}

void gweather_update (GWeatherApplet *gw_applet)
{
    gtk_widget_set_tooltip_text (GTK_WIDGET(gw_applet->applet),  _("Updating..."));

    gweather_info_set_location (gw_applet->gweather_info, get_default_location (gw_applet));
    gweather_info_update (gw_applet->gweather_info);
}
