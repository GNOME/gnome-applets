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

enum
{
	COL_DAY=0,
	COL_LOW,
	COL_HIGH,
	COL_PRECIP,
	COL_COND,
	COL_PIXBUF,
	NUM_COL,
};


static void response_cb (GtkDialog *dialog, gint id, gpointer data)
{
    GWeatherApplet *gw_applet = (GWeatherApplet *)data;
	
    gweather_dialog_close(gw_applet);
    return;
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
  	GtkWidget *cond_frame_alignment;
  	GtkWidget *cond_vsep;
  	GtkWidget *current_note_lbl;
  	GtkWidget *forecast_note_lbl;
  	GtkWidget *close_button;
  	GtkWidget *ebox;
  	GtkWidget *scrolled;
	GtkWidget *frame;
	GtkWidget *hbox, *hbox1;
	GtkWidget *vbox, *vbox1, *vbox2, *vbox3;
	GtkWidget *table;
	GtkWidget *label;
	GtkTreeStore *model;
	GtkWidget *tree;
	GtkCellRenderer *cell;
	GtkTreeViewColumn *column;
	gchar *tmp;
	static gchar *title[] = {N_("Day"), N_("Low"), N_("High"), 
				 N_("Precipitation"), N_("Condition")};
	gint i;

  	gw_applet->gweather_dialog = gtk_dialog_new_with_buttons (_("Forecast"), NULL,
						  GTK_DIALOG_DESTROY_WITH_PARENT,
						  GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
						  NULL);
  	gtk_dialog_set_default_response (GTK_DIALOG (gw_applet->gweather_dialog), 
  				   GTK_RESPONSE_CLOSE);

 
  	gtk_window_set_default_size (GTK_WINDOW(gw_applet->gweather_dialog), 490, 510);

  	gtk_window_set_screen (GTK_WINDOW (gw_applet->gweather_dialog),
			 gtk_widget_get_screen (GTK_WIDGET (gw_applet->applet)));
 
  	weather_vbox = GTK_DIALOG (gw_applet->gweather_dialog)->vbox;
  	gtk_widget_show (weather_vbox);

	vbox = gtk_vbox_new (FALSE, 18);
	gtk_box_pack_start (GTK_BOX (weather_vbox), vbox, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 12);

	frame = gtk_frame_new (NULL);
    	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);
    	gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);	
   	 label = gtk_label_new (NULL);
    	tmp = g_strdup_printf ("<b>%s</b>", _("Current Conditions"));
    	gtk_label_set_markup (GTK_LABEL (label), tmp);
    	g_free (tmp);
    	gtk_frame_set_label_widget (GTK_FRAME (frame), label);

	table = gtk_table_new (9, 2, FALSE);
	gtk_container_add (GTK_CONTAINER (frame), table);
    	gtk_container_set_border_width (GTK_CONTAINER (table), 12);
	gtk_table_set_row_spacings (GTK_TABLE (table), 6);
	gtk_table_set_col_spacings (GTK_TABLE (table), 6);

	label = gtk_label_new (_("City:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1, GTK_FILL, 0, 0, 0);
	gw_applet->cond_location = gtk_label_new (NULL);
	gtk_misc_set_alignment (GTK_MISC (gw_applet->cond_location), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), gw_applet->cond_location, 1, 2, 0, 1, GTK_FILL, 0, 0, 0);
	
	label = gtk_label_new (_("Last update:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2, GTK_FILL, 0, 0, 0);
	gw_applet->cond_update = gtk_label_new (NULL);
	gtk_misc_set_alignment (GTK_MISC (gw_applet->cond_update), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), gw_applet->cond_update, 1, 2, 1, 2, GTK_FILL, 0, 0, 0);
	
	label = gtk_label_new (_("Conditions:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3, GTK_FILL, 0, 0, 0);
	hbox1 = gtk_hbox_new (FALSE, 6);
	gtk_table_attach (GTK_TABLE (table), hbox1, 1, 2, 2, 3, GTK_FILL, 0, 0, 0);
	gw_applet->cond_image = gtk_image_new ();
	gtk_box_pack_start (GTK_BOX (hbox1), gw_applet->cond_image, FALSE, FALSE, 0);
	gw_applet->cond_cond = gtk_label_new (NULL);
	gtk_misc_set_alignment (GTK_MISC (gw_applet->cond_cond), 0.0, 0.5);
	gtk_box_pack_start (GTK_BOX (hbox1), gw_applet->cond_cond, FALSE, FALSE, 0);
	
	label = gtk_label_new (_("Temperature:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 3, 4, GTK_FILL, 0, 0, 0);
	gw_applet->cond_temp = gtk_label_new (NULL);
	gtk_misc_set_alignment (GTK_MISC (gw_applet->cond_temp), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), gw_applet->cond_temp, 1, 2, 3, 4, GTK_FILL, 0, 0, 0);

	label = gtk_label_new (_("Feels like:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 4, 5, GTK_FILL, 0, 0, 0);
	gw_applet->cond_feeltemp = gtk_label_new (NULL);
	gtk_misc_set_alignment (GTK_MISC (gw_applet->cond_feeltemp), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), gw_applet->cond_feeltemp, 1, 2, 4, 5, GTK_FILL, 0, 0, 0);
	
	label = gtk_label_new (_("Humidity:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 5, 6, GTK_FILL, 0, 0, 0);
	gw_applet->cond_humidity = gtk_label_new (NULL);
	gtk_misc_set_alignment (GTK_MISC (gw_applet->cond_humidity), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), gw_applet->cond_humidity, 1, 2, 5, 6, GTK_FILL, 0, 0, 0);
	
	label = gtk_label_new (_("Wind:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 6, 7, GTK_FILL, 0, 0, 0);
	gw_applet->cond_wind = gtk_label_new (NULL);
	gtk_misc_set_alignment (GTK_MISC (gw_applet->cond_wind), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), gw_applet->cond_wind, 1, 2, 6, 7, GTK_FILL, 0, 0, 0);
	
	label = gtk_label_new (_("Visibilty:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 7, 8, GTK_FILL, 0, 0, 0);
	gw_applet->cond_vis = gtk_label_new (NULL);
	gtk_misc_set_alignment (GTK_MISC (gw_applet->cond_vis), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), gw_applet->cond_vis, 1, 2, 7, 8, GTK_FILL, 0, 0, 0);

	label = gtk_label_new (_("Pressure:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 8, 9, GTK_FILL, 0, 0, 0);
	gw_applet->cond_pressure = gtk_label_new (NULL);
	gtk_misc_set_alignment (GTK_MISC (gw_applet->cond_pressure), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), gw_applet->cond_pressure, 1, 2, 8, 9, GTK_FILL, 0, 0, 0);
	
	frame = gtk_frame_new (NULL);
    	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);
    	gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);	
   	label = gtk_label_new (NULL);
    	tmp = g_strdup_printf ("<b>%s</b>", _("Forecast"));
    	gtk_label_set_markup (GTK_LABEL (label), tmp);
    	g_free (tmp);
    	gtk_frame_set_label_widget (GTK_FRAME (frame), label);

	vbox2 = gtk_vbox_new (FALSE, 6);
    	gtk_container_add (GTK_CONTAINER (frame), vbox2);
    	gtk_container_set_border_width (GTK_CONTAINER (vbox2), 12);

	scrolled = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled)	,
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start (GTK_BOX (vbox2), scrolled, TRUE, TRUE, 0);

	model = gtk_tree_store_new (6, G_TYPE_STRING,  G_TYPE_STRING,
							     G_TYPE_STRING, G_TYPE_STRING,
							     G_TYPE_STRING, GDK_TYPE_PIXBUF);

	tree = gtk_tree_view_new_with_model (GTK_TREE_MODEL (model));
	gw_applet->forecast_tree = tree;
	gw_applet->forecast_model = GTK_TREE_MODEL (model);
	g_object_unref (G_OBJECT (model));
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (tree), TRUE);
	gtk_container_add (GTK_CONTAINER (scrolled), tree);

	for (i=0; i<4; i++) {
		cell = gtk_cell_renderer_text_new ();
		column = gtk_tree_view_column_new_with_attributes (_(title[i]),
						    		   cell,
						     		   "text", i,
						     		   NULL);
		gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);
		gtk_tree_view_column_set_alignment (column, 0.5);
	}

	column = gtk_tree_view_column_new ();
	cell = gtk_cell_renderer_pixbuf_new ();
	gtk_tree_view_column_pack_start (column, cell, FALSE);
	gtk_tree_view_column_set_attributes (column, cell,
                                            				     "pixbuf", COL_PIXBUF,
                                             				     NULL);
	cell = gtk_cell_renderer_text_new ();
        gtk_tree_view_column_pack_start (column, cell, FALSE);
        gtk_tree_view_column_set_attributes (column, cell,
                                             				     "text", COL_COND,
                                             				      NULL);
        gtk_tree_view_column_set_title (column, _(title[4]));
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);

  	g_signal_connect (G_OBJECT (gw_applet->gweather_dialog), "response",
  		    G_CALLBACK (response_cb), gw_applet);
  
}

void gweather_dialog_open (GWeatherApplet *gw_applet)
{
    if (!gw_applet->gweather_dialog)
        gweather_dialog_create(gw_applet);
    else {
	gtk_window_present (GTK_WINDOW (gw_applet->gweather_dialog));
	return;
    }

    gweather_dialog_update(gw_applet);

   gtk_widget_show_all(gw_applet->gweather_dialog);
}

void gweather_dialog_close (GWeatherApplet *gw_applet)
{
    g_return_if_fail(gw_applet->gweather_dialog != NULL);
    gtk_widget_destroy(gw_applet->gweather_dialog);
    gw_applet->gweather_dialog = NULL;
}

void gweather_dialog_update (GWeatherApplet *gw_applet)
{
    	WeatherInfo *info = gw_applet->gweather_info;
	gchar *tmp;
	gchar *degree, *speed, *vis, *barunit;
	gfloat strength, visvalue, barometer;
	GList *list = info->forecasts;

    	if(gw_applet->gweather_info == NULL)
    		return;

   	if (!gw_applet->gweather_dialog)
        	return;
	if (gw_applet->gweather_pref.city)
		gtk_label_set_text(GTK_LABEL(gw_applet->cond_location), gw_applet->gweather_pref.city);
	gtk_label_set_text(GTK_LABEL(gw_applet->cond_update), info->date);
	gtk_label_set_text(GTK_LABEL(gw_applet->cond_cond), get_conditions (info->wid));
	if (gw_applet->gweather_pref.use_metric)
		degree = "C";
	else
		degree = "F";
	tmp = g_strdup_printf ("%d \302\260%s", info->curtemp, degree);
    	gtk_label_set_text(GTK_LABEL(gw_applet->cond_temp), tmp);
	g_free (tmp);
	tmp = g_strdup_printf ("%d \302\260%s", info->realtemp, degree);
    	gtk_label_set_text(GTK_LABEL(gw_applet->cond_feeltemp), tmp);
	g_free (tmp);
	tmp = g_strdup_printf ("%d %%", info->humidity);
    	gtk_label_set_text(GTK_LABEL(gw_applet->cond_humidity), tmp);
	g_free (tmp);

	if (info->winddir) {
		if (gw_applet->gweather_pref.use_metric) {
			speed = "km/h";
			strength = (float) info->windstrength * 1.620934;
		}
		else {
			speed = "mi/h";
			strength = (float) info->windstrength;
		}
		tmp = g_strdup_printf (_("%s / %.1f %s"), 
							 get_wind_direction (info->winddir),
						         strength, speed);
	}
	else
		tmp = NULL;
    	gtk_label_set_text(GTK_LABEL(gw_applet->cond_wind), tmp);
	g_free (tmp);
	if (gw_applet->gweather_pref.use_metric) {
		vis = "km";
		visvalue = info->visibility * 1.60934;
	}
	else {
		vis = "mi";
		visvalue = info->visibility;
	}
	tmp = g_strdup_printf ("%.1f %s", visvalue, vis);
    	gtk_label_set_text(GTK_LABEL(gw_applet->cond_vis), tmp);
	g_free (tmp); 
	if (gw_applet->gweather_pref.use_metric) {
		barunit = "hPa";
		barometer = info->barometer * 33.86;
	}
	else {
		barunit = "in";
		barometer = info->barometer;
	}
	tmp = g_strdup_printf ("%.2f %s", barometer, barunit);
	gtk_label_set_text(GTK_LABEL(gw_applet->cond_pressure), tmp);
	g_free (tmp); 
	gtk_image_set_from_pixbuf (GTK_IMAGE (gw_applet->cond_image), 
                               				     get_conditions_pixbuf (info->wid));

	/* Update forecast */
	gtk_tree_store_clear (GTK_TREE_STORE (gw_applet->forecast_model));
	while (list) {
		WeatherForecast *forecast = list->data;
		GtkTreeIter iter;
		gchar *high, *low, *prec;
		high = g_strdup_printf ("%d", forecast->high);
		low = g_strdup_printf ("%d", forecast->low);
		prec = g_strdup_printf ("%d %%", forecast->precip);
		gtk_tree_store_append (GTK_TREE_STORE (gw_applet->forecast_model), &iter, NULL);
		gtk_tree_store_set (GTK_TREE_STORE (gw_applet->forecast_model),
						   &iter,
						   COL_DAY, forecast->day, 
						   COL_LOW, low,
						   COL_HIGH, high,
						   COL_COND, get_conditions (forecast->wid),
						   COL_PRECIP, prec,
						   COL_PIXBUF, get_conditions_pixbuf (forecast->wid), -1);
		g_free (high);
		g_free (low);
		g_free (prec);
		list = g_list_next (list);
	}

   
}

