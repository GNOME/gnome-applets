/* properties.c -- properties dialog box and session management for character
 * picker applet. Much of this is adapted from modemlights/properties.c
 */

#include "charpick.h"

static GtkWidget *propwindow = NULL;

/* temporary variable modified by the properties dialog.  they get
   copied to the permanent variables when the users selects Apply or
   Ok */
charpick_persistant_properties temp_properties;

void property_load (char *path, gpointer data)
{
  /* charpick_persistant_properties * properties = data; */
  /* default charlist is not yet handled by sm.(this is now done in main) */
  /*  properties->default_charlist = a_list; */
  /*  gnome_config_push_prefix(APPLET_WIDGET(applet)->privcfgpath); */
  gnome_config_push_prefix(path);
  /* FIXME: sprintf() these strings so I can use the #defines
   */
  curr_data.properties->follow_panel_size = gnome_config_get_bool("charpick/follow_panel_size=true");
  curr_data.properties->min_cells = gnome_config_get_int("charpick/min_cells=8");
  curr_data.properties->rows = gnome_config_get_int("charpick/rows=2");
  curr_data.properties->cols = gnome_config_get_int("charpick/cols=4");
  curr_data.properties->size = gnome_config_get_int("charpick/buttonsize=22");
  curr_data.properties->default_charlist = 
    gnome_config_get_string("charpick/deflist=байнсуЅ©");

  /* sanity check the properties read from config */
  if (curr_data.properties->rows < 1)
  {
    curr_data.properties->rows = DEFAULT_ROWS; 
  }
  if (curr_data.properties->cols < 1)
  {
    curr_data.properties->cols =  DEFAULT_COLS; 
  }
  if (curr_data.properties->size < 1)
  {
    curr_data.properties->size = DEFAULT_SIZE; 
  }

  if (curr_data.properties->min_cells < 1)
  {
    curr_data.properties->min_cells = DEFAULT_ROWS; 
  }
  else if (curr_data.properties->min_cells > MAX_BUTTONS)
  {
    curr_data.properties->min_cells = DEFAULT_ROWS; 
  }

  gnome_config_pop_prefix();
  return;
}

void property_save (char *path, charpick_persistant_properties *properties)
{
  gnome_config_push_prefix(path);
  gnome_config_set_int("charpick/buttonsize", properties->size);
  gnome_config_set_int("charpick/rows", properties->rows);
  gnome_config_set_int("charpick/cols", properties->cols);
  gnome_config_set_int("charpick/min_cells", properties->min_cells);
  gnome_config_set_bool("charpick/follow_panel_size",
			properties->follow_panel_size);
  gnome_config_set_string("charpick/deflist", 
  		          properties->default_charlist);
  gnome_config_pop_prefix();
  gnome_config_sync();
}

static void update_spin_cb( GtkWidget *spin, gint *data)
{
  *data = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin));
  gnome_property_box_changed(GNOME_PROPERTY_BOX(propwindow));
  return;
}

static void update_bool_cb( GtkWidget *cb, gboolean *data)
{
  *data = GTK_TOGGLE_BUTTON (cb)->active;
  gnome_property_box_changed(GNOME_PROPERTY_BOX(propwindow));
  return;
}

static void update_default_list_cb( GtkWidget *entry, gpointer data)
{
  temp_properties.default_charlist = g_strdup(gtk_entry_get_text(GTK_ENTRY(entry)));
   gnome_property_box_changed(GNOME_PROPERTY_BOX(propwindow));
  return;
}

static void property_apply_cb (GtkWidget *widget, void *data)
{
  /*charpick_data * curr_data = data;*/
  curr_data.properties->follow_panel_size = temp_properties.follow_panel_size;
  curr_data.properties->min_cells = temp_properties.min_cells;
  curr_data.properties->rows = temp_properties.rows;
  curr_data.properties->cols = temp_properties.cols;
  curr_data.properties->size = temp_properties.size;
  curr_data.properties->default_charlist = 
    g_strdup(temp_properties.default_charlist);
  applet_widget_sync_config(APPLET_WIDGET(curr_data.applet));
  /* set applet to applied state */
  curr_data.charlist = curr_data.properties->default_charlist;
  build_table(&curr_data);
  return;
}

static gint property_destroy_cb (GtkWidget *widget, void *data)
{
  propwindow = NULL;
  return FALSE;
}

static void check_button_disable_cb (GtkWidget *cb, GtkWidget *todisable)
{
  gboolean active = GTK_TOGGLE_BUTTON(cb)->active;
  gtk_widget_set_sensitive (todisable, !active);
}

static void check_button_enable_cb (GtkWidget *cb, GtkWidget *todisable)
{
  gboolean active = GTK_TOGGLE_BUTTON(cb)->active;
  gtk_widget_set_sensitive (todisable, active);
}

static void size_frame_create()
{

  GtkWidget *frame;
  GtkWidget *min_cells_hbox;
  GtkWidget *size_hbox;
  GtkWidget *rows_hbox;
  GtkWidget *cols_hbox;
  GtkWidget *min_cells_label;
  GtkWidget *size_label;
  GtkWidget *rows_label;
  GtkWidget *cols_label;
  GtkWidget *tab_label;
  GtkObject *min_cells_adj;
  GtkObject *size_adj;
  GtkObject *rows_adj;
  GtkObject *cols_adj;
  GtkWidget *min_cells_sb;
  GtkWidget *size_sb;
  GtkWidget *rows_sb;
  GtkWidget *cols_sb;
  GtkWidget *follow_cb;

  /* make some widgets */
  frame = gtk_vbox_new(FALSE, 5);
  min_cells_hbox = gtk_hbox_new(FALSE, 5);
  size_hbox = gtk_hbox_new(FALSE, 5);
  rows_hbox = gtk_hbox_new(FALSE, 5);
  cols_hbox = gtk_hbox_new(FALSE, 5);
  min_cells_label = gtk_label_new(_("Minimum number of cells: (for autosize)"));
  size_label = gtk_label_new(_("Size of button: (pixels)"));
  rows_label = gtk_label_new(_("Number of rows of buttons:"));
  cols_label = gtk_label_new(_("Number of columns of buttons:"));

  /* the follow_panel_size check button */
  follow_cb = gtk_check_button_new_with_label(_("Follow panel size"));
  gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (follow_cb),
			       temp_properties.follow_panel_size);
  gtk_signal_connect (GTK_OBJECT (follow_cb), "toggled",
		      GTK_SIGNAL_FUNC (update_bool_cb),
		      &temp_properties.follow_panel_size);


  /* pack the main vbox */
  gtk_box_pack_start (GTK_BOX(frame), follow_cb, FALSE, FALSE, 5);
  gtk_box_pack_start (GTK_BOX(frame), min_cells_hbox, FALSE, FALSE, 5);
  gtk_box_pack_start (GTK_BOX(frame), rows_hbox, FALSE, FALSE, 5);
  gtk_box_pack_start (GTK_BOX(frame), cols_hbox, FALSE, FALSE, 5);
  gtk_box_pack_start (GTK_BOX(frame), size_hbox, FALSE, FALSE, 5);

  /*min_cells hbox */
  gtk_box_pack_start (GTK_BOX(min_cells_hbox), min_cells_label, FALSE, FALSE, 5);
  gtk_widget_show(min_cells_label);
  min_cells_adj = gtk_adjustment_new( temp_properties.min_cells, 1.0,
				      MAX_BUTTONS, 1, 1, 1 );
  min_cells_sb  = gtk_spin_button_new( GTK_ADJUSTMENT(min_cells_adj), 1, 0 );
  gtk_box_pack_start( GTK_BOX(min_cells_hbox), min_cells_sb, FALSE, FALSE, 5);
  gtk_signal_connect
    (GTK_OBJECT(min_cells_sb), "changed", GTK_SIGNAL_FUNC(update_spin_cb), &temp_properties.min_cells);
  gtk_spin_button_set_update_policy
    (GTK_SPIN_BUTTON(min_cells_sb),GTK_UPDATE_ALWAYS);
  gtk_widget_show(min_cells_sb);
  gtk_signal_connect (GTK_OBJECT (follow_cb), "toggled",
		      GTK_SIGNAL_FUNC (check_button_enable_cb),
		      min_cells_hbox);
  check_button_enable_cb (follow_cb, min_cells_hbox);

  /*size hbox*/
  gtk_box_pack_start (GTK_BOX(size_hbox), size_label, FALSE, FALSE, 5);
  gtk_widget_show(size_label);
  size_adj = gtk_adjustment_new (temp_properties.size, 1.0, 40.0, 1, 1, 1 );
  size_sb  = gtk_spin_button_new( GTK_ADJUSTMENT(size_adj), 1, 0 );
  gtk_box_pack_start( GTK_BOX(size_hbox), size_sb, FALSE, FALSE, 5);
  /*gtk_signal_connect
    (GTK_OBJECT(size_adj), "value_changed", 
    GTK_SIGNAL_FUNC(update_size_cb), &temp_properties.size);*/
  gtk_signal_connect
    (GTK_OBJECT(size_sb), "changed", GTK_SIGNAL_FUNC(update_spin_cb), &temp_properties.size);
  gtk_spin_button_set_update_policy
    (GTK_SPIN_BUTTON(size_sb),GTK_UPDATE_ALWAYS);
  gtk_widget_show(size_sb);

  /*rows hbox */
  gtk_box_pack_start (GTK_BOX(rows_hbox), rows_label, FALSE, FALSE, 5);
  gtk_widget_show(rows_label);
  rows_adj = gtk_adjustment_new( temp_properties.rows, 1.0, 5.0, 1, 1, 1 );
  rows_sb  = gtk_spin_button_new( GTK_ADJUSTMENT(rows_adj), 1, 0 );
  gtk_box_pack_start( GTK_BOX(rows_hbox), rows_sb, FALSE, FALSE, 5);
  /*gtk_signal_connect
    (GTK_OBJECT(rows_adj), "value_changed", 
    GTK_SIGNAL_FUNC(update_rows_cb), &temp_properties.rows);*/
  gtk_signal_connect
    (GTK_OBJECT(rows_sb), "changed", GTK_SIGNAL_FUNC(update_spin_cb), &temp_properties.rows);
  gtk_spin_button_set_update_policy
    (GTK_SPIN_BUTTON(rows_sb),GTK_UPDATE_ALWAYS);
  gtk_widget_show(rows_sb);
  gtk_signal_connect (GTK_OBJECT (follow_cb), "toggled",
		      GTK_SIGNAL_FUNC (check_button_disable_cb),
		      rows_hbox);
  check_button_disable_cb (follow_cb, rows_hbox);

  /*cols hbox */
  gtk_box_pack_start (GTK_BOX(cols_hbox), cols_label, FALSE, FALSE, 5);
  gtk_widget_show (cols_label);
  cols_adj = gtk_adjustment_new (temp_properties.cols, 1.0, 5.0, 1, 1, 1 );
  cols_sb  = gtk_spin_button_new (GTK_ADJUSTMENT(cols_adj), 1, 0 );
  gtk_box_pack_start (GTK_BOX(cols_hbox), cols_sb, FALSE, FALSE, 5);
  /*gtk_signal_connect
    (GTK_OBJECT(cols_adj), "value_changed", 
     GTK_SIGNAL_FUNC(update_cols_cb), &temp_properties.cols);*/
  gtk_signal_connect
    (GTK_OBJECT(cols_sb), "changed", GTK_SIGNAL_FUNC(update_spin_cb), &temp_properties.cols);
  gtk_spin_button_set_update_policy
    (GTK_SPIN_BUTTON(cols_sb),GTK_UPDATE_ALWAYS);
  gtk_widget_show(cols_sb);
  gtk_signal_connect (GTK_OBJECT (follow_cb), "toggled",
		      GTK_SIGNAL_FUNC (check_button_disable_cb),
		      cols_hbox);
  check_button_disable_cb (follow_cb, cols_hbox);

  tab_label = gtk_label_new(_("Size"));
  gtk_widget_show(frame);
  gnome_property_box_append_page (GNOME_PROPERTY_BOX(propwindow), 
                                  frame , tab_label);
  return;
}

static void default_chars_frame_create(void)
{
  GtkWidget *tab_label;
  GtkWidget *frame;
  GtkWidget *default_list_hbox;
  GtkWidget *default_list_label;
  GtkWidget *default_list_entry;
  GtkWidget *explain_label;
  
  /* init widgets */
  frame = gtk_vbox_new(FALSE, 5);
  default_list_hbox = gtk_hbox_new(FALSE, 5);
  default_list_label = gtk_label_new(_("Default character list:"));
  default_list_entry = gtk_entry_new_with_max_length (MAX_BUTTONS);
  gtk_entry_set_text(GTK_ENTRY(default_list_entry), 
		     curr_data.properties->default_charlist);
  explain_label = gtk_label_new(_("These characters will appear when the panel"
                                  " is started. To return to this list, hit"
                                  " <space> while the applet has focus."));
  gtk_label_set_line_wrap(GTK_LABEL(explain_label), TRUE);
  /* pack the main vbox */
  gtk_box_pack_start (GTK_BOX(frame), default_list_hbox, FALSE, FALSE, 5);
  gtk_box_pack_start (GTK_BOX(frame), explain_label, FALSE, FALSE, 5);
  /* default_list hbox */
  gtk_box_pack_start(GTK_BOX(default_list_hbox), default_list_label, FALSE, FALSE, 5);
  gtk_box_pack_start( GTK_BOX(default_list_hbox), default_list_entry, 
		      FALSE, FALSE, 5);
  gtk_signal_connect (GTK_OBJECT(default_list_entry), "changed", 
		      GTK_SIGNAL_FUNC(update_default_list_cb), 
		      &temp_properties.default_charlist);

  /* add tab to propwindow */
  tab_label = gtk_label_new(_("Default List"));
  gtk_widget_show_all(frame);
  gnome_property_box_append_page (GNOME_PROPERTY_BOX(propwindow), 
                                  frame, tab_label);
  return;
}

static void
phelp_cb (GtkWidget *w, gint tab, gpointer data)
{
	GnomeHelpMenuEntry help_entry = { "charpick_applet",
                                          "index.html#CHARPICKAPPLET-PREFS" };
	gnome_help_display(NULL, &help_entry);
}


void
property_show(AppletWidget *applet, gpointer data)
{
  temp_properties.default_charlist = 
    g_strdup(curr_data.properties->default_charlist);
  temp_properties.follow_panel_size = curr_data.properties->follow_panel_size;
  temp_properties.min_cells = curr_data.properties->min_cells;
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

  size_frame_create();
  default_chars_frame_create();
  gtk_signal_connect (GTK_OBJECT(propwindow), "apply",
		      GTK_SIGNAL_FUNC(property_apply_cb), &curr_data);
  gtk_signal_connect (GTK_OBJECT(propwindow), "destroy",
		      GTK_SIGNAL_FUNC(property_destroy_cb), NULL);
  gtk_signal_connect( GTK_OBJECT(propwindow), "help",
		      GTK_SIGNAL_FUNC(phelp_cb), NULL);
  gtk_widget_show_all(propwindow);
  return;
}





