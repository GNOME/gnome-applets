/*###################################################################*/
/*##                         clock & mail applet 0.1.3             ##*/
/*###################################################################*/

#include "clockmail.h"

GtkWidget *propwindow = NULL;
GtkWidget *mail_file_entry = NULL;
GtkWidget *newmail_exec_cmd_entry = NULL;
gint P_AM_PM_ENABLE;
gint P_ALWAYS_BLINK;
gint P_EXEC_CMD_ON_NEWMAIL;

void property_load(char *path)
{
        if (mail_file) free(mail_file);

	gnome_config_push_prefix (path);
        AM_PM_ENABLE = gnome_config_get_int("clock/12hour=0");
	ALWAYS_BLINK = gnome_config_get_int("mail/blink=0");
	mail_file    = gnome_config_get_string("mail/mailfile=default");
	newmail_exec_cmd = gnome_config_get_string("mail/newmail_command=");
	EXEC_CMD_ON_NEWMAIL = gnome_config_get_int("mail/newmail_command_enable=0");
        gnome_config_pop_prefix ();
}

void property_save(char *path)
{
        gnome_config_push_prefix(path);
        gnome_config_set_int("clock/12hour", AM_PM_ENABLE);
        gnome_config_set_int("mail/blink", ALWAYS_BLINK);
	gnome_config_set_string("mail/mailfile", mail_file);
	gnome_config_set_string("mail/newmail_command", newmail_exec_cmd);
        gnome_config_set_int("mail/newmail_command_enable",
			     EXEC_CMD_ON_NEWMAIL);
	gnome_config_sync();
        gnome_config_pop_prefix();
}

void newmail_exec_cb(GtkWidget *w)
{
	P_EXEC_CMD_ON_NEWMAIL = GTK_TOGGLE_BUTTON (w)->active;
	gnome_property_box_changed(GNOME_PROPERTY_BOX(propwindow));
}

void am_pm_time_cb(GtkWidget *w)
{
	P_AM_PM_ENABLE = GTK_TOGGLE_BUTTON (w)->active;
	gnome_property_box_changed(GNOME_PROPERTY_BOX(propwindow));
}

void always_blink_cb(GtkWidget *w)
{
	P_ALWAYS_BLINK = GTK_TOGGLE_BUTTON (w)->active;
	gnome_property_box_changed(GNOME_PROPERTY_BOX(propwindow));
}

void property_apply_cb( GtkWidget *widget, void *data )
{
	gchar *new_text;
	gchar *command;

	new_text = gtk_entry_get_text(GTK_ENTRY(mail_file_entry));
	if (strcmp(new_text,mail_file) != 0)
		{
		if (mail_file) free(mail_file);
		mail_file = strdup(new_text);
		check_mail_file_status (TRUE);
		}

	command = gtk_entry_get_text(GTK_ENTRY(newmail_exec_cmd_entry));
	if (newmail_exec_cmd) free (newmail_exec_cmd);
	newmail_exec_cmd = strdup(command);

        AM_PM_ENABLE = P_AM_PM_ENABLE;
	ALWAYS_BLINK = P_ALWAYS_BLINK;
	EXEC_CMD_ON_NEWMAIL = P_EXEC_CMD_ON_NEWMAIL;
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
	GtkWidget *button;

	if(propwindow)
		{
                gdk_window_raise(propwindow->window);
                return;
		}

	P_AM_PM_ENABLE = AM_PM_ENABLE;
	P_ALWAYS_BLINK = ALWAYS_BLINK;
	P_EXEC_CMD_ON_NEWMAIL = EXEC_CMD_ON_NEWMAIL;

	propwindow = gnome_property_box_new();
	gtk_window_set_title(GTK_WINDOW(&GNOME_PROPERTY_BOX(propwindow)->dialog.window),
		"ClockMail Settings");
	
	frame = gtk_vbox_new(5, TRUE);

	button = gtk_check_button_new_with_label (_("Display time in 12 hour format (AM/PM)"));
	gtk_box_pack_start(GTK_BOX(frame), button, FALSE, FALSE, 0);
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(button), P_AM_PM_ENABLE);
	gtk_signal_connect (GTK_OBJECT(button),"clicked",(GtkSignalFunc) am_pm_time_cb, NULL);
	gtk_widget_show(button);

	button = gtk_check_button_new_with_label (_("Blink when any mail is waiting. (Not just when mail arrives)"));
	gtk_box_pack_start(GTK_BOX(frame), button, FALSE, FALSE, 0);
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(button), P_ALWAYS_BLINK);
	gtk_signal_connect (GTK_OBJECT(button),"clicked",(GtkSignalFunc) always_blink_cb, NULL);
	gtk_widget_show(button);

	/* mail file entry */
	hbox = gtk_hbox_new(FALSE, 5);
	gtk_box_pack_start( GTK_BOX(frame), hbox, FALSE, FALSE, 5);
	gtk_widget_show(hbox);

	label = gtk_label_new(_("Mail file:"));
	gtk_box_pack_start( GTK_BOX(hbox), label, FALSE, FALSE, 5);
	gtk_widget_show(label);

	mail_file_entry = gtk_entry_new_with_max_length(255);
	gtk_entry_set_text(GTK_ENTRY(mail_file_entry), mail_file);
	gtk_signal_connect_object(GTK_OBJECT(mail_file_entry), "changed",
				GTK_SIGNAL_FUNC(gnome_property_box_changed),
				GTK_OBJECT(propwindow));
	gtk_box_pack_start( GTK_BOX(hbox),mail_file_entry , TRUE, TRUE, 5);
	gtk_widget_show(mail_file_entry);

	/* newmail exec command */
	hbox = gtk_hbox_new(FALSE, 5);
	gtk_box_pack_start( GTK_BOX(frame), hbox, FALSE, FALSE, 5);
	gtk_widget_show(hbox);

	button = gtk_check_button_new_with_label (_("When new mail is received run:"));
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 5);
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(button), P_EXEC_CMD_ON_NEWMAIL);
	gtk_signal_connect (GTK_OBJECT(button),"clicked",(GtkSignalFunc) newmail_exec_cb, NULL);
	gtk_widget_show(button);

	newmail_exec_cmd_entry = gtk_entry_new_with_max_length(255);
	if (newmail_exec_cmd)
		gtk_entry_set_text(GTK_ENTRY(newmail_exec_cmd_entry), newmail_exec_cmd);
	gtk_signal_connect_object(GTK_OBJECT(newmail_exec_cmd_entry), "changed",
				GTK_SIGNAL_FUNC(gnome_property_box_changed),
				GTK_OBJECT(propwindow));
	gtk_box_pack_start( GTK_BOX(hbox),newmail_exec_cmd_entry , TRUE, TRUE, 5);
	gtk_widget_show(newmail_exec_cmd_entry);

        label = gtk_label_new(_("General"));
        gtk_widget_show(frame);
        gnome_property_box_append_page( GNOME_PROPERTY_BOX(propwindow),frame ,label);

	gtk_signal_connect( GTK_OBJECT(propwindow),"apply", GTK_SIGNAL_FUNC(property_apply_cb), NULL );
	gtk_signal_connect( GTK_OBJECT(propwindow),"destroy", GTK_SIGNAL_FUNC(property_destroy_cb), NULL );

        gtk_widget_show_all(propwindow);
} 
