/*###################################################################*/
/*##                         clock & mail applet 0.1.5             ##*/
/*###################################################################*/

#include "clockmail.h"

static void newmail_exec_cb(GtkWidget *w, gpointer data);
static void am_pm_time_cb(GtkWidget *w, gpointer data);
static void always_blink_cb(GtkWidget *w, gpointer data);
static void property_apply_cb(GtkWidget *widget, void *nodata, gpointer data);
static gint property_destroy_cb(GtkWidget *widget, gpointer data);

void property_load(gchar *path, AppData *ad)
{
        if (ad->mail_file) free(ad->mail_file);

	gnome_config_push_prefix (path);
        ad->am_pm_enable = gnome_config_get_int("clock/12hour=0");
	ad->always_blink = gnome_config_get_int("mail/blink=0");
	ad->mail_file = gnome_config_get_string("mail/mailfile=default");
	ad->newmail_exec_cmd = gnome_config_get_string("mail/newmail_command=");
	ad->exec_cmd_on_newmail = gnome_config_get_int("mail/newmail_command_enable=0");
        gnome_config_pop_prefix ();
}

void property_save(gchar *path, AppData *ad)
{
        gnome_config_push_prefix(path);
        gnome_config_set_int("clock/12hour", ad->am_pm_enable);
        gnome_config_set_int("mail/blink", ad->always_blink);
	gnome_config_set_string("mail/mailfile", ad->mail_file);
	gnome_config_set_string("mail/newmail_command", ad->newmail_exec_cmd);
        gnome_config_set_int("mail/newmail_command_enable",
			     ad->exec_cmd_on_newmail);
	gnome_config_sync();
        gnome_config_pop_prefix();
}

static void newmail_exec_cb(GtkWidget *w, gpointer data)
{
	AppData *ad = data;
	ad->p_exec_cmd_on_newmail = GTK_TOGGLE_BUTTON (w)->active;
	gnome_property_box_changed(GNOME_PROPERTY_BOX(ad->propwindow));
}

static void am_pm_time_cb(GtkWidget *w, gpointer data)
{
	AppData *ad = data;
	ad->p_am_pm_enable = GTK_TOGGLE_BUTTON (w)->active;
	gnome_property_box_changed(GNOME_PROPERTY_BOX(ad->propwindow));
}

static void always_blink_cb(GtkWidget *w, gpointer data)
{
	AppData *ad = data;
	ad->p_always_blink = GTK_TOGGLE_BUTTON (w)->active;
	gnome_property_box_changed(GNOME_PROPERTY_BOX(ad->propwindow));
}

static void property_apply_cb(GtkWidget *widget, void *nodata, gpointer data)
{
	AppData *ad = data;
	gchar *new_text;
	gchar *command;

	new_text = gtk_entry_get_text(GTK_ENTRY(ad->mail_file_entry));
	if (strcmp(new_text,ad->mail_file) != 0)
		{
		if (ad->mail_file) g_free(ad->mail_file);
		ad->mail_file = g_strdup(new_text);
		check_mail_file_status (TRUE, ad);
		}

	command = gtk_entry_get_text(GTK_ENTRY(ad->newmail_exec_cmd_entry));
	if (ad->newmail_exec_cmd) g_free (ad->newmail_exec_cmd);
	ad->newmail_exec_cmd = g_strdup(command);

        ad->am_pm_enable = ad->p_am_pm_enable;
	ad->always_blink = ad->p_always_blink;
	ad->exec_cmd_on_newmail = ad->p_exec_cmd_on_newmail;

	applet_widget_sync_config(APPLET_WIDGET(ad->applet));
}

static gint property_destroy_cb(GtkWidget *widget, gpointer data)
{
	AppData *ad = data;
	ad->propwindow = NULL;
	return FALSE;
}

void property_show(AppletWidget *applet, gpointer data)
{
	AppData *ad = data;
	GtkWidget *frame;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *button;

	if(ad->propwindow)
		{
                gdk_window_raise(ad->propwindow->window);
                return;
		}

        ad->p_am_pm_enable = ad->am_pm_enable;
	ad->p_always_blink = ad->always_blink;
	ad->p_exec_cmd_on_newmail = ad->exec_cmd_on_newmail;

	ad->propwindow = gnome_property_box_new();
	gtk_window_set_title(GTK_WINDOW(&GNOME_PROPERTY_BOX(ad->propwindow)->dialog.window),
		"ClockMail Settings");
	
	frame = gtk_vbox_new(5, TRUE);

	button = gtk_check_button_new_with_label (_("Display time in 12 hour format (AM/PM)"));
	gtk_box_pack_start(GTK_BOX(frame), button, FALSE, FALSE, 0);
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(button), ad->p_am_pm_enable);
	gtk_signal_connect (GTK_OBJECT(button),"clicked",(GtkSignalFunc) am_pm_time_cb, ad);
	gtk_widget_show(button);

	button = gtk_check_button_new_with_label (_("Blink when any mail is waiting. (Not just when mail arrives)"));
	gtk_box_pack_start(GTK_BOX(frame), button, FALSE, FALSE, 0);
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(button), ad->p_always_blink);
	gtk_signal_connect (GTK_OBJECT(button),"clicked",(GtkSignalFunc) always_blink_cb, ad);
	gtk_widget_show(button);

	/* mail file entry */
	hbox = gtk_hbox_new(FALSE, 5);
	gtk_box_pack_start( GTK_BOX(frame), hbox, FALSE, FALSE, 5);
	gtk_widget_show(hbox);

	label = gtk_label_new(_("Mail file:"));
	gtk_box_pack_start( GTK_BOX(hbox), label, FALSE, FALSE, 5);
	gtk_widget_show(label);

	ad->mail_file_entry = gtk_entry_new_with_max_length(255);
	gtk_entry_set_text(GTK_ENTRY(ad->mail_file_entry), ad->mail_file);
	gtk_signal_connect_object(GTK_OBJECT(ad->mail_file_entry), "changed",
				GTK_SIGNAL_FUNC(gnome_property_box_changed),
				GTK_OBJECT(ad->propwindow));
	gtk_box_pack_start( GTK_BOX(hbox),ad->mail_file_entry , TRUE, TRUE, 5);
	gtk_widget_show(ad->mail_file_entry);

	/* newmail exec command */
	hbox = gtk_hbox_new(FALSE, 5);
	gtk_box_pack_start( GTK_BOX(frame), hbox, FALSE, FALSE, 5);
	gtk_widget_show(hbox);

	button = gtk_check_button_new_with_label (_("When new mail is received run:"));
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 5);
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(button), ad->p_exec_cmd_on_newmail);
	gtk_signal_connect (GTK_OBJECT(button),"clicked",(GtkSignalFunc) newmail_exec_cb, ad);
	gtk_widget_show(button);

	ad->newmail_exec_cmd_entry = gtk_entry_new_with_max_length(255);
	if (ad->newmail_exec_cmd)
		gtk_entry_set_text(GTK_ENTRY(ad->newmail_exec_cmd_entry), ad->newmail_exec_cmd);
	gtk_signal_connect_object(GTK_OBJECT(ad->newmail_exec_cmd_entry), "changed",
				GTK_SIGNAL_FUNC(gnome_property_box_changed),
				GTK_OBJECT(ad->propwindow));
	gtk_box_pack_start( GTK_BOX(hbox),ad->newmail_exec_cmd_entry , TRUE, TRUE, 5);
	gtk_widget_show(ad->newmail_exec_cmd_entry);

        label = gtk_label_new(_("General"));
        gtk_widget_show(frame);
        gnome_property_box_append_page( GNOME_PROPERTY_BOX(ad->propwindow),frame ,label);

	gtk_signal_connect( GTK_OBJECT(ad->propwindow),"apply", GTK_SIGNAL_FUNC(property_apply_cb), ad);
	gtk_signal_connect( GTK_OBJECT(ad->propwindow),"destroy", GTK_SIGNAL_FUNC(property_destroy_cb), ad );

        gtk_widget_show_all(ad->propwindow);
} 
