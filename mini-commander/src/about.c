/*
 * Mini-Commander Applet
 * Copyright (C) 1998 Oliver Maruhn <om@linuxhq.com>
 *
 * Author: Oliver Maruhn <om@linuxhq.com>
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


void aboutBox(AppletWidget *applet, gpointer data)
{
        GtkWidget *aboutBox;
	const gchar *authors[] = {(gchar *) "Oliver Maruhn <om@linuxhq.com>", (gchar *) NULL};

        aboutBox = gnome_about_new (_("Mini-Commander Applet"), INTERNAL_VERSION,
				    "(C) 1998 Oliver Maruhn",
				    authors,
("This GNOME applet adds a command line to the panel. It features command completion, command history, changeable macros and an optional built-in clock.\n\n\
This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version."),
				    (gchar *) NULL);
        gtk_widget_show (aboutBox);
}



