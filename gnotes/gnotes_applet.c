/*
#
#   GNotes!
#   Copyright (C) 1998-1999 spoon <spoon@ix.netcom.com>
#   Copyright (C) 1999 dres <dres@debian.org>
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <config.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <gnome.h>
#include <applet-widget.h>
#include <signal.h>

#include "gnote.h"
#include "gnotes.xpm"
#include "gnotes_session.h"


GNotes gnotes;

GtkWidget *propwindow;
gint tmp_default_height;
gint tmp_default_width;

#define DEFAULT_HEIGHT (1)
#define DEFAULT_WIDTH (1)

static void gnotes_preferences_load(const char *path)
{
    gboolean is_default;

    gnome_config_push_prefix(path);

    gnotes.default_height =
        gnome_config_get_int_with_default("height=", &is_default);
    if (is_default)
        gnotes.default_height = DEFAULT_HEIGHT;

    gnotes.default_width =
        gnome_config_get_int_with_default("width=", &is_default);
    if (is_default)
        gnotes.default_width = DEFAULT_WIDTH;

    gnome_config_pop_prefix();
};

static void gnotes_preferences_save(const char *path)
{
    gnome_config_push_prefix(path);

    gnome_config_set_int("height", gnotes.default_height); 
    gnome_config_set_int("width", gnotes.default_width); 
    gnome_config_sync();
    gnome_config_drop_all();
    gnome_config_pop_prefix();
};


/* what happens when a button is pressed */
static gint applet_button_press_cb(GtkWidget *widget, GdkEventButton *event,
                                   gpointer data)
{
    switch(event->button)
    {
    case 1:
        gnote_new_cb(APPLET_WIDGET(gnotes.applet), GNOTE_NEW_1x1);
        break;
    default:
        return(FALSE);
    };
    return(TRUE);
};

static GtkWidget *create_gnotes_button(void)
{
    GtkWidget *frame;
    GtkWidget *vbox;
    GtkWidget *button;
    GtkWidget *pixmap;

    frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
    gtk_widget_show(frame);

    vbox = gtk_vbox_new(FALSE, FALSE);
    gtk_container_add(GTK_CONTAINER(frame), vbox);
    gtk_widget_show(vbox);

    button = gtk_button_new();
    GTK_WIDGET_UNSET_FLAGS(button, GTK_CAN_DEFAULT);
    GTK_WIDGET_UNSET_FLAGS(button, GTK_CAN_FOCUS);
    pixmap = gnome_pixmap_new_from_xpm_d(gnotes_xpm);
    gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, TRUE, 0);
    gtk_widget_show(pixmap);
    gtk_container_add(GTK_CONTAINER(button), pixmap);
    gtk_widget_show(button);

    gtk_signal_connect(GTK_OBJECT(button), "button_press_event",
                       GTK_SIGNAL_FUNC(applet_button_press_cb), NULL);

    return(frame);
};

static void about(AppletWidget *applet, gpointer data)
{
    static const char *authors[] = { "spoon <spoon@ix.netcom.com>",
                                     "dres <dres@debian.org", NULL };
    GtkWidget *about_box;

    about_box = gnome_about_new(_("GNotes!"), VERSION,
                                _("Copyright (C) 1998-1999 spoon <spoon@ix.netcom.com>, "
                                  "Copyright (C) 1999 dres <dres@debian.org>"),
                                authors,
                                _("Create sticky notes on your screen."), NULL);

    gtk_widget_show(about_box);
}

static void update_height_cb(GtkWidget *wid, GtkWidget *data)
{
    tmp_default_height =
        gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(data));
    gnome_property_box_changed(GNOME_PROPERTY_BOX(propwindow));
}

static void update_width_cb(GtkWidget *wid, GtkWidget *data)
{
    tmp_default_width =
        gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(data));
    gnome_property_box_changed(GNOME_PROPERTY_BOX(propwindow));
}

static void property_apply_cb(GtkWidget *wid, gpointer data)
{
    gnotes.default_height = tmp_default_height;
    gnotes.default_width = tmp_default_width;

    applet_widget_sync_config(APPLET_WIDGET(gnotes.applet));
}

static gint property_destroy_cb(GtkWidget *wid, gpointer data)
{
    propwindow = NULL;
    return FALSE;
}

static void properties_show(AppletWidget *applet, gpointer data)
{
    static GnomeHelpMenuEntry help_entry = { NULL, "properties" };
    
    GtkWidget *frame;
    GtkWidget *height_box;
    GtkWidget *width_box;
    GtkWidget *height_spinner;
    GtkWidget *width_spinner;
    GtkObject *height_adj;
    GtkObject *width_adj;
    GtkWidget *tmp_label;
    
    if(propwindow != 0)
    {
        gdk_window_raise(propwindow->window);
        return;
    }

    help_entry.name = gnome_app_id;
    
    tmp_default_height = gnotes.default_height;
    tmp_default_width = gnotes.default_width;

    propwindow = gnome_property_box_new();

    gtk_window_set_title(
        GTK_WINDOW(&GNOME_PROPERTY_BOX(propwindow)->dialog.window),
        _("GNotes Settings"));

    frame = gtk_vbox_new(FALSE, 5);

    /* HEIGHT PART */
    height_box = gtk_hbox_new(FALSE, 5);
    gtk_box_pack_start(GTK_BOX(frame), height_box, FALSE, FALSE, 5);
    gtk_widget_show(height_box);

    tmp_label = gtk_label_new(_("Default Height"));
    gtk_box_pack_start(GTK_BOX(height_box), tmp_label, FALSE, FALSE, 5);
    gtk_widget_show(tmp_label);
    
    height_adj = gtk_adjustment_new(tmp_default_height, 10.0, 500.0, 1, 1, 1);
    
    height_spinner = gtk_spin_button_new(GTK_ADJUSTMENT(height_adj), 1, 0);
    gtk_box_pack_start(GTK_BOX(height_box), height_spinner, FALSE, FALSE, 5);
    gtk_widget_show(height_spinner);

    gtk_signal_connect(GTK_OBJECT(height_adj), "value_changed",
                       GTK_SIGNAL_FUNC(update_height_cb), height_spinner);
    gtk_signal_connect(GTK_OBJECT(height_spinner), "changed",
                       GTK_SIGNAL_FUNC(update_height_cb), height_spinner);

    /* WIDTH PART */
    width_box = gtk_hbox_new(FALSE, 5);
    gtk_box_pack_start(GTK_BOX(frame), width_box, FALSE, FALSE, 5);
    gtk_widget_show(width_box);
    
    tmp_label = gtk_label_new(_("Default Width"));
    gtk_box_pack_start(GTK_BOX(width_box), tmp_label, FALSE, FALSE, 5);
    gtk_widget_show(tmp_label);

    width_adj = gtk_adjustment_new(tmp_default_width, 10.0, 500.0, 1, 1, 1);
    
    width_spinner = gtk_spin_button_new(GTK_ADJUSTMENT(width_adj), 1, 0);
    gtk_box_pack_start(GTK_BOX(width_box), width_spinner, FALSE, FALSE, 5);
    gtk_widget_show(width_spinner);

    gtk_signal_connect(GTK_OBJECT(width_adj), "value_changed",
                       GTK_SIGNAL_FUNC(update_width_cb), width_spinner);
    gtk_signal_connect(GTK_OBJECT(width_spinner), "changed",
                       GTK_SIGNAL_FUNC(update_width_cb), width_spinner);


    tmp_label = gtk_label_new(_("General"));
    gtk_widget_show(frame);
    gnome_property_box_append_page(GNOME_PROPERTY_BOX(propwindow), frame,
                                   tmp_label);

    gtk_signal_connect(GTK_OBJECT(propwindow), "apply",
                       GTK_SIGNAL_FUNC(property_apply_cb), NULL);
    gtk_signal_connect(GTK_OBJECT(propwindow), "destroy",
                       GTK_SIGNAL_FUNC(property_destroy_cb), NULL);
    gtk_signal_connect( GTK_OBJECT(propwindow), "help",
                        GTK_SIGNAL_FUNC(gnome_help_pbox_display),
                        &help_entry );

    gtk_widget_show(propwindow);
    
}

static gint applet_save_session(GtkWidget *widget, char *privcfpath,
                                char *globcfpath)
{
    g_debug("Saving session\n");
    gnotes_preferences_save(privcfpath);
    gnotes_save(0, 0);
    return FALSE;
}


int main(int argc, char **argv)
{
    GtkWidget *gnotes_button;
    GnomeClient *smClient;

    /* set up the GNotes directory */
    umask(0077);
    mkdir(get_gnotes_dir(), 0700);
    chmod(get_gnotes_dir(), 0700);
    chdir(get_gnotes_dir());
    
    bindtextdomain(PACKAGE, GNOMELOCALEDIR);
    textdomain(PACKAGE);

    /* initialize the gnotes structures */
    gnotes_init();

    applet_widget_init("gnotes_applet", VERSION, argc, argv, NULL, 0, NULL);

    smClient = newGnomeClient();        /* initialize session management */

    gnotes.applet = applet_widget_new("gnotes_applet");

    if (!gnotes.applet)
    {
        g_error(_("Can't create GNotes applet!"));
        exit(1);
    }

    gnotes_button = create_gnotes_button();
    applet_widget_add(APPLET_WIDGET(gnotes.applet), gnotes_button);

    gtk_widget_show(gnotes_button);
    gtk_widget_show(gnotes.applet);

    applet_widget_register_stock_callback(APPLET_WIDGET(gnotes.applet),
                                          "about", GNOME_STOCK_MENU_ABOUT,
                                          _("About..."), about, NULL);
/*     applet_widget_register_stock_callback(APPLET_WIDGET(gnotes.applet), */
/*                                           "properties", GNOME_STOCK_MENU_PROP, */
/*                                           _("Properties..."), properties_show, */
/*                                           NULL); */

    /*
      applet_widget_register_stock_callback(APPLET_WIDGET(gnotes.applet),
      GNOTE_NEW_4x5, GNOME_STOCK_PIXMAP_NEW, _(GNOTE_NEW_4x5), gnote_new_cb,
      GNOTE_NEW_4x5);
      applet_widget_register_callback(APPLET_WIDGET(gnotes.applet), GNOTE_NEW_4x4,
      _(GNOTE_NEW_4x4), gnote_new_cb, GNOTE_NEW_4x4);
      applet_widget_register_callback(APPLET_WIDGET(gnotes.applet), GNOTE_NEW_3x4,
      _(GNOTE_NEW_3x4), gnote_new_cb, GNOTE_NEW_3x4);
      applet_widget_register_callback(APPLET_WIDGET(gnotes.applet), GNOTE_NEW_3x3,
      _(GNOTE_NEW_3x3), gnote_new_cb, GNOTE_NEW_3x3);
      applet_widget_register_callback(APPLET_WIDGET(gnotes.applet), GNOTE_NEW_2x3,
      _(GNOTE_NEW_2x3), gnote_new_cb, GNOTE_NEW_2x3);
      applet_widget_register_callback(APPLET_WIDGET(gnotes.applet), GNOTE_NEW_2x2,
      _(GNOTE_NEW_2x2), gnote_new_cb, GNOTE_NEW_2x2);
      applet_widget_register_callback(APPLET_WIDGET(gnotes.applet), GNOTE_NEW_1x2,
      _(GNOTE_NEW_1x2), gnote_new_cb, GNOTE_NEW_1x2);
      applet_widget_register_callback(APPLET_WIDGET(gnotes.applet), GNOTE_NEW_1x1,
      _(GNOTE_NEW_1x1), gnote_new_cb, GNOTE_NEW_1x1);
    */
    applet_widget_register_callback(APPLET_WIDGET(gnotes.applet), GNOTES_RAISE,
                                    _(GNOTES_RAISE), gnotes_raise, 0);
    applet_widget_register_callback(APPLET_WIDGET(gnotes.applet), GNOTES_LOWER,
                                    _(GNOTES_LOWER), gnotes_lower, 0);
    applet_widget_register_callback(APPLET_WIDGET(gnotes.applet), GNOTES_HIDE,
                                    _(GNOTES_HIDE), gnotes_hide, 0);
    applet_widget_register_callback(APPLET_WIDGET(gnotes.applet), GNOTES_SHOW,
                                    _(GNOTES_SHOW), gnotes_show, 0);
    /*
      applet_widget_register_stock_callback(APPLET_WIDGET(gnotes.applet),
      GNOTES_SAVE, GNOME_STOCK_PIXMAP_SAVE, _(GNOTES_SAVE), 
      GTK_SIGNAL_FUNC(gnotes_preferences_save_cb), NULL);
    */

    /* let's load us some notes! */
    gnotes_load(0, 0);

    /* load up the preferences */
    gnotes_preferences_load(APPLET_WIDGET(gnotes.applet)->privcfgpath);
    
    gtk_signal_connect(GTK_OBJECT(gnotes.applet), "save_session",
                       GTK_SIGNAL_FUNC(applet_save_session), NULL);

    /* wait for a signal! */
    signal(SIGHUP, gnote_signal_handler);

    applet_widget_gtk_main ();

    return(0);
};

