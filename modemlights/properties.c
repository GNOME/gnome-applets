/*#####################################################*/
/*##           modemlights applet 0.3.0 alpha        ##*/
/*#####################################################*/

#include "modemlights.h"

GtkWidget *propwindow = NULL;
GtkWidget *lockfile_entry = NULL;
GtkWidget *connect_entry = NULL;
GtkWidget *disconnect_entry = NULL;
GtkWidget *confirm_checkbox = NULL;

/* temporary variables modified by the properties dialog.  they get
   compied to the permanent variables when the users selects Apply or
   Ok */
gint P_UPDATE_DELAY = 10;
gint P_ask_for_confirmation = TRUE;


void property_load(char *path)
{
	if (lock_file) g_free(lock_file);
	if (command_connect) g_free(command_connect);
	if (command_disconnect) g_free(command_disconnect);
        gnome_config_push_prefix (path);
        UPDATE_DELAY       = gnome_config_get_int("delay=10");
	lock_file          = gnome_config_get_string("lockfile=/var/lock/LCK..modem");
	command_connect    = gnome_config_get_string("connect=pppon");
	command_disconnect = gnome_config_get_string("disconnect=pppoff");
	ask_for_confirmation = gnome_config_get_int("confirmation=1");
	gnome_config_pop_prefix ();
}

void property_save(char *path)
{
        gnome_config_push_prefix(path);
        gnome_config_set_int("delay", UPDATE_DELAY);
        gnome_config_set_string("lockfile", lock_file);
        gnome_config_set_string("connect", command_connect);
        gnome_config_set_string("disconnect", command_disconnect);
        gnome_config_set_int("confirmation", ask_for_confirmation);
	gnome_config_sync();
	gnome_config_drop_all();
        gnome_config_pop_prefix();
}

void update_delay_cb( GtkWidget *widget, GtkWidget *spin )
{
        P_UPDATE_DELAY = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin));
        gnome_property_box_changed(GNOME_PROPERTY_BOX(propwindow));
}

void confirm_checkbox_cb( GtkWidget *widget, GtkWidget *spin )
{
  P_ask_for_confirmation = !P_ask_for_confirmation;
  gnome_property_box_changed(GNOME_PROPERTY_BOX(propwindow));
}

void property_apply_cb( GtkWidget *widget, void *data )
{
	gchar *new_text;

	if (lock_file) free(lock_file);
	new_text = gtk_entry_get_text(GTK_ENTRY(lockfile_entry));
	lock_file = strdup(new_text);

	if (command_connect) free(command_connect);
	new_text = gtk_entry_get_text(GTK_ENTRY(connect_entry));
	command_connect = strdup(new_text);

	if (command_disconnect) free(command_disconnect);
	new_text = gtk_entry_get_text(GTK_ENTRY(disconnect_entry));
	command_disconnect = strdup(new_text);

        UPDATE_DELAY = P_UPDATE_DELAY;
	ask_for_confirmation = P_ask_for_confirmation;
	start_callback_update();
}

gint property_destroy_cb( GtkWidget *widget, void *data )
{
        propwindow = NULL;
	return FALSE;
}

void property_show(AppletWidget *applet, gpointer data)
{
	GtkWidget *frame;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *delay_w;
	GtkObject *delay_adj;

	if(propwindow)
		{
                gdk_window_raise(propwindow->window);
                return;
		}

        P_UPDATE_DELAY = UPDATE_DELAY;
	P_ask_for_confirmation = ask_for_confirmation;

	propwindow = gnome_property_box_new();
	gtk_window_set_title(GTK_WINDOW(&GNOME_PROPERTY_BOX(propwindow)->dialog.window),
		_("Modem Lights Settings"));
	
	frame = gtk_vbox_new(FALSE, 5);

	hbox = gtk_hbox_new(FALSE, 5);
        gtk_box_pack_start( GTK_BOX(frame), hbox, FALSE, FALSE, 5);
	gtk_widget_show(hbox);

        label = gtk_label_new(_("Updates per second"));
        gtk_box_pack_start( GTK_BOX(hbox), label, FALSE, FALSE, 5);
	gtk_widget_show(label);

	delay_adj = gtk_adjustment_new( P_UPDATE_DELAY, 1.0, 20.0, 1, 1, 1 );
        delay_w  = gtk_spin_button_new( GTK_ADJUSTMENT(delay_adj), 1, 0 );
        gtk_box_pack_start( GTK_BOX(hbox), delay_w, FALSE, FALSE, 5);
	gtk_signal_connect( GTK_OBJECT(delay_adj),"value_changed",GTK_SIGNAL_FUNC(update_delay_cb), delay_w);
        gtk_spin_button_set_update_policy( GTK_SPIN_BUTTON(delay_w),GTK_UPDATE_ALWAYS );
	gtk_widget_show(delay_w);

	/* lock file entry */
	hbox = gtk_hbox_new(FALSE, 5);
        gtk_box_pack_start( GTK_BOX(frame), hbox, FALSE, FALSE, 5);
	gtk_widget_show(hbox);

        label = gtk_label_new(_("Modem lock file:"));
        gtk_box_pack_start( GTK_BOX(hbox), label, FALSE, FALSE, 5);
	gtk_widget_show(label);

	lockfile_entry = gtk_entry_new_with_max_length(255);
	gtk_entry_set_text(GTK_ENTRY(lockfile_entry), lock_file);
	gtk_signal_connect_object(GTK_OBJECT(lockfile_entry), "changed",
                            GTK_SIGNAL_FUNC(gnome_property_box_changed),
                            GTK_OBJECT(propwindow));
        gtk_box_pack_start( GTK_BOX(hbox),lockfile_entry , TRUE, TRUE, 5);
	gtk_widget_show(lockfile_entry);

	/* connect entry */
	hbox = gtk_hbox_new(FALSE, 5);
        gtk_box_pack_start( GTK_BOX(frame), hbox, FALSE, FALSE, 5);
	gtk_widget_show(hbox);

        label = gtk_label_new(_("Connect command:"));
        gtk_box_pack_start( GTK_BOX(hbox), label, FALSE, FALSE, 5);
	gtk_widget_show(label);

	connect_entry = gtk_entry_new_with_max_length(255);
	gtk_entry_set_text(GTK_ENTRY(connect_entry), command_connect);
	gtk_signal_connect_object(GTK_OBJECT(connect_entry), "changed",
                            GTK_SIGNAL_FUNC(gnome_property_box_changed),
                            GTK_OBJECT(propwindow));
        gtk_box_pack_start( GTK_BOX(hbox),connect_entry , TRUE, TRUE, 5);
	gtk_widget_show(connect_entry);

	/* disconnect entry */
	hbox = gtk_hbox_new(FALSE, 5);
        gtk_box_pack_start( GTK_BOX(frame), hbox, FALSE, FALSE, 5);
	gtk_widget_show(hbox);

        label = gtk_label_new(_("Disconnect command:"));
        gtk_box_pack_start( GTK_BOX(hbox), label, FALSE, FALSE, 5);
	gtk_widget_show(label);

	disconnect_entry = gtk_entry_new_with_max_length(255);
	gtk_entry_set_text(GTK_ENTRY(disconnect_entry), command_disconnect);
	gtk_signal_connect_object(GTK_OBJECT(disconnect_entry), "changed",
                            GTK_SIGNAL_FUNC(gnome_property_box_changed),
                            GTK_OBJECT(propwindow));
        gtk_box_pack_start( GTK_BOX(hbox),disconnect_entry , TRUE, TRUE, 5);
	gtk_widget_show(disconnect_entry);

	/* confirmation checkbox */
	confirm_checkbox = gtk_check_button_new_with_label(_("Confirm connection?"));
	if (ask_for_confirmation) {
	  gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (confirm_checkbox),
				       TRUE);
	} else {
	  gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (confirm_checkbox),
				       FALSE);
	}
	gtk_signal_connect( GTK_OBJECT(confirm_checkbox), "toggled",
			    GTK_SIGNAL_FUNC(confirm_checkbox_cb),
			    confirm_checkbox);
        gtk_box_pack_start( GTK_BOX(frame), confirm_checkbox, FALSE, FALSE, 5);
	gtk_widget_show(confirm_checkbox);


        label = gtk_label_new(_("General"));
        gtk_widget_show(frame);
        gnome_property_box_append_page( GNOME_PROPERTY_BOX(propwindow),frame ,label);

	gtk_signal_connect( GTK_OBJECT(propwindow),"apply", GTK_SIGNAL_FUNC(property_apply_cb), NULL );
	gtk_signal_connect( GTK_OBJECT(propwindow),"destroy", GTK_SIGNAL_FUNC(property_destroy_cb), NULL );

        gtk_widget_show_all(propwindow);
} 
