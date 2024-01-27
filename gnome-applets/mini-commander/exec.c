/*
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
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include "exec.h"

#include <string.h>

#include <glib.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include "macro.h"
#include "preferences.h"
#include "history.h"
#include "gsettings.h"

static void beep (void);

void
mc_exec_command (MCData     *mc,
		 const char *cmd)
{
	GError *error = NULL;
	char command [1000];
	char **argv = NULL;
	gchar *str;

	strncpy (command, cmd, sizeof (command));
	command [sizeof (command) - 1] = '\0';

	mc_macro_expand_command (mc, command);

	if (!g_shell_parse_argv (command, NULL, &argv, &error)) {
		if (error != NULL) {
			g_error_free (error);
			error = NULL;
		}

		return;
	}

	if(!g_spawn_async (NULL, /* working directory */
	                   argv,
	                   NULL, /* envp */
	                   G_SPAWN_SEARCH_PATH,
	                   NULL,
	                   NULL,
	                   NULL,
	                   &error)) {
		str = g_strconcat ("(?)", command, NULL);
		gtk_entry_set_text (GTK_ENTRY (mc->entry), str);
		//gtk_editable_set_position (GTK_EDITABLE (mc->entry), 0);
		mc->error = TRUE;
		beep ();
		g_free (str);
	} else {
		gtk_entry_set_text (GTK_ENTRY (mc->entry), (gchar *) "");
		append_history_entry (mc, cmd, FALSE);
		}
	g_strfreev (argv);

	if (error != NULL)
		g_error_free (error);
}

static void beep (void)
{
	GSettings *settings = g_settings_new (GNOME_DESKTOP_WM_PREFERENCES); 
	gboolean audible_bell = g_settings_get_boolean (settings, KEY_AUDIBLE_BELL);

	if (audible_bell)
		gdk_beep ();
	
	g_object_unref (settings);
}
