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
#include "location.h"


static void
set_all_tooltips (GWeatherApplet *applet, gchar *text)
{
	gint i;

	for (i=0; i<=applet->gweather_info->numforecasts-1; i++) {
		gtk_tooltips_set_tip (applet->tooltips,applet->events[i], text, NULL);
	}
}

void update_display (GWeatherApplet *applet)
{
	WeatherInfo *info = applet->gweather_info;
	GList *list = NULL;
	GdkPixbuf *pixbuf;
	gint i = 1;
	gchar *tmp, *degree;

	/* Translators: This is either C for Celius or F for Farenheit */
	if (applet->gweather_pref.use_metric)
		degree = "\302\260C";
	else
		degree = "\302\260F";
		
	if (info->success) {
		tmp = g_strdup_printf (_("%s\nToday: %s\nCurrent temperature %d%s%s"), 
						  applet->gweather_pref.city, get_conditions (info->wid), 
						  info->curtemp, degree);
		gtk_tooltips_set_tip (applet->tooltips, applet->events[0], tmp, NULL);
		g_free (tmp);
	}
	else
		gtk_tooltips_set_tip (applet->tooltips, applet-> events[0], NULL, NULL);
	pixbuf = get_conditions_pixbuf (info->wid);
	gtk_image_set_from_pixbuf (GTK_IMAGE (applet->images[0]), pixbuf);
	tmp = g_strdup_printf ("%d%s", info->curtemp, degree);
	gtk_label_set_text (GTK_LABEL (applet->labels[0]), tmp);
	g_free (tmp);

	list = info->forecasts;
	for (i=1; i<=applet->gweather_info->numforecasts-1; i++) {
		WeatherForecast *forecast = NULL;
		
		if (!list)
			continue;
		forecast = list->data;	
		if (info->success)	 {	
			tmp = g_strdup_printf (_("%s\n%s: %s\nLow %d%s, High %d%s"),
						  applet->gweather_pref.city,
						  forecast->day, 
						  get_conditions (forecast->wid), forecast->low, degree,
						  forecast->high, degree);
			gtk_tooltips_set_tip (applet->tooltips, applet->events[i], tmp, NULL);
			g_free (tmp);
		}
		else
			gtk_tooltips_set_tip (applet->tooltips,applet->events[i], NULL, NULL);
		pixbuf = get_conditions_pixbuf (forecast->wid);
		gtk_image_set_from_pixbuf (GTK_IMAGE (applet->images[i]), pixbuf);
		tmp = g_strdup_printf ("%d/%d%s", forecast->high, forecast->low, degree);
		gtk_label_set_text (GTK_LABEL (applet->labels[i]), tmp);
		g_free (tmp);
		list = g_list_next (list);
	
	}

	if (!info->success) {
		set_all_tooltips (applet, _("There was an error retrieving the forecast"));
		for (i=0; i<=applet->gweather_info->numforecasts-1; i++) {
			gtk_image_set_from_pixbuf (GTK_IMAGE (applet->images[i]), 
							  get_conditions_pixbuf (-1));
		}
	}
}

void place_widgets (GWeatherApplet *gw_applet)
{
    gint i, max_height = 0, max_width = 0;
    gint numrows_or_columns = 1;
    gboolean horiz;
    gchar*degree;
 
    if (gw_applet->box)
        gtk_widget_destroy (gw_applet->box);
    
    if ((gw_applet->orient == PANEL_APPLET_ORIENT_LEFT) || 
         (gw_applet->orient == PANEL_APPLET_ORIENT_RIGHT)) {
	 horiz = FALSE;
         gw_applet->box = gtk_vbox_new (TRUE, 6);         
    }
    else {
         gw_applet->box = gtk_hbox_new (TRUE, 6);
	 horiz = TRUE;
    }
    
    if (gw_applet->gweather_pref.use_metric)
		degree = "\302\260C";
	else
		degree = "\302\260F";
    
    gtk_container_add (GTK_CONTAINER (gw_applet->applet), gw_applet->box);

    for (i=0; i<=gw_applet->gweather_info->numforecasts-1; i++) {
		gchar *tmp;
		GtkRequisition imagereq, labelreq;
		gw_applet->events[i] = gtk_event_box_new ();
		gtk_box_pack_start (GTK_BOX (gw_applet->box), 
				                      gw_applet->events[i], FALSE, FALSE, 0);
		
		gw_applet->images[i] = gtk_image_new ();		
		gtk_image_set_from_pixbuf (GTK_IMAGE (gw_applet->images[i]),
								     get_conditions_pixbuf (31));
								     
		gtk_widget_size_request (gw_applet->images[i], &imagereq);
		
		gw_applet->labels[i] = gtk_label_new (NULL);
		
		if (i ==0)
			tmp = g_strdup_printf ("%d\302\260%s", 0, degree);
		else
			tmp = g_strdup_printf ("%d/%d%s", 0, 0, degree);
		gtk_label_set_text (GTK_LABEL (gw_applet->labels[i]), tmp);
		gtk_widget_size_request (gw_applet->labels[i], &labelreq);
		
		max_width = MAX (max_width, imagereq.width+labelreq.width);
		max_height = MAX (max_height, imagereq.height+labelreq.height);
		g_free (tmp);
    }
    
    if (horiz) {
    	if (max_height <= gw_applet->size)
    		numrows_or_columns = 2;
    } else {
    	if (max_width <= gw_applet->size)
    		numrows_or_columns = 2;
    }
    
    for (i=0; i<=gw_applet->gweather_info->numforecasts-1; i++) {
     	if (horiz && numrows_or_columns == 2)
     		gw_applet->boxes[i] = gtk_vbox_new (FALSE, 0);
     	else if (horiz && numrows_or_columns == 1)
     		gw_applet->boxes[i] = gtk_hbox_new (FALSE, 0);
     	else if (!horiz && numrows_or_columns == 2)
     		gw_applet->boxes[i] = gtk_hbox_new (FALSE, 0);
     	else
     		gw_applet->boxes[i] = gtk_vbox_new (FALSE, 0);
     		
     	gtk_container_add (GTK_CONTAINER (gw_applet->events[i]),  gw_applet->boxes[i]);
     	gtk_box_pack_start (GTK_BOX (gw_applet->boxes[i]), gw_applet->images[i], TRUE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (gw_applet->boxes[i]), 
				                      gw_applet->labels[i], TRUE, FALSE, 0);
    }
    
    gtk_widget_show_all (GTK_WIDGET (gw_applet->applet));

    if (!gw_applet->gweather_pref.show_labels) {
	for (i=0; i<=gw_applet->gweather_info->numforecasts-1; i++) {
		gtk_widget_hide (gw_applet->labels[i]);
	}
    }
}

static void change_orient_cb (PanelApplet *w, PanelAppletOrient o, gpointer data)
{
    GWeatherApplet *gw_applet = (GWeatherApplet *)data;
	
    gw_applet->orient = o;
    place_widgets(gw_applet);
    update_display (gw_applet);	

}

static void change_size_cb(PanelApplet *w, gint s, gpointer data)
{
    GWeatherApplet *gw_applet = (GWeatherApplet *)data;
	
    gw_applet->size = s;
    place_widgets(gw_applet);
    update_display (gw_applet);

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

    if (gw_applet->gweather_dialog)
       gtk_widget_destroy (gw_applet->gweather_dialog);
}

void gweather_applet_create (GWeatherApplet *gw_applet)
{
   
    gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gweather/tstorm.xpm");
    panel_applet_set_flags (gw_applet->applet, PANEL_APPLET_EXPAND_MINOR);

    g_signal_connect (G_OBJECT(gw_applet->applet), "change_orient",
                       G_CALLBACK(change_orient_cb), gw_applet);
    g_signal_connect (G_OBJECT(gw_applet->applet), "change_size",
                       G_CALLBACK(change_size_cb), gw_applet);
    g_signal_connect (G_OBJECT(gw_applet->applet), "destroy", 
                       G_CALLBACK (applet_destroy), gw_applet);
    g_signal_connect (G_OBJECT(gw_applet->applet), "button_press_event",
                       G_CALLBACK(clicked_cb), gw_applet);
                    
    gw_applet->tooltips = gtk_tooltips_new();

    gw_applet->size = panel_applet_get_size (gw_applet->applet);

    gw_applet->orient = panel_applet_get_orient (gw_applet->applet);
    
    panel_applet_setup_menu_from_file (gw_applet->applet,
                                       NULL,
			               "GNOME_GWeatherApplet.xml",
                                       NULL,
			               weather_applet_menu_verbs,
			               gw_applet);
   
    place_widgets(gw_applet);
    update_display(gw_applet);   

    gw_applet->timeout_tag =  
        	gtk_timeout_add (gw_applet->gweather_pref.update_interval * 1000,
                                 timeout_cb, gw_applet);
    return;
}

gint timeout_cb (gpointer data)
{
    GWeatherApplet *gw_applet = (GWeatherApplet *)data;
	
    gweather_update(gw_applet);

    return TRUE;
}

void gweather_update (GWeatherApplet *gw_applet)
{
    gboolean update_success;
    gint i;

    set_all_tooltips (gw_applet, _("Updating..."));
    
    update_success = update_weather (gw_applet);
    
    return;
}
