/* properties.c -- properties dialog box and session management for character
 * picker applet. Much of this is adapted from modemlights/properties.c
 */

#include "charpick.h"

static GtkWidget *propwindow = NULL;

/* temporary variable modified by the properties dialog.  they get
   copied to the permanent variables when the users selects Apply or
   Ok */
static charpick_persistant_properties temp_properties;

void property_load (char *path, gpointer data)
{
  charpick_persistant_properties * properties = data;
  /* default charlist is not yet handled by sm.(this is now done in main) */
  /*  properties->default_charlist = a_list; */
  /*  gnome_config_push_prefix(APPLET_WIDGET(applet)->privcfgpath); */
  gnome_config_push_prefix(path);
  /* c sucks because defines don't work in string literals.
   * which seems to be the only place I ever need them.
   */
  properties->rows = gnome_config_get_int("charpick/rows=2");
  properties->cols = gnome_config_get_int("charpick/cols=4");
  properties->size = gnome_config_get_int("charpick/buttonsize=24");
  gnome_config_pop_prefix();
}

void property_save (char *path, charpick_persistant_properties *properties)
{
  gnome_config_push_prefix(path);
  gnome_config_set_int("charpick/buttonsize", temp_properties.size);
  gnome_config_set_int("charpick/rows", temp_properties.rows);
  gnome_config_set_int("charpick/cols", temp_properties.cols);
  gnome_config_sync();
  gnome_config_drop_all();
  gnome_config_pop_prefix();
}

static void update_size_cb( GtkWidget *spin, gpointer data)
{
  temp_properties.size = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin));
  gnome_property_box_changed(GNOME_PROPERTY_BOX(propwindow));
}

static void update_rows_cb( GtkWidget *spin, gpointer data)
{
  temp_properties.rows = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin));
  gnome_property_box_changed(GNOME_PROPERTY_BOX(propwindow));
}

static void update_cols_cb( GtkWidget *spin, gpointer data)
{
  gint * cols = data;
  temp_properties.cols = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin));
  gnome_property_box_changed(GNOME_PROPERTY_BOX(propwindow));
}

static void property_apply_cb (GtkWidget *widget, void *data)
{
  /*charpick_data * curr_data = data;*/
  curr_data.properties->rows = temp_properties.rows;
  curr_data.properties->cols = temp_properties.cols;
  curr_data.properties->size = temp_properties.size;
  applet_widget_sync_config(APPLET_WIDGET(curr_data.applet));
  build_table(&curr_data);
}

static gint property_destroy_cb (GtkWidget *widget, void *data)
{
        propwindow = NULL;
	return FALSE;
}

void property_show(AppletWidget *applet, gpointer data)
{
  /* the property box consists of three hboxen in a vbox, each with a 
   *label, an adjustment entry, and a spin-button.
   */
  GtkWidget *frame;
  GtkWidget *size_hbox;
  GtkWidget *rows_hbox;
  GtkWidget *cols_hbox;
  GtkWidget *size_label;
  GtkWidget *rows_label;
  GtkWidget *cols_label;
  GtkWidget *tab_label;
  GtkObject *size_adj;
  GtkObject *rows_adj;
  GtkObject *cols_adj;
  GtkWidget *size_sb;
  GtkWidget *rows_sb;
  GtkWidget *cols_sb;
  gint * temp_rows = &temp_properties.rows;
  gint * temp_cols = &temp_properties.cols;
  gint * temp_size = &temp_properties.size;

  /*curr_data = data;*/
  temp_properties.size = curr_data.properties->size;
  temp_properties.rows = curr_data.properties->rows;
  temp_properties.cols = curr_data.properties->cols;
  /*if the properties dialog is already open, just raise it.*/
  if(propwindow)
  {
    gdk_window_raise(propwindow->window);
    return;
  }
  propwindow = gnome_property_box_new();
  gtk_window_set_title
    (GTK_WINDOW(&GNOME_PROPERTY_BOX(propwindow)->dialog.window),
       _("Character Picker Settings"));

  /* make some widgets */
  frame = gtk_vbox_new(FALSE, 5);
  size_hbox = gtk_hbox_new(FALSE, 5);
  rows_hbox = gtk_hbox_new(FALSE, 5);
  cols_hbox = gtk_hbox_new(FALSE, 5);
  size_label = gtk_label_new(_("Size of button: (pixels)"));
  rows_label = gtk_label_new(_("Number of rows of buttons:"));
  cols_label = gtk_label_new(_("Number of columns of buttons:"));

  /* pack the main vbox */
  gtk_box_pack_start (GTK_BOX(frame), rows_hbox, FALSE, FALSE, 5);
  gtk_box_pack_start (GTK_BOX(frame), cols_hbox, FALSE, FALSE, 5);
  gtk_box_pack_start (GTK_BOX(frame), size_hbox, FALSE, FALSE, 5);

  /*size hbox*/
  gtk_box_pack_start (GTK_BOX(size_hbox), size_label, FALSE, FALSE, 5);
  gtk_widget_show(size_label);
  size_adj = gtk_adjustment_new (*temp_size, 1.0, 30.0, 1, 1, 1 );
  size_sb  = gtk_spin_button_new( GTK_ADJUSTMENT(size_adj), 1, 0 );
  gtk_box_pack_start( GTK_BOX(size_hbox), size_sb, FALSE, FALSE, 5);
  /*gtk_signal_connect
    (GTK_OBJECT(size_adj), "value_changed", 
    GTK_SIGNAL_FUNC(update_size_cb), &temp_size);*/
  gtk_signal_connect
    (GTK_OBJECT(size_sb), "changed", GTK_SIGNAL_FUNC(update_size_cb), &temp_properties.size);
  gtk_spin_button_set_update_policy
    (GTK_SPIN_BUTTON(size_sb),GTK_UPDATE_ALWAYS);
  gtk_widget_show(size_sb);

  /*rows hbox */
  gtk_box_pack_start (GTK_BOX(rows_hbox), rows_label, FALSE, FALSE, 5);
  gtk_widget_show(rows_label);
  rows_adj = gtk_adjustment_new( *temp_rows, 1.0, 5.0, 1, 1, 1 );
  rows_sb  = gtk_spin_button_new( GTK_ADJUSTMENT(rows_adj), 1, 0 );
  gtk_box_pack_start( GTK_BOX(rows_hbox), rows_sb, FALSE, FALSE, 5);
  /*gtk_signal_connect
    (GTK_OBJECT(rows_adj), "value_changed", 
    GTK_SIGNAL_FUNC(update_rows_cb), &temp_rows);*/
  gtk_signal_connect
    (GTK_OBJECT(rows_sb), "changed", GTK_SIGNAL_FUNC(update_rows_cb), &temp_properties.rows);
  gtk_spin_button_set_update_policy
    (GTK_SPIN_BUTTON(rows_sb),GTK_UPDATE_ALWAYS);
  gtk_widget_show(rows_sb);

  /*cols hbox */
  gtk_box_pack_start (GTK_BOX(cols_hbox), cols_label, FALSE, FALSE, 5);
  gtk_widget_show (cols_label);
  cols_adj = gtk_adjustment_new (*temp_cols, 1.0, 5.0, 1, 1, 1 );
  cols_sb  = gtk_spin_button_new (GTK_ADJUSTMENT(cols_adj), 1, 0 );
  gtk_box_pack_start (GTK_BOX(cols_hbox), cols_sb, FALSE, FALSE, 5);
  /*gtk_signal_connect
    (GTK_OBJECT(cols_adj), "value_changed", 
     GTK_SIGNAL_FUNC(update_cols_cb), &temp_cols);*/
  gtk_signal_connect
    (GTK_OBJECT(cols_sb), "changed", GTK_SIGNAL_FUNC(update_cols_cb), &temp_properties.cols);
  gtk_spin_button_set_update_policy
    (GTK_SPIN_BUTTON(cols_sb),GTK_UPDATE_ALWAYS);
  gtk_widget_show(cols_sb);

  tab_label = gtk_label_new(_("General"));
  gtk_widget_show(frame);
  gnome_property_box_append_page (GNOME_PROPERTY_BOX(propwindow), 
                                  frame , tab_label);

  gtk_signal_connect (GTK_OBJECT(propwindow), "apply",
		      GTK_SIGNAL_FUNC(property_apply_cb), &curr_data);
  gtk_signal_connect (GTK_OBJECT(propwindow), "destroy",
		      GTK_SIGNAL_FUNC(property_destroy_cb), NULL);
  gtk_widget_show_all(propwindow);
}

