/* GNOME clock & mailcheck applet
 * (C) 2000 John Ellis
 *
 * Author: John Ellis
 *
 */

#include "clockmail.h"

static void newmail_exec_cb(GtkWidget *w, gpointer data);
static void am_pm_time_cb(GtkWidget *w, gpointer data);
static void use_gmt_cb(GtkWidget *w, gpointer data);
static void gmt_offset_cb(GtkObject *adj, gpointer data);
static void always_blink_cb(GtkWidget *w, gpointer data);
static void mail_max_cb(GtkObject *adj, gpointer data);
static void property_apply_cb(GtkWidget *widget, gint page_num, gpointer data);
static gint property_destroy_cb(GtkWidget *widget, gpointer data);

void property_load(const gchar *path, AppData *ad)
{
	g_free(ad->mail_file);
	g_free(ad->theme_file);
	g_free(ad->newmail_exec_cmd);
	g_free(ad->reader_exec_cmd);
	gnome_config_push_prefix (path);
        ad->am_pm_enable = gnome_config_get_int("clockmail/12hour=0");
	ad->always_blink = gnome_config_get_int("clockmail/blink=0");
	ad->mail_file = gnome_config_get_string("clockmail/mailfile=default");
	ad->reader_exec_cmd = gnome_config_get_string("clockmail/reader_command=balsa");
	ad->newmail_exec_cmd = gnome_config_get_string("clockmail/newmail_command=");
	ad->exec_cmd_on_newmail = gnome_config_get_int("clockmail/newmail_command_enable=0");
	ad->theme_file = gnome_config_get_string("clockmail/theme=");
	ad->use_gmt = gnome_config_get_int("clockmail/gmt=0");
	ad->gmt_offset = gnome_config_get_int("clockmail/gmt_offset=0");
	ad->mail_max = gnome_config_get_int("clockmail/mailmax=100");
        gnome_config_pop_prefix ();
}

void property_save(const gchar *path, AppData *ad)
{
        gnome_config_push_prefix(path);
        gnome_config_set_int("clockmail/12hour", ad->am_pm_enable);
        gnome_config_set_int("clockmail/blink", ad->always_blink);
	gnome_config_set_string("clockmail/mailfile", ad->mail_file);
	gnome_config_set_string("clockmail/reader_command", ad->reader_exec_cmd);
	gnome_config_set_string("clockmail/newmail_command", ad->newmail_exec_cmd);
        gnome_config_set_int("clockmail/newmail_command_enable",
			     ad->exec_cmd_on_newmail);
	gnome_config_set_string("clockmail/theme", ad->theme_file);
        gnome_config_set_int("clockmail/gmt", ad->use_gmt);
        gnome_config_set_int("clockmail/gmt_offset", ad->gmt_offset);
        gnome_config_set_int("clockmail/mailmax", ad->mail_max);
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
	if (GTK_IS_ADJUSTMENT(adj))
		{
		ad->p_gmt_offset = (gint)GTK_ADJUSTMENT(adj)->value;
		}
	else
		{
		ad->p_gmt_offset = (gint)GTK_ADJUSTMENT(GTK_SPIN_BUTTON(adj)->adjustment)->value;
		}
	gnome_property_box_changed(GNOME_PROPERTY_BOX(ad->propwindow));
} 

static void always_blink_cb(GtkWidget *w, gpointer data)
{
	AppData *ad = data;
	ad->p_always_blink = GTK_TOGGLE_BUTTON (w)->active;
	gnome_property_box_changed(GNOME_PROPERTY_BOX(ad->propwindow));
}

static void mail_max_cb(GtkObject *adj, gpointer data)
{
	AppData *ad = data;
	if (GTK_IS_ADJUSTMENT(adj))
		{
		ad->p_mail_max = (gint)GTK_ADJUSTMENT(adj)->value;
		}
	else
		{
		ad->p_mail_max = (gint)GTK_ADJUSTMENT(GTK_SPIN_BUTTON(adj)->adjustment)->value;
		}
	gnome_property_box_changed(GNOME_PROPERTY_BOX(ad->propwindow));
}

static void theme_selected_cb(GtkWidget *clist, gint row, gint column,
		GdkEventButton *bevent, gpointer data)
{
	AppData *ad = data;
	gchar *text = gtk_clist_get_row_data(GTK_CLIST(clist), row);
	if (text) gtk_entry_set_text(GTK_ENTRY(ad->theme_entry),text);
	return;
}

static gint sort_theme_list_cb(void *a, void *b)
{
	return strcmp((gchar *)a, (gchar *)b);
}

static void populate_theme_list(GtkWidget *clist)
{
	DIR *dp;
	struct dirent *dir;
	struct stat ent_sbuf;
	gint row;
	gchar *buf[] = { "x", };
	gchar *themepath;
	GList *theme_list = NULL;
	GList *list;

	/* add default theme */
	buf[0] = _("None (default)");
	row = gtk_clist_append(GTK_CLIST(clist),buf);
	gtk_clist_set_row_data(GTK_CLIST(clist), row, "");

	themepath = gnome_unconditional_datadir_file("clockmail");

	if((dp = opendir(themepath))==NULL)
		{
		/* dir not found */
		g_free(themepath);
		return;
		}

	while ((dir = readdir(dp)) != NULL)
		{
		/* skips removed files */
		if (dir->d_ino > 0)
			{
			gchar *name;
			gchar *path;

			name = dir->d_name; 
			path = g_strconcat(themepath, "/", name, NULL);

			if (stat(path,&ent_sbuf) >= 0 && S_ISDIR(ent_sbuf.st_mode))
				{
				theme_list = g_list_insert_sorted(theme_list,
						g_strdup(name),
						(GCompareFunc) sort_theme_list_cb);
				}
			g_free(path);
			}
		}
        closedir(dp);

	list = theme_list;
	while (list)
		{
		gchar *themedata_file = g_strconcat(themepath, "/", list->data, "/clockmaildata", NULL);
		if (g_file_exists(themedata_file))
			{
			gchar *theme_file = g_strconcat(themepath, "/", list->data, NULL);
			buf[0] = list->data;
			row = gtk_clist_append(GTK_CLIST(clist),buf);
			gtk_clist_set_row_data_full(GTK_CLIST(clist), row,
					theme_file, (GtkDestroyNotify) g_free);
			}
		g_free(themedata_file);
		g_free(list->data);
		list = list->next;
		}

	g_list_free(theme_list);

	g_free(themepath);
}

static void property_apply_cb(GtkWidget *widget, gint page_num, gpointer data)
{
	AppData *ad = data;
	gchar *buf;

	if (page_num != -1) return;

	buf = gtk_entry_get_text(GTK_ENTRY(ad->mail_file_entry));
	if (ad->mail_file == NULL || strcmp(buf, ad->mail_file) != 0)
		{
		if (ad->mail_file) g_free(ad->mail_file);
		ad->mail_file = g_strdup(buf);
		check_mail_file_status (TRUE, ad);
		}

	buf = gtk_entry_get_text(GTK_ENTRY(ad->reader_exec_cmd_entry));
	g_free (ad->reader_exec_cmd);
	ad->reader_exec_cmd = g_strdup(buf);

	buf = gtk_entry_get_text(GTK_ENTRY(ad->newmail_exec_cmd_entry));
	if (ad->newmail_exec_cmd) g_free (ad->newmail_exec_cmd);
	ad->newmail_exec_cmd = g_strdup(buf);

	buf = gtk_entry_get_text(GTK_ENTRY(ad->theme_entry));
	if ((!ad->theme_file && buf && strlen(buf) > 0) ||
	    (ad->theme_file && (!buf || strlen(buf) == 0)) ||
	    (buf && ad->theme_file && strcmp(buf, ad->theme_file) != 0))
		{
		g_free (ad->theme_file);
		ad->theme_file = NULL;
		if (buf && strlen(buf) > 0) ad->theme_file = g_strdup(buf);
		reload_skin(ad);
		}

        ad->am_pm_enable = ad->p_am_pm_enable;
	ad->always_blink = ad->p_always_blink;

	if (ad->use_gmt != ad->p_use_gmt || ad->gmt_offset != ad->p_gmt_offset )
		ad->old_yday = -1;

	ad->use_gmt = ad->p_use_gmt;
	ad->gmt_offset = ad->p_gmt_offset;
	ad->mail_max = ad->p_mail_max;

	ad->exec_cmd_on_newmail = ad->p_exec_cmd_on_newmail;

	applet_widget_sync_config(APPLET_WIDGET(ad->applet));
	return;
}

static gint property_destroy_cb(GtkWidget *widget, gpointer data)
{
	AppData *ad = data;
	ad->propwindow = NULL;
	return FALSE;
}

static void
phelp_cb (GtkWidget *w, gint tab, gpointer data)
{
	GnomeHelpMenuEntry help_entry = { 
		"clockmail_applet", "index.html#CLOCKMAIL-PREFS"
	};
	gnome_help_display (NULL, &help_entry);
}

void property_show(AppletWidget *applet, gpointer data)
{
	AppData *ad = data;
	GtkWidget *frame;
	GtkWidget *vbox;
	GtkWidget *vbox1;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *button;
	GtkObject *adj;
	GtkWidget *spin;
	GtkWidget *scrolled;
	GtkWidget *theme_clist;
	gchar *theme_title[] = { N_("Themes:"), };

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
	ad->p_mail_max = ad->mail_max;

	ad->propwindow = gnome_property_box_new();
	gtk_window_set_title(GTK_WINDOW(&GNOME_PROPERTY_BOX(ad->propwindow)->dialog.window),
		_("ClockMail Settings"));
	
	vbox = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), GNOME_PAD_SMALL);

	frame = gtk_frame_new(_("Clock"));
	gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);
	gtk_widget_show(frame);

	vbox1 = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_container_set_border_width (GTK_CONTAINER (vbox1), GNOME_PAD_SMALL);
	gtk_container_add(GTK_CONTAINER(frame), vbox1);
	gtk_widget_show(vbox1);

	button = gtk_check_button_new_with_label (_("Display time in 12 hour format (AM/PM)"));
	gtk_box_pack_start(GTK_BOX(vbox1), button, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), ad->p_am_pm_enable);
	gtk_signal_connect (GTK_OBJECT(button),"clicked",(GtkSignalFunc) am_pm_time_cb, ad);
	gtk_widget_show(button);

	hbox = gtk_hbox_new(FALSE, 5);
	gtk_box_pack_start( GTK_BOX(vbox1), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

	button = gtk_check_button_new_with_label (_("Display time relative to GMT (Greenwich Mean Time):"));
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), ad->p_use_gmt);
	gtk_signal_connect (GTK_OBJECT(button),"clicked",(GtkSignalFunc) use_gmt_cb, ad);
	gtk_widget_show(button);

	adj = gtk_adjustment_new((float)ad->gmt_offset, -12.0, 12.0, 1, 1, 1 );
	spin = gtk_spin_button_new( GTK_ADJUSTMENT(adj), 1, 0 );
	gtk_box_pack_start( GTK_BOX(hbox), spin, FALSE, FALSE, 5);
	gtk_signal_connect( GTK_OBJECT(adj),"value_changed",GTK_SIGNAL_FUNC(gmt_offset_cb), ad);
	gtk_signal_connect( GTK_OBJECT(spin),"changed",GTK_SIGNAL_FUNC(gmt_offset_cb), ad);
	gtk_spin_button_set_update_policy( GTK_SPIN_BUTTON(spin),GTK_UPDATE_ALWAYS );
	gtk_widget_show(spin);

	frame = gtk_frame_new(_("Mail"));
	gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);
	gtk_widget_show(frame);

	vbox1 = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_container_set_border_width (GTK_CONTAINER (vbox1), GNOME_PAD_SMALL);
	gtk_container_add(GTK_CONTAINER(frame), vbox1);
	gtk_widget_show(vbox1);

	/* mail file entry */
	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start( GTK_BOX(vbox1), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

	label = gtk_label_new(_("Mail file:"));
	gtk_box_pack_start( GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_widget_show(label);

	ad->mail_file_entry = gtk_entry_new_with_max_length(255);
	gtk_entry_set_text(GTK_ENTRY(ad->mail_file_entry), ad->mail_file);
	gtk_signal_connect_object(GTK_OBJECT(ad->mail_file_entry), "changed",
				GTK_SIGNAL_FUNC(gnome_property_box_changed),
				GTK_OBJECT(ad->propwindow));
	gtk_box_pack_start( GTK_BOX(hbox),ad->mail_file_entry , TRUE, TRUE, 0);
	gtk_widget_show(ad->mail_file_entry);

	/* newmail exec command */
	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start( GTK_BOX(vbox1), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

	button = gtk_check_button_new_with_label (_("When new mail is received run:"));
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), ad->p_exec_cmd_on_newmail);
	gtk_signal_connect (GTK_OBJECT(button),"clicked",(GtkSignalFunc) newmail_exec_cb, ad);
	gtk_widget_show(button);

	ad->newmail_exec_cmd_entry = gtk_entry_new_with_max_length(255);
	if (ad->newmail_exec_cmd)
		gtk_entry_set_text(GTK_ENTRY(ad->newmail_exec_cmd_entry), ad->newmail_exec_cmd);
	gtk_signal_connect_object(GTK_OBJECT(ad->newmail_exec_cmd_entry), "changed",
				GTK_SIGNAL_FUNC(gnome_property_box_changed),
				GTK_OBJECT(ad->propwindow));
	gtk_box_pack_start( GTK_BOX(hbox),ad->newmail_exec_cmd_entry , TRUE, TRUE, 0);
	gtk_widget_show(ad->newmail_exec_cmd_entry);

	button = gtk_check_button_new_with_label (_("Always blink when any mail is waiting."));
	gtk_box_pack_start(GTK_BOX(vbox1), button, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), ad->p_always_blink);
	gtk_signal_connect (GTK_OBJECT(button),"clicked",(GtkSignalFunc) always_blink_cb, ad);
	gtk_widget_show(button);

	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start( GTK_BOX(vbox1), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

	label = gtk_label_new(_("Number of messages to consider mailbox full:"));
	gtk_box_pack_start( GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_widget_show(label);

	adj = gtk_adjustment_new((float)ad->mail_max, 10.0, 9000.0, 1, 5, 50);
	spin = gtk_spin_button_new( GTK_ADJUSTMENT(adj), 1, 0 );
	gtk_widget_set_usize(spin, 100, -1);
	gtk_box_pack_start( GTK_BOX(hbox), spin, FALSE, FALSE, 0);
	gtk_signal_connect( GTK_OBJECT(adj),"value_changed",GTK_SIGNAL_FUNC(mail_max_cb), ad);
	gtk_signal_connect( GTK_OBJECT(spin),"changed",GTK_SIGNAL_FUNC(mail_max_cb), ad);
	gtk_spin_button_set_update_policy( GTK_SPIN_BUTTON(spin),GTK_UPDATE_ALWAYS );
	gtk_widget_show(spin);

	/* reader exec command */
	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

	label= gtk_label_new (_("When clicked, run:"));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_widget_show(label);

	ad->reader_exec_cmd_entry = gtk_entry_new_with_max_length(255);
	if (ad->reader_exec_cmd)
		gtk_entry_set_text(GTK_ENTRY(ad->reader_exec_cmd_entry), ad->reader_exec_cmd);
	gtk_signal_connect_object(GTK_OBJECT(ad->reader_exec_cmd_entry), "changed",
				GTK_SIGNAL_FUNC(gnome_property_box_changed),
				GTK_OBJECT(ad->propwindow));
	gtk_box_pack_start( GTK_BOX(hbox),ad->reader_exec_cmd_entry , TRUE, TRUE, 0);
	gtk_widget_show(ad->reader_exec_cmd_entry);

        label = gtk_label_new(_("General"));
        gtk_widget_show(vbox);
        gnome_property_box_append_page( GNOME_PROPERTY_BOX(ad->propwindow),vbox ,label);

	/* theme tab */

	frame = gtk_frame_new(NULL);
	gtk_container_set_border_width (GTK_CONTAINER (frame), GNOME_PAD_SMALL);

	vbox = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), GNOME_PAD_SMALL);
	gtk_container_add(GTK_CONTAINER(frame), vbox);
	gtk_widget_show(vbox);

	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start( GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

	label = gtk_label_new(_("Theme file (directory):"));
	gtk_box_pack_start( GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_widget_show(label);

	ad->theme_entry = gtk_entry_new_with_max_length(255);
	if (ad->theme_file)
		gtk_entry_set_text(GTK_ENTRY(ad->theme_entry), ad->theme_file);
	gtk_signal_connect_object(GTK_OBJECT(ad->theme_entry), "changed",
				GTK_SIGNAL_FUNC(gnome_property_box_changed),
				GTK_OBJECT(ad->propwindow));
	gtk_box_pack_start( GTK_BOX(hbox),ad->theme_entry , TRUE, TRUE, 0);
	gtk_widget_show(ad->theme_entry);

	scrolled = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
				GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
	gtk_box_pack_start(GTK_BOX(vbox), scrolled, TRUE, TRUE, 0);
	gtk_widget_show(scrolled);

	/* theme list */
#ifdef ENABLE_NLS
	theme_title[0]=_(theme_title[0]);
#endif
	theme_clist=gtk_clist_new_with_titles (1, theme_title);
	gtk_clist_column_titles_passive (GTK_CLIST (theme_clist)); 
	gtk_signal_connect (GTK_OBJECT (theme_clist), "select_row",(GtkSignalFunc) theme_selected_cb, ad);
	gtk_container_add (GTK_CONTAINER (scrolled), theme_clist);
	gtk_widget_show (theme_clist);

	populate_theme_list(theme_clist);

        label = gtk_label_new(_("Theme"));
        gtk_widget_show(frame);
        gnome_property_box_append_page( GNOME_PROPERTY_BOX(ad->propwindow),frame ,label);

	gtk_signal_connect( GTK_OBJECT(ad->propwindow),"apply", GTK_SIGNAL_FUNC(property_apply_cb), ad);
	gtk_signal_connect( GTK_OBJECT(ad->propwindow),"destroy", GTK_SIGNAL_FUNC(property_destroy_cb), ad );
	gtk_signal_connect( GTK_OBJECT(ad->propwindow),"help", GTK_SIGNAL_FUNC(phelp_cb), NULL);

	gtk_widget_show_all(ad->propwindow);
	return;
} 
