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

#include "gweather.h"
#include "gweather-about.h"


void gweather_about_run (GWeatherApplet *gw_applet)
{
    GdkPixbuf   *pixbuf;
    GError	*error = NULL;
    gchar	*file;
    
    static const gchar *authors[] = {
	"Kevin Vandersloot <kfv101@psu.edu>",
        "Spiros Papadimitriou <spapadim+@cs.cmu.edu>",
        "Todd Kulesza <fflewddur@dropline.net>",
        NULL
    };

    const gchar *documenters[] = {
	NULL
    };

    const gchar *translator_credits = _("translator_credits");

    static GtkWidget *about_dialog = NULL;
    
    if (about_dialog) {
	gtk_window_set_screen (GTK_WINDOW (about_dialog),
			       gtk_widget_get_screen (GTK_WIDGET (gw_applet->applet)));
	gtk_window_present (GTK_WINDOW (about_dialog));
	return;
    }
    
    file = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_PIXMAP, "gweather/tstorm.xpm", FALSE, NULL);
    pixbuf = gdk_pixbuf_new_from_file (file, &error);
    g_free (file);
    
    if (error) {
    	g_warning (G_STRLOC ": cannot open %s: %s", file, error->message);
	g_error_free (error);
    }
    
    about_dialog = gnome_about_new (_("Weather Report"), VERSION,
                                    _("Copyright (c)1999 by S. Papadimitriou"),
                                    _("Released under the GNU General Public License.\n\n"
                                    	"An applet for monitoring local weather conditions."),
                                    authors,
                                    documenters,
                                    strcmp (translator_credits, "translator_credits") != 0 ? translator_credits : NULL,
                                    pixbuf);
    if (pixbuf)
    	gdk_pixbuf_unref (pixbuf);

    gtk_window_set_screen (GTK_WINDOW (about_dialog),
			   gtk_widget_get_screen (GTK_WIDGET (gw_applet->applet)));
    gtk_window_set_wmclass (GTK_WINDOW (about_dialog), "weather report", "Weather Report");	
    gnome_window_icon_set_from_file (GTK_WINDOW (about_dialog), GNOME_ICONDIR"/gweather/tstorm.xpm");	
    
    gtk_signal_connect( GTK_OBJECT(about_dialog), "destroy",
		        GTK_SIGNAL_FUNC(gtk_widget_destroyed), &about_dialog );
    gtk_widget_show(about_dialog);
    
    return;
}
