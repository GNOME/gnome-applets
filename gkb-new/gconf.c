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

gboolean
gconf_applet_set_string (char const *gconf_key, gchar const *value)
{
	static GConfClient *client = NULL;
	gchar *full_key;
	gboolean success;

	if (!value)
		return FALSE;
		
	if (client == NULL)
		client = gconf_client_get_default ();

	full_key =  g_strconcat (GKB_GCONF_ROOT "/", gconf_key, NULL);
	success = gconf_client_set_string (client, full_key, value, NULL);
	g_free (full_key);
        return success;                                  
}

gchar *
gconf_applet_get_string (char const *gconf_key)
{
	static GConfClient *client = NULL;
	gchar *full_key;
	gchar * value;

	if (client == NULL) {
		client = gconf_client_get_default ();
	}

	full_key = g_strconcat (GKB_GCONF_ROOT "/", gconf_key, NULL);
	value = gconf_client_get_string (client, full_key, NULL);
	g_free (full_key);

	return value;                                  
}

gboolean
gconf_applet_set_int (char const *gconf_key, gint value)
{
	static GConfClient *client = NULL;
	gchar *full_key;
	gboolean success;

	if (client == NULL)
		client = gconf_client_get_default ();

	full_key = g_strconcat (GKB_GCONF_ROOT "/", gconf_key, NULL);
	success = gconf_client_set_int (client, full_key, value, NULL);
	g_free (full_key);
        return success;                   
}

gint
gconf_applet_get_int (char const *gconf_key)
{
	static GConfClient *client = NULL;
	gchar *full_key;
	gint value;

	if (client == NULL)
		client = gconf_client_get_default ();

	full_key = g_strconcat (GKB_GCONF_ROOT "/", gconf_key, NULL);
	value = gconf_client_get_int (client, full_key, NULL);
	g_free (full_key);

	return value;
                                  
}

gboolean
gconf_applet_set_bool (char const *gconf_key, gboolean value)
{
	static GConfClient *client = NULL;
	gchar *full_key;
	gboolean success;

	if (client == NULL)
		client = gconf_client_get_default ();

	full_key = g_strconcat (GKB_GCONF_ROOT "/", gconf_key, NULL);
	success = gconf_client_set_bool (client, full_key, value, NULL);
	g_free (full_key);
        return success;                   
}

gboolean
gconf_applet_get_bool (char const *gconf_key)
{
	static GConfClient *client = NULL;
	gchar *full_key;
	gboolean value;

	if (client == NULL)
		client = gconf_client_get_default ();

	full_key = g_strconcat (GKB_GCONF_ROOT "/", gconf_key, NULL);
	value = gconf_client_get_bool (client, full_key, NULL);
	g_free (full_key);

	return value;
}

gboolean
gconf_applet_is_writable (char const *gconf_key)
{
	static GConfClient *client = NULL;
	gchar *full_key;
	gboolean writable;

	if (client == NULL)
		client = gconf_client_get_default ();

	full_key = g_strconcat (GKB_GCONF_ROOT "/", gconf_key, NULL);
	writable = gconf_client_key_is_writable (client, full_key, NULL);
	g_free (full_key);

	return writable;
}

/* Writability of the keyboards list all in one go.  Yes it is
   kind of ugly, but it would be very hard to handle this properly with
   the current preferences setup.  So basically if we can't seem to change
   any one thing, just disallow changes completely.
   -George */
gboolean
gconf_applet_keyboard_list_is_writable (void)
{
	static GConfClient *client = NULL;
	char buf[256];
	int i;

	if (client == NULL)
		client = gconf_client_get_default ();

	if ( ! gconf_client_key_is_writable (client, GKB_GCONF_ROOT "/num", NULL))
		return FALSE;

	/* FIXME: 10?  Why 10?  The whole thing is not very
	   easy to handle, so we check 10 layouts since no one
	   is likely to use more.  Yes it's a hack, but given
	   that lockdown of this applet is not very likely,
	   this should be good enough.
	   -George */
	for (i = 0; i < 10; i++) {
		g_snprintf (buf, sizeof (buf), GKB_GCONF_ROOT "/name_%d", i);
		if ( ! gconf_client_key_is_writable (client, buf, NULL))
			return FALSE;
		g_snprintf (buf, sizeof (buf), GKB_GCONF_ROOT "/country_%d", i);
		if ( ! gconf_client_key_is_writable (client, buf, NULL))
			return FALSE;
		g_snprintf (buf, sizeof (buf),GKB_GCONF_ROOT  "/lang_%d", i);
		if ( ! gconf_client_key_is_writable (client, buf, NULL))
			return FALSE;
		g_snprintf (buf, sizeof (buf),GKB_GCONF_ROOT  "/label_%d", i);
		if ( ! gconf_client_key_is_writable (client, buf, NULL))
			return FALSE;
		g_snprintf (buf, sizeof (buf), GKB_GCONF_ROOT "/flag_%d", i);
		if ( ! gconf_client_key_is_writable (client, buf, NULL))
			return FALSE;
		g_snprintf (buf, sizeof (buf), GKB_GCONF_ROOT "/command_%d", i);
		if ( ! gconf_client_key_is_writable (client, buf, NULL))
			return FALSE;
	}

	return TRUE;
}
