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

/* FIX - This code is WAY too kludgy!... */
static void place_widgets (GWeatherApplet *gw_applet)
{
    gint size = gw_applet->size;
    const gchar *temp;
 
    if (gw_applet->box)
        gtk_widget_destroy (gw_applet->box);
    
    if (((gw_applet->orient == PANEL_APPLET_ORIENT_LEFT) || 
         (gw_applet->orient == PANEL_APPLET_ORIENT_RIGHT)) ^ (size < 25)) {
         gw_applet->box = gtk_hbox_new (FALSE, 2);
         
    }
    else {
         gw_applet->box = gtk_vbox_new (FALSE, 2);
    }
    
    gtk_container_add (GTK_CONTAINER (gw_applet->applet), gw_applet->box);
    
    weather_info_get_pixbuf_mini(gw_applet->gweather_info, 
    				 &(gw_applet->applet_pixbuf));     
    gw_applet->image = gtk_image_new_from_pixbuf (gw_applet->applet_pixbuf);
    gtk_box_pack_start (GTK_BOX (gw_applet->box), gw_applet->image, FALSE, FALSE, 0);
         
    gw_applet->label = gtk_label_new("0\302\260");
    gtk_box_pack_start (GTK_BOX (gw_applet->box), gw_applet->label, FALSE, FALSE, 0);
    
    gtk_image_set_from_pixbuf (GTK_IMAGE (gw_applet->image), 
    			       gw_applet->applet_pixbuf);

    /* Update temperature text */

    temp = weather_info_get_temp_summary(gw_applet->gweather_info);
    if (temp)
    	gtk_label_set_text(GTK_LABEL(gw_applet->label), temp);

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


static gboolean clicked_cb (GtkWidget *widget, GdkEventButton *ev, gpointer data)
{
    GWeatherApplet *gw_applet = data;
    if ((ev == NULL) || (ev->button != 1))
	    return FALSE;
    if (ev->type == GDK_2BUTTON_PRESS) {
	gweather_dialog_open(gw_applet);
	return TRUE;
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

    egg_screen_help_display (
		gtk_widget_get_screen (GTK_WIDGET (gw_applet->applet)),
		"gweather", NULL, &error);
 
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

    if (gw_applet->gweather_dialog)
       gtk_widget_destroy (gw_applet->gweather_dialog);
}

void gweather_applet_create (GWeatherApplet *gw_applet)
{
    GtkWidget *label;
    GtkWidget *pixmap;
    GtkTooltips *tooltips;
 
    gw_applet->gweather_pref.location = NULL;
    gw_applet->gweather_pref.update_interval = 1800;
    gw_applet->gweather_pref.update_enabled = TRUE;
    gw_applet->gweather_pref.use_metric = FALSE;
    gw_applet->gweather_pref.detailed = FALSE;
    gw_applet->gweather_pref.radar_enabled = TRUE;
    
 
    gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gweather/tstorm.xpm");

    /* PUSH */
    gtk_widget_push_colormap (gdk_rgb_get_cmap ());

    /*gtk_widget_realize(GTK_WIDGET(gw_applet->applet));

    gtk_widget_set_events(GTK_WIDGET(gw_applet->applet), gtk_widget_get_events(GTK_WIDGET(gw_applet->applet)) | \
                          GDK_BUTTON_PRESS_MASK);*/


    g_signal_connect (G_OBJECT(gw_applet->applet), "change_orient",
                       G_CALLBACK(change_orient_cb), gw_applet);
    g_signal_connect (G_OBJECT(gw_applet->applet), "change_size",
                       G_CALLBACK(change_size_cb), gw_applet);
    g_signal_connect (G_OBJECT(gw_applet->applet), "destroy", 
                       G_CALLBACK (applet_destroy), gw_applet);
    gtk_signal_connect (GTK_OBJECT(gw_applet->applet), "button_press_event",
                       GTK_SIGNAL_FUNC(clicked_cb), gw_applet);
                     
    tooltips = gtk_tooltips_new();

    gtk_tooltips_set_tip(tooltips, GTK_WIDGET(gw_applet->applet), _("GNOME Weather"), NULL);

    gw_applet->size = panel_applet_get_size (gw_applet->applet);

    gw_applet->orient = panel_applet_get_orient (gw_applet->applet);
    
    panel_applet_setup_menu_from_file (gw_applet->applet,
                                       NULL,
			               "GNOME_GWeatherApplet.xml",
                                       NULL,
			               weather_applet_menu_verbs,
			               gw_applet);

    gw_applet->tooltips = tooltips;
	
    place_widgets(gw_applet);
    /* POP */
    gtk_widget_pop_colormap ();
    
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
    
/*
    if (info != gweather_info) {
	    if (gweather_info != NULL) {
		    weather_info_free (gweather_info);
		    gweather_info = NULL;
	    }

*/	    /* Save weather info */ /*
	    gweather_info = weather_info_clone (info);
    }
*/
    /* Store current conditions */
   
    gw_applet->gweather_info = info;
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
