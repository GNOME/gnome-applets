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

#include <glib.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <gconf/gconf-client.h>

#include "exec.h"
#include "macro.h"
#include "preferences.h"
#include "history.h"

#define KEY_AUDIBLE_BELL "/apps/metacity/general/audible_bell"

static void beep (void);

/*
 * Set the DISPLAY variable, to be use by g_spawn_async.
 */
static void
set_environment (gpointer display)
{
	g_setenv ("DISPLAY", display, TRUE);
}

void
mc_exec_command (MCData     *mc,
		 const char *cmd)
{
	GError *error = NULL;
	char command [1000];
	char **argv = NULL;
	gchar *str;
	gchar *display;

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

	display = gdk_screen_make_display_name (gtk_widget_get_screen (GTK_WIDGET (mc->applet)));

	if(!g_spawn_async (NULL, /* working directory */
	                   argv,
	                   NULL, /* envp */
	                   G_SPAWN_SEARCH_PATH,
	                   set_environment,
	                   &display,
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
	g_free (display);
	g_strfreev (argv);

	if (error != NULL)
		g_error_free (error);
}

static void beep (void)
{
	GConfClient *default_client;
	gboolean audible_bell_set;

	default_client = gconf_client_get_default ();
	audible_bell_set = gconf_client_get_bool (default_client, KEY_AUDIBLE_BELL, NULL);
	if (audible_bell_set) {
		gdk_beep ();
	}
}
