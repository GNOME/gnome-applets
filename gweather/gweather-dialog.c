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
#include "gweather-applet.h"
#include "gweather-pref.h"
#include "gweather-dialog.h"

static GtkWidget *gweather_dialog = NULL;

static GtkWidget *cond_location;
static GtkWidget *cond_update;
static GtkWidget *cond_cond;
static GtkWidget *cond_sky;
static GtkWidget *cond_temp;
static GtkWidget *cond_dew;
static GtkWidget *cond_humidity;
static GtkWidget *cond_wind;
static GtkWidget *cond_pressure;
static GtkWidget *cond_vis;
static GtkWidget *cond_pixmap;
static GtkWidget *forecast_text;
static GtkWidget *radar_pixmap;

static GdkPixmap *pixmap;
static GdkBitmap *mask = NULL;


static void close_cb (GtkButton *button, gpointer data)
{
    gweather_dialog_close();
    return;
    button = NULL;
    data = NULL;
}

static void link_cb (GtkButton *button, gpointer data)
{
    gnome_url_show("http://www.weather.com/");
    return;
    button = NULL;
    data = NULL;
}

void gweather_dialog_create (void)
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
  GtkWidget *cond_frame_alignment;
  GtkWidget *cond_vsep;
  GtkWidget *current_note_lbl;
  GtkWidget *forecast_note_lbl;
  GtkWidget *radar_note_lbl;
  GtkWidget *radar_vbox;
  GtkWidget *radar_link_btn;
  GtkWidget *radar_link_alignment;
  GtkWidget *forecast_hbox;
  GtkWidget *weather_action_area;
  GtkWidget *close_button;
  GtkWidget *ebox;
  GtkWidget *scrolled_window;

  g_return_if_fail(gweather_dialog == NULL);

  gweather_dialog = gnome_dialog_new (_("GNOME Weather"), NULL);

  if (gweather_pref.radar_enabled)
      gtk_widget_set_usize (gweather_dialog, 570, 440);
  else
      gtk_widget_set_usize (gweather_dialog, 590, 340);
  gtk_window_set_policy (GTK_WINDOW (gweather_dialog), FALSE, FALSE, FALSE);
  gnome_dialog_close_hides(GNOME_DIALOG(gweather_dialog), TRUE);

  weather_vbox = GNOME_DIALOG (gweather_dialog)->vbox;
  gtk_widget_show (weather_vbox);

  weather_notebook = gtk_notebook_new ();
  gtk_widget_show (weather_notebook);
  gtk_box_pack_start (GTK_BOX (weather_vbox), weather_notebook, TRUE, TRUE, 0);

  cond_hbox = gtk_hbox_new (FALSE, 2);
  gtk_widget_show (cond_hbox);
  gtk_container_add (GTK_CONTAINER (weather_notebook), cond_hbox);
  gtk_container_set_border_width (GTK_CONTAINER (cond_hbox), 4);

  cond_table = gtk_table_new (10, 2, FALSE);
  gtk_widget_show (cond_table);
  gtk_box_pack_start (GTK_BOX (cond_hbox), cond_table, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (cond_table), 2);
  gtk_table_set_row_spacings (GTK_TABLE (cond_table), 4);
  gtk_table_set_col_spacings (GTK_TABLE (cond_table), 4);

  cond_location_lbl = gtk_label_new (_("Location:"));
  gtk_widget_show (cond_location_lbl);
  gtk_table_attach (GTK_TABLE (cond_table), cond_location_lbl, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (cond_location_lbl), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (cond_location_lbl), 1, 0.5);

  cond_update_lbl = gtk_label_new (_("Update:"));
  gtk_widget_show (cond_update_lbl);
  gtk_table_attach (GTK_TABLE (cond_table), cond_update_lbl, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (cond_update_lbl), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (cond_update_lbl), 1, 0.5);

  cond_cond_lbl = gtk_label_new (_("Conditions:"));
  gtk_widget_show (cond_cond_lbl);
  gtk_table_attach (GTK_TABLE (cond_table), cond_cond_lbl, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (cond_cond_lbl), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (cond_cond_lbl), 1, 0.5);

  cond_sky_lbl = gtk_label_new (_("Sky:"));
  gtk_widget_show (cond_sky_lbl);
  gtk_table_attach (GTK_TABLE (cond_table), cond_sky_lbl, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (cond_sky_lbl), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (cond_sky_lbl), 1, 0.5);

  cond_temp_lbl = gtk_label_new (_("Temperature:"));
  gtk_widget_show (cond_temp_lbl);
  gtk_table_attach (GTK_TABLE (cond_table), cond_temp_lbl, 0, 1, 4, 5,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (cond_temp_lbl), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (cond_temp_lbl), 1, 0.5);

  cond_dew_lbl = gtk_label_new (_("Dew point:"));
  gtk_widget_show (cond_dew_lbl);
  gtk_table_attach (GTK_TABLE (cond_table), cond_dew_lbl, 0, 1, 5, 6,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (cond_dew_lbl), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (cond_dew_lbl), 1, 0.5);

  cond_humidity_lbl = gtk_label_new (_("Humidity:"));
  gtk_widget_show (cond_humidity_lbl);
  gtk_table_attach (GTK_TABLE (cond_table), cond_humidity_lbl, 0, 1, 6, 7,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (cond_humidity_lbl), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (cond_humidity_lbl), 1, 0.5);

  cond_wind_lbl = gtk_label_new (_("Wind:"));
  gtk_widget_show (cond_wind_lbl);
  gtk_table_attach (GTK_TABLE (cond_table), cond_wind_lbl, 0, 1, 7, 8,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (cond_wind_lbl), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (cond_wind_lbl), 1, 0.5);

  cond_pressure_lbl = gtk_label_new (_("Pressure:"));
  gtk_widget_show (cond_pressure_lbl);
  gtk_table_attach (GTK_TABLE (cond_table), cond_pressure_lbl, 0, 1, 8, 9,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (cond_pressure_lbl), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (cond_pressure_lbl), 1, 0.5);

  cond_vis_lbl = gtk_label_new (_("Visibility:"));
  gtk_widget_show (cond_vis_lbl);
  gtk_table_attach (GTK_TABLE (cond_table), cond_vis_lbl, 0, 1, 9, 10,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (cond_vis_lbl), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (cond_vis_lbl), 1, 0.5);

  cond_location = gtk_label_new ("");
  gtk_widget_show (cond_location);
  gtk_table_attach (GTK_TABLE (cond_table), cond_location, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (cond_location), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (cond_location), 0, 0.5);

  cond_update = gtk_label_new ("");
  gtk_widget_show (cond_update);
  gtk_table_attach (GTK_TABLE (cond_table), cond_update, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (cond_update), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (cond_update), 0, 0.5);

  cond_cond = gtk_label_new ("");
  gtk_widget_show (cond_cond);
  gtk_table_attach (GTK_TABLE (cond_table), cond_cond, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (cond_cond), 0, 0.5);

  cond_sky = gtk_label_new ("");
  gtk_widget_show (cond_sky);
  gtk_table_attach (GTK_TABLE (cond_table), cond_sky, 1, 2, 3, 4,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (cond_sky), 0, 0.5);

  cond_temp = gtk_label_new ("");
  gtk_widget_show (cond_temp);
  gtk_table_attach (GTK_TABLE (cond_table), cond_temp, 1, 2, 4, 5,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (cond_temp), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (cond_temp), 0, 0.5);

  cond_dew = gtk_label_new ("");
  gtk_widget_show (cond_dew);
  gtk_table_attach (GTK_TABLE (cond_table), cond_dew, 1, 2, 5, 6,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (cond_dew), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (cond_dew), 0, 0.5);

  cond_humidity = gtk_label_new ("");
  gtk_widget_show (cond_humidity);
  gtk_table_attach (GTK_TABLE (cond_table), cond_humidity, 1, 2, 6, 7,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (cond_humidity), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (cond_humidity), 0, 0.5);

  cond_wind = gtk_label_new ("");
  gtk_widget_show (cond_wind);
  gtk_table_attach (GTK_TABLE (cond_table), cond_wind, 1, 2, 7, 8,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (cond_wind), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (cond_wind), 0, 0.5);

  cond_pressure = gtk_label_new ("");
  gtk_widget_show (cond_pressure);
  gtk_table_attach (GTK_TABLE (cond_table), cond_pressure, 1, 2, 8, 9,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (cond_pressure), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (cond_pressure), 0, 0.5);

  cond_vis = gtk_label_new ("");
  gtk_widget_show (cond_vis);
  gtk_table_attach (GTK_TABLE (cond_table), cond_vis, 1, 2, 9, 10,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (cond_vis), 0, 0.5);

  cond_frame_alignment = gtk_alignment_new (0.5, 0, 1, 0);
  gtk_widget_show (cond_frame_alignment);
  gtk_box_pack_end (GTK_BOX (cond_hbox), cond_frame_alignment, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (cond_frame_alignment), 2);

  weather_info_get_pixmap (NULL, &pixmap, &mask);
  cond_pixmap = gtk_pixmap_new (pixmap, mask);
  gtk_widget_show (cond_pixmap);
  gtk_container_add (GTK_CONTAINER (cond_frame_alignment), cond_pixmap);

  cond_vsep = gtk_vseparator_new ();
  gtk_widget_show (cond_vsep);
  gtk_box_pack_end (GTK_BOX (cond_hbox), cond_vsep, FALSE, FALSE, 0);

  current_note_lbl = gtk_label_new (_("Current conditions"));
  gtk_widget_show (current_note_lbl);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (weather_notebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (weather_notebook), 0), current_note_lbl);

  forecast_hbox = gtk_hbox_new(FALSE, 0);
  gtk_widget_show (forecast_hbox);

  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_AUTOMATIC);

  forecast_text = gtk_text_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (scrolled_window), forecast_text);
  gtk_widget_show (forecast_text);
  gtk_widget_show (scrolled_window);
  gtk_box_pack_start (GTK_BOX (forecast_hbox), scrolled_window, TRUE, TRUE, 0);

  gtk_container_add (GTK_CONTAINER (weather_notebook), forecast_hbox);

  forecast_note_lbl = gtk_label_new (_("Forecast"));
  gtk_widget_show (forecast_note_lbl);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (weather_notebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (weather_notebook), 1), forecast_note_lbl);

  if (gweather_pref.radar_enabled) {
      radar_vbox = gtk_vbox_new (FALSE, 6);
      gtk_widget_show (radar_vbox);
      gtk_container_add (GTK_CONTAINER (weather_notebook), radar_vbox);
      gtk_container_set_border_width (GTK_CONTAINER (radar_vbox), 6);


      /* PUSH */
      gtk_widget_push_visual (gdk_rgb_get_visual ());
      gtk_widget_push_colormap (gdk_rgb_get_cmap ());

      ebox = gtk_event_box_new ();
      gtk_widget_show (ebox);
      gtk_box_pack_start (GTK_BOX (radar_vbox), ebox, FALSE, FALSE, 0);


      radar_pixmap = gtk_pixmap_new (pixmap, mask);  /* Tmp hack */
      gtk_widget_show (radar_pixmap);
      gtk_container_add (GTK_CONTAINER (ebox), radar_pixmap);

      /* POP */
      gtk_widget_pop_colormap ();
      gtk_widget_pop_visual ();


      radar_link_alignment = gtk_alignment_new (0.5, 0.5, 0, 0);
      gtk_widget_show (radar_link_alignment);
      gtk_box_pack_start (GTK_BOX (radar_vbox), radar_link_alignment, FALSE, FALSE, 0);

      radar_link_btn = gtk_button_new_with_label (_("Visit Weather.com"));
      gtk_widget_set_usize(radar_link_btn, 450, -2);
      gtk_widget_show (radar_link_btn);
      gtk_container_add (GTK_CONTAINER (radar_link_alignment), radar_link_btn);

      gtk_signal_connect (GTK_OBJECT (radar_link_btn), "clicked",
                          GTK_SIGNAL_FUNC (link_cb), NULL);

      radar_note_lbl = gtk_label_new (_("Radar map"));
      gtk_widget_show (radar_note_lbl);
      gtk_notebook_set_tab_label (GTK_NOTEBOOK (weather_notebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (weather_notebook), 2), radar_note_lbl);
  }

  weather_action_area = GNOME_DIALOG (gweather_dialog)->action_area;
  gtk_widget_show (weather_action_area);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (weather_action_area), GTK_BUTTONBOX_END);
  gtk_button_box_set_spacing (GTK_BUTTON_BOX (weather_action_area), 8);

  gnome_dialog_append_button (GNOME_DIALOG (gweather_dialog), GNOME_STOCK_BUTTON_CLOSE);
  close_button = g_list_last (GNOME_DIALOG (gweather_dialog)->buttons)->data;
  gtk_widget_show (close_button);
  GTK_WIDGET_SET_FLAGS (close_button, GTK_CAN_DEFAULT);

  gtk_signal_connect (GTK_OBJECT (close_button), "clicked",
                      GTK_SIGNAL_FUNC (close_cb), NULL);
}

void gweather_dialog_open (void)
{
    if (!gweather_dialog)
        gweather_dialog_create();
    gweather_dialog_update();
    gtk_widget_show(gweather_dialog);
}

void gweather_dialog_close (void)
{
    g_return_if_fail(gweather_dialog != NULL);
    gtk_widget_hide(gweather_dialog);
    gtk_widget_destroy(gweather_dialog);
    gweather_dialog = NULL;
    mask = NULL;
}

void gweather_dialog_display_toggle (void)
{
    if (!gweather_dialog || !GTK_WIDGET_VISIBLE(gweather_dialog))
        gweather_dialog_open();
    else
        gweather_dialog_close();
}

void gweather_dialog_update (void)
{
    const gchar *forecast;
    GdkFont* detailed_forecast_font = gdk_fontset_load ( "fixed" );

    /* Check for parallel network update in progress */
    if(gweather_info == NULL)
    	return;

    if (!gweather_dialog)
        return;

    /* Update pixmap */
    weather_info_get_pixmap(gweather_info, &pixmap, &mask);
    gtk_pixmap_set(GTK_PIXMAP(cond_pixmap), pixmap, mask);

    /* Update current condition fields and forecast */
    gtk_label_set_text(GTK_LABEL(cond_location), weather_info_get_location(gweather_info));
    gtk_label_set_text(GTK_LABEL(cond_update), weather_info_get_update(gweather_info));
    gtk_label_set_text(GTK_LABEL(cond_cond), weather_info_get_conditions(gweather_info));
    gtk_label_set_text(GTK_LABEL(cond_sky), weather_info_get_sky(gweather_info));
    gtk_label_set_text(GTK_LABEL(cond_temp), weather_info_get_temp(gweather_info));
    gtk_label_set_text(GTK_LABEL(cond_dew), weather_info_get_dew(gweather_info));
    gtk_label_set_text(GTK_LABEL(cond_humidity), weather_info_get_humidity(gweather_info));
    gtk_label_set_text(GTK_LABEL(cond_wind), weather_info_get_wind(gweather_info));
    gtk_label_set_text(GTK_LABEL(cond_pressure), weather_info_get_pressure(gweather_info));
    gtk_label_set_text(GTK_LABEL(cond_vis), weather_info_get_visibility(gweather_info));

    /* Update forecast */
    gtk_text_set_point(GTK_TEXT(forecast_text), 0);
    gtk_text_forward_delete(GTK_TEXT(forecast_text), gtk_text_get_length(GTK_TEXT(forecast_text)));
    forecast = weather_info_get_forecast(gweather_info);
    if (forecast) {
        gtk_text_insert(GTK_TEXT(forecast_text), detailed_forecast_font, NULL, NULL, forecast, strlen(forecast));
    } else {
        if (gweather_pref.detailed)
            gtk_text_insert(GTK_TEXT(forecast_text), detailed_forecast_font, NULL, NULL, _("Detailed forecast not available for this location.\nPlease try the state forecast; note that IWIN forecasts are available only for US cities."), -1);
        else
            gtk_text_insert(GTK_TEXT(forecast_text), detailed_forecast_font, NULL, NULL, _("State forecast not available for this location.\nPlease try the detailed forecast; note that IWIN forecasts are available only for US cities."), -1);
    }

    /* Update radar map */
    if (gweather_pref.radar_enabled) {
        GdkPixmap *radar = weather_info_get_radar(gweather_info);
        if (radar) {
            gtk_pixmap_set (GTK_PIXMAP(radar_pixmap), radar, NULL);
        }
    }
}

