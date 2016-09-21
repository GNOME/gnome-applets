/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * Inhibit Applet
 * Copyright (C) 2006 Benjamin Canou <bookeldor@gmail.com>
 * Copyright (C) 2006-2009 Richard Hughes <richard@hughsie.com>
 *
 * Licensed under the GNU General Public License Version 2
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <panel-applet.h>
#include <gtk/gtk.h>
#include <glib-object.h>
#include <glib/gi18n.h>

#include "dbus-inhibit.h"

#define GPM_TYPE_INHIBIT_APPLET		(gpm_inhibit_applet_get_type ())
#define GPM_INHIBIT_APPLET(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), GPM_TYPE_INHIBIT_APPLET, GpmInhibitApplet))
#define GPM_INHIBIT_APPLET_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k), GPM_TYPE_INHIBIT_APPLET, GpmInhibitAppletClass))
#define GPM_IS_INHIBIT_APPLET(o)	(G_TYPE_CHECK_INSTANCE_TYPE ((o), GPM_TYPE_INHIBIT_APPLET))
#define GPM_IS_INHIBIT_APPLET_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), GPM_TYPE_INHIBIT_APPLET))
#define GPM_INHIBIT_APPLET_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), GPM_TYPE_INHIBIT_APPLET, GpmInhibitAppletClass))

typedef struct{
	PanelApplet parent;
	/* applet state */
	guint cookie;
	/* the icon */
	GtkWidget *image;
	/* connection to gnome-session */
	DBusSessionManager *proxy;
	guint bus_watch_id;
	guint level;
} GpmInhibitApplet;

typedef struct{
	PanelAppletClass	parent_class;
} GpmInhibitAppletClass;

GType                gpm_inhibit_applet_get_type  (void);

#define GS_DBUS_SERVICE		"org.gnome.SessionManager"
#define GS_DBUS_PATH		"/org/gnome/SessionManager"

G_DEFINE_TYPE (GpmInhibitApplet, gpm_inhibit_applet, PANEL_TYPE_APPLET)

static void	gpm_applet_update_icon		(GpmInhibitApplet *applet);
static void	gpm_applet_size_allocate_cb     (GtkWidget *widget, GdkRectangle *allocation);
static void	gpm_applet_update_tooltip	(GpmInhibitApplet *applet);
static gboolean	gpm_applet_click_cb		(GpmInhibitApplet *applet, GdkEventButton *event);
static void	gpm_applet_dialog_about_cb	(GSimpleAction *action, GVariant *parameter, gpointer data);
static gboolean	gpm_applet_cb		        (PanelApplet *_applet, const gchar *iid, gpointer data);
static void	gpm_applet_destroy_cb		(GObject *object);

#define GPM_INHIBIT_APPLET_ID		        "InhibitApplet"
#define GPM_INHIBIT_APPLET_FACTORY_ID	        "InhibitAppletFactory"
#define GPM_INHIBIT_APPLET_ICON		        "gnome-inhibit-applet"
#define GPM_INHIBIT_APPLET_ICON_INHIBIT		"gpm-inhibit"
#define GPM_INHIBIT_APPLET_ICON_INVALID		"gpm-inhibit-invalid"
#define GPM_INHIBIT_APPLET_ICON_UNINHIBIT	"gpm-uninhibit"
#define GPM_INHIBIT_APPLET_NAME			_("Inhibit Applet")
#define GPM_INHIBIT_APPLET_DESC			_("Allows user to inhibit automatic power saving.")
#define PANEL_APPLET_VERTICAL(p)					\
	 (((p) == PANEL_APPLET_ORIENT_LEFT) || ((p) == PANEL_APPLET_ORIENT_RIGHT))


/** cookie is returned as an unsigned integer */
static gboolean
gpm_applet_inhibit (GpmInhibitApplet *applet,
		    const gchar     *appname,
		    const gchar     *reason,
		    guint           *cookie)
{
	GError  *error = NULL;
	gboolean ret;

	g_return_val_if_fail (cookie != NULL, FALSE);

	if (applet->proxy == NULL) {
		g_warning ("not connected\n");
		return FALSE;
	}

	ret = dbus_session_manager_call_inhibit_sync (applet->proxy,
						      appname,
						      0, /* xid */
						      reason,
						      1+2+4+8, /* logoff, switch, suspend, and idle */
						      cookie,
						      NULL,
						      &error);
	if (error) {
		g_debug ("ERROR: %s", error->message);
		g_error_free (error);
		*cookie = 0;
	}
	if (!ret) {
		/* abort as the DBUS method failed */
		g_warning ("Inhibit failed!");
	}

	return ret;
}

static gboolean
gpm_applet_uninhibit (GpmInhibitApplet *applet,
		      guint            cookie)
{
	GError *error = NULL;
	gboolean ret;

	if (applet->proxy == NULL) {
		g_warning ("not connected");
		return FALSE;
	}

	ret = dbus_session_manager_call_uninhibit_sync (applet->proxy,
							cookie,
							NULL,
							&error);
	if (error) {
		g_debug ("ERROR: %s", error->message);
		g_error_free (error);
	}
	if (!ret) {
		/* abort as the DBUS method failed */
		g_warning ("Uninhibit failed!");
	}

	return ret;
}

/**
 * gpm_applet_update_icon:
 * @applet: Inhibit applet instance
 *
 * sets an icon from stock
 **/
static void
gpm_applet_update_icon (GpmInhibitApplet *applet)
{
	const gchar *icon;

	/* get icon */
	if (applet->proxy == NULL) {
		icon = GPM_INHIBIT_APPLET_ICON_INVALID;
	} else if (applet->cookie > 0) {
		icon = GPM_INHIBIT_APPLET_ICON_INHIBIT;
	} else {
		icon = GPM_INHIBIT_APPLET_ICON_UNINHIBIT;
	}
	gtk_image_set_from_icon_name (GTK_IMAGE(applet->image),
				      icon,
				      GTK_ICON_SIZE_BUTTON);
}

/**
 * gpm_applet_size_allocate_cb:
 * @applet: Inhibit applet instance
 *
 * resize icon when panel size changed
 **/
static void
gpm_applet_size_allocate_cb (GtkWidget    *widget,
                             GdkRectangle *allocation)
{
	GpmInhibitApplet *applet = GPM_INHIBIT_APPLET (widget);
	int               size = NULL;

	switch (panel_applet_get_orient (PANEL_APPLET (applet))) {
		case PANEL_APPLET_ORIENT_LEFT:
		case PANEL_APPLET_ORIENT_RIGHT:
			size = allocation->width;
			break;

		case PANEL_APPLET_ORIENT_UP:
		case PANEL_APPLET_ORIENT_DOWN:
			size = allocation->height;
			break;
	}

	/* copied from button-widget.c in the panel */
	if (size < 22)
		size = 16;
	else if (size < 24)
		size = 22;
	else if (size < 32)
		size = 24;
	else if (size < 48)
		size = 32;
	else
		size = 48;

	/* GtkImage already contains a check to do nothing if it's the same */
	gtk_image_set_pixel_size (GTK_IMAGE(applet->image), size);
}

/**
 * gpm_applet_update_tooltip:
 * @applet: Inhibit applet instance
 *
 * sets tooltip's content (percentage or disabled)
 **/
static void
gpm_applet_update_tooltip (GpmInhibitApplet *applet)
{
	const gchar *buf;
	if (applet->proxy == NULL) {
		buf = _("Cannot connect to gnome-session");
	} else {
		if (applet->cookie > 0) {
			buf = _("Automatic sleep inhibited");
		} else {
			buf = _("Automatic sleep enabled");
		}
	}
	gtk_widget_set_tooltip_text (GTK_WIDGET(applet), buf);
}

/**
 * gpm_applet_click_cb:
 * @applet: Inhibit applet instance
 *
 * pops and unpops
 **/
static gboolean
gpm_applet_click_cb (GpmInhibitApplet *applet, GdkEventButton *event)
{
	/* react only to left mouse button */
	if (event->button != 1) {
		return FALSE;
	}

	if (applet->cookie > 0) {
		g_debug ("uninhibiting %u", applet->cookie);
		gpm_applet_uninhibit (applet, applet->cookie);
		applet->cookie = 0;
	} else {
		g_debug ("inhibiting");
		gpm_applet_inhibit (applet,
					  GPM_INHIBIT_APPLET_NAME,
					  _("Manual inhibit"),
					  &(applet->cookie));
	}
	/* update icon */
	gpm_applet_update_icon (applet);
	gpm_applet_update_tooltip (applet);

	return TRUE;
}

/**
 * gpm_applet_dialog_about_cb:
 *
 * displays about dialog
 **/
static void
gpm_applet_dialog_about_cb (GSimpleAction *action, GVariant *parameter, gpointer data)
{
	GtkAboutDialog *about;

	GdkPixbuf *logo =
		gtk_icon_theme_load_icon (gtk_icon_theme_get_default (),
					  GPM_INHIBIT_APPLET_ICON,
					  128, 0, NULL);

	static const gchar *authors[] = {
		"Benjamin Canou <bookeldor@gmail.com>",
		"Richard Hughes <richard@hughsie.com>",
		NULL
	};
	const char *license[] = {
		 N_("Licensed under the GNU General Public License Version 2"),
		 N_("Inhibit Applet is free software; you can redistribute it and/or\n"
		   "modify it under the terms of the GNU General Public License\n"
		   "as published by the Free Software Foundation; either version 2\n"
		   "of the License, or (at your option) any later version."),
		 N_("Inhibit Applet is distributed in the hope that it will be useful,\n"
		   "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
		   "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
		   "GNU General Public License for more details."),
		 N_("You should have received a copy of the GNU General Public License\n"
		   "along with this program; if not, write to the Free Software\n"
		   "Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA\n"
		   "02110-1301, USA.")
	};
	const char *translator_credits = NULL;
	char	   *license_trans;

	license_trans = g_strconcat (_(license[0]), "\n\n", _(license[1]), "\n\n",
				     _(license[2]), "\n\n", _(license[3]), "\n", NULL);

	about = (GtkAboutDialog*) gtk_about_dialog_new ();
	gtk_about_dialog_set_program_name (about, GPM_INHIBIT_APPLET_NAME);
	gtk_about_dialog_set_version (about, VERSION);
	gtk_about_dialog_set_copyright (about, _("Copyright \xc2\xa9 2006-2007 Richard Hughes"));
	gtk_about_dialog_set_comments (about, GPM_INHIBIT_APPLET_DESC);
	gtk_about_dialog_set_authors (about, authors);
	gtk_about_dialog_set_translator_credits (about, translator_credits);
	gtk_about_dialog_set_logo (about, logo);
	gtk_about_dialog_set_license (about, license_trans);

	g_signal_connect (G_OBJECT(about), "response",
			  G_CALLBACK(gtk_widget_destroy), NULL);

	gtk_widget_show (GTK_WIDGET(about));

	g_free (license_trans);
	g_object_unref (logo);
}

/**
 * gpm_applet_destroy_cb:
 * @object: Class instance to destroy
 **/
static void
gpm_applet_destroy_cb (GObject *object)
{
	GpmInhibitApplet *applet = GPM_INHIBIT_APPLET(object);

	g_bus_unwatch_name (applet->bus_watch_id);
}

/**
 * gpm_inhibit_applet_class_init:
 * @klass: Class instance
 **/
static void
gpm_inhibit_applet_class_init (GpmInhibitAppletClass *class)
{
	/* nothing to do here */
}


/**
 * gpm_inhibit_applet_dbus_connect:
 **/
static gboolean
gpm_inhibit_applet_dbus_connect (GpmInhibitApplet *applet)
{
	GError *error = NULL;

	if (applet->proxy == NULL) {
		g_debug ("get proxy\n");
		g_clear_error (&error);
		applet->proxy = dbus_session_manager_proxy_new_for_bus_sync (G_BUS_TYPE_SESSION,
									     G_DBUS_PROXY_FLAGS_NONE,
									     GS_DBUS_SERVICE,
									     GS_DBUS_PATH,
									     NULL,
									     &error);
		if (error != NULL) {
			g_warning ("Cannot connect, maybe the daemon is not running: %s\n", error->message);
			g_error_free (error);
			applet->proxy = NULL;
			return FALSE;
		}
	}
	return TRUE;
}

/**
 * gpm_inhibit_applet_dbus_disconnect:
 **/
static gboolean
gpm_inhibit_applet_dbus_disconnect (GpmInhibitApplet *applet)
{
	if (applet->proxy != NULL) {
		g_debug ("removing proxy\n");
		g_object_unref (applet->proxy);
		applet->proxy = NULL;
		/* we have no inhibit, these are not persistant across reboots */
		applet->cookie = 0;
	}
	return TRUE;
}

/**
 * gpm_inhibit_applet_name_appeared_cb:
 **/
static void
gpm_inhibit_applet_name_appeared_cb (GDBusConnection *connection, const gchar *name, const gchar *name_owner, GpmInhibitApplet *applet)
{
	gpm_inhibit_applet_dbus_connect (applet);
	gpm_applet_update_tooltip (applet);
	gpm_applet_update_icon (applet);
}

/**
 * gpm_inhibit_applet_name_vanished_cb:
 **/
static void
gpm_inhibit_applet_name_vanished_cb (GDBusConnection *connection, const gchar *name, GpmInhibitApplet *applet)
{
	gpm_inhibit_applet_dbus_disconnect (applet);
	gpm_applet_update_tooltip (applet);
	gpm_applet_update_icon (applet);
}

/**
 * gpm_inhibit_applet_init:
 * @applet: Inhibit applet instance
 **/
static void
gpm_inhibit_applet_init (GpmInhibitApplet *applet)
{
	/* initialize fields */
	applet->image = NULL;
	applet->cookie = 0;
	applet->proxy = NULL;

	/* Add application specific icons to search path */
	gtk_icon_theme_append_search_path (gtk_icon_theme_get_default (),
					   PKG_DATA_DIR G_DIR_SEPARATOR_S "icons");

	/* monitor the daemon */
	applet->bus_watch_id =
		g_bus_watch_name (G_BUS_TYPE_SESSION,
				  GS_DBUS_SERVICE,
				  G_BUS_NAME_WATCHER_FLAGS_NONE,
				  (GBusNameAppearedCallback) gpm_inhibit_applet_name_appeared_cb,
				  (GBusNameVanishedCallback) gpm_inhibit_applet_name_vanished_cb,
				  applet, NULL);

	/* prepare */
	panel_applet_set_flags (PANEL_APPLET (applet), PANEL_APPLET_EXPAND_MINOR);
	applet->image = gtk_image_new();
	gtk_container_add (GTK_CONTAINER (applet), applet->image);

	/* show */
	gtk_widget_show_all (GTK_WIDGET(applet));

	/* connect */
	g_signal_connect (G_OBJECT(applet), "button-release-event",
			  G_CALLBACK(gpm_applet_click_cb), NULL);

	g_signal_connect (G_OBJECT(applet), "size-allocate",
			  G_CALLBACK(gpm_applet_size_allocate_cb), NULL);

	g_signal_connect (G_OBJECT(applet), "destroy",
			  G_CALLBACK(gpm_applet_destroy_cb), NULL);
}

/**
 * gpm_applet_cb:
 * @_applet: GpmInhibitApplet instance created by the applet factory
 * @iid: Applet id
 *
 * the function called by libpanel-applet factory after creation
 **/
static gboolean
gpm_applet_cb (PanelApplet *_applet, const gchar *iid, gpointer data)
{
	GpmInhibitApplet *applet = GPM_INHIBIT_APPLET(_applet);
	GSimpleActionGroup *action_group;
	gchar *ui_path;

	static const GActionEntry menu_actions [] = {
		{ "about", gpm_applet_dialog_about_cb, NULL, NULL, NULL },
	};

	if (strcmp (iid, GPM_INHIBIT_APPLET_ID) != 0) {
		return FALSE;
	}

	gtk_window_set_default_icon_name (GPM_INHIBIT_APPLET_ICON);

	action_group = g_simple_action_group_new ();
	g_action_map_add_action_entries (G_ACTION_MAP (action_group),
					 menu_actions,
					 G_N_ELEMENTS (menu_actions),
					 applet);
	ui_path = g_build_filename (INHIBIT_MENU_UI_DIR, "inhibit-applet-menu.xml", NULL);
	panel_applet_setup_menu_from_file (PANEL_APPLET (applet), ui_path, action_group, GETTEXT_PACKAGE);
	g_free (ui_path);

	gtk_widget_insert_action_group (GTK_WIDGET (applet), "inhibit",
					G_ACTION_GROUP (action_group));

	g_object_unref (action_group);

	return TRUE;
}

/**
 * this generates a main with a applet factory
 **/
PANEL_APPLET_IN_PROCESS_FACTORY (GPM_INHIBIT_APPLET_FACTORY_ID,
                                 GPM_TYPE_INHIBIT_APPLET,
                                 gpm_applet_cb,
                                 NULL)
