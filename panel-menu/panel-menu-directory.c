/*  panel-menu-directory.c
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
#include "panel-menu-common.h"
#include "panel-menu-directory.h"

typedef struct _PanelMenuDocuments
{
    GtkWidget *checkitem;
    GtkWidget *directory;
    GtkWidget *regenitem;
    GtkWidget *menu;
    GtkWidget *popup;
    gchar *name;
    gchar *path;
    gint level;
    time_t mtime;
    gint timeout_id;
}PanelMenuDirectory;

static gint check_update_directory(PanelMenuEntry *entry);
static time_t get_directory_mtime(gchar *uri);
static void regenerate_menus_cb(GtkWidget *menuitem, PanelMenuEntry *entry);
static void panel_menu_directory_load(const gchar *uri, GtkMenuShell *parent, gint level);
static void directory_load_cb(GnomeVFSAsyncHandle *handle, GnomeVFSResult result, GList *list, guint entries_read, GtkMenuShell *parent);
static void set_visibility(GtkCheckMenuItem *checkitem, GtkWidget *target);
static void change_directory_cb(GtkWidget *widget, PanelMenuEntry *entry);
static GtkWidget *panel_menu_directory_edit_dialog_new(gchar *title, GtkWidget **nentry, GtkWidget **pentry, GtkWidget **spin);

PanelMenuEntry *
panel_menu_directory_new(PanelMenu *parent, gchar *name, gchar *path, gint level)
{
    PanelMenuEntry *entry;
    PanelMenuDirectory *directory;
    GtkWidget *tearoff;
    GtkWidget *image;
    GtkWidget *sep;
    GtkWidget *item;

    entry = g_new0(PanelMenuEntry, 1);
    entry->type = PANEL_MENU_TYPE_DIRECTORY;
    entry->parent = parent;
    directory = g_new0(PanelMenuDirectory, 1);
    entry->data = (gpointer)directory;
    directory->level = level;
    directory->directory = gtk_menu_item_new_with_label("");
    panel_menu_common_widget_dnd_init(entry);
    gtk_widget_show(directory->directory);
    directory->checkitem = gtk_check_menu_item_new_with_label("");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(directory->checkitem), TRUE);
    g_signal_connect(G_OBJECT(directory->checkitem), "toggled", G_CALLBACK(panel_menu_common_set_visibility), directory->directory);
    directory->menu = gtk_menu_new();
    tearoff = gtk_tearoff_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(directory->menu), tearoff);
    gtk_widget_show(tearoff);
    directory->regenitem = gtk_image_menu_item_new_with_label(_("Regenerate Menus"));
    image = gtk_image_new_from_stock("gtk-refresh", GTK_ICON_SIZE_MENU);
    gtk_widget_show(image);
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(directory->regenitem), image);
    gtk_menu_shell_append(GTK_MENU_SHELL(directory->menu), directory->regenitem);
    g_signal_connect(G_OBJECT(directory->regenitem), "activate", G_CALLBACK(regenerate_menus_cb), entry);
    gtk_widget_show(directory->regenitem);
    sep = gtk_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(directory->menu), sep);
    gtk_widget_show(sep);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(directory->directory), directory->menu);
    directory->popup = panel_menu_common_construct_entry_popup(entry);
    item = gtk_image_menu_item_new_with_label(_("Change Settings..."));
    image = gtk_image_new_from_stock(GTK_STOCK_CONVERT, GTK_ICON_SIZE_MENU);
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(item), image);
    gtk_widget_show(image);
    gtk_menu_shell_append(GTK_MENU_SHELL(directory->popup), item);
    g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(change_directory_cb), entry);
    gtk_widget_show(item);
    panel_menu_directory_set_name(entry, name);
    panel_menu_directory_set_path(entry, path);
    return(entry);
}

void
panel_menu_directory_set_name(PanelMenuEntry *entry, gchar *name)
{
    PanelMenuDirectory *directory;
    gchar *show_label;

    g_return_if_fail(entry != NULL);
    g_return_if_fail(name != NULL);
    g_return_if_fail(entry->type == PANEL_MENU_TYPE_DIRECTORY);

    directory = (PanelMenuDirectory *)entry->data;
    if(directory->name) g_free(directory->name);
    directory->name = name ? g_strdup(name) : g_strdup("");
    gtk_label_set_text(GTK_LABEL(GTK_BIN(directory->directory)->child), name);
    show_label = g_strconcat("Show ", directory->name, NULL);
    gtk_label_set_text(GTK_LABEL(GTK_BIN(directory->checkitem)->child), show_label);
    g_free(show_label);
}

void
panel_menu_directory_set_path(PanelMenuEntry *entry, gchar *path)
{
    PanelMenuDirectory *directory;
    gchar *full_path = NULL;
    gchar *label;
    gchar *show_label;

    g_return_if_fail(entry != NULL);
    g_return_if_fail(entry->type == PANEL_MENU_TYPE_DIRECTORY);

    directory = (PanelMenuDirectory *)entry->data;
    if(directory->path) g_free(directory->path);
    directory->path = g_strdup(path);
    regenerate_menus_cb(NULL, entry);
    directory->mtime = get_directory_mtime(directory->path);
    panel_menu_directory_start_timeout(entry);
}

void
panel_menu_directory_start_timeout(PanelMenuEntry *entry)
{
    PanelMenuDirectory *directory;

    g_return_if_fail(entry != NULL);
    g_return_if_fail(entry->type == PANEL_MENU_TYPE_DIRECTORY);

    directory = (PanelMenuDirectory *)entry->data;
    panel_menu_directory_stop_timeout(entry);
    if(entry->parent->auto_directory_update && entry->parent->auto_directory_update_timeout)
    {
        directory->timeout_id = gtk_timeout_add(entry->parent->auto_directory_update_timeout * 1000,
                                               (GtkFunction) check_update_directory, entry);
    }
}

void
panel_menu_directory_stop_timeout(PanelMenuEntry *entry)
{
    PanelMenuDirectory *directory;

    g_return_if_fail(entry != NULL);
    g_return_if_fail(entry->type == PANEL_MENU_TYPE_DIRECTORY);

    directory = (PanelMenuDirectory *)entry->data;
    if(directory->timeout_id) gtk_timeout_remove(directory->timeout_id);
    directory->timeout_id = 0;
}

static gint
check_update_directory(PanelMenuEntry *entry)
{
    PanelMenuDirectory *directory;
    time_t time;

    g_return_if_fail(entry != NULL);
    g_return_if_fail(entry->type == PANEL_MENU_TYPE_DIRECTORY);

    directory = (PanelMenuDirectory *)entry->data;
    if(entry->parent->auto_directory_update)
    {
        time = get_directory_mtime(directory->path);
        if(time > directory->mtime)
        {
            g_print("directory modified, updating menu contents.\n");
            regenerate_menus_cb(NULL, entry);
            directory->mtime = time;
        }
    }
    else
    {
        panel_menu_directory_stop_timeout(entry);
        return(FALSE);
    }
    return(TRUE);
}

static time_t
get_directory_mtime(gchar *uri)
{
    struct stat s;
    time_t mtime = 0;
    if((stat(uri, &s) == 0) && S_ISDIR(s.st_mode))
    {
        mtime = s.st_mtime;
    }
    return(mtime);
}

static void
regenerate_menus_cb(GtkWidget *menuitem, PanelMenuEntry *entry)
{
    PanelMenuDirectory *directory;
    GList *list;
    GList *cur;

    g_return_if_fail(entry != NULL);
    g_return_if_fail(entry->type == PANEL_MENU_TYPE_DIRECTORY);

    directory = (PanelMenuDirectory *)entry->data;
    list = g_list_copy(GTK_MENU_SHELL(directory->menu)->children);
    for(cur = list; cur; cur = cur->next)
    {
        if(g_object_get_data(G_OBJECT(cur->data), "uri-path"))
            gtk_widget_destroy(GTK_WIDGET(cur->data));
    }
    g_list_free(list);
    panel_menu_directory_stop_timeout(entry);
    panel_menu_directory_load(directory->path, GTK_MENU_SHELL(directory->menu), directory->level);
    panel_menu_directory_start_timeout(entry);
}

void
panel_menu_directory_destroy(PanelMenuEntry *entry)
{
    PanelMenuDirectory *directory;

    g_return_if_fail(entry != NULL);
    g_return_if_fail(entry->type == PANEL_MENU_TYPE_DIRECTORY);

    directory = (PanelMenuDirectory *)entry->data;
    if(directory->name) g_free(directory->name);
    if(directory->path) g_free(directory->path);
    gtk_widget_destroy(directory->checkitem);
    gtk_widget_destroy(directory->directory);
    g_free(directory);
}

GtkWidget *
panel_menu_directory_get_widget(PanelMenuEntry *entry)
{
    PanelMenuDirectory *directory;

    g_return_if_fail(entry != NULL);
    g_return_if_fail(entry->type == PANEL_MENU_TYPE_DIRECTORY);

    directory = (PanelMenuDirectory *)entry->data;
    return(directory->directory);
}

GtkWidget *
panel_menu_directory_get_checkitem(PanelMenuEntry *entry)
{
    PanelMenuDirectory *directory;

    g_return_val_if_fail(entry != NULL, NULL);
    g_return_val_if_fail(entry->type == PANEL_MENU_TYPE_DIRECTORY, NULL);

    directory = (PanelMenuDirectory *)entry->data;
    return(directory->checkitem);
}

GtkWidget *
panel_menu_directory_get_popup(PanelMenuEntry *entry)
{
    PanelMenuDirectory *directory;

    g_return_val_if_fail(entry != NULL, NULL);
    g_return_val_if_fail(entry->type == PANEL_MENU_TYPE_DIRECTORY, NULL);

    directory = (PanelMenuDirectory *)entry->data;
    return(directory->popup);
}

gchar *
panel_menu_directory_dump_xml(PanelMenuEntry *entry)
{
    PanelMenuDirectory *directory;
    GString *string;
    gchar *str;
    gboolean visible;
    gchar *level;

    g_return_val_if_fail(entry != NULL, NULL);
    g_return_val_if_fail(entry->type == PANEL_MENU_TYPE_DIRECTORY, NULL);

    directory = (PanelMenuDirectory *)entry->data;
    visible = GTK_CHECK_MENU_ITEM(directory->checkitem)->active;
    level = g_strdup_printf("%d", directory->level);

    string = g_string_new("    <directory-item>\n"
                          "        <name>");
    g_string_append(string, directory->name);
    g_string_append(string, "</name>\n");
    g_string_append(string, "        <level>");
    g_string_append(string, level);
    g_string_append(string, "</level>\n");
    g_string_append(string, "        <path>");
    g_string_append(string, directory->path);
    g_string_append(string, "</path>\n");
    g_string_append(string, "        <visible>");
    g_string_append(string, visible ? "true" : "false");
    g_string_append(string, "</visible>\n");
    g_string_append(string, "    </directory-item>\n");
    g_free(level);
    str = string->str;
    g_string_free(string, FALSE);
    return(str);
}

gboolean
panel_menu_directory_accept_drop(PanelMenuEntry *entry, GnomeVFSURI *uri)
{
    gchar *fileuri;
    gchar *icon;
    GnomeVFSFileInfo finfo;
    GtkWidget *menuitem;
    GtkWidget *image;
    gboolean retval = FALSE;

    g_return_val_if_fail(entry != NULL, FALSE);
    g_return_val_if_fail(entry->type == PANEL_MENU_TYPE_DIRECTORY, FALSE);

    return(retval);
}

static void
panel_menu_directory_load(const gchar *uri, GtkMenuShell *parent, gint level)
{
    GnomeVFSResult result;
    GnomeVFSAsyncHandle *load_handle;
    gchar *path;

    if(uri && parent)
    {
        path = panel_menu_common_build_full_path(uri, "");
        g_object_set_data(G_OBJECT(parent), "uri-path", path);
        g_object_set_data(G_OBJECT(parent), "level", GINT_TO_POINTER(level));
        gnome_vfs_async_load_directory(&load_handle, path, GNOME_VFS_FILE_INFO_FOLLOW_LINKS, 5, 1, (GnomeVFSAsyncDirectoryLoadCallback)directory_load_cb, (gpointer)parent);
    }
}

static void
directory_load_cb(GnomeVFSAsyncHandle *handle, GnomeVFSResult result, GList *list, guint entries_read, GtkMenuShell *parent)
{
    GList *iter = NULL;
    GnomeVFSFileInfo *finfo = NULL;
    GtkWidget *menuitem = NULL;
    GtkWidget *submenu = NULL;
    GtkWidget *image = NULL;
    gchar *icon = NULL;
    gchar *path = NULL;
    gchar *subpath = NULL;
    gint level = 0;
    gint count = 0;

    path = (gchar *)g_object_get_data(G_OBJECT(parent), "uri-path");
    level = (gint)g_object_get_data(G_OBJECT(parent), "level") - 1;

    for(iter = list, count = 0; iter && count < entries_read; iter = iter->next, count++)
    {
        finfo = (GnomeVFSFileInfo *)iter->data;
        subpath = panel_menu_common_build_full_path(path, finfo->name);
        if(!subpath) continue;
        if(finfo->type == GNOME_VFS_FILE_TYPE_DIRECTORY)
        {
            if(level > 0) 
            {
                menuitem = gtk_image_menu_item_new_with_label(finfo->name);
                g_object_set_data(G_OBJECT(menuitem), "uri-path", g_strdup(subpath));
                g_signal_connect(G_OBJECT(menuitem), "destroy", G_CALLBACK(panel_menu_common_destroy_apps_menuitem), NULL);
                panel_menu_common_set_icon_scaled_from_file(GTK_MENU_ITEM(menuitem), "/usr/share/pixmaps/gnome-folder.png");
                gtk_menu_shell_append(parent, menuitem);
                gtk_widget_show(menuitem);
                submenu = gtk_menu_new();
                GTK_MENU(submenu)->parent_menu_item = menuitem;
                gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem), submenu);
                panel_menu_directory_load((const gchar *)subpath, GTK_MENU_SHELL(submenu), level);
            }
        }
        else
        {
            menuitem = gtk_image_menu_item_new_with_label(finfo->name);
            panel_menu_common_apps_menuitem_dnd_init(menuitem);
            icon = (gchar *)gnome_vfs_mime_get_value((gchar *)gnome_vfs_mime_type_from_name(finfo->name), "icon-filename");
            if(icon)
            {
                panel_menu_common_set_icon_scaled_from_file(GTK_MENU_ITEM(menuitem), icon);
            }
            else
            {
                image = gtk_image_new_from_stock(GTK_STOCK_NEW, GTK_ICON_SIZE_MENU);
                gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem), image);
                gtk_widget_show(image);
            }
            gtk_menu_shell_append(parent, menuitem);
            g_object_set_data(G_OBJECT(menuitem), "uri-path", g_strdup(subpath));
            g_signal_connect(G_OBJECT(menuitem), "destroy", G_CALLBACK(panel_menu_common_destroy_apps_menuitem), NULL);
            gtk_widget_show(menuitem);
        }
        g_free(subpath);
    }
    if(result != GNOME_VFS_OK)
    {
        if(parent)
        {
            subpath = (gchar *)g_object_get_data(G_OBJECT(parent), "uri-path");
            g_free(subpath);
            g_object_set_data(G_OBJECT(parent), "uri-path", NULL);
            if(GTK_MENU(parent)->parent_menu_item && g_list_length(parent->children) < 2)
            {
                gtk_widget_destroy(GTK_MENU(parent)->parent_menu_item);
            }
        }
    }
}

void
panel_menu_directory_new_with_dialog(PanelMenu *panel_menu)
{
    GtkWidget *dialog;
    GtkWidget *name_entry;
    GtkWidget *path_entry;
    GtkWidget *level_spin;
    gint response;
    gchar *name;
    gchar *path;
    gint level;

    PanelMenuEntry *entry;

    dialog = panel_menu_directory_edit_dialog_new(_("Create directory item..."), &name_entry, &path_entry, &level_spin);
    gtk_entry_set_text(GTK_ENTRY(name_entry), _("Home"));
    gtk_entry_set_text(GTK_ENTRY(path_entry), g_get_home_dir());
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(level_spin), 1.0);
    gtk_widget_show(dialog);
    gtk_widget_grab_focus(path_entry);

    response = gtk_dialog_run(GTK_DIALOG(dialog));
    if(response == GTK_RESPONSE_OK)
    {
        name = (gchar *)gtk_entry_get_text(GTK_ENTRY(name_entry));
        path = (gchar *)gtk_entry_get_text(GTK_ENTRY(path_entry));
        level = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(level_spin));
        entry = panel_menu_directory_new(panel_menu, name, path, level);
        panel_menu->entries = g_list_append(panel_menu->entries, (gpointer)entry);
        panel_menu_options_append_option(panel_menu_common_find_options(panel_menu), panel_menu_directory_get_checkitem(entry));
        gtk_menu_shell_append(GTK_MENU_SHELL(panel_menu->menubar), panel_menu_directory_get_widget(entry));
        panel_menu_common_set_changed(panel_menu);
    }
    gtk_widget_destroy(dialog);
}

static void
change_directory_cb(GtkWidget *widget, PanelMenuEntry *entry)
{
    PanelMenuDirectory *directory;
    GtkWidget *dialog;
    GtkWidget *name_entry;
    GtkWidget *path_entry;
    GtkWidget *level_spin;
    gint response;
    gchar *new_name;
    gchar *new_path;
    gint new_level;
    gboolean changed = FALSE;

    g_return_if_fail(entry != NULL);
    g_return_if_fail(entry->type == PANEL_MENU_TYPE_DIRECTORY);

    directory = (PanelMenuDirectory *)entry->data;
    dialog = panel_menu_directory_edit_dialog_new(_("Edit directory item..."), &name_entry, &path_entry, &level_spin);
    gtk_entry_set_text(GTK_ENTRY(name_entry), directory->name);
    gtk_entry_set_text(GTK_ENTRY(path_entry), directory->path);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(level_spin), (gfloat)directory->level);

    gtk_widget_show(dialog);
    gtk_widget_grab_focus(name_entry);

    response = gtk_dialog_run(GTK_DIALOG(dialog));
    if(response == GTK_RESPONSE_OK)
    {
        new_name = (gchar *)gtk_entry_get_text(GTK_ENTRY(name_entry));
        new_path = (gchar *)gtk_entry_get_text(GTK_ENTRY(path_entry));
        new_level = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(level_spin));
        if(strcmp(directory->name, new_name))
        {
            panel_menu_directory_set_name(entry, new_name);
            changed = TRUE;
        }
        if(strcmp(directory->path, new_path))
        {
            directory->level = new_level;
            panel_menu_directory_set_path(entry, new_path);
            changed = TRUE;
        }
        else if(directory->level != new_level)
        {
            directory->level = new_level;
            panel_menu_directory_set_path(entry, directory->path);
            changed = TRUE;
        }
        if(changed) panel_menu_common_set_changed(entry->parent);
    }
    gtk_widget_destroy(dialog);
}

static GtkWidget *
panel_menu_directory_edit_dialog_new(gchar *title, GtkWidget **nentry, GtkWidget **pentry, GtkWidget **spin)
{
    GtkWidget *dialog;
    GtkWidget *box;
    GtkWidget *hbox;
    GtkWidget *label;

    dialog = gtk_dialog_new_with_buttons(title,
                                         NULL, GTK_DIALOG_MODAL,
                                         GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
                                         GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);

    box = GTK_DIALOG(dialog)->vbox;

    hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
    gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 5);
    gtk_widget_show(hbox);
    label = gtk_label_new(_("Name:"));
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 5);
    gtk_widget_show(label);
    *nentry = gtk_entry_new_with_max_length(50);
    gtk_box_pack_start(GTK_BOX(hbox), *nentry, TRUE, TRUE, 5);
    gtk_widget_show(*nentry);

    hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
    gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 5);
    gtk_widget_show(hbox);
    label = gtk_label_new(_("Path:"));
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 5);
    gtk_widget_show(label);
    *pentry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(hbox), *pentry, TRUE, TRUE, 5);
    gtk_widget_show(*pentry);

    hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
    gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 5);
    gtk_widget_show(hbox);
    label = gtk_label_new(_("Depth level:"));
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 5);
    gtk_widget_show(label);
    *spin = gtk_spin_button_new(GTK_ADJUSTMENT(gtk_adjustment_new(1, 1.0, 10, 1, 1, 1)), 1, 0);
    gtk_box_pack_end(GTK_BOX(hbox), *spin, TRUE, TRUE, 5);
    gtk_spin_button_set_update_policy(GTK_SPIN_BUTTON(*spin), GTK_UPDATE_ALWAYS);
    gtk_widget_show(*spin);
    return(dialog);
}
