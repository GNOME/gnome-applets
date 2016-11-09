/***************************************************************************
 *            external.c
 *
 *  Wed Sep  29 01:56:39 2010
 *  Copyright  2010  Andrej Belcijan
 *  <{andrejx} at {gmail.com}>
 ****************************************************************************/

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */

/* 
 * This file is responsible for handling of things that are a part of outside
 * programs (Metacity, Compiz...).
 */

#include "external.h"

gchar *getMetacityLayout(void);
gchar *getMetacityTheme(void);
gboolean issetCompizDecoration(void);
//gboolean issetMetacityDecoration(void); //TODO
void toggleCompizDecoration(gboolean);
//void toggleMetacityDecoration(gboolean); //TODO

gchar *getMetacityLayout() {
	GConfClient *gcc = gconf_client_get_default();
	gchar *retval = gconf_client_get_string(gcc, GCONF_METACITY_BUTTON_LAYOUT, NULL);
	g_object_unref(gcc);
	return retval;
}

gchar *getMetacityTheme() {
	GConfClient *gcc = gconf_client_get_default();
	gchar *retval = gconf_client_get_string(gcc, GCONF_METACITY_THEME, NULL);
	g_object_unref(gcc);
	return retval;
}

gboolean issetCompizDecoration() {
	gboolean retval = FALSE;
	GConfClient *gcc = gconf_client_get_default();
	gchar *gconf_cdm = gconf_client_get_string(gcc, GCONF_COMPIZ_DECORATION_MATCH, NULL);
	
	if (gconf_cdm == NULL) {
		retval = FALSE;
	} else if (!g_strcmp0(gconf_cdm, COMPIZ_DECORATION_MATCH)) {
		retval = TRUE;
	}
	g_free(gconf_cdm);
	g_object_unref(gcc);
	return retval;
}

void toggleCompizDecoration(gboolean new_value) {
	GError *err = NULL;
	GConfClient *gconfclient = gconf_client_get_default();

	if (new_value) {
		if (gconf_client_unset(gconfclient, GCONF_COMPIZ_DECORATION_MATCH, &err))
			gconf_client_unset(gconfclient, GCONF_COMPIZ_DECORATION_MATCH_OLD, &err);
	} else if (gconf_client_get_string(gconfclient, GCONF_COMPIZ_DECORATION_MATCH, &err)) {
					 gconf_client_set_string(gconfclient,
					 GCONF_COMPIZ_DECORATION_MATCH,
					 COMPIZ_DECORATION_MATCH,
					 NULL);
	} else if (gconf_client_get_string(gconfclient, GCONF_COMPIZ_DECORATION_MATCH_OLD, &err)) {
					 gconf_client_set_string(gconfclient,
					 GCONF_COMPIZ_DECORATION_MATCH_OLD,
					 COMPIZ_DECORATION_MATCH,
					 NULL);
	} else {
		// Compiz probably not installed. Unset newly created value.
		// TODO: This doesn't really remove it. I hate GConf.
		toggleCompizDecoration(0);
	}

	g_free(err);
	g_object_unref(gconfclient);
}