/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "drivemount.h"
#include "properties.h"

static void set_widget_sensitivity_false_cb(GtkWidget *widget, GtkWidget *target);
static void set_widget_sensitivity_true_cb(GtkWidget *widget, GtkWidget *target);

static gchar *remove_level_from_path(const gchar *path);
static void sync_mount_base(DriveData *dd);


void
property_load(const gchar *path, DriveData *dd)
{
/*
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
*/
}

void
property_save(const gchar *path, DriveData *dd)
{
/*
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
*/
}

void
property_show(PanelApplet *applet, gpointer data)
{
	DriveData *dd = data;
	GtkWidget *dialog;
	GtkWidget *box;
	GtkWidget *frame;
	GtkWidget *hbox;
	GtkWidget *vbox;
	GtkWidget *fbox;
	GtkWidget *label;
	GtkWidget *omenu;
	GtkWidget *menu;
	GtkWidget *item;
	GtkWidget *mount_entry;
	GtkWidget *update_spin;
	GtkWidget *icon_entry_in;
	GtkWidget *icon_entry_out;
	GtkWidget *scale_toggle;
	GtkWidget *eject_toggle;
	GtkWidget *automount_toggle;
	gint response;

    dialog = gtk_dialog_new_with_buttons(_("Drive Mount Applet Properties"),
										 NULL, GTK_DIALOG_MODAL,
										 GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
										 GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);

    box = GTK_DIALOG(dialog)->vbox;
	frame = gtk_frame_new("Settings");
	gtk_container_set_border_width(GTK_CONTAINER(frame), GNOME_PAD_SMALL);
    gtk_box_pack_start(GTK_BOX(box), frame, TRUE, TRUE, 0);

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

	mount_entry = gtk_entry_new_with_max_length(255);
	gtk_entry_set_text(GTK_ENTRY(mount_entry), dd->mount_point);
	gtk_box_pack_start(GTK_BOX(hbox), mount_entry , TRUE, TRUE, 0);
	gtk_widget_show(mount_entry);

	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

	label = gtk_label_new(_("Update in seconds:"));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_widget_show(label);

	update_spin = gtk_spin_button_new(GTK_ADJUSTMENT(gtk_adjustment_new(dd->interval, 1.0, 30.0, 1, 1, 1)), 1, 0);
	gtk_box_pack_start(GTK_BOX(hbox), update_spin, FALSE, FALSE, 0);
	gtk_spin_button_set_update_policy(GTK_SPIN_BUTTON(update_spin),GTK_UPDATE_ALWAYS);
	gtk_widget_show(update_spin);

	label = gtk_label_new(_("Icon:"));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_widget_show(label);

	omenu = gtk_option_menu_new ();
	gtk_box_pack_start(GTK_BOX(hbox), omenu, TRUE, TRUE, 0);
	gtk_widget_show (omenu);
	menu = gtk_menu_new ();
	gtk_option_menu_set_menu(GTK_OPTION_MENU (omenu), menu);

	/* This must be created before the menu items, so we can pass it to a callback */
	fbox = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(vbox), fbox, FALSE, FALSE, 0);
	gtk_widget_show(fbox);

	item = gtk_menu_item_new_with_label(_("Floppy"));
	gtk_menu_append (GTK_MENU (menu), item);
	g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(set_widget_sensitivity_false_cb), fbox);
	item = gtk_menu_item_new_with_label(_("Cdrom"));
	gtk_menu_append (GTK_MENU (menu), item);
	g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(set_widget_sensitivity_false_cb), fbox);
	item = gtk_menu_item_new_with_label(_("Zip Drive"));
	gtk_menu_append (GTK_MENU (menu), item);
	g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(set_widget_sensitivity_false_cb), fbox);
	item = gtk_menu_item_new_with_label(_("Hard Disk"));
	gtk_menu_append (GTK_MENU (menu), item);
	g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(set_widget_sensitivity_false_cb), fbox);
	item = gtk_menu_item_new_with_label(_("Jaz Drive"));
	gtk_menu_append (GTK_MENU (menu), item);
	g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(set_widget_sensitivity_false_cb), fbox);
	item = gtk_menu_item_new_with_label(_("Custom"));
	gtk_menu_append (GTK_MENU (menu), item);
	g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(set_widget_sensitivity_true_cb), fbox);

	if (dd->device_pixmap == -1)
		gtk_option_menu_set_history (GTK_OPTION_MENU (omenu), 5);
	else
		gtk_option_menu_set_history (GTK_OPTION_MENU (omenu), dd->device_pixmap);

	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(fbox), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

	icon_entry_in = gnome_icon_entry_new("none", _("Select icon for mounted"));
	gnome_icon_entry_set_filename(GNOME_ICON_ENTRY(icon_entry_in), dd->custom_icon_in);
	gtk_box_pack_end(GTK_BOX(hbox), icon_entry_in, FALSE, FALSE, 0);
	gtk_widget_show(icon_entry_in);

	label = gtk_label_new(_("Custom icon for mounted:"));
	gtk_box_pack_end(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_widget_show(label);

	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(fbox), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

	icon_entry_out = gnome_icon_entry_new("none", _("Select icon for unmounted"));
	gnome_icon_entry_set_filename(GNOME_ICON_ENTRY(icon_entry_out), dd->custom_icon_out);
	gtk_box_pack_end(GTK_BOX(hbox), icon_entry_out, FALSE, FALSE, 0);
	gtk_widget_show(icon_entry_out);

	label = gtk_label_new(_("Custom icon for not mounted:"));
	gtk_box_pack_end(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_widget_show(label);

    if(dd->device_pixmap < 0)
       gtk_widget_set_sensitive(fbox, TRUE);
    else
       gtk_widget_set_sensitive(fbox, FALSE);

	scale_toggle = gtk_check_button_new_with_label (_("Scale size to panel"));
	gtk_box_pack_start(GTK_BOX(vbox), scale_toggle, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(scale_toggle), dd->scale_applet);
	gtk_widget_show(scale_toggle);

	eject_toggle = gtk_check_button_new_with_label (_("Eject on unmount"));
	gtk_box_pack_start(GTK_BOX(vbox), eject_toggle, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(eject_toggle), dd->auto_eject);
	gtk_widget_show(eject_toggle);

	automount_toggle = gtk_check_button_new_with_label (_("Use automount friendly status test"));
	gtk_box_pack_start(GTK_BOX(vbox), automount_toggle, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(automount_toggle), dd->autofs_friendly);
	gtk_widget_show(automount_toggle);
	gtk_widget_show_all(frame);
    gtk_widget_queue_resize(omenu);
	response = gtk_dialog_run(GTK_DIALOG(dialog));

	if(response == GTK_RESPONSE_OK)
	{
		gchar *temp;
		gint history;

		temp = (gchar *)gtk_entry_get_text(GTK_ENTRY(mount_entry));
		if(dd->mount_point) g_free(dd->mount_point);
		if(temp) dd->mount_point = g_strdup(temp);
		else dd->mount_point = NULL;

    	dd->interval = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(update_spin));

		history = gtk_option_menu_get_history(GTK_OPTION_MENU(omenu));
		dd->device_pixmap = history < 5 ? history : -1;

		temp = gnome_icon_entry_get_filename(GNOME_ICON_ENTRY(icon_entry_in));
		if(dd->custom_icon_in) g_free(dd->custom_icon_in);
		if(temp) dd->custom_icon_in = temp;
		else dd->custom_icon_in = NULL;

		temp = gnome_icon_entry_get_filename(GNOME_ICON_ENTRY(icon_entry_out));
		if(dd->custom_icon_out) g_free(dd->custom_icon_out);
		if(temp) dd->custom_icon_out = temp;
		else dd->custom_icon_out = NULL;

        dd->scale_applet = GTK_TOGGLE_BUTTON(scale_toggle)->active;
        dd->auto_eject = GTK_TOGGLE_BUTTON(eject_toggle)->active;
        dd->autofs_friendly = GTK_TOGGLE_BUTTON(automount_toggle)->active;
		redraw_pixmap(dd);
		start_callback_update(dd);
	}
	gtk_widget_destroy(dialog);
}

static void
set_widget_sensitivity_false_cb(GtkWidget *widget, GtkWidget *target)
{
	gtk_widget_set_sensitive(target, FALSE);
}

static void
set_widget_sensitivity_true_cb(GtkWidget *widget, GtkWidget *target)
{
	gtk_widget_set_sensitive(target, TRUE);
}

static gchar *
remove_level_from_path(const gchar *path)
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

static void
sync_mount_base(DriveData *dd)
{
	g_free(dd->mount_base);
	dd->mount_base = remove_level_from_path(dd->mount_point);
}
