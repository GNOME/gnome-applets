/* GNOME modemlights applet
 * (C) 1999 John Ellis
 *
 * Authors: John Ellis
 *          Martin Baulig
 *
 */

#include "modemlights.h"

static GtkWidget *propwindow = NULL;
static GtkWidget *connect_entry = NULL;
static GtkWidget *disconnect_entry = NULL;
static GtkWidget *lockfile_entry = NULL;
static GtkWidget *device_entry = NULL;
static GtkWidget *verify_checkbox = NULL;

/* temporary variables modified by the properties dialog.  they get
   compied to the permanent variables when the users selects Apply or
   Ok */
static gint P_UPDATE_DELAY = 10;
static gint P_ask_for_confirmation = TRUE;
static gint P_use_ISDN = FALSE;
static gint P_verify_lock_file = TRUE;
static gint P_show_extra_info = FALSE;

static void show_extra_info_cb( GtkWidget *widget, gpointer data );
static void verify_lock_file_cb( GtkWidget *widget, gpointer data );
static void update_delay_cb( GtkWidget *widget, GtkWidget *spin );
static void confirm_checkbox_cb( GtkWidget *widget, gpointer data );
static void isdn_checkbox_cb( GtkWidget *widget, gpointer data );
static void property_apply_cb( GtkWidget *widget, void *data );
static gint property_destroy_cb( GtkWidget *widget, void *data );

void property_load(char *path)
{
	gchar *buf;

	if (command_connect) g_free(command_connect);
	if (command_disconnect) g_free(command_disconnect);
	if (device_name) g_free(device_name);
        gnome_config_push_prefix (path);
        UPDATE_DELAY       = gnome_config_get_int("modem/delay=5");

	buf                = gnome_config_get_string("modem/lockfile=");
	if (buf && strlen(buf) > 0)
		{
		g_free(lock_file);
		lock_file = g_strdup(buf);
		}
	g_free(buf);

	command_connect    = gnome_config_get_string("modem/connect=pppon");
	command_disconnect = gnome_config_get_string("modem/disconnect=pppoff");
	ask_for_confirmation = gnome_config_get_int("modem/confirmation=1");
	device_name          = gnome_config_get_string("modem/device=ppp0");
        use_ISDN	   = gnome_config_get_int("modem/isdn=0");
	verify_lock_file   = gnome_config_get_int("modem/verify_lock=1");
	show_extra_info    = gnome_config_get_int("modem/extra_info=0");
	gnome_config_pop_prefix ();
}

void property_save(char *path)
{
        gnome_config_push_prefix(path);
        gnome_config_set_int("modem/delay", UPDATE_DELAY);
        gnome_config_set_string("modem/lockfile", lock_file);
        gnome_config_set_string("modem/connect", command_connect);
        gnome_config_set_string("modem/disconnect", command_disconnect);
        gnome_config_set_int("modem/confirmation", ask_for_confirmation);
        gnome_config_set_string("modem/device", device_name);
        gnome_config_set_int("modem/isdn", use_ISDN);
        gnome_config_set_int("modem/verify_lock", verify_lock_file);
        gnome_config_set_int("modem/extra_info", show_extra_info);
	gnome_config_sync();
	gnome_config_drop_all();
        gnome_config_pop_prefix();
}

static void show_extra_info_cb( GtkWidget *widget, gpointer data )
{
	P_show_extra_info = GTK_TOGGLE_BUTTON (widget)->active;
	gnome_property_box_changed(GNOME_PROPERTY_BOX(propwindow));
}

static void verify_lock_file_cb( GtkWidget *widget, gpointer data )
{
	P_verify_lock_file = GTK_TOGGLE_BUTTON (widget)->active;
	gnome_property_box_changed(GNOME_PROPERTY_BOX(propwindow));
}

static void update_delay_cb( GtkWidget *widget, GtkWidget *spin )
{
        P_UPDATE_DELAY = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin));
        gnome_property_box_changed(GNOME_PROPERTY_BOX(propwindow));
}

static void confirm_checkbox_cb( GtkWidget *widget, gpointer data )
{
	P_ask_for_confirmation = GTK_TOGGLE_BUTTON (widget)->active;
	gnome_property_box_changed(GNOME_PROPERTY_BOX(propwindow));
}

static void isdn_checkbox_cb( GtkWidget *widget, gpointer data )
{
	P_use_ISDN = GTK_TOGGLE_BUTTON (widget)->active;

	gtk_widget_set_sensitive(lockfile_entry, !P_use_ISDN);
	gtk_widget_set_sensitive(device_entry, !P_use_ISDN);
	gtk_widget_set_sensitive(verify_checkbox, !P_use_ISDN);

	gnome_property_box_changed(GNOME_PROPERTY_BOX(propwindow));
}

static void property_apply_cb( GtkWidget *widget, void *data )
{
	gchar *new_text;

	if (lock_file) g_free(lock_file);
	new_text = gtk_entry_get_text(GTK_ENTRY(lockfile_entry));
	lock_file = g_strdup(new_text);

	if (command_connect) g_free(command_connect);
	new_text = gtk_entry_get_text(GTK_ENTRY(connect_entry));
	command_connect = g_strdup(new_text);

	if (command_disconnect) g_free(command_disconnect);
	new_text = gtk_entry_get_text(GTK_ENTRY(disconnect_entry));
	command_disconnect = g_strdup(new_text);

	if (device_name) g_free(device_name);
	new_text = gtk_entry_get_text(GTK_ENTRY(device_entry));
	device_name = g_strdup(new_text);

        UPDATE_DELAY = P_UPDATE_DELAY;
	ask_for_confirmation = P_ask_for_confirmation;
	use_ISDN = P_use_ISDN;
	verify_lock_file = P_verify_lock_file;

	if (P_show_extra_info != show_extra_info)
		{
		show_extra_info = P_show_extra_info;
		/* change display */
		reset_orientation();
		}

	start_callback_update();

	applet_widget_sync_config(APPLET_WIDGET(applet));
}

static gint property_destroy_cb( GtkWidget *widget, void *data )
{
        propwindow = NULL;
	return FALSE;
}

void property_show(AppletWidget *applet, gpointer data)
{
        static GnomeHelpMenuEntry help_entry = { NULL, "properties" };
	GtkWidget *frame;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *delay_w;
	GtkObject *delay_adj;
	GtkWidget *checkbox;

	help_entry.name = gnome_app_id;
	
	if(propwindow)
		{
                gdk_window_raise(propwindow->window);
                return;
		}

        P_UPDATE_DELAY = UPDATE_DELAY;
	P_ask_for_confirmation = ask_for_confirmation;
	P_verify_lock_file = verify_lock_file;
	P_show_extra_info = show_extra_info;

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
	gtk_signal_connect( GTK_OBJECT(delay_w),"changed",GTK_SIGNAL_FUNC(update_delay_cb), delay_w);
        gtk_spin_button_set_update_policy( GTK_SPIN_BUTTON(delay_w),GTK_UPDATE_ALWAYS );
	gtk_widget_show(delay_w);

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
	checkbox = gtk_check_button_new_with_label(_("Confirm connection"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbox), ask_for_confirmation);
	gtk_signal_connect( GTK_OBJECT(checkbox), "toggled",
			    GTK_SIGNAL_FUNC(confirm_checkbox_cb), NULL);
        gtk_box_pack_start( GTK_BOX(frame), checkbox, FALSE, FALSE, 5);
	gtk_widget_show(checkbox);

	/* extra info checkbox */
	checkbox = gtk_check_button_new_with_label(_("Show connect time and throughput"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbox), show_extra_info);
	gtk_signal_connect( GTK_OBJECT(checkbox), "toggled",
			    GTK_SIGNAL_FUNC(show_extra_info_cb), NULL);
        gtk_box_pack_start( GTK_BOX(frame), checkbox, FALSE, FALSE, 5);
	gtk_widget_show(checkbox);

        label = gtk_label_new(_("General"));
        gtk_widget_show(frame);
        gnome_property_box_append_page( GNOME_PROPERTY_BOX(propwindow),frame ,label);

	/* advanced settings */

	frame = gtk_vbox_new(FALSE, 5);

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

	verify_checkbox = gtk_check_button_new_with_label(_("Verify owner of lock file"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (verify_checkbox), verify_lock_file);
	gtk_signal_connect( GTK_OBJECT(verify_checkbox), "toggled",
			    GTK_SIGNAL_FUNC(verify_lock_file_cb), NULL);
        gtk_box_pack_start( GTK_BOX(frame), verify_checkbox, FALSE, FALSE, 5);
	gtk_widget_show(verify_checkbox);

	/* device entry */
	hbox = gtk_hbox_new(FALSE, 5);
        gtk_box_pack_start( GTK_BOX(frame), hbox, FALSE, FALSE, 5);
	gtk_widget_show(hbox);

        label = gtk_label_new(_("Device:"));
        gtk_box_pack_start( GTK_BOX(hbox), label, FALSE, FALSE, 5);
	gtk_widget_show(label);

	device_entry = gtk_entry_new_with_max_length(16);
	gtk_entry_set_text(GTK_ENTRY(device_entry), device_name);
	gtk_signal_connect_object(GTK_OBJECT(device_entry), "changed",
                            GTK_SIGNAL_FUNC(gnome_property_box_changed),
                            GTK_OBJECT(propwindow));
        gtk_box_pack_start( GTK_BOX(hbox),device_entry , TRUE, TRUE, 5);
	gtk_widget_show(device_entry);

	/* ISDN checkbox */
	checkbox = gtk_check_button_new_with_label(_("Use ISDN"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbox), use_ISDN);
	gtk_signal_connect( GTK_OBJECT(checkbox), "toggled",
			    GTK_SIGNAL_FUNC(isdn_checkbox_cb), NULL);
        gtk_box_pack_start( GTK_BOX(frame), checkbox, FALSE, FALSE, 5);
	gtk_widget_show(checkbox);

	if (use_ISDN)
		{
		gtk_widget_set_sensitive(lockfile_entry, FALSE);
		gtk_widget_set_sensitive(device_entry, FALSE);
		gtk_widget_set_sensitive(verify_checkbox, FALSE);
		}

        label = gtk_label_new(_("Advanced"));
        gtk_widget_show(frame);
        gnome_property_box_append_page( GNOME_PROPERTY_BOX(propwindow),frame ,label);

	gtk_signal_connect( GTK_OBJECT(propwindow), "apply",
			    GTK_SIGNAL_FUNC(property_apply_cb), NULL );
	gtk_signal_connect( GTK_OBJECT(propwindow), "destroy",
			    GTK_SIGNAL_FUNC(property_destroy_cb), NULL );
	gtk_signal_connect( GTK_OBJECT(propwindow), "help",
			    GTK_SIGNAL_FUNC(gnome_help_pbox_display),
			    &help_entry );

        gtk_widget_show_all(propwindow);
} 
