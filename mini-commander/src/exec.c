/*
 * Mini-Commander Applet
 * Copyright (C) 1998 Oliver Maruhn <oliver@maruhn.com>
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

#include <libgnome/gnome-exec.h>

#include "exec.h"
#include "macro.h"
#include "preferences.h"

void
exec_command (const char  *cmd,
	      PanelApplet *applet)
{
	char        command [1000];
	properties *prop;

	strncpy (command, cmd, sizeof (command));
	command [sizeof (command) - 1] = '\0';

	prop = g_object_get_data (G_OBJECT (applet), "prop");

	expand_command (command, prop);

	gnome_execute_shell (g_get_home_dir (), command);
}
