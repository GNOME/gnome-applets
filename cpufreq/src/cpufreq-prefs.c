/*
 * Copyright (C) 2001, 2002 Free Software Foundation
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Authors : Carlos García Campos <carlosgc@gnome.org>
 */

#include <config.h>

#include <gnome.h>
#include <panel-applet.h>
#include <panel-applet-gconf.h>
#include <gconf/gconf-client.h>
#include <glade/glade.h>

#include "cpufreq-applet.h"
#include "cpufreq-prefs.h"

static gboolean cpufreq_key_is_writable          (CPUFreqApplet *applet, const gchar *key);
static void     cpufreq_prefs_response_cb        (GtkDialog *dialog, gint response, gpointer gdata);
static void     cpufreq_prefs_show_freq_toggled  (GtkWidget *show_freq, gpointer *gdata);
static void     cpufreq_prefs_show_unit_toggled  (GtkWidget *show_unit, gpointer *gdata);
static void     cpufreq_prefs_show_perc_toggled  (GtkWidget *show_perc, gpointer *gdata);
static void     cpufreq_prefs_show_mode_changed  (GtkWidget *cpu_number, gpointer *gdata);
static void     cpufreq_prefs_cpu_number_changed (GtkWidget *show_mode, gpointer *gdata);

static gboolean
cpufreq_key_is_writable (CPUFreqApplet *applet, const gchar *key)
{
	   gboolean     writable;
	   gchar       *fullkey;
	   static GConfClient *gconf_client = NULL;

	   if (gconf_client == NULL)
			 gconf_client = gconf_client_get_default ();

	   fullkey = panel_applet_gconf_get_full_key (PANEL_APPLET (applet), key);

	   writable = gconf_client_key_is_writable (gconf_client, fullkey, NULL);

	   g_free (fullkey);

	   return writable;
}

static void
cpufreq_prefs_show_freq_toggled (GtkWidget *show_freq, gpointer *gdata)
{
	   GtkWidget *show_unit;
	   gboolean   key_writable;
	   CPUFreqApplet *applet;

	   applet = (CPUFreqApplet *) gdata;
	   
	   show_unit = g_object_get_data (G_OBJECT (applet->prefs), "prefs-show-unit");

	   if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (show_freq))) {
			 /* Show cpu usage in frequency (Hz) */
			 key_writable = cpufreq_key_is_writable (applet, "show_text_mode");
			 gtk_widget_set_sensitive (show_unit, (TRUE && key_writable));

			 if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (show_unit)))
				    applet->show_text_mode = MODE_TEXT_FREQUENCY_UNIT;
			 else
				    applet->show_text_mode = MODE_TEXT_FREQUENCY;
				
			 if (applet->freq) {
				    gtk_label_set_label (GTK_LABEL (applet->label), applet->freq);
				    
				    if (applet->show_text_mode == MODE_TEXT_FREQUENCY_UNIT) {
						  gtk_label_set_label (GTK_LABEL (applet->unit_label), applet->unit);
						  gtk_widget_show (applet->unit_label);
				    } else {
						  gtk_widget_hide (applet->unit_label);
				    }
			 }
				    
			 gtk_widget_show (applet->label);
			 panel_applet_gconf_set_int (PANEL_APPLET (applet), "show_text_mode",
								    applet->show_text_mode, NULL);
	   } else {
			 gtk_widget_set_sensitive (show_unit, FALSE);
	   }
}

static void
cpufreq_prefs_show_unit_toggled (GtkWidget *show_unit, gpointer *gdata)
{
	   CPUFreqApplet *applet;

	   applet = (CPUFreqApplet *) gdata;
	   
	   if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (show_unit))) {
			 applet->show_text_mode = MODE_TEXT_FREQUENCY_UNIT;
			 gtk_label_set_label (GTK_LABEL (applet->unit_label), applet->unit);
			 gtk_widget_show (applet->unit_label);
	   } else {
			 applet->show_text_mode = MODE_TEXT_FREQUENCY;
			 gtk_widget_hide (applet->unit_label);
	   }

	   panel_applet_gconf_set_int (PANEL_APPLET (applet), "show_text_mode",
							 applet->show_text_mode, NULL);
}

static void
cpufreq_prefs_show_perc_toggled (GtkWidget *show_perc, gpointer *gdata)
{
	   CPUFreqApplet *applet;

	   applet = (CPUFreqApplet *) gdata;

	   if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (show_perc))) {
			 /* Show cpu usage in percentage */
			 applet->show_text_mode = MODE_TEXT_PERCENTAGE;
			 
			 if (applet->perc)
				    gtk_label_set_label (GTK_LABEL (applet->label), applet->perc);
			 
			 gtk_widget_show (applet->label);
			 gtk_widget_hide (applet->unit_label);
			 
			 panel_applet_gconf_set_int (PANEL_APPLET (applet), "show_text_mode",
								    applet->show_text_mode, NULL);
	   }
}

static void
cpufreq_prefs_cpu_number_changed (GtkWidget *cpu_number, gpointer *gdata)
{
	   CPUFreqApplet *applet;
	   
	   applet = (CPUFreqApplet *) gdata;
	   
	   applet->cpu = gtk_combo_box_get_active (GTK_COMBO_BOX (cpu_number));
	   panel_applet_gconf_set_int (PANEL_APPLET (applet), "cpu", applet->cpu, NULL);
}

static void
cpufreq_prefs_show_mode_changed (GtkWidget *show_mode, gpointer *gdata)
{
	   gboolean   key_writable;
	   GtkWidget *show_freq, *show_unit, *show_perc;
	   CPUFreqApplet *applet;

	   applet = (CPUFreqApplet *) gdata;

	   show_freq = g_object_get_data (G_OBJECT (applet->prefs), "prefs-show-freq");
	   show_unit = g_object_get_data (G_OBJECT (applet->prefs), "prefs-show-unit");
	   show_perc = g_object_get_data (G_OBJECT (applet->prefs), "prefs-show-perc");
	   
	   applet->show_mode = gtk_combo_box_get_active (GTK_COMBO_BOX (show_mode));

	   if (applet->show_mode != MODE_GRAPHIC) {
			 key_writable = cpufreq_key_is_writable (applet, "show_text_mode");
			 
			 if (applet->show_mode == MODE_TEXT)
				    gtk_widget_hide (applet->pixmap);
			 else
				    gtk_widget_show (applet->pixmap);
			 
			 gtk_widget_show (applet->label);
			 
			 if (applet->show_text_mode == MODE_TEXT_FREQUENCY_UNIT)
				    gtk_widget_show (applet->unit_label);
			 else
				    gtk_widget_hide (applet->unit_label);

			 gtk_widget_set_sensitive (show_freq, (TRUE && key_writable));
			 
			 if (applet->show_text_mode == MODE_TEXT_PERCENTAGE) {
				    gtk_widget_set_sensitive (show_unit, FALSE);
			 } else {
				    gtk_widget_set_sensitive (show_unit, (TRUE && key_writable));
			 }

			 gtk_widget_set_sensitive (show_perc, (TRUE && key_writable));
	   } else {
			 gtk_widget_show (applet->pixmap);
			 gtk_widget_hide (applet->label);
			 gtk_widget_hide (applet->unit_label);
			 
			 gtk_widget_set_sensitive (show_freq, FALSE);
			 gtk_widget_set_sensitive (show_unit, FALSE);
			 gtk_widget_set_sensitive (show_perc, FALSE);
	   }
	   
	   panel_applet_gconf_set_int (PANEL_APPLET (applet), "show_mode",
							 applet->show_mode, NULL);
}

static void
cpufreq_prefs_response_cb (GtkDialog *dialog, gint response, gpointer gdata)
{
	   GError *error;
	   CPUFreqApplet *applet;

	   applet = (CPUFreqApplet *) gdata;

	   if (response == GTK_RESPONSE_HELP) {
			 error = NULL;
			 gnome_help_display_on_screen ("cpufreq-applet", "cpufreq-applet-prefs",
									 gtk_widget_get_screen (GTK_WIDGET (applet)),
									 &error);
			 if (error) {
				    cpufreq_applet_display_error (_("Could not open help document"),
				                                  error->message);
				    g_error_free (error);
			 }
	   } else {
			 gtk_widget_destroy (applet->prefs);
			 applet->prefs = NULL;
	   }
}

void
cpufreq_prefs_cpu_combo_setup (GtkWidget *cpu_number, CPUFreqApplet *applet)
{
	   GtkListStore *model;
	   GtkTreeIter iter;
	   GtkCellRenderer *renderer;
	   gint i;
	   gchar *text_label;
	   
	   model = gtk_list_store_new (1, G_TYPE_STRING);
	   gtk_combo_box_set_model (GTK_COMBO_BOX (cpu_number),
						   GTK_TREE_MODEL (model));

	   for (i=0; i <= applet->mcpu; i++) {
			 text_label = g_strdup_printf ("CPU %d", i);

			 gtk_list_store_append (model, &iter);
			 gtk_list_store_set (model, &iter,
							 0, text_label,
							 -1);

			 g_free (text_label);
	   }

	   g_object_unref (model);

	   renderer = gtk_cell_renderer_text_new ();
	   gtk_cell_layout_clear (GTK_CELL_LAYOUT (cpu_number));
	   gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (cpu_number), renderer, TRUE);
	   gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (cpu_number), renderer,
								"text", 0, NULL);

	   gtk_combo_box_set_active (GTK_COMBO_BOX (cpu_number), applet->cpu);

	   gtk_widget_set_sensitive (GTK_WIDGET (cpu_number),
						    cpufreq_key_is_writable (applet, "cpu"));
}

void
cpufreq_prefs_show_mode_combo_setup (GtkWidget *show_mode, CPUFreqApplet *applet)
{
	   GtkListStore *model;
	   GtkTreeIter iter;
	   GtkCellRenderer *renderer;

	   model = gtk_list_store_new (1, G_TYPE_STRING);
	   gtk_combo_box_set_model (GTK_COMBO_BOX (show_mode),
						   GTK_TREE_MODEL (model));

	   gtk_list_store_append (model, &iter);
	   gtk_list_store_set (model, &iter,
					   0, _("Graphic"),
					   -1);
	   
	   gtk_list_store_append (model, &iter);
	   gtk_list_store_set (model, &iter,
					   0, _("Text"),
					   -1);

	   gtk_list_store_append (model, &iter);
	   gtk_list_store_set (model, &iter,
					   0, _("Graphic and Text"),
					   -1);
	   
	   g_object_unref (model);

	   renderer = gtk_cell_renderer_text_new ();
	   gtk_cell_layout_clear (GTK_CELL_LAYOUT (show_mode));
	   gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (show_mode), renderer, TRUE);
	   gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (show_mode), renderer,
								"text", 0, NULL);

	   gtk_combo_box_set_active (GTK_COMBO_BOX (show_mode), applet->show_mode);

	   gtk_widget_set_sensitive (GTK_WIDGET (show_mode),
						    cpufreq_key_is_writable (applet, "show_mode"));
}

void 
cpufreq_preferences_dialog_run (CPUFreqApplet *applet)
{
	   GladeXML  *xml;
	   GtkWidget *show_freq, *show_unit, *show_perc;
	   GtkWidget *cpu_number;
	   GtkWidget *monitor_settings_box;
	   GtkWidget *show_mode;
	   gboolean   key_writable;

	   g_return_if_fail (PANEL_IS_APPLET (PANEL_APPLET (applet)));

	   if (applet->prefs) { /* the dialog already exist, only show it */
			 gtk_window_present (GTK_WINDOW (applet->prefs));
			 return;
	   }
	   
	   xml = glade_xml_new (DATADIR"/cpufreq-applet/cpufreq-preferences.glade",
					    "prefs_dialog", NULL);

	   applet->prefs = glade_xml_get_widget (xml, "prefs_dialog");
	   show_mode = glade_xml_get_widget (xml, "prefs_show_mode");
	   show_freq = glade_xml_get_widget (xml, "prefs_show_freq");
	   show_unit = glade_xml_get_widget (xml, "prefs_show_unit");
	   show_perc = glade_xml_get_widget (xml, "prefs_show_perc");
	   cpu_number = glade_xml_get_widget (xml, "prefs_cpu_number");
	   monitor_settings_box = glade_xml_get_widget (xml, "monitor_settings_box");

	   g_object_unref (G_OBJECT (xml));

	   gtk_window_set_screen (GTK_WINDOW (applet->prefs),
						 gtk_widget_get_screen (GTK_WIDGET (applet)));

	   switch (applet->show_text_mode) {
	   case MODE_TEXT_FREQUENCY:
			 gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (show_freq), TRUE);
			 gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (show_unit), FALSE);

			 break;
	   case MODE_TEXT_FREQUENCY_UNIT:
			 gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (show_freq), TRUE);
			 gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (show_unit), TRUE);

			 break;
	   case MODE_TEXT_PERCENTAGE:
			 gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (show_perc), TRUE);

			 break;
	   }

	   if (applet->show_mode != MODE_GRAPHIC) {
			 key_writable = cpufreq_key_is_writable (applet, "show_text_mode");
			 
			 gtk_widget_set_sensitive (show_freq, (TRUE && key_writable));
			 gtk_widget_set_sensitive (show_perc, (TRUE && key_writable));
			 
			 if (applet->show_text_mode == MODE_TEXT_PERCENTAGE)
				    gtk_widget_set_sensitive (show_unit, FALSE);
			 else
				    gtk_widget_set_sensitive (show_unit, (TRUE && key_writable));
	   } else {
			 gtk_widget_set_sensitive (show_freq, FALSE);
			 gtk_widget_set_sensitive (show_unit, FALSE);
			 gtk_widget_set_sensitive (show_perc, FALSE);
	   }

	   /* Fill the cpu combo */
	   
	   if (applet->mcpu > 0) {
			 cpufreq_prefs_cpu_combo_setup (cpu_number, applet);
			 gtk_widget_show (cpu_number);
			 gtk_widget_show (monitor_settings_box);
	   } else {
			 gtk_widget_hide (monitor_settings_box);
	   }

	   cpufreq_prefs_show_mode_combo_setup (show_mode, applet);
	   gtk_widget_show (show_mode);

	   g_object_set_data (G_OBJECT (applet->prefs), "prefs-show-freq", show_freq);
	   g_object_set_data (G_OBJECT (applet->prefs), "prefs-show-unit", show_unit);
	   g_object_set_data (G_OBJECT (applet->prefs), "prefs-show-perc", show_perc);
	   
	   g_signal_connect (G_OBJECT (applet->prefs), "response",
					 G_CALLBACK (cpufreq_prefs_response_cb),
					 (gpointer) applet);
	   g_signal_connect (G_OBJECT (applet->prefs), "destroy",
					 G_CALLBACK (gtk_widget_destroy),
					 NULL);
	   g_signal_connect (G_OBJECT (show_freq), "toggled",
					 G_CALLBACK (cpufreq_prefs_show_freq_toggled),
					 (gpointer) applet);
	   g_signal_connect (G_OBJECT (show_unit), "toggled",
					 G_CALLBACK (cpufreq_prefs_show_unit_toggled),
					 (gpointer) applet);
	   g_signal_connect (G_OBJECT (show_perc), "toggled",
					 G_CALLBACK (cpufreq_prefs_show_perc_toggled),
					 (gpointer) applet);
	   g_signal_connect (G_OBJECT (cpu_number), "changed",
					 G_CALLBACK (cpufreq_prefs_cpu_number_changed),
					 (gpointer) applet);
	   g_signal_connect (G_OBJECT (show_mode), "changed",
					 G_CALLBACK (cpufreq_prefs_show_mode_changed),
					 (gpointer) applet);

	   gtk_widget_show (applet->prefs);
}
