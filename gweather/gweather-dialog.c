/* $Id$ */

/*
 *  Papadimitriou Spiros <spapadim+@cs.cmu.edu>
 *
 *  This code released under the GNU GPL.
 *  Read the file COPYING for more information.
 *
 *  Main status dialog
 *
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <gnome.h>

#include "weather.h"
#include "gweather.h"
#include "gweather-applet.h"
#include "gweather-pref.h"
#include "gweather-dialog.h"


static void response_cb (GtkDialog *dialog, gint id, gpointer data)
{
    GWeatherApplet *gw_applet = (GWeatherApplet *)data;

    if (id == GTK_RESPONSE_OK) {
	gweather_update (gw_applet);

	gweather_dialog_update (gw_applet);

	return;
    }
    
    gweather_dialog_close(gw_applet);
    return;
}

static void link_cb (GtkButton *button, gpointer data)
{
    gnome_url_show("http://www.weather.com/", NULL);
    return;
    button = NULL;
    data = NULL;
}

void gweather_dialog_create (GWeatherApplet *gw_applet)
{
  GtkWidget *weather_vbox;
  GtkWidget *weather_notebook;
  GtkWidget *cond_hbox;
  GtkWidget *cond_table;
  GtkWidget *cond_location_lbl;
  GtkWidget *cond_update_lbl;
  GtkWidget *cond_temp_lbl;
  GtkWidget *cond_cond_lbl;
  GtkWidget *cond_sky_lbl;
  GtkWidget *cond_wind_lbl;
  GtkWidget *cond_humidity_lbl;
  GtkWidget *cond_pressure_lbl;
  GtkWidget *cond_vis_lbl;
  GtkWidget *cond_dew_lbl;
  GtkWidget *cond_apparent_lbl;
  GtkWidget *cond_frame_alignment;
  GtkWidget *current_note_lbl;
  GtkWidget *forecast_note_lbl;
  GtkWidget *radar_note_lbl;
  GtkWidget *radar_vbox;
  GtkWidget *radar_link_btn;
  GtkWidget *radar_link_alignment;
  GtkWidget *forecast_hbox;
  GtkWidget *ebox;
  GtkWidget *scrolled_window;
  GtkWidget *imagescroll_window;

  gw_applet->gweather_dialog = gtk_dialog_new_with_buttons (_("Details"), NULL,
						  GTK_DIALOG_DESTROY_WITH_PARENT,
						  _("_Update"),
						  GTK_RESPONSE_OK,
						  GTK_STOCK_CLOSE,
						  GTK_RESPONSE_CLOSE,
						  NULL);

  gtk_dialog_set_default_response (GTK_DIALOG (gw_applet->gweather_dialog), 
  				   GTK_RESPONSE_CLOSE);

  if (gw_applet->gweather_pref.radar_enabled)
      gtk_window_set_default_size (GTK_WINDOW(gw_applet->gweather_dialog), 570,440);
  else
      gtk_window_set_default_size (GTK_WINDOW(gw_applet->gweather_dialog), 590, 340);

  gtk_window_set_screen (GTK_WINDOW (gw_applet->gweather_dialog),
			 gtk_widget_get_screen (GTK_WIDGET (gw_applet->applet)));
  gtk_window_set_policy (GTK_WINDOW (gw_applet->gweather_dialog), FALSE, FALSE, FALSE);
  
  weather_vbox = GTK_DIALOG (gw_applet->gweather_dialog)->vbox;
  gtk_widget_show (weather_vbox);

  weather_notebook = gtk_notebook_new ();
  gtk_container_set_border_width (GTK_CONTAINER (weather_notebook), 5);
  gtk_widget_show (weather_notebook);
  gtk_box_pack_start (GTK_BOX (weather_vbox), weather_notebook, TRUE, TRUE, 0);

  cond_hbox = gtk_hbox_new (FALSE, 2);
  gtk_widget_show (cond_hbox);
  gtk_container_add (GTK_CONTAINER (weather_notebook), cond_hbox);
  gtk_container_set_border_width (GTK_CONTAINER (cond_hbox), 4);

  cond_table = gtk_table_new (11, 2, FALSE);
  gtk_widget_show (cond_table);
  gtk_box_pack_start (GTK_BOX (cond_hbox), cond_table, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (cond_table), 12);
  gtk_table_set_row_spacings (GTK_TABLE (cond_table), 6);
  gtk_table_set_col_spacings (GTK_TABLE (cond_table), 12);

  cond_location_lbl = gtk_label_new (_("City:"));
  gtk_widget_show (cond_location_lbl);
  gtk_table_attach (GTK_TABLE (cond_table), cond_location_lbl, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (cond_location_lbl), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (cond_location_lbl), 0, 0.5);

  cond_update_lbl = gtk_label_new (_("Last update:"));
  gtk_widget_show (cond_update_lbl);
  gtk_table_attach (GTK_TABLE (cond_table), cond_update_lbl, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (cond_update_lbl), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (cond_update_lbl), 0, 0.5);

  cond_cond_lbl = gtk_label_new (_("Conditions:"));
  gtk_widget_show (cond_cond_lbl);
  gtk_table_attach (GTK_TABLE (cond_table), cond_cond_lbl, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (cond_cond_lbl), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (cond_cond_lbl), 0, 0.5);

  cond_sky_lbl = gtk_label_new (_("Sky:"));
  gtk_widget_show (cond_sky_lbl);
  gtk_table_attach (GTK_TABLE (cond_table), cond_sky_lbl, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (cond_sky_lbl), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (cond_sky_lbl), 0, 0.5);

  cond_temp_lbl = gtk_label_new (_("Temperature:"));
  gtk_widget_show (cond_temp_lbl);
  gtk_table_attach (GTK_TABLE (cond_table), cond_temp_lbl, 0, 1, 4, 5,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (cond_temp_lbl), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (cond_temp_lbl), 0, 0.5);

  cond_apparent_lbl = gtk_label_new (_("Feels like:"));
  gtk_widget_show (cond_apparent_lbl);
  gtk_table_attach (GTK_TABLE (cond_table), cond_apparent_lbl, 0, 1, 5, 6,
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (cond_apparent_lbl), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (cond_apparent_lbl), 0, 0.5);
  
  cond_dew_lbl = gtk_label_new (_("Dew point:"));
  gtk_widget_show (cond_dew_lbl);
  gtk_table_attach (GTK_TABLE (cond_table), cond_dew_lbl, 0, 1, 6, 7,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (cond_dew_lbl), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (cond_dew_lbl), 0, 0.5);

  cond_humidity_lbl = gtk_label_new (_("Relative humidity:"));
  gtk_widget_show (cond_humidity_lbl);
  gtk_table_attach (GTK_TABLE (cond_table), cond_humidity_lbl, 0, 1, 7, 8,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (cond_humidity_lbl), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (cond_humidity_lbl), 0, 0.5);

  cond_wind_lbl = gtk_label_new (_("Wind:"));
  gtk_widget_show (cond_wind_lbl);
  gtk_table_attach (GTK_TABLE (cond_table), cond_wind_lbl, 0, 1, 8, 9,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (cond_wind_lbl), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (cond_wind_lbl), 0, 0.5);

  cond_pressure_lbl = gtk_label_new (_("Pressure:"));
  gtk_widget_show (cond_pressure_lbl);
  gtk_table_attach (GTK_TABLE (cond_table), cond_pressure_lbl, 0, 1, 9, 10,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (cond_pressure_lbl), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (cond_pressure_lbl), 0, 0.5);

  cond_vis_lbl = gtk_label_new (_("Visibility:"));
  gtk_widget_show (cond_vis_lbl);
  gtk_table_attach (GTK_TABLE (cond_table), cond_vis_lbl, 0, 1, 10, 11,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (cond_vis_lbl), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (cond_vis_lbl), 0, 0.5);

  gw_applet->cond_location = gtk_label_new ("");
  gtk_widget_show (gw_applet->cond_location);
  gtk_table_attach (GTK_TABLE (cond_table), gw_applet->cond_location, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_selectable (GTK_LABEL (gw_applet->cond_location), TRUE);
  gtk_label_set_justify (GTK_LABEL (gw_applet->cond_location), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (gw_applet->cond_location), 0, 0.5);

  gw_applet->cond_update = gtk_label_new ("");
  gtk_widget_show (gw_applet->cond_update);
  gtk_table_attach (GTK_TABLE (cond_table), gw_applet->cond_update, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_selectable (GTK_LABEL (gw_applet->cond_update), TRUE);
  gtk_label_set_justify (GTK_LABEL (gw_applet->cond_update), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (gw_applet->cond_update), 0, 0.5);

  gw_applet->cond_cond = gtk_label_new ("");
  gtk_widget_show (gw_applet->cond_cond);
  gtk_table_attach (GTK_TABLE (cond_table), gw_applet->cond_cond, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_selectable (GTK_LABEL (gw_applet->cond_cond), TRUE);
  gtk_label_set_justify (GTK_LABEL (gw_applet->cond_cond), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (gw_applet->cond_cond), 0, 0.5);

  gw_applet->cond_sky = gtk_label_new ("");
  gtk_widget_show (gw_applet->cond_sky);
  gtk_table_attach (GTK_TABLE (cond_table), gw_applet->cond_sky, 1, 2, 3, 4,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_selectable (GTK_LABEL (gw_applet->cond_sky), TRUE);
  gtk_label_set_justify (GTK_LABEL (gw_applet->cond_sky), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (gw_applet->cond_sky), 0, 0.5);

  gw_applet->cond_temp = gtk_label_new ("");
  gtk_widget_show (gw_applet->cond_temp);
  gtk_table_attach (GTK_TABLE (cond_table), gw_applet->cond_temp, 1, 2, 4, 5,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_selectable (GTK_LABEL (gw_applet->cond_temp), TRUE);
  gtk_label_set_justify (GTK_LABEL (gw_applet->cond_temp), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (gw_applet->cond_temp), 0, 0.5);

  gw_applet->cond_apparent = gtk_label_new ("");
  gtk_widget_show (gw_applet->cond_apparent);
  gtk_table_attach (GTK_TABLE (cond_table), gw_applet->cond_apparent, 1, 2, 5, 6,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_selectable (GTK_LABEL (gw_applet->cond_apparent), TRUE);
  gtk_label_set_justify (GTK_LABEL (gw_applet->cond_apparent), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (gw_applet->cond_apparent), 0, 0.5);

  gw_applet->cond_dew = gtk_label_new ("");
  gtk_widget_show (gw_applet->cond_dew);
  gtk_table_attach (GTK_TABLE (cond_table), gw_applet->cond_dew, 1, 2, 6, 7,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_selectable (GTK_LABEL (gw_applet->cond_dew), TRUE);
  gtk_label_set_justify (GTK_LABEL (gw_applet->cond_dew), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (gw_applet->cond_dew), 0, 0.5);

  gw_applet->cond_humidity = gtk_label_new ("");
  gtk_widget_show (gw_applet->cond_humidity);
  gtk_table_attach (GTK_TABLE (cond_table), gw_applet->cond_humidity, 1, 2, 7, 8,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_selectable (GTK_LABEL (gw_applet->cond_humidity), TRUE);
  gtk_label_set_justify (GTK_LABEL (gw_applet->cond_humidity), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (gw_applet->cond_humidity), 0, 0.5);

  gw_applet->cond_wind = gtk_label_new ("");
  gtk_widget_show (gw_applet->cond_wind);
  gtk_table_attach (GTK_TABLE (cond_table), gw_applet->cond_wind, 1, 2, 8, 9,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_selectable (GTK_LABEL (gw_applet->cond_wind), TRUE);
  gtk_label_set_justify (GTK_LABEL (gw_applet->cond_wind), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (gw_applet->cond_wind), 0, 0.5);

  gw_applet->cond_pressure = gtk_label_new ("");
  gtk_widget_show (gw_applet->cond_pressure);
  gtk_table_attach (GTK_TABLE (cond_table), gw_applet->cond_pressure, 1, 2, 9, 10,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_selectable (GTK_LABEL (gw_applet->cond_pressure), TRUE);
  gtk_label_set_justify (GTK_LABEL (gw_applet->cond_pressure), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (gw_applet->cond_pressure), 0, 0.5);

  gw_applet->cond_vis = gtk_label_new ("");
  gtk_widget_show (gw_applet->cond_vis);
  gtk_table_attach (GTK_TABLE (cond_table), gw_applet->cond_vis, 1, 2, 10, 11,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_selectable (GTK_LABEL (gw_applet->cond_vis), TRUE);
  gtk_label_set_justify (GTK_LABEL (gw_applet->cond_vis), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (gw_applet->cond_vis), 0, 0.5);

  cond_frame_alignment = gtk_alignment_new (0.5, 0, 1, 0);
  gtk_widget_show (cond_frame_alignment);
  gtk_box_pack_end (GTK_BOX (cond_hbox), cond_frame_alignment, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (cond_frame_alignment), 2);

  weather_info_get_pixbuf (NULL, &(gw_applet->dialog_pixbuf));
  gw_applet->cond_image = gtk_image_new_from_pixbuf (gw_applet->dialog_pixbuf);
  gtk_widget_show (gw_applet->cond_image);
  gtk_container_add (GTK_CONTAINER (cond_frame_alignment), gw_applet->cond_image);

  current_note_lbl = gtk_label_new (_("Current Conditions"));
  gtk_widget_show (current_note_lbl);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (weather_notebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (weather_notebook), 0), current_note_lbl);

  if (gw_applet->gweather_pref.location->zone_valid) {

      forecast_hbox = gtk_hbox_new(FALSE, 0);
      gtk_container_set_border_width (GTK_CONTAINER (forecast_hbox), 12);
      gtk_widget_show (forecast_hbox);

      scrolled_window = gtk_scrolled_window_new (NULL, NULL);
      gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                      GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
      gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window),
                                           GTK_SHADOW_ETCHED_IN);

      gw_applet->forecast_text = gtk_text_view_new ();
      set_access_namedesc (gw_applet->forecast_text, _("Forecast Report"), _("See the ForeCast Details"));
      gtk_container_add (GTK_CONTAINER (scrolled_window), gw_applet->forecast_text);
      gtk_text_view_set_editable (GTK_TEXT_VIEW (gw_applet->forecast_text), FALSE);
      gtk_text_view_set_left_margin (GTK_TEXT_VIEW (gw_applet->forecast_text), 6);
      gtk_widget_show (gw_applet->forecast_text);
      gtk_widget_show (scrolled_window);
      gtk_box_pack_start (GTK_BOX (forecast_hbox), scrolled_window, TRUE, TRUE, 0);

      gtk_container_add (GTK_CONTAINER (weather_notebook), forecast_hbox);

      forecast_note_lbl = gtk_label_new (_("Forecast"));
      gtk_widget_show (forecast_note_lbl);
      gtk_notebook_set_tab_label (GTK_NOTEBOOK (weather_notebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (weather_notebook), 1), forecast_note_lbl);

  }

  if (gw_applet->gweather_pref.radar_enabled) {

      radar_note_lbl = gtk_label_new_with_mnemonic (_("Radar Map"));
      gtk_widget_show (radar_note_lbl);

      radar_vbox = gtk_vbox_new (FALSE, 6);
      gtk_widget_show (radar_vbox);
      gtk_notebook_append_page (GTK_NOTEBOOK (weather_notebook), radar_vbox, radar_note_lbl);
      gtk_container_set_border_width (GTK_CONTAINER (radar_vbox), 6);

      gw_applet->radar_image = gtk_image_new_from_pixbuf (gw_applet->dialog_pixbuf);  /* Tmp hack */
      
      imagescroll_window = gtk_scrolled_window_new (NULL, NULL);
      gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (imagescroll_window),
                                 GTK_POLICY_AUTOMATIC,
                                 GTK_POLICY_AUTOMATIC);
      gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (imagescroll_window),
                                      GTK_SHADOW_ETCHED_IN);
      
      ebox = gtk_event_box_new ();
      gtk_widget_show (ebox);

      gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(imagescroll_window),ebox);
      gtk_box_pack_start (GTK_BOX (radar_vbox), imagescroll_window, TRUE, TRUE, 0);
      gtk_widget_show (gw_applet->radar_image);
      gtk_widget_show (imagescroll_window);
      
      gtk_container_add (GTK_CONTAINER (ebox), gw_applet->radar_image);

      radar_link_alignment = gtk_alignment_new (0.5, 0.5, 0, 0);
      gtk_widget_show (radar_link_alignment);
      gtk_box_pack_start (GTK_BOX (radar_vbox), radar_link_alignment, FALSE, FALSE, 0);

      radar_link_btn = gtk_button_new_with_mnemonic (_("_Visit Weather.com"));
      set_access_namedesc (radar_link_btn, _("Visit Weather.com"), _("Click to Enter Weather.com"));
      gtk_widget_set_usize(radar_link_btn, 450, -2);
      gtk_widget_show (radar_link_btn);
      gtk_container_add (GTK_CONTAINER (radar_link_alignment), radar_link_btn);

      gtk_signal_connect (GTK_OBJECT (radar_link_btn), "clicked",
                          GTK_SIGNAL_FUNC (link_cb), NULL);

  }

  g_signal_connect (G_OBJECT (gw_applet->gweather_dialog), "response",
  		    G_CALLBACK (response_cb), gw_applet);
  
}

void gweather_dialog_open (GWeatherApplet *gw_applet)
{
    if (gw_applet->gweather_dialog == NULL)
        gweather_dialog_create(gw_applet);
    else {
	gtk_window_set_screen (GTK_WINDOW (gw_applet->gweather_dialog),
		gtk_widget_get_screen (GTK_WIDGET (gw_applet->applet)));
	
	gtk_window_present (GTK_WINDOW (gw_applet->gweather_dialog));
    }
    
    gweather_dialog_update(gw_applet);
    gtk_dialog_set_has_separator (GTK_DIALOG (gw_applet->gweather_dialog), FALSE);
    gtk_container_set_border_width (GTK_CONTAINER (gw_applet->gweather_dialog), 5);
    gtk_box_set_spacing (GTK_BOX (GTK_DIALOG (gw_applet->gweather_dialog)->vbox), 2);
    gtk_window_set_resizable(GTK_WINDOW(gw_applet->gweather_dialog), TRUE);
    gtk_widget_show(gw_applet->gweather_dialog);
}

void gweather_dialog_close (GWeatherApplet *gw_applet)
{
    g_return_if_fail(gw_applet->gweather_dialog != NULL);
    gtk_widget_destroy(gw_applet->gweather_dialog);
    gw_applet->gweather_dialog = NULL;
    gw_applet->dialog_mask = NULL;
}

void gweather_dialog_display_toggle (GWeatherApplet *gw_applet)
{
    if (!gw_applet->gweather_dialog || !GTK_WIDGET_VISIBLE(gw_applet->gweather_dialog))
        gweather_dialog_open(gw_applet);
    else
        gweather_dialog_close(gw_applet);
}

void gweather_dialog_update (GWeatherApplet *gw_applet)
{
    const gchar *forecast;
    GtkTextBuffer *buffer;
    /*GdkFont* detailed_forecast_font = gdk_fontset_load ( "fixed" );*/

    /* Check for parallel network update in progress */
    if(gw_applet->gweather_info == NULL)
    	return;

    if (!gw_applet->gweather_dialog)
        return;

    /* Update pixbuf */
    weather_info_get_pixbuf(gw_applet->gweather_info, 
                            &(gw_applet->dialog_pixbuf));
    gtk_image_set_from_pixbuf (GTK_IMAGE (gw_applet->cond_image), 
                               gw_applet->dialog_pixbuf);

    /* Update current condition fields and forecast */
    gtk_label_set_text(GTK_LABEL(gw_applet->cond_location), weather_info_get_location(gw_applet->gweather_info));
    gtk_label_set_text(GTK_LABEL(gw_applet->cond_update), weather_info_get_update(gw_applet->gweather_info));
    gtk_label_set_text(GTK_LABEL(gw_applet->cond_cond), weather_info_get_conditions(gw_applet->gweather_info));
    gtk_label_set_text(GTK_LABEL(gw_applet->cond_sky), weather_info_get_sky(gw_applet->gweather_info));
    gtk_label_set_text(GTK_LABEL(gw_applet->cond_temp), weather_info_get_temp(gw_applet->gweather_info));
    gtk_label_set_text(GTK_LABEL(gw_applet->cond_apparent), weather_info_get_apparent(gw_applet->gweather_info));
    gtk_label_set_text(GTK_LABEL(gw_applet->cond_dew), weather_info_get_dew(gw_applet->gweather_info));
    gtk_label_set_text(GTK_LABEL(gw_applet->cond_humidity), weather_info_get_humidity(gw_applet->gweather_info));
    gtk_label_set_text(GTK_LABEL(gw_applet->cond_wind), weather_info_get_wind(gw_applet->gweather_info));
    gtk_label_set_text(GTK_LABEL(gw_applet->cond_pressure), weather_info_get_pressure(gw_applet->gweather_info));
    gtk_label_set_text(GTK_LABEL(gw_applet->cond_vis), weather_info_get_visibility(gw_applet->gweather_info));

    /* Update forecast */
    if (gw_applet->gweather_pref.location->zone_valid) {
        buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (gw_applet->forecast_text));
        forecast = weather_info_get_forecast(gw_applet->gweather_info);
        if (forecast) {
            gtk_text_buffer_set_text(buffer, forecast, -1);
        } else {
            gtk_text_buffer_set_text(buffer, _("Forecast not currently available for this location."), -1);
        }
    }

    /* Update radar map */
    if (gw_applet->gweather_pref.radar_enabled) {
        GdkPixmap *radar = weather_info_get_radar(gw_applet->gweather_info);
        if (radar) {
            gtk_image_set_from_pixmap (GTK_IMAGE (gw_applet->radar_image), 
                                       radar, gw_applet->gweather_info->radar_mask);
        }
    }
}
