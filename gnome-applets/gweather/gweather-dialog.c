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

#include "config.h"

#include "gweather-dialog.h"
#include "gweather-pref.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define MONOSPACE_GSCHEMA "org.gnome.desktop.interface"
#define MONOSPACE_FONT_KEY "monospace-font-name"

struct _GWeatherDialog
{
  GtkDialog       parent;

  GWeatherApplet *applet;

  GtkWidget      *cond_location;
  GtkWidget      *cond_update;
  GtkWidget      *cond_cond;
  GtkWidget      *cond_sky;
  GtkWidget      *cond_temp;
  GtkWidget      *cond_dew;
  GtkWidget      *cond_humidity;
  GtkWidget      *cond_wind;
  GtkWidget      *cond_pressure;
  GtkWidget      *cond_vis;
  GtkWidget      *cond_apparent;
  GtkWidget      *cond_sunrise;
  GtkWidget      *cond_sunset;
  GtkWidget      *cond_image;

  GtkWidget      *forecast_text;
  GSettings      *monospace_settings;
  GtkCssProvider *css_provider;
};

enum
{
  PROP_0,

  PROP_GWEATHER_APPLET,

  PROP_LAST
};

static GParamSpec *dialog_properties[PROP_LAST] = { NULL };

G_DEFINE_TYPE (GWeatherDialog, gweather_dialog, GTK_TYPE_DIALOG)

static void
response_cb (GWeatherDialog *dialog,
             gint id,
             gpointer data)
{
    if (id == GTK_RESPONSE_OK) {
	gweather_update (dialog->applet);

	gweather_dialog_update (dialog);
    } else {
        gtk_widget_destroy (GTK_WIDGET(dialog));
    }
}

static GString *
font_description_to_textview_css (PangoFontDescription *font_desc)
{
  GString *css;

  css = g_string_new ("textview {");

  g_string_append_printf (css, "font-family: %s;",
                          pango_font_description_get_family (font_desc));

  g_string_append_printf (css, "font-weight: %d;",
                          pango_font_description_get_weight (font_desc));

  if (pango_font_description_get_style (font_desc) == PANGO_STYLE_NORMAL)
    g_string_append (css, "font-style: normal;");
  else if (pango_font_description_get_style (font_desc) == PANGO_STYLE_OBLIQUE)
    g_string_append (css, "font-style: oblique;");
  else if (pango_font_description_get_style (font_desc) == PANGO_STYLE_ITALIC)
    g_string_append (css, "font-style: italic;");

  g_string_append_printf (css, "font-size: %d%s;",
                          pango_font_description_get_size (font_desc) / PANGO_SCALE,
                          pango_font_description_get_size_is_absolute (font_desc) ? "px" : "pt");

  g_string_append (css, "}");

  return css;
}

static void
update_forecast_font (GWeatherDialog *dialog)
{
  gchar *font_name;
  PangoFontDescription *font_desc;
  GString *css;

  font_name = g_settings_get_string (dialog->monospace_settings,
                                     MONOSPACE_FONT_KEY);

  font_desc = pango_font_description_from_string (font_name);
  g_free (font_name);

  if (font_desc == NULL)
    return;

  css = font_description_to_textview_css (font_desc);
  pango_font_description_free (font_desc);

  gtk_css_provider_load_from_data (dialog->css_provider, css->str, css->len, NULL);
  g_string_free (css, TRUE);
}

static void
monospace_font_name_changed_cb (GSettings      *settings,
                                const gchar    *key,
                                GWeatherDialog *dialog)
{
  update_forecast_font (dialog);
}

static void
gweather_dialog_create (GWeatherDialog *dialog)
{
  GWeatherApplet *gw_applet;

  GtkWidget *weather_vbox;
  GtkWidget *weather_notebook;
  GtkWidget *cond_hbox;
  GtkWidget *cond_grid;
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
  GtkWidget *cond_sunrise_lbl;
  GtkWidget *cond_sunset_lbl;
  GtkWidget *cond_frame_alignment;
  GtkWidget *current_note_lbl;
  GtkWidget *forecast_note_lbl;
  GtkWidget *forecast_hbox;
  GtkWidget *scrolled_window;

  gw_applet = dialog->applet;

  g_object_set (dialog, "destroy-with-parent", TRUE, NULL);
  gtk_window_set_title (GTK_WINDOW (dialog), _("Details"));
  gtk_dialog_add_buttons (GTK_DIALOG (dialog),
  			  _("_Update"), GTK_RESPONSE_OK,
  			  GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
			  NULL);
  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_CLOSE);
  gtk_container_set_border_width (GTK_CONTAINER (dialog), 5);
  gtk_box_set_spacing (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), 2);
  gtk_container_set_border_width (GTK_CONTAINER (dialog), 5);

  gtk_window_set_default_size (GTK_WINDOW (dialog), 590, 340);

  gtk_window_set_screen (GTK_WINDOW (dialog),
			 gtk_widget_get_screen (GTK_WIDGET (gw_applet)));

  /* Must come after load geometry, otherwise it will get reset. */
  gtk_window_set_resizable (GTK_WINDOW (dialog), TRUE);
  
  weather_vbox = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
  gtk_widget_show (weather_vbox);

  weather_notebook = gtk_notebook_new ();
  gtk_container_set_border_width (GTK_CONTAINER (weather_notebook), 5);
  gtk_widget_show (weather_notebook);
  gtk_box_pack_start (GTK_BOX (weather_vbox), weather_notebook, TRUE, TRUE, 0);

  cond_hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
  gtk_widget_show (cond_hbox);
  gtk_container_add (GTK_CONTAINER (weather_notebook), cond_hbox);
  gtk_container_set_border_width (GTK_CONTAINER (cond_hbox), 4);

  cond_grid = gtk_grid_new ();
  gtk_widget_show (cond_grid);
  gtk_box_pack_start (GTK_BOX (cond_hbox), cond_grid, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (cond_grid), 12);
  gtk_grid_set_row_spacing (GTK_GRID (cond_grid), 6);
  gtk_grid_set_column_spacing (GTK_GRID (cond_grid), 12);

  cond_location_lbl = gtk_label_new (_("City:"));
  gtk_widget_show (cond_location_lbl);
  gtk_grid_attach (GTK_GRID (cond_grid), cond_location_lbl, 0, 0, 1, 1);
  gtk_label_set_justify (GTK_LABEL (cond_location_lbl), GTK_JUSTIFY_LEFT);
  gtk_label_set_xalign (GTK_LABEL (cond_location_lbl), 0.0);

  cond_update_lbl = gtk_label_new (_("Last update:"));
  gtk_widget_show (cond_update_lbl);
  gtk_grid_attach (GTK_GRID (cond_grid), cond_update_lbl, 0, 1, 1, 1);
  gtk_label_set_justify (GTK_LABEL (cond_update_lbl), GTK_JUSTIFY_LEFT);
  gtk_label_set_xalign (GTK_LABEL (cond_update_lbl), 0.0);

  cond_cond_lbl = gtk_label_new (_("Conditions:"));
  gtk_widget_show (cond_cond_lbl);
  gtk_grid_attach (GTK_GRID (cond_grid), cond_cond_lbl, 0, 2, 1, 1);
  gtk_label_set_justify (GTK_LABEL (cond_cond_lbl), GTK_JUSTIFY_LEFT);
  gtk_label_set_xalign (GTK_LABEL (cond_cond_lbl), 0.0);

  cond_sky_lbl = gtk_label_new (_("Sky:"));
  gtk_widget_show (cond_sky_lbl);
  gtk_grid_attach (GTK_GRID (cond_grid), cond_sky_lbl, 0, 3, 1, 1);
  gtk_label_set_justify (GTK_LABEL (cond_sky_lbl), GTK_JUSTIFY_LEFT);
  gtk_label_set_xalign (GTK_LABEL (cond_sky_lbl), 0.0);

  cond_temp_lbl = gtk_label_new (_("Temperature:"));
  gtk_widget_show (cond_temp_lbl);
  gtk_grid_attach (GTK_GRID (cond_grid), cond_temp_lbl, 0, 4, 1, 1);
  gtk_label_set_justify (GTK_LABEL (cond_temp_lbl), GTK_JUSTIFY_LEFT);
  gtk_label_set_xalign (GTK_LABEL (cond_temp_lbl), 0.0);

  cond_apparent_lbl = gtk_label_new (_("Feels like:"));
  gtk_widget_show (cond_apparent_lbl);
  gtk_grid_attach (GTK_GRID (cond_grid), cond_apparent_lbl, 0, 5, 1, 1);
  gtk_label_set_justify (GTK_LABEL (cond_apparent_lbl), GTK_JUSTIFY_LEFT);
  gtk_label_set_xalign (GTK_LABEL (cond_apparent_lbl), 0.0);
  
  cond_dew_lbl = gtk_label_new (_("Dew point:"));
  gtk_widget_show (cond_dew_lbl);
  gtk_grid_attach (GTK_GRID (cond_grid), cond_dew_lbl, 0, 6, 1, 1);
  gtk_label_set_justify (GTK_LABEL (cond_dew_lbl), GTK_JUSTIFY_LEFT);
  gtk_label_set_xalign (GTK_LABEL (cond_dew_lbl), 0.0);

  cond_humidity_lbl = gtk_label_new (_("Relative humidity:"));
  gtk_widget_show (cond_humidity_lbl);
  gtk_grid_attach (GTK_GRID (cond_grid), cond_humidity_lbl, 0, 7, 1, 1);
  gtk_label_set_justify (GTK_LABEL (cond_humidity_lbl), GTK_JUSTIFY_LEFT);
  gtk_label_set_xalign (GTK_LABEL (cond_humidity_lbl), 0.0);

  cond_wind_lbl = gtk_label_new (_("Wind:"));
  gtk_widget_show (cond_wind_lbl);
  gtk_grid_attach (GTK_GRID (cond_grid), cond_wind_lbl, 0, 8, 1, 1);
  gtk_label_set_justify (GTK_LABEL (cond_wind_lbl), GTK_JUSTIFY_LEFT);
  gtk_label_set_xalign (GTK_LABEL (cond_wind_lbl), 0.0);

  cond_pressure_lbl = gtk_label_new (_("Pressure:"));
  gtk_widget_show (cond_pressure_lbl);
  gtk_grid_attach (GTK_GRID (cond_grid), cond_pressure_lbl, 0, 9, 1, 1);
  gtk_label_set_justify (GTK_LABEL (cond_pressure_lbl), GTK_JUSTIFY_LEFT);
  gtk_label_set_xalign (GTK_LABEL (cond_pressure_lbl), 0.0);

  cond_vis_lbl = gtk_label_new (_("Visibility:"));
  gtk_widget_show (cond_vis_lbl);
  gtk_grid_attach (GTK_GRID (cond_grid), cond_vis_lbl, 0, 10, 1, 1);
  gtk_label_set_justify (GTK_LABEL (cond_vis_lbl), GTK_JUSTIFY_LEFT);
  gtk_label_set_xalign (GTK_LABEL (cond_vis_lbl), 0.0);

  cond_sunrise_lbl = gtk_label_new (_("Sunrise:"));
  gtk_widget_show (cond_sunrise_lbl);
  gtk_grid_attach (GTK_GRID (cond_grid), cond_sunrise_lbl, 0, 11, 1, 1);
  gtk_label_set_justify (GTK_LABEL (cond_sunrise_lbl), GTK_JUSTIFY_LEFT);
  gtk_label_set_xalign (GTK_LABEL (cond_sunrise_lbl), 0.0);

  cond_sunset_lbl = gtk_label_new (_("Sunset:"));
  gtk_widget_show (cond_sunset_lbl);
  gtk_grid_attach (GTK_GRID (cond_grid), cond_sunset_lbl, 0, 12, 1, 1);
  gtk_label_set_justify (GTK_LABEL (cond_sunset_lbl), GTK_JUSTIFY_LEFT);
  gtk_label_set_xalign (GTK_LABEL (cond_sunset_lbl), 0.0);

  dialog->cond_location = gtk_label_new ("");
  gtk_widget_show (dialog->cond_location);
  gtk_grid_attach (GTK_GRID (cond_grid), dialog->cond_location, 1, 0, 1, 1);
  gtk_label_set_selectable (GTK_LABEL (dialog->cond_location), TRUE);
  gtk_label_set_justify (GTK_LABEL (dialog->cond_location), GTK_JUSTIFY_LEFT);
  gtk_label_set_xalign (GTK_LABEL (dialog->cond_location), 0.0);

  dialog->cond_update = gtk_label_new ("");
  gtk_widget_show (dialog->cond_update);
  gtk_grid_attach (GTK_GRID (cond_grid), dialog->cond_update, 1, 1, 1, 1);
  gtk_label_set_selectable (GTK_LABEL (dialog->cond_update), TRUE);
  gtk_label_set_justify (GTK_LABEL (dialog->cond_update), GTK_JUSTIFY_LEFT);
  gtk_label_set_xalign (GTK_LABEL (dialog->cond_update), 0.0);

  dialog->cond_cond = gtk_label_new ("");
  gtk_widget_show (dialog->cond_cond);
  gtk_grid_attach (GTK_GRID (cond_grid), dialog->cond_cond, 1, 2, 1, 1);
  gtk_label_set_selectable (GTK_LABEL (dialog->cond_cond), TRUE);
  gtk_label_set_justify (GTK_LABEL (dialog->cond_cond), GTK_JUSTIFY_LEFT);
  gtk_label_set_xalign (GTK_LABEL (dialog->cond_cond), 0.0);

  dialog->cond_sky = gtk_label_new ("");
  gtk_widget_show (dialog->cond_sky);
  gtk_grid_attach (GTK_GRID (cond_grid), dialog->cond_sky, 1, 3, 1, 1);
  gtk_label_set_selectable (GTK_LABEL (dialog->cond_sky), TRUE);
  gtk_label_set_justify (GTK_LABEL (dialog->cond_sky), GTK_JUSTIFY_LEFT);
  gtk_label_set_xalign (GTK_LABEL (dialog->cond_sky), 0.0);

  dialog->cond_temp = gtk_label_new ("");
  gtk_widget_show (dialog->cond_temp);
  gtk_grid_attach (GTK_GRID (cond_grid), dialog->cond_temp, 1, 4, 1, 1);
  gtk_label_set_selectable (GTK_LABEL (dialog->cond_temp), TRUE);
  gtk_label_set_justify (GTK_LABEL (dialog->cond_temp), GTK_JUSTIFY_LEFT);
  gtk_label_set_xalign (GTK_LABEL (dialog->cond_temp), 0.0);

  dialog->cond_apparent = gtk_label_new ("");
  gtk_widget_show (dialog->cond_apparent);
  gtk_grid_attach (GTK_GRID (cond_grid), dialog->cond_apparent, 1, 5, 1, 1);
  gtk_label_set_selectable (GTK_LABEL (dialog->cond_apparent), TRUE);
  gtk_label_set_justify (GTK_LABEL (dialog->cond_apparent), GTK_JUSTIFY_LEFT);
  gtk_label_set_xalign (GTK_LABEL (dialog->cond_apparent), 0.0);

  dialog->cond_dew = gtk_label_new ("");
  gtk_widget_show (dialog->cond_dew);
  gtk_grid_attach (GTK_GRID (cond_grid), dialog->cond_dew, 1, 6, 1, 1);
  gtk_label_set_selectable (GTK_LABEL (dialog->cond_dew), TRUE);
  gtk_label_set_justify (GTK_LABEL (dialog->cond_dew), GTK_JUSTIFY_LEFT);
  gtk_label_set_xalign (GTK_LABEL (dialog->cond_dew), 0.0);

  dialog->cond_humidity = gtk_label_new ("");
  gtk_widget_show (dialog->cond_humidity);
  gtk_grid_attach (GTK_GRID (cond_grid), dialog->cond_humidity, 1, 7, 1, 1);
  gtk_label_set_selectable (GTK_LABEL (dialog->cond_humidity), TRUE);
  gtk_label_set_justify (GTK_LABEL (dialog->cond_humidity), GTK_JUSTIFY_LEFT);
  gtk_label_set_xalign (GTK_LABEL (dialog->cond_humidity), 0.0);

  dialog->cond_wind = gtk_label_new ("");
  gtk_widget_show (dialog->cond_wind);
  gtk_grid_attach (GTK_GRID (cond_grid), dialog->cond_wind, 1, 8, 1, 1);
  gtk_label_set_selectable (GTK_LABEL (dialog->cond_wind), TRUE);
  gtk_label_set_justify (GTK_LABEL (dialog->cond_wind), GTK_JUSTIFY_LEFT);
  gtk_label_set_xalign (GTK_LABEL (dialog->cond_wind), 0.0);

  dialog->cond_pressure = gtk_label_new ("");
  gtk_widget_show (dialog->cond_pressure);
  gtk_grid_attach (GTK_GRID (cond_grid), dialog->cond_pressure, 1, 9, 1, 1);
  gtk_label_set_selectable (GTK_LABEL (dialog->cond_pressure), TRUE);
  gtk_label_set_justify (GTK_LABEL (dialog->cond_pressure), GTK_JUSTIFY_LEFT);
  gtk_label_set_xalign (GTK_LABEL (dialog->cond_pressure), 0.0);

  dialog->cond_vis = gtk_label_new ("");
  gtk_widget_show (dialog->cond_vis);
  gtk_grid_attach (GTK_GRID (cond_grid), dialog->cond_vis, 1, 10, 1, 1);
  gtk_label_set_selectable (GTK_LABEL (dialog->cond_vis), TRUE);
  gtk_label_set_justify (GTK_LABEL (dialog->cond_vis), GTK_JUSTIFY_LEFT);
  gtk_label_set_xalign (GTK_LABEL (dialog->cond_vis), 0.0);

  dialog->cond_sunrise = gtk_label_new ("");
  gtk_widget_show (dialog->cond_sunrise);
  gtk_grid_attach (GTK_GRID (cond_grid), dialog->cond_sunrise, 1, 11, 1, 1);
  gtk_label_set_selectable (GTK_LABEL (dialog->cond_sunrise), TRUE);
  gtk_label_set_justify (GTK_LABEL (dialog->cond_sunrise), GTK_JUSTIFY_LEFT);
  gtk_label_set_xalign (GTK_LABEL (dialog->cond_sunrise), 0.0);

  dialog->cond_sunset = gtk_label_new ("");
  gtk_widget_show (dialog->cond_sunset);
  gtk_grid_attach (GTK_GRID (cond_grid), dialog->cond_sunset, 1, 12, 1, 1);
  gtk_label_set_selectable (GTK_LABEL (dialog->cond_sunset), TRUE);
  gtk_label_set_justify (GTK_LABEL (dialog->cond_sunset), GTK_JUSTIFY_LEFT);
  gtk_label_set_xalign (GTK_LABEL (dialog->cond_sunset), 0.0);

  cond_frame_alignment = gtk_alignment_new (0.5, 0, 1, 0);
  gtk_widget_show (cond_frame_alignment);
  gtk_box_pack_end (GTK_BOX (cond_hbox), cond_frame_alignment, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (cond_frame_alignment), 2);

  dialog->cond_image = gtk_image_new_from_icon_name ("image-missing", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (dialog->cond_image);
  gtk_container_add (GTK_CONTAINER (cond_frame_alignment), dialog->cond_image);

  current_note_lbl = gtk_label_new (_("Current Conditions"));
  gtk_widget_show (current_note_lbl);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (weather_notebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (weather_notebook), 0), current_note_lbl);

  forecast_hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_container_set_border_width (GTK_CONTAINER (forecast_hbox), 12);
  gtk_widget_show (forecast_hbox);

  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window),
				       GTK_SHADOW_ETCHED_IN);

  dialog->forecast_text = gtk_text_view_new ();
  set_access_namedesc (dialog->forecast_text, _("Forecast Report"), _("See the ForeCast Details"));
  gtk_container_add (GTK_CONTAINER (scrolled_window), dialog->forecast_text);
  gtk_text_view_set_editable (GTK_TEXT_VIEW (dialog->forecast_text), FALSE);
  gtk_text_view_set_left_margin (GTK_TEXT_VIEW (dialog->forecast_text), 6);
  gtk_widget_show (dialog->forecast_text);
  gtk_widget_show (scrolled_window);
  gtk_box_pack_start (GTK_BOX (forecast_hbox), scrolled_window, TRUE, TRUE, 0);

  dialog->monospace_settings = g_settings_new (MONOSPACE_GSCHEMA);
  dialog->css_provider = gtk_css_provider_new ();

  g_signal_connect (dialog->monospace_settings, "changed::" MONOSPACE_FONT_KEY,
                    G_CALLBACK (monospace_font_name_changed_cb), dialog);

  gtk_style_context_add_provider (gtk_widget_get_style_context (dialog->forecast_text),
                                  GTK_STYLE_PROVIDER (dialog->css_provider),
                                  GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  gtk_container_add (GTK_CONTAINER (weather_notebook), forecast_hbox);

  forecast_note_lbl = gtk_label_new (_("Forecast"));
  gtk_widget_show (forecast_note_lbl);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (weather_notebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (weather_notebook), 1), forecast_note_lbl);

  g_signal_connect (G_OBJECT (dialog), "response", G_CALLBACK (response_cb), NULL);
}

static gchar *
get_forecast (GWeatherInfo *info)
{
  GSList *forecast;
  GString *buffer;
  GSList *l;

  forecast = gweather_info_get_forecast_list (info);
  if (!forecast)
    return NULL;

  buffer = g_string_new ("");

  for (l = forecast; l != NULL; l = l->next)
    {
      GWeatherInfo *forecast_info;
      gchar *date;
      gchar *summary;
      gchar *temp;

      forecast_info = l->data;

      date = gweather_info_get_update (forecast_info);
      summary = gweather_info_get_conditions (forecast_info);
      temp = gweather_info_get_temp_summary (forecast_info);

      if (g_str_equal (summary, "-"))
        {
          g_free (summary);
          summary = gweather_info_get_sky (forecast_info);
        }

      g_string_append_printf (buffer, " * %s: %s, %s\n", date, summary, temp);

      g_free (date);
      g_free (summary);
      g_free (temp);
    }

  return g_string_free (buffer, FALSE);
}

static void
gweather_dialog_constructed (GObject *object)
{
  GWeatherDialog *dialog;

  dialog = GWEATHER_DIALOG (object);
  G_OBJECT_CLASS (gweather_dialog_parent_class)->constructed (object);

  gweather_dialog_create (dialog);
  gweather_dialog_update (dialog);
}

static void
gweather_dialog_dispose (GObject *object)
{
  GWeatherDialog *dialog;

  dialog = GWEATHER_DIALOG (object);

  g_clear_object (&dialog->monospace_settings);
  g_clear_object (&dialog->css_provider);

  G_OBJECT_CLASS (gweather_dialog_parent_class)->dispose (object);
}

static void
gweather_dialog_get_property (GObject    *object,
                              guint       property_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  GWeatherDialog *dialog;

  dialog = GWEATHER_DIALOG (object);

  switch (property_id)
    {
      case PROP_GWEATHER_APPLET:
        g_value_set_pointer (value, dialog->applet);
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}

static void
gweather_dialog_set_property (GObject      *object,
                              guint         property_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  GWeatherDialog *dialog;

  dialog = GWEATHER_DIALOG (object);

  switch (property_id)
    {
      case PROP_GWEATHER_APPLET:
        dialog->applet = g_value_get_pointer (value);
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}

static void
gweather_dialog_style_updated (GtkWidget *widget)
{
  GTK_WIDGET_CLASS (gweather_dialog_parent_class)->style_updated (widget);
  update_forecast_font (GWEATHER_DIALOG (widget));
}

static void
gweather_dialog_init (GWeatherDialog *dialog)
{
}

static void
gweather_dialog_class_init (GWeatherDialogClass *dialog_class)
{
  GObjectClass *object_class;
  GtkWidgetClass *widget_class;

  object_class = G_OBJECT_CLASS (dialog_class);
  widget_class = GTK_WIDGET_CLASS (dialog_class);

  object_class->constructed = gweather_dialog_constructed;
  object_class->dispose = gweather_dialog_dispose;
  object_class->get_property = gweather_dialog_get_property;
  object_class->set_property = gweather_dialog_set_property;

  widget_class->style_updated = gweather_dialog_style_updated;

  /* This becomes an OBJECT property when GWeatherApplet is redone */
  dialog_properties[PROP_GWEATHER_APPLET] =
    g_param_spec_pointer ("gweather-applet",
                          "GWeather Applet",
                          "The GWeather Applet",
                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  g_object_class_install_properties (object_class, PROP_LAST, dialog_properties);
}

GtkWidget *
gweather_dialog_new (GWeatherApplet *applet)
{
  return g_object_new (GWEATHER_TYPE_DIALOG,
                       "gweather-applet", applet,
                        NULL);
}

void
gweather_dialog_update (GWeatherDialog *dialog)
{
  GWeatherInfo *weather_info;
  const gchar *icon_name;
  gchar *text;
  GtkTextBuffer *buffer;
  gchar *forecast;

  weather_info = dialog->applet->gweather_info;

  /* Check for parallel network update in progress */
  if (weather_info == NULL)
	  return;

  /* Update icon */
  icon_name = gweather_info_get_icon_name (weather_info);
  gtk_image_set_from_icon_name (GTK_IMAGE (dialog->cond_image),
                                icon_name, GTK_ICON_SIZE_DIALOG);

  /* Update current condition fields and forecast */
  text = gweather_info_get_location_name (weather_info);
  gtk_label_set_text (GTK_LABEL (dialog->cond_location), text);
  g_free (text);

  text = gweather_info_get_update (weather_info);
  gtk_label_set_text (GTK_LABEL (dialog->cond_update), text);
  g_free (text);

  text = gweather_info_get_conditions (weather_info);
  gtk_label_set_text (GTK_LABEL (dialog->cond_cond), text);
  g_free (text);

  text = gweather_info_get_sky (weather_info);
  gtk_label_set_text (GTK_LABEL (dialog->cond_sky), text);
  g_free (text);

  text = gweather_info_get_temp (weather_info);
  gtk_label_set_text (GTK_LABEL (dialog->cond_temp), text);
  g_free (text);

  text = gweather_info_get_apparent (weather_info);
  gtk_label_set_text (GTK_LABEL (dialog->cond_apparent), text);
  g_free (text);

  text = gweather_info_get_dew (weather_info);
  gtk_label_set_text (GTK_LABEL (dialog->cond_dew), text);
  g_free (text);

  text = gweather_info_get_humidity (weather_info);
  gtk_label_set_text (GTK_LABEL (dialog->cond_humidity), text);
  g_free (text);

  text = gweather_info_get_wind (weather_info);
  gtk_label_set_text (GTK_LABEL (dialog->cond_wind), text);
  g_free (text);

  text = gweather_info_get_pressure (weather_info);
  gtk_label_set_text (GTK_LABEL (dialog->cond_pressure), text);
  g_free (text);

  text = gweather_info_get_visibility (weather_info);
  gtk_label_set_text (GTK_LABEL (dialog->cond_vis), text);
  g_free (text);

  text = gweather_info_get_sunrise (weather_info);
  gtk_label_set_text (GTK_LABEL (dialog->cond_sunrise), text);
  g_free (text);

  text = gweather_info_get_sunset (weather_info);
  gtk_label_set_text (GTK_LABEL (dialog->cond_sunset), text);
  g_free (text);

  /* Update forecast */
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (dialog->forecast_text));
  forecast = get_forecast (weather_info);

  if (forecast && *forecast != '\0')
    {
      gtk_text_buffer_set_text (buffer, forecast, -1);
    }
  else
    {
      const gchar *unavailable;

      unavailable = _("Forecast not currently available for this location.");
      gtk_text_buffer_set_text (buffer, unavailable, -1);
    }

  g_free (forecast);
}
