/* $Id$ */

/*
 *  Papadimitriou Spiros <spapadim+@cs.cmu.edu>
 *
 *  This code released under the GNU GPL.
 *  Read the file COPYING for more information.
 *
 *  Main applet widget
 *
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

#include <gconf/gconf-client.h>
#include <panel-applet.h>
#include <panel-applet-gconf.h>

#include <gdk/gdkkeysyms.h>

#ifdef HAVE_LIBNOTIFY
#include <libnotify/notify.h>
#include <libnotify/notification.h>
#endif

#define GWEATHER_I_KNOW_THIS_IS_UNSTABLE

#ifdef HAVE_NETWORKMANAGER
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <NetworkManager/NetworkManager.h>
#endif

#include "gweather.h"
#include "gweather-about.h"
#include "gweather-pref.h"
#include "gweather-dialog.h"
#include "gweather-applet.h"

#define MAX_CONSECUTIVE_FAULTS (3)

static void about_cb (GtkAction      *action,
		      GWeatherApplet *gw_applet)
{

    gweather_about_run (gw_applet);
}

static void help_cb (GtkAction      *action,
		     GWeatherApplet *gw_applet)
{
    GError *error = NULL;

    gtk_show_uri (gtk_widget_get_screen (GTK_WIDGET (gw_applet->applet)),
		"ghelp:gweather",
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

static void pref_cb (GtkAction      *action,
		     GWeatherApplet *gw_applet)
{
   if (gw_applet->pref_dialog) {
	gtk_window_present (GTK_WINDOW (gw_applet->pref_dialog));
   } else {
	gw_applet->pref_dialog = gweather_pref_new(gw_applet);
	g_object_add_weak_pointer(G_OBJECT(gw_applet->pref_dialog),
				  (gpointer *)&(gw_applet->pref_dialog));
	gtk_widget_show_all (gw_applet->pref_dialog);
   }
}

static void details_cb (GtkAction      *action,
			GWeatherApplet *gw_applet)
{
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

static void update_cb (GtkAction      *action,
		       GWeatherApplet *gw_applet)
{
    gweather_update (gw_applet);
}


static const GtkActionEntry weather_applet_menu_actions [] = {
	{ "Details", NULL, N_("_Details"),
	  NULL, NULL,
	  G_CALLBACK (details_cb) },
	{ "Update", GTK_STOCK_REFRESH, N_("_Update"),
	  NULL, NULL,
	  G_CALLBACK (update_cb) },
	{ "Props", GTK_STOCK_PROPERTIES, N_("_Preferences"),
	  NULL, NULL,
	  G_CALLBACK (pref_cb) },
	{ "Help", GTK_STOCK_HELP, N_("_Help"),
	  NULL, NULL,
	  G_CALLBACK (help_cb) },
	{ "About", GTK_STOCK_ABOUT, N_("_About"),
	  NULL, NULL,
	  G_CALLBACK (about_cb) }
};

static void place_widgets (GWeatherApplet *gw_applet)
{
    GtkRequisition req;
    int total_size = 0;
    gboolean horizontal = FALSE;
    int panel_size = gw_applet->size;
    const gchar *temp;   
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
    icon_name = gweather_info_get_icon_name (gw_applet->gweather_info);
    gw_applet->image = gtk_image_new_from_icon_name(icon_name, GTK_ICON_SIZE_BUTTON); 

    if (icon_name != NULL) {
        gtk_widget_size_request(gw_applet->image, &req);
        if (horizontal)
            total_size += req.height;
        else
            total_size += req.width;
    }

    /* Create the temperature label */
    gw_applet->label = gtk_label_new("--");
    
    /* Update temperature text */
    temp = gweather_info_get_temp_summary(gw_applet->gweather_info);
    if (temp) 
        gtk_label_set_text(GTK_LABEL(gw_applet->label), temp);

    /* Check the label size to determine box layout */
    gtk_widget_size_request(gw_applet->label, &req);
    if (horizontal)
        total_size += req.height;
    else
        total_size += req.width;

    /* Pack the box */
    if (gw_applet->box)
        gtk_widget_destroy (gw_applet->box);
    
    if (horizontal && (total_size <= panel_size))
        gw_applet->box = gtk_vbox_new(FALSE, 0);
    else if (horizontal && (total_size > panel_size))
        gw_applet->box = gtk_hbox_new(FALSE, 2);
    else if (!horizontal && (total_size <= panel_size))
        gw_applet->box = gtk_hbox_new(FALSE, 2);
    else 
        gw_applet->box = gtk_vbox_new(FALSE, 0);

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
		details_cb (NULL, gw_applet);
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
			details_cb (NULL, gw_applet);
			return TRUE;
		}
		break;		
	case GDK_KEY_KP_Enter:
	case GDK_KEY_ISO_Enter:
	case GDK_KEY_3270_Enter:
	case GDK_KEY_Return:
	case GDK_KEY_space:
	case GDK_KEY_KP_Space:
		details_cb (NULL, gw_applet);
		return TRUE;
	default:
		break;
	}

	return FALSE;

}

static void
applet_destroy (GtkWidget *widget, GWeatherApplet *gw_applet)
{
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

    gweather_info_abort (gw_applet->gweather_info);
}

#ifdef HAVE_NETWORKMANAGER
static void setup_network_monitor (GWeatherApplet *gw_applet);
#endif

void gweather_applet_create (GWeatherApplet *gw_applet)
{
    GtkActionGroup *action_group;
    gchar          *ui_path;
    AtkObject      *atk_obj;

    panel_applet_set_flags (gw_applet->applet, PANEL_APPLET_EXPAND_MINOR);

    panel_applet_set_background_widget(gw_applet->applet,
                                       GTK_WIDGET(gw_applet->applet));

    g_set_application_name (_("Weather Report"));

    gtk_window_set_default_icon_name ("weather-storm");

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

    gw_applet->size = panel_applet_get_size (gw_applet->applet);

    gw_applet->orient = panel_applet_get_orient (gw_applet->applet);

    action_group = gtk_action_group_new ("GWeather Applet Actions");
    gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);
    gtk_action_group_add_actions (action_group,
				  weather_applet_menu_actions,
				  G_N_ELEMENTS (weather_applet_menu_actions),
				  gw_applet);
    ui_path = g_build_filename (GWEATHER_MENU_UI_DIR, "gweather-applet-menu.xml", NULL);
    panel_applet_setup_menu_from_file (gw_applet->applet,
				       ui_path, action_group);
    g_free (ui_path);

    if (panel_applet_get_locked_down (gw_applet->applet)) {
	    GtkAction *action;

	    action = gtk_action_group_get_action (action_group, "Props");
	    gtk_action_set_visible (action, FALSE);
    }
    g_object_unref (action_group);
	
    place_widgets(gw_applet);        

#ifdef HAVE_NETWORKMANAGER
    setup_network_monitor (gw_applet);     
#endif
}

gint timeout_cb (gpointer data)
{
    GWeatherApplet *gw_applet = (GWeatherApplet *)data;
	
    gweather_update(gw_applet);
    return 0;  /* Do not repeat timeout (will be re-set by gweather_update) */
}

static void
update_finish (GWeatherInfo *info, gpointer data)
{
    static int gw_fault_counter = 0;
#ifdef HAVE_LIBNOTIFY
    char *message, *detail;
#endif
    char *s;
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
	    gw_fault_counter = 0;
            icon_name = gweather_info_get_icon_name (gw_applet->gweather_info);
            gtk_image_set_from_icon_name (GTK_IMAGE(gw_applet->image), 
                                          icon_name, GTK_ICON_SIZE_BUTTON);
	      
	    gtk_label_set_text (GTK_LABEL (gw_applet->label), 
				gweather_info_get_temp_summary(
					gw_applet->gweather_info));
	    
	    s = gweather_info_get_weather_summary (gw_applet->gweather_info);
	    gtk_widget_set_tooltip_text (GTK_WIDGET (gw_applet->applet), s);
	    g_free (s);

	    /* Update dialog -- if one is present */
	    if (gw_applet->details_dialog) {
	    	gweather_dialog_update (GWEATHER_DIALOG (gw_applet->details_dialog));
	    }

	    /* update applet */
	    place_widgets(gw_applet);

#ifdef HAVE_LIBNOTIFY
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
			 
	           	 /* Show notification */
	           	 message = g_strdup_printf ("%s: %s",
					 gweather_info_get_location_name (info),
					 gweather_info_get_sky (info));
	           	 detail = g_strdup_printf (
					 _("City: %s\nSky: %s\nTemperature: %s"),
					 gweather_info_get_location_name (info),
					 gweather_info_get_sky (info),
					 gweather_info_get_temp_summary (info));

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
#endif
    }
    else
    {
	    /* there has been an error during retrival
	     * just update the fault counter
	     */
	    gw_fault_counter++;
    }
}

gint suncalc_timeout_cb (gpointer data)
{
    GWeatherInfo *info = ((GWeatherApplet *)data)->gweather_info;
    update_finish(info, data);
    return 0;  /* Do not repeat timeout (will be re-set by update_finish) */
}


void gweather_update (GWeatherApplet *gw_applet)
{
    const gchar *icon_name;
    GWeatherForecastType type;

    icon_name = gweather_info_get_icon_name(gw_applet->gweather_info);
    gtk_image_set_from_icon_name (GTK_IMAGE (gw_applet->image), 
    			          icon_name, GTK_ICON_SIZE_BUTTON); 
    gtk_widget_set_tooltip_text (GTK_WIDGET(gw_applet->applet),  _("Updating..."));

    /* Set preferred forecast type */
    type = g_settings_get_boolean (gw_applet->applet_settings, "detailed") ?
      GWEATHER_FORECAST_ZONE : GWEATHER_FORECAST_STATE;

    /* Update current conditions */
    g_object_unref(gw_applet->gweather_info);
    gw_applet->gweather_info = gweather_info_new(NULL, /* default location */
						 type);
    g_signal_connect(gw_applet->gweather_info, "updated", G_CALLBACK (update_finish), gw_applet);
}

#ifdef HAVE_NETWORKMANAGER
static void
state_notify (DBusPendingCall *pending, gpointer data)
{
	GWeatherApplet *gw_applet = data;

        DBusMessage *msg = dbus_pending_call_steal_reply (pending);

        if (!msg)
                return;

        if (dbus_message_get_type (msg) == DBUS_MESSAGE_TYPE_METHOD_RETURN) {
                dbus_uint32_t result;

                if (dbus_message_get_args (msg, NULL,
                                           DBUS_TYPE_UINT32, &result,
                                           DBUS_TYPE_INVALID)) {
                        if (result == NM_STATE_CONNECTED) {
				/* thank you, glibc */
				res_init ();
				gweather_update (gw_applet);
                        }
                }
        }

        dbus_message_unref (msg);
}

static void
check_network (DBusConnection *connection, gpointer user_data)
{
        DBusMessage *message;
        DBusPendingCall *reply;

        message = dbus_message_new_method_call (NM_DBUS_SERVICE,
                                                NM_DBUS_PATH,
                                                NM_DBUS_INTERFACE,
                                                "state");
        if (dbus_connection_send_with_reply (connection, message, &reply, -1)) {
                dbus_pending_call_set_notify (reply, state_notify, user_data, NULL);
                dbus_pending_call_unref (reply);
        }

        dbus_message_unref (message);
}

static DBusHandlerResult
filter_func (DBusConnection *connection, DBusMessage *message, void *user_data)
{
    if (dbus_message_is_signal (message, 
				NM_DBUS_INTERFACE, 
				"StateChanged")) {
	check_network (connection, user_data);

        return DBUS_HANDLER_RESULT_HANDLED;
    }

    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static void
setup_network_monitor (GWeatherApplet *gw_applet)
{
    GError *error;
    static DBusGConnection *bus = NULL;
    DBusConnection *dbus;

    if (bus == NULL) {
        error = NULL;
        bus = dbus_g_bus_get (DBUS_BUS_SYSTEM, &error);
        if (bus == NULL) {
            g_warning ("Couldn't connect to system bus: %s",
                       error->message);
            g_error_free (error);

            return;
        }

        dbus = dbus_g_connection_get_connection (bus);	
        dbus_connection_add_filter (dbus, filter_func, gw_applet, NULL);
        dbus_bus_add_match (dbus,
                            "type='signal',"
                            "interface='" NM_DBUS_INTERFACE "'",
                            NULL);
    }
}	
#endif /* HAVE_NETWORKMANAGER */
