/* $Id$ */

/*
 *  Papadimitriou Spiros <spapadim+@cs.cmu.edu>
 *
 *  This code released under the GNU GPL.
 *  Read the file COPYING for more information.
 *
 *  About box
 *
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <gnome.h>

#include "gweather-about.h"


static GtkWidget *gweather_about_new (void)
{
    const gchar *authors[] = {
        "Spiros Papadimitriou <spapadim+@cs.cmu.edu>",
        NULL
    };
    GtkWidget *about_dialog;
    
    about_dialog = gnome_about_new ("GNOME Weather", "0.05",
                                    _("Copyright (c)1999 by S. Papadimitriou"),
                                    authors,
                                    _("GNOME weather monitor applet.\nWeb: http://gweather.dhs.org/"),
                                    NULL);
    gtk_window_set_modal (GTK_WINDOW (about_dialog), TRUE);
    
    
    return about_dialog;
}


void gweather_about_run (void)
{
    GtkWidget *about = gweather_about_new();

    gtk_widget_show(about);
}
