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

void aboutBox(AppletWidget *applet, gpointer data)
{
        GtkWidget *aboutBox;
	const gchar *authors[] = {(gchar *) "Oliver Maruhn <om@linuxhq.com>", (gchar *) NULL};

        aboutBox = gnome_about_new (_("Mini-Commander Applet"), VERSION,
				    "(C) 1998 Oliver Maruhn",
				    authors,
("This GNOME applet adds a command line to the panel. It features command completion, command history, changeable macros and an optional built-in clock.

You can simply start any program by typing its name. You often have not to enter the full name but only the first characters followed by the [tab] key. Mini-Commander will try to complete the program name in the same way most UNIX shells do.

The last commands can be recalled by pressing the [ArrowUp] or [ArrowDown] keys. This works like the command history in most UNIX shells.

Mini-Commander has some predefined macros. For example if you enter \"term:command\" then \"command\" is executed in a terminal window. Or if you enter an URL then your web browser is used to view it. Additionally you can define your own macros or change the predefined ones.

There is a built-in clock which can optionally show the date or can be completely disabled.

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

Any feedback from you is welcome. Latest info and releases are at <http://om.filewatcher.org/mini-commander/>.
 
     Oliver Maruhn <om@linuxhq.com>"),
				    (gchar *) NULL);
        gtk_widget_show (aboutBox);
}



