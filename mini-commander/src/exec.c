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
#include "egg-screen-exec.h"

#include "exec.h"
#include "macro.h"
#include "preferences.h"
#include "egg-screen-exec.h"

void
mc_exec_command (MCData     *mc,
		 const char *cmd)
{
	char command [1000];

	strncpy (command, cmd, sizeof (command));
	command [sizeof (command) - 1] = '\0';

	mc_macro_expand_command (mc, command);

#ifdef HAVE_GTK_MULTIHEAD
	egg_screen_execute_shell (
			gtk_widget_get_screen (GTK_WIDGET (mc->applet)),
			g_get_home_dir (), command);
#else
	gnome_execute_shell (g_get_home_dir (), command);
#endif
}
