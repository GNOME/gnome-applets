/*  panel-menu-config.c
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
#include <panel-applet-gconf.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

#include "panel-menu.h"
#include "panel-menu-common.h"
#include "panel-menu-options.h"
#include "panel-menu-path.h"
#include "panel-menu-links.h"
#include "panel-menu-directory.h"
#include "panel-menu-documents.h"
#include "panel-menu-actions.h"
#include "panel-menu-windows.h"
#include "panel-menu-workspaces.h"

#include "panel-menu-config.h"

void
applet_load_config_cb(BonoboUIComponent *uic, PanelMenu *panel_menu, const gchar *verbname)
{
    PanelMenuEntry *options = NULL;
    PanelMenuEntry *entry = NULL;
    GList *cur = NULL;

    for(cur = panel_menu->entries; cur; cur = cur->next)
    {
        entry = (PanelMenuEntry *)cur->data;
        if(entry->type != PANEL_MENU_TYPE_OPTIONS)
            panel_menu_common_call_entry_destroy(entry);
        else
            options = entry;
    }
    if(options) panel_menu_options_destroy(options);
    if(!panel_menu_config_load_xml(panel_menu))
    {
        panel_menu_config_load_xml_string(panel_menu, default_config, strlen(default_config));
    }
}

void
applet_save_config_cb(BonoboUIComponent *uic, PanelMenu *panel_menu, const gchar *verbname)
{
    panel_menu_config_save_xml(panel_menu);
}

void
panel_menu_config_load_prefs(PanelMenu *panel_menu)
{
    panel_menu->show_icon_handle = panel_applet_gconf_get_bool(panel_menu->applet, "show-icon-handle", NULL);
    panel_menu->icon_handle_image = panel_applet_gconf_get_string(panel_menu->applet, "icon-handle-image", NULL);
    if(!panel_menu->icon_handle_image || !strcmp(panel_menu->icon_handle_image, "none"))
    {
        g_free(panel_menu->icon_handle_image);
        panel_menu->icon_handle_image = g_strdup(DATADIR "/pixmaps/panel-menu-icon.png");
    }
    panel_menu->auto_popup_menus = panel_applet_gconf_get_bool(panel_menu->applet, "auto-popup-menus", NULL);
    panel_menu->auto_popup_menus_timeout = panel_applet_gconf_get_int(panel_menu->applet, "auto-popup-menus-timeout", NULL);
    panel_menu->auto_directory_update = panel_applet_gconf_get_bool(panel_menu->applet, "auto-directory-update", NULL);
    panel_menu->auto_directory_update_timeout = panel_applet_gconf_get_int(panel_menu->applet, "auto-directory-update-timeout", NULL);
    panel_menu->auto_save_config = panel_applet_gconf_get_bool(panel_menu->applet, "auto-save-config", NULL);
}

void
panel_menu_config_save_prefs(PanelMenu *panel_menu)
{
    panel_applet_gconf_set_bool(panel_menu->applet, "show-icon-handle", panel_menu->show_icon_handle, NULL);
    panel_applet_gconf_set_string(panel_menu->applet, "icon-handle-image", panel_menu->icon_handle_image, NULL);
    panel_applet_gconf_set_bool(panel_menu->applet, "auto-popup-menus", panel_menu->auto_popup_menus, NULL);
    panel_applet_gconf_set_int(panel_menu->applet, "auto-popup-menus-timeout", panel_menu->auto_popup_menus_timeout, NULL);
    panel_applet_gconf_set_bool(panel_menu->applet, "auto-directory-update", panel_menu->auto_directory_update, NULL);
    panel_applet_gconf_set_int(panel_menu->applet, "auto-directory-update-timeout", panel_menu->auto_directory_update_timeout, NULL);
    panel_applet_gconf_set_bool(panel_menu->applet, "auto-save-config", panel_menu->auto_save_config, NULL);
}

gboolean
panel_menu_config_load_xml(PanelMenu *panel_menu)
{
    gchar *filename;
    gchar *string;
    guint len;
    gboolean retval = FALSE;

    filename = g_strdup_printf("%s/.gnome2/panel-menu/%s-%s.xml", g_get_home_dir(), panel_menu->profile_id, panel_menu->applet_id);
    g_file_get_contents(filename, &string, &len, NULL);
    g_free(filename);
    if(len)
    {
        retval = panel_menu_config_load_xml_string(panel_menu, string, len);
        g_free(string);
    }
    return(retval);
}

gboolean
panel_menu_config_load_xml_string(PanelMenu *panel_menu, gchar *string, gint length)
{
    xmlDocPtr doc = NULL;
    xmlNodePtr root = NULL;
    xmlNodePtr section = NULL;
    xmlNodePtr node = NULL;
    PanelMenuEntry *options;
    PanelMenuEntry *entry;
    gchar *text;
    GtkWidget *menuitem;
    GtkWidget *checkitem;
    gboolean constructed;
    gboolean retval = FALSE;

    if(!panel_menu->profile_id || !panel_menu->applet_id) return(FALSE);
    xmlKeepBlanksDefault(0);
    doc = xmlParseMemory(string, length);
    if(!doc) return(FALSE);
    root = xmlDocGetRootElement(doc);
    if(!root) return(FALSE);

    /* Must be constructed here, since each sub-item needs to add a checkitem */
    options = panel_menu_options_new(panel_menu, panel_menu->icon_handle_image);

    for(section = root->xmlChildrenNode; section; section = section->next)
    {
        constructed = FALSE;
        if(!strcmp(section->name, "options-item"))
        {
            panel_menu->entries = g_list_append(panel_menu->entries, (gpointer)options);
            gtk_menu_shell_append(GTK_MENU_SHELL(panel_menu->menubar), panel_menu_options_get_widget(options));
            if(!panel_menu->show_icon_handle) gtk_widget_hide(panel_menu_options_get_widget(options));
            retval = TRUE;
        }
        else if(!strcmp(section->name, "path-item"))
        {
            for(node = section->xmlChildrenNode; node != NULL; node = node->next)
            {
                if(!strcmp(node->name, "base-path"))
                {
                    text = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
                    if(text && strlen(text) > 1)
                    {
                        entry = panel_menu_path_new(panel_menu, text);
                        panel_menu->entries = g_list_append(panel_menu->entries, (gpointer)entry);
                        panel_menu_options_append_option(options, panel_menu_path_get_checkitem(entry));
                        gtk_menu_shell_append(GTK_MENU_SHELL(panel_menu->menubar), panel_menu_path_get_widget(entry));
                        constructed =TRUE;
                    }
                    if(text) g_free(text);
                }
                else if(constructed && !strcmp(node->name, "path"))
                {
                    text = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
                    if(text && strlen(text) > 1)
                    {
                        panel_menu_path_append_item(entry, text);
                    }
                    if(text) g_free(text);
                }
                else if(constructed && !strcmp(node->name, "visible"))
                {
                    text = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
                    if(text && !strcmp(text, "false"))
                    {
                        checkitem = panel_menu_path_get_checkitem(entry);
                        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(checkitem), FALSE);
                    }
                    if(text) g_free(text);
                }
            }
        }
        else if(!strcmp(section->name, "links-item"))
        {
            for(node = section->xmlChildrenNode; node != NULL; node = node->next)
            {
                if(!strcmp(node->name, "name"))
                {
                    text = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
                    if(text && strlen(text) > 1)
                    {
                        entry = panel_menu_links_new(panel_menu, text);
                        panel_menu->entries = g_list_append(panel_menu->entries, (gpointer)entry);
                        panel_menu_options_append_option(options, panel_menu_links_get_checkitem(entry));
                        gtk_menu_shell_append(GTK_MENU_SHELL(panel_menu->menubar), panel_menu_links_get_widget(entry));
                        constructed = TRUE;
                    }
                }
                else if(constructed && !strcmp(node->name, "link"))
                {
                    text = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
                    if(text && strlen(text) > 1)
                    {
                        panel_menu_links_append_item(entry, text);
                    }
                    if(text) g_free(text);
                }
                else if(constructed && !strcmp(node->name, "visible"))
                {
                    text = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
                    if(text && !strcmp(text, "false"))
                    {
                        checkitem = panel_menu_links_get_checkitem(entry);
                        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(checkitem), FALSE);
                    }
                    if(text) g_free(text);
                }
            }
        }
        else if(!strcmp(section->name, "directory-item"))
        {
            gchar *name = NULL;
            gint level = 0;
            for(node = section->xmlChildrenNode; node != NULL; node = node->next)
            {
                if(!strcmp(node->name, "name"))
                {
                    text = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
                    if(text && strlen(text) > 1)
                    {
                        name = g_strdup(text);
                    }
                }
                else if(!strcmp(node->name, "level"))
                {
                    text = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
                    if(text && strlen(text))
                    {
                        level = atoi(text);
                    }
                }
                else if(name && level && !strcmp(node->name, "path"))
                {
                    text = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
                    if(text && strlen(text) > 1)
                    {
                        entry = panel_menu_directory_new(panel_menu, name, text, level);
                        panel_menu->entries = g_list_append(panel_menu->entries, (gpointer)entry);
                        panel_menu_options_append_option(options, panel_menu_directory_get_checkitem(entry));
                        gtk_menu_shell_append(GTK_MENU_SHELL(panel_menu->menubar), panel_menu_directory_get_widget(entry));
                        constructed = TRUE;
                    }
                    if(text) g_free(text);
                    if(name) g_free(name);
                }
                else if(constructed && !strcmp(node->name, "visible"))
                {
                    text = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
                    if(text && !strcmp(text, "false"))
                    {
                        checkitem = panel_menu_directory_get_checkitem(entry);
                        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(checkitem), FALSE);
                    }
                    if(text) g_free(text);
                }
            }
        }
        else if(!strcmp(section->name, "documents-item"))
        {
            for(node = section->xmlChildrenNode; node != NULL; node = node->next)
            {
                if(!strcmp(node->name, "name"))
                {
                    text = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
                    if(text && strlen(text) > 1)
                    {
                        entry = panel_menu_documents_new(panel_menu, text);
                        panel_menu->entries = g_list_append(panel_menu->entries, (gpointer)entry);
                        panel_menu_options_append_option(options, panel_menu_documents_get_checkitem(entry));
                        gtk_menu_shell_append(GTK_MENU_SHELL(panel_menu->menubar), panel_menu_documents_get_widget(entry));
                        constructed = TRUE;
                    }
                }
                else if(constructed && !strcmp(node->name, "document"))
                {
                    text = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
                    if(text && strlen(text) > 1)
                    {
                        panel_menu_documents_append_item(entry, text);
                    }
                    if(text) g_free(text);
                }
                else if(constructed && !strcmp(node->name, "visible"))
                {
                    text = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
                    if(text && !strcmp(text, "false"))
                    {
                        checkitem = panel_menu_documents_get_checkitem(entry);
                        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(checkitem), FALSE);
                    }
                    if(text) g_free(text);
                }
            }
        }
        else if(!strcmp(section->name, "actions-item"))
        {
            for(node = section->xmlChildrenNode; node != NULL; node = node->next)
            {
                if(!strcmp(node->name, "name"))
                {
                    text = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
                    if(text && strlen(text) > 1)
                    {
                        entry = panel_menu_actions_new(panel_menu, text);
                        panel_menu->entries = g_list_append(panel_menu->entries, (gpointer)entry);
                        panel_menu_options_append_option(options, panel_menu_actions_get_checkitem(entry));
                        gtk_menu_shell_append(GTK_MENU_SHELL(panel_menu->menubar), panel_menu_actions_get_widget(entry));
                        constructed = TRUE;
                    }
                }
                else if(constructed && !strcmp(node->name, "action"))
                {
                    text = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
                    if(text && strlen(text) > 1)
                    {
                        panel_menu_actions_append_item(entry, text);
                    }
                    if(text) g_free(text);
                }
                else if(constructed && !strcmp(node->name, "visible"))
                {
                    text = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
                    if(text && !strcmp(text, "false"))
                    {
                        checkitem = panel_menu_actions_get_checkitem(entry);
                        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(checkitem), FALSE);
                    }
                    if(text) g_free(text);
                }
            }
        }
        else if(!strcmp(section->name, "windows-item"))
        {
            entry = panel_menu_windows_new(panel_menu);
            panel_menu->entries = g_list_append(panel_menu->entries, (gpointer)entry);
            panel_menu_options_append_option(options, panel_menu_windows_get_checkitem(entry));
            gtk_menu_shell_append(GTK_MENU_SHELL(panel_menu->menubar), panel_menu_windows_get_widget(entry));
            for(node = section->xmlChildrenNode; node != NULL; node = node->next)
            {
                if(!strcmp(node->name, "visible"))
                {
                    text = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
                    if(text && !strcmp(text, "false"))
                    {
                        checkitem = panel_menu_windows_get_checkitem(entry);
                        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(checkitem), FALSE);
                    }
                    if(text) g_free(text);
                }
            }
        }
        else if(!strcmp(section->name, "workspaces-item"))
        {
            entry = panel_menu_workspaces_new(panel_menu);
            panel_menu->entries = g_list_append(panel_menu->entries, (gpointer)entry);
            panel_menu_options_append_option(options, panel_menu_workspaces_get_checkitem(entry));
            gtk_menu_shell_append(GTK_MENU_SHELL(panel_menu->menubar), panel_menu_workspaces_get_widget(entry));
            for(node = section->xmlChildrenNode; node != NULL; node = node->next)
            {
                if(!strcmp(node->name, "visible"))
                {
                    text = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
                    if(text && !strcmp(text, "false"))
                    {
                        checkitem = panel_menu_workspaces_get_checkitem(entry);
                        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(checkitem), FALSE);
                    }
                    if(text) g_free(text);
                }
            }
        }
    }
    xmlFreeDoc(doc);
    return(retval);
}

void
panel_menu_config_save_xml(PanelMenu *panel_menu)
{
    FILE *out;
    gchar *filename;
    GList *cur;
    PanelMenuEntry *entry;
    gchar *string;

    if(!panel_menu->profile_id || !panel_menu->applet_id) return;

    filename = g_strdup_printf("%s/.gnome2/panel-menu/%s-%s.xml", g_get_home_dir(), panel_menu->profile_id, panel_menu->applet_id);
    out = fopen(filename, "w");
    g_free(filename);
    if(!out) return;

    g_print("Saving xml specification...\n");

    fprintf(out, "<panel-menu>\n");
    for(cur = panel_menu->entries; cur; cur = cur->next)
    {
        entry = (PanelMenuEntry *)cur->data;
        string = NULL;
        switch(entry->type)
        {
            case PANEL_MENU_TYPE_OPTIONS:
                string = panel_menu_options_dump_xml(entry);
                break;
            case PANEL_MENU_TYPE_MENU_PATH:
                string = panel_menu_path_dump_xml(entry);
                break;
            case PANEL_MENU_TYPE_LINKS:
                string = panel_menu_links_dump_xml(entry);
                break;
            case PANEL_MENU_TYPE_DIRECTORY:
                string = panel_menu_directory_dump_xml(entry);
                break;
            case PANEL_MENU_TYPE_DOCUMENTS:
                string = panel_menu_documents_dump_xml(entry);
                break;
            case PANEL_MENU_TYPE_ACTIONS:
                string = panel_menu_actions_dump_xml(entry);
                break;
            case PANEL_MENU_TYPE_WINDOWS:
                string = panel_menu_windows_dump_xml(entry);
                break;
            case PANEL_MENU_TYPE_WORKSPACES:
                string = panel_menu_workspaces_dump_xml(entry);
                break;
            default:
                break;
        }
        if(string)
        {
            fprintf(out, string);
            g_free(string);
        }
    }
    fprintf(out, "</panel-menu>\n");
    fclose(out);
    panel_menu->changed = FALSE;
}
