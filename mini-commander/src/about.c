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
#include "about.h"
#include "mini-commander_applet.h"


void about_box(AppletWidget *applet, gpointer data)
{
        static GtkWidget *about_box = NULL;
	const gchar *authors[] = {(gchar *) "Oliver Maruhn <oliver@maruhn.com>", (gchar *) NULL};

	if (about_box != NULL)
	{
		gdk_window_show(about_box->window);
		gdk_window_raise(about_box->window);
		return;
	}
        about_box = gnome_about_new (_("Mini-Commander Applet"), 
				    VERSION,
				    "(C) 1998-2000 Oliver Maruhn",
				    authors,
_("This GNOME applet adds a command line to the panel. It features command completion, command history, changeable macros and an optional built-in clock.\n\n\
This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version."),
				    (gchar *) NULL);
	gtk_signal_connect( GTK_OBJECT(about_box), "destroy",
			    GTK_SIGNAL_FUNC(gtk_widget_destroyed), &about_box );
        gtk_widget_show (about_box);
	return;
	applet = NULL;
	data = NULL;
}
