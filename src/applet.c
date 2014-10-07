/*
 * Copyright (C) 2008 Canonical Ltd
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by Neil Jagdish Patel <neil.patel@canonical.com>
 * GTK3 Port by Sebastian Geiger <sbastig@gmx.net>
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <stdlib.h>

#include <glib.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include <glib/gi18n.h>

#include <libwnck/libwnck.h>

#include <panel-applet.h>

#include "common.h"
#include "task-list.h"
#include "task-title.h"

WinPickerApp *mainapp;

static void display_about_dialog (
    GtkAction *action,
    PanelApplet *applet
);

static void display_prefs_dialog (
    GtkAction *action,
    PanelApplet *applet
);

static const GtkActionEntry menuActions [] = {
    {"Preferences", GTK_STOCK_PREFERENCES, N_("_Preferences"),
        NULL, NULL,
        G_CALLBACK (display_prefs_dialog) },
    { "About", GTK_STOCK_ABOUT, N_("_About"),
        NULL, NULL,
      G_CALLBACK (display_about_dialog) }
};

static const gchar *windowPickerAppletAuthors[] = {
    "Neil J. Patel <neil.patel@canonical.com>",
    "Sebastian Geiger <sbastig@gmx.net>",
    NULL
};

/**
 * This functions loads our custom CSS and registers the CSS style class
 * for the applets style context
 */
static inline void loadAppletStyle (GtkWidget *widget) {
    static gboolean first_time = TRUE;
    if (first_time) {
        GtkStyleContext *context = gtk_widget_get_style_context (widget);
        //Prepare the provider for our applet specific CSS
        GtkCssProvider *provider = gtk_css_provider_new ();
        gtk_css_provider_load_from_data (
            GTK_CSS_PROVIDER(provider),
            ".na-tray-style {\n"
            "   -GtkWidget-focus-line-width: 0;\n"
            "   -GtkWidget-focus-padding: 0;\n"
            "}\n",
            -1, NULL);
        gtk_style_context_add_provider (context, GTK_STYLE_PROVIDER(provider),
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
        g_object_unref (provider);
        //register the CSS style for the applets context
        gtk_style_context_add_class (context, "na-tray-style");
        first_time = FALSE;
    }
}

static void setupPanelContextMenu() {
    GtkActionGroup* action_group = gtk_action_group_new ("Window Picker Applet Actions");
    gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);
    gtk_action_group_add_actions (action_group,
        menuActions,
        G_N_ELEMENTS (menuActions),
        NULL); //NULL because we are not passing any data to the callbacks
    char *ui_path = g_build_filename (WINDOW_PICKER_MENU_UI_DIR, "menu.xml", NULL);
    panel_applet_setup_menu_from_file(
        PANEL_APPLET(mainapp->applet), //mainapp is a global struct
        ui_path,
        action_group
    );
    g_free(ui_path);
    g_object_unref (action_group);
}

static gboolean load_window_picker (
    PanelApplet *applet,
    const gchar *iid,
    gpointer     data)
{
    WinPickerApp *app;
    GtkWidget *grid, *tasks, *title;
    if (strcmp (iid, "WindowPicker") != 0)
        return FALSE;

    bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    textdomain (GETTEXT_PACKAGE);
    wnck_set_client_type (WNCK_CLIENT_TYPE_PAGER);
    mainapp = app = g_slice_new0 (WinPickerApp);
    GSettings* settings = g_settings_new(
        "org.gnome.window-picker-applet"
    );
    mainapp->settings = settings;
    app->applet = GTK_WIDGET (applet);
    loadAppletStyle (GTK_WIDGET (applet)); //Load applet specific CSS Styles
    gtk_container_set_border_width (GTK_CONTAINER (applet), 0);
    grid = gtk_grid_new ();
    //10 pixel spacing between task icons and title bar
    gtk_grid_set_row_spacing (GTK_GRID(grid), 10);
    gtk_container_add (GTK_CONTAINER (applet), grid);
    panel_applet_set_background_widget(applet, GTK_WIDGET(applet));
    gtk_container_set_border_width (GTK_CONTAINER (grid), 0);
    tasks = app->tasks = task_list_get_default ();
    gtk_widget_set_vexpand (tasks, TRUE);
    gtk_grid_attach (GTK_GRID(grid), tasks, 0, 0, 1, 1);
    title = app->title = task_title_new ();
    gtk_widget_set_hexpand (title, TRUE);
    gtk_grid_attach (GTK_GRID(grid), title, 1, 0, 1, 1);
    gboolean show_windows = g_settings_get_boolean (settings, SHOW_WIN_KEY);
    g_object_set (app->tasks, SHOW_WIN_KEY, show_windows, NULL);

    //Setup the applets context menu
    setupPanelContextMenu();

    int flags = PANEL_APPLET_EXPAND_MINOR |
                PANEL_APPLET_HAS_HANDLE;
	gboolean expand_task_list = g_settings_get_boolean (settings, EXPAND_TASK_LIST);
	if(expand_task_list) {
		flags |= PANEL_APPLET_EXPAND_MAJOR;
	}
    panel_applet_set_flags (PANEL_APPLET (applet), flags);

    gtk_widget_show_all (GTK_WIDGET (applet));
    return TRUE;
}

PANEL_APPLET_OUT_PROCESS_FACTORY (
    "WindowPickerFactory",
    PANEL_TYPE_APPLET,
    load_window_picker,
    NULL
);

static void display_about_dialog (
    GtkAction *action,
    PanelApplet *applet)
{
    GtkWidget *panel_about_dialog = gtk_about_dialog_new ();
    g_object_set (panel_about_dialog,
        "name", _("Window Picker"),
        "comments", _("Window Picker"),
        "version", PACKAGE_VERSION,
        "authors", windowPickerAppletAuthors,
        "logo-icon-name", "system-preferences-windows",
        "copyright", "Copyright \xc2\xa9 2008 Canonical Ltd\nand Sebastian Geiger",
        NULL
    );
    char *logo_filename = g_build_filename (WINDOW_PICKER_MENU_UI_DIR, "window-picker-about-logo.png", NULL);
    GdkPixbuf* logo = gdk_pixbuf_new_from_file(logo_filename, NULL);
    gtk_about_dialog_set_logo (GTK_ABOUT_DIALOG(panel_about_dialog), logo);
    if (logo)
        g_object_unref (logo);
    gtk_widget_show (panel_about_dialog);
    g_signal_connect (panel_about_dialog, "response",
        G_CALLBACK (gtk_widget_destroy), NULL);
    gtk_window_present (GTK_WINDOW (panel_about_dialog));
}

static void on_checkbox_toggled (GtkToggleButton *check, gpointer userdata) {
    gboolean is_active = gtk_toggle_button_get_active (check);
    char* key = (char *) userdata;
    g_settings_set_boolean (mainapp->settings, key, is_active);
}

/**
 * Utility function which prepares the check box for the preferences
 * window.
 */
static GtkWidget* prepareCheckBox(char* text, char* key) {
    GtkWidget *check = gtk_check_button_new_with_label (text);
    gboolean is_active = g_settings_get_boolean(
        mainapp->settings,
        key
    );
    gtk_toggle_button_set_active (
        GTK_TOGGLE_BUTTON (check),
        is_active
    );
    g_signal_connect (check, "toggled",
        G_CALLBACK (on_checkbox_toggled), key);
    return check;
}

static void display_prefs_dialog(
    GtkAction *action,
    PanelApplet *applet)
{
    //Setup the Preferences window
    GtkWidget *window, *notebook, *check, *button, *grid;
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (window), _("Preferences"));
    gtk_window_set_type_hint (GTK_WINDOW (window), GDK_WINDOW_TYPE_HINT_DIALOG);
    gtk_container_set_border_width (GTK_CONTAINER (window), 12);
    //Setup the notebook which holds our gui items
    notebook = gtk_notebook_new ();
    gtk_container_add (GTK_CONTAINER (window), notebook);
    gtk_notebook_set_show_tabs (GTK_NOTEBOOK(notebook), FALSE);
    grid = gtk_grid_new ();
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), grid, NULL);
    //Prepare the checkboxes and a button and add it to the grid in the notebook
    check = prepareCheckBox (_("Show windows from all workspaces"), SHOW_WIN_KEY);
    int i=-1;
    gtk_grid_attach (GTK_GRID (grid), check, 0, ++i, 1, 1);
    check = prepareCheckBox (_("Show the home title and\n"
        "logout icon, when on the desktop"),
        SHOW_HOME_TITLE_KEY);
    gtk_grid_attach (GTK_GRID (grid), check, 0, ++i, 1, 1);
    check = prepareCheckBox (_("Show the application title and\nclose icon"),
        SHOW_APPLICATION_TITLE_KEY);
    gtk_grid_attach (GTK_GRID (grid), check, 0, ++i, 1, 1);
    check = prepareCheckBox (_("Grey out non active window icons"), ICONS_GREYSCALE_KEY);
    gtk_grid_attach (GTK_GRID (grid), check, 0, ++i, 1, 1);
    check = prepareCheckBox (_("Automatically expand task list to use full space"), EXPAND_TASK_LIST);
    gtk_grid_attach (GTK_GRID (grid), check, 0, ++i, 1, 1);
    button = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
    gtk_widget_set_halign (button, GTK_ALIGN_END);
    gtk_grid_set_row_spacing (GTK_GRID (grid), 0);
    gtk_grid_attach (GTK_GRID(grid), button, 0, ++i, 1, 1);
    //Register all events and show the window
    g_signal_connect (window, "delete-event",
        G_CALLBACK (gtk_widget_destroy), window);
    g_signal_connect (window, "destroy",
        G_CALLBACK (gtk_widget_destroy), window);
    g_signal_connect_swapped (button, "clicked",
        G_CALLBACK (gtk_widget_destroy), window);
    gtk_widget_show_all (window);
    gtk_window_present (GTK_WINDOW (window));
}
