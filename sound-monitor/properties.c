/* GNOME sound-monitor applet
 * (C) 2000 John Ellis
 *
 * Author: John Ellis
 *
 */

#include <config.h>
#include "sound-monitor.h"
#include "update.h"
#include "esdcalls.h"

static void property_apply_cb(GtkWidget *widget, gint page_num, gpointer data);
static gint property_destroy_cb(GtkWidget *widget, gpointer data);

void property_load(const gchar *path, AppData *ad)
{
	g_free(ad->theme_file);
	g_free(ad->esd_host);

	gnome_config_push_prefix (path);

        ad->peak_mode = gnome_config_get_int("sound-monitor/peak=1");
        ad->falloff_speed = gnome_config_get_int("sound-monitor/falloff=3");
        ad->scope_scale = gnome_config_get_int("sound-monitor/scale=5");
	ad->draw_scope_as_segments = gnome_config_get_int("sound-monitor/segments=1");

        ad->refresh_fps = gnome_config_get_int("sound-monitor/fps=10");
	ad->theme_file = gnome_config_get_string("sound-monitor/theme=");

	ad->esd_host = gnome_config_get_string("sound-monitor/host=");
        ad->monitor_input = gnome_config_get_int("sound-monitor/monitor_input=0");

	if (ad->theme_file && strlen(ad->theme_file) == 0)
		{
		g_free(ad->theme_file);
		ad->theme_file = NULL;
		}

	if (ad->esd_host && strlen(ad->esd_host) == 0)
		{
		g_free(ad->esd_host);
		ad->esd_host = NULL;
		}

        gnome_config_pop_prefix ();
}

void property_save(const gchar *path, AppData *ad)
{
	gnome_config_push_prefix(path);

	gnome_config_set_int("sound-monitor/peak", ad->peak_mode);
	gnome_config_set_int("sound-monitor/falloff", ad->falloff_speed);
	gnome_config_set_int("sound-monitor/scale", ad->scope_scale);
	gnome_config_set_int("sound-monitor/segments", ad->draw_scope_as_segments);

	gnome_config_set_int("sound-monitor/fps", ad->refresh_fps);
	gnome_config_set_string("sound-monitor/theme", ad->theme_file);

	gnome_config_set_string("sound-monitor/host", ad->esd_host);
	gnome_config_set_int("sound-monitor/monitor_input", ad->monitor_input);

	gnome_config_sync();
        gnome_config_pop_prefix();
}

static void peak_mode_off_cb(GtkWidget *w, gpointer data)
{
	AppData *ad = data;
	if (GTK_TOGGLE_BUTTON (w)->active)
		{
		ad->p_peak_mode = PeakMode_Off;
		gnome_property_box_changed(GNOME_PROPERTY_BOX(ad->propwindow));
		}
}

static void peak_mode_active_cb(GtkWidget *w, gpointer data)
{
	AppData *ad = data;
	if (GTK_TOGGLE_BUTTON (w)->active)
		{
		ad->p_peak_mode = PeakMode_Active;
		gnome_property_box_changed(GNOME_PROPERTY_BOX(ad->propwindow));
		}
}

static void peak_mode_smooth_cb(GtkWidget *w, gpointer data)
{
	AppData *ad = data;
	if (GTK_TOGGLE_BUTTON (w)->active)
		{
		ad->p_peak_mode = PeakMode_Smooth;
		gnome_property_box_changed(GNOME_PROPERTY_BOX(ad->propwindow));
		}
}

static void falloff_speed_cb(GtkObject *adj, gpointer data)
{
	AppData *ad = data;
	ad->p_falloff_speed = (gint)GTK_ADJUSTMENT(adj)->value;
	gnome_property_box_changed(GNOME_PROPERTY_BOX(ad->propwindow));
}

static void refresh_fps_cb(GtkObject *adj, gpointer data)
{
	AppData *ad = data;
	ad->p_refresh_fps = (gint)GTK_ADJUSTMENT(adj)->value;
	gnome_property_box_changed(GNOME_PROPERTY_BOX(ad->propwindow));
}

static void scope_scale_cb(GtkObject *adj, gpointer data)
{
	AppData *ad = data;
	ad->p_scope_scale = (gint)GTK_ADJUSTMENT(adj)->value;
	gnome_property_box_changed(GNOME_PROPERTY_BOX(ad->propwindow));
}

static void scope_segment_cb(GtkWidget *w, gpointer data)
{
	AppData *ad = data;
	ad->p_draw_scope_as_segments = GTK_TOGGLE_BUTTON (w)->active;
	gnome_property_box_changed(GNOME_PROPERTY_BOX(ad->propwindow));
}

static void monitor_input_cb(GtkWidget *w, gpointer data)
{
	AppData *ad = data;
	ad->p_monitor_input = GTK_TOGGLE_BUTTON (w)->active;
	gnome_property_box_changed(GNOME_PROPERTY_BOX(ad->propwindow));

	/* alert user this may be dangerous */
	if (ad->p_monitor_input)
		{
		gnome_warning_dialog(_("You have enabled the \"Monitor sound input\" option.\n\nIf your sound card is not fully 100% FULL DUPLEX (can do input and output simultaneously),\nthen this may touch bugs in sound drivers/esound that may hang the system.\n\nYou have been warned!"));
		}
}

static void theme_selected_cb(GtkWidget *clist, gint row, gint column,
		GdkEventButton *bevent, gpointer data)
{
	AppData *ad = data;
	gchar *text = gtk_clist_get_row_data(GTK_CLIST(clist), row);
	if (text) gtk_entry_set_text(GTK_ENTRY(ad->theme_entry),text);
	return;
	column = 0;
	bevent = NULL;
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

	themepath = gnome_unconditional_datadir_file("sound-monitor");

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
		gchar *themedata_file = g_strconcat(themepath, "/", list->data, "/themedata", NULL);
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
	gint info_changed = FALSE;

	if (page_num != -1) return;

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

	ad->peak_mode = ad->p_peak_mode;
	ad->falloff_speed = ad->p_falloff_speed;
	ad->scope_scale = ad->p_scope_scale;
	ad->draw_scope_as_segments = ad->p_draw_scope_as_segments;

	if (ad->p_refresh_fps != ad->refresh_fps)
		{
		ad->refresh_fps = ad->p_refresh_fps;
		update_fps_timeout(ad);
		}

	update_modes(ad);

	if (ad->monitor_input != ad->p_monitor_input)
		{
		info_changed = TRUE;
		ad->monitor_input = ad->p_monitor_input;
		}

	buf = gtk_entry_get_text(GTK_ENTRY(ad->host_entry));
	if ((!ad->esd_host && buf && strlen(buf) > 0) ||
	    (ad->esd_host && !buf) ||
	    (ad->esd_host && buf && strcmp(ad->esd_host, buf) != 0))
		{
		g_free(ad->esd_host);
		ad->esd_host = NULL;
		if (buf && strlen(buf) > 0) ad->esd_host = g_strdup(buf);
		info_changed = TRUE;
		}

	/* reset sound input if relevent options changed */
	if (info_changed)
		{
		sound_free(ad->sound);
		ad->sound = sound_init(ad->esd_host, ad->monitor_input);
		}

	applet_widget_sync_config(APPLET_WIDGET(ad->applet));
	return;
	widget = NULL;
}

static void
phelp_cb (GtkWidget *w, gint tab, gpointer data)
{
	GnomeHelpMenuEntry help_entry = { "sound-monitor_applet",
					  "index.html#SOUNDMONITORAPPLET-PREFS" };
	gnome_help_display(NULL, &help_entry);
}

static gint property_destroy_cb(GtkWidget *widget, gpointer data)
{
	AppData *ad = data;
	ad->propwindow = NULL;
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
	GtkWidget *radio_group;
	GtkObject *adj;
	GtkWidget *hscale;
	GtkWidget *scrolled;
	GtkWidget *theme_clist;
	gchar *theme_title[] = { "Themes:", };

	if(ad->propwindow)
		{
                gdk_window_raise(ad->propwindow->window);
                return;
		}

	ad->p_refresh_fps = ad->refresh_fps;

	ad->p_peak_mode = ad->peak_mode;
	ad->p_falloff_speed = ad->falloff_speed;
	ad->p_scope_scale = ad->scope_scale;
	ad->p_draw_scope_as_segments = ad->draw_scope_as_segments;
	ad->p_monitor_input = ad->monitor_input;

	ad->propwindow = gnome_property_box_new();
	gtk_window_set_title(GTK_WINDOW(&GNOME_PROPERTY_BOX(ad->propwindow)->dialog.window),
		_("Sound Monitor Applet Settings"));
	
	vbox = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), GNOME_PAD_SMALL);

	frame = gtk_frame_new(_("Peak indicator"));
	gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);
	gtk_widget_show(frame);

	vbox1 = gtk_hbox_new(TRUE, GNOME_PAD_SMALL);
	gtk_container_set_border_width(GTK_CONTAINER(vbox1), GNOME_PAD_SMALL);
	gtk_container_add(GTK_CONTAINER(frame), vbox1);
	gtk_widget_show(vbox1);

	radio_group = gtk_radio_button_new_with_label (NULL, _("off"));
	if (ad->p_peak_mode == PeakMode_Off)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(radio_group), 1);
	gtk_signal_connect (GTK_OBJECT(radio_group),"clicked",(GtkSignalFunc) peak_mode_off_cb, ad);
	gtk_box_pack_start(GTK_BOX(vbox1), radio_group, FALSE, FALSE, 0);
	gtk_widget_show(radio_group);

	button = gtk_radio_button_new_with_label (gtk_radio_button_group(GTK_RADIO_BUTTON(radio_group)), _("active"));
	if (ad->p_peak_mode == PeakMode_Active)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(button), 1);
	gtk_signal_connect (GTK_OBJECT(button),"clicked",(GtkSignalFunc) peak_mode_active_cb, ad);
	gtk_box_pack_start(GTK_BOX(vbox1), button, FALSE, FALSE, 0);
	gtk_widget_show(button);

	button = gtk_radio_button_new_with_label (gtk_radio_button_group(GTK_RADIO_BUTTON(radio_group)), _("smooth"));
	if (ad->p_peak_mode == PeakMode_Smooth)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(button), 1);
	gtk_signal_connect (GTK_OBJECT(button),"clicked",(GtkSignalFunc) peak_mode_smooth_cb, ad);
	gtk_box_pack_start(GTK_BOX(vbox1), button, FALSE, FALSE, 0);
	gtk_widget_show(button);

	frame = gtk_frame_new(_("Peak indicator falloff speed"));
	gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);
	gtk_widget_show(frame);

	vbox1 = gtk_hbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox1), GNOME_PAD_SMALL);
	gtk_container_add(GTK_CONTAINER(frame), vbox1);
	gtk_widget_show(vbox1);

	adj = gtk_adjustment_new((float)ad->p_falloff_speed, 1.0, 10.0, 1.0, 1.0, 0.0);
	hscale = gtk_hscale_new (GTK_ADJUSTMENT(adj));
	gtk_range_set_update_policy (GTK_RANGE (hscale), GTK_UPDATE_DELAYED);
	gtk_scale_set_digits (GTK_SCALE (hscale), 0);
	gtk_signal_connect( GTK_OBJECT(adj),"value_changed",GTK_SIGNAL_FUNC(falloff_speed_cb), ad);
	gtk_box_pack_start(GTK_BOX(vbox1), hscale, TRUE, TRUE, 0);
	gtk_widget_show(hscale);

	frame = gtk_frame_new(_("Scope (scale 1:X, where X = ? )"));
	gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);
	gtk_widget_show(frame);

	vbox1 = gtk_vbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox1), GNOME_PAD_SMALL);
	gtk_container_add(GTK_CONTAINER(frame), vbox1);
	gtk_widget_show(vbox1);

	adj = gtk_adjustment_new((float)ad->p_scope_scale, 1.0, 10.0, 1.0, 1.0, 0.0);
	hscale = gtk_hscale_new (GTK_ADJUSTMENT(adj));
	gtk_range_set_update_policy (GTK_RANGE (hscale), GTK_UPDATE_DELAYED);
	gtk_scale_set_digits (GTK_SCALE (hscale), 0);
	gtk_signal_connect( GTK_OBJECT(adj),"value_changed",GTK_SIGNAL_FUNC(scope_scale_cb), ad);
	gtk_box_pack_start(GTK_BOX(vbox1), hscale, FALSE, FALSE, 0);
	gtk_widget_show(hscale);

	button = gtk_check_button_new_with_label (_("Connect points in scope"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), ad->p_draw_scope_as_segments);
	gtk_signal_connect (GTK_OBJECT(button),"clicked",(GtkSignalFunc) scope_segment_cb, ad);
	gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);
	gtk_widget_show(button);

	frame = gtk_frame_new(_("Screen refresh (frames per second)"));
	gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);
	gtk_widget_show(frame);

	vbox1 = gtk_vbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox1), GNOME_PAD_SMALL);
	gtk_container_add(GTK_CONTAINER(frame), vbox1);
	gtk_widget_show(vbox1);

	adj = gtk_adjustment_new((float)ad->p_refresh_fps, 5.0, 40.0, 1.0, 1.0, 0.0);
	hscale = gtk_hscale_new (GTK_ADJUSTMENT(adj));
	gtk_range_set_update_policy (GTK_RANGE (hscale), GTK_UPDATE_DELAYED);
	gtk_scale_set_digits (GTK_SCALE (hscale), 0);
	gtk_signal_connect( GTK_OBJECT(adj),"value_changed",GTK_SIGNAL_FUNC(refresh_fps_cb), ad);
	gtk_box_pack_start(GTK_BOX(vbox1), hscale, FALSE, FALSE, 0);
	gtk_widget_show(hscale);

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
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

	label = gtk_label_new(_("Theme file (directory):"));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_widget_show(label);

	ad->theme_entry = gtk_entry_new();
	if (ad->theme_file)
		gtk_entry_set_text(GTK_ENTRY(ad->theme_entry), ad->theme_file);
	gtk_signal_connect_object(GTK_OBJECT(ad->theme_entry), "changed",
				GTK_SIGNAL_FUNC(gnome_property_box_changed),
				GTK_OBJECT(ad->propwindow));
	gtk_box_pack_start(GTK_BOX(hbox),ad->theme_entry , TRUE, TRUE, 0);
	gtk_widget_show(ad->theme_entry);

	scrolled = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
				GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
	gtk_box_pack_start(GTK_BOX(vbox), scrolled, TRUE, TRUE, 0);
	gtk_widget_show(scrolled);

	/* theme list */
	theme_clist=gtk_clist_new_with_titles (1, theme_title);
	gtk_clist_column_titles_passive (GTK_CLIST (theme_clist)); 
	gtk_signal_connect (GTK_OBJECT (theme_clist), "select_row",(GtkSignalFunc) theme_selected_cb, ad);
	gtk_container_add (GTK_CONTAINER (scrolled), theme_clist);
	gtk_widget_show (theme_clist);

	populate_theme_list(theme_clist);

        label = gtk_label_new(_("Theme"));
        gtk_widget_show(frame);
        gnome_property_box_append_page( GNOME_PROPERTY_BOX(ad->propwindow),frame ,label);

	/* advanced */

	frame = gtk_frame_new(NULL);
	gtk_container_set_border_width(GTK_CONTAINER(frame), GNOME_PAD_SMALL);

	vbox = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_container_set_border_width(GTK_CONTAINER (vbox), GNOME_PAD_SMALL);
	gtk_container_add(GTK_CONTAINER(frame), vbox);
	gtk_widget_show(vbox);

	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

	label = gtk_label_new(_("ESD host to monitor:"));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_widget_show(label);

	ad->host_entry = gtk_entry_new();
	if (ad->esd_host)
		gtk_entry_set_text(GTK_ENTRY(ad->host_entry), ad->esd_host);
	gtk_signal_connect_object(GTK_OBJECT(ad->host_entry), "changed",
				GTK_SIGNAL_FUNC(gnome_property_box_changed),
				GTK_OBJECT(ad->propwindow));
	gtk_box_pack_start(GTK_BOX(hbox), ad->host_entry, TRUE, TRUE, 0);
	gtk_widget_show(ad->host_entry);

	button = gtk_check_button_new_with_label (_("Monitor sound input (only use if sound card is FULL DUPLEX)"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), ad->p_monitor_input);
	gtk_signal_connect (GTK_OBJECT(button),"clicked",(GtkSignalFunc) monitor_input_cb, ad);
	gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);
	gtk_widget_show(button);

        label = gtk_label_new(_("Advanced"));
        gtk_widget_show(frame);
        gnome_property_box_append_page( GNOME_PROPERTY_BOX(ad->propwindow),frame ,label);

	gtk_signal_connect( GTK_OBJECT(ad->propwindow),"apply", GTK_SIGNAL_FUNC(property_apply_cb), ad);
	gtk_signal_connect( GTK_OBJECT(ad->propwindow),"destroy", GTK_SIGNAL_FUNC(property_destroy_cb), ad );
	gtk_signal_connect( GTK_OBJECT(ad->propwindow),"help", GTK_SIGNAL_FUNC(phelp_cb), NULL );
	gtk_widget_show_all(ad->propwindow);
	return;
	applet = NULL;
} 

