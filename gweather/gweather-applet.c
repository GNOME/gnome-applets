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

#include <gnome.h>
#include <panel-applet.h>
#include <libgnomeui/gnome-window-icon.h>
#include <egg-screen-help.h>

#include "weather.h"
#include "gweather.h"
#include "gweather-about.h"
#include "gweather-pref.h"
#include "gweather-dialog.h"
#include "gweather-applet.h"

static void place_widgets (GWeatherApplet *gw_applet)
{
    GtkRequisition req;
    int total_size = 0;
    gboolean horizontal = FALSE;
    int panel_size = gw_applet->size;
    const gchar *temp;   
	
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
    weather_info_get_pixbuf_mini(gw_applet->gweather_info, 
    				 &(gw_applet->applet_pixbuf));     
    gw_applet->image = gtk_image_new_from_pixbuf (gw_applet->applet_pixbuf);
    gtk_image_set_from_pixbuf (GTK_IMAGE (gw_applet->image), 
    			       gw_applet->applet_pixbuf);

    /* Check the weather icon sizes to determine box layout */
    if (gw_applet->applet_pixbuf != NULL) {
        gtk_widget_size_request(gw_applet->image, &req);
        if (horizontal)
            total_size += req.height;
        else
            total_size += req.width;
    }

    /* Create the temperature label */
    gw_applet->label = gtk_label_new("0\302\260F");
    
    /* Update temperature text */
    temp = weather_info_get_temp_summary(gw_applet->gweather_info);
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

static void change_size_cb(PanelApplet *w, gint s, gpointer data)
{
    GWeatherApplet *gw_applet = (GWeatherApplet *)data;
	
    gw_applet->size = s;
    place_widgets(gw_applet);
    return;
}

static void change_background_cb (
				PanelApplet *a, 
				PanelAppletBackgroundType type,
				GdkColor *color, GdkPixmap *pixmap, 
				gpointer data)
{
	GWeatherApplet *gw_applet = (GWeatherApplet *) data;
	GtkRcStyle *rc_style = gtk_rc_style_new ();

	switch (type) {
		case PANEL_PIXMAP_BACKGROUND:
			gtk_widget_modify_style (GTK_WIDGET (gw_applet->applet), rc_style);
			break;

		case PANEL_COLOR_BACKGROUND:
			gtk_widget_modify_bg (GTK_WIDGET (gw_applet->applet), GTK_STATE_NORMAL, color);
			break;

		case PANEL_NO_BACKGROUND:
			gtk_widget_modify_style (GTK_WIDGET (gw_applet->applet), rc_style);
			break;

		default:
			gtk_widget_modify_style (GTK_WIDGET (gw_applet->applet), rc_style);
			break;
	}

	gtk_rc_style_unref (rc_style);
}


static gboolean clicked_cb (GtkWidget *widget, GdkEventButton *ev, gpointer data)
{
    GWeatherApplet *gw_applet = data;

    if ((ev == NULL) || (ev->button != 1))
        return FALSE;

    if (ev->type == GDK_BUTTON_PRESS) {
	gweather_dialog_open(gw_applet);
	return TRUE;
    }
    
    return FALSE;
}

static gboolean 
key_press_cb (GtkWidget *widget, GdkEventKey *event, GWeatherApplet *gw_applet)
{
	switch (event->keyval) {	
	case GDK_u:
		if (event->state == GDK_CONTROL_MASK) {
			gweather_update (gw_applet);
			return TRUE;
		}
		break;
	case GDK_f:
		if (event->state == GDK_CONTROL_MASK) {
			gweather_dialog_open (gw_applet);
			return TRUE;
		}
		break;		
	case GDK_KP_Enter:
	case GDK_ISO_Enter:
	case GDK_3270_Enter:
	case GDK_Return:
	case GDK_space:
	case GDK_KP_Space:
		gweather_dialog_open(gw_applet);
		return TRUE;
		break;
	default:
		break;
	}

	return FALSE;

}

static void about_cb (BonoboUIComponent *uic,
		      GWeatherApplet    *gw_applet,
		      const gchar       *verbname)
{

    gweather_about_run (gw_applet);
}

static void help_cb (BonoboUIComponent *uic,
		     GWeatherApplet    *gw_applet,
		     const gchar       *verbname)
{
    GError *error = NULL;

    egg_help_display_on_screen (
		"gweather", NULL,
		gtk_widget_get_screen (GTK_WIDGET (gw_applet->applet)),
		&error);

    if (error) { /* FIXME: the user needs to see this error */
        g_warning ("help error: %s\n", error->message);
        g_error_free (error);
        error = NULL;
    }
}

static void pref_cb (BonoboUIComponent *uic,
		     GWeatherApplet    *gw_applet,
		     const gchar       *verbname)
{
    gweather_pref_run (gw_applet);
}

static void forecast_cb (BonoboUIComponent *uic,
		         GWeatherApplet    *gw_applet,
			 const gchar       *verbname)
{
    gweather_dialog_open (gw_applet);
}

static void update_cb (BonoboUIComponent *uic,
		       GWeatherApplet    *gw_applet,
		       const gchar       *verbname)
{
    gweather_update (gw_applet);
}


static const BonoboUIVerb weather_applet_menu_verbs [] = {
	BONOBO_UI_UNSAFE_VERB ("Forecast", forecast_cb),
	BONOBO_UI_UNSAFE_VERB ("Update", update_cb),
        BONOBO_UI_UNSAFE_VERB ("Props", pref_cb),
        BONOBO_UI_UNSAFE_VERB ("Help", help_cb), 
        BONOBO_UI_UNSAFE_VERB ("About", about_cb),

        BONOBO_UI_VERB_END
};

static void
applet_destroy (GtkWidget *widget, GWeatherApplet *gw_applet)
{
    if (gw_applet->pref)
       gtk_widget_destroy (gw_applet->pref);

    if (gw_applet->about_dialog)
	gtk_widget_destroy (gw_applet->about_dialog);

    if (gw_applet->gweather_dialog)
       gtk_widget_destroy (gw_applet->gweather_dialog);

    if (gw_applet->timeout_tag > 0) {
       gtk_timeout_remove(gw_applet->timeout_tag);
       gw_applet->timeout_tag = 0;
    }
	
    if (gw_applet->gweather_info->metar_handle) {
       gnome_vfs_async_cancel(gw_applet->gweather_info->metar_handle);
       gw_applet->gweather_info->metar_handle = 0;
    }

    if (gw_applet->gweather_info->iwin_handle) {
       gnome_vfs_async_cancel(gw_applet->gweather_info->iwin_handle);
       gw_applet->gweather_info->iwin_handle = 0;
    }

    if (gw_applet->gweather_info->wx_handle) {
       gnome_vfs_async_cancel(gw_applet->gweather_info->wx_handle);
       gw_applet->gweather_info->wx_handle = 0;
    }

    if (gw_applet->gweather_info->met_handle) {
       gnome_vfs_async_cancel(gw_applet->gweather_info->met_handle);
       gw_applet->gweather_info->met_handle = 0;
    }

    if (gw_applet->gweather_info->bom_handle) {
       gnome_vfs_async_cancel(gw_applet->gweather_info->bom_handle);
       gw_applet->gweather_info->bom_handle = 0;
    }
}

void gweather_applet_create (GWeatherApplet *gw_applet)
{
    GtkWidget *label;
    GtkWidget *pixmap;
    GtkTooltips *tooltips;
    GtkIconInfo *icon_info;
    AtkObject *atk_obj;

    gw_applet->about_dialog = NULL;
 
    gw_applet->gweather_pref.location = NULL;
    gw_applet->gweather_pref.update_interval = 1800;
    gw_applet->gweather_pref.update_enabled = TRUE;
    gw_applet->gweather_pref.use_metric = FALSE;
    gw_applet->gweather_pref.detailed = FALSE;
    gw_applet->gweather_pref.radar_enabled = TRUE;
    
    panel_applet_set_flags (gw_applet->applet, PANEL_APPLET_EXPAND_MINOR);
    
    icon_info = gtk_icon_theme_lookup_icon (gtk_icon_theme_get_default (), "stock_weather-storm", 48, 0);
    if (icon_info) {
        gnome_window_icon_set_default_from_file (gtk_icon_info_get_filename (icon_info));
        gtk_icon_info_free (icon_info);
    }

    gw_applet->container = gtk_alignment_new (0.5, 0.5, 0, 0);
    gtk_container_add (GTK_CONTAINER (gw_applet->applet), gw_applet->container);

    g_signal_connect (G_OBJECT(gw_applet->applet), "change_orient",
                       G_CALLBACK(change_orient_cb), gw_applet);
    g_signal_connect (G_OBJECT(gw_applet->applet), "change_size",
                       G_CALLBACK(change_size_cb), gw_applet);
    g_signal_connect (G_OBJECT(gw_applet->applet), "change_background",
		       G_CALLBACK(change_background_cb), gw_applet);
    g_signal_connect (G_OBJECT(gw_applet->applet), "destroy", 
                       G_CALLBACK (applet_destroy), gw_applet);
    gtk_signal_connect (GTK_OBJECT(gw_applet->applet), "button_press_event",
                       GTK_SIGNAL_FUNC(clicked_cb), gw_applet);
    g_signal_connect (G_OBJECT(gw_applet->applet), "key_press_event",           
			G_CALLBACK(key_press_cb), gw_applet);                    
                     
    tooltips = gtk_tooltips_new();

    gtk_tooltips_set_tip(tooltips, GTK_WIDGET(gw_applet->applet), _("GNOME Weather"), NULL);

    atk_obj = gtk_widget_get_accessible (GTK_WIDGET (gw_applet->applet));
    if (GTK_IS_ACCESSIBLE (atk_obj))
	   atk_object_set_name (atk_obj, _("GNOME Weather"));

    gw_applet->size = panel_applet_get_size (gw_applet->applet);

    gw_applet->orient = panel_applet_get_orient (gw_applet->applet);
    
    panel_applet_setup_menu_from_file (gw_applet->applet,
                                       NULL,
			               "GNOME_GWeatherApplet.xml",
                                       NULL,
			               weather_applet_menu_verbs,
			               gw_applet);

    if (panel_applet_get_locked_down (gw_applet->applet)) {
	    BonoboUIComponent *popup_component;

	    popup_component = panel_applet_get_popup_component (gw_applet->applet);

	    bonobo_ui_component_set_prop (popup_component,
					  "/commands/Props",
					  "hidden", "1",
					  NULL);
    }

    gw_applet->tooltips = tooltips;
	
    place_widgets(gw_applet);
  
	return;
}


void gweather_info_save (const gchar *path, GWeatherApplet *gw_applet)
{
    gchar *prefix;

    g_return_if_fail(gw_applet->gweather_info != NULL);
    g_return_if_fail(path != NULL);

    prefix = g_strconcat (path, "WeatherInfo/", NULL);
    gnome_config_push_prefix(prefix);
    /* fprintf(stderr, "gweather_info_save: %s\n", prefix); */
    g_free(prefix);

    weather_info_config_write(gw_applet->gweather_info);

    gnome_config_pop_prefix();
    gnome_config_sync();
    gnome_config_drop_all();
}

void gweather_info_load (const gchar *path, GWeatherApplet *gw_applet)
{
    gchar *prefix;

    g_return_if_fail(path != NULL);

    prefix = g_strconcat (path, "WeatherInfo/", NULL);
    gnome_config_push_prefix(prefix);
    /* fprintf(stderr, "gweather_info_save: %s\n", prefix); */
    g_free(prefix);

    weather_info_free (gw_applet->gweather_info);
    gw_applet->gweather_info = weather_info_config_read(gw_applet->applet);

    gnome_config_pop_prefix();
}

gint timeout_cb (gpointer data)
{
    GWeatherApplet *gw_applet = (GWeatherApplet *)data;
	
    gweather_update(gw_applet);
    return 0;  /* Do not repeat timeout (will be re-set by gweather_update) */
}

void update_finish (WeatherInfo *info)
{
    char *s;
    GWeatherApplet *gw_applet = info->applet;
   
    weather_info_get_pixbuf_mini(gw_applet->gweather_info, 
    				 &(gw_applet->applet_pixbuf));
    gtk_image_set_from_pixbuf (GTK_IMAGE (gw_applet->image), 
    			       gw_applet->applet_pixbuf);

    if (gw_applet->gweather_pref.use_metric)
        weather_info_to_metric (gw_applet->gweather_info);
      
    gtk_label_set_text(GTK_LABEL(gw_applet->label), 
			weather_info_get_temp_summary(gw_applet->gweather_info));
    
    s = weather_info_get_weather_summary(gw_applet->gweather_info);
    gtk_tooltips_set_tip(gw_applet->tooltips, GTK_WIDGET(gw_applet->applet), s, NULL);
    g_free (s);


    /* Update timer */
    if (gw_applet->timeout_tag > 0)
        gtk_timeout_remove(gw_applet->timeout_tag);
    if (gw_applet->gweather_pref.update_enabled)
        gw_applet->timeout_tag =  
        	gtk_timeout_add (gw_applet->gweather_pref.update_interval * 1000,
                                 timeout_cb, gw_applet);

    /* Update dialog -- if one is present */
    gweather_dialog_update(gw_applet);
}

void gweather_update (GWeatherApplet *gw_applet)
{
    gboolean update_success;

    weather_info_get_pixbuf_mini(gw_applet->gweather_info, 
    				 &(gw_applet->applet_pixbuf));

    gtk_image_set_from_pixbuf (GTK_IMAGE (gw_applet->image), 
    			       gw_applet->applet_pixbuf);
    gtk_tooltips_set_tip(gw_applet->tooltips, GTK_WIDGET(gw_applet->applet), 
    			 _("Updating..."), NULL);

    /* Set preferred forecast type */
    weather_forecast_set(gw_applet->gweather_pref.detailed ? 
    			 	FORECAST_ZONE : FORECAST_STATE);

    /* Set radar map retrieval option */
    weather_radar_set(gw_applet->gweather_pref.radar_enabled);

    /* Update current conditions */
    if (gw_applet->gweather_info && 
    	weather_location_equal(gw_applet->gweather_info->location, 
    			       gw_applet->gweather_pref.location)) {
        update_success = weather_info_update(gw_applet, 
        				      gw_applet->gweather_info, 
        				      update_finish);
    } else {
        weather_info_free(gw_applet->gweather_info);
        gw_applet->gweather_info = NULL;
        update_success = weather_info_new((gpointer)gw_applet, 
        		 gw_applet->gweather_pref.location, update_finish);
    }
    
    return;
}
