/* GNOME drivemount applet
 * (C) 1999 John Ellis
 *
 * Author: John Ellis
 *
 */

#include "drivemount.h"

static gchar *remove_level_from_path(const gchar *path)
{
	gchar *new_path;
	const gchar *ptr;
	gint p;

	if (!path) return NULL;

	p = strlen(path) - 1;
	if (p < 0) return NULL;

	ptr = path;
	while(ptr[p] != '/' && p > 0) p--;

	if (p == 0 && ptr[p] == '/') p++;
	new_path = g_strndup(path, (guint)p);
	return new_path;
}

static void sync_mount_base(DriveData *dd)
{
	g_free(dd->mount_base);
	dd->mount_base = remove_level_from_path(dd->mount_point);
}

void property_load(const gchar *path, DriveData *dd)
{
        gnome_config_push_prefix (path);

        dd->interval = gnome_config_get_int("mount/interval=10");
	dd->device_pixmap = gnome_config_get_int("mount/pixmap=0");
	dd->scale_applet = gnome_config_get_int("mount/scale=0");

	dd->auto_eject = gnome_config_get_int("mount/auto_eject=0");

	g_free(dd->mount_point);
	dd->mount_point = gnome_config_get_string("mount/mountpoint=/mnt/floppy");
	sync_mount_base(dd);
	dd->autofs_friendly = gnome_config_get_int("mount/autofs_friendly=0");

	g_free(dd->custom_icon_in);
	dd->custom_icon_in = gnome_config_get_string("mount/custom_icon_mounted=");
	if (dd->custom_icon_in && strlen(dd->custom_icon_in) < 1)
		{
		g_free(dd->custom_icon_in);
		dd->custom_icon_in = NULL;
		}

	g_free(dd->custom_icon_out);
	dd->custom_icon_out = gnome_config_get_string("mount/custom_icon_unmounted=");
	if (dd->custom_icon_out && strlen(dd->custom_icon_out) < 1)
		{
		g_free(dd->custom_icon_out);
		dd->custom_icon_out = NULL;
		}

	gnome_config_pop_prefix ();
}

void property_save(const gchar *path, DriveData *dd)
{
        gnome_config_push_prefix(path);

        gnome_config_set_int("mount/interval", dd->interval);
        gnome_config_set_int("mount/pixmap", dd->device_pixmap);
        gnome_config_set_int("mount/scale", dd->scale_applet);

	gnome_config_set_int("mount/auto_eject", dd->auto_eject);

        gnome_config_set_string("mount/mountpoint", dd->mount_point);
        gnome_config_set_int("mount/autofs_friendly", dd->autofs_friendly);

	gnome_config_set_string("mount/custom_icon_mounted", dd->custom_icon_in);
	gnome_config_set_string("mount/custom_icon_unmounted", dd->custom_icon_out);

        gnome_config_pop_prefix();
	gnome_config_sync();
	gnome_config_drop_all();
}

static void set_icon_entry_sensitivity(DriveData *dd, gint val)
{
	gtk_widget_set_sensitive(dd->icon_entry_in, val);
	gtk_widget_set_sensitive(dd->icon_entry_out, val);
}

static void autofs_friendly_cb(GtkWidget *w, gpointer data)
{
	DriveData *dd = data;
	dd->prop_autofs_friendly = GTK_TOGGLE_BUTTON (w)->active;
	gnome_property_box_changed(GNOME_PROPERTY_BOX(dd->propwindow));
}

static void scale_applet_cb(GtkWidget *w, gpointer data)
{
	DriveData *dd = data;
	dd->prop_scale_applet = GTK_TOGGLE_BUTTON (w)->active;
	gnome_property_box_changed(GNOME_PROPERTY_BOX(dd->propwindow));
}

static void auto_eject_applet_cb(GtkWidget *w, gpointer data)
{
	DriveData *dd = data;
	dd->prop_auto_eject = GTK_TOGGLE_BUTTON (w)->active;
	gnome_property_box_changed(GNOME_PROPERTY_BOX(dd->propwindow));
}

static void pixmap_floppy_cb(GtkWidget *widget, gpointer data)
{
	DriveData *dd = data;
	dd->prop_device_pixmap = 0;
	set_icon_entry_sensitivity(dd, FALSE);
        gnome_property_box_changed(GNOME_PROPERTY_BOX(dd->propwindow));
	return;
}

static void pixmap_cdrom_cb(GtkWidget *widget, gpointer data)
{
	DriveData *dd = data;
	dd->prop_device_pixmap = 1;
	set_icon_entry_sensitivity(dd, FALSE);
        gnome_property_box_changed(GNOME_PROPERTY_BOX(dd->propwindow));
        return;
}

static void pixmap_zipdrive_cb(GtkWidget *widget, gpointer data)
{
	DriveData *dd = data;
	dd->prop_device_pixmap = 2;
	set_icon_entry_sensitivity(dd, FALSE);
        gnome_property_box_changed(GNOME_PROPERTY_BOX(dd->propwindow));
        return;
}

static void pixmap_jazdrive_cb(GtkWidget *widget, gpointer data)
{
	DriveData *dd = data;
	dd->prop_device_pixmap = 4;
	set_icon_entry_sensitivity(dd, FALSE);
	gnome_property_box_changed(GNOME_PROPERTY_BOX(dd->propwindow));
        return;
}

static void pixmap_harddisk_cb(GtkWidget *widget, gpointer data)
{
	DriveData *dd = data;
	dd->prop_device_pixmap = 3;
	set_icon_entry_sensitivity(dd, FALSE);
        gnome_property_box_changed(GNOME_PROPERTY_BOX(dd->propwindow));
        return;
}

static void pixmap_custom_cb(GtkWidget *widget, gpointer data)
{
	DriveData *dd = data;
	dd->prop_device_pixmap = -1;
	set_icon_entry_sensitivity(dd, TRUE);
        gnome_property_box_changed(GNOME_PROPERTY_BOX(dd->propwindow));
        return;
}

static void update_delay_cb(GtkWidget *widget, gpointer data)
{
	DriveData *dd = data;
        dd->prop_interval = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(dd->prop_spin));
        gnome_property_box_changed(GNOME_PROPERTY_BOX(dd->propwindow));
        return;
}

static void property_apply_cb(GtkWidget *propertybox, gint page_num, DriveData *dd)
{
	gchar *new_file;

	if(page_num != -1) return;	/* only do this once, on global signal */

	dd->auto_eject = dd->prop_auto_eject;

	dd->scale_applet = dd->prop_scale_applet;
	dd->autofs_friendly = dd->prop_autofs_friendly;
	new_file = gtk_entry_get_text(GTK_ENTRY(dd->mount_point_entry));
	if (dd->mount_point) g_free(dd->mount_point);
	dd->mount_point = g_strdup(new_file);
	sync_mount_base(dd);
        dd->interval = dd->prop_interval;
	if (dd->device_pixmap != dd->prop_device_pixmap)
		{
		dd->device_pixmap = dd->prop_device_pixmap;
		}

	g_free(dd->custom_icon_in);
	g_free(dd->custom_icon_out);
	dd->custom_icon_in = gnome_icon_entry_get_filename(GNOME_ICON_ENTRY(dd->icon_entry_in));
	dd->custom_icon_out = gnome_icon_entry_get_filename(GNOME_ICON_ENTRY(dd->icon_entry_out));

	redraw_pixmap(dd);
	start_callback_update(dd);

	/*make the panel save our config*/
	applet_widget_sync_config(APPLET_WIDGET(dd->applet));
        return;
}

static gint property_destroy_cb( GtkWidget *w, DriveData *dd)
{
        dd->propwindow = NULL;
	return FALSE;
}

static void
phelp_cb (GtkWidget *w, gint tab, gpointer data)
{
	GnomeHelpMenuEntry help_entry = { "drivemount_applet",
					  "index.html#DRIVEMOUNTAPPLET-PREFS" };
	gnome_help_display(NULL, &help_entry);
}

void property_show(AppletWidget *applet, gpointer data)
{
        static GnomeHelpMenuEntry help_entry = { NULL, "properties" };
	DriveData *dd = data;
	GtkWidget *frame;
	GtkWidget *hbox;
	GtkWidget *vbox;
	GtkWidget *label;
	GtkWidget *omenu;
	GtkWidget *menu;
	GtkWidget *item;
	GtkObject *delay_adj;
	GtkWidget *button;

	help_entry.name = gnome_app_id;

	if(dd->propwindow)
		{
                gdk_window_raise(dd->propwindow->window);
                return;
		}

        dd->prop_interval = dd->interval;
	dd->prop_device_pixmap = dd->device_pixmap;
	dd->prop_autofs_friendly = dd->autofs_friendly;
	dd->prop_scale_applet = dd->scale_applet;
	dd->prop_auto_eject = dd->auto_eject;

	dd->propwindow = gnome_property_box_new();
	gtk_window_set_title(GTK_WINDOW(GNOME_PROPERTY_BOX(dd->propwindow)),
		_("Drive Mount Applet Properties"));

	frame = gtk_frame_new(NULL);
	gtk_container_set_border_width(GTK_CONTAINER(frame), GNOME_PAD_SMALL);

	vbox = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), GNOME_PAD_SMALL);
	gtk_container_add(GTK_CONTAINER(frame), vbox);
	gtk_widget_show(vbox);

	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
        gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

        label = gtk_label_new(_("Mount point:"));
        gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_widget_show(label);

	dd->mount_point_entry = gtk_entry_new_with_max_length(255);
	gtk_entry_set_text(GTK_ENTRY(dd->mount_point_entry), dd->mount_point);
	gtk_signal_connect_object(GTK_OBJECT(dd->mount_point_entry), "changed",
                            GTK_SIGNAL_FUNC(gnome_property_box_changed),
                            GTK_OBJECT(dd->propwindow));
        gtk_box_pack_start(GTK_BOX(hbox),dd->mount_point_entry , TRUE, TRUE, 0);
	gtk_widget_show(dd->mount_point_entry);

	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
        gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

        label = gtk_label_new(_("Update in seconds:"));
        gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_widget_show(label);

	delay_adj = gtk_adjustment_new( dd->prop_interval, 1.0, 30.0, 1, 1, 1 );
        dd->prop_spin = gtk_spin_button_new( GTK_ADJUSTMENT(delay_adj), 1, 0 );
        gtk_box_pack_start(GTK_BOX(hbox), dd->prop_spin, FALSE, FALSE, 0);
	gtk_signal_connect(GTK_OBJECT(delay_adj),"value_changed",GTK_SIGNAL_FUNC(update_delay_cb), dd);
	gtk_signal_connect(GTK_OBJECT(dd->prop_spin),"changed",GTK_SIGNAL_FUNC(update_delay_cb), dd);
        gtk_spin_button_set_update_policy( GTK_SPIN_BUTTON(dd->prop_spin),GTK_UPDATE_ALWAYS );
	gtk_widget_show(dd->prop_spin);

        label = gtk_label_new(_("Icon:"));
        gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_widget_show(label);

	omenu = gtk_option_menu_new ();
	menu = gtk_menu_new ();

	item = gtk_menu_item_new_with_label(_("Floppy"));
	gtk_signal_connect (GTK_OBJECT (item), "activate", (GtkSignalFunc) pixmap_floppy_cb, dd);
	gtk_menu_append (GTK_MENU (menu), item);

	item = gtk_menu_item_new_with_label(_("Cdrom"));
	gtk_signal_connect (GTK_OBJECT (item), "activate", (GtkSignalFunc) pixmap_cdrom_cb, dd);
	gtk_menu_append (GTK_MENU (menu), item);

	item = gtk_menu_item_new_with_label(_("Zip Drive"));
	gtk_signal_connect (GTK_OBJECT (item), "activate", (GtkSignalFunc) pixmap_zipdrive_cb, dd);
	gtk_menu_append (GTK_MENU (menu), item);

	item = gtk_menu_item_new_with_label(_("Hard Disk"));
	gtk_signal_connect (GTK_OBJECT (item), "activate", (GtkSignalFunc) pixmap_harddisk_cb, dd);
	gtk_menu_append (GTK_MENU (menu), item);

	item = gtk_menu_item_new_with_label(_("Jaz Drive"));
	gtk_signal_connect (GTK_OBJECT (item), "activate", (GtkSignalFunc) pixmap_jazdrive_cb, dd);
	gtk_menu_append (GTK_MENU (menu), item);

	item = gtk_menu_item_new_with_label(_("Custom"));
	gtk_signal_connect (GTK_OBJECT (item), "activate", (GtkSignalFunc) pixmap_custom_cb, dd);
	gtk_menu_append (GTK_MENU (menu), item);

	gtk_option_menu_set_menu (GTK_OPTION_MENU (omenu), menu);

	if (dd->prop_device_pixmap == -1 /* custom */)
		gtk_option_menu_set_history (GTK_OPTION_MENU (omenu), 5);
	else
		gtk_option_menu_set_history (GTK_OPTION_MENU (omenu), dd->prop_device_pixmap);

        gtk_box_pack_start(GTK_BOX(hbox), omenu, TRUE, TRUE, 0);
	gtk_widget_show (omenu);

	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
        gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

	dd->icon_entry_in = gnome_icon_entry_new("icon_in", _("Select icon for mounted"));
	gnome_icon_entry_set_icon(GNOME_ICON_ENTRY(dd->icon_entry_in), dd->custom_icon_in);
	gtk_signal_connect_object(GTK_OBJECT(gnome_icon_entry_gtk_entry(GNOME_ICON_ENTRY(dd->icon_entry_in))),
				  "changed", GTK_SIGNAL_FUNC(gnome_property_box_changed),
				  GTK_OBJECT(dd->propwindow));
	gtk_box_pack_end(GTK_BOX(hbox), dd->icon_entry_in, FALSE, FALSE, 0);
	gtk_widget_show(dd->icon_entry_in);

	label = gtk_label_new(_("Custom icon for mounted:"));
	gtk_box_pack_end(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_widget_show(label);

	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
        gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

	dd->icon_entry_out = gnome_icon_entry_new("icon_in", _("Select icon for unmounted"));
	gnome_icon_entry_set_icon(GNOME_ICON_ENTRY(dd->icon_entry_out), dd->custom_icon_out);
	gtk_signal_connect_object(GTK_OBJECT(gnome_icon_entry_gtk_entry(GNOME_ICON_ENTRY(dd->icon_entry_out))),
				  "changed", GTK_SIGNAL_FUNC(gnome_property_box_changed),
				  GTK_OBJECT(dd->propwindow));
	gtk_box_pack_end(GTK_BOX(hbox), dd->icon_entry_out, FALSE, FALSE, 0);
	gtk_widget_show(dd->icon_entry_out);

	label = gtk_label_new(_("Custom icon for not mounted:"));
	gtk_box_pack_end(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_widget_show(label);

	set_icon_entry_sensitivity(dd, ( dd->prop_device_pixmap < 0 ) );

	button = gtk_check_button_new_with_label (_("Scale size to panel"));
        gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), dd->scale_applet);
        gtk_signal_connect (GTK_OBJECT(button),"clicked",(GtkSignalFunc) scale_applet_cb, dd);
        gtk_widget_show(button);

	button = gtk_check_button_new_with_label (_("Eject on unmount"));
	gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), dd->auto_eject);
	gtk_signal_connect (GTK_OBJECT(button),"clicked",(GtkSignalFunc) auto_eject_applet_cb, dd);
	gtk_widget_show(button);

	button = gtk_check_button_new_with_label (_("Use automount friendly status test"));
        gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), dd->prop_autofs_friendly);
        gtk_signal_connect (GTK_OBJECT(button),"clicked",(GtkSignalFunc) autofs_friendly_cb, dd);
        gtk_widget_show(button);

        label = gtk_label_new(_("General"));
        gtk_widget_show(frame);
        gnome_property_box_append_page(GNOME_PROPERTY_BOX(dd->propwindow), frame, label);

	gtk_signal_connect( GTK_OBJECT(dd->propwindow), "apply",
			    GTK_SIGNAL_FUNC(property_apply_cb), dd );

	gtk_signal_connect( GTK_OBJECT(dd->propwindow), "destroy",
			    GTK_SIGNAL_FUNC(property_destroy_cb), dd );

	gtk_signal_connect( GTK_OBJECT(dd->propwindow), "help",
			    GTK_SIGNAL_FUNC(phelp_cb), NULL);

	gtk_widget_show_all(dd->propwindow);
	return;
}


