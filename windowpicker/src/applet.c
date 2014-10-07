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

#include "task-title.h"
#include "task-list.h"
#include "applet.h"

#include <string.h>

#include <glib.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include <glib/gi18n.h>

#include <libwnck/libwnck.h>

#define SETTINGS_SCHEMA            "org.gnome.gnome-applets.window-picker-applet"
#define KEY_SHOW_ALL_WINDOWS       "show-all-windows"
#define KEY_SHOW_APPLICATION_TITLE "show-application-title"
#define KEY_SHOW_HOME_TITLE        "show-home-title"
#define KEY_ICONS_GREYSCALE        "icons-greyscale"
#define KEY_EXPAND_TASK_LIST       "expand-task-list"

struct _WindowPickerAppletPrivate {
    GtkWidget *tasks;
    GtkWidget *title; /* a pointer to the window title widget */
    GSettings *settings;

    gboolean show_all_windows;
    gboolean show_application_title;
    gboolean show_home_title;
    gboolean icons_greyscale;
    gboolean expand_task_list;
};

enum {
     PROP_0,
     PROP_SHOW_ALL_WINDOWS,
     PROP_SHOW_APPLICATION_TITLE,
     PROP_SHOW_HOME_TITLE,
     PROP_ICONS_GREYSCALE,
     PROP_EXPAND_TASK_LIST,
};

G_DEFINE_TYPE_WITH_PRIVATE(WindowPickerApplet, window_picker_applet, PANEL_TYPE_APPLET);

static void display_about_dialog (GtkAction *action, WindowPickerApplet *applet);
static void display_prefs_dialog (GtkAction *action, WindowPickerApplet *applet);

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

static void setupPanelContextMenu(WindowPickerApplet *windowPickerApplet) {
    GtkActionGroup* action_group = gtk_action_group_new ("Window Picker Applet Actions");
    gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);
    gtk_action_group_add_actions (action_group,
        menuActions,
        G_N_ELEMENTS (menuActions),
        windowPickerApplet);
    char *ui_path = g_build_filename (WINDOW_PICKER_MENU_UI_DIR, "menu.xml", NULL);
    panel_applet_setup_menu_from_file(
        PANEL_APPLET(windowPickerApplet),
        ui_path,
        action_group
    );
    g_free(ui_path);
    g_object_unref (action_group);
}

static gboolean
load_window_picker (PanelApplet *applet) {
    WindowPickerApplet *windowPickerApplet = WINDOW_PICKER_APPLET(applet);
    WindowPickerAppletPrivate *priv = windowPickerApplet->priv;
    GSettings *settings = windowPickerApplet->priv->settings
                        = panel_applet_settings_new(applet, SETTINGS_SCHEMA);

    GtkWidget *grid = gtk_grid_new ();
    gtk_grid_set_row_spacing (GTK_GRID(grid), 10);
    gtk_container_add (GTK_CONTAINER (applet), grid);
    gtk_container_set_border_width (GTK_CONTAINER (applet), 0);
    gtk_container_set_border_width (GTK_CONTAINER (grid), 0);

    priv->tasks = task_list_new (windowPickerApplet);
    gtk_widget_set_vexpand (priv->tasks, TRUE);
    gtk_grid_attach (GTK_GRID(grid), priv->tasks, 0, 0, 1, 1);

    priv->title = task_title_new (windowPickerApplet);
    gtk_widget_set_hexpand (priv->title, TRUE);
    gtk_grid_attach (GTK_GRID(grid), priv->title, 1, 0, 1, 1);

    priv->show_all_windows = g_settings_get_boolean (settings, KEY_SHOW_ALL_WINDOWS);
    g_settings_bind (settings, KEY_SHOW_ALL_WINDOWS,
            windowPickerApplet, KEY_SHOW_ALL_WINDOWS,
            G_SETTINGS_BIND_GET);
    priv->show_application_title = g_settings_get_boolean (settings, KEY_SHOW_APPLICATION_TITLE);
    g_settings_bind (settings, KEY_SHOW_APPLICATION_TITLE,
            windowPickerApplet, KEY_SHOW_APPLICATION_TITLE,
            G_SETTINGS_BIND_GET);
    priv->show_home_title = g_settings_get_boolean (settings, KEY_SHOW_HOME_TITLE);
    g_settings_bind (settings, KEY_SHOW_HOME_TITLE,
            windowPickerApplet, KEY_SHOW_HOME_TITLE,
            G_SETTINGS_BIND_GET);
    priv->icons_greyscale = g_settings_get_boolean (settings, KEY_ICONS_GREYSCALE);
    g_settings_bind (settings, KEY_ICONS_GREYSCALE,
            windowPickerApplet, KEY_ICONS_GREYSCALE,
            G_SETTINGS_BIND_GET);
    priv->expand_task_list = g_settings_get_boolean (settings, KEY_EXPAND_TASK_LIST);
    g_settings_bind (settings, KEY_EXPAND_TASK_LIST,
            windowPickerApplet, KEY_EXPAND_TASK_LIST,
            G_SETTINGS_BIND_GET);

    loadAppletStyle (GTK_WIDGET (applet));
    setupPanelContextMenu (windowPickerApplet);

    PanelAppletFlags flags = PANEL_APPLET_EXPAND_MINOR | PANEL_APPLET_HAS_HANDLE;
    if (g_settings_get_boolean(priv->settings, KEY_EXPAND_TASK_LIST))
        flags |= PANEL_APPLET_EXPAND_MAJOR;

    panel_applet_set_flags(applet, flags);
    panel_applet_set_background_widget (applet, GTK_WIDGET(applet));

    gtk_widget_show_all(GTK_WIDGET (applet));

    return TRUE;
}

static void display_about_dialog (GtkAction *action,
                                  WindowPickerApplet *windowPickerApplet)
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

static GtkWidget *
prepareCheckBox (WindowPickerApplet *windowPickerApplet,
                 const gchar        *text,
                 const gchar        *key)
{
    GtkWidget *check = gtk_check_button_new_with_label (text);
    GSettings *settings = windowPickerApplet->priv->settings;
    gboolean is_active = g_settings_get_boolean (settings, key);

    gtk_toggle_button_set_active (
        GTK_TOGGLE_BUTTON (check),
        is_active
    );

    g_settings_bind (windowPickerApplet->priv->settings, key,
            check, "active",
            G_SETTINGS_BIND_DEFAULT);

    return check;
}

static void
display_prefs_dialog (GtkAction          *action,
                      WindowPickerApplet *windowPickerApplet)
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
    int i=-1;

    check = prepareCheckBox (windowPickerApplet, _("Show windows from all workspaces"), KEY_SHOW_ALL_WINDOWS);
    gtk_grid_attach (GTK_GRID (grid), check, 0, ++i, 1, 1);

    check = prepareCheckBox (windowPickerApplet, _("Show the home title and\n"
                                                   "logout icon, when on the desktop"),
                                                   KEY_SHOW_HOME_TITLE);
    gtk_grid_attach (GTK_GRID (grid), check, 0, ++i, 1, 1);

    check = prepareCheckBox (windowPickerApplet, _("Show the application title and\nclose icon"),
                                                   KEY_SHOW_APPLICATION_TITLE);
    gtk_grid_attach (GTK_GRID (grid), check, 0, ++i, 1, 1);

    check = prepareCheckBox (windowPickerApplet, _("Grey out non active window icons"), KEY_ICONS_GREYSCALE);
    gtk_grid_attach (GTK_GRID (grid), check, 0, ++i, 1, 1);

    check = prepareCheckBox (windowPickerApplet, _("Automatically expand task list to use full space"), KEY_EXPAND_TASK_LIST);
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

static gboolean
window_picker_factory (PanelApplet *applet,
                       const gchar *iid,
                       gpointer data)
{
    gboolean result = FALSE;
    static gboolean type_registered = FALSE;

    if (!type_registered) {
        wnck_set_client_type (WNCK_CLIENT_TYPE_PAGER);
        type_registered = TRUE;
    }

    if (strcmp (iid, "WindowPicker") == 0) {
        result = load_window_picker(applet);
    }

    return result;
}

static void
window_picker_get_property(GObject *object,
        guint prop_id,
        GValue *value,
        GParamSpec *pspec) {
    WindowPickerApplet *picker = WINDOW_PICKER_APPLET(object);
    g_return_if_fail (IS_WINDOW_PICKER_APPLET(picker));

    switch (prop_id) {
        case PROP_SHOW_ALL_WINDOWS:
            g_value_set_boolean(value, picker->priv->show_all_windows);
            break;
        case PROP_SHOW_APPLICATION_TITLE:
            g_value_set_boolean(value, picker->priv->show_application_title);
            break;
        case PROP_SHOW_HOME_TITLE:
            g_value_set_boolean(value, picker->priv->show_home_title);
            break;
        case PROP_ICONS_GREYSCALE:
            g_value_set_boolean(value, picker->priv->icons_greyscale);
            break;
        case PROP_EXPAND_TASK_LIST:
            g_value_set_boolean(value, picker->priv->expand_task_list);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
window_picker_set_property(GObject *object,
        guint prop_id,
        const GValue *value,
        GParamSpec *pspec)
{
    WindowPickerApplet *picker = WINDOW_PICKER_APPLET(object);
    g_return_if_fail (IS_WINDOW_PICKER_APPLET(picker));

    switch (prop_id) {
        case PROP_SHOW_ALL_WINDOWS:
            picker->priv->show_all_windows = g_value_get_boolean(value);
            break;
        case PROP_SHOW_APPLICATION_TITLE:
            picker->priv->show_application_title = g_value_get_boolean(value);
            break;
        case PROP_SHOW_HOME_TITLE:
            picker->priv->show_home_title = g_value_get_boolean(value);
            break;
        case PROP_ICONS_GREYSCALE:
            picker->priv->icons_greyscale = g_value_get_boolean(value);
            break;
        case PROP_EXPAND_TASK_LIST:
            picker->priv->expand_task_list = g_value_get_boolean(value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
window_picker_finalize(GObject *object) {
    WindowPickerApplet *windowPickerApplet = WINDOW_PICKER_APPLET(object);

    if (windowPickerApplet->priv->settings) {
        g_object_unref(windowPickerApplet->priv->settings);
        windowPickerApplet->priv->settings = NULL;
    }

    G_OBJECT_CLASS (window_picker_applet_parent_class)->finalize(object);
}

static void
window_picker_applet_init (WindowPickerApplet *picker)
{
    picker->priv = window_picker_applet_get_instance_private (picker);
    picker->priv->tasks = NULL;
    picker->priv->title = NULL;

    picker->priv->settings = NULL;
}

static void
window_picker_applet_class_init (WindowPickerAppletClass *class)
{
    GObjectClass *obj_class = G_OBJECT_CLASS (class);
    obj_class->finalize = window_picker_finalize;
    obj_class->set_property = window_picker_set_property;
    obj_class->get_property = window_picker_get_property;

    g_object_class_install_property (obj_class, PROP_SHOW_ALL_WINDOWS,
            g_param_spec_boolean ("show-all-windows",
                    "Show All Windows",
                    "Show windows from all workspaces",
                    TRUE,
                    G_PARAM_READWRITE));
    g_object_class_install_property (obj_class, PROP_SHOW_APPLICATION_TITLE,
            g_param_spec_boolean ("show-application-title",
                    "Show Application Title",
                    "Show the application title",
                    FALSE,
                    G_PARAM_READWRITE));
    g_object_class_install_property (obj_class, PROP_SHOW_HOME_TITLE,
            g_param_spec_boolean ("show-home-title",
                    "Show Home Title",
                    "Show the home title and logout button",
                    FALSE,
                    G_PARAM_READWRITE));
    g_object_class_install_property (obj_class, PROP_ICONS_GREYSCALE,
            g_param_spec_boolean ("icons-greyscale",
                    "Icons Greyscale",
                    "All icons except the current active window icon are greyed out",
                    FALSE,
                    G_PARAM_READWRITE));
    g_object_class_install_property (obj_class, PROP_EXPAND_TASK_LIST,
            g_param_spec_boolean ("expand-task-list",
                    "Expand Task List",
                    "Whether the task list will expand automatically and use all available space",
                    FALSE,
                    G_PARAM_READWRITE));
}

GtkWidget
*window_picker_applet_get_tasks (WindowPickerApplet *picker) {
    return picker->priv->tasks;
}

gboolean
window_picker_applet_get_show_all_windows(WindowPickerApplet *picker) {
    return picker->priv->show_all_windows;
}

gboolean
window_picker_applet_get_show_application_title (WindowPickerApplet *picker) {
    return picker->priv->show_application_title;
}

gboolean
window_picker_applet_get_show_home_title (WindowPickerApplet *picker) {
    return picker->priv->show_home_title;
}

gboolean
window_picker_applet_get_icons_greyscale (WindowPickerApplet *picker) {
     return picker->priv->icons_greyscale;
}

gboolean
window_picker_applet_get_expand_task_list (WindowPickerApplet *picker) {
    return picker->priv->expand_task_list;
}

PANEL_APPLET_OUT_PROCESS_FACTORY ("WindowPickerFactory",
                                  WINDOW_PICKER_APPLET_TYPE,
                                  window_picker_factory,
                                  NULL);
