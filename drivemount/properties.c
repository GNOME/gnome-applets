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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gconf/gconf-client.h>
#include <panel-applet.h>
#include <panel-applet-gconf.h>
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>

#include "drivemount.h"
#include "properties.h"

#define NEVER_SENSITIVE "never_sensitive"

/* same as in drivemount.c: number of icons to choose from */
static gint icon_list_count = 7;

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
static gchar *remove_level_from_path(const gchar *path);
static void sync_mount_base(DriveData *dd);

/* set sensitive and setup NEVER_SENSITIVE appropriately */
static void
hard_set_sensitive (GtkWidget *w, gboolean sensitivity)
{
	gtk_widget_set_sensitive (w, sensitivity);
	g_object_set_data (G_OBJECT (w), NEVER_SENSITIVE,
			   GINT_TO_POINTER ( ! sensitivity));
}


/* set sensitive, but always insensitive if NEVER_SENSITIVE is set */
static void
soft_set_sensitive (GtkWidget *w, gboolean sensitivity)
{
	if (g_object_get_data (G_OBJECT (w), NEVER_SENSITIVE))
		gtk_widget_set_sensitive (w, FALSE);
	else
		gtk_widget_set_sensitive (w, sensitivity);
}


static gboolean
key_writable (PanelApplet *applet, const char *key)
{
	gboolean writable;
	char *fullkey;
	static GConfClient *client = NULL;
	if (client == NULL)
		client = gconf_client_get_default ();

	fullkey = panel_applet_gconf_get_full_key (applet, key);

	writable = gconf_client_key_is_writable (client, fullkey, NULL);

	g_free (fullkey);

	return writable;
}


void
properties_load(DriveData *dd)
{
	GConfClient *client;
	gchar *key;
	GError *error = NULL;

	client = gconf_client_get_default ();
	key = panel_applet_get_preferences_key (PANEL_APPLET (dd->applet));

	if (gconf_client_dir_exists (client, key, NULL)) {
		dd->interval = panel_applet_gconf_get_int(PANEL_APPLET(dd->applet), "interval", &error);
		if (error) {
			g_print ("%s \n", error->message);
			g_error_free (error);
			error = NULL;
		}
		dd->interval = MAX (dd->interval, 1);
		dd->device_pixmap = panel_applet_gconf_get_int(PANEL_APPLET(dd->applet), "pixmap", NULL);
		dd->device_pixmap =MIN (dd->device_pixmap, icon_list_count-1);
		dd->scale_applet = panel_applet_gconf_get_bool(PANEL_APPLET(dd->applet), "scale", NULL);
		dd->auto_eject = panel_applet_gconf_get_bool(PANEL_APPLET(dd->applet), "auto_eject", NULL);
		dd->mount_point = panel_applet_gconf_get_string(PANEL_APPLET(dd->applet), "mount_point", NULL);
		if (!dd->mount_point)
			dd->mount_point = g_strdup ("/mnt/floppy");
		dd->autofs_friendly = panel_applet_gconf_get_bool(PANEL_APPLET(dd->applet), "autofs_friendly", NULL);
		dd->custom_icon_in = panel_applet_gconf_get_string(PANEL_APPLET(dd->applet), "custom_icon_mounted", NULL);
		dd->custom_icon_out = panel_applet_gconf_get_string(PANEL_APPLET(dd->applet), "custom_icon_unmounted", NULL);
	} else {
		dd->mount_point = g_strdup("/mnt/floppy");
		dd->interval = 10;
	}
	g_object_unref (G_OBJECT (client));
	g_free (key);
	sync_mount_base(dd);
}

static void
cb_mount_activate (GtkEditable *entry, gpointer data)
{
	DriveData *dd = data;
	gchar *text;
	
	text = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
	
	if (!text)
		return;
		
	if (dd->mount_point) {
		if (!g_ascii_strcasecmp (text, dd->mount_point)) {
			g_free (text);
			return;
		}
		
		g_free(dd->mount_point);
		dd->mount_point = g_strdup(text);
						
	}
	else
		dd->mount_point = g_strdup(text);
	
	sync_mount_base (dd);
	redraw_pixmap (dd);	
	panel_applet_gconf_set_string(PANEL_APPLET(dd->applet), "mount_point", 
				      dd->mount_point, NULL);
	g_free (text);
	
}

static gboolean
cb_mount_focus_out (GtkWidget *widget, GdkEventFocus *event, gpointer data)
{
	DriveData *dd = data;
	
	cb_mount_activate (GTK_EDITABLE (widget), dd);
	
	return FALSE;
	
}

static void
spin_changed (GtkWidget *button, GdkEventFocus *event, gpointer data)
{
	DriveData *dd = data;
	
	dd->interval = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON (button));
	
	start_callback_update(dd);
	panel_applet_gconf_set_int(PANEL_APPLET(dd->applet), "interval", 
				   dd->interval, NULL);


}

static void
scale_toggled (GtkToggleButton *button, gpointer data)
{
	DriveData *dd = data;
	
	dd->scale_applet = gtk_toggle_button_get_active (button);
	redraw_pixmap(dd);
	panel_applet_gconf_set_bool(PANEL_APPLET(dd->applet), "scale", 
				    dd->scale_applet, NULL);
	
}

static void
eject_toggled (GtkToggleButton *button, gpointer data)
{
	DriveData *dd = data;
	
	dd->auto_eject = gtk_toggle_button_get_active (button);
	panel_applet_gconf_set_bool(PANEL_APPLET(dd->applet), "auto_eject", 
				    dd->auto_eject, NULL);
	
}

static void
automount_toggled (GtkToggleButton *button, gpointer data)
{
	DriveData *dd = data;
	
	dd->autofs_friendly = gtk_toggle_button_get_active (button);
	panel_applet_gconf_set_bool(PANEL_APPLET(dd->applet), "autofs_friendly", 
				    dd->autofs_friendly, NULL);
	
}

static void
omenu_changed (GtkOptionMenu *menu, gpointer data)
{
	DriveData *dd = data;
	gint num;
	
	num = gtk_option_menu_get_history (menu);
	dd->device_pixmap = num < icon_list_count ? num : -1;
	panel_applet_gconf_set_int(PANEL_APPLET(dd->applet), "pixmap", 
				   dd->device_pixmap, NULL);
	redraw_pixmap(dd);
	
}
	
static void
mount_icon_changed (GnomeIconEntry *entry, gpointer data) 
{
	DriveData *dd = data;
	gchar *temp;

	temp = gnome_icon_entry_get_filename(entry);

	if (!temp)
		return;
		
	if(dd->custom_icon_in) 
		g_free(dd->custom_icon_in);
	dd->custom_icon_in = temp;
	
	dd->device_pixmap = -1;		
	redraw_pixmap(dd);
	panel_applet_gconf_set_string(PANEL_APPLET(dd->applet), "custom_icon_mounted",
				      dd->custom_icon_in, NULL);	
		
}

static void
unmount_icon_changed (GnomeIconEntry *entry, gpointer data) 
{
	DriveData *dd = data;
	gchar *temp;
	
	temp = gnome_icon_entry_get_filename(entry);
	
	if (!temp)
		return;
		
	if(dd->custom_icon_out) 
		g_free(dd->custom_icon_out);
	dd->custom_icon_out = temp;
	
	dd->device_pixmap = -1;		
	redraw_pixmap(dd);
	panel_applet_gconf_set_string(PANEL_APPLET(dd->applet), "custom_icon_unmounted",
				      dd->custom_icon_out, NULL);
			
}

static GtkWidget *
create_hig_catagory (GtkWidget *main_box, gchar *title)
{
	GtkWidget *vbox, *vbox2, *hbox;
	GtkWidget *label;
	gchar *tmp;
		
	vbox = gtk_vbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (main_box), vbox, FALSE, FALSE, 0);
	
	tmp = g_strdup_printf ("<b>%s</b>", title);
	label = gtk_label_new (NULL);
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_label_set_markup (GTK_LABEL (label), tmp);
	g_free (tmp);
	gtk_box_pack_start (GTK_BOX (vbox), label, TRUE, FALSE, 0);

	hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
	
	label = gtk_label_new ("    ");
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	
	vbox2 = gtk_vbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (hbox), vbox2, TRUE, TRUE, 0);

	return vbox2;
		
}

void
properties_show (BonoboUIComponent *uic,
		 DriveData         *dd,
		 const char        *verb)
{
	GtkWidget *dialog;
	GtkWidget *box;
	GtkWidget *frame;
	GtkWidget *hbox, *hbox2;
	GtkWidget *vbox, *vbox1;
	GtkWidget *fbox;
	GtkWidget *label;
	GtkWidget *menu;
	GtkWidget *item;
	GtkWidget *entry;
	ResponseWidgets *widgets;
	GtkSizeGroup *size;
	gint response;

	widgets = g_new0(ResponseWidgets, 1);
	dialog = gtk_dialog_new_with_buttons(_("Disk Mounter Preferences"), NULL, 
					     GTK_DIALOG_DESTROY_WITH_PARENT,
					     GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
					     GTK_STOCK_HELP, GTK_RESPONSE_HELP,
					     NULL);
	gtk_window_set_screen (GTK_WINDOW (dialog),
			       gtk_widget_get_screen (dd->applet));
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_CLOSE);
	gtk_container_set_border_width (GTK_CONTAINER (dialog), 5);
	gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
	gtk_dialog_set_has_separator (GTK_DIALOG (dialog), FALSE);

	box = GTK_DIALOG(dialog)->vbox;
	gtk_box_set_spacing (GTK_BOX (box), 2);
	
	size = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
	
	vbox = gtk_vbox_new(FALSE, 18);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);
	gtk_box_pack_start (GTK_BOX (box), vbox, FALSE, FALSE, 0);
	gtk_widget_show(vbox);
	
	vbox1 = create_hig_catagory (vbox, _("General"));
	
	hbox = gtk_hbox_new(FALSE, 12);
	gtk_box_pack_start(GTK_BOX(vbox1), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

	label = gtk_label_new_with_mnemonic(_("_Mount directory:"));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_widget_show(label);
	gtk_size_group_add_widget (size, label);

	widgets->mount_entry = gnome_file_entry_new ("drivemountentry", _("Select Mount Directory"));
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), widgets->mount_entry);
	gnome_file_entry_set_filename (GNOME_FILE_ENTRY  (widgets->mount_entry), 
						       dd->mount_point);
	gnome_file_entry_set_default_path (GNOME_FILE_ENTRY (widgets->mount_entry), "/mnt/");
	gnome_file_entry_set_directory_entry(GNOME_FILE_ENTRY (widgets->mount_entry), TRUE);
	entry = gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (widgets->mount_entry));
	gtk_entry_set_activates_default (GTK_ENTRY (entry), TRUE);
	gtk_box_pack_start(GTK_BOX(hbox), widgets->mount_entry , TRUE, TRUE, 0);
	gtk_widget_show(widgets->mount_entry);
	g_signal_connect (G_OBJECT (entry), "changed",
			  G_CALLBACK (cb_mount_activate), dd);

	if ( ! key_writable (PANEL_APPLET (dd->applet), "mount_point")) {
		hard_set_sensitive (label, FALSE);
		hard_set_sensitive (widgets->mount_entry, FALSE);
	}

	hbox = gtk_hbox_new(FALSE, 12);
	gtk_box_pack_start(GTK_BOX(vbox1), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

	label = gtk_label_new_with_mnemonic (_("_Update interval:"));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_widget_show(label);
	gtk_size_group_add_widget (size, label);

	hbox2 = gtk_hbox_new(FALSE, 6);
	gtk_box_pack_start(GTK_BOX(hbox), hbox2, FALSE, FALSE, 0);
	gtk_widget_show(hbox2);
	
	widgets->update_spin = gtk_spin_button_new(GTK_ADJUSTMENT(gtk_adjustment_new(dd->interval, 1.0, 300.0, 5, 1, 1)), 1, 0);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), widgets->update_spin);
	gtk_box_pack_start(GTK_BOX(hbox2), widgets->update_spin, FALSE, FALSE, 0);
	gtk_spin_button_set_update_policy(GTK_SPIN_BUTTON(widgets->update_spin),GTK_UPDATE_ALWAYS);
	gtk_widget_show(widgets->update_spin);
	g_signal_connect (G_OBJECT (widgets->update_spin), "focus_out_event",
				  G_CALLBACK (spin_changed), dd);

	if ( ! key_writable (PANEL_APPLET (dd->applet), "interval")) {
		hard_set_sensitive (label, FALSE);
		hard_set_sensitive (widgets->update_spin, FALSE);
	}
	
	label = gtk_label_new_with_mnemonic (_("seconds"));
	gtk_box_pack_start (GTK_BOX (hbox2), label, FALSE, FALSE, 0);
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_widget_show (label);

	if ( ! key_writable (PANEL_APPLET (dd->applet), "interval")) {
		hard_set_sensitive (label, FALSE);
	}

	hbox = gtk_hbox_new(FALSE, 12);
	gtk_box_pack_start(GTK_BOX(vbox1), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

	label = gtk_label_new_with_mnemonic(_("_Icon:"));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_widget_show(label);
	gtk_size_group_add_widget (size, label);

	widgets->omenu = gtk_option_menu_new ();
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), widgets->omenu);
	gtk_box_pack_start(GTK_BOX(hbox), widgets->omenu, TRUE, TRUE, 0);
	gtk_widget_show(widgets->omenu);
	menu = gtk_menu_new();
	gtk_option_menu_set_menu(GTK_OPTION_MENU(widgets->omenu), menu);

	if ( ! key_writable (PANEL_APPLET (dd->applet), "pixmap")) {
		hard_set_sensitive (label, FALSE);
		hard_set_sensitive (widgets->omenu, FALSE);
	} else {
		g_signal_connect (G_OBJECT (widgets->omenu), "changed",
				  G_CALLBACK (omenu_changed), dd);
	}
	
	/* This must be created before the menu items, so we can pass it to a callback */
	fbox = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(vbox1), fbox, FALSE, FALSE, 0);
	gtk_widget_show(fbox);

	item = gtk_menu_item_new_with_label(_("Floppy"));
	gtk_menu_append (GTK_MENU (menu), item);
	g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(set_widget_sensitivity_false_cb), fbox);
	item = gtk_menu_item_new_with_label(_("Cdrom"));
	gtk_menu_append (GTK_MENU (menu), item);
	g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(set_widget_sensitivity_false_cb), fbox);
	item = gtk_menu_item_new_with_label(_("Cd Recorder"));
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
	item = gtk_menu_item_new_with_label(_("USB Stick"));
	gtk_menu_append (GTK_MENU (menu), item);
	g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(set_widget_sensitivity_false_cb), fbox);
	item = gtk_menu_item_new_with_label(_("Custom"));
	gtk_menu_append (GTK_MENU (menu), item);
	g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(set_widget_sensitivity_true_cb), fbox);

	if (dd->device_pixmap == -1)
		gtk_option_menu_set_history(GTK_OPTION_MENU(widgets->omenu), icon_list_count);
	else
		gtk_option_menu_set_history(GTK_OPTION_MENU(widgets->omenu), dd->device_pixmap);

	hbox = gtk_hbox_new(FALSE, 12);
	gtk_box_pack_start(GTK_BOX(fbox), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);
	
	label = gtk_label_new_with_mnemonic(NULL);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_widget_show(label);
	gtk_size_group_add_widget (size, label);

	widgets->icon_entry_in = gnome_icon_entry_new("drivemount-applet-id-in", _("Select icon for mounted"));
	gnome_icon_entry_set_filename(GNOME_ICON_ENTRY(widgets->icon_entry_in), dd->custom_icon_in);
	gtk_box_pack_end(GTK_BOX(hbox), widgets->icon_entry_in, FALSE, FALSE, 0);
	gtk_widget_show(widgets->icon_entry_in);
	g_signal_connect (G_OBJECT (widgets->icon_entry_in), "changed",
			  G_CALLBACK (mount_icon_changed), dd);

	label = gtk_label_new_with_mnemonic(_("Moun_ted icon:"));
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), widgets->icon_entry_in);
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_widget_show(label);

	if ( ! key_writable (PANEL_APPLET (dd->applet), "custom_icon_mounted")) {
		hard_set_sensitive (label, FALSE);
		hard_set_sensitive (widgets->icon_entry_in, FALSE);
	}

	hbox = gtk_hbox_new(FALSE, 12);
	gtk_box_pack_start(GTK_BOX(fbox), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);
	
	label = gtk_label_new_with_mnemonic(NULL);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_widget_show(label);
	gtk_size_group_add_widget (size, label);

	widgets->icon_entry_out = gnome_icon_entry_new("drivemount-applet-id-out", _("Select icon for unmounted"));
	gnome_icon_entry_set_filename(GNOME_ICON_ENTRY(widgets->icon_entry_out), dd->custom_icon_out);
	gtk_box_pack_end(GTK_BOX(hbox), widgets->icon_entry_out, FALSE, FALSE, 0);
	gtk_widget_show(widgets->icon_entry_out);
	g_signal_connect (G_OBJECT (widgets->icon_entry_out), "changed",
			  G_CALLBACK (unmount_icon_changed), dd);

	label = gtk_label_new_with_mnemonic(_("Unmou_nted icon:"));
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), widgets->icon_entry_out);
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_widget_show(label);

	if ( ! key_writable (PANEL_APPLET (dd->applet), "custom_icon_unmounted")) {
		hard_set_sensitive (label, FALSE);
		hard_set_sensitive (widgets->icon_entry_out, FALSE);
	}

	if(dd->device_pixmap < 0)
		soft_set_sensitive(fbox, TRUE);
	else
		soft_set_sensitive(fbox, FALSE);
#if 0
	widgets->scale_toggle = gtk_check_button_new_with_mnemonic (_("_Scale size to panel"));
	gtk_box_pack_start(GTK_BOX(vbox1), widgets->scale_toggle, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widgets->scale_toggle), dd->scale_applet);
	gtk_widget_show(widgets->scale_toggle);
	g_signal_connect (G_OBJECT (widgets->scale_toggle), "toggled",
			  G_CALLBACK (scale_toggled), dd);
#endif
	widgets->eject_toggle = gtk_check_button_new_with_mnemonic (_("_Eject disk when unmounting"));
	gtk_box_pack_start(GTK_BOX(vbox1), widgets->eject_toggle, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widgets->eject_toggle), dd->auto_eject);
	gtk_widget_show(widgets->eject_toggle);
	g_signal_connect (G_OBJECT (widgets->eject_toggle), "toggled",
			  G_CALLBACK (eject_toggled), dd);

	if ( ! key_writable (PANEL_APPLET (dd->applet), "auto_eject"))
		hard_set_sensitive (widgets->eject_toggle, FALSE);

	widgets->automount_toggle = gtk_check_button_new_with_mnemonic (_("Use _automount friendly status test"));
	gtk_box_pack_start(GTK_BOX(vbox1), widgets->automount_toggle, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widgets->automount_toggle), dd->autofs_friendly);
	gtk_widget_show(widgets->automount_toggle);
	g_signal_connect (G_OBJECT (widgets->automount_toggle), "toggled",
			  G_CALLBACK (automount_toggled), dd);
	gtk_widget_show_all(vbox);

	if ( ! key_writable (PANEL_APPLET (dd->applet), "autofs_friendly"))
		hard_set_sensitive (widgets->automount_toggle, FALSE);

	widgets->dd = dd;
	g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(handle_response_cb), widgets);
	gtk_widget_show_all(dialog);
}

static void
help_cb (GtkDialog *dialog)
{
	GError *error = NULL;
	static GnomeProgram *applet_program = NULL;
	
	if (!applet_program) {
		int argc = 1;
		char *argv[2] = { "drivemount" };
		applet_program = gnome_program_init ("drivemount", VERSION,
						      LIBGNOME_MODULE, argc, argv,
     						      GNOME_PROGRAM_STANDARD_PROPERTIES, NULL);
	}

	egg_help_display_desktop_on_screen (
			applet_program, "drivemount", "drivemount", "drivemountapplet-prefs",
			gtk_widget_get_screen (GTK_WIDGET (dialog)),
			&error);

	if (error) {
		GtkWidget *error_dialog;
		char *msg;

		msg = g_strdup_printf (_("There was an error displaying help: %s"),
				       error->message);
		error_dialog = hig_dialog_new (NULL /* parent */,
					       GTK_DIALOG_DESTROY_WITH_PARENT,
					       GTK_MESSAGE_ERROR,
					       GTK_BUTTONS_OK,
					       _("Error displaying help"),
					       msg);
		g_free (msg);

		g_signal_connect (error_dialog, "response",
				  G_CALLBACK (gtk_widget_destroy),
				  NULL);

		gtk_window_set_resizable (GTK_WINDOW (error_dialog), FALSE);
		gtk_window_set_screen (GTK_WINDOW (error_dialog),
				       gtk_widget_get_screen (GTK_WIDGET (dialog)));
		gtk_widget_show (error_dialog);
		g_error_free (error);
	}
}

static void
handle_response_cb(GtkDialog *dialog, gint response, ResponseWidgets *widgets)
{
	if (response == GTK_RESPONSE_HELP) {
		help_cb (dialog);
		return;
	}

	gtk_widget_destroy(GTK_WIDGET(dialog));
	g_free(widgets);
}

static void
set_widget_sensitivity_false_cb(GtkWidget *widget, GtkWidget *target)
{
	soft_set_sensitive(target, FALSE);
}

static void
set_widget_sensitivity_true_cb(GtkWidget *widget, GtkWidget *target)
{
	soft_set_sensitive(target, TRUE);
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
