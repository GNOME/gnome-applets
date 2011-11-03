/*
 * Copyright (C) 2008 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as 
 * published by the Free Software Foundation.
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
 *
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <stdlib.h>

#include <glib.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include <libwnck/libwnck.h>

#include <panel-applet.h>
#include <gconf/gconf-client.h>

#include "task-list.h"
#include "task-title.h"

#define SHOW_WIN_KEY "show_all_windows"
#define WNCK_MENU_UI_DIR "data/menu.xml"

typedef struct {
    GtkWidget    *tasks;
    GtkWidget    *applet;
    GtkWidget    *title;
} WinPickerApp;

static WinPickerApp *mainapp;

#if GTK_CHECK_VERSION(2,0,0)
static void cw_panel_background_changed (
    PanelApplet               *applet,
    PanelAppletBackgroundType  type,
	GdkColor                  *colour,
	GdkPixmap                 *pixmap,
    gpointer                   user_data
);

static void display_about_dialog (
    BonoboUIComponent *component, 
    gpointer           user_data, 
    const gchar       *verb
);

static void display_prefs_dialog (
    BonoboUIComponent *component,
    gpointer           user_data, 
    const gchar       *verb
);
#elif GTK_CHECK_VERSION(3,0,0)
static void display_about_dialog (
    GtkAction *component, 
    gpointer           user_data
);

static void display_prefs_dialog (
    GtkAction *component,
    gpointer           user_data
);
#endif

#if GTK_CHECK_VERSION(2,0,0)
static const char Context_menu_xml [] =
   "<popup name=\"button3\">\n"
   "  <menuitem name=\"Preferences Item\" "
   "             verb=\"MenuPrefs\" "
   "           _label=\"_Preferences\"\n"
   "          pixtype=\"stock\" "
   "          pixname=\"gtk-preferences\"/>\n"
   "  <menuitem name=\"About Item\" "
   "             verb=\"MenuAbout\" "
   "           _label=\"_About\"\n"
   "          pixtype=\"stock\" "
   "          pixname=\"gtk-about\"/>\n"
   "</popup>\n";

static const BonoboUIVerb _menu_verbs [] = {
    BONOBO_UI_VERB ("MenuPrefs", display_prefs_dialog),
    BONOBO_UI_VERB ("MenuAbout", display_about_dialog),
    BONOBO_UI_VERB_END
};
#elif GTK_CHECK_VERSION(3,0,0)
static const GtkActionEntry _menu_verbs [] = {
    {"MenuPref", GTK_STOCK_PREFERENCES, N_("_Preferences"),
        NULL, NULL,
        G_CALLBACK (display_prefs_dialog) },
    { "MenuAbout", GTK_STOCK_ABOUT, N_("_About"),
        NULL, NULL,
      G_CALLBACK (display_about_dialog) }
};
#endif

static const gchar *close_window_authors [] = {
	"Neil J. Patel <neil.patel@canonical.com>",
    "Lanoxx <lanoxx@gmx.net",
	NULL
};

static void on_show_all_windows_changed (
    GConfClient *client,
    guint        conn_id,
    GConfEntry  *entry,
    gpointer     data)
{
    WinPickerApp *app;
    gboolean show_windows = TRUE;
    app = (WinPickerApp*)data;
    show_windows = panel_applet_gconf_get_bool (
        PANEL_APPLET (app->applet),
        SHOW_WIN_KEY, NULL
    );
    g_object_set (app->tasks, "show_all_windows", show_windows, NULL);
}

static inline void force_no_focus_padding (GtkWidget *widget) {
  static gboolean first_time = TRUE;
  if (first_time) {
        gtk_rc_parse_string ("\n"
            " style \"na-tray-style\"\n"
            " {\n"
            " GtkWidget::focus-line-width=0\n"
            " GtkWidget::focus-padding=0\n"
            " }\n"
            "\n"
            " widget \"*.na-tray\" style \"na-tray-style\"\n"
            "\n"
        );
        first_time = FALSE;
    }
    gtk_widget_set_name (widget, "na-tray");
}

static gboolean cw_applet_fill (PanelApplet *applet, 
    const gchar *iid, 
    gpointer     data)
{
    WnckScreen *screen;
    WinPickerApp *app;
    GtkWidget *eb, *tasks, *title;
    GError *error = NULL;
    gchar *key;
    #if GTK_CHECK_VERSION(2,0,0)
    if (strcmp (iid, "OAFIID:GNOME_WindowPicker") != 0)
        return FALSE;
    #elif GTK_CHECK_VERSION(3,0,0)
    if (strcmp (iid, "WindowPicker") != 0)
        return FALSE;
    #endif
    bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    textdomain (GETTEXT_PACKAGE);
    wnck_set_client_type (WNCK_CLIENT_TYPE_PAGER);  
    app = g_slice_new0 (WinPickerApp);
    mainapp = app;
    screen = wnck_screen_get_default ();
    #if GTK_CHECK_VERSION(2,0,0)
        /* Gconf prefs */
        panel_applet_add_preferences (applet, 
            "/schemas/apps/window-picker-applet/prefs",
            &error
        );
        if (error) {
            g_warning ("%s", error->message);
            g_error_free (error);
        }
        key = panel_applet_gconf_get_full_key (applet, SHOW_WIN_KEY);
        gconf_client_notify_add (
            gconf_client_get_default (), key,
            on_show_all_windows_changed, app,
            NULL, NULL
       );
        g_free (key);
    #elif GTK_CHECK_VERSION(3,0,0)
    GSettings* settings = panel_applet_settings_new(
        PANEL_APPLET(applet), 
        "/schemas/apps/window-picker-applet/prefs"
    );
    #endif
    app->applet = GTK_WIDGET (applet);
    force_no_focus_padding (GTK_WIDGET (applet));
    gtk_container_set_border_width (GTK_CONTAINER (applet), 0);
    eb = gtk_hbox_new (FALSE, 6);
	gtk_container_add (GTK_CONTAINER (applet), eb);
    gtk_container_set_border_width (GTK_CONTAINER (eb), 0);
    tasks = app->tasks = task_list_get_default ();
    gtk_box_pack_start (GTK_BOX (eb), tasks, FALSE, FALSE, 0);
    title = app->title = task_title_new ();
    gtk_box_pack_start (GTK_BOX (eb), title, TRUE, TRUE, 0);
    gtk_widget_show_all (GTK_WIDGET (applet));
    on_show_all_windows_changed (NULL, 0, NULL, app);
            
    #if GTK_CHECK_VERSION(2,0,0)
        /* Signals */
        g_signal_connect (applet, "change-background",
            G_CALLBACK (cw_panel_background_changed), NULL);
        panel_applet_setup_menu (PANEL_APPLET (applet),
            Context_menu_xml,
            _menu_verbs,
            NULL
        );    
    #elif GTK_CHECK_VERSION(3,0,0)
        /* Signals */
        g_signal_connect (applet, "change-background",
            G_CALLBACK (update_panel_background), NULL);
        GtkActionGroup* action_group = gtk_action_group_new ("Window Picker Applet Actions");
        gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);
        gtk_action_group_add_actions (action_group,
            _menu_verbs,
            G_N_ELEMENTS (_menu_verbs),
            NULL); //we are not passing any data to the callbacks
        char* ui_path = g_build_filename (WNCK_MENU_UI_DIR, "menu.xml", NULL);
        panel_applet_setup_menu_from_file (PANEL_APPLET (applet),
            ui_path, action_group);
        g_free (ui_path);
        g_object_unref (action_group);
    #endif

    panel_applet_set_flags (PANEL_APPLET (applet), 
        PANEL_APPLET_EXPAND_MAJOR
        | PANEL_APPLET_EXPAND_MINOR
        | PANEL_APPLET_HAS_HANDLE
    );
	return TRUE;
}

#if GTK_CHECK_VERSION(2,0,0)
PANEL_APPLET_BONOBO_FACTORY (
    "OAFIID:GNOME_WindowPicker_Factory",
    PANEL_TYPE_APPLET,
    "Show Desktop",
    "0",
    cw_applet_fill,
    NULL
);
#elif GTK_CHECK_VERSION(3,0,0)
PANEL_APPLET_OUT_PROCESS_FACTORY (
    "WindowPickerFactory",
    PANEL_TYPE_APPLET,
    cw_applet_fill,
    NULL
);
#endif

#if GTK_CHECK_VERSION(2,0,0)
static void cw_panel_background_changed (
    PanelApplet               *applet,
    PanelAppletBackgroundType  type,
    GdkColor                  *colour,
    GdkPixmap                 *pixmap,
    gpointer                   user_data)
{
    GtkRcStyle *rc_style;
    GtkStyle *style;
    /* reset style */
    gtk_widget_set_style (GTK_WIDGET (applet), NULL);
    rc_style = gtk_rc_style_new ();
    gtk_widget_modify_style (GTK_WIDGET (applet), rc_style);
    g_object_unref (rc_style);
    gtk_widget_set_style (mainapp->title, NULL);
    rc_style = gtk_rc_style_new ();
    gtk_widget_modify_style (mainapp->title, rc_style);
    g_object_unref (rc_style);
    switch (type) {
        case PANEL_NO_BACKGROUND: break;
        case PANEL_COLOR_BACKGROUND:
            gtk_widget_modify_bg (GTK_WIDGET (applet), GTK_STATE_NORMAL, colour);
            gtk_widget_modify_bg (mainapp->title, GTK_STATE_NORMAL, colour); 
            break;
        case PANEL_PIXMAP_BACKGROUND:
            style = gtk_style_copy (GTK_WIDGET (applet)->style);
            if (style->bg_pixmap[GTK_STATE_NORMAL])
                g_object_unref (style->bg_pixmap[GTK_STATE_NORMAL]);
            style->bg_pixmap[GTK_STATE_NORMAL] = g_object_ref (pixmap);
            gtk_widget_set_style (GTK_WIDGET (applet), style);
            g_object_unref (style);
            break;
    }
}
#elif GTK_CHECK_VERSION(3,0,0)
static void update_panel_background (
    PanelApplet  *applet,
    CairoPattern *pattern,
    gpointer      user_data)
{
    //do stuff to update the background...   
    GtkStyleContext* context = gtk_widget_get_style_context (GTK_WIDGET (applet));
    cairo_pattern_t* pattern = cairo_get_source(cr);
    GtkAllocation allocation;
    gtk_widget_get_allocation(GTK_WIDGET(applet), &allocation);
    int height = gtk_widget_get_allocated_height();
    int width = gtk_widget_get_allocated_width();
    gtk_render_background(
        context,
        cr,
        0, 0,
        width, heighgt);
}
#endif

#if GTK_CHECK_VERSION(2,0,0)
static void display_about_dialog (
    BonoboUIComponent *component, 
    gpointer           user_data, 
    const gchar       *verb)    
#elif GTK_CHECK_VERSION(3,0,0)
static void display_about_dialog (
    GtkAction* action,
    gpointer user_data)
#endif
{
    GtkWidget *panel_about_dialog = gtk_about_dialog_new ();
    g_object_set (panel_about_dialog,
        "name", _("Window Picker"),
        "comments", _("Window Picker"),
        "version", PACKAGE_VERSION,
        "authors", close_window_authors,
        "logo-icon-name", "system-preferences-windows",
        "copyright", "Copyright \xc2\xa9 2008 Canonical Ltd\nand Sebastian",
        NULL
    );
    gtk_widget_show (panel_about_dialog);
    g_signal_connect (panel_about_dialog, "response",
        G_CALLBACK (gtk_widget_destroy), NULL);
    gtk_window_present (GTK_WINDOW (panel_about_dialog));
}

static void on_checkbox_toggled (GtkToggleButton *check, gpointer null) {
    gboolean is_active = gtk_toggle_button_get_active (check);
    panel_applet_gconf_set_bool (PANEL_APPLET (mainapp->applet),
        SHOW_WIN_KEY, is_active, NULL);
}

#if GTK_CHECK_VERSION(2,0,0)
static void display_prefs_dialog (BonoboUIComponent *component, 
    gpointer           user_data, 
    const gchar       *verb)
#elif GTK_CHECK_VERSION(3,0,0)
static void display_prefs_dialog(
    GtkAction* action,
    gpointer* user_data)
#endif
{
    GtkWidget *window, *box, *vbox, *nb, *hbox, *label, *check, *button;
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (window), _("Preferences"));
    gtk_window_set_type_hint (GTK_WINDOW (window),
        GDK_WINDOW_TYPE_HINT_DIALOG);
    gtk_container_set_border_width (GTK_CONTAINER (window), 12);
    box = gtk_vbox_new (FALSE, 8);
    gtk_container_add (GTK_CONTAINER (window), box);
    nb = gtk_notebook_new ();
    g_object_set (nb, "show-tabs", FALSE, "show-border", TRUE, NULL);
    gtk_box_pack_start (GTK_BOX (box), nb, TRUE, TRUE, 0);
    vbox = gtk_vbox_new (FALSE, 8);
    gtk_container_set_border_width (GTK_CONTAINER (vbox), 8);
    gtk_notebook_append_page (GTK_NOTEBOOK (nb), vbox, NULL);
    check = gtk_check_button_new_with_label (_("Show windows from all workspaces"));
    gtk_box_pack_start (GTK_BOX (vbox), check, FALSE, TRUE, 0);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check),
        panel_applet_gconf_get_bool (
            PANEL_APPLET (mainapp->applet),
            SHOW_WIN_KEY, NULL)
    );
    g_signal_connect (check, "toggled",
        G_CALLBACK (on_checkbox_toggled), NULL);
    check = gtk_label_new (" ");
    gtk_box_pack_start (GTK_BOX (vbox), check, TRUE, TRUE, 0);
    gtk_widget_set_size_request (nb, -1, 100);
    hbox = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (box), hbox, TRUE, TRUE, 0);  
    label = gtk_label_new (" ");
    gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);
    button = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
    gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
    gtk_widget_show_all (window);
    g_signal_connect (window, "delete-event",
        G_CALLBACK (gtk_widget_destroy), window);
    g_signal_connect (window, "destroy",
        G_CALLBACK (gtk_widget_destroy), window);
    g_signal_connect_swapped (button, "clicked",
        G_CALLBACK (gtk_widget_destroy), window);
    gtk_window_present (GTK_WINDOW (window));
}
