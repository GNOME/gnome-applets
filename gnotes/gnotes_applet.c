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
#include <libgnomeui/gnome-window-icon.h>
#include <signal.h>

#include "gnotes_applet.h"
#include "gnotes.xpm"
#include "gnotes_session.h"
#include "gnote.h"

GNotes gnotes;

GNotes *gnotes_get_main_info(void)
{
    return &gnotes;
}

/* what happens when a button is pressed */
static gint applet_button_press_cb(GtkWidget *widget, GdkEventButton *event,
                                   gpointer data)
{
    switch(event->button)
    {
    case 1:
        gnote_new_cb(APPLET_WIDGET(gnotes.applet), 0);
        break;
    default:
        return(FALSE);
    };
    return(TRUE);
};

static GtkWidget *create_gnotes_button(GNotes *gnotes)
{
    GtkWidget *frame;
    GtkWidget *vbox;
    int size;

    frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
    gtk_widget_show(frame);

    vbox = gtk_vbox_new(FALSE, FALSE);
    gtk_container_add(GTK_CONTAINER(frame), vbox);
    gtk_widget_show(vbox);

    gnotes->button = gtk_button_new();
    GTK_WIDGET_UNSET_FLAGS(gnotes->button, GTK_CAN_DEFAULT);
    GTK_WIDGET_UNSET_FLAGS(gnotes->button, GTK_CAN_FOCUS);
    size = applet_widget_get_panel_pixel_size(APPLET_WIDGET(gnotes->applet));
    gnotes->pixmap = gnome_pixmap_new_from_xpm_d_at_size(gnotes_xpm, 
							 size-8, size-8);
    gtk_box_pack_start(GTK_BOX(vbox), gnotes->button, FALSE, TRUE, 0);
    gtk_widget_show(gnotes->pixmap);
    gtk_container_add(GTK_CONTAINER(gnotes->button), gnotes->pixmap);
    gtk_widget_show(gnotes->button);

    gtk_signal_connect(GTK_OBJECT(gnotes->button), "button_press_event",
                       GTK_SIGNAL_FUNC(applet_button_press_cb), NULL);

    return(frame);
};

static void about(AppletWidget *applet, gpointer data)
{
    static const char *authors[] = { "spoon <spoon@ix.netcom.com>",
                                     "dres <dres@debian.org", NULL };
    static GtkWidget *about_box = NULL;

    if (about_box != NULL)
    {
    	gdk_window_show(about_box->window);
        gdk_window_raise(about_box->window);
        return;
    }

    about_box = gnome_about_new(_("GNotes!"), VERSION,
                                _("Copyright (C) 1998-1999"),
                                authors,
                                _("Create sticky notes on your screen."), NULL);
    gtk_signal_connect(GTK_OBJECT(about_box), "destroy",
                       GTK_SIGNAL_FUNC(gtk_widget_destroyed), &about_box);

    gtk_widget_show(about_box);
}


static gint applet_save_session(GtkWidget *widget, char *privcfpath,
                                char *globcfpath, gpointer data)
{
    g_debug("Saving session\n");
    gnotes_preferences_save(privcfpath, &gnotes);
    gnotes_save(0, 0);
    return FALSE;
}

static void applet_change_pixel_size(GtkWidget *widget, int size,
				     gpointer data)
{
    GNotes *gnotes = data;
    g_debug("Changing pixel size to: %d\n", size);
    gtk_widget_destroy(gnotes->pixmap);
    gnotes->pixmap = gnome_pixmap_new_from_xpm_d_at_size(gnotes_xpm, 
							 size-8, size-8);
    gtk_widget_show(gnotes->pixmap);
    gtk_container_add(GTK_CONTAINER(gnotes->button), gnotes->pixmap);
}

static void
help_cb (AppletWidget *w, gpointer data)
{
	GnomeHelpMenuEntry help_entry = { "gnotes_applet",
					  "index.html" };
	gnome_help_display(NULL, &help_entry);
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
    gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gnome-gnotes.png");


    smClient = newGnomeClient();        /* initialize session management */

    gnotes.applet = applet_widget_new("gnotes_applet");

    gnotes.defaults.onbottom = FALSE;
    
    if (!gnotes.applet)
    {
        g_error(_("Can't create GNotes applet!"));
        exit(1);
    }

    gnotes_button = create_gnotes_button(&gnotes);

    gtk_signal_connect(GTK_OBJECT(gnotes.applet), "change_pixel_size",
                       GTK_SIGNAL_FUNC(applet_change_pixel_size), &gnotes);
    gtk_signal_connect(GTK_OBJECT(gnotes.applet), "save_session",
                       GTK_SIGNAL_FUNC(applet_save_session), NULL);

    applet_widget_add(APPLET_WIDGET(gnotes.applet), gnotes_button);

    gtk_widget_show(gnotes_button);
    gtk_widget_show(gnotes.applet);


    applet_widget_register_callback(APPLET_WIDGET(gnotes.applet), "raise-notes",
                                    _("Raise Notes"), gnotes_raise, 0);
    applet_widget_register_callback(APPLET_WIDGET(gnotes.applet), "lower-notes",
                                    _("Lower Notes"), gnotes_lower, 0);
    applet_widget_register_callback(APPLET_WIDGET(gnotes.applet), "hide-notes",
                                    _("Hide Notes"), gnotes_hide, 0);
    applet_widget_register_callback(APPLET_WIDGET(gnotes.applet), "show-notes",
                                    _("Show Notes"), gnotes_show, 0);
    applet_widget_register_stock_callback(APPLET_WIDGET(gnotes.applet),
                                          "properties", GNOME_STOCK_MENU_PROP,
                                          _("Properties..."), properties_show,
                                          &gnotes);
    applet_widget_register_stock_callback(APPLET_WIDGET(gnotes.applet),
                                          "help", GNOME_STOCK_PIXMAP_HELP,
                                          _("Help"), help_cb, NULL);
    applet_widget_register_stock_callback(APPLET_WIDGET(gnotes.applet),
                                          "about", GNOME_STOCK_MENU_ABOUT,
                                          _("About..."), about, NULL);

    /* let's load us some notes! */
    gnotes_load(0, 0);

    /* load up the preferences */
    gnotes_preferences_load(APPLET_WIDGET(gnotes.applet)->privcfgpath,
                            &gnotes);
    
    applet_widget_gtk_main ();

    return(0);
};

