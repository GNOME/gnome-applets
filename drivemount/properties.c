/*#####################################################*/
/*##           drivemount applet 0.1.0 alpha         ##*/
/*#####################################################*/

#include "drivemount.h"

void property_load(char *path, DriveData *dd)
{
	char *mount_text = NULL;

        gnome_config_push_prefix (path);
        dd->interval = gnome_config_get_int("interval=10");
	dd->device_pixmap = gnome_config_get_int("pixmap=0");
	mount_text = gnome_config_get_string("mountpoint=/mnt/floppy");
	gnome_config_pop_prefix ();

	if (dd->mount_point) free(dd->mount_point);
	dd->mount_point = strdup(mount_text);
}

void property_save(char *path, DriveData *dd)
{
        gnome_config_push_prefix(path);
        gnome_config_set_int("interval", dd->interval);
        gnome_config_set_int("pixmap", dd->device_pixmap);
        gnome_config_set_string("mountpoint", dd->mount_point);
        gnome_config_pop_prefix();
	gnome_config_sync();
	gnome_config_drop_all();
}

void pixmap_floppy_cb(GtkWidget *widget, gpointer data)
{
	DriveData *dd = data;
	dd->prop_device_pixmap = 0;
        gnome_property_box_changed(GNOME_PROPERTY_BOX(dd->propwindow));
}

void pixmap_cdrom_cb(GtkWidget *widget, gpointer data)
{
	DriveData *dd = data;
	dd->prop_device_pixmap = 1;
        gnome_property_box_changed(GNOME_PROPERTY_BOX(dd->propwindow));
}

void pixmap_zipdrive_cb(GtkWidget *widget, gpointer data)
{
	DriveData *dd = data;
	dd->prop_device_pixmap = 2;
        gnome_property_box_changed(GNOME_PROPERTY_BOX(dd->propwindow));
}

void pixmap_harddisk_cb(GtkWidget *widget, gpointer data)
{
	DriveData *dd = data;
	dd->prop_device_pixmap = 3;
        gnome_property_box_changed(GNOME_PROPERTY_BOX(dd->propwindow));
}

void update_delay_cb( GtkWidget *widget, gpointer data )
{
	DriveData *dd = data;
        dd->prop_interval = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(dd->prop_spin));
        gnome_property_box_changed(GNOME_PROPERTY_BOX(dd->propwindow));
}

void property_apply_cb( GtkWidget *widget, void *data, DriveData *dd)
{
	gchar *new_file;
	new_file = gtk_entry_get_text(GTK_ENTRY(dd->mount_point_entry));
	if (dd->mount_point) free(dd->mount_point);
	dd->mount_point = strdup(new_file);
        dd->interval = dd->prop_interval;
	if (dd->device_pixmap != dd->prop_device_pixmap)
		{
		dd->device_pixmap = dd->prop_device_pixmap;
		create_pixmaps(dd);
		}
	redraw_pixmap(dd);
	start_callback_update(dd);
}

gint property_destroy_cb( GtkWidget *w, DriveData *dd)
{
        dd->propwindow = NULL;
	return FALSE;
}

void property_show(AppletWidget *applet, gpointer data)
{
	DriveData *dd = data;
	GtkWidget *frame;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *button;
	GtkWidget *omenu;
	GtkWidget *menu;
	GtkWidget *item;
	GtkObject *delay_adj;

	if(dd->propwindow)
		{
                gdk_window_raise(dd->propwindow->window);
                return;
		}

        dd->prop_interval = dd->interval;
	dd->prop_device_pixmap = dd->device_pixmap;

	dd->propwindow = gnome_property_box_new();
	gtk_window_set_title(GTK_WINDOW(&GNOME_PROPERTY_BOX(dd->propwindow)->dialog.window),
		_("Drive Mount Settings"));
	
	frame = gtk_vbox_new(FALSE, 5);

	hbox = gtk_hbox_new(FALSE, 5);
        gtk_box_pack_start( GTK_BOX(frame), hbox, FALSE, FALSE, 5);
	gtk_widget_show(hbox);

        label = gtk_label_new(_("Update in seconds:"));
        gtk_box_pack_start( GTK_BOX(hbox), label, FALSE, FALSE, 5);
	gtk_widget_show(label);

	delay_adj = gtk_adjustment_new( dd->prop_interval, 1.0, 30.0, 1, 1, 1 );
        dd->prop_spin = gtk_spin_button_new( GTK_ADJUSTMENT(delay_adj), 1, 0 );
        gtk_box_pack_start( GTK_BOX(hbox), dd->prop_spin, FALSE, FALSE, 5);
	gtk_signal_connect( GTK_OBJECT(delay_adj),"value_changed",GTK_SIGNAL_FUNC(update_delay_cb), dd);
        gtk_spin_button_set_update_policy( GTK_SPIN_BUTTON(dd->prop_spin),GTK_UPDATE_ALWAYS );
	gtk_widget_show(dd->prop_spin);

        label = gtk_label_new(_("Icon:"));
        gtk_box_pack_start( GTK_BOX(hbox), label, FALSE, FALSE, 5);
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

	gtk_option_menu_set_menu (GTK_OPTION_MENU (omenu), menu);
	gtk_option_menu_set_history (GTK_OPTION_MENU (omenu), dd->prop_device_pixmap);
        gtk_box_pack_start( GTK_BOX(hbox), omenu, TRUE, TRUE, 5);
	gtk_widget_show (omenu);

	hbox = gtk_hbox_new(FALSE, 5);
        gtk_box_pack_start( GTK_BOX(frame), hbox, FALSE, FALSE, 5);
	gtk_widget_show(hbox);

        label = gtk_label_new(_("Mount point:"));
        gtk_box_pack_start( GTK_BOX(hbox), label, FALSE, FALSE, 5);
	gtk_widget_show(label);

	dd->mount_point_entry = gtk_entry_new_with_max_length(255);
	gtk_entry_set_text(GTK_ENTRY(dd->mount_point_entry), dd->mount_point);
	gtk_signal_connect_object(GTK_OBJECT(dd->mount_point_entry), "changed",
                            GTK_SIGNAL_FUNC(gnome_property_box_changed),
                            GTK_OBJECT(dd->propwindow));
        gtk_box_pack_start( GTK_BOX(hbox),dd->mount_point_entry , TRUE, TRUE, 5);
	gtk_widget_show(dd->mount_point_entry);

        label = gtk_label_new(_("General"));
        gtk_widget_show(frame);
        gnome_property_box_append_page( GNOME_PROPERTY_BOX(dd->propwindow),frame ,label);

	gtk_signal_connect( GTK_OBJECT(dd->propwindow),"apply", GTK_SIGNAL_FUNC(property_apply_cb), dd );
	gtk_signal_connect( GTK_OBJECT(dd->propwindow),"destroy", GTK_SIGNAL_FUNC(property_destroy_cb), dd );

        gtk_widget_show_all(dd->propwindow);
} 
