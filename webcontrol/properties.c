/* Webcontrol properties dialog and actions */

#include "properties.h"

webcontrol_properties props = {FALSE, 
			       TRUE, 
			       FALSE, 
			       200, 
			       FALSE,
			       10};

webcontrol_properties tmp_props = {-1, -1, -1, -1, -1, -1};

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

	if (tmp_props.width != -1)
	{
		props.width = tmp_props.width;
		tmp_props.width = -1;
	}
		
	if (tmp_props.show_url != -1) 
	{
		props.show_url = tmp_props.show_url;
		tmp_props.show_url = -1;
	}
	
	if (tmp_props.show_check != -1) 
	{
		props.show_check = tmp_props.show_check;
		tmp_props.show_check = -1;
	}

	draw_applet ();

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
}


extern void 
check_box_toggled (GtkWidget *check, int *data)
{
	*data = GTK_TOGGLE_BUTTON (check)->active;
}


extern void
properties_box (AppletWidget *widget, gpointer data)
{
	static GtkWidget *pb = NULL;
	GtkWidget *vbox;
	GtkWidget *check;
	GtkWidget *size_hbox;
	GtkWidget *width_label;
	GtkWidget *width_entry;
	char width_c[10];

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
	width_label = gtk_label_new (_("Width:"));
	gtk_widget_show (width_label);

	width_entry = gtk_entry_new ();
	gtk_widget_set_usize (width_entry, 50, 0); 
	g_snprintf (width_c, sizeof (width_c), "%d", props.width);
	gtk_entry_set_text (GTK_ENTRY (width_entry),  width_c);
	gtk_widget_show (width_entry);
	
	gtk_signal_connect (GTK_OBJECT (width_entry), "changed", 
			    GTK_SIGNAL_FUNC (entry_changed_int),
			    &tmp_props.width);

	gtk_signal_connect_object (GTK_OBJECT (width_entry), "changed",
				   GTK_SIGNAL_FUNC (gnome_property_box_changed),
				   GTK_OBJECT (pb));

	size_hbox = gtk_hbox_new (FALSE, 3);
	
	gtk_box_pack_start (GTK_BOX (size_hbox), width_label, FALSE, FALSE, 3);
	gtk_box_pack_start (GTK_BOX (size_hbox), width_entry, FALSE, FALSE, 3);
	
	gtk_box_pack_start (GTK_BOX (vbox), size_hbox, FALSE, FALSE, GNOME_PAD);

	/* create Show URL property */
	check = gtk_check_button_new_with_label (_("Display URL label"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check), props.show_url);

	gtk_signal_connect (GTK_OBJECT (check), "toggled",
			    GTK_SIGNAL_FUNC (check_box_toggled),
			    &tmp_props.show_url);

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

	gtk_box_pack_start (GTK_BOX (vbox), check, TRUE, TRUE, GNOME_PAD);

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

	gnome_property_box_append_page (GNOME_PROPERTY_BOX (pb), vbox,
					gtk_label_new(_("Appearance")));

/* END OF APPEARANCE TAB */
/* START OF BEHAVIOR TAB  - not finished

	vbox = gtk_vbox_new (FALSE, 2);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 2);

	frame = gtk_frame_new (_("URL Activation"));
	gtk_widget_show (frame);

	vbox2 = gtk_vbox_new (FALSE, 2);
	gtk_widget_show (vbox2);

	button = gtk_radio_button_new_with_label 
		(NULL, _("Use default MIME handler"));
	gtk_widget_show (button);
	gtk_box_pack_start (GTK_BOX (vbox2), button, FALSE, FALSE, 2);

	url_act_group = gtk_radio_button_group (GTK_RADIO_BUTTON (button));

	button = gtk_radio_button_new_with_label 
		(url_act_group, _("Use custom browser:"));
	gtk_widget_show (button);
	gtk_box_pack_start (GTK_BOX (vbox2), button, FALSE, FALSE, 2);



	gtk_container_add (GTK_CONTAINER (frame), vbox2);

	gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 2);

	frame = gtk_frame_new (_("URL History"));
	gtk_widget_show (frame);
	
	vbox2 = gtk_vbox_new (FALSE, 2);
	gtk_widget_show (vbox2);

	label = gtk_label_new (_("Size of URL history: "));
	gtk_widget_show (label);

	gtk_box_pack_start (GTK_BOX (vbox2), label, FALSE, FALSE, 2);

	gtk_container_add (GTK_CONTAINER (frame), vbox2);

	gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 2);

	gnome_property_box_append_page (GNOME_PROPERTY_BOX (pb), vbox,
					gtk_label_new(_("Behavior")));


END OF BEHAVIOR TAB */

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

