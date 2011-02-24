/* -*- mode: C; c-basic-offset: 4 -*-
 * Null Applet - The Applet Deprecation Applet
 * Copyright (c) 2004, Davyd Madeley
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author:
 *   Davyd Madeley <davyd@madeley.id.au>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>
#include <gconf/gconf-client.h>
#include "null_applet.h"

G_DEFINE_TYPE(NullApplet, null_applet, PANEL_TYPE_APPLET)

static void
null_applet_class_init (NullAppletClass *klass)
{
}

static void
null_applet_init (NullApplet *applet)
{
}

static inline void
insert_oafiids (GHashTable *hash_table)
{
	/*
	 * Add OAFIID's and descriptions of deprecated applets here
	 */
	g_hash_table_insert (hash_table,
			     "OAFIID:GNOME_MailcheckApplet", _("Inbox Monitor"));
	g_hash_table_insert (hash_table,
			     "OAFIID:GNOME_CDPlayerApplet", _("CD Player"));
	g_hash_table_insert (hash_table,
			     "OAFIID:GNOME_MixerApplet_Factory", _("Volume Control"));
	g_hash_table_insert (hash_table,
			     "OAFIID:GNOME_MixerApplet", _("Volume Control"));
	g_hash_table_insert (hash_table,
			     "OAFIID:GNOME_KeyboardApplet", _("Keyboard Indicator"));
}

static void
response_cb (GtkWidget *dialog, gint arg1, gpointer user_data)
{
	gtk_widget_destroy (dialog);
}

static char
*get_all_applets (void)
{
	GConfClient *client;
	GError *error;
	GSList *list, *l;
	char *key, *oafiid, *name;
	GHashTable *hash_table;
	GString *string;

	error = NULL;
	hash_table = g_hash_table_new (g_str_hash, g_str_equal);
	insert_oafiids (hash_table);

	string = g_string_new ("");

	client = gconf_client_get_default ();

	gconf_client_suggest_sync (client, NULL);
	
	list = gconf_client_all_dirs (client,
		"/apps/panel/applets",
		&error);

	if (error)
	{
		g_warning ("Error: %s", error->message);
		g_error_free (error);
		error = NULL;
	}

	for (l = list; l; l = l->next)
	{
	    key = g_strdup_printf ("%s/bonobo_iid", (gchar *)l->data);
		oafiid = gconf_client_get_string (client, key, &error);
		if (error)
		{
			g_warning ("Error: %s", error->message);
			g_error_free (error);
			error = NULL;
		}
		g_free (key);
	
		if (oafiid)
		{
			name = g_hash_table_lookup (hash_table, oafiid);
			if (name)
			{
				gconf_client_recursive_unset (client, l->data,
					GCONF_UNSET_INCLUDING_SCHEMA_NAMES,
					&error);
				if (error)
				{
					g_warning ("Error: %s", error->message);
					g_error_free (error);
					error = NULL;
				}
				g_string_append_printf (string,
						"    • %s\n", name);
			}
			g_free (oafiid);
		}
		g_free (l->data);
	}

	g_slist_free (list);
	g_hash_table_destroy (hash_table);
	
	return g_string_free (string, FALSE);
}

static gboolean
applet_factory (PanelApplet *applet,
		const char  *iid,
		gpointer     user_data)
{
	char *applet_list;
	GtkWidget *dialog;

	if (!g_str_equal (iid, "NullApplet"))
	    return FALSE;

	applet_list = get_all_applets ();

	dialog = gtk_message_dialog_new_with_markup (NULL,
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
			"<span size=\"large\" weight=\"bold\">%s</span>"
			"\n\n%s\n\n%s\n%s\n%s",
			_("Some panel items are no longer available"),
			_("One or more panel items (also referred to as applets"
			  ") are no longer available in the GNOME desktop."),
			_("These items will now be removed from your "
			  "configuration:"),
			applet_list,
			_("You will not receive this message again.")
			);

	g_free (applet_list);

	g_signal_connect (G_OBJECT (dialog), "response",
			G_CALLBACK (response_cb), applet);
	
	gtk_widget_show_all (dialog);
		
	return TRUE;
}

PANEL_APPLET_OUT_PROCESS_FACTORY ("NullApplet",
				  TYPE_NULL_APPLET,
				  applet_factory,
				  NULL)
