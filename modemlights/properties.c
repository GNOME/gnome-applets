/* GNOME modemlights applet
 * (C) 2000 John Ellis
 *
 * Authors: John Ellis
 *          Martin Baulig
 *
 */

#include "modemlights.h"
#include <panel-applet.h>
#include <gconf/gconf-client.h>

#define DEFAULT_PATH "/modemlights/"

static GtkWidget *propwindow = NULL;
static GtkWidget *connect_entry = NULL;
static GtkWidget *disconnect_entry = NULL;
static GtkWidget *lockfile_entry = NULL;
static GtkWidget *device_entry = NULL;
static GtkWidget *verify_checkbox = NULL;
GConfClient *client;

static void show_extra_info_cb(GtkWidget *widget, gpointer data);
static void verify_lock_file_cb(GtkWidget *widget, gpointer data);
static void update_delay_cb(GtkWidget *widget, GdkEventFocus *event, gpointer data);
static void confirm_checkbox_cb(GtkWidget *widget, gpointer data);
static void isdn_checkbox_cb(GtkWidget *widget, gpointer data);
static void property_apply_cb(GtkWidget *widget, gint page_num, gpointer data);
static gint property_destroy_cb(GtkWidget *widget, gpointer data);

static gchar *color_rc_names[] = {
	"color_rx",
	"color_rx_bg",
	"color_tx",
	"color_tx_bg",
	"color_status_bg",
	"color_status_ok",
	"color_status_wait",
	"color_text_bg",
	"color_text_fg",
	"color_text_mid",
	NULL
};

/* the default colors */
static gchar *color_defaults[] = {
	"#FF0000",
	"#4D0000",
	"#00FF00",
	"#004D00",
	"#004D00",
	"#00FF00",
	"#FEF06B",
	"#000000",
	"#00FF00",
	"#004D00",
	NULL
};

void property_load(const char *path)
{
	gchar *buf, *key;
	gint i;

	g_free(command_connect);
	g_free(command_disconnect);
	g_free(device_name);

#ifdef FIXME
	client = gconf_client_get_default ();
	
	key = g_strconcat (path, "delay", NULL);
        UPDATE_DELAY = gconf_client_get_int (client, "key", NULL);
	g_free (key);
	
	key = g_strconcat (path, "lockfile", NULL);
	buf = gconf_client_get_string(client, key, NULL);
	g_freee (key);
	if (buf && strlen(buf) > 0)
		{
		g_free(lock_file);
		lock_file = g_strdup(buf);
		}
	g_free(buf);

	key = g_strconcat (path, "conect", NULL);
	command_connect    = gconf_client_get_string(client, key, NULL);
	g_free (key);
	key = g_strconcat (path, "disconnect", NULL);
	command_disconnect = gconf_client_get_string(client, key, NULL);
	g_free (key);
	key = g_strconcat (path, "confirmation", NULL);
	ask_for_confirmation = gconf_client_get_int(client, key, NULL);
	g_free (key);
	key = g_strconcat (path, "device", NULL);
	device_name          = gconf_client_get_string(client, key, NULL);
	g_free (key);
	key = g_strconcat (path, "isdn", NULL);
        use_ISDN	   = gconf_client_get_int(client, key, NULL);
       	g_free (key);
       	key = g_strconcat (path, "verify_lock", NULL);
	verify_lock_file   = gconf_client_get_int(client, key, NULL);
	g_free (key);
	key = g_strconcat (path, "extra_info", NULL);
	show_extra_info    = gconf_client_get_int(client, key, NULL);
	g_free (key);
	key = g_strconcat (path, "wait_blink", NULL);
	status_wait_blink  = gconf_client_get_int(client, key, NULL);
	g_free (key);
	
	for (i = 0; i < COLOR_COUNT; i++)
		{
		buf = g_strconcat(path, color_rc_names[i], NULL);
		g_free(display_color_text[i]);
		
		display_color_text[i] = gconf_client_get_string(buf);

		g_free(buf);
		}

	
#endif
	
	UPDATE_DELAY = 5;

	buf = "";
	if (buf && strlen(buf) > 0)
		{
		g_free(lock_file);
		lock_file = g_strdup(buf);
		}
	/*g_free(buf);*/

	command_connect    = g_strdup ("pppon");
	command_disconnect = g_strdup ("pppoff");
	ask_for_confirmation = 1;
	device_name          = g_strdup ("ppp0");
        use_ISDN	   = 0;
	verify_lock_file   = 1;
	show_extra_info    = 1;
	status_wait_blink  = 0;

	for (i = 0; i < COLOR_COUNT; i++)
		{
		buf = g_strconcat("modem/", color_rc_names[i], "=", color_defaults[i], NULL);
		g_free(display_color_text[i]);
		display_color_text[i] = g_strdup (color_defaults[i]);

		g_free(buf);
		}
}


void property_save(const char *path, gint to_default)
{
	gint i;
#ifdef FIXME
	if (path && !to_default)
		{
	        gnome_config_push_prefix(path);
		}
	else
		{
		gnome_config_push_prefix(DEFAULT_PATH);
		}

        gnome_config_set_int("modem/delay", UPDATE_DELAY);
        gnome_config_set_string("modem/lockfile", lock_file);
        gnome_config_set_string("modem/connect", command_connect);
        gnome_config_set_string("modem/disconnect", command_disconnect);
        gnome_config_set_int("modem/confirmation", ask_for_confirmation);
        gnome_config_set_string("modem/device", device_name);
        gnome_config_set_int("modem/isdn", use_ISDN);
        gnome_config_set_int("modem/verify_lock", verify_lock_file);
        gnome_config_set_int("modem/extra_info", show_extra_info);
        gnome_config_set_int("modem/wait_blink", status_wait_blink);

	for (i = 0; i < COLOR_COUNT; i++)
		{
		gchar *buf;

		buf = g_strconcat("modem/", color_rc_names[i], NULL);
	        gnome_config_set_string(buf, display_color_text[i]);
		g_free(buf);
		}

	gnome_config_sync();
	gnome_config_drop_all();
        gnome_config_pop_prefix();
#endif
}

static void connect_changed_cb(GtkEntry *entry, gpointer data)
{
	if (command_connect) g_free(command_connect);
	command_connect = gtk_editable_get_chars(GTK_EDITABLE(entry), 0, -1);
#ifdef SAVE_WORKS
	blah
#endif
}

static void disconnect_changed_cb(GtkEntry *entry, gpointer data)
{
	if (command_disconnect) g_free(command_disconnect);
	command_disconnect = gtk_editable_get_chars(GTK_EDITABLE(entry), 0, -1);
#ifdef SAVE_WORKS
	blah
#endif
}

static void lockfile_changed_cb(GtkEntry *entry, gpointer data)
{
	if (lock_file) g_free(lock_file);
	lock_file = gtk_editable_get_chars(GTK_EDITABLE(entry), 0, -1);
#ifdef SAVE_WORKS
	blah
#endif
}

static void device_changed_cb(GtkEntry *entry, gpointer data)
{
	if (device_name) g_free(device_name);
	device_name = gtk_editable_get_chars(GTK_EDITABLE(entry), 0, -1);
#ifdef SAVE_WORKS
	blah
#endif
}

static void show_extra_info_cb(GtkWidget *widget, gpointer data)
{
	gint tmp = GTK_TOGGLE_BUTTON (widget)->active;
	if (tmp != show_extra_info)
		{
		show_extra_info = tmp;
		/* change display */
		reset_orientation();
		}
#ifdef SAVE_WORKS
	key = ?;
	gconf_client_set_int (client, key, show_extra_info, NULL);
#endif
	return;
	data = NULL;
}

static void verify_lock_file_cb(GtkWidget *widget, gpointer data)
{
	verify_lock_file = GTK_TOGGLE_BUTTON (widget)->active;
	
#ifdef SAVE_WORKS
	blah
#endif
	return;
        data = NULL;
}

static void update_delay_cb(GtkWidget *widget, GdkEventFocus *event, gpointer data)
{
	gchar *key;
        UPDATE_DELAY = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
        start_callback_update();
#ifdef SAVE_WORKS
	key = ?;
        gconf_client_set_int (client, key, UPDATE_DELAY, NULL);
        g_free (key);
#endif
        return;
        widget = NULL;
}

static void confirm_checkbox_cb(GtkWidget *widget, gpointer data)
{
	ask_for_confirmation = GTK_TOGGLE_BUTTON (widget)->active;
#ifdef SAVE_WORKS
	blah
#endif
	
        return;
        data = NULL;
}

static void wait_blink_cb(GtkWidget *widget, gpointer data)
{
	status_wait_blink = GTK_TOGGLE_BUTTON (widget)->active;
#ifdef SAVE_WORKS
	key = ?;
	gconf_client_set_int (client, key, status_wait_blink, NULL);
	g_free (key);
#endif
        return;
        data = NULL;
}

static void isdn_checkbox_cb(GtkWidget *widget, gpointer data)
{
	use_ISDN = GTK_TOGGLE_BUTTON (widget)->active;

	gtk_widget_set_sensitive(lockfile_entry, !use_ISDN);
	gtk_widget_set_sensitive(device_entry, !use_ISDN);
	gtk_widget_set_sensitive(verify_checkbox, !use_ISDN);

#ifdef SAVE_WORKS
	blah;
#endif
        return;
        data = NULL;
}

static void set_default_cb(GtkWidget *widget, gpointer data)
{
	property_apply_cb(propwindow, -1, NULL);
	property_save(NULL, TRUE);
	gnome_property_box_set_modified((GnomePropertyBox *)propwindow, FALSE);
}

static void box_color_cb(GnomeColorPicker *cp, guint nopr, guint nopg, guint nopb, guint nopa, gpointer data)
{
	ColorType color	= (ColorType)GPOINTER_TO_INT(data);
	guint8 r, g, b, a;

	gnome_color_picker_get_i8 (GNOME_COLOR_PICKER(cp), &r, &g, &b, &a);

	g_free(display_color_text[color]);
	display_color_text[color] = g_strdup_printf("#%06X", (r << 16) + (g << 8) + b);
	
	reset_colors ();

#ifdef SAVE_WORKS
	
#endif
}

static GtkWidget *box_add_color(GtkWidget *box, const gchar *text, ColorType color)
{
	GtkWidget *vbox;
	GtkWidget *label;
	GtkWidget *color_sel;
	GdkColor c;

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 0);
	gtk_box_pack_start(GTK_BOX(box), vbox, TRUE, TRUE, 0);
	gtk_widget_show(vbox);

	if (display_color_text[color] != NULL)
		{
		gdk_color_parse(display_color_text[color], &c);
		}
	else
		{
		c.red = c.green = c.blue = 0;
		}

	color_sel = gnome_color_picker_new();
	gnome_color_picker_set_use_alpha(GNOME_COLOR_PICKER(color_sel), FALSE);
	gnome_color_picker_set_i16(GNOME_COLOR_PICKER(color_sel), c.red, c.green, c.blue, 0);
	g_signal_connect(G_OBJECT(color_sel), "color_set", 
			 G_CALLBACK (box_color_cb), GINT_TO_POINTER((gint)color));
	gtk_box_pack_start(GTK_BOX(vbox), color_sel, FALSE, FALSE, 0);
	gtk_widget_show(color_sel);

	label = gtk_label_new(text);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, GNOME_PAD_SMALL);
	gtk_widget_show(label);

	return vbox;
}

static GtkWidget *color_frame_new(GtkWidget *vbox, const gchar *text)
	{
	GtkWidget *frame;
	GtkWidget *hbox;

	frame = gtk_frame_new(text);
	gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);
	gtk_widget_show(frame);

	hbox = gtk_hbox_new(TRUE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), GNOME_PAD_SMALL);
	gtk_container_add(GTK_CONTAINER(frame), hbox);
	gtk_widget_show(hbox);

	return hbox;
	}

static void property_apply_cb(GtkWidget *widget, gint page_num, gpointer data)
{
	const gchar *new_text;
	gint i;
	gint c_changed;
#ifdef FIXME
	if (page_num != -1) return;

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
	status_wait_blink = P_status_wait_blink;

	if (P_show_extra_info != show_extra_info)
		{
		show_extra_info = P_show_extra_info;
		/* change display */
		reset_orientation();
		}

	c_changed = FALSE;
	for (i = 0; i < COLOR_COUNT; i++)
		{
		if (P_display_color_text[i] != NULL)
			{
			g_free(display_color_text[i]);
			display_color_text[i] = P_display_color_text[i];
			P_display_color_text[i] = NULL;
			c_changed = TRUE;
			}
		}

	if (c_changed) reset_colors();

	start_callback_update();

	applet_widget_sync_config(APPLET_WIDGET(applet));
#endif
	return;
	widget = NULL;
	data = NULL;
}

static void
phelp_cb (GtkWidget *w, gint tab, gpointer data)
{
#ifdef FIXME
	GnomeHelpMenuEntry help_entry = { "modemlights_applet",
					  "index.html#MODEMLIGHTS-PREFS" };
	gnome_help_display(NULL, &help_entry);
#endif
}

static void
property_response_cb (GtkDialog *dialog, gint id, gpointer data)
{
	gtk_widget_destroy (GTK_WIDGET (dialog));
	propwindow = NULL;
}

void property_show(BonoboUIComponent *uic, gpointer data, const gchar *verbname)
{
	GtkWidget *frame;
	GtkWidget *hbox;
	GtkWidget *notebook;
	GtkWidget *vbox;
	GtkWidget *vbox1;
	GtkWidget *label;
	GtkWidget *delay_w;
	GtkObject *delay_adj;
	GtkWidget *checkbox;
	GtkWidget *button;

	if(propwindow)
		{
                gdk_window_raise(propwindow->window);
                return;
		}

        propwindow = gtk_dialog_new_with_buttons (_("Modem Lights Settings"), NULL,
						  GTK_DIALOG_DESTROY_WITH_PARENT,
						  GTK_STOCK_OK, GTK_RESPONSE_OK,
						  NULL);
						  
	notebook = gtk_notebook_new ();
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (propwindow)->vbox), notebook,
			    TRUE, TRUE, 0);
	
	vbox = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), GNOME_PAD_SMALL);

	frame = gtk_frame_new(_("Connecting"));
	gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);
	gtk_widget_show(frame);

	vbox1 = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_container_set_border_width(GTK_CONTAINER(vbox1), GNOME_PAD_SMALL);
	gtk_container_add(GTK_CONTAINER(frame), vbox1);
	gtk_widget_show(vbox1);

	/* connect entry */
	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
        gtk_box_pack_start(GTK_BOX(vbox1), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

        label = gtk_label_new(_("Connect command:"));
        gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_widget_show(label);

	connect_entry = gtk_entry_new_with_max_length(255);
	gtk_entry_set_text(GTK_ENTRY(connect_entry), command_connect);
	g_signal_connect (G_OBJECT (connect_entry), "activate",
			  G_CALLBACK (connect_changed_cb), NULL);			  
        gtk_box_pack_start(GTK_BOX(hbox), connect_entry , TRUE, TRUE, 0);
	gtk_widget_show(connect_entry);

	/* disconnect entry */
	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
        gtk_box_pack_start(GTK_BOX(vbox1), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

        label = gtk_label_new(_("Disconnect command:"));
        gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_widget_show(label);

	disconnect_entry = gtk_entry_new_with_max_length(255);
	gtk_entry_set_text(GTK_ENTRY(disconnect_entry), command_disconnect);
	g_signal_connect (G_OBJECT (disconnect_entry), "activate",
			  G_CALLBACK (disconnect_changed_cb), NULL);
        gtk_box_pack_start(GTK_BOX(hbox), disconnect_entry, TRUE, TRUE, 0);
	gtk_widget_show(disconnect_entry);

	/* confirmation checkbox */
	checkbox = gtk_check_button_new_with_label(_("Confirm connection"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbox), ask_for_confirmation);
	g_signal_connect (G_OBJECT (checkbox), "toggled",
			  G_CALLBACK (confirm_checkbox_cb), NULL);
        gtk_box_pack_start(GTK_BOX(vbox1), checkbox, FALSE, FALSE, 0);
	gtk_widget_show(checkbox);

	frame = gtk_frame_new(_("Display"));
	gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);
	gtk_widget_show(frame);

	vbox1 = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_container_set_border_width(GTK_CONTAINER(vbox1), GNOME_PAD_SMALL);
	gtk_container_add(GTK_CONTAINER(frame), vbox1);
	gtk_widget_show(vbox1);

	/* update adjustment */
	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
        gtk_box_pack_start(GTK_BOX(vbox1), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

        label = gtk_label_new(_("Updates per second"));
        gtk_box_pack_start( GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_widget_show(label);

	delay_adj = gtk_adjustment_new( UPDATE_DELAY, 1.0, 20.0, 1, 1, 1 );
        delay_w  = gtk_spin_button_new( GTK_ADJUSTMENT(delay_adj), 1, 0 );
        gtk_box_pack_start(GTK_BOX(hbox), delay_w, FALSE, FALSE, 0);
	g_signal_connect (G_OBJECT (delay_w), "focus-out-event",
			  G_CALLBACK (update_delay_cb), NULL);
        gtk_spin_button_set_update_policy( GTK_SPIN_BUTTON(delay_w),GTK_UPDATE_ALWAYS );
	gtk_widget_show(delay_w);

	/* extra info checkbox */
	checkbox = gtk_check_button_new_with_label(_("Show connect time and throughput"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbox), show_extra_info);
	g_signal_connect(G_OBJECT(checkbox), "toggled",
			 G_CALLBACK(show_extra_info_cb), NULL);
        gtk_box_pack_start(GTK_BOX(vbox1), checkbox, FALSE, FALSE, 0);
	gtk_widget_show(checkbox);

        label = gtk_label_new_with_mnemonic (_("_General"));
        gtk_widget_show(vbox);
        /*gnome_property_box_append_page( GNOME_PROPERTY_BOX(propwindow), vbox, label);*/
        gtk_notebook_append_page (GTK_NOTEBOOK (notebook), vbox, label);

	/* color settings */

	vbox = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), GNOME_PAD_SMALL);

	hbox = color_frame_new(vbox, _("Receive data"));

	box_add_color(hbox, _("Foreground"), COLOR_RX);
	box_add_color(hbox, _("Background"), COLOR_RX_BG);

	hbox = color_frame_new(vbox, _("Send data"));

	box_add_color(hbox, _("Foreground"), COLOR_TX);
	box_add_color(hbox, _("Background"), COLOR_TX_BG);

	hbox = color_frame_new(vbox, _("Connection status"));

	box_add_color(hbox, _("Connected"), COLOR_STATUS_OK);
	box_add_color(hbox, _("Not connected"), COLOR_STATUS_BG);
	vbox1 = box_add_color(hbox, _("Awaiting connection"), COLOR_STATUS_WAIT);


	checkbox = gtk_check_button_new_with_label(_("Blink"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbox), status_wait_blink);
	g_signal_connect(G_OBJECT(checkbox), "toggled",
			 G_CALLBACK(wait_blink_cb), NULL);
        gtk_box_pack_start(GTK_BOX(vbox1), checkbox, FALSE, FALSE, 0);
	gtk_widget_show(checkbox);

	hbox = color_frame_new(vbox, _("Text"));

	box_add_color(hbox, _("Foreground"), COLOR_TEXT_FG);
	box_add_color(hbox, _("Background"), COLOR_TEXT_BG);
	box_add_color(hbox, _("Outline"), COLOR_TEXT_MID);

	label = gtk_label_new(_("Colors"));
	gtk_widget_show(vbox);
	gtk_notebook_append_page( GTK_NOTEBOOK(notebook), vbox, label);

	/* advanced settings */

	vbox = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), GNOME_PAD_SMALL);

	frame = gtk_frame_new(_("Modem options"));
	gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);
	gtk_widget_show(frame);

	vbox1 = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_container_set_border_width(GTK_CONTAINER(vbox1), GNOME_PAD_SMALL);
	gtk_container_add(GTK_CONTAINER(frame), vbox1);
	gtk_widget_show(vbox1);

	/* lock file entry */
	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
        gtk_box_pack_start(GTK_BOX(vbox1), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

        label = gtk_label_new(_("Modem lock file:"));
        gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_widget_show(label);

	lockfile_entry = gtk_entry_new_with_max_length(255);
	gtk_entry_set_text(GTK_ENTRY(lockfile_entry), lock_file);
	g_signal_connect (G_OBJECT (lockfile_entry), "activate",
			  G_CALLBACK (lockfile_changed_cb), NULL);
        gtk_box_pack_start(GTK_BOX(hbox), lockfile_entry, TRUE, TRUE, 0);
	gtk_widget_show(lockfile_entry);

	verify_checkbox = gtk_check_button_new_with_label(_("Verify owner of lock file"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (verify_checkbox), verify_lock_file);
	g_signal_connect(G_OBJECT(verify_checkbox), "toggled",
			 G_CALLBACK(verify_lock_file_cb), NULL);
        gtk_box_pack_start(GTK_BOX(vbox1), verify_checkbox, FALSE, FALSE, 0);
	gtk_widget_show(verify_checkbox);

	/* device entry */
	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
        gtk_box_pack_start(GTK_BOX(vbox1), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

        label = gtk_label_new(_("Device:"));
        gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_widget_show(label);

	device_entry = gtk_entry_new_with_max_length(16);
	gtk_entry_set_text(GTK_ENTRY(device_entry), device_name);
	g_signal_connect (G_OBJECT (device_entry), "activate",
			  G_CALLBACK (device_changed_cb), NULL);
        gtk_box_pack_start(GTK_BOX(hbox), device_entry, TRUE, TRUE, 0);
	gtk_widget_show(device_entry);

	/* ISDN checkbox */
	checkbox = gtk_check_button_new_with_label(_("Use ISDN"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbox), use_ISDN);
	g_signal_connect(G_OBJECT(checkbox), "toggled",
			   G_CALLBACK(isdn_checkbox_cb), NULL);
        gtk_box_pack_start(GTK_BOX(vbox), checkbox, FALSE, FALSE, 0);
	gtk_widget_show(checkbox);

	if (use_ISDN)
		{
		gtk_widget_set_sensitive(lockfile_entry, FALSE);
		gtk_widget_set_sensitive(device_entry, FALSE);
		gtk_widget_set_sensitive(verify_checkbox, FALSE);
		}
#ifdef SHOULD_THIS_BE_HERE
	/* defaults save button */
	
	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_end(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

	button = gtk_button_new_with_label(_("Set options as default"));
	/*gtk_signal_connect(GTK_OBJECT(button), "clicked",
			   GTK_SIGNAL_FUNC(set_default_cb), NULL);*/
	gtk_box_pack_end(GTK_BOX(hbox), button, FALSE, FALSE, 0);
	gtk_widget_show(button);
#endif
        label = gtk_label_new(_("Advanced"));
        gtk_widget_show(vbox);
        gtk_notebook_append_page( GTK_NOTEBOOK(notebook), vbox, label);

	g_signal_connect (G_OBJECT (propwindow), "response",
			  G_CALLBACK (property_response_cb), NULL);

        gtk_widget_show_all(propwindow);
        return;
        applet = NULL;
        data = NULL;
        
    
} 



