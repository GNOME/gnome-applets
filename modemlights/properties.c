/* GNOME modemlights applet
 * (C) 2000 John Ellis
 *
 * Authors: John Ellis
 *          Martin Baulig
 *
 */

#include <config.h>
#include "modemlights.h"
#include <panel-applet.h>
#include <panel-applet-gconf.h>
#include <gconf/gconf-client.h>

#define DEFAULT_PATH "/modemlights/"

static GtkWidget *propwindow = NULL;
static GtkWidget *connect_entry = NULL;
static GtkWidget *disconnect_entry = NULL;
static GtkWidget *lockfile_entry = NULL;
static GtkWidget *device_entry = NULL;
static GtkWidget *verify_checkbox = NULL;

static void show_extra_info_cb(GtkWidget *widget, gpointer data);
static void verify_lock_file_cb(GtkWidget *widget, gpointer data);
static void update_delay_cb(GtkWidget *widget, GdkEventFocus *event, gpointer data);
static void confirm_checkbox_cb(GtkWidget *widget, gpointer data);
static void isdn_checkbox_cb(GtkWidget *widget, gpointer data);

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

void property_load(PanelApplet *applet)
{
	GError *error = NULL;
	gchar *buf;
	gint i;

	g_free(command_connect);
	g_free(command_disconnect);
	g_free(device_name);

	UPDATE_DELAY = panel_applet_gconf_get_int (applet, "delay", &error);
	if (error) {
		g_print ("%s \n", error->message);
		g_error_free (error);
		error = NULL;
	}
	UPDATE_DELAY = MAX (UPDATE_DELAY, 1);
	
	buf = panel_applet_gconf_get_string(applet, "lockfile", NULL);
	if (buf && strlen(buf) > 0)
		{
		g_free(lock_file);
		lock_file = g_strdup(buf);
		}
	g_free(buf);

	command_connect    = panel_applet_gconf_get_string(applet, "connect", NULL);
	if (!command_connect)
		command_connect = g_strdup ("pppon");
	command_disconnect = panel_applet_gconf_get_string(applet, "disconnect", NULL);
	if (!command_disconnect)
		command_disconnect = g_strdup ("pppoff");
	ask_for_confirmation = panel_applet_gconf_get_int(applet, "confirmation", NULL);
	device_name          = panel_applet_gconf_get_string(applet, "device", NULL);
	if (!device_name)
		device_name = g_strdup ("ppp0");
	use_ISDN	   = panel_applet_gconf_get_int(applet, "isdn", NULL);
       	verify_lock_file   = panel_applet_gconf_get_int(applet, "verify_lock", NULL);
	show_extra_info    = panel_applet_gconf_get_int(applet, "extra_info", NULL);
	status_wait_blink  = panel_applet_gconf_get_int(applet, "wait_blink", NULL);
	
	for (i = 0; i < COLOR_COUNT; i++)
		{
		g_free(display_color_text[i]);
		
		display_color_text[i] = panel_applet_gconf_get_string(applet, 
							              color_rc_names[i], 
							              NULL);
		

		}

}

static void connect_changed_cb(GtkEntry *entry, GdkEventFocus *event, gpointer data)
{
	PanelApplet *applet = data;
	
	if (command_connect) g_free(command_connect);
	command_connect = gtk_editable_get_chars(GTK_EDITABLE(entry), 0, -1);
	panel_applet_gconf_set_string (applet, "connect", command_connect, NULL);
}

static void disconnect_changed_cb(GtkEntry *entry, GdkEventFocus *event, gpointer data)
{
	PanelApplet *applet = data;
	
	if (command_disconnect) g_free(command_disconnect);
	command_disconnect = gtk_editable_get_chars(GTK_EDITABLE(entry), 0, -1);
	panel_applet_gconf_set_string (applet, "disconnect", command_disconnect, NULL);
}

static void lockfile_changed_cb(GtkEntry *entry, GdkEventFocus *event, gpointer data)
{
	PanelApplet *applet = data;
	
	if (lock_file) g_free(lock_file);
	lock_file = gtk_editable_get_chars(GTK_EDITABLE(entry), 0, -1);
	panel_applet_gconf_set_string (applet, "lockfile", lock_file, NULL);
}

static void device_changed_cb(GtkEntry *entry, GdkEventFocus *event, gpointer data)
{
	PanelApplet *applet = data;
	
	if (device_name) g_free(device_name);
	device_name = gtk_editable_get_chars(GTK_EDITABLE(entry), 0, -1);
	panel_applet_gconf_set_string (applet, "device", device_name, NULL);
}

static void show_extra_info_cb(GtkWidget *widget, gpointer data)
{
	PanelApplet *applet = data;
	
	gint tmp = GTK_TOGGLE_BUTTON (widget)->active;
	if (tmp != show_extra_info)
		{
		show_extra_info = tmp;
		/* change display */
		reset_orientation();
		panel_applet_gconf_set_int (applet, "extra_info", show_extra_info, NULL);
		}
}

static void verify_lock_file_cb(GtkWidget *widget, gpointer data)
{
	PanelApplet *applet = data;
	
	verify_lock_file = GTK_TOGGLE_BUTTON (widget)->active;
	panel_applet_gconf_set_int (applet, "verify_lock", verify_lock_file, NULL);

}

static void update_delay_cb(GtkWidget *widget, GdkEventFocus *event, gpointer data)
{
	PanelApplet *applet = data;
	gchar *key;
	
        UPDATE_DELAY = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
        start_callback_update();
        panel_applet_gconf_set_int (applet, "delay", UPDATE_DELAY, NULL);
}

static void confirm_checkbox_cb(GtkWidget *widget, gpointer data)
{
	PanelApplet *applet = data;
	
	ask_for_confirmation = GTK_TOGGLE_BUTTON (widget)->active;
	panel_applet_gconf_set_int (applet, "confirmation", ask_for_confirmation, NULL);
}

static void wait_blink_cb(GtkWidget *widget, gpointer data)
{
	PanelApplet *applet = data;
	
	status_wait_blink = GTK_TOGGLE_BUTTON (widget)->active;
	panel_applet_gconf_set_int (applet, "wait_blink", status_wait_blink, NULL);
}

static void isdn_checkbox_cb(GtkWidget *widget, gpointer data)
{
	PanelApplet *applet = data;
	
	use_ISDN = GTK_TOGGLE_BUTTON (widget)->active;

	gtk_widget_set_sensitive(lockfile_entry, !use_ISDN);
	gtk_widget_set_sensitive(device_entry, !use_ISDN);
	gtk_widget_set_sensitive(verify_checkbox, !use_ISDN);
	panel_applet_gconf_set_int (applet, "isdn", use_ISDN, NULL);
}

static void set_default_cb(GtkWidget *widget, gpointer data)
{
	
}

static void box_color_cb(GnomeColorPicker *cp, guint nopr, guint nopg, guint nopb, guint nopa, gpointer data)
{
	PanelApplet *applet;
	ColorType color	= (ColorType)GPOINTER_TO_INT(data);
	guint8 r, g, b, a;

	gnome_color_picker_get_i8 (GNOME_COLOR_PICKER(cp), &r, &g, &b, &a);

	g_free(display_color_text[color]);
	display_color_text[color] = g_strdup_printf("#%06X", (r << 16) + (g << 8) + b);
	
	reset_colors ();
	
	applet = g_object_get_data (G_OBJECT (cp), "applet");
	panel_applet_gconf_set_string (applet, color_rc_names[color], 
				       display_color_text[color], NULL);
}

static GtkWidget *box_add_color(PanelApplet *applet, GtkWidget *box, 
				const gchar *text, ColorType color)
{
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *color_sel;
	GdkColor c;

	hbox = gtk_hbox_new (FALSE, 12);
	gtk_box_pack_start (GTK_BOX(box), hbox, FALSE, FALSE, 0);
	gtk_widget_show (hbox);

	if (display_color_text[color] != NULL)
		{
		gdk_color_parse(display_color_text[color], &c);
		}
	else
		{
		c.red = c.green = c.blue = 0;
		}

	label = gtk_label_new_with_mnemonic (text);
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	gtk_widget_show (label);

	color_sel = gnome_color_picker_new();
	gnome_color_picker_set_use_alpha(GNOME_COLOR_PICKER(color_sel), FALSE);
	gnome_color_picker_set_i16(GNOME_COLOR_PICKER(color_sel), c.red, c.green, c.blue, 0);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), color_sel);
	g_signal_connect(G_OBJECT(color_sel), "color_set", 
			 G_CALLBACK (box_color_cb), GINT_TO_POINTER((gint)color));
	g_object_set_data (G_OBJECT (color_sel), "applet", applet);
	gtk_box_pack_start(GTK_BOX(hbox), color_sel, FALSE, FALSE, 0);
	gtk_widget_show(color_sel);

	return label;
}

static GtkWidget *create_hig_category (GtkWidget *main_box, gchar *title)
{
	GtkWidget *vbox, *vbox2, *hbox;
	GtkWidget *label;
	gchar *tmp;
		
	vbox = gtk_vbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (main_box), vbox, FALSE, FALSE, 0);
	
	tmp = g_strdup_printf ("<b>%s</b>", title);
	label = gtk_label_new (NULL);
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_label_set_markup (GTK_LABEL (label), tmp);
	g_free (tmp);
	gtk_box_pack_start (GTK_BOX (vbox), label, TRUE, FALSE, 0);

	hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
	
	label = gtk_label_new ("    ");
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	
	vbox2 = gtk_vbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (hbox), vbox2, TRUE, TRUE, 0);

	return vbox2;	
}

static GtkWidget *color_frame_new(GtkWidget *vbox, const gchar *text)
	{
	GtkWidget *category;
	GtkWidget *hbox;

	category = create_hig_category (vbox, (gchar *)text);
	gtk_widget_show (category);

	hbox = gtk_hbox_new (FALSE, 12);
	gtk_box_pack_start (GTK_BOX (category), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

	return hbox;
	}

static void
phelp_cb (GtkDialog *dialog)
{
	GError *error = NULL;

	egg_help_display_on_screen (
		"modemlights_applet", "modemlights-prefs",
		gtk_window_get_screen (GTK_WINDOW (dialog)),
		&error);

	if (error) {
		g_warning ("help error: %s\n", error->message);
		g_error_free (error);
		error = NULL;
	}
}

static void
property_response_cb (GtkDialog *dialog, gint id, gpointer data)
{
	if (id == GTK_RESPONSE_HELP) {
		phelp_cb (dialog);
		return;
	}
	
	gtk_widget_destroy (GTK_WIDGET (dialog));
	propwindow = NULL;
}

void property_show (BonoboUIComponent *uic,
		    PanelApplet       *applet,
		    const gchar       *verbname)
{
	GtkWidget *frame;
	GtkWidget *hbox, *hbox2, *hbox3;
	GtkWidget *notebook;
	GtkWidget *vbox, *vbox2;
	GtkWidget *label;
	GtkWidget *delay_w;
	GtkObject *delay_adj;
	GtkWidget *checkbox;
	GtkSizeGroup *size_group;

	if (propwindow) {
		gtk_window_set_screen (GTK_WINDOW (propwindow),
				       gtk_widget_get_screen (GTK_WIDGET (applet)));
		gtk_window_present (GTK_WINDOW (propwindow));
		return;
	}

        propwindow = gtk_dialog_new_with_buttons (_("Modem Lights Preferences"), NULL,
						  GTK_DIALOG_DESTROY_WITH_PARENT,
						  GTK_STOCK_HELP, GTK_RESPONSE_HELP,
						  GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
						  NULL);
	gtk_window_set_screen (GTK_WINDOW (propwindow),
			       gtk_widget_get_screen (GTK_WIDGET (applet)));
	gtk_dialog_set_default_response (GTK_DIALOG (propwindow), GTK_RESPONSE_CLOSE);
  	gtk_dialog_set_has_separator (GTK_DIALOG (propwindow), FALSE);
	gtk_window_set_resizable (GTK_WINDOW (propwindow), FALSE);

	notebook = gtk_notebook_new ();
	vbox = gtk_vbox_new (FALSE, 6);
	gtk_container_set_border_width (GTK_CONTAINER (notebook), 12);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (propwindow)->vbox), notebook,
			    TRUE, TRUE, 0);
	
	vbox = gtk_vbox_new (FALSE, 18);
	gtk_container_set_border_width (GTK_CONTAINER(vbox), 12);

	frame = create_hig_category (vbox, _("Display"));

	/* update adjustment */
	hbox = gtk_hbox_new (FALSE, 12);
	gtk_box_pack_start (GTK_BOX (frame), hbox, FALSE, FALSE, 0);
	gtk_widget_show (hbox);

        label = gtk_label_new_with_mnemonic (_("U_pdate every:"));
        gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	gtk_widget_show (label);

	delay_adj = gtk_adjustment_new( UPDATE_DELAY, 1.0, 20.0, 1, 1, 1 );
        delay_w  = gtk_spin_button_new( GTK_ADJUSTMENT(delay_adj), 1, 0 );
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), delay_w);
	gtk_box_pack_start (GTK_BOX (hbox), delay_w, FALSE, FALSE, 0);
	g_signal_connect (G_OBJECT (delay_w), "focus-out-event",
			  G_CALLBACK (update_delay_cb), applet);
	gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (delay_w),GTK_UPDATE_ALWAYS );
	gtk_widget_show (delay_w);

	label = gtk_label_new_with_mnemonic (_("seconds"));
        gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	gtk_widget_show (label);

	/* extra info checkbox */
	checkbox = gtk_check_button_new_with_mnemonic (_("Sho_w connect time and throughput"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbox), show_extra_info);
	g_signal_connect (G_OBJECT (checkbox), "toggled",
			 G_CALLBACK (show_extra_info_cb), applet);
	gtk_box_pack_start (GTK_BOX (frame), checkbox, FALSE, FALSE, 0);
	gtk_widget_show (checkbox);

	checkbox = gtk_check_button_new_with_mnemonic (_("B_link connection status when connecting"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbox), status_wait_blink);
	g_signal_connect (G_OBJECT (checkbox), "toggled",
			 G_CALLBACK (wait_blink_cb), applet);
	gtk_box_pack_start (GTK_BOX (frame), checkbox, FALSE, FALSE, 0);
	gtk_widget_show (checkbox);

	frame = create_hig_category (vbox, _("Connections"));

	size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

	/* connect entry */
	hbox = gtk_hbox_new (FALSE, 12);
	gtk_box_pack_start (GTK_BOX (frame), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

	label = gtk_label_new_with_mnemonic (_("Co_nnection command:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_size_group_add_widget (size_group, label);
        gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_widget_show(label);

	connect_entry = gtk_entry_new_with_max_length(255);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), connect_entry);
	gtk_entry_set_text(GTK_ENTRY(connect_entry), command_connect);
	g_signal_connect (G_OBJECT (connect_entry), "focus_out_event",
			  G_CALLBACK (connect_changed_cb), applet);			  
        gtk_box_pack_start(GTK_BOX(hbox), connect_entry , TRUE, TRUE, 0);
	gtk_widget_show(connect_entry);

	/* disconnect entry */
	hbox = gtk_hbox_new(FALSE, 12);
        gtk_box_pack_start(GTK_BOX(frame), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

        label = gtk_label_new_with_mnemonic(_("_Disconnection command:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_size_group_add_widget (size_group, label);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	gtk_widget_show(label);

	disconnect_entry = gtk_entry_new_with_max_length(255);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), disconnect_entry);
	gtk_entry_set_text(GTK_ENTRY(disconnect_entry), command_disconnect);
	g_signal_connect (G_OBJECT (disconnect_entry), "focus_out_event",
			  G_CALLBACK (disconnect_changed_cb), applet);
        gtk_box_pack_start(GTK_BOX(hbox), disconnect_entry, TRUE, TRUE, 0);
	gtk_widget_show(disconnect_entry);

	/* confirmation checkbox */
	checkbox = gtk_check_button_new_with_mnemonic(_("Con_firm connection"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbox), ask_for_confirmation);
	g_signal_connect (G_OBJECT (checkbox), "toggled",
			  G_CALLBACK (confirm_checkbox_cb), applet);
	gtk_box_pack_start (GTK_BOX (frame), checkbox, FALSE, FALSE, 0);
	gtk_widget_show(checkbox);

	label = gtk_label_new (_("General"));
	gtk_widget_show (vbox);
        gtk_notebook_append_page (GTK_NOTEBOOK (notebook), vbox, label);

	/* color settings */

	vbox = gtk_vbox_new (FALSE, 18);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 12);

	hbox = color_frame_new (vbox, _("Receive Data"));
	size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

	hbox2 = gtk_hbox_new (FALSE, 24);
	gtk_box_pack_start (GTK_BOX (hbox), hbox2, FALSE, FALSE, 0);

	hbox3 = box_add_color (applet, hbox2, _("_Foreground:"), COLOR_RX);
	gtk_size_group_add_widget (size_group, hbox3);
	hbox3 = box_add_color (applet, hbox2, _("_Background:"), COLOR_RX_BG);
	gtk_size_group_add_widget (size_group, hbox3);
	
	hbox = color_frame_new(vbox, _("Send Data"));
	
	hbox2 = gtk_hbox_new (FALSE, 24);
	gtk_box_pack_start (GTK_BOX (hbox), hbox2, FALSE, FALSE, 0);
	
	hbox3 = box_add_color (applet, hbox2, _("Foregroun_d:"), COLOR_TX);
	gtk_size_group_add_widget (size_group, hbox3);
	hbox3 = box_add_color (applet, hbox2, _("Backg_round:"), COLOR_TX_BG);
	gtk_size_group_add_widget (size_group, hbox3);
	
	hbox = color_frame_new (vbox, _("Connection Status"));
	
	vbox2 = gtk_vbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (hbox), vbox2, FALSE, FALSE, 0);
	
	hbox2 = gtk_hbox_new (FALSE, 24);
	gtk_box_pack_start (GTK_BOX (vbox2), hbox2, FALSE, FALSE, 0);
	
	hbox3 = box_add_color (applet, hbox2, _("Co_nnected:"), COLOR_STATUS_OK);
	gtk_size_group_add_widget (size_group, hbox3);
	hbox3 = box_add_color (applet, hbox2, _("Disconnec_ted:"), COLOR_STATUS_BG);
	gtk_size_group_add_widget (size_group, hbox3);
		
	hbox3 = box_add_color (applet, vbox2, _("C_onnecting:"), COLOR_STATUS_WAIT);
	gtk_size_group_add_widget (size_group, hbox3);
	
	hbox = color_frame_new (vbox, _("Text"));
	
	vbox2 = gtk_vbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (hbox), vbox2, FALSE, FALSE, 0);
	
	hbox2 = gtk_hbox_new (FALSE, 24);
	gtk_box_pack_start (GTK_BOX (vbox2), hbox2, FALSE, FALSE, 0);
	
	hbox3 = box_add_color (applet, hbox2, _("For_eground:"), COLOR_TEXT_FG);
	gtk_size_group_add_widget (size_group, hbox3);
	hbox3 = box_add_color (applet, hbox2, _("Bac_kground:"), COLOR_TEXT_BG);
	gtk_size_group_add_widget (size_group, hbox3);
	hbox3 = box_add_color (applet, vbox2, _("O_utline"), COLOR_TEXT_MID);
        gtk_size_group_add_widget (size_group, hbox3);

	label = gtk_label_new (_("Colors"));
	gtk_widget_show (vbox);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), vbox, label);

	/* advanced settings */

	vbox = gtk_vbox_new (FALSE, 18);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 12);

	frame = create_hig_category (vbox, _("Modem Options"));

	size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

	/* device entry */
	hbox = gtk_hbox_new (FALSE, 12);
        gtk_box_pack_start (GTK_BOX (frame), hbox, FALSE, FALSE, 0);
	gtk_widget_show (hbox);

        label = gtk_label_new_with_mnemonic (_("_Device:"));
	gtk_size_group_add_widget (size_group, label);
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
        gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	gtk_widget_show (label);

	device_entry = gtk_entry_new_with_max_length (16);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), device_entry);
	gtk_entry_set_text (GTK_ENTRY (device_entry), device_name);
	g_signal_connect (G_OBJECT (device_entry), "focus_out_event",
			  G_CALLBACK (device_changed_cb), applet);
        gtk_box_pack_start (GTK_BOX (hbox), device_entry, TRUE, TRUE, 0);
	gtk_widget_show (device_entry);
	
	/* lock file entry */
	hbox = gtk_hbox_new(FALSE, 12);
        gtk_box_pack_start(GTK_BOX(frame), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

        label = gtk_label_new_with_mnemonic (_("_Lock file:"));
	gtk_size_group_add_widget (size_group, label);
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
        gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	gtk_widget_show (label);

	lockfile_entry = gtk_entry_new_with_max_length(255);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), lockfile_entry);
	gtk_entry_set_text(GTK_ENTRY(lockfile_entry), lock_file);
	g_signal_connect (G_OBJECT (lockfile_entry), "focus_out_event",
			  G_CALLBACK (lockfile_changed_cb), applet);
        gtk_box_pack_start(GTK_BOX(hbox), lockfile_entry, TRUE, TRUE, 0);
	gtk_widget_show(lockfile_entry);

	verify_checkbox = gtk_check_button_new_with_mnemonic(_("_Verify owner of lock file"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (verify_checkbox), verify_lock_file);
	g_signal_connect(G_OBJECT(verify_checkbox), "toggled",
			 G_CALLBACK(verify_lock_file_cb), applet);
	gtk_box_pack_start (GTK_BOX (frame), verify_checkbox, FALSE, FALSE, 0);
	gtk_widget_show(verify_checkbox);

	/* ISDN checkbox */
	checkbox = gtk_check_button_new_with_mnemonic(_("U_se ISDN"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbox), use_ISDN);
	g_signal_connect(G_OBJECT(checkbox), "toggled",
			   G_CALLBACK(isdn_checkbox_cb), applet);
	gtk_box_pack_start (GTK_BOX (frame), checkbox, FALSE, FALSE, 0);
	gtk_widget_show(checkbox);

	if (use_ISDN)
		{
		gtk_widget_set_sensitive(lockfile_entry, FALSE);
		gtk_widget_set_sensitive(device_entry, FALSE);
		gtk_widget_set_sensitive(verify_checkbox, FALSE);
		}
#ifdef SHOULD_THIS_BE_HERE
	/* defaults save button */
	
	hbox = gtk_hbox_new (FALSE, 12);
	gtk_box_pack_end (GTK_BOX (frame), hbox, FALSE, FALSE, 0);
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
} 
