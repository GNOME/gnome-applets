/*###################################################################*/
/*##                         clock & mail applet 0.2.0             ##*/
/*###################################################################*/

#include "clockmail.h"

static void newmail_exec_cb(GtkWidget *w, gpointer data);
static void am_pm_time_cb(GtkWidget *w, gpointer data);
static void use_gmt_cb(GtkWidget *w, gpointer data);
static void gmt_offset_cb(GtkObject *adj, gpointer data);
static void always_blink_cb(GtkWidget *w, gpointer data);
static void property_apply_cb(GtkWidget *widget, void *nodata, gpointer data);
static gint property_destroy_cb(GtkWidget *widget, gpointer data);

void property_load(gchar *path, AppData *ad)
{
        if (ad->mail_file) g_free(ad->mail_file);
	if (ad->theme_file) g_free(ad->theme_file);
	gnome_config_push_prefix (path);
        ad->am_pm_enable = gnome_config_get_int("clockmail/12hour=0");
	ad->always_blink = gnome_config_get_int("clockmail/blink=0");
	ad->mail_file = gnome_config_get_string("clockmail/mailfile=default");
	ad->newmail_exec_cmd = gnome_config_get_string("clockmail/newmail_command=");
	ad->exec_cmd_on_newmail = gnome_config_get_int("clockmail/newmail_command_enable=0");
	ad->theme_file = gnome_config_get_string("clockmail/theme=");
	ad->use_gmt = gnome_config_get_int("clockmail/gmt=0");
	ad->gmt_offset = gnome_config_get_int("clockmail/gmt_offset=0");
        gnome_config_pop_prefix ();
}

void property_save(gchar *path, AppData *ad)
{
        gnome_config_push_prefix(path);
        gnome_config_set_int("clockmail/12hour", ad->am_pm_enable);
        gnome_config_set_int("clockmail/blink", ad->always_blink);
	gnome_config_set_string("clockmail/mailfile", ad->mail_file);
	gnome_config_set_string("clockmail/newmail_command", ad->newmail_exec_cmd);
        gnome_config_set_int("clockmail/newmail_command_enable",
			     ad->exec_cmd_on_newmail);
	gnome_config_set_string("clockmail/theme", ad->theme_file);
        gnome_config_set_int("clockmail/gmt", ad->use_gmt);
        gnome_config_set_int("clockmail/gmt_offset", ad->gmt_offset);
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

static void use_gmt_cb(GtkWidget *w, gpointer data)
{
	AppData *ad = data;
	ad->p_use_gmt = GTK_TOGGLE_BUTTON (w)->active;
	gnome_property_box_changed(GNOME_PROPERTY_BOX(ad->propwindow));
}

static void gmt_offset_cb(GtkObject *adj, gpointer data)
{
	AppData *ad = data;
	ad->p_gmt_offset = (gint)GTK_ADJUSTMENT(adj)->value;
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
	gchar *buf;

	buf = gtk_entry_get_text(GTK_ENTRY(ad->mail_file_entry));
	if (strcmp(buf,ad->mail_file) != 0)
		{
		if (ad->mail_file) g_free(ad->mail_file);
		ad->mail_file = g_strdup(buf);
		check_mail_file_status (TRUE, ad);
		}

	buf = gtk_entry_get_text(GTK_ENTRY(ad->newmail_exec_cmd_entry));
	if (ad->newmail_exec_cmd) g_free (ad->newmail_exec_cmd);
	ad->newmail_exec_cmd = g_strdup(buf);

	buf = gtk_entry_get_text(GTK_ENTRY(ad->theme_entry));
	if (buf && ad->theme_file && strcmp(buf, ad->theme_file) != 0)
		{
		g_free (ad->theme_file);
		ad->theme_file = g_strdup(buf);
		if (!change_to_skin(ad->theme_file, ad))
			change_to_skin(NULL, ad);
		}
	else if (buf && strlen(buf) != 0)
		{
		if (ad->theme_file) g_free (ad->theme_file);
		ad->theme_file = g_strdup(buf);
		if (!change_to_skin(ad->theme_file, ad))
			change_to_skin(NULL, ad);
		}
	else
		{
		if (ad->theme_file) g_free (ad->theme_file);
		ad->theme_file = NULL;
		change_to_skin(NULL, ad);
		}

        ad->am_pm_enable = ad->p_am_pm_enable;
	ad->always_blink = ad->p_always_blink;

	if (ad->use_gmt != ad->p_use_gmt || ad->gmt_offset != ad->p_gmt_offset )
		ad->old_yday = -1;

	ad->use_gmt = ad->p_use_gmt;
	ad->gmt_offset = ad->p_gmt_offset;

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
	GtkObject *adj;
	GtkWidget *spin;

	if(ad->propwindow)
		{
                gdk_window_raise(ad->propwindow->window);
                return;
		}

        ad->p_am_pm_enable = ad->am_pm_enable;
	ad->p_always_blink = ad->always_blink;
	ad->p_use_gmt = ad->use_gmt;
	ad->p_gmt_offset = ad->gmt_offset;
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

	hbox = gtk_hbox_new(FALSE, 5);
	gtk_box_pack_start( GTK_BOX(frame), hbox, FALSE, FALSE, 5);
	gtk_widget_show(hbox);

	button = gtk_check_button_new_with_label (_("Display time relative to GMT (Greenwich Mean Time):"));
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(button), ad->p_use_gmt);
	gtk_signal_connect (GTK_OBJECT(button),"clicked",(GtkSignalFunc) use_gmt_cb, ad);
	gtk_widget_show(button);

	adj = gtk_adjustment_new((float)ad->gmt_offset, -12.0, 12.0, 1, 1, 1 );
	spin = gtk_spin_button_new( GTK_ADJUSTMENT(adj), 1, 0 );
	gtk_box_pack_start( GTK_BOX(hbox), spin, FALSE, FALSE, 5);
	gtk_signal_connect( GTK_OBJECT(adj),"value_changed",GTK_SIGNAL_FUNC(gmt_offset_cb), ad);
	gtk_spin_button_set_update_policy( GTK_SPIN_BUTTON(spin),GTK_UPDATE_ALWAYS );
	gtk_widget_show(spin);

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

	/* theme tab */

	frame = gtk_vbox_new(5, TRUE);

	hbox = gtk_hbox_new(FALSE, 5);
	gtk_box_pack_start( GTK_BOX(frame), hbox, FALSE, FALSE, 5);
	gtk_widget_show(hbox);

	label = gtk_label_new(_("Theme file (directory):"));
	gtk_box_pack_start( GTK_BOX(hbox), label, FALSE, FALSE, 5);
	gtk_widget_show(label);

	ad->theme_entry = gtk_entry_new_with_max_length(255);
	if (ad->theme_file)
		gtk_entry_set_text(GTK_ENTRY(ad->theme_entry), ad->theme_file);
	gtk_signal_connect_object(GTK_OBJECT(ad->theme_entry), "changed",
				GTK_SIGNAL_FUNC(gnome_property_box_changed),
				GTK_OBJECT(ad->propwindow));
	gtk_box_pack_start( GTK_BOX(hbox),ad->theme_entry , TRUE, TRUE, 5);
	gtk_widget_show(ad->theme_entry);

        label = gtk_label_new(_("Theme"));
        gtk_widget_show(frame);
        gnome_property_box_append_page( GNOME_PROPERTY_BOX(ad->propwindow),frame ,label);

	gtk_signal_connect( GTK_OBJECT(ad->propwindow),"apply", GTK_SIGNAL_FUNC(property_apply_cb), ad);
	gtk_signal_connect( GTK_OBJECT(ad->propwindow),"destroy", GTK_SIGNAL_FUNC(property_destroy_cb), ad );

        gtk_widget_show_all(ad->propwindow);
} 
