/*  panel-menu-properties.c
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <libbonobo.h>
#include <libbonoboui.h>
#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>
#include <panel-applet.h>

#include "panel-menu.h"
#include "panel-menu-common.h"
#include "panel-menu-options.h"
#include "panel-menu-path.h"
#include "panel-menu-properties.h"

typedef struct _ResponseProperties
{
	GtkWidget *show_icon_handle;
	GtkWidget *icon_handle_image;
	GtkWidget *icon_size;
    GtkWidget *auto_popup_menus;
    GtkWidget *auto_popup_menus_timeout;
	GtkWidget *auto_directory_update;
	GtkWidget *auto_directory_update_timeout;
	GtkWidget *auto_save_config;
    PanelMenu *panel_menu;
}ResponseProperties;

static void set_widget_sensitivity(GtkWidget *checkitem, GtkWidget *target);
static void handle_response_cb(GtkDialog *dialog, gint response, ResponseProperties *properties);

void
applet_properties_cb(BonoboUIComponent *uic, PanelMenu *panel_menu, const gchar *verbname)
{
	ResponseProperties *properties;
	GtkWidget *dialog;
	GtkWidget *box;
	GtkWidget *frame;
	GtkWidget *hbox;
	GtkWidget *vbox;
	GtkWidget *fbox;
	GtkWidget *label;
	GtkWidget *menu;
	GtkWidget *item;
	gint response;

    properties = g_new0(ResponseProperties, 1);
    properties->panel_menu = panel_menu;

    dialog = gtk_dialog_new_with_buttons(_("PanelMenu Properties"),
										 NULL, GTK_DIALOG_MODAL,
										 GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
										 GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);

    box = GTK_DIALOG(dialog)->vbox;
	frame = gtk_frame_new(NULL);
	gtk_container_set_border_width(GTK_CONTAINER(frame), GNOME_PAD_SMALL);
    gtk_box_pack_start(GTK_BOX(box), frame, TRUE, TRUE, 0);
    gtk_widget_show(frame);

	vbox = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), GNOME_PAD_SMALL);
	gtk_container_add(GTK_CONTAINER(frame), vbox);
	gtk_widget_show(vbox);

    /* Options Item */
	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

	properties->show_icon_handle = gtk_check_button_new_with_label(_("Show the icon handle ?"));
	gtk_box_pack_start(GTK_BOX(hbox), properties->show_icon_handle, FALSE, FALSE, 0);
	gtk_widget_show(properties->show_icon_handle);

	fbox = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(hbox), fbox, TRUE, TRUE, 0);
	gtk_widget_set_sensitive(fbox, panel_menu->show_icon_handle);
	gtk_widget_show(fbox);

    g_signal_connect(G_OBJECT(properties->show_icon_handle), "toggled", G_CALLBACK(set_widget_sensitivity), fbox);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(properties->show_icon_handle), panel_menu->show_icon_handle);

	properties->icon_handle_image = gnome_icon_entry_new("panel-menu-applet-id", _("Select icon for handle"));
	gnome_icon_entry_set_filename(GNOME_ICON_ENTRY(properties->icon_handle_image), panel_menu->icon_handle_image);
	gtk_box_pack_end(GTK_BOX(fbox), properties->icon_handle_image, FALSE, FALSE, 0);
	gtk_widget_show(properties->icon_handle_image);

    /* General */
	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

	properties->auto_popup_menus = gtk_check_button_new_with_label(_("Popup items on mouse-over ?"));
	gtk_box_pack_start(GTK_BOX(hbox), properties->auto_popup_menus, FALSE, FALSE, 0);
	gtk_widget_show(properties->auto_popup_menus);

	fbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(hbox), fbox, TRUE, TRUE, 0);
	gtk_widget_set_sensitive(fbox, panel_menu->auto_popup_menus);
	gtk_widget_show(fbox);

    g_signal_connect(G_OBJECT(properties->auto_popup_menus), "toggled", G_CALLBACK(set_widget_sensitivity), fbox);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(properties->auto_popup_menus), panel_menu->auto_popup_menus);

    properties->auto_popup_menus_timeout = gtk_spin_button_new(GTK_ADJUSTMENT(gtk_adjustment_new(panel_menu->auto_popup_menus_timeout, 1.0, 1000.0, 1, 1, 1)), 1, 0);
    gtk_box_pack_end(GTK_BOX(fbox), properties->auto_popup_menus_timeout, FALSE, FALSE, 0);
    gtk_spin_button_set_update_policy(GTK_SPIN_BUTTON(properties->auto_popup_menus_timeout), GTK_UPDATE_ALWAYS);
    gtk_widget_show(properties->auto_popup_menus_timeout);

	label = gtk_label_new(_("Delay in milli-seconds:"));
	gtk_box_pack_end(GTK_BOX(fbox), label, FALSE, FALSE, 0);
	gtk_widget_show(label);

    /* Directory Item */
	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

	properties->auto_directory_update = gtk_check_button_new_with_label(_("Auto update directory menus ?"));
	gtk_box_pack_start(GTK_BOX(hbox), properties->auto_directory_update, FALSE, FALSE, 0);
	gtk_widget_show(properties->auto_directory_update);

	fbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(hbox), fbox, TRUE, TRUE, 0);
	gtk_widget_set_sensitive(fbox, panel_menu->auto_directory_update);
	gtk_widget_show(fbox);

    g_signal_connect(G_OBJECT(properties->auto_directory_update), "toggled", G_CALLBACK(set_widget_sensitivity), fbox);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(properties->auto_directory_update), panel_menu->auto_directory_update);

    properties->auto_directory_update_timeout = gtk_spin_button_new(GTK_ADJUSTMENT(gtk_adjustment_new(panel_menu->auto_directory_update_timeout, 1.0, 120.0, 1, 1, 1)), 1, 0);
    gtk_box_pack_end(GTK_BOX(fbox), properties->auto_directory_update_timeout, FALSE, FALSE, 0);
    gtk_spin_button_set_update_policy(GTK_SPIN_BUTTON(properties->auto_directory_update_timeout), GTK_UPDATE_ALWAYS);
    gtk_widget_show(properties->auto_directory_update_timeout);

	label = gtk_label_new(_("Update in seconds:"));
	gtk_box_pack_end(GTK_BOX(fbox), label, FALSE, FALSE, 0);
	gtk_widget_show(label);

    /* General */
	properties->auto_save_config = gtk_check_button_new_with_label(_("Auto-save configuration when changes occur ?"));
	gtk_box_pack_start(GTK_BOX(vbox), properties->auto_save_config, FALSE, FALSE, 0);
	gtk_widget_show(properties->auto_save_config);

    g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(handle_response_cb), properties);
    gtk_widget_show_all(dialog);
}

static void
set_widget_sensitivity(GtkWidget *checkitem, GtkWidget *target)
{
	gtk_widget_set_sensitive(target, GTK_TOGGLE_BUTTON(checkitem)->active);
}

static void
handle_response_cb(GtkDialog *dialog, gint response, ResponseProperties *properties)
{
    PanelMenuEntry *options;
    PanelMenuEntry *entry;
    gchar *temp;
    GList *cur;

    if(response == GTK_RESPONSE_OK)
    {
        options = panel_menu_common_find_options(properties->panel_menu);
        if(properties->panel_menu->show_icon_handle != GTK_TOGGLE_BUTTON(properties->show_icon_handle)->active)
        {
            properties->panel_menu->show_icon_handle = GTK_TOGGLE_BUTTON(properties->show_icon_handle)->active;
            panel_menu_common_set_changed(properties->panel_menu);
            if(GTK_TOGGLE_BUTTON(properties->show_icon_handle)->active)
            {
                gtk_widget_show(panel_menu_options_get_widget(options));
            }
            else
            {
                gtk_widget_hide(panel_menu_options_get_widget(options));
            }
        }
        temp = gnome_icon_entry_get_filename(GNOME_ICON_ENTRY(properties->icon_handle_image));
        if(temp && strcmp(temp, properties->panel_menu->icon_handle_image))
        {
            panel_menu_options_set_icon(options, temp, properties->panel_menu->size - 8);
            panel_menu_common_set_changed(properties->panel_menu);
        }
        if(properties->panel_menu->auto_directory_update != GTK_TOGGLE_BUTTON(properties->auto_directory_update)->active)
        {
            properties->panel_menu->auto_directory_update = GTK_TOGGLE_BUTTON(properties->auto_directory_update)->active;
            properties->panel_menu->auto_directory_update_timeout = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(properties->auto_directory_update_timeout));
            for(cur = properties->panel_menu->entries; cur; cur = cur->next)
            {
                entry = (PanelMenuEntry *)cur->data;
                if(entry->type == PANEL_MENU_TYPE_DIRECTORY)
                    panel_menu_directory_start_timeout(entry);
            }
        }
        properties->panel_menu->auto_save_config = GTK_TOGGLE_BUTTON(properties->auto_save_config)->active;
        if(properties->panel_menu->auto_save_config && properties->panel_menu->changed)
        {
            panel_menu_config_save_xml(properties->panel_menu);
        }
    }
    panel_menu_config_save_prefs(properties->panel_menu);
    gtk_widget_destroy(GTK_WIDGET(dialog));
    g_free(properties);
}
