/*  panel-menu-links.c
 *
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
#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>
#include <panel-applet.h>

#include <libgnomevfs/gnome-vfs-types.h>
#include <libgnomevfs/gnome-vfs-init.h>
#include <libgnomevfs/gnome-vfs-directory.h>
#include <libgnomevfs/gnome-vfs-mime-utils.h>
#include <libgnomevfs/gnome-vfs-uri.h>
#include <libgnomevfs/gnome-vfs-handle.h>
#include <libgnomevfs/gnome-vfs-file-info.h>
#include <libgnomevfs/gnome-vfs-result.h>
#include <libgnomevfs/gnome-vfs-utils.h>

#include "panel-menu.h"
#include "panel-menu-desktop-item.h"
#include "panel-menu-common.h"
#include "panel-menu-options.h"
#include "panel-menu-links.h"

typedef struct _PanelMenuLinks
{
    GtkWidget *checkitem;
    GtkWidget *links;
    GtkWidget *menu;
    GtkWidget *popup;
    gchar *name;
    GList *items_list;
    GList *links_list;
}PanelMenuLinks;

static void set_visibility(GtkCheckMenuItem *checkitem, GtkWidget *target);
static gint panel_menu_links_remove_cb(GtkWidget *widget, GdkEventKey *event, PanelMenuLinks *links);
static void rename_links_cb(GtkWidget *widget, PanelMenuEntry *entry);

PanelMenuEntry *
panel_menu_links_new(PanelMenu *parent, gchar *name)
{
    PanelMenuEntry *entry;
    PanelMenuLinks *links;
    GtkWidget *tearoff;
    GtkWidget *item;
    GtkWidget *image;

    entry = g_new0(PanelMenuEntry, 1);
    entry->type = PANEL_MENU_TYPE_LINKS;
    entry->parent = parent;
    links = g_new0(PanelMenuLinks, 1);
    entry->data = (gpointer)links;
    links->name = g_strdup(name);
    links->links = gtk_menu_item_new_with_label("");
    panel_menu_common_widget_dnd_init(entry);
    gtk_widget_show(links->links);
    links->checkitem = gtk_check_menu_item_new_with_label("");
    panel_menu_links_set_name(entry, name);
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(links->checkitem), TRUE);
    g_signal_connect(G_OBJECT(links->checkitem), "toggled", G_CALLBACK(panel_menu_common_set_visibility), links->links);
    links->menu = gtk_menu_new();
    tearoff = gtk_tearoff_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(links->menu), tearoff);
    gtk_widget_show(tearoff);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(links->links), links->menu);
    g_signal_connect(G_OBJECT(links->menu), "key_press_event", G_CALLBACK(panel_menu_links_remove_cb), links);
    links->popup = panel_menu_common_construct_entry_popup(entry);
    item = gtk_image_menu_item_new_with_label(_("Rename..."));
    image = gtk_image_new_from_stock(GTK_STOCK_CONVERT, GTK_ICON_SIZE_MENU);
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(item), image);
    gtk_widget_show(image);
    gtk_menu_shell_append(GTK_MENU_SHELL(links->popup), item);
    g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(rename_links_cb), entry);
    gtk_widget_show(item);
    return(entry);
}

void
panel_menu_links_set_name(PanelMenuEntry *entry, gchar *name)
{
    PanelMenuLinks *links;
    gchar *show_label;

    g_return_if_fail(entry != NULL);
    g_return_if_fail(name != NULL);
    g_return_if_fail(entry->type == PANEL_MENU_TYPE_LINKS);

    links = (PanelMenuLinks *)entry->data;
    if(links->name) g_free(links->name);
    links->name = name ? g_strdup(name) : g_strdup("");
    gtk_label_set_text(GTK_LABEL(GTK_BIN(links->links)->child), name);
    show_label = g_strconcat("Show ", links->name, NULL);
    gtk_label_set_text(GTK_LABEL(GTK_BIN(links->checkitem)->child), show_label);
    g_free(show_label);
}

void
panel_menu_links_destroy(PanelMenuEntry *entry)
{
    PanelMenuLinks *links;
    GList *cur;

    g_return_if_fail(entry != NULL);
    g_return_if_fail(entry->type == PANEL_MENU_TYPE_LINKS);

    links = (PanelMenuLinks *)entry->data;
    if(links->name) g_free(links->name);
    if(links->items_list) g_list_free(links->items_list);
    for(cur = links->links_list; cur; cur = cur->next)
    {
        if(cur->data) g_free(cur->data);
    }
    gtk_widget_destroy(links->checkitem);
    gtk_widget_destroy(links->links);
    g_free(links);
}

GtkWidget *
panel_menu_links_get_widget(PanelMenuEntry *entry)
{
    PanelMenuLinks *links;

    g_return_if_fail(entry != NULL);
    g_return_if_fail(entry->type == PANEL_MENU_TYPE_LINKS);

    links = (PanelMenuLinks *)entry->data;
    return(links->links);
}

GtkWidget *
panel_menu_links_get_checkitem(PanelMenuEntry *entry)
{
    PanelMenuLinks *links;

    g_return_val_if_fail(entry != NULL, NULL);
    g_return_val_if_fail(entry->type == PANEL_MENU_TYPE_LINKS, NULL);

    links = (PanelMenuLinks *)entry->data;
    return(links->checkitem);
}

GtkWidget *
panel_menu_links_get_popup(PanelMenuEntry *entry)
{
    PanelMenuLinks *links;

    g_return_val_if_fail(entry != NULL, NULL);
    g_return_val_if_fail(entry->type == PANEL_MENU_TYPE_LINKS, NULL);

    links = (PanelMenuLinks *)entry->data;
    return(links->popup);
}

gchar *
panel_menu_links_dump_xml(PanelMenuEntry *entry)
{
    PanelMenuLinks *links;
    GString *string;
    GList *cur;
    gchar *str;
    gboolean visible;

    g_return_val_if_fail(entry != NULL, NULL);
    g_return_val_if_fail(entry->type == PANEL_MENU_TYPE_LINKS, NULL);

    links = (PanelMenuLinks *)entry->data;
    visible = GTK_CHECK_MENU_ITEM(links->checkitem)->active;

    string = g_string_new("    <links-item>\n"
                          "        <name>");
    g_string_append(string, links->name);
    g_string_append(string, "</name>\n");
    for(cur = links->links_list; cur; cur = cur->next)
    {
        g_string_append(string, "        <link>");
        g_string_append(string, (gchar *)cur->data);
        g_string_append(string, "</link>\n");
    }
    g_string_append(string, "        <visible>");
    g_string_append(string, visible ? "true" : "false");
    g_string_append(string, "</visible>\n");
    g_string_append(string, "    </links-item>\n");
    str = string->str;
    g_string_free(string, FALSE);
    return(str);
}

gboolean
panel_menu_links_accept_drop(PanelMenuEntry *entry, GnomeVFSURI *uri)
{
    gchar *fileuri;
    gboolean retval = FALSE;

    g_return_val_if_fail(entry != NULL, FALSE);
    g_return_val_if_fail(entry->type == PANEL_MENU_TYPE_LINKS, FALSE);

    fileuri = gnome_vfs_uri_to_string(uri, GNOME_VFS_URI_HIDE_NONE);
    g_print("(links) uri is %s\n", fileuri);
    retval = panel_menu_links_append_item(entry, fileuri);
    if(retval) panel_menu_common_set_changed(entry->parent);
    g_free(fileuri);
    return(retval);
}

gboolean
panel_menu_links_append_item(PanelMenuEntry *entry, gchar *uri)
{
    PanelMenuLinks *links;
    PanelMenuDesktopItem *item;
    GnomeVFSFileInfo finfo;
    gchar *icon;
    GtkWidget *menuitem;
    gboolean retval = FALSE;

    g_return_val_if_fail(entry != NULL, FALSE);
    g_return_val_if_fail(entry->type == PANEL_MENU_TYPE_LINKS, FALSE);

    links = (PanelMenuLinks *)entry->data;

    if((!strncmp(uri, "applications:", strlen("applications:")) || !strncmp(uri, "file:", strlen("file:"))) && strstr(uri, ".desktop"))
    {
        if((menuitem = panel_menu_common_menuitem_from_path(uri, GTK_MENU_SHELL(links->menu), FALSE)))
            retval = TRUE;
    }
    else if(!strncmp(uri, "file:", strlen("file:")))
    {
        if(gnome_vfs_get_file_info(uri, &finfo, GNOME_VFS_FILE_INFO_FOLLOW_LINKS) == GNOME_VFS_OK)
        {
            if(finfo.type == GNOME_VFS_FILE_TYPE_DIRECTORY)
            {
                menuitem = gtk_image_menu_item_new_with_label(finfo.name);
                panel_menu_common_apps_menuitem_dnd_init(menuitem);
                panel_menu_common_set_icon_scaled_from_file(GTK_MENU_ITEM(menuitem), DATADIR "/pixmaps/gnome-folder.png");
                gtk_menu_shell_append(GTK_MENU_SHELL(links->menu), menuitem);
                g_object_set_data(G_OBJECT(menuitem), "exec-string", g_strdup_printf("nautilus %s", uri));
                g_object_set_data(G_OBJECT(menuitem), "uri-path", g_strdup(uri));
                g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(panel_menu_common_activate_apps_menuitem), NULL);
                g_signal_connect(G_OBJECT(menuitem), "destroy", G_CALLBACK(panel_menu_common_destroy_apps_menuitem), NULL);
                gtk_widget_show(menuitem);
                retval = TRUE;
            }
        }
    }
    if(retval)
    {
        links->items_list = g_list_append(links->items_list, menuitem);
        links->links_list = g_list_append(links->links_list, g_strdup(uri));
    }
}

static gint
panel_menu_links_remove_cb(GtkWidget *widget, GdkEventKey *event, PanelMenuLinks *links)
{
    GtkWidget *item;
    GList *cur;
    gchar *string;
    gchar *temp;
    gint counter = 0;
    gboolean retval = FALSE;

    if(event->keyval == GDK_Delete)
    {
        item = GTK_MENU_SHELL(widget)->active_menu_item;
        string = g_object_get_data(G_OBJECT(item), "uri-path");
        if(!string) return(FALSE);
        for(cur = links->items_list; cur; cur = cur->next, counter++)
        {
            if(item == GTK_WIDGET(cur->data))
            {
                links->items_list = g_list_remove(links->items_list, cur->data);
                temp = (gchar *)g_list_nth_data(links->links_list, counter);
                g_print("removing %s from the links list\n", temp);
                links->links_list = g_list_remove(links->links_list, temp);
                gtk_widget_destroy(item);
                g_free(temp);
                retval = TRUE;
                break;
            }
        }
    }
    return(retval);
}

void
panel_menu_links_new_with_dialog(PanelMenu *panel_menu)
{
    GtkWidget *dialog;
    GtkWidget *name_entry;
    gint response;
    gchar *name;
    PanelMenuEntry *entry;

    dialog = panel_menu_common_single_entry_dialog_new(_("Create links item..."), _("Name:"), _("Favorites"), &name_entry);
    gtk_widget_show(dialog);
    gtk_widget_grab_focus(name_entry);

    response = gtk_dialog_run(GTK_DIALOG(dialog));
    if(response == GTK_RESPONSE_OK)
    {
        name = (gchar *)gtk_entry_get_text(GTK_ENTRY(name_entry));
        entry = panel_menu_links_new(panel_menu, name);
        panel_menu->entries = g_list_append(panel_menu->entries, (gpointer)entry);
        panel_menu_options_append_option(panel_menu_common_find_options(panel_menu), panel_menu_links_get_checkitem(entry));
        gtk_menu_shell_append(GTK_MENU_SHELL(panel_menu->menubar), panel_menu_links_get_widget(entry));
        panel_menu_common_set_changed(panel_menu);
    }
    gtk_widget_destroy(dialog);
}

static void
rename_links_cb(GtkWidget *widget, PanelMenuEntry *entry)
{
    PanelMenuLinks *links;
    GtkWidget *dialog;
    GtkWidget *name_entry;
    gint response;
    gchar *name;

    g_return_if_fail(entry != NULL);
    g_return_if_fail(entry->type == PANEL_MENU_TYPE_LINKS);

    links = (PanelMenuLinks *)entry->data;

    dialog = panel_menu_common_single_entry_dialog_new(_("Rename links item..."), _("Name:"), links->name, &name_entry);
    gtk_widget_show(dialog);
    gtk_widget_grab_focus(name_entry);

    response = gtk_dialog_run(GTK_DIALOG(dialog));
    if(response == GTK_RESPONSE_OK)
    {
        name = (gchar *)gtk_entry_get_text(GTK_ENTRY(name_entry));
        if(strcmp(name, links->name))
        {
            panel_menu_links_set_name(entry, name);
            panel_menu_common_set_changed(entry->parent);
        }
    }
    gtk_widget_destroy(dialog);
}
