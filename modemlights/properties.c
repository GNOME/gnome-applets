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

#define NEVER_SENSITIVE "never_sensitive"

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

/* set sensitive and setup NEVER_SENSITIVE appropriately */
static void
hard_set_sensitive (GtkWidget *w, gboolean sensitivity)
{
	gtk_widget_set_sensitive (w, sensitivity);
	g_object_set_data (G_OBJECT (w), NEVER_SENSITIVE,
			   GINT_TO_POINTER ( ! sensitivity));
}


/* set sensitive, but always insensitive if NEVER_SENSITIVE is set */
static void
soft_set_sensitive (GtkWidget *w, gboolean sensitivity)
{
	if (g_object_get_data (G_OBJECT (w), NEVER_SENSITIVE))
		gtk_widget_set_sensitive (w, FALSE);
	else
		gtk_widget_set_sensitive (w, sensitivity);
}


static gboolean
key_writable (PanelApplet *applet, const char *key)
{
	gboolean writable;
	char *fullkey;
	static GConfClient *client = NULL;
	if (client == NULL)
		client = gconf_client_get_default ();

	fullkey = panel_applet_gconf_get_full_key (applet, key);

	writable = gconf_client_key_is_writable (client, fullkey, NULL);

	g_free (fullkey);

	return writable;
}

void property_load(MLData *mldata)
{
	PanelApplet *applet = PANEL_APPLET (mldata->applet);
	GError *error = NULL;
	gchar *buf;
	gint i;

	g_free(mldata->command_connect);
	g_free(mldata->command_disconnect);
	g_free(mldata->device_name);

	mldata->UPDATE_DELAY = panel_applet_gconf_get_int (applet, "delay", &error);
	if (error) {
		g_print ("%s \n", error->message);
		g_error_free (error);
		error = NULL;
	}
	mldata->UPDATE_DELAY = MAX (mldata->UPDATE_DELAY, 1);
	
	buf = panel_applet_gconf_get_string(applet, "lockfile", NULL);
	if (buf && strlen(buf) > 0)
		{
		g_free(mldata->lock_file);
		mldata->lock_file = g_strdup(buf);
		}
	g_free(buf);

	mldata->command_connect    = panel_applet_gconf_get_string(applet, "connect", NULL);
	if (!mldata->command_connect)
		mldata->command_connect = g_strdup ("pppon");
	mldata->command_disconnect = panel_applet_gconf_get_string(applet, "disconnect", NULL);
	if (!mldata->command_disconnect)
		mldata->command_disconnect = g_strdup ("pppoff");
	mldata->ask_for_confirmation = panel_applet_gconf_get_int(applet, "confirmation", NULL);
	mldata->device_name = panel_applet_gconf_get_string(applet, "device", NULL);
	if (!mldata->device_name)
		mldata->device_name = g_strdup ("ppp0");
	mldata->use_ISDN = panel_applet_gconf_get_int(applet, "isdn", NULL);
       	mldata->verify_lock_file  = panel_applet_gconf_get_int(applet, "verify_lock", NULL);
	mldata->show_extra_info  = panel_applet_gconf_get_int(applet, "extra_info", NULL);
	mldata->status_wait_blink  = panel_applet_gconf_get_int(applet, "wait_blink", NULL);
	
	for (i = 0; i < COLOR_COUNT; i++)
		{
		g_free(mldata->display_color_text[i]);
		
		mldata->display_color_text[i] = panel_applet_gconf_get_string(applet, 
							              color_rc_names[i], 
							              NULL);
		

		}

}

static void connect_changed_cb(GtkEntry *entry, GdkEventFocus *event, gpointer data)
{
	MLData *mldata = data;
	PanelApplet *applet = PANEL_APPLET (mldata->applet);
	
	if (mldata->command_connect) g_free(mldata->command_connect);
	mldata->command_connect = gtk_editable_get_chars(GTK_EDITABLE(entry), 0, -1);
	panel_applet_gconf_set_string (applet, "connect", mldata->command_connect, NULL);
}

static void disconnect_changed_cb(GtkEntry *entry, GdkEventFocus *event, gpointer data)
{
	MLData *mldata = data;
	PanelApplet *applet = PANEL_APPLET (mldata->applet);
	
	if (mldata->command_disconnect) g_free(mldata->command_disconnect);
	mldata->command_disconnect = gtk_editable_get_chars(GTK_EDITABLE(entry), 0, -1);
	panel_applet_gconf_set_string (applet, "disconnect", mldata->command_disconnect, NULL);
}

static void lockfile_changed_cb(GtkEntry *entry, GdkEventFocus *event, gpointer data)
{
	MLData *mldata = data;
	PanelApplet *applet = PANEL_APPLET (mldata->applet);
	
	if (mldata->lock_file) g_free(mldata->lock_file);
	mldata->lock_file = gtk_editable_get_chars(GTK_EDITABLE(entry), 0, -1);
	panel_applet_gconf_set_string (applet, "lockfile", mldata->lock_file, NULL);
}

static void device_changed_cb(GtkEntry *entry, GdkEventFocus *event, gpointer data)
{
	MLData *mldata = data;
	PanelApplet *applet = PANEL_APPLET (mldata->applet);
	
	if (mldata->device_name) g_free(mldata->device_name);
	mldata->device_name = gtk_editable_get_chars(GTK_EDITABLE(entry), 0, -1);
	panel_applet_gconf_set_string (applet, "device", mldata->device_name, NULL);
}

static void show_extra_info_cb(GtkWidget *widget, gpointer data)
{
	MLData *mldata = data;
	PanelApplet *applet = PANEL_APPLET (mldata->applet);
	
	gint tmp = GTK_TOGGLE_BUTTON (widget)->active;
	if (tmp != mldata->show_extra_info)
		{
		mldata->show_extra_info = tmp;
		/* change display */
		reset_orientation(mldata);
		panel_applet_gconf_set_int (applet, "extra_info", mldata->show_extra_info, NULL);
		}
}

static void verify_lock_file_cb(GtkWidget *widget, gpointer data)
{
	MLData *mldata = data;
	PanelApplet *applet = PANEL_APPLET (mldata->applet);
	
	mldata->verify_lock_file = GTK_TOGGLE_BUTTON (widget)->active;
	panel_applet_gconf_set_int (applet, "verify_lock", mldata->verify_lock_file, NULL);

}

static void update_delay_cb(GtkWidget *widget, GdkEventFocus *event, gpointer data)
{
	MLData *mldata = data;
	PanelApplet *applet = PANEL_APPLET (mldata->applet);
	gchar *key;
	
        mldata->UPDATE_DELAY = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
        start_callback_update(mldata);
        panel_applet_gconf_set_int (applet, "delay", mldata->UPDATE_DELAY, NULL);
}

static void confirm_checkbox_cb(GtkWidget *widget, gpointer data)
{
	MLData *mldata = data;
	PanelApplet *applet = PANEL_APPLET (mldata->applet);
	
	mldata->ask_for_confirmation = GTK_TOGGLE_BUTTON (widget)->active;
	panel_applet_gconf_set_int (applet, "confirmation", mldata->ask_for_confirmation, NULL);
}

static void wait_blink_cb(GtkWidget *widget, gpointer data)
{
	MLData *mldata = data;
	PanelApplet *applet = PANEL_APPLET (mldata->applet);
	
	mldata->status_wait_blink = GTK_TOGGLE_BUTTON (widget)->active;
	panel_applet_gconf_set_int (applet, "wait_blink", mldata->status_wait_blink, NULL);
}

static void isdn_checkbox_cb(GtkWidget *widget, gpointer data)
{
	MLData *mldata = data;
	PanelApplet *applet = PANEL_APPLET (mldata->applet);
	
	mldata->use_ISDN = GTK_TOGGLE_BUTTON (widget)->active;

	soft_set_sensitive(mldata->lockfile_entry, !mldata->use_ISDN);
	soft_set_sensitive(mldata->device_entry, !mldata->use_ISDN);
	soft_set_sensitive(mldata->verify_checkbox, !mldata->use_ISDN);
	panel_applet_gconf_set_int (applet, "isdn", mldata->use_ISDN, NULL);
}

static void set_default_cb(GtkWidget *widget, gpointer data)
{
	
}

static void box_color_cb(GtkColorButton *cp, gpointer data)
{
	MLData *mldata;
	PanelApplet *applet;
	ColorType color	= (ColorType)GPOINTER_TO_INT(data);
	GdkColor new_color;

	gtk_color_button_get_color(cp, &new_color);
	mldata = g_object_get_data (G_OBJECT (cp), "mldata");
	
	g_free(mldata->display_color_text[color]);
	mldata->display_color_text[color] = g_strdup_printf("#%02X%02X%02X",
							    new_color.red / 256,
							    new_color.green / 256,
							    new_color.blue / 256);
	
	reset_colors (mldata);
	
	applet = PANEL_APPLET (mldata->applet);
	panel_applet_gconf_set_string (applet, color_rc_names[color], 
				       mldata->display_color_text[color], NULL);
}

static GtkWidget *box_add_color(MLData *mldata, GtkWidget *box, 
				const gchar *text, ColorType color)
{
	PanelApplet *applet = PANEL_APPLET (mldata->applet);
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *color_sel;
	GdkColor c;

	hbox = gtk_hbox_new (FALSE, 12);
	gtk_box_pack_start (GTK_BOX(box), hbox, FALSE, FALSE, 0);
	gtk_widget_show (hbox);

	if (mldata->display_color_text[color] != NULL)
		{
		gdk_color_parse(mldata->display_color_text[color], &c);
		}
	else
		{
		c.red = c.green = c.blue = 0;
		}

	label = gtk_label_new_with_mnemonic (text);
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	gtk_widget_show (label);

	color_sel = gtk_color_button_new();
	gtk_color_button_set_color(GTK_COLOR_BUTTON(color_sel), &c);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), color_sel);
	g_signal_connect(G_OBJECT(color_sel), "color_set", 
			 G_CALLBACK (box_color_cb), GINT_TO_POINTER((gint)color));
	g_object_set_data (G_OBJECT (color_sel), "mldata", mldata);
	gtk_box_pack_start(GTK_BOX(hbox), color_sel, FALSE, FALSE, 0);
	gtk_widget_show(color_sel);

	if ( ! key_writable (PANEL_APPLET (mldata->applet), color_rc_names[color]))
		hard_set_sensitive (hbox, FALSE);

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
		"modemlights", "modemlights-prefs",
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
	MLData *mldata = data;
	if (id == GTK_RESPONSE_HELP) {
		phelp_cb (dialog);
		return;
	}
	
	gtk_widget_destroy (GTK_WIDGET (dialog));
	mldata->propwindow = NULL;
}

void property_show (BonoboUIComponent *uic,
		    MLData       *mldata,
		    const gchar       *verbname)
{
	PanelApplet *applet = PANEL_APPLET (mldata->applet);
	GtkWidget *frame;
	GtkWidget *hbox, *hbox2, *hbox3;
	GtkWidget *notebook;
	GtkWidget *vbox, *vbox2;
	GtkWidget *label;
	GtkWidget *delay_w;
	GtkObject *delay_adj;
	GtkWidget *checkbox;
	GtkSizeGroup *size_group;
	GConfClient *client;
	gboolean inhibit_command_line;

	if (mldata->propwindow) {
		gtk_window_set_screen (GTK_WINDOW (mldata->propwindow),
				       gtk_widget_get_screen (GTK_WIDGET (applet)));
		gtk_window_present (GTK_WINDOW (mldata->propwindow));
		return;
	}

	client = gconf_client_get_default ();
	inhibit_command_line = gconf_client_get_bool (client, "/desktop/gnome/lockdown/inhibit_command_line", NULL);

        mldata->propwindow = gtk_dialog_new_with_buttons (_("Modem Lights Preferences"), NULL,
						  GTK_DIALOG_DESTROY_WITH_PARENT,
						  GTK_STOCK_HELP, GTK_RESPONSE_HELP,
						  GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
						  NULL);
	gtk_window_set_screen (GTK_WINDOW (mldata->propwindow),
			       gtk_widget_get_screen (GTK_WIDGET (applet)));
	gtk_dialog_set_default_response (GTK_DIALOG (mldata->propwindow), GTK_RESPONSE_CLOSE);
  	gtk_dialog_set_has_separator (GTK_DIALOG (mldata->propwindow), FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (mldata->propwindow), 5);
	gtk_box_set_spacing (GTK_BOX (GTK_DIALOG (mldata->propwindow)->vbox), 2);
	gtk_window_set_resizable (GTK_WINDOW (mldata->propwindow), FALSE);

	notebook = gtk_notebook_new ();
	gtk_container_set_border_width (GTK_CONTAINER (notebook), 5);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (mldata->propwindow)->vbox), notebook,
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

	hbox2 = gtk_hbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (hbox), hbox2, FALSE, FALSE, 0);
	gtk_widget_show (hbox2);

	delay_adj = gtk_adjustment_new( mldata->UPDATE_DELAY, 1.0, 20.0, 1, 1, 1 );
        delay_w  = gtk_spin_button_new( GTK_ADJUSTMENT(delay_adj), 1, 0 );
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), delay_w);
	gtk_box_pack_start (GTK_BOX (hbox2), delay_w, FALSE, FALSE, 0);
	g_signal_connect (G_OBJECT (delay_w), "focus-out-event",
			  G_CALLBACK (update_delay_cb), mldata);
	gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (delay_w),GTK_UPDATE_ALWAYS );
	gtk_widget_show (delay_w);

	label = gtk_label_new_with_mnemonic (_("seconds"));
        gtk_box_pack_start (GTK_BOX (hbox2), label, FALSE, FALSE, 0);
	gtk_widget_show (label);

	if ( ! key_writable (PANEL_APPLET (mldata->applet), "delay"))
		hard_set_sensitive (hbox, FALSE);

	/* extra info checkbox */
	checkbox = gtk_check_button_new_with_mnemonic (_("Sho_w connect time and throughput"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbox), mldata->show_extra_info);
	g_signal_connect (G_OBJECT (checkbox), "toggled",
			 G_CALLBACK (show_extra_info_cb), mldata);
	gtk_box_pack_start (GTK_BOX (frame), checkbox, FALSE, FALSE, 0);
	gtk_widget_show (checkbox);

	if ( ! key_writable (PANEL_APPLET (mldata->applet), "extra_info"))
		hard_set_sensitive (checkbox, FALSE);

	checkbox = gtk_check_button_new_with_mnemonic (_("B_link connection status when connecting"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbox), mldata->status_wait_blink);
	g_signal_connect (G_OBJECT (checkbox), "toggled",
			 G_CALLBACK (wait_blink_cb), mldata);
	gtk_box_pack_start (GTK_BOX (frame), checkbox, FALSE, FALSE, 0);
	gtk_widget_show (checkbox);

	if ( ! key_writable (PANEL_APPLET (mldata->applet), "wait_blink"))
		hard_set_sensitive (checkbox, FALSE);

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

	mldata->connect_entry = gtk_entry_new ();
	gtk_entry_set_max_length (GTK_ENTRY (mldata->connect_entry), 255);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), mldata->connect_entry);
	gtk_entry_set_text(GTK_ENTRY(mldata->connect_entry), mldata->command_connect);
	g_signal_connect (G_OBJECT (mldata->connect_entry), "focus_out_event",
			  G_CALLBACK (connect_changed_cb), mldata);			  
        gtk_box_pack_start(GTK_BOX(hbox), mldata->connect_entry , TRUE, TRUE, 0);
	gtk_widget_show(mldata->connect_entry);

	if ( ! key_writable (PANEL_APPLET (mldata->applet), "connect") ||
	    inhibit_command_line) {
		hard_set_sensitive (label, FALSE);
		hard_set_sensitive (mldata->connect_entry, FALSE);
	}

	/* disconnect entry */
	hbox = gtk_hbox_new(FALSE, 12);
        gtk_box_pack_start(GTK_BOX(frame), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

        label = gtk_label_new_with_mnemonic(_("_Disconnection command:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_size_group_add_widget (size_group, label);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	gtk_widget_show(label);

	mldata->disconnect_entry = gtk_entry_new ();
	gtk_entry_set_max_length (GTK_ENTRY (mldata->disconnect_entry), 255);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), mldata->disconnect_entry);
	gtk_entry_set_text(GTK_ENTRY(mldata->disconnect_entry), mldata->command_disconnect);
	g_signal_connect (G_OBJECT (mldata->disconnect_entry), "focus_out_event",
			  G_CALLBACK (disconnect_changed_cb), mldata);
        gtk_box_pack_start(GTK_BOX(hbox), mldata->disconnect_entry, TRUE, TRUE, 0);
	gtk_widget_show(mldata->disconnect_entry);

	if ( ! key_writable (PANEL_APPLET (mldata->applet), "disconnect") ||
	    inhibit_command_line) {
		hard_set_sensitive (label, FALSE);
		hard_set_sensitive (mldata->disconnect_entry, FALSE);
	}

	/* confirmation checkbox */
	checkbox = gtk_check_button_new_with_mnemonic(_("Con_firm connection"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbox), mldata->ask_for_confirmation);
	g_signal_connect (G_OBJECT (checkbox), "toggled",
			  G_CALLBACK (confirm_checkbox_cb), mldata);
	gtk_box_pack_start (GTK_BOX (frame), checkbox, FALSE, FALSE, 0);
	gtk_widget_show(checkbox);

	if ( ! key_writable (PANEL_APPLET (mldata->applet), "confirmation"))
		hard_set_sensitive (checkbox, FALSE);

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

	hbox3 = box_add_color (mldata, hbox2, _("_Foreground:"), COLOR_RX);
	gtk_size_group_add_widget (size_group, hbox3);
	hbox3 = box_add_color (mldata, hbox2, _("_Background:"), COLOR_RX_BG);
	gtk_size_group_add_widget (size_group, hbox3);
	
	hbox = color_frame_new(vbox, _("Send Data"));
	
	hbox2 = gtk_hbox_new (FALSE, 24);
	gtk_box_pack_start (GTK_BOX (hbox), hbox2, FALSE, FALSE, 0);
	
	hbox3 = box_add_color (mldata, hbox2, _("Foregroun_d:"), COLOR_TX);
	gtk_size_group_add_widget (size_group, hbox3);
	hbox3 = box_add_color (mldata, hbox2, _("Backg_round:"), COLOR_TX_BG);
	gtk_size_group_add_widget (size_group, hbox3);
	
	hbox = color_frame_new (vbox, _("Connection Status"));
	
	vbox2 = gtk_vbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (hbox), vbox2, FALSE, FALSE, 0);
	
	hbox2 = gtk_hbox_new (FALSE, 24);
	gtk_box_pack_start (GTK_BOX (vbox2), hbox2, FALSE, FALSE, 0);
	
	hbox3 = box_add_color (mldata, hbox2, _("Co_nnected:"), COLOR_STATUS_OK);
	gtk_size_group_add_widget (size_group, hbox3);
	hbox3 = box_add_color (mldata, hbox2, _("Disconnec_ted:"), COLOR_STATUS_BG);
	gtk_size_group_add_widget (size_group, hbox3);
		
	hbox3 = box_add_color (mldata, vbox2, _("C_onnecting:"), COLOR_STATUS_WAIT);
	gtk_size_group_add_widget (size_group, hbox3);
	
	hbox = color_frame_new (vbox, _("Text"));
	
	vbox2 = gtk_vbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (hbox), vbox2, FALSE, FALSE, 0);
	
	hbox2 = gtk_hbox_new (FALSE, 24);
	gtk_box_pack_start (GTK_BOX (vbox2), hbox2, FALSE, FALSE, 0);
	
	hbox3 = box_add_color (mldata, hbox2, _("For_eground:"), COLOR_TEXT_FG);
	gtk_size_group_add_widget (size_group, hbox3);
	hbox3 = box_add_color (mldata, hbox2, _("Bac_kground:"), COLOR_TEXT_BG);
	gtk_size_group_add_widget (size_group, hbox3);
	hbox3 = box_add_color (mldata, vbox2, _("O_utline:"), COLOR_TEXT_MID);
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

	mldata->device_entry = gtk_entry_new ();
	gtk_entry_set_max_length (GTK_ENTRY(mldata->device_entry), 16);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), mldata->device_entry);
	gtk_entry_set_text (GTK_ENTRY (mldata->device_entry), mldata->device_name);
	g_signal_connect (G_OBJECT (mldata->device_entry), "focus_out_event",
			  G_CALLBACK (device_changed_cb), mldata);
        gtk_box_pack_start (GTK_BOX (hbox), mldata->device_entry, TRUE, TRUE, 0);
	gtk_widget_show (mldata->device_entry);

	if ( ! key_writable (PANEL_APPLET (mldata->applet), "device") ||
	    inhibit_command_line) {
		hard_set_sensitive (label, FALSE);
		hard_set_sensitive (mldata->device_entry, FALSE);
	}
	
	/* lock file entry */
	hbox = gtk_hbox_new(FALSE, 12);
        gtk_box_pack_start(GTK_BOX(frame), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

        label = gtk_label_new_with_mnemonic (_("_Lock file:"));
	gtk_size_group_add_widget (size_group, label);
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
        gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	gtk_widget_show (label);

	mldata->lockfile_entry = gtk_entry_new ();
	gtk_entry_set_max_length (GTK_ENTRY (mldata->lockfile_entry), 255);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), mldata->lockfile_entry);
	gtk_entry_set_text(GTK_ENTRY(mldata->lockfile_entry), mldata->lock_file);
	g_signal_connect (G_OBJECT (mldata->lockfile_entry), "focus_out_event",
			  G_CALLBACK (lockfile_changed_cb), mldata);
        gtk_box_pack_start(GTK_BOX(hbox), mldata->lockfile_entry, TRUE, TRUE, 0);
	gtk_widget_show(mldata->lockfile_entry);

	if ( ! key_writable (PANEL_APPLET (mldata->applet), "lockfile") ||
	    inhibit_command_line) {
		hard_set_sensitive (label, FALSE);
		hard_set_sensitive (mldata->lockfile_entry, FALSE);
	}

	mldata->verify_checkbox = gtk_check_button_new_with_mnemonic(_("_Verify owner of lock file"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (mldata->verify_checkbox), mldata->verify_lock_file);
	g_signal_connect(G_OBJECT(mldata->verify_checkbox), "toggled",
			 G_CALLBACK(verify_lock_file_cb), mldata);
	gtk_box_pack_start (GTK_BOX (frame), mldata->verify_checkbox, FALSE, FALSE, 0);
	gtk_widget_show(mldata->verify_checkbox);

	if ( ! key_writable (PANEL_APPLET (mldata->applet), "verify_lock"))
		hard_set_sensitive (mldata->verify_checkbox, FALSE);

	/* ISDN checkbox */
	checkbox = gtk_check_button_new_with_mnemonic(_("U_se ISDN"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbox), mldata->use_ISDN);
	g_signal_connect(G_OBJECT(checkbox), "toggled",
			   G_CALLBACK(isdn_checkbox_cb), mldata);
	gtk_box_pack_start (GTK_BOX (frame), checkbox, FALSE, FALSE, 0);
	gtk_widget_show(checkbox);

	if ( ! key_writable (PANEL_APPLET (mldata->applet), "isdn"))
		hard_set_sensitive (checkbox, FALSE);

	if (mldata->use_ISDN)
		{
		soft_set_sensitive(mldata->lockfile_entry, FALSE);
		soft_set_sensitive(mldata->device_entry, FALSE);
		soft_set_sensitive(mldata->verify_checkbox, FALSE);
		}

        label = gtk_label_new(_("Advanced"));
        gtk_widget_show(vbox);
        gtk_notebook_append_page( GTK_NOTEBOOK(notebook), vbox, label);

	g_signal_connect (G_OBJECT (mldata->propwindow), "response",
			  G_CALLBACK (property_response_cb), mldata);

        gtk_widget_show_all(mldata->propwindow);
} 
