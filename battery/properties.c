/*--------------------------------*-C-*---------------------------------*
 *
 *  Copyright 1999, Nat Friedman <nat@nat.org>.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Street #330, Boston, MA 02111-1307, USA.
 *
 *
 *----------------------------------------------------------------------*/

/*
 * File: applets/battery/properties.c
 *
 * This file contains the code which generates the
 * applet properties configuration window.
 *
 * Author: Nat Friedman <nat@nat.org>
 */

#include <config.h>
#include <stdio.h>
#include <gnome.h>
#include "battery.h"
#include "properties.h"

static int prop_cancel (GtkWidget * w, gpointer data);
static void prop_apply (GtkWidget *w, int page, gpointer data);

static void
phelp_cb (GtkWidget *w, gint tab, gpointer data)
{
	GnomeHelpMenuEntry help_entry = { 
		"battery_applet", "index.html#BATTERY-CHARGE-MONITOR-PROPERTIES"
	};
	gnome_help_display (NULL, &help_entry);
}

/* Create the properties window */
void
battery_properties_window (AppletWidget * applet, gpointer data)
{
  BatteryData * bat = data; 
  GtkWidget * t, * l, * r2;
  GtkWidget * height_spin, * width_spin, * interval_spin, * low_charge_spin,
    * low_warn_spin;
  int r, g, b;

  if (bat->prop_win)
    {
      gtk_widget_show (GTK_WIDGET (bat->prop_win));
      gdk_window_raise (GTK_WIDGET (bat->prop_win)->window);

      return;
    }
	
  bat->prop_win = GNOME_PROPERTY_BOX (gnome_property_box_new ());

  gnome_dialog_close_hides
    (GNOME_DIALOG (& (bat->prop_win->dialog)), TRUE);

  gtk_window_set_title (
	GTK_WINDOW (&GNOME_PROPERTY_BOX (bat->prop_win)->dialog.window),
	_("Battery Monitor Settings"));
  
  /*
   *
   * General Properties
   *
   */
  t = gtk_table_new (0, 0, FALSE);
  gnome_property_box_append_page (GNOME_PROPERTY_BOX (bat->prop_win), t,
				  gtk_label_new (_("General")));

  bat->follow_toggle = gtk_check_button_new_with_label (_("Follow panel size"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bat->follow_toggle), 
				bat->follow_panel_size);
  gtk_signal_connect (GTK_OBJECT (bat->follow_toggle), "toggled",
		      GTK_SIGNAL_FUNC (toggle_value_changed_cb), bat);
  gtk_table_attach_defaults ( GTK_TABLE (t), bat->follow_toggle, 0, 2, 0, 1);

  /* Applet height */
  l = gtk_label_new (_("Applet Height:")); 
  bat->height_adj = gtk_adjustment_new ( bat->height, 1.0, 666, 1, 8, 8 );
  height_spin = gtk_spin_button_new ( GTK_ADJUSTMENT (bat->height_adj), 1, 0 );
  gtk_table_attach_defaults ( GTK_TABLE (t), l, 0, 1, 1, 2 );
  gtk_table_attach_defaults ( GTK_TABLE (t), height_spin, 1, 2, 1, 2 );
  gtk_spin_button_set_update_policy ( GTK_SPIN_BUTTON (height_spin),
				     GTK_UPDATE_ALWAYS );
  gtk_signal_connect (GTK_OBJECT (bat->height_adj), "value_changed",
		     GTK_SIGNAL_FUNC (adj_value_changed_cb), bat);

  /* Applet width */
  l = gtk_label_new (_("Applet Width:")); 
  gtk_table_attach_defaults ( GTK_TABLE (t), l, 0, 1, 2, 3 ); 

  bat->width_adj = gtk_adjustment_new ( bat->width, 1, 666, 1, 8, 8 );
  width_spin = gtk_spin_button_new ( GTK_ADJUSTMENT (bat->width_adj), 1, 0 );
  gtk_table_attach_defaults ( GTK_TABLE (t), width_spin, 1, 2, 2, 3 );
  gtk_spin_button_set_update_policy ( GTK_SPIN_BUTTON (width_spin),
				     GTK_UPDATE_ALWAYS );
  gtk_signal_connect (GTK_OBJECT (bat->width_adj), "value_changed",
		     GTK_SIGNAL_FUNC (adj_value_changed_cb), bat);

  /* Update interval */
  l = gtk_label_new (_("Update Interval (seconds):"));
  bat->interval_adj = gtk_adjustment_new (bat->update_interval,
					     1, 666, 1, 8, 8);
  interval_spin =
    gtk_spin_button_new (GTK_ADJUSTMENT (bat->interval_adj), 1, 0);
  gtk_signal_connect (GTK_OBJECT (bat->interval_adj), "value_changed",
		     GTK_SIGNAL_FUNC (adj_value_changed_cb), bat);
  gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (interval_spin),
				     GTK_UPDATE_ALWAYS);
  
  gtk_table_attach_defaults (GTK_TABLE (t), l, 0, 1, 3, 4);
  gtk_table_attach_defaults (GTK_TABLE (t), interval_spin, 1, 2, 3, 4);

  /* Low battery value */
  l = gtk_label_new (_("Low Charge Threshold:"));
  bat->low_charge_adj = gtk_adjustment_new (bat->low_charge_val,
					    0, 100, 1, 8, 8);
  low_charge_spin =
    gtk_spin_button_new (GTK_ADJUSTMENT (bat->low_charge_adj), 1, 0);
  gtk_signal_connect (GTK_OBJECT (bat->low_charge_adj), "value_changed",
		     GTK_SIGNAL_FUNC (adj_value_changed_cb), bat);
  gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (low_charge_spin),
				     GTK_UPDATE_ALWAYS);
  
  gtk_table_attach_defaults (GTK_TABLE (t), l, 0, 1, 4, 5);
  gtk_table_attach_defaults (GTK_TABLE (t), low_charge_spin, 1, 2, 4, 5);

  /* Applet mode label */
  l = gtk_label_new (_("Applet Mode:"));
  bat->mode_radio_graph = gtk_radio_button_new_with_label (NULL, _("Graph"));
  bat->mode_radio_readout = gtk_radio_button_new_with_label_from_widget
    (GTK_RADIO_BUTTON (bat->mode_radio_graph), _("Readout"));

  if (!strcmp (bat->mode_string, BATTERY_MODE_GRAPH))
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bat->mode_radio_graph),
				  1);
  else
    gtk_toggle_button_set_active
      (GTK_TOGGLE_BUTTON (bat->mode_radio_readout), 1);
    
  gtk_signal_connect (GTK_OBJECT (bat->mode_radio_graph), "toggled",
		      GTK_SIGNAL_FUNC (toggle_value_changed_cb), bat);
	
  gtk_table_attach ( GTK_TABLE (t), l,  4, 5, 1, 2, GTK_FILL | GTK_EXPAND,
		     GTK_EXPAND, 10, 0); 
  gtk_table_attach ( GTK_TABLE (t), bat->mode_radio_readout, 4, 5, 2, 3,
		     GTK_FILL | GTK_EXPAND, GTK_EXPAND, 10, 0); 
  gtk_table_attach ( GTK_TABLE (t), bat->mode_radio_graph, 4, 5, 3, 4,
		     GTK_FILL | GTK_EXPAND, GTK_EXPAND, 10, 0); 

  /*
   *
   * Readout Properties
   *
   */
  t = gtk_table_new (0, 0, FALSE);
  gnome_property_box_append_page (GNOME_PROPERTY_BOX (bat->prop_win), t,
				  gtk_label_new (_("Readout")));

  bat->readout_ac_on_color_sel =
    GNOME_COLOR_PICKER (gnome_color_picker_new ());
  bat->readout_ac_off_color_sel =
    GNOME_COLOR_PICKER (gnome_color_picker_new ());
  bat->readout_low_color_sel =
    GNOME_COLOR_PICKER (gnome_color_picker_new ());

  gtk_signal_connect (GTK_OBJECT (bat->readout_ac_on_color_sel), "color_set",
		     GTK_SIGNAL_FUNC (col_value_changed_cb), bat);

  gtk_signal_connect (GTK_OBJECT (bat->readout_ac_off_color_sel), "color_set",
		     GTK_SIGNAL_FUNC (col_value_changed_cb), bat);

  gtk_signal_connect (GTK_OBJECT (bat->readout_low_color_sel), "color_set",
		     GTK_SIGNAL_FUNC (col_value_changed_cb), bat);

  /* Initialize the selector colors */
  sscanf (bat->readout_color_ac_on_s, "#%02x%02x%02x", &r, &g, &b);
  gnome_color_picker_set_i8 (bat->readout_ac_on_color_sel,
			     r, g, b, 255);

  sscanf (bat->readout_color_ac_off_s, "#%02x%02x%02x", &r, &g, &b);
  gnome_color_picker_set_i8 (bat->readout_ac_off_color_sel,
			     r, g, b, 255);

  sscanf (bat->readout_color_low_s, "#%02x%02x%02x", &r, &g, &b);
  gnome_color_picker_set_i8 (bat->readout_low_color_sel,
			     r, g, b, 255);

  l = gtk_label_new (_("AC-On Battery Color:"));
  gtk_table_attach (GTK_TABLE (t), l, 0, 1, 0, 1, GTK_EXPAND, 0, 0, 0);
  gtk_table_attach (GTK_TABLE (t),
	    GTK_WIDGET (bat->readout_ac_on_color_sel),
	    1, 2, 0, 1, GTK_EXPAND, 0, 0, 0);

  l = gtk_label_new (_("AC-Off Battery Color:"));
  gtk_table_attach (GTK_TABLE (t), l, 0, 1, 1, 2, 0, 0, 0, 0);
  gtk_table_attach (GTK_TABLE (t),
	    GTK_WIDGET (bat->readout_ac_off_color_sel),
	    1, 2, 1, 2, GTK_EXPAND, 0, 0, 0);

  l = gtk_label_new (_("Low Battery Color:"));
  gtk_table_attach (GTK_TABLE (t), l, 0, 1, 2, 3, 0, 0, 0, 0);
  gtk_table_attach (GTK_TABLE (t),
	    GTK_WIDGET (bat->readout_low_color_sel),
	    1, 2, 2, 3, GTK_EXPAND, 0, 0, 0);

  /*
   *
   * Graph Properties
   *
   */
  t = gtk_table_new (0, 0, FALSE);
  gnome_property_box_append_page (GNOME_PROPERTY_BOX (bat->prop_win), t,
				  gtk_label_new (_("Graph")));

  bat->graph_ac_on_color_sel = GNOME_COLOR_PICKER (gnome_color_picker_new ());
  gtk_signal_connect (GTK_OBJECT (bat->graph_ac_on_color_sel), "color_set",
		     GTK_SIGNAL_FUNC (col_value_changed_cb), bat );

  bat->graph_ac_off_color_sel = GNOME_COLOR_PICKER (gnome_color_picker_new ());
  gtk_signal_connect (GTK_OBJECT (bat->graph_ac_off_color_sel), "color_set",
		     GTK_SIGNAL_FUNC (col_value_changed_cb), bat );

  bat->graph_low_color_sel = GNOME_COLOR_PICKER (gnome_color_picker_new ());
  gtk_signal_connect (GTK_OBJECT (bat->graph_low_color_sel), "color_set",
		     GTK_SIGNAL_FUNC (col_value_changed_cb), bat );

  bat->graph_line_color_sel = GNOME_COLOR_PICKER (gnome_color_picker_new ());
  gtk_signal_connect (GTK_OBJECT (bat->graph_line_color_sel), "color_set",
		     GTK_SIGNAL_FUNC (col_value_changed_cb), bat );

  /* Initialize the selector colors */
  sscanf (bat->graph_color_ac_on_s, "#%02x%02x%02x", &r, &g, &b);
  gnome_color_picker_set_i8 (bat->graph_ac_on_color_sel,
				      r, g, b, 255);

  sscanf (bat->graph_color_ac_off_s, "#%02x%02x%02x", &r, &g, &b);
  gnome_color_picker_set_i8 (bat->graph_ac_off_color_sel,
				      r, g, b, 255);

  sscanf (bat->graph_color_low_s, "#%02x%02x%02x", &r, &g, &b);
  gnome_color_picker_set_i8 (bat->graph_low_color_sel,
				      r, g, b, 255);

  sscanf (bat->graph_color_line_s, "#%02x%02x%02x", &r, &g, &b);
  gnome_color_picker_set_i8 (bat->graph_line_color_sel,
				      r, g, b, 255);

  l = gtk_label_new (_("AC-On Battery Color:"));
  gtk_table_attach_defaults (GTK_TABLE (t), l, 0, 1, 0, 1);
  gtk_table_attach (GTK_TABLE (t),
	    GTK_WIDGET (bat->graph_ac_on_color_sel),
	    1, 2, 0, 1, GTK_EXPAND, 0, 0, 0);

  l = gtk_label_new (_("AC-Off Battery Color:"));
  gtk_table_attach (GTK_TABLE (t), l, 0, 1, 1, 2, 0, 0, 0, 0);
  gtk_table_attach (GTK_TABLE (t),
	    GTK_WIDGET (bat->graph_ac_off_color_sel),
	    1, 2, 1, 2, GTK_EXPAND, 0, 0, 0);

  l = gtk_label_new (_("Graph Battery Low Color:"));
  gtk_table_attach (GTK_TABLE (t), l, 0, 1, 2, 3, 0, 0, 0, 0);
  gtk_table_attach (GTK_TABLE (t),
	    GTK_WIDGET (bat->graph_low_color_sel),
	    1, 2, 2, 3, GTK_EXPAND, 0, 0, 0);

  l = gtk_label_new (_("Graph Tick Color:"));
  gtk_table_attach (GTK_TABLE (t), l, 0, 1, 3, 4, 0, 0, 0, 0);
  gtk_table_attach (GTK_TABLE (t),
	    GTK_WIDGET (bat->graph_line_color_sel),
	    1, 2, 3, 4, GTK_EXPAND, 0, 0, 0);

 
  l = gtk_label_new (_("Graph Direction:"));
  gtk_table_attach_defaults ( GTK_TABLE (t), l, 0, 1, 5, 6 );

  bat->dir_radio = gtk_radio_button_new_with_label (NULL, _("Left to Right"));
  r2 = gtk_radio_button_new_with_label_from_widget 
		    (GTK_RADIO_BUTTON (bat->dir_radio), _("Right to Left"));

  if (bat->graph_direction == BATTERY_GRAPH_LEFT_TO_RIGHT)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bat->dir_radio), 1);
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (r2), 1);

  gtk_signal_connect (GTK_OBJECT (bat->dir_radio), "toggled",
		      GTK_SIGNAL_FUNC (toggle_value_changed_cb), bat);

  gtk_table_attach_defaults ( GTK_TABLE (t), bat->dir_radio, 1, 2, 5, 6);
  gtk_table_attach_defaults ( GTK_TABLE (t), r2, 2, 3, 5, 6);

  /*
   *
   * Low Battery Warning / Battery Full Notification
   *
   */
  t = gtk_table_new (0, 0, FALSE);
  gnome_property_box_append_page (GNOME_PROPERTY_BOX (bat->prop_win), t,
			  gtk_label_new (_("Battery Charge Messages")));

  bat->warn_check = gtk_check_button_new_with_label
    (_("Enable Low Battery Warning"));

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bat->warn_check),
				bat->low_warn_enable ? 1 : 0);

  l = gtk_label_new (_("Warn if the battery charge dips below:"));
  bat->low_warn_adj = gtk_adjustment_new (bat->low_warn_val,
					  0, 100, 1, 8, 8);
  low_warn_spin =
    gtk_spin_button_new (GTK_ADJUSTMENT (bat->low_warn_adj), 1, 0);
  gtk_signal_connect (GTK_OBJECT (bat->low_warn_adj), "value_changed",
		     GTK_SIGNAL_FUNC (adj_value_changed_cb), bat);
  gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (low_warn_spin),
				     GTK_UPDATE_ALWAYS);
  
  gtk_signal_connect (GTK_OBJECT (bat->warn_check), "toggled",
		      GTK_SIGNAL_FUNC (toggle_value_changed_cb), bat);

  bat->full_notify_check = gtk_check_button_new_with_label
    (_("Enable Full-Charge Notification"));

  gtk_signal_connect (GTK_OBJECT (bat->full_notify_check), "toggled",
		      GTK_SIGNAL_FUNC (toggle_value_changed_cb), bat);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bat->full_notify_check),
				bat->full_notify_enable ? 1 : 0);

  gtk_table_attach_defaults (GTK_TABLE (t), l, 0, 2, 0, 1);
  gtk_table_attach_defaults (GTK_TABLE (t), low_warn_spin, 2, 3, 0, 1);
  gtk_table_attach_defaults (GTK_TABLE (t), bat->warn_check, 1, 2, 1, 2);
  gtk_table_attach_defaults (GTK_TABLE (t), bat->full_notify_check,
			     1, 2, 2, 3);


  gtk_signal_connect (GTK_OBJECT (bat->prop_win), "destroy",
		      GTK_SIGNAL_FUNC (prop_cancel), bat);
	
  gtk_signal_connect (GTK_OBJECT (bat->prop_win), "delete_event",
		      GTK_SIGNAL_FUNC (prop_cancel), bat);
	
  gtk_signal_connect (GTK_OBJECT (bat->prop_win), "apply",
		      GTK_SIGNAL_FUNC (prop_apply), bat);

  gtk_signal_connect (GTK_OBJECT (bat->prop_win), "help",
		      GTK_SIGNAL_FUNC (phelp_cb), NULL);
	
  gtk_widget_show_all (GTK_WIDGET (bat->prop_win));

  return;
} /* battery_properties_window */


/*
 *
 * Properties window button callbacks (apply, cancel, etc)
 *
 */
static int
prop_cancel (GtkWidget * w, gpointer data)
{
  /* BatteryData * bat = data; */

  return FALSE;
} /* prop_cancel */

static void
prop_apply (GtkWidget *w, int page, gpointer data)
{
  BatteryData * bat = data;
  int width, height, size_changed = FALSE;
  guint8 r, g, b;
  gint new_direction;

  /*
   * Update the running session from the properties.  The session
   * state will be saved when the applet exits and the panel tells it
   * to save state.
   */

  /*
   * Don't let the update function run while we're changing these
   * values.
   */
  bat->setup = FALSE;

  bat->update_interval = GTK_ADJUSTMENT (bat->interval_adj)->value;
  gtk_timeout_remove (bat->update_timeout_id);
  bat->update_timeout_id = gtk_timeout_add (1000 * bat->update_interval,
					   (GtkFunction) battery_update, bat);

  new_direction = (GTK_TOGGLE_BUTTON (bat->dir_radio)->active) ?
    BATTERY_GRAPH_LEFT_TO_RIGHT : BATTERY_GRAPH_RIGHT_TO_LEFT;

  bat->low_charge_val = GTK_ADJUSTMENT (bat->low_charge_adj)->value;
  bat->low_warn_val = GTK_ADJUSTMENT (bat->low_warn_adj)->value;
  bat->low_warn_enable =
    GTK_TOGGLE_BUTTON (bat->warn_check)->active;
  bat->full_notify_enable =
    GTK_TOGGLE_BUTTON (bat->full_notify_check)->active;

  /*
   * If the direction changed, reverse the graph.
   */
  if (bat->graph_direction != new_direction)
    {
      unsigned char *new_graph;
      int i;

      new_graph = (unsigned char *)
	g_malloc (sizeof (unsigned char) * bat->width);

      for (i = 0; i < bat->width; i ++)
	new_graph [i] = bat->graph_values [ (bat->width - 1) - i];

      memcpy (bat->graph_values, new_graph,
	      sizeof (unsigned char) * bat->width);

      g_free (new_graph);

      bat->graph_direction = new_direction;
    }

  /*
   * Update the size
   */

  bat->follow_panel_size = GTK_TOGGLE_BUTTON (bat->follow_toggle)->active;

  if (!bat->follow_panel_size)
    {
      height = GTK_ADJUSTMENT (bat->height_adj)->value;
      width = GTK_ADJUSTMENT (bat->width_adj)->value;

      if ( (height != bat->height) || (width != bat->width) )
        {
          size_changed = TRUE;
          bat->height = height;
          bat->width = width;
        }
    }

  gnome_color_picker_get_i8 (bat->graph_ac_on_color_sel,
			     &r, &g, &b, NULL);
  g_snprintf (bat->graph_color_ac_on_s, sizeof (bat->graph_color_ac_on_s),
	   "#%02x%02x%02x", r, g, b);

  gnome_color_picker_get_i8 ( bat->graph_ac_off_color_sel,
				       &r, &g, &b, NULL);
  g_snprintf (bat->graph_color_ac_off_s, sizeof (bat->graph_color_ac_off_s),
	  "#%02x%02x%02x", r, g, b);

  gnome_color_picker_get_i8 ( bat->graph_low_color_sel,
				       &r, &g, &b, NULL);
  g_snprintf (bat->graph_color_low_s, sizeof (bat->graph_color_low_s),
	  "#%02x%02x%02x", r, g, b);

  gnome_color_picker_get_i8 (bat->graph_line_color_sel,
			     &r, &g, &b, NULL);
  g_snprintf (bat->graph_color_line_s, sizeof (bat->graph_color_line_s),
	   "#%02x%02x%02x", r, g, b);

  gnome_color_picker_get_i8 ( bat->readout_ac_off_color_sel,
				       &r, &g, &b, NULL);
  g_snprintf (bat->readout_color_ac_off_s, sizeof (bat->readout_color_ac_off_s),
	  "#%02x%02x%02x", r, g, b);

  gnome_color_picker_get_i8 ( bat->readout_ac_on_color_sel,
				       &r, &g, &b, NULL);
  g_snprintf (bat->readout_color_ac_on_s, sizeof (bat->readout_color_ac_on_s),
	  "#%02x%02x%02x", r, g, b);

  gnome_color_picker_get_i8 ( bat->readout_low_color_sel,
				       &r, &g, &b, NULL);
  g_snprintf (bat->readout_color_low_s, sizeof (bat->readout_color_low_s),
	  "#%02x%02x%02x", r, g, b);

  g_free (bat->mode_string);
  bat->mode_string =
    g_strdup (GTK_TOGGLE_BUTTON (bat->mode_radio_graph)->active ?
	      BATTERY_MODE_GRAPH : BATTERY_MODE_READOUT);

  if (bat->follow_panel_size)
    battery_set_follow_size (bat);
  else if (size_changed)
    battery_set_size (bat);

  bat->setup = TRUE;

  battery_setup_colors (bat);

  battery_set_mode (bat);

  bat->force_update = TRUE;
  battery_update ((gpointer) bat);
  
  /* Make the panel save our config */
  applet_widget_sync_config (APPLET_WIDGET (bat->applet));
  return;
} /* prop_apply */

/*
 *
 * Property element callbacks (whenever a property is changed one of
 * these is called)
 *
 */

void
adj_value_changed_cb ( GtkAdjustment * ignored, gpointer data )
{
  BatteryData * bat = data;

  gnome_property_box_changed (GNOME_PROPERTY_BOX (bat->prop_win)); 
  return;
} /* value_changed_cb */

void
toggle_value_changed_cb ( GtkToggleButton * ignored, gpointer data )
{
  BatteryData * bat = data;

  gnome_property_box_changed (GNOME_PROPERTY_BOX (bat->prop_win)); 
  return;
} /* value_changed_cb */

void
col_value_changed_cb ( GtkObject * ignored, guint arg1, guint arg2,
		      guint arg3, guint arg4, gpointer data )
{
  BatteryData * bat = data;

  gnome_property_box_changed (GNOME_PROPERTY_BOX (bat->prop_win)); 
  return;
} /* value_changed_cb */
