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
                                _("Copyright (C) 1998-1999 spoon <spoon@ix.netcom.com> \nCopyright (C) 1999 dres <dres@debian.org>"),
                                authors,
                                _("Create sticky notes on your screen."), NULL);

    gtk_widget_show(about_box);
}


static gint applet_save_session(GtkWidget *widget, char *privcfpath,
                                char *globcfpath)
{
    g_debug("Saving session\n");
    gnotes_preferences_save(privcfpath, &gnotes);
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

    gnotes.defaults.onbottom = FALSE;
    
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
    applet_widget_register_stock_callback(APPLET_WIDGET(gnotes.applet),
                                          "properties", GNOME_STOCK_MENU_PROP,
                                          _("Properties..."), properties_show,
                                          &gnotes);

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
    gnotes_preferences_load(APPLET_WIDGET(gnotes.applet)->privcfgpath,
                            &gnotes);
    
    gtk_signal_connect(GTK_OBJECT(gnotes.applet), "save_session",
                       GTK_SIGNAL_FUNC(applet_save_session),
                       APPLET_WIDGET(gnotes.applet)->privcfgpath);
    gtk_signal_connect(GTK_OBJECT(gnotes.applet), "destroy",
                       GTK_SIGNAL_FUNC(applet_save_session),
                       APPLET_WIDGET(gnotes.applet)->privcfgpath);

    applet_widget_gtk_main ();

    return(0);
};

