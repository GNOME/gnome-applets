/*  panel-menu-windows.c
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
#include <libwnck/libwnck.h>

#include "panel-menu.h"
#include "panel-menu-common.h"
#include "panel-menu-windows.h"

static const gchar *windows_menu_xml =
	"    <placeholder name=\"ChildItem\">\n"
	"        <menuitem name=\"Remove\" verb=\"Remove\" label=\"Remove Windows\"\n"
	"                  pixtype=\"stock\" pixname=\"gtk-close\"/>\n"
	"        <separator/>"
	"    </placeholder>";

typedef struct _PanelMenuWindows
{
    GtkWidget *checkitem;
    GtkWidget *windows;
    GtkWidget *menu;
    GtkWidget *active;
    /* What we are attached to */
    WnckScreen *screen;
    /* Signal Handlers */
    gulong active_window;
    gulong window_opened;
    gulong window_closed;
    gulong active_workspace;
}PanelMenuWindows;

/* get/remove the window items from the menu */
static void fill_windows_menu(GtkMenuShell *menu);

static void handle_window_change(GtkWidget *menuitem, WnckWindow *window);
static void handle_window_name_change(WnckWindow *window, GtkMenuItem *menuitem);
static void handle_window_icon_change(WnckWindow *window, GtkMenuItem *menuitem);

/* libwnck action */
static void setup_windows_signals(PanelMenuWindows *windows);
static void wnck_active_window_changed(WnckScreen *screen, PanelMenuWindows *windows);
static void wnck_window_added(WnckScreen *screen, WnckWindow *window, PanelMenuWindows *windows);
static void wnck_window_removed(WnckScreen *screen, WnckWindow *window, PanelMenuWindows *windows);
static void wnck_active_workspace_changed(WnckScreen *screen, PanelMenuWindows *windows);

static GtkWidget *construct_window_menuitem(WnckWindow *window, GtkMenuShell *menu);
static void remove_menuitem_signals(GtkWidget *menuitem, WnckWindow *window);

static void set_icon_from_window(GtkImageMenuItem *menuitem, WnckWindow *window);
static void set_visibility(GtkCheckMenuItem *checkitem, GtkWidget *target);

PanelMenuEntry *
panel_menu_windows_new(PanelMenu *parent)
{
    PanelMenuEntry *entry;
    PanelMenuWindows *windows;
    GtkWidget *tearoff;

    entry = g_new0(PanelMenuEntry, 1);
    entry->type = PANEL_MENU_TYPE_WINDOWS;
    entry->parent = parent;
    windows = g_new0(PanelMenuWindows, 1);
    entry->data = (gpointer)windows;
    windows->windows = gtk_menu_item_new_with_label(_("Windows"));
    panel_menu_common_widget_dnd_init(entry);
    gtk_widget_show(windows->windows);
    windows->checkitem = gtk_check_menu_item_new_with_label(_("Show Windows"));
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(windows->checkitem), TRUE);
    g_signal_connect(G_OBJECT(windows->checkitem), "toggled", G_CALLBACK(panel_menu_common_set_visibility), windows->windows);
    windows->menu = gtk_menu_new();
    tearoff = gtk_tearoff_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(windows->menu), tearoff);
    gtk_widget_show(tearoff);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(windows->windows), windows->menu);
    /* Put *all* of the items in the menu */
    fill_windows_menu(GTK_MENU_SHELL(windows->menu));
    /* hide/show the proper windows */
    wnck_active_workspace_changed(wnck_screen_get(0), windows);
    setup_windows_signals(windows);
    return(entry);
}

void
panel_menu_windows_merge_ui (PanelMenuEntry *entry)
{
	BonoboUIComponent  *component;
	gchar *xml;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_WINDOWS);

	component = panel_applet_get_popup_component (entry->parent->applet);

	bonobo_ui_component_add_verb (component, "Remove",
				     (BonoboUIVerbFn)panel_menu_common_remove_entry, entry);
	bonobo_ui_component_set (component, "/popups/button3/ChildMerge/",
				 windows_menu_xml, NULL);
}

void
panel_menu_windows_destroy(PanelMenuEntry *entry)
{
    PanelMenuWindows *windows;

    g_return_if_fail(entry != NULL);
    g_return_if_fail(entry->type == PANEL_MENU_TYPE_WINDOWS);

    windows = (PanelMenuWindows *)entry->data;
    g_signal_handler_disconnect(windows->screen, windows->active_window);
    g_signal_handler_disconnect(windows->screen, windows->window_opened);
    g_signal_handler_disconnect(windows->screen, windows->window_closed);
    g_signal_handler_disconnect(windows->screen, windows->active_workspace);
    gtk_widget_destroy(windows->checkitem);
    gtk_widget_destroy(windows->windows);
    g_free(windows);
}

GtkWidget *
panel_menu_windows_get_widget(PanelMenuEntry *entry)
{
    PanelMenuWindows *windows;

    g_return_val_if_fail(entry != NULL, NULL);
    g_return_val_if_fail(entry->type == PANEL_MENU_TYPE_WINDOWS, NULL);

    windows = (PanelMenuWindows *)entry->data;
    return(windows->windows);
}

GtkWidget *
panel_menu_windows_get_checkitem(PanelMenuEntry *entry)
{
    PanelMenuWindows *windows;

    g_return_val_if_fail(entry != NULL, NULL);
    g_return_val_if_fail(entry->type == PANEL_MENU_TYPE_WINDOWS, NULL);

    windows = (PanelMenuWindows *)entry->data;
    return(windows->checkitem);
}

gchar *
panel_menu_windows_dump_xml(PanelMenuEntry *entry)
{
    PanelMenuWindows *windows;
    GString *string;
    gchar *str;
    gboolean visible;

    g_return_val_if_fail(entry != NULL, NULL);
    g_return_val_if_fail(entry->type == PANEL_MENU_TYPE_WINDOWS, NULL);

    windows = (PanelMenuWindows *)entry->data;
    visible = GTK_CHECK_MENU_ITEM(windows->checkitem)->active;
    string = g_string_new("    <windows-item>\n"
                          "        <visible>");
    g_string_append(string, visible ? "true" : "false");
    g_string_append(string, "</visible>\n");
    g_string_append(string, "    </windows-item>\n");
    str = string->str;
    g_string_free(string, FALSE);
    return(str);
}

static void
fill_windows_menu(GtkMenuShell *menu)
{
    WnckScreen *screen = NULL;
    WnckWindow *window = NULL;
    GList *cur = NULL;

    screen = wnck_screen_get(0);
    for(cur = wnck_screen_get_windows(screen); cur; cur = cur->next)
    {
        window = (WnckWindow *)cur->data;
        if(!wnck_window_is_skip_tasklist(window))
        {
            construct_window_menuitem(window, menu);
        }
    }
}

/* Connect after? */
static void
handle_window_change(GtkWidget *menuitem, WnckWindow *window)
{
    WnckScreen *screen;
    WnckWindow *active;

    screen = wnck_screen_get(0);
    active = wnck_screen_get_active_window(screen);
    if(window != active) wnck_window_activate(window);
}

static void
handle_window_name_change(WnckWindow *window, GtkMenuItem *menuitem)
{
    GtkWidget *label;
    label = GTK_BIN(menuitem)->child;
    gtk_label_set_text(GTK_LABEL(label), wnck_window_get_name(window));
}

static void
handle_window_icon_change(WnckWindow *window, GtkMenuItem *menuitem)
{
    set_icon_from_window(GTK_IMAGE_MENU_ITEM(menuitem), window);
}

static void
setup_windows_signals(PanelMenuWindows *windows)
{
    WnckScreen *screen;

    screen = wnck_screen_get(0);
    windows->screen = screen;
    windows->active_window = g_signal_connect(G_OBJECT(screen), "active_window_changed", G_CALLBACK(wnck_active_window_changed), windows);
    windows->window_opened = g_signal_connect(G_OBJECT(screen), "window_opened", G_CALLBACK(wnck_window_added), windows);
    windows->window_closed = g_signal_connect(G_OBJECT(screen), "window_closed", G_CALLBACK(wnck_window_removed), windows);
    windows->active_workspace = g_signal_connect(G_OBJECT(screen), "active_workspace_changed", G_CALLBACK(wnck_active_workspace_changed), windows);
}

static void
wnck_active_window_changed(WnckScreen *screen, PanelMenuWindows *windows)
{
    GtkWidget *label;
    GList *cur;
    WnckWindow *active;
    WnckWindow *check;
    gchar *string;

    windows->active = NULL;

    active = wnck_screen_get_active_window(screen);
    if(active && !wnck_window_is_skip_tasklist(active))
    {
        for(cur = GTK_MENU_SHELL(windows->menu)->children->next; cur; cur = cur->next)
        {
            check = g_object_get_data(G_OBJECT(cur->data), "wnck-window");
            if(check == active)
            {
                windows->active = GTK_WIDGET(cur->data);
                break;
            }
        }
    }
}

static void
wnck_window_added(WnckScreen *screen, WnckWindow *window, PanelMenuWindows *windows)
{
    GtkWidget *menuitem;

    if(!wnck_window_is_skip_tasklist(window))
    {
        menuitem = construct_window_menuitem(window, GTK_MENU_SHELL(windows->menu));
        if(wnck_window_get_workspace(window) == wnck_screen_get_active_workspace(screen))
            gtk_widget_show(menuitem);
    }
}

static void
wnck_window_removed(WnckScreen *screen, WnckWindow *window, PanelMenuWindows *windows)
{
    GList *cur;
    WnckWindow *check;

    if(!wnck_window_is_skip_tasklist(window))
    {
        for(cur = GTK_MENU_SHELL(windows->menu)->children->next; cur; cur = cur->next)
        {
            check = g_object_get_data(G_OBJECT(cur->data), "wnck-window");
            if(check == window)
            {
                gtk_widget_destroy(GTK_WIDGET(cur->data));
                break;
            }
        }
    }
}

/* Hide/Show the children based on current workspace setting */
static void
wnck_active_workspace_changed(WnckScreen *screen, PanelMenuWindows *windows)
{
    WnckWorkspace* active;
    WnckWorkspace* check;
    WnckWindow* window;
    GList *cur;

    active = wnck_screen_get_active_workspace(screen);
    for(cur = GTK_MENU_SHELL(windows->menu)->children->next; cur; cur = cur->next)
    {
        window = (WnckWindow *)g_object_get_data(G_OBJECT(cur->data), "wnck-window");
        check = wnck_window_get_workspace(window);
        if(check == active)
            gtk_widget_show(GTK_WIDGET(cur->data));
        else
            gtk_widget_hide(GTK_WIDGET(cur->data));
    }
}

static GtkWidget *
construct_window_menuitem(WnckWindow *window, GtkMenuShell *menu)
{
    GtkWidget *menuitem = NULL;
    gulong signal_id;

    menuitem = gtk_image_menu_item_new_with_label(wnck_window_get_name(window));
    set_icon_from_window(GTK_IMAGE_MENU_ITEM(menuitem), window);
    gtk_menu_shell_append(menu, menuitem);
    g_object_set_data(G_OBJECT(menuitem), "wnck-window", window);
    g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(handle_window_change), window);
    signal_id = g_signal_connect(G_OBJECT(window), "name_changed", G_CALLBACK(handle_window_name_change), menuitem);
    g_object_set_data(G_OBJECT(menuitem), "name-changed-signal", GINT_TO_POINTER(signal_id));
    signal_id = g_signal_connect(G_OBJECT(window), "icon_changed", G_CALLBACK(handle_window_icon_change), menuitem);
    g_object_set_data(G_OBJECT(menuitem), "icon-changed-signal", GINT_TO_POINTER(signal_id));
    g_signal_connect(G_OBJECT(menuitem), "destroy", G_CALLBACK(remove_menuitem_signals), window);
    return(menuitem);
}

static void
remove_menuitem_signals(GtkWidget *menuitem, WnckWindow *window)
{
    gulong signal_id;

    signal_id = (gulong)g_object_get_data(G_OBJECT(menuitem), "name-changed-signal");
    if(signal_id) g_signal_handler_disconnect(window, signal_id);
    signal_id = (gulong)g_object_get_data(G_OBJECT(menuitem), "icon-changed-signal");
    if(signal_id) g_signal_handler_disconnect(window, signal_id);
}

#define ICON_SIZE 20

static void
set_icon_from_window(GtkImageMenuItem *menuitem, WnckWindow *window)
{
    GdkPixbuf *pixbuf;
    GtkWidget *image;
    double pix_x, pix_y;
    double greatest;

    pixbuf = wnck_window_get_mini_icon(window);
    if(pixbuf != NULL)
    {
        pix_x = gdk_pixbuf_get_width(pixbuf);
        pix_y = gdk_pixbuf_get_height(pixbuf);
        if (pix_x > ICON_SIZE || pix_y > ICON_SIZE)
        {
            greatest = pix_x > pix_y ? pix_x : pix_y;
            pixbuf = gdk_pixbuf_scale_simple(pixbuf,
                                            (ICON_SIZE / greatest) * pix_x,
                                            (ICON_SIZE / greatest) * pix_y,
                                             GDK_INTERP_BILINEAR);
        }
        image = gtk_image_new_from_pixbuf(pixbuf);
        gtk_widget_show(image);
        gtk_image_menu_item_set_image(menuitem, image);
    }
}
