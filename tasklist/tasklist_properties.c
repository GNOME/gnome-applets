#include <gnome.h>
#include "tasklist_applet.h"

extern Config config;

GtkWidget *dialog;
Config temp_config;

/* Prototypes */
void properties_cb_apply (GtkWidget *widget, gint page, gpointer data);
GtkWidget* properties_create_tasks_page (void);
GtkWidget* properties_create_general_page (void);
void properties_add_checkbutton (GtkWidget *box, gchar *name, gboolean config_value);
GtkWidget* properties_create_geometry_page (void);
GtkWidget* properties_add_spin_button (gint def, gint min, gint max, 
				       gint step, gint page);
gboolean properties_cb_checkbutton_disable (GtkWidget *widget, gboolean data);
gboolean properties_cb_checkbutton (GtkWidget *widget, gboolean *data);

gboolean properties_cb_checkbutton_disable (GtkWidget *widget, gboolean data)
{
  GtkWidget *enabled;
  
  enabled = (GtkWidget *)data;

  if (GTK_TOGGLE_BUTTON (widget)->active)
    gtk_widget_set_sensitive (enabled, FALSE);
  else
    gtk_widget_set_sensitive (enabled, TRUE);

  gnome_property_box_changed (GNOME_PROPERTY_BOX (dialog));
  
  return FALSE;
}

gboolean properties_cb_checkbutton (GtkWidget *widget, gboolean *data)
{
  
  if (GTK_TOGGLE_BUTTON (widget)->active)
    *data = TRUE;
  else
    *data = FALSE;
  
  gnome_property_box_changed (GNOME_PROPERTY_BOX (dialog));

  return FALSE;
}

void properties_cb_apply (GtkWidget *widget, gint page, gpointer data)
{
  config = temp_config;

  tasklist_layout ();
}

void properties_add_checkbutton (GtkWidget *box, gchar *name, gboolean config_value)
{
  GtkWidget *checkbutton;
  checkbutton = gtk_check_button_new_with_label(name);
  
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config_value);
  gtk_signal_connect (GTK_OBJECT (checkbutton), "toggled",
		      GTK_SIGNAL_FUNC (properties_cb_checkbutton), &config_value);
  gtk_box_pack_start_defaults (GTK_BOX (box), checkbutton);
  
}


GtkWidget* properties_add_spin_button (gint def, gint min, gint max, 
				       gint step, gint page)
{
  GtkWidget *spinbutton;
  GtkAdjustment *adj;
  
  adj = gtk_adjustment_new (def, min, max, step, page, page);
  spinbutton = gtk_spin_button_new (adj, 1, 0);

  return spinbutton;
}

GtkWidget* properties_create_geometry_page (void)
{
  GtkWidget *hbox;
  GtkWidget *frame;
  GtkWidget *table;
  GtkWidget *checkbutton;
  GtkWidget *label;
  GtkWidget *spinbutton;

  hbox = gtk_hbox_new (FALSE, GNOME_PAD_SMALL);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), GNOME_PAD_SMALL);
  
  frame = gtk_frame_new ("Horizontal");
  gtk_box_pack_start_defaults (GTK_BOX (hbox), frame);

  table = gtk_table_new (3, 2, FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table);
  
  label = gtk_label_new ("Tasklist width");
  gtk_table_attach_defaults (GTK_TABLE (table), label,
			     0, 1,
			     0, 1);
  gtk_table_attach_defaults (GTK_TABLE (table),
			     properties_add_spin_button (350, 20, 1024, 1, 1),
			     1, 2,
			     0, 1);

  checkbutton = gtk_check_button_new_with_label ("Number of rows follows panel height");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), config.numrows_follows_panel);
  gtk_signal_connect (GTK_OBJECT (checkbutton), "toggled",
		      GTK_SIGNAL_FUNC (properties_cb_checkbutton), &config.numrows_follows_panel);
  gtk_table_attach_defaults (GTK_TABLE (table), checkbutton,
			     0, 2,
			     1, 2);

  label = gtk_label_new ("Number of rows");
  gtk_table_attach_defaults (GTK_TABLE (table), label,
			     0, 1,
			     2, 3);
  
  spinbutton = properties_add_spin_button (2, 1, 8, 1, 1);
 
  gtk_table_attach_defaults (GTK_TABLE (table),
			     spinbutton,
			     1, 2,
			     2, 3);
  
  frame = gtk_frame_new ("Vertical");
  gtk_box_pack_start_defaults (GTK_BOX (hbox), frame);
 table = gtk_table_new (3, 2, FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table);
  
  label = gtk_label_new ("Tasklist height");
  gtk_table_attach_defaults (GTK_TABLE (table), label,
			     0, 1,
			     0, 1);
  gtk_table_attach_defaults (GTK_TABLE (table),
			     properties_add_spin_button (350, 20, 1024, 1, 1),
			     1, 2,
			     0, 1);

  checkbutton = gtk_check_button_new_with_label ("Number of columns follows panel width");
  
  gtk_table_attach_defaults (GTK_TABLE (table), checkbutton,
			     0, 2,
			     1, 2);

  label = gtk_label_new ("Number of columns");
  gtk_table_attach_defaults (GTK_TABLE (table), label,
			     0, 1,
			     2, 3);
  
  spinbutton = properties_add_spin_button (2, 1, 8, 1, 1);
  gtk_signal_connect (GTK_OBJECT (checkbutton), "toggled",
		      GTK_SIGNAL_FUNC (properties_cb_checkbutton_disable),
		      spinbutton);
  gtk_signal_connect (GTK_OBJECT (checkbutton), "toggled",
		      GTK_SIGNAL_FUNC (properties_cb_checkbutton_disable),
		      label);

  gtk_table_attach_defaults (GTK_TABLE (table),
			     spinbutton,
			     1, 2,
			     2, 3);
  gtk_widget_show_all (hbox);
  return hbox;
}

GtkWidget* properties_create_tasks_page (void)
{
  GtkWidget *radiobutton;
  GtkWidget *vbox;
  GtkWidget *frame;
  GtkWidget *smallvbox;

  vbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), GNOME_PAD_SMALL);

  frame = gtk_frame_new ("Which tasks to show");
  gtk_box_pack_start_defaults (GTK_BOX (vbox), frame);

  smallvbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (frame), smallvbox);

  radiobutton = gtk_radio_button_new_with_label (NULL, "Show all tasks");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radiobutton), temp_config.show_all);
  gtk_signal_connect (GTK_OBJECT (radiobutton), "toggled",
		      GTK_SIGNAL_FUNC (properties_cb_checkbutton), &temp_config.show_all);
  gtk_box_pack_start_defaults (GTK_BOX (smallvbox), radiobutton);

  radiobutton = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radiobutton),
							     "Show normal tasks only");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radiobutton), temp_config.show_normal);
  gtk_signal_connect (GTK_OBJECT (radiobutton), "toggled",
		      GTK_SIGNAL_FUNC (properties_cb_checkbutton), &temp_config.show_normal);
  gtk_box_pack_start_defaults (GTK_BOX (smallvbox), radiobutton);

  radiobutton = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radiobutton),
							     "Show minimized tasks only");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radiobutton), temp_config.show_minimized);
  gtk_signal_connect (GTK_OBJECT (radiobutton), "toggled",
		      GTK_SIGNAL_FUNC (properties_cb_checkbutton), &temp_config.show_minimized);
  gtk_box_pack_start_defaults (GTK_BOX (smallvbox), radiobutton);

  /*  checkbutton = gtk_check_button_new_with_label ("All tasks appear on all desktops");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), temp_config.all_tasks);
  gtk_signal_connect (GTK_OBJECT (checkbutton), "toggled",
		      GTK_SIGNAL_FUNC (properties_cb_checkbutton), &temp_config.all_tasks);
  gtk_box_pack_start_defaults (GTK_BOX (smallvbox), checkbutton);
  */
  properties_add_checkbutton (smallvbox, "All tasks appear on all desktops", temp_config.all_tasks);

  properties_add_checkbutton (smallvbox, "Minimized tasks appear on all window", temp_config.minimized_tasks);

  /*  checkbutton = gtk_check_button_new_with_label ("Minimized tasks appear on all windows");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), temp_config.minimized_tasks);
  gtk_signal_connect (GTK_OBJECT (checkbutton), "toggled",
		      GTK_SIGNAL_FUNC (properties_cb_checkbutton), &temp_config.minimized_tasks);
  gtk_box_pack_start_defaults (GTK_BOX (smallvbox), checkbutton);
  */
  gtk_widget_show_all (vbox);
  return vbox;
}


GtkWidget* properties_create_general_page (void)
{
  GtkWidget *vbox;
  GtkWidget *smallvbox;
  GtkWidget *frame;
  GtkWidget *checkbutton;

  vbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), GNOME_PAD_SMALL);

  frame = gtk_frame_new ("Appearance");
  gtk_box_pack_start_defaults (GTK_BOX (vbox), frame);
  
  smallvbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (frame), smallvbox);
  
  checkbutton = gtk_check_button_new_with_label ("Show tasklist");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), TRUE);
  gtk_widget_set_sensitive (checkbutton, FALSE);

  gtk_box_pack_start_defaults (GTK_BOX (smallvbox), checkbutton);

  checkbutton = gtk_check_button_new_with_label ("Show tasklist arrow");
  gtk_widget_set_sensitive (checkbutton, FALSE);
  gtk_box_pack_start_defaults (GTK_BOX (smallvbox), checkbutton);

  checkbutton = gtk_check_button_new_with_label ("Switch tasklist arrow");
  gtk_widget_set_sensitive (checkbutton, FALSE);
  gtk_box_pack_start_defaults (GTK_BOX (smallvbox), checkbutton);
  
  frame = gtk_frame_new ("Behavior");
  gtk_box_pack_start_defaults (GTK_BOX (vbox), frame);
  
  smallvbox = gtk_vbox_new (TRUE, 0);
  gtk_container_add (GTK_CONTAINER (frame), smallvbox);
  
  checkbutton = gtk_check_button_new_with_label ("Right click shows window operations menu");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), temp_config.show_winops);
  gtk_signal_connect (GTK_OBJECT (checkbutton), "toggled",
		      GTK_SIGNAL_FUNC (properties_cb_checkbutton), &temp_config.show_winops);
  gtk_box_pack_start_defaults (GTK_BOX (smallvbox), checkbutton);
  
  checkbutton = gtk_check_button_new_with_label ("Left click minimizes when focused");
  gtk_widget_set_sensitive (checkbutton, FALSE);
  gtk_box_pack_start_defaults (GTK_BOX (smallvbox), checkbutton);
 
  checkbutton = gtk_check_button_new_with_label ("Confirm before killing windows");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), temp_config.confirm_kill);
  gtk_signal_connect (GTK_OBJECT (checkbutton), "toggled",
		      GTK_SIGNAL_FUNC (properties_cb_checkbutton), &temp_config.confirm_kill);
  gtk_box_pack_start_defaults (GTK_BOX (smallvbox), checkbutton);
  
  gtk_widget_show_all (vbox);
  return vbox;
}

void properties_show(void)
{
  temp_config = config;

  dialog = gnome_property_box_new ();
  gtk_window_set_title(GTK_WINDOW (dialog), "Tasklist properties");

  gtk_signal_connect (GTK_OBJECT (dialog), "apply",
		      GTK_SIGNAL_FUNC (properties_cb_apply), NULL);
  
  gnome_property_box_append_page (GNOME_PROPERTY_BOX (dialog),
				  properties_create_general_page(),
				  gtk_label_new("General"));
  gnome_property_box_append_page (GNOME_PROPERTY_BOX (dialog),
				  properties_create_tasks_page(),
				  gtk_label_new("Tasks"));
  gnome_property_box_append_page (GNOME_PROPERTY_BOX (dialog),
				  properties_create_geometry_page(),
				  gtk_label_new("Geometry"));
  gtk_widget_show (dialog);
  gdk_window_raise (dialog->window);
}
