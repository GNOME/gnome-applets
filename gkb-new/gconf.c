/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/*
 * Copyright (C) 2002 Szabolcs Ban <shooby@gnome.hu>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gkb.h"

gboolean gconf_applet_set_string (PanelApplet *parent, 
                                  const char *gconf_key,
                                  gchar *value,
                                  gchar *blah) 
{
	static GConfClient *client = NULL;
	gchar *full_key;
	gboolean success;

	if (!value)
		return FALSE;
		
	if (client == NULL) {
		client = gconf_client_get_default ();
	}

	full_key = g_strdup_printf ("/desktop/gnome/peripherals/keyboard/layout/%s",
				   gconf_key);

	success = gconf_client_set_string (client, full_key, value, NULL);
	g_free (full_key);
        return success;                                  
}

gchar * gconf_applet_get_string  (PanelApplet *parent, 
                                  const char *gconf_key,
                                  gchar *blah)
{
	static GConfClient *client = NULL;
	gchar *full_key;
	gchar * value;

	full_key = g_strdup_printf ("/desktop/gnome/peripherals/keyboard/layout/%s",
				    gconf_key);
	
	if (client == NULL) {
		client = gconf_client_get_default ();
	}

	value = gconf_client_get_string (client, full_key, NULL);

	g_free (full_key);

	return value;                                  
}

gboolean gconf_applet_set_int (PanelApplet *parent, 
                                  const char *gconf_key,
                                  gint value,
                                  gchar *blah)
{
	static GConfClient *client = NULL;
	gchar *full_key;
	gboolean success;

	if (client == NULL) {
		client = gconf_client_get_default ();
	}

	full_key = g_strdup_printf ("/desktop/gnome/peripherals/keyboard/layout/%s",
				   gconf_key);

	success = gconf_client_set_int (client, full_key, value, NULL);
	g_free (full_key);
        return success;                   
                                  
}

gint gconf_applet_get_int  (PanelApplet *parent, 
                                  const char *gconf_key,
                                  gchar *blah)
{
	static GConfClient *client = NULL;
	gchar *full_key;
	gint value;

	full_key = g_strdup_printf ("/desktop/gnome/peripherals/keyboard/layout/%s",
				    gconf_key);
	
	if (client == NULL) {
		client = gconf_client_get_default ();
	}

	value = gconf_client_get_int (client, full_key, NULL);

	g_free (full_key);

	return value;
                                  
}

gboolean gconf_applet_set_bool (PanelApplet *parent, 
                                  const char *gconf_key,
                                  gboolean value,
                                  gchar *blah)
{
	static GConfClient *client = NULL;
	gchar *full_key;
	gboolean success;

	if (client == NULL) {
		client = gconf_client_get_default ();
	}

	full_key = g_strdup_printf ("/desktop/gnome/peripherals/keyboard/layout/%s",
				   gconf_key);

	success = gconf_client_set_bool (client, full_key, value, NULL);
	g_free (full_key);
        return success;                   
}

gboolean gconf_applet_get_bool  (PanelApplet *parent, 
                                  const char *gconf_key,
                                  gchar *blah)
{
	static GConfClient *client = NULL;
	gchar *full_key;
	gboolean value;

	full_key = g_strdup_printf ("/desktop/gnome/peripherals/keyboard/layout/%s",
				    gconf_key);
	
	if (client == NULL) {
		client = gconf_client_get_default ();
	}

	value = gconf_client_get_bool (client, full_key, NULL);

	g_free (full_key);

	return value;
}

