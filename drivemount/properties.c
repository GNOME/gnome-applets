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

#include <panel-applet.h>
#include <panel-applet-gconf.h>

#include "drivemount.h"
#include "properties.h"

typedef struct _ResponseWidgets
{
	DriveData *dd;
	GtkWidget *mount_entry;
	GtkWidget *update_spin;
	GtkWidget *omenu;
	GtkWidget *icon_entry_in;
	GtkWidget *icon_entry_out;
	GtkWidget *scale_toggle;
	GtkWidget *eject_toggle;
	GtkWidget *automount_toggle;
}ResponseWidgets;

static void handle_response_cb(GtkDialog *dialog, gint response, ResponseWidgets *widgets);
static void set_widget_sensitivity_false_cb(GtkWidget *widget, GtkWidget *target);
static void set_widget_sensitivity_true_cb(GtkWidget *widget, GtkWidget *target);
static void browse_icons_dialog_popup_handler(GnomeIconEntry *entry, GtkWidget *parent_dialog);

static gchar *remove_level_from_path(const gchar *path);
static void sync_mount_base(DriveData *dd);

void
properties_load(DriveData *dd)
{
	dd->interval = panel_applet_gconf_get_int(PANEL_APPLET(dd->applet), "interval", NULL);
	dd->device_pixmap = panel_applet_gconf_get_int(PANEL_APPLET(dd->applet), "pixmap", NULL);
	dd->scale_applet = panel_applet_gconf_get_bool(PANEL_APPLET(dd->applet), "scale", NULL);
	dd->auto_eject = panel_applet_gconf_get_bool(PANEL_APPLET(dd->applet), "auto-eject", NULL);
	dd->mount_point = panel_applet_gconf_get_string(PANEL_APPLET(dd->applet), "mount-point", NULL);
	dd->autofs_friendly = panel_applet_gconf_get_bool(PANEL_APPLET(dd->applet), "autofs-friendly", NULL);
	dd->custom_icon_in = panel_applet_gconf_get_string(PANEL_APPLET(dd->applet), "custom-icon-mounted", NULL);
	dd->custom_icon_out = panel_applet_gconf_get_string(PANEL_APPLET(dd->applet), "custom-icon-unmounted", NULL);
	sync_mount_base(dd);
}

void
properties_save(DriveData *dd)
{
	panel_applet_gconf_set_int(PANEL_APPLET(dd->applet), "interval", dd->interval, NULL);
	panel_applet_gconf_set_int(PANEL_APPLET(dd->applet), "pixmap", dd->device_pixmap, NULL);
	panel_applet_gconf_set_bool(PANEL_APPLET(dd->applet), "scale", dd->scale_applet, NULL);
	panel_applet_gconf_set_bool(PANEL_APPLET(dd->applet), "auto-eject", dd->auto_eject, NULL);
	panel_applet_gconf_set_string(PANEL_APPLET(dd->applet), "mount-point", dd->mount_point, NULL);
	panel_applet_gconf_set_bool(PANEL_APPLET(dd->applet), "autofs-friendly", dd->autofs_friendly, NULL);
	panel_applet_gconf_set_string(PANEL_APPLET(dd->applet), "custom-icon-mounted", dd->custom_icon_in, NULL);
	panel_applet_gconf_set_string(PANEL_APPLET(dd->applet), "custom-icon-unmounted", dd->custom_icon_out, NULL);
}

void
properties_show(PanelApplet *applet, gpointer data)
{
	DriveData *dd = data;
	GtkWidget *dialog;
	GtkWidget *box;
	GtkWidget *frame;
	GtkWidget *hbox;
	GtkWidget *vbox;
	GtkWidget *fbox;
	GtkWidget *label;
	GtkWidget *menu;
	GtkWidget *item;
	ResponseWidgets *widgets;
	gint response;

	widgets = g_new0(ResponseWidgets, 1);
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

	widgets->mount_entry = gtk_entry_new_with_max_length(255);
	gtk_entry_set_text(GTK_ENTRY(widgets->mount_entry), dd->mount_point);
	gtk_box_pack_start(GTK_BOX(hbox), widgets->mount_entry , TRUE, TRUE, 0);
	gtk_widget_show(widgets->mount_entry);

	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

	label = gtk_label_new(_("Update in seconds:"));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_widget_show(label);

	widgets->update_spin = gtk_spin_button_new(GTK_ADJUSTMENT(gtk_adjustment_new(dd->interval, 1.0, 30.0, 1, 1, 1)), 1, 0);
	gtk_box_pack_start(GTK_BOX(hbox), widgets->update_spin, FALSE, FALSE, 0);
	gtk_spin_button_set_update_policy(GTK_SPIN_BUTTON(widgets->update_spin),GTK_UPDATE_ALWAYS);
	gtk_widget_show(widgets->update_spin);

	label = gtk_label_new(_("Icon:"));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_widget_show(label);

	widgets->omenu = gtk_option_menu_new ();
	gtk_box_pack_start(GTK_BOX(hbox), widgets->omenu, TRUE, TRUE, 0);
	gtk_widget_show(widgets->omenu);
	menu = gtk_menu_new();
	gtk_option_menu_set_menu(GTK_OPTION_MENU(widgets->omenu), menu);

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
		gtk_option_menu_set_history(GTK_OPTION_MENU(widgets->omenu), 5);
	else
		gtk_option_menu_set_history(GTK_OPTION_MENU(widgets->omenu), dd->device_pixmap);

	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(fbox), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

	widgets->icon_entry_in = gnome_icon_entry_new("drivemount-applet-id-in", _("Select icon for mounted"));
	gnome_icon_entry_set_filename(GNOME_ICON_ENTRY(widgets->icon_entry_in), dd->custom_icon_in);
	gtk_box_pack_end(GTK_BOX(hbox), widgets->icon_entry_in, FALSE, FALSE, 0);
	gtk_widget_show(widgets->icon_entry_in);

	label = gtk_label_new(_("Custom icon for mounted:"));
	gtk_box_pack_end(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_widget_show(label);

	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(fbox), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

	widgets->icon_entry_out = gnome_icon_entry_new("drivemount-applet-id-out", _("Select icon for unmounted"));
	gnome_icon_entry_set_filename(GNOME_ICON_ENTRY(widgets->icon_entry_out), dd->custom_icon_out);
	gtk_box_pack_end(GTK_BOX(hbox), widgets->icon_entry_out, FALSE, FALSE, 0);
	gtk_widget_show(widgets->icon_entry_out);

	label = gtk_label_new(_("Custom icon for not mounted:"));
	gtk_box_pack_end(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_widget_show(label);

	if(dd->device_pixmap < 0)
		gtk_widget_set_sensitive(fbox, TRUE);
	else
		gtk_widget_set_sensitive(fbox, FALSE);

	widgets->scale_toggle = gtk_check_button_new_with_label (_("Scale size to panel"));
	gtk_box_pack_start(GTK_BOX(vbox), widgets->scale_toggle, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widgets->scale_toggle), dd->scale_applet);
	gtk_widget_show(widgets->scale_toggle);

	widgets->eject_toggle = gtk_check_button_new_with_label (_("Eject on unmount"));
	gtk_box_pack_start(GTK_BOX(vbox), widgets->eject_toggle, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widgets->eject_toggle), dd->auto_eject);
	gtk_widget_show(widgets->eject_toggle);

	widgets->automount_toggle = gtk_check_button_new_with_label (_("Use automount friendly status test"));
	gtk_box_pack_start(GTK_BOX(vbox), widgets->automount_toggle, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widgets->automount_toggle), dd->autofs_friendly);
	gtk_widget_show(widgets->automount_toggle);
	gtk_widget_show_all(frame);

	widgets->dd = dd;
	g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(handle_response_cb), widgets);
	gtk_widget_show_all(dialog);
}

static void
handle_response_cb(GtkDialog *dialog, gint response, ResponseWidgets *widgets)
{
	gchar *temp;
	gint history;

	if(response == GTK_RESPONSE_OK)
	{
		temp = (gchar *)gtk_entry_get_text(GTK_ENTRY(widgets->mount_entry));
		if(widgets->dd->mount_point) g_free(widgets->dd->mount_point);
		if(temp) widgets->dd->mount_point = g_strdup(temp);
		else widgets->dd->mount_point = NULL;

		widgets->dd->interval = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widgets->update_spin));

		history = gtk_option_menu_get_history(GTK_OPTION_MENU(widgets->omenu));
		widgets->dd->device_pixmap = history < 5 ? history : -1;

		temp = gnome_icon_entry_get_filename(GNOME_ICON_ENTRY(widgets->icon_entry_in));
		if(widgets->dd->custom_icon_in) g_free(widgets->dd->custom_icon_in);
		if(temp) widgets->dd->custom_icon_in = temp;
		else widgets->dd->custom_icon_in = NULL;

		temp = gnome_icon_entry_get_filename(GNOME_ICON_ENTRY(widgets->icon_entry_out));
		if(widgets->dd->custom_icon_out) g_free(widgets->dd->custom_icon_out);
		if(temp) widgets->dd->custom_icon_out = temp;
		else widgets->dd->custom_icon_out = NULL;

		widgets->dd->scale_applet = GTK_TOGGLE_BUTTON(widgets->scale_toggle)->active;
		widgets->dd->auto_eject = GTK_TOGGLE_BUTTON(widgets->eject_toggle)->active;
		widgets->dd->autofs_friendly = GTK_TOGGLE_BUTTON(widgets->automount_toggle)->active;
		redraw_pixmap(widgets->dd);
		start_callback_update(widgets->dd);
		properties_save(widgets->dd);
	}
	gtk_widget_destroy(GTK_WIDGET(dialog));
	g_free(widgets);
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

static void
browse_icons_dialog_popup_handler(GnomeIconEntry *entry, GtkWidget *parent_dialog)
{
    GtkWidget *dialog;

    g_print("browse called\n");

    dialog = gnome_icon_entry_pick_dialog(entry);
    if(dialog)
    {
        g_print("has dialog\n");
        gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(parent_dialog));
    }
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
