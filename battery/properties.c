/*
 * File: applets/battery/properties.c
 *
 * This file contains the code which generates the
 * applet properties configuration window.
 *
 * Author: Nat Friedman <ndf@mit.edu>
 */

#include <stdio.h>
#include <gnome.h>
#include "battery.h"
#include "properties.h"

static int prop_cancel (GtkWidget * w, gpointer data);
static void prop_apply (GtkWidget *w, int page, gpointer data);

/* Create the properties window */
void
battery_properties_window(AppletWidget * applet, gpointer data)
{
  static GnomeHelpMenuEntry help_entry = { NULL, "properties" };
  BatteryData * bat = data; 
  GtkWidget * t, * l, * r2;
  GtkWidget * height_spin, * width_spin, * graph_speed_spin;
  int r, g, b;

  help_entry.name = gnome_app_id;

  if (bat->prop_win)
    return; 
	
  bat->prop_win = gnome_property_box_new ();

  gtk_window_set_title (
	GTK_WINDOW(&GNOME_PROPERTY_BOX(bat->prop_win)->dialog.window),
	_("Battery Monitor Settings"));
  
  /*
   *
   * General Properties
   *
   */
  t = gtk_table_new (0, 0, 0);
  gnome_property_box_append_page (GNOME_PROPERTY_BOX (bat->prop_win), t,
				  gtk_label_new (_("General Properties")));

  l = gtk_label_new(_("Applet Height")); 
  bat->height_adj = gtk_adjustment_new ( bat->height, 0.5, 256, 1, 8, 8 );
  height_spin = gtk_spin_button_new( GTK_ADJUSTMENT(bat->height_adj), 1, 0 );
  gtk_table_attach_defaults ( GTK_TABLE(t), l, 0, 1, 0, 1 );
  gtk_table_attach_defaults ( GTK_TABLE(t), height_spin, 1, 2, 0, 1 );
  gtk_spin_button_set_update_policy( GTK_SPIN_BUTTON(height_spin),
				     GTK_UPDATE_ALWAYS );
  gtk_signal_connect(GTK_OBJECT(bat->height_adj), "value_changed",
		     GTK_SIGNAL_FUNC(adj_value_changed_cb), bat);
  
  l = gtk_label_new(_("Applet Width")); 
  bat->width_adj = gtk_adjustment_new ( bat->width, 0.5, 666, 1, 8, 8 );
  width_spin = gtk_spin_button_new( GTK_ADJUSTMENT(bat->width_adj), 1, 0 );
  gtk_table_attach_defaults( GTK_TABLE(t), l, 0, 1, 2, 3 ); 
  gtk_table_attach_defaults( GTK_TABLE(t), width_spin, 1, 2, 2, 3 );
  gtk_spin_button_set_update_policy( GTK_SPIN_BUTTON(width_spin),
				     GTK_UPDATE_ALWAYS );
  gtk_signal_connect(GTK_OBJECT(bat->width_adj), "value_changed",
		     GTK_SIGNAL_FUNC(adj_value_changed_cb), bat);

  l = gtk_label_new(_("Applet Mode"));
  bat->mode_radio_graph = gtk_radio_button_new_with_label (NULL, _("Graph"));
  bat->mode_radio_readout = gtk_radio_button_new_with_label_from_widget
    (GTK_RADIO_BUTTON (bat->mode_radio_graph), _("Readout"));

  if (!strcmp(bat->mode_string, BATTERY_MODE_GRAPH))
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bat->mode_radio_graph), 1);
  else
    gtk_toggle_button_set_active
      (GTK_TOGGLE_BUTTON (bat->mode_radio_readout), 1);
    
  gtk_signal_connect (GTK_OBJECT (bat->mode_radio_graph), "toggled",
		      GTK_SIGNAL_FUNC (toggle_value_changed_cb), bat);
	
  gtk_table_attach ( GTK_TABLE(t), l,  3, 4, 0, 1, GTK_FILL | GTK_EXPAND,
		     GTK_EXPAND, 0, 0); 
  gtk_table_attach ( GTK_TABLE(t), bat->mode_radio_readout, 3, 4, 1, 2,
		     GTK_FILL | GTK_EXPAND, GTK_EXPAND, 0, 0); 
  gtk_table_attach ( GTK_TABLE(t), bat->mode_radio_graph, 3, 4, 2, 3,
		     GTK_FILL | GTK_EXPAND, GTK_EXPAND, 0, 0); 

  /*
   *
   * Graph Properties
   *
   */
  t = gtk_table_new (0, 0, 0);
  gnome_property_box_append_page (GNOME_PROPERTY_BOX (bat->prop_win), t,
				  gtk_label_new ("Graph Properties"));

  bat->graph_ac_on_color_sel = GNOME_COLOR_PICKER(gnome_color_picker_new());
  gtk_signal_connect(GTK_OBJECT(bat->graph_ac_on_color_sel), "color_set",
		     GTK_SIGNAL_FUNC(col_value_changed_cb), bat );

  bat->graph_ac_off_color_sel = GNOME_COLOR_PICKER(gnome_color_picker_new());
  gtk_signal_connect(GTK_OBJECT(bat->graph_ac_off_color_sel), "color_set",
		     GTK_SIGNAL_FUNC(col_value_changed_cb), bat );

  /* Initialize the selector colors */
  sscanf(bat->graph_color_ac_on_s, "#%02x%02x%02x", &r, &g, &b);
  gnome_color_picker_set_i8(bat->graph_ac_on_color_sel,
				      r, g, b, 255);

  sscanf(bat->graph_color_ac_off_s, "#%02x%02x%02x", &r, &g, &b);
  gnome_color_picker_set_i8(bat->graph_ac_off_color_sel,
				      r, g, b, 255);

  l = gtk_label_new (_("AC-On Battery Color:"));
  gtk_table_attach_defaults (GTK_TABLE (t), l, 0, 1, 0, 1);
  gtk_table_attach (GTK_TABLE(t),
	    GTK_WIDGET(bat->graph_ac_on_color_sel),
	    1, 2, 0, 1, GTK_EXPAND, 0, 0, 0);

  l = gtk_label_new(_("AC-Off Battery Color:"));
  gtk_table_attach (GTK_TABLE(t), l, 0, 1, 1, 2, 0, 0, 0, 0);
  gtk_table_attach (GTK_TABLE (t),
	    GTK_WIDGET(bat->graph_ac_off_color_sel),
	    1, 2, 1, 2, GTK_EXPAND, 0, 0, 0);

  l = gtk_label_new(_("Graph Interval:"));
  bat->graph_speed_adj = gtk_adjustment_new ( bat->graph_interval, 1, 666, 1,
					      8, 8 );
  graph_speed_spin = gtk_spin_button_new( GTK_ADJUSTMENT(bat->graph_speed_adj),
					  1, 0 );
  gtk_signal_connect(GTK_OBJECT(bat->graph_speed_adj), "value_changed",
		     GTK_SIGNAL_FUNC(adj_value_changed_cb), bat);
  gtk_spin_button_set_update_policy( GTK_SPIN_BUTTON(graph_speed_spin),
				     GTK_UPDATE_ALWAYS );
  
		     
  gtk_table_attach_defaults ( GTK_TABLE(t), l, 0, 1, 2, 3 );
  gtk_table_attach_defaults ( GTK_TABLE(t), graph_speed_spin, 1, 2, 2, 3 );
  
  l = gtk_label_new(_("Graph Direction:"));
  gtk_table_attach_defaults ( GTK_TABLE(t), l, 0, 1, 3, 4 );

  bat->dir_radio = gtk_radio_button_new_with_label(NULL, _("Left to Right"));
  r2 = gtk_radio_button_new_with_label_from_widget 
		    (GTK_RADIO_BUTTON(bat->dir_radio), _("Right to Left"));

  gtk_signal_connect (GTK_OBJECT (bat->dir_radio), "toggled",
		      GTK_SIGNAL_FUNC (toggle_value_changed_cb), bat);

  gtk_table_attach_defaults ( GTK_TABLE(t), bat->dir_radio, 1, 2, 3, 4);
  gtk_table_attach_defaults ( GTK_TABLE(t), r2, 2, 3, 3, 4);

  /*
   *
   * Readout Properties
   *
   */
  t = gtk_table_new (0, 0, 0);
  gnome_property_box_append_page (GNOME_PROPERTY_BOX (bat->prop_win), t,
				  gtk_label_new ("Readout Properties"));

  bat->readout_ac_on_color_sel = GNOME_COLOR_PICKER(gnome_color_picker_new());
  gtk_signal_connect(GTK_OBJECT(bat->readout_ac_on_color_sel), "color_set",
		     GTK_SIGNAL_FUNC(col_value_changed_cb), bat );

  bat->readout_ac_off_color_sel = GNOME_COLOR_PICKER(gnome_color_picker_new());
  gtk_signal_connect(GTK_OBJECT(bat->readout_ac_off_color_sel), "color_set",
		     GTK_SIGNAL_FUNC(col_value_changed_cb), bat );

  /* Initialize the selector colors */
  sscanf(bat->readout_color_ac_on_s, "#%02x%02x%02x", &r, &g, &b);
  gnome_color_picker_set_i8( bat->readout_ac_on_color_sel,
				      r, g, b, 255);

  sscanf(bat->readout_color_ac_off_s, "#%02x%02x%02x", &r, &g, &b);
  gnome_color_picker_set_i8( bat->readout_ac_off_color_sel,
				      r, g, b, 255);

  l = gtk_label_new (_("AC-On Battery Color:"));
  gtk_table_attach_defaults (GTK_TABLE (t), l, 0, 1, 0, 1);
  gtk_table_attach (GTK_TABLE(t),
	    GTK_WIDGET(bat->readout_ac_on_color_sel),
	    1, 2, 0, 1, GTK_EXPAND, 0, 0, 0);

  l = gtk_label_new(_("AC-Off Battery Color:"));
  gtk_table_attach (GTK_TABLE(t), l, 0, 1, 1, 2, 0, 0, 0, 0);
  gtk_table_attach (GTK_TABLE (t),
	    GTK_WIDGET(bat->readout_ac_off_color_sel),
	    1, 2, 1, 2, GTK_EXPAND, 0, 0, 0);

  gtk_signal_connect (GTK_OBJECT (bat->prop_win), "destroy",
		      GTK_SIGNAL_FUNC (prop_cancel), bat);
	
  gtk_signal_connect (GTK_OBJECT (bat->prop_win), "delete_event",
		      GTK_SIGNAL_FUNC (prop_cancel), bat);
	
  gtk_signal_connect (GTK_OBJECT (bat->prop_win), "apply",
		      GTK_SIGNAL_FUNC (prop_apply), bat);

  gtk_signal_connect (GTK_OBJECT (bat->prop_win), "help",
		      GTK_SIGNAL_FUNC (gnome_help_pbox_display),
		      &help_entry);
	
  gtk_widget_show_all (bat->prop_win);

} /* battery_properties_window */


/*
 *
 * Properties window button callbacks (apply, cancel, etc)
 *
 */
static int
prop_cancel (GtkWidget * w, gpointer data)
{
  BatteryData * bat = data;

  bat->prop_win = 0;

  return FALSE;
} /* prop_cancel */

static void
prop_apply (GtkWidget *w, int page, gpointer data)
{
  BatteryData * bat = data;
  int width, height, size_changed = 0;
  guint8 r, g, b;

  /* Update the running session from the properties.  The session
     state will be saved when the applet exits and the panel tells it
     to save state. */
  height = GTK_ADJUSTMENT(bat->height_adj)->value;
  width = GTK_ADJUSTMENT(bat->width_adj)->value;
  if (height != bat->height ||  width != bat->width)
    {
      size_changed = 1;
      bat->height = height;
      bat->width = width;
    }

  bat->graph_interval = GTK_ADJUSTMENT(bat->graph_speed_adj)->value;
  gtk_timeout_remove (bat->graph_timeout_id);
  bat->graph_timeout_id = gtk_timeout_add(1000 * bat->graph_interval,
					  (GtkFunction) battery_update, bat);
  /* FIXME: set the direction! */

  gnome_color_picker_get_i8 ( bat->graph_ac_on_color_sel,
				       &r, &g, &b, NULL);
  snprintf(bat->graph_color_ac_on_s, sizeof(bat->graph_color_ac_on_s),
	   "#%02x%02x%02x", r, g, b);

  gnome_color_picker_get_i8 ( bat->graph_ac_off_color_sel,
				       &r, &g, &b, NULL);
  snprintf(bat->graph_color_ac_off_s, sizeof(bat->graph_color_ac_off_s),
	  "#%02x%02x%02x", r, g, b);

  gnome_color_picker_get_i8 ( bat->readout_ac_off_color_sel,
				       &r, &g, &b, NULL);
  snprintf(bat->readout_color_ac_off_s, sizeof(bat->readout_color_ac_off_s),
	  "#%02x%02x%02x", r, g, b);

  gnome_color_picker_get_i8 ( bat->readout_ac_on_color_sel,
				       &r, &g, &b, NULL);
  snprintf(bat->readout_color_ac_on_s, sizeof(bat->readout_color_ac_on_s),
	  "#%02x%02x%02x", r, g, b);

  battery_setup_colors(bat);

  if (size_changed)
    battery_set_size(bat);

  bat->mode_string = GTK_TOGGLE_BUTTON(bat->mode_radio_graph)->active ?
      BATTERY_MODE_GRAPH : BATTERY_MODE_READOUT;
      
  battery_set_mode(bat);

  bat->force_update = TRUE;
  battery_update((gpointer) bat);
  
  /*make the panel save our config*/
  applet_widget_sync_config(APPLET_WIDGET(bat->applet));

} /* prop_apply */

/*
 *
 * Property element callbacks (whenever a property is changed one of
 * these is called)
 *
 */

void
adj_value_changed_cb( GtkAdjustment * ignored, gpointer data )
{
  BatteryData * bat = data;

  gnome_property_box_changed (GNOME_PROPERTY_BOX (bat->prop_win)); 
} /* value_changed_cb */

void
toggle_value_changed_cb( GtkToggleButton * ignored, gpointer data )
{
  BatteryData * bat = data;

  gnome_property_box_changed (GNOME_PROPERTY_BOX (bat->prop_win)); 
} /* value_changed_cb */

void
col_value_changed_cb( GtkObject * ignored, guint arg1, guint arg2,
		      guint arg3, guint arg4, gpointer data )
{
  BatteryData * bat = data;

  gnome_property_box_changed (GNOME_PROPERTY_BOX (bat->prop_win)); 
} /* value_changed_cb */

