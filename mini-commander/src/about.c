/*
 * Mini-Commander Applet
 * Copyright (C) 1998, 1999 Oliver Maruhn <oliver@maruhn.com>
 *
 * Author: Oliver Maruhn <oliver@maruhn.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <config.h>
#include <string.h>

#include <libgnomeui/gnome-about.h>

#include "about.h"

void about_box (BonoboUIComponent *uic,
		MCData            *mcdata,
		const char        *verbname)
{
        static GtkWidget *about_box = NULL;
	GdkPixbuf   	 *pixbuf;
  	GError      	 *error     = NULL;
   	gchar       	 *file;
	
	static const gchar *authors[] = {
		"Oliver Maruhn <oliver@maruhn.com>",
		"Mark McLoughlin <mark@skynet.ie>",
		NULL
	};

	const gchar *documenters[] = {
		NULL
	};

	const gchar *translator_credits = _("translator_credits");

	if (about_box) {
		gtk_window_set_screen (GTK_WINDOW (about_box),
				       gtk_widget_get_screen (GTK_WIDGET (mcdata->applet)));
		gtk_window_present (GTK_WINDOW (about_box));
		return;
	}
	
	file = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_PIXMAP, "gnome-mini-commander.png", FALSE, NULL);
   	pixbuf = gdk_pixbuf_new_from_file (file, &error);
   
   	if (error) {
   		g_warning (G_STRLOC ": cannot open %s: %s", file, error->message);
		g_error_free (error);
   	}
   	g_free (file);
   
        about_box = gnome_about_new (_("Command Line"), 
				     VERSION,
				     "(C) 1998-2002 Oliver Maruhn",
_("This GNOME applet adds a command line to the panel. It features command completion, command history, and changeable macros.\n\n\
This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version."),
				     authors,
				     documenters,
				     strcmp (translator_credits, "translator_credits") != 0 ? translator_credits : NULL,
				     pixbuf);
        if (pixbuf) 
   		gdk_pixbuf_unref (pixbuf);

	gtk_window_set_screen (GTK_WINDOW (about_box),
			       gtk_widget_get_screen (GTK_WIDGET (mcdata->applet)));
   	gtk_window_set_wmclass (GTK_WINDOW (about_box), "command line", "Command Line");
   	g_signal_connect (about_box, "destroy",
			  G_CALLBACK (gtk_widget_destroyed),
			  &about_box);
        gtk_widget_show (about_box);
}
