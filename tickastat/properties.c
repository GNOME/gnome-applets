/* tick-a-stat, a GNOME panel applet
 * (C) 1999 John Ellis
 *
 * Author: John Ellis
 *
 */

#include "tickastat.h"

static void enable_log_cb(GtkWidget *w, gpointer data);
static void smooth_scroll_cb(GtkWidget *w, gpointer data);
static void smooth_type_cb(GtkWidget *w, gpointer data);
static void scroll_delay_cb(GtkObject *adj, gpointer data);
static void scroll_speed_cb(GtkObject *adj, gpointer data);
static void module_select_cb(GtkWidget *widget, gint row, gint col,
			     GdkEvent *event, gpointer data);
static void module_list_populate(GtkWidget *clist, AppData *ad);
static void property_apply_cb(GtkWidget *widget, gint page_num, gpointer data);
static gint property_destroy_cb(GtkWidget *widget, gpointer data);

void property_load(gchar *path, AppData *ad)
{
	GList *work;
	gchar *buf;

	gnome_config_push_prefix (path);
	ad->smooth_scroll = gnome_config_get_int("tickastat/smooth_scroll=1");
	ad->smooth_type = gnome_config_get_int("tickastat/smooth_type=1");
	ad->scroll_delay = gnome_config_get_int("tickastat/scroll_delay=10");
	ad->scroll_speed = gnome_config_get_int("tickastat/scroll_speed=3");

	ad->user_width = gnome_config_get_int("tickastat/user_width=200");
	ad->user_height = gnome_config_get_int("tickastat/user_height=48");

	ad->follow_hint_width = gnome_config_get_int("tickastat/use_hint_width=0");
	ad->follow_hint_height = gnome_config_get_int("tickastat/use_hint_height=1");

	ad->enable_log = gnome_config_get_int("tickastat/log_enable=1");
	buf = gnome_config_get_string("tickastat/log_path=");
	if (buf && strlen(buf) > 0)
		{
		g_free(ad->log_path);
		ad->log_path = g_strdup(buf);
		}

	work = ad->modules;
	while(work)
		{
		ModuleData *md = work->data;
		if (md->config_load_func) md->config_load_func(md->internal_data, ad);
		work = work->next;
		}

        gnome_config_pop_prefix ();
}

void property_save(gchar *path, AppData *ad)
{
	GList *work;
        gnome_config_push_prefix(path);
        gnome_config_set_int("tickastat/smooth_scroll", ad->smooth_scroll);
        gnome_config_set_int("tickastat/smooth_type", ad->smooth_type);
        gnome_config_set_int("tickastat/scroll_delay", ad->scroll_delay);
        gnome_config_set_int("tickastat/scroll_speed", ad->scroll_speed);

        gnome_config_set_int("tickastat/user_width", ad->user_width);
        gnome_config_set_int("tickastat/user_height", ad->user_height);

        gnome_config_set_int("tickastat/use_hint_width", ad->follow_hint_width);
        gnome_config_set_int("tickastat/use_hint_height", ad->follow_hint_height);

        gnome_config_set_int("tickastat/log_enable", ad->enable_log);
        gnome_config_set_string("tickastat/log_path", ad->log_path);

	work = ad->modules;
	while(work)
		{
		ModuleData *md = work->data;
		if (md->config_load_func) md->config_save_func(md->internal_data, ad);
		work = work->next;
		}

	gnome_config_sync();
        gnome_config_pop_prefix();
}

static void enable_log_cb(GtkWidget *w, gpointer data)
{
	AppData *ad = data;
	ad->p_enable_log = GTK_TOGGLE_BUTTON (w)->active;
	gnome_property_box_changed(GNOME_PROPERTY_BOX(ad->propwindow));
}

static void smooth_scroll_cb(GtkWidget *w, gpointer data)
{
	AppData *ad = data;
	ad->p_smooth_scroll = GTK_TOGGLE_BUTTON (w)->active;
	gnome_property_box_changed(GNOME_PROPERTY_BOX(ad->propwindow));
}

static void smooth_type_cb(GtkWidget *w, gpointer data)
{
	AppData *ad = data;
	ad->p_smooth_type = GTK_TOGGLE_BUTTON (w)->active;
	gnome_property_box_changed(GNOME_PROPERTY_BOX(ad->propwindow));
}

static void scroll_delay_cb(GtkObject *adj, gpointer data)
{
        AppData *ad = data;

	if (GTK_IS_ADJUSTMENT(adj))
	        ad->p_scroll_delay = (gint)GTK_ADJUSTMENT(adj)->value;
	else
		ad->p_scroll_delay = (gint)GTK_ADJUSTMENT(GTK_SPIN_BUTTON(adj)->adjustment)->value;

        gnome_property_box_changed(GNOME_PROPERTY_BOX(ad->propwindow));
}

static void scroll_speed_cb(GtkObject *adj, gpointer data)
{
        AppData *ad = data;

	if (GTK_IS_ADJUSTMENT(adj))
	        ad->p_scroll_speed = (gint)GTK_ADJUSTMENT(adj)->value;
	else
		ad->p_scroll_speed = (gint)GTK_ADJUSTMENT(GTK_SPIN_BUTTON(adj)->adjustment)->value;

        gnome_property_box_changed(GNOME_PROPERTY_BOX(ad->propwindow));
}

static void width_cb(GtkObject *adj, gpointer data)
{
        AppData *ad = data;

	if (GTK_IS_ADJUSTMENT(adj))
	        ad->p_user_width = (gint)GTK_ADJUSTMENT(adj)->value;
	else
		ad->p_user_width = (gint)GTK_ADJUSTMENT(GTK_SPIN_BUTTON(adj)->adjustment)->value;

        gnome_property_box_changed(GNOME_PROPERTY_BOX(ad->propwindow));
}

static void hint_width_cb(GtkWidget *w, gpointer data)
{
	AppData *ad = data;
	ad->p_follow_hint_width = GTK_TOGGLE_BUTTON (w)->active;
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_object_get_user_data(GTK_OBJECT(w))), !ad->p_follow_hint_width);
	gnome_property_box_changed(GNOME_PROPERTY_BOX(ad->propwindow));
}

static void height_cb(GtkObject *adj, gpointer data)
{
        AppData *ad = data;

	if (GTK_IS_ADJUSTMENT(adj))
	        ad->p_user_height = (gint)GTK_ADJUSTMENT(adj)->value;
	else
		ad->p_user_height = (gint)GTK_ADJUSTMENT(GTK_SPIN_BUTTON(adj)->adjustment)->value;

        gnome_property_box_changed(GNOME_PROPERTY_BOX(ad->propwindow));
}

static void hint_height_cb(GtkWidget *w, gpointer data)
{
	AppData *ad = data;
	ad->p_follow_hint_height = GTK_TOGGLE_BUTTON (w)->active;
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_object_get_user_data(GTK_OBJECT(w))), !ad->p_follow_hint_height);
	gnome_property_box_changed(GNOME_PROPERTY_BOX(ad->propwindow));
}

static void module_select_cb(GtkWidget *widget, gint row, gint col,
			     GdkEvent *event, gpointer data)
{
	AppData *ad = data;
	GtkWidget *w = NULL;
	ModuleData *md = gtk_clist_get_row_data (GTK_CLIST(widget), row);
	if (!md) return;

	if (md->config_show_func)
		w = md->config_show_func(md->internal_data, ad);

	if (w)
		{
		if (ad->prop_current_module)
			{
			ModuleData *old_md = ad->prop_current_module;
			if (old_md->config_hide_func)
				{
				old_md->config_hide_func(old_md->internal_data, ad);
				}
			}
		if (ad->prop_current_module_widget)
			gtk_widget_destroy(ad->prop_current_module_widget);

		ad->prop_current_module = md;
		ad->prop_current_module_widget = w;
		gtk_paned_add2 (GTK_PANED(ad->prop_module_pane), w);
		gtk_widget_show(w);
		}
	return;
	col = 0;
	event = NULL;
}

static void module_list_populate(GtkWidget *clist, AppData *ad)
{
	GList *work;

	work = ad->modules;
	while(work)
		{
		ModuleData *md = work->data;
		gchar *buf[2];
		gint row;
		buf[0] = md->title;
		buf[1] = NULL;
		row = gtk_clist_append(GTK_CLIST(clist), buf);
		gtk_clist_set_row_data(GTK_CLIST(clist), row, md);
		work = work->next;
		}
}

void property_changed(ModuleData *md, AppData *ad)
{
	if (!ad->propwindow) return;
	gnome_property_box_changed(GNOME_PROPERTY_BOX(ad->propwindow));
	return;
	md = NULL;
}

static void
phelp_cb (GtkWidget *w, gint tab, gpointer data)
{
        GnomeHelpMenuEntry help_entry = { "tickastat_applet",
					  "index.html#TICKASTAT-PREFS" };
        gnome_help_display(NULL, &help_entry);
}

static void property_apply_cb(GtkWidget *widget, gint page_num, gpointer data)
{
	AppData *ad = data;
	GList *work;
	gchar *buf;

	if (page_num != -1) return;

	ad->smooth_scroll = ad->p_smooth_scroll;
	ad->smooth_type = ad->p_smooth_type;
	ad->scroll_delay = ad->p_scroll_delay;
	ad->scroll_speed = ad->p_scroll_speed;

	ad->enable_log = ad->p_enable_log;

	buf = gtk_entry_get_text(GTK_ENTRY(ad->log_path_entry));
	if (buf && strlen(buf) > 0)
		{
		g_free(ad->log_path);
		ad->log_path = g_strdup(buf);
		}

	/* apply the module config states */
	work = ad->modules;
	while(work)
		{
		ModuleData *md = work->data;
		if (md->config_apply_func) md->config_apply_func(md->internal_data, ad);
		work = work->next;
		}

	if (ad->p_user_width != ad->user_width ||
	    ad->p_user_height != ad->user_height ||
	    ad->p_follow_hint_width != ad->follow_hint_width ||
	    ad->p_follow_hint_height != ad->follow_hint_height)
		{
		ad->user_width = ad->p_user_width;
		ad->user_height = ad->p_user_height;

		ad->follow_hint_width = ad->p_follow_hint_width;
		ad->follow_hint_height = ad->p_follow_hint_height;

		resized_app_display(ad, FALSE);
		}

	applet_widget_sync_config(APPLET_WIDGET(ad->applet));
	return;
	widget = NULL;
}

static gint property_destroy_cb(GtkWidget *widget, gpointer data)
{
	AppData *ad = data;
	GList *work;
	ad->propwindow = NULL;

	/* hide the selected module config state */
	if (ad->prop_current_module)
		{
		ModuleData *old_md = ad->prop_current_module;
		if (old_md->config_hide_func)
			{
			old_md->config_hide_func(old_md->internal_data, ad);
			}
		}

	/* close the module config states */
	work = ad->modules;
	while(work)
		{
		ModuleData *md = work->data;
		if (md->config_close_func) md->config_close_func(md->internal_data, ad);
		work = work->next;
		}

	return FALSE;
	widget = NULL;
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
	GtkWidget *clist;

	GList *work;

	gchar *titles[] = {"Module:", };

	if(ad->propwindow)
		{
                gdk_window_raise(ad->propwindow->window);
                return;
		}

	ad->p_smooth_scroll = ad->smooth_scroll;
	ad->p_smooth_type = ad->smooth_type;
	ad->p_scroll_delay = ad->scroll_delay;
	ad->p_scroll_speed = ad->scroll_speed;
	ad->p_enable_log = ad->enable_log;

	ad->p_user_width = ad->user_width;
	ad->p_user_height = ad->user_height;

	ad->p_follow_hint_width = ad->follow_hint_width;
	ad->p_follow_hint_height = ad->follow_hint_height;

	ad->propwindow = gnome_property_box_new();
	gtk_window_set_title(GTK_WINDOW(&GNOME_PROPERTY_BOX(ad->propwindow)->dialog.window),
		"Tick-a-Stat Settings");
	
	/* module tab */

	ad->prop_module_pane = gtk_hpaned_new();
	gtk_container_set_border_width(GTK_CONTAINER(ad->prop_module_pane), GNOME_PAD_SMALL);
        gtk_widget_show (ad->prop_module_pane);

	/* clist on left pane */
	scrolled = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_set_usize(scrolled, 120, -1);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
				GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
	gtk_paned_add1 (GTK_PANED(ad->prop_module_pane), scrolled);
	gtk_widget_show(scrolled);

	clist=gtk_clist_new_with_titles(1, titles);
	gtk_signal_connect (GTK_OBJECT (clist), "select_row",(GtkSignalFunc) module_select_cb, ad);
	gtk_clist_column_titles_passive (GTK_CLIST (clist));
	gtk_container_add (GTK_CONTAINER (scrolled), clist);
	gtk_widget_show(clist);

	/* left pane will be filled in select cb */

        label = gtk_label_new(_("Modules"));
        gnome_property_box_append_page(GNOME_PROPERTY_BOX(ad->propwindow), ad->prop_module_pane, label);

	/* general tab */

	vbox = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), GNOME_PAD_SMALL);
	gtk_widget_show(vbox);

	frame = gtk_frame_new(_("Event Log"));
	gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);
	gtk_widget_show(frame);

	vbox1 = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_container_set_border_width(GTK_CONTAINER(vbox1), GNOME_PAD_SMALL);
	gtk_container_add(GTK_CONTAINER(frame), vbox1);
	gtk_widget_show(vbox1);

	button = gtk_check_button_new_with_label (_("Enable logging of events"));
	gtk_box_pack_start(GTK_BOX(vbox1), button, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), ad->p_enable_log);
	gtk_signal_connect (GTK_OBJECT(button),"clicked",(GtkSignalFunc) enable_log_cb, ad);
	gtk_widget_show(button);

	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(vbox1), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

        label = gtk_label_new(_("Log path:"));
        gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
        gtk_widget_show(label);

	label = gnome_file_entry_new ("tickastat log path",
				      _("Choose a log file"));
	ad->log_path_entry = gnome_file_entry_gtk_entry(GNOME_FILE_ENTRY(label));
	if (ad->log_path)
		gtk_entry_set_text(GTK_ENTRY(ad->log_path_entry), ad->log_path);
	gtk_signal_connect_object(GTK_OBJECT(ad->log_path_entry), "changed",
				GTK_SIGNAL_FUNC(gnome_property_box_changed),
				GTK_OBJECT(ad->propwindow));
        gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
        gtk_widget_show(label);

        label = gtk_label_new(_("General"));
        gtk_widget_show(frame);
        gnome_property_box_append_page( GNOME_PROPERTY_BOX(ad->propwindow),vbox ,label);

	/* display tab */

	vbox = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), GNOME_PAD_SMALL);
	gtk_widget_show(vbox);

	frame = gtk_frame_new(_("Scrolling"));
	gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);
	gtk_widget_show(frame);

	vbox1 = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_container_set_border_width(GTK_CONTAINER(vbox1), GNOME_PAD_SMALL);
	gtk_container_add(GTK_CONTAINER(frame), vbox1);
	gtk_widget_show(vbox1);

	button = gtk_check_button_new_with_label (_("Smooth scroll"));
	gtk_box_pack_start(GTK_BOX(vbox1), button, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), ad->p_smooth_scroll);
	gtk_signal_connect (GTK_OBJECT(button),"clicked",(GtkSignalFunc) smooth_scroll_cb, ad);
	gtk_widget_show(button);

	button = gtk_check_button_new_with_label (_("Smooth type"));
	gtk_box_pack_start(GTK_BOX(vbox1), button, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), ad->p_smooth_type);
	gtk_signal_connect (GTK_OBJECT(button),"clicked",(GtkSignalFunc) smooth_type_cb, ad);
	gtk_widget_show(button);

	frame = gtk_frame_new(_("Speed"));
	gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);
	gtk_widget_show(frame);

	vbox1 = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_container_set_border_width(GTK_CONTAINER(vbox1), GNOME_PAD_SMALL);
	gtk_container_add(GTK_CONTAINER(frame), vbox1);
	gtk_widget_show(vbox1);

	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(vbox1), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

        label = gtk_label_new(_("Delay when wrapping text:"));
        gtk_box_pack_start( GTK_BOX(hbox), label, FALSE, FALSE, 0);
        gtk_widget_show(label);

        adj = gtk_adjustment_new(ad->p_scroll_delay, 0.0, 50.0, 1, 1, 1 );
        spin = gtk_spin_button_new( GTK_ADJUSTMENT(adj), 1, 0 );
        gtk_box_pack_start( GTK_BOX(hbox), spin, FALSE, FALSE, 0);
        gtk_signal_connect( GTK_OBJECT(adj),"value_changed",
			GTK_SIGNAL_FUNC(scroll_delay_cb), ad);
        gtk_signal_connect( GTK_OBJECT(spin),"changed",
			GTK_SIGNAL_FUNC(scroll_delay_cb), ad);
        gtk_spin_button_set_update_policy(GTK_SPIN_BUTTON(spin), GTK_UPDATE_ALWAYS);
        gtk_widget_show(spin);

	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(vbox1), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

        label = gtk_label_new(_("Scroll speed between lines (Smooth scroll):"));
        gtk_box_pack_start( GTK_BOX(hbox), label, FALSE, FALSE, 0);
        gtk_widget_show(label);

        adj = gtk_adjustment_new( ad->p_scroll_speed, 1.0, (float)ad->scroll_height - 1, 1, 1, 1 );
        spin = gtk_spin_button_new( GTK_ADJUSTMENT(adj), 1, 0 );
        gtk_box_pack_start( GTK_BOX(hbox), spin, FALSE, FALSE, 0);
        gtk_signal_connect( GTK_OBJECT(adj),"value_changed",
			GTK_SIGNAL_FUNC(scroll_speed_cb), ad);
        gtk_signal_connect( GTK_OBJECT(spin),"changed",
			GTK_SIGNAL_FUNC(scroll_speed_cb), ad);
        gtk_spin_button_set_update_policy(GTK_SPIN_BUTTON(spin), GTK_UPDATE_ALWAYS);
        gtk_widget_show(spin);

	frame = gtk_frame_new(_("Size"));
	gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);
	gtk_widget_show(frame);

	vbox1 = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_container_set_border_width(GTK_CONTAINER(vbox1), GNOME_PAD_SMALL);
	gtk_container_add(GTK_CONTAINER(frame), vbox1);
	gtk_widget_show(vbox1);

	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(vbox1), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

        label = gtk_label_new(_("Width:"));
        gtk_box_pack_start( GTK_BOX(hbox), label, FALSE, FALSE, 0);
        gtk_widget_show(label);

        adj = gtk_adjustment_new( ad->p_user_width, 100.0, 2000.0, 1, 1, 1 );
        spin = gtk_spin_button_new( GTK_ADJUSTMENT(adj), 1, 0 );
        gtk_box_pack_start( GTK_BOX(hbox), spin, FALSE, FALSE, 0);
        gtk_signal_connect( GTK_OBJECT(adj),"value_changed",
			GTK_SIGNAL_FUNC(width_cb), ad);
        gtk_signal_connect( GTK_OBJECT(spin),"changed",
			GTK_SIGNAL_FUNC(width_cb), ad);
        gtk_spin_button_set_update_policy(GTK_SPIN_BUTTON(spin), GTK_UPDATE_ALWAYS);
        gtk_widget_show(spin);

#ifdef HAVE_PANEL_PIXEL_SIZE
	button = gtk_check_button_new_with_label (_("Use all room on panel"));
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), ad->p_follow_hint_width);
	gtk_signal_connect (GTK_OBJECT(button),"clicked",(GtkSignalFunc) hint_width_cb, ad);
	gtk_widget_show(button);
	gtk_object_set_user_data(GTK_OBJECT(button), spin);
	if (ad->p_follow_hint_width) gtk_widget_set_sensitive(spin, FALSE);
#endif

	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(vbox1), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

        label = gtk_label_new(_("Height:"));
        gtk_box_pack_start( GTK_BOX(hbox), label, FALSE, FALSE, 0);
        gtk_widget_show(label);

        adj = gtk_adjustment_new( ad->p_user_height, 24.0, 200.0, 1, 1, 1 );
        spin = gtk_spin_button_new( GTK_ADJUSTMENT(adj), 1, 0 );
        gtk_box_pack_start( GTK_BOX(hbox), spin, FALSE, FALSE, 0);
        gtk_signal_connect( GTK_OBJECT(adj),"value_changed",
			GTK_SIGNAL_FUNC(height_cb), ad);
        gtk_signal_connect( GTK_OBJECT(spin),"changed",
			GTK_SIGNAL_FUNC(height_cb), ad);
        gtk_spin_button_set_update_policy(GTK_SPIN_BUTTON(spin), GTK_UPDATE_ALWAYS);
        gtk_widget_show(spin);

#ifdef HAVE_PANEL_PIXEL_SIZE
	button = gtk_check_button_new_with_label (_("Use panel size hint"));
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), ad->p_follow_hint_height);
	gtk_signal_connect (GTK_OBJECT(button),"clicked",(GtkSignalFunc) hint_height_cb, ad);
	gtk_widget_show(button);
	gtk_object_set_user_data(GTK_OBJECT(button), spin);
	if (ad->p_follow_hint_height) gtk_widget_set_sensitive(spin, FALSE);
#endif

        label = gtk_label_new(_("Display"));
        gtk_widget_show(frame);
        gnome_property_box_append_page( GNOME_PROPERTY_BOX(ad->propwindow),vbox ,label);

	gtk_signal_connect( GTK_OBJECT(ad->propwindow),"apply", GTK_SIGNAL_FUNC(property_apply_cb), ad);
	gtk_signal_connect( GTK_OBJECT(ad->propwindow),"destroy", GTK_SIGNAL_FUNC(property_destroy_cb), ad );
	gtk_signal_connect( GTK_OBJECT(ad->propwindow),"help", GTK_SIGNAL_FUNC(phelp_cb), ad );

	/* init the module config states */
	work = ad->modules;
	while(work)
		{
		ModuleData *md = work->data;
		if (md->config_init_func) md->config_init_func(md->internal_data, ad);
		work = work->next;
		}

	ad->prop_current_module = NULL;
	ad->prop_current_module_widget = NULL;

	module_list_populate(clist, ad);

	gtk_clist_select_row(GTK_CLIST(clist), 0, 0);

	gtk_widget_show_all(ad->propwindow);
	return;
	applet = NULL;
} 
