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
	GSettings *gs = g_settings_new(GSETTINGS_METACITY_SCHEMA);
	gchar *retval = g_settings_get_string(gs, CFG_METACITY_BUTTON_LAYOUT);
	g_object_unref(gs);
	return retval;
}

gchar *getMetacityTheme() {
	GSettings *gs = g_settings_new(GSETTINGS_METACITY_SCHEMA);
	gchar *retval = g_settings_get_string(gs, CFG_METACITY_THEME);
	g_object_unref(gs);
	return retval;
}

gboolean issetCompizDecoration() {
	if(!decorPluginInstalled()) return FALSE;

	gboolean retval = FALSE;
	
	GSettings *settings = g_settings_new(GSETTINGS_COMPIZ_SCHEMA);
	gchar *current_profile = g_settings_get_string(settings, CFG_COMPIZ_CURRENT_PROFILE);

	gchar *path = g_strdup_printf(GSETTINGS_COMPIZ_DECOR_PATH, current_profile);

	settings = g_settings_new_with_path(GSETTINGS_COMPIZ_DECOR_SCHEMA, path);
	gchar *cdm = g_settings_get_string(settings, CFG_COMPIZ_DECORATION_MATCH);

	if (cdm == NULL) {
		retval = FALSE;
	} else if (!g_strcmp0(cdm, CFG_COMPIZ_DECORATION_MATCH_VALUE)) {
		retval = TRUE;
	}
	g_free(cdm);
	g_free(path);
	g_object_unref(settings);
	return retval;
}

void toggleCompizDecoration(gboolean new_value) {
	if(!decorPluginInstalled()) return NULL;

	GSettings *settings = g_settings_new(GSETTINGS_COMPIZ_SCHEMA);
	gchar *current_profile = g_settings_get_string(settings, CFG_COMPIZ_CURRENT_PROFILE);

	gchar *path = g_strdup_printf(GSETTINGS_COMPIZ_DECOR_PATH, current_profile);

	settings = g_settings_new_with_path(GSETTINGS_COMPIZ_DECOR_SCHEMA, path);

	if (new_value) {
		g_settings_reset(settings, CFG_COMPIZ_DECORATION_MATCH);
	} else if(!new_value) {
		g_settings_set_string(settings, CFG_COMPIZ_DECORATION_MATCH, CFG_COMPIZ_DECORATION_MATCH_VALUE);
	}

	g_free(path);
	g_object_unref(settings);
}

// Check if Compiz and decor plugin are installed / schema exists
gboolean decorPluginInstalled () {
	return gsettings_schema_exists(GSETTINGS_COMPIZ_SCHEMA)
		&& gsettings_schema_exists(GSETTINGS_COMPIZ_DECOR_SCHEMA);
}

gboolean gsettings_schema_exists (const gchar* schema) {
	GSettingsSchemaSource *schema_source;
	GSettingsSchema *schema_schema;
	gboolean schema_exists;

	schema_source = g_settings_schema_source_get_default();
	schema_schema = g_settings_schema_source_lookup (schema_source, schema, TRUE);
	schema_exists = (schema_schema != NULL);
	if (schema_schema)
		g_settings_schema_unref (schema_schema);

	return schema_exists;
}
