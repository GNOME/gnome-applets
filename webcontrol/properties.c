/* Webcontrol properties dialog and actions */

#include "properties.h"
#include "webcontrol.h"

webcontrol_properties props;

webcontrol_properties tmp_props = {-1, -1, -1, -1, -1,
				   -1, -1, -1, NULL, NULL};

extern WebControl WC;

static void
phelp_cb (GtkWidget *w, gint tab, gpointer data)
{
	GnomeHelpMenuEntry help_entry = {"webcontrol_applet", 
					 "index.html#WEBCONTROL-APPLET-PREFS"};

	gnome_help_display (NULL, &help_entry);
}


static void
apply_cb (GnomePropertyBox *pb, gint page, gpointer data)
{
        /* Only honor global apply */
	if (page != -1) return;

	/* apply properties to the applet */
	if (tmp_props.show_clear != -1)
	{
		props.show_clear = tmp_props.show_clear;
		tmp_props.show_clear = -1;
	}

	if (tmp_props.clear_top != -1)
	{	      			
		props.clear_top = tmp_props.clear_top;
		tmp_props.clear_top = -1;
	}

	if (tmp_props.width != -1)
	{
		props.width = tmp_props.width;
		tmp_props.width = -1;
	}
		
	if (tmp_props.show_go != -1) 
	{
		props.show_go = tmp_props.show_go;
		tmp_props.show_go = -1;
	}

	if (tmp_props.show_check != -1) 
	{
		props.show_check = tmp_props.show_check;
		tmp_props.show_check = -1;
	}

	if (tmp_props.hist_len != -1)
	{
		if (props.hist_len > tmp_props.hist_len)
			gtk_list_clear_items (GTK_LIST (GTK_COMBO (WC.input)->list),
					      tmp_props.hist_len, -1);
		props.hist_len = tmp_props.hist_len;
		tmp_props.hist_len = -1;
	}

	if (tmp_props.use_mime != -1)
	{
		props.use_mime = tmp_props.use_mime;
		tmp_props.use_mime = -1;
	}

	if (tmp_props.curr_browser)
	{
		props.curr_browser = tmp_props.curr_browser;
		tmp_props.curr_browser = NULL;
	}

	applet_widget_sync_config (APPLET_WIDGET (WC.applet));

	draw_applet ();

	return;
}


/* when a user selects a row, change the properties to reflect this. */
static void
browser_list_row_selected (GtkCList *list, gint row, gint column,
			   GdkEventButton *event, gpointer user_data)
{
	GSList *ptr = NULL;

	ptr = (GSList *) gtk_clist_get_row_data (list, row);

	tmp_props.curr_browser = ptr;

	return;
}

static void
entry_changed_int (GtkWidget *entry, int *data)
{
	char *text;
	int new_value;

	text = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);

	new_value = atoi (text);

	*data = new_value;

	g_free (text);
	
	return;
}


extern void 
check_box_toggled (GtkWidget *toggle, int *data)
{
	*data = GTK_TOGGLE_BUTTON (toggle)->active;
}


/* fill the clist widget, and while we're at it, select the 
 * current browser */
static void
fill_browser_list (GtkCList *list)
{
	GSList *ptr;
	gint row;

	for (ptr = props.browsers; ptr; ptr = ptr->next)
	{
		row = gtk_clist_prepend (list, &(B_LIST_NAME (ptr)));		
		gtk_clist_set_row_data (list, row, (gpointer) ptr);
		if (ptr == props.curr_browser)
		{
			gtk_clist_select_row (list, row, 0);
		}
	}
	
	return;
}


extern void
properties_box (AppletWidget *widget, gpointer data)
{
	static GtkWidget *pb = NULL;
	GtkWidget *vbox;
	GtkWidget *vbox2;
	GtkWidget *vbox3;
	GtkWidget *check;
	GtkWidget *size_hbox;
	GtkWidget *hbox;
	GtkWidget *width_label;
	GtkWidget *entry;
	GtkWidget *label;
	GtkWidget *button, *button2;
	GtkWidget *frame;
	GtkWidget *sw;
	GtkWidget *browser_list;
	GSList *url_act_group;
	char str_t[10];

	/* Stop the property box from being opened multiple times */
	if (pb != NULL)
	{
		gdk_window_show (GTK_WIDGET (pb)->window);
		gdk_window_raise (GTK_WIDGET (pb)->window);
		return;
	}

	pb = gnome_property_box_new ();

	gtk_window_set_title (GTK_WINDOW (pb), _("WebControl Properties"));

/* START OF APPEARANCE TAB */

	vbox = gtk_vbox_new (FALSE, GNOME_PAD);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), GNOME_PAD);

	/* create size properties */
	width_label = gtk_label_new (_("URL bar width:"));
	gtk_widget_show (width_label);

	entry = gtk_entry_new ();
	gtk_widget_set_usize (entry, 50, 0); 
	g_snprintf (str_t, sizeof (str_t), "%d", props.width);
	gtk_entry_set_text (GTK_ENTRY (entry),  str_t);
	gtk_widget_show (entry);
	
	gtk_signal_connect (GTK_OBJECT (entry), "changed", 
			    GTK_SIGNAL_FUNC (entry_changed_int),
			    &tmp_props.width);

	gtk_signal_connect_object (GTK_OBJECT (entry), "changed",
				   GTK_SIGNAL_FUNC (gnome_property_box_changed),
				   GTK_OBJECT (pb));

	size_hbox = gtk_hbox_new (FALSE, 3);
	
	gtk_box_pack_start (GTK_BOX (size_hbox), width_label, FALSE, FALSE, 3);
	gtk_box_pack_start (GTK_BOX (size_hbox), entry, FALSE, FALSE, 3);
	
	gtk_box_pack_start (GTK_BOX (vbox), size_hbox, FALSE, FALSE, GNOME_PAD);

	/* create Show Go button property */
	check = gtk_check_button_new_with_label (_("Display GO Button"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check), props.show_go);

	gtk_signal_connect (GTK_OBJECT (check), "toggled",
			    GTK_SIGNAL_FUNC (check_box_toggled),
			    &tmp_props.show_go);

	gtk_signal_connect_object (GTK_OBJECT (check), "toggled",
				   GTK_SIGNAL_FUNC (gnome_property_box_changed),
				   GTK_OBJECT (pb));

	gtk_box_pack_start (GTK_BOX (vbox), check, FALSE, FALSE, GNOME_PAD);	

	/* create Launch New Window property */
	check = gtk_check_button_new_with_label (_("Display \"launch new window\" option"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check), props.show_check);
	
	gtk_signal_connect (GTK_OBJECT (check),"toggled",
			    GTK_SIGNAL_FUNC (check_box_toggled),
			    &tmp_props.show_check);

	gtk_signal_connect_object (GTK_OBJECT (check), "toggled",
				   GTK_SIGNAL_FUNC (gnome_property_box_changed),
				   GTK_OBJECT (pb));

	gtk_box_pack_start (GTK_BOX (vbox), check, FALSE, FALSE, GNOME_PAD);

	/* create show clear button property */
	check = gtk_check_button_new_with_label (_("Display \"Clear\" button"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check), 
				      props.show_clear);

	gtk_signal_connect (GTK_OBJECT (check), "toggled",
			    GTK_SIGNAL_FUNC (check_box_toggled),
			    &tmp_props.show_clear);

	gtk_signal_connect_object (GTK_OBJECT (check), "toggled",
				   GTK_SIGNAL_FUNC (gnome_property_box_changed),
				   GTK_OBJECT (pb));

	gtk_box_pack_start (GTK_BOX (vbox), check, FALSE, FALSE, GNOME_PAD);

        /* create clear button on top property */
	check = gtk_check_button_new_with_label (_("\"Clear\" button on top?"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check), 
				      props.clear_top);

	gtk_signal_connect (GTK_OBJECT (check), "toggled",
			    GTK_SIGNAL_FUNC (check_box_toggled),
			    &tmp_props.clear_top);

	gtk_signal_connect_object (GTK_OBJECT (check), "toggled",
				   GTK_SIGNAL_FUNC (gnome_property_box_changed),
				   GTK_OBJECT (pb));

	gtk_box_pack_start (GTK_BOX (vbox), check, FALSE, FALSE, GNOME_PAD);

	gnome_property_box_append_page (GNOME_PROPERTY_BOX (pb), vbox,
					gtk_label_new (_("Appearance")));

/* END OF APPEARANCE TAB */
/* START OF BEHAVIOR TAB */

	/* URL activation frame */

	vbox = gtk_vbox_new (FALSE, 2);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 2);

	frame = gtk_frame_new (_(" URL Activation "));
	gtk_widget_show (frame);

	vbox2 = gtk_vbox_new (FALSE, 2);
	gtk_widget_show (vbox2);

	button = gtk_radio_button_new_with_label 
		(NULL, _("Use default MIME handler"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button),
				      props.use_mime);
	gtk_widget_show (button);
	
	url_act_group = gtk_radio_button_group (GTK_RADIO_BUTTON (button));

	button2 = gtk_radio_button_new_with_label 
		(url_act_group, _("Use custom browser:"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button2),
				      !props.use_mime);
	gtk_widget_show (button2);

	gtk_signal_connect (GTK_OBJECT (button), "toggled",
			    GTK_SIGNAL_FUNC (check_box_toggled),
			    &tmp_props.use_mime);

	gtk_signal_connect_object (GTK_OBJECT (button), "toggled",
				   GTK_SIGNAL_FUNC (gnome_property_box_changed),
				   GTK_OBJECT (pb));
	
	gtk_box_pack_start (GTK_BOX (vbox2), button, FALSE, FALSE, 2);
	gtk_box_pack_start (GTK_BOX (vbox2), button2, FALSE, FALSE, 2);

	hbox = gtk_hbox_new (FALSE, 3);
	gtk_widget_show (hbox);

	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
					GTK_POLICY_AUTOMATIC, 
					GTK_POLICY_ALWAYS);
	gtk_widget_show (sw);
					
	browser_list = gtk_clist_new (3);
	gtk_clist_set_column_width (GTK_CLIST (browser_list), 0, 100);
        gtk_clist_set_column_visibility (GTK_CLIST (browser_list), 0, TRUE);
	gtk_clist_set_column_visibility (GTK_CLIST (browser_list), 1, FALSE);
	gtk_clist_set_column_visibility (GTK_CLIST (browser_list), 2, FALSE);
	fill_browser_list (GTK_CLIST (browser_list));
	gtk_widget_show (browser_list);

	gtk_signal_connect (GTK_OBJECT (browser_list), "select-row",
			    GTK_SIGNAL_FUNC (browser_list_row_selected),
			    NULL);

	gtk_signal_connect_object (GTK_OBJECT (browser_list), "select-row",
				   GTK_SIGNAL_FUNC (gnome_property_box_changed),
				   GTK_OBJECT (pb));

	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (sw), 
					       browser_list);
	gtk_widget_set_usize (sw, 300, 100);

	gtk_box_pack_start (GTK_BOX (hbox), sw, FALSE, FALSE, 2);

	vbox3 = gtk_vbox_new (FALSE, GNOME_PAD);
	gtk_widget_show (vbox3);

	button = gtk_button_new_with_label (_("Add"));
	gtk_widget_show (button);
	gtk_box_pack_start (GTK_BOX (vbox3), button, FALSE, FALSE, 2);

	button = gtk_button_new_with_label (_("Edit"));
	gtk_widget_show (button);
	gtk_box_pack_start (GTK_BOX (vbox3), button, FALSE, FALSE, 2);

	button = gtk_button_new_with_label (_("Remove"));
	gtk_widget_show (button);
	gtk_box_pack_start (GTK_BOX (vbox3), button, FALSE, FALSE, 2);

	gtk_box_pack_start (GTK_BOX (hbox), vbox3, FALSE, FALSE, 10);
	
	gtk_box_pack_start (GTK_BOX (vbox2), hbox, FALSE, FALSE, 2);

	gtk_container_add (GTK_CONTAINER (frame), vbox2);

	gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 2);

	/* URL history frame */

	frame = gtk_frame_new (_(" URL History "));
	gtk_widget_show (frame);
	
	vbox2 = gtk_vbox_new (FALSE, 2);
	gtk_widget_show (vbox2);

	hbox = gtk_hbox_new (FALSE, 2);
	gtk_widget_show (hbox);

	label = gtk_label_new (_("Size of URL history: "));
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 2);

	entry = gtk_entry_new ();
	gtk_widget_set_usize (entry, 50, 0);
	g_snprintf (str_t, sizeof (str_t), "%d", props.hist_len);
	gtk_entry_set_text (GTK_ENTRY (entry),  str_t);
	gtk_widget_show (entry);	
	
	gtk_signal_connect (GTK_OBJECT (entry), "changed", 
			    GTK_SIGNAL_FUNC (entry_changed_int),
			    &tmp_props.hist_len);

	gtk_signal_connect_object (GTK_OBJECT (entry), "changed",
				   GTK_SIGNAL_FUNC (gnome_property_box_changed),
				   GTK_OBJECT (pb));

	gtk_box_pack_start (GTK_BOX (hbox), entry, FALSE, FALSE, 2);

	gtk_box_pack_start (GTK_BOX (vbox2), hbox, FALSE, FALSE, 2);

	hbox = gtk_hbox_new (FALSE, 2);
	gtk_widget_show (hbox);

	button = gtk_button_new_with_label (_("Clear"));
	gtk_misc_set_padding (GTK_MISC (GTK_BIN (button)->child), 5, 2);
	gtk_widget_show (button);	

	gtk_signal_connect (GTK_OBJECT (button), "clicked",
			    GTK_SIGNAL_FUNC (clear_url_history),
			    (gpointer) WC.input);

	gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 2);
	
	label = gtk_label_new (_("all entries in URL history"));
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 2);

	gtk_box_pack_start (GTK_BOX (vbox2), hbox, FALSE, FALSE, 2);

	gtk_container_add (GTK_CONTAINER (frame), vbox2);

	gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 2);

	gnome_property_box_append_page (GNOME_PROPERTY_BOX (pb), vbox,
					gtk_label_new(_("Behavior")));

/* END OF BEHAVIOR TAB */

	gtk_signal_connect (GTK_OBJECT (pb), "apply", GTK_SIGNAL_FUNC (apply_cb),
			    NULL);

	gtk_signal_connect (GTK_OBJECT (pb), "destroy",
			    gtk_widget_destroyed,
			    (gpointer) &pb);

	gtk_signal_connect (GTK_OBJECT (pb), "help",
			    phelp_cb, NULL);

	gtk_widget_show_all (pb);

	return;
}
