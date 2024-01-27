/*
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

#include "config.h"
#include "inhibit-applet.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib-object.h>
#include <glib/gi18n-lib.h>

#include "dbus-inhibit.h"

struct _InhibitApplet
{
	GpApplet parent;

	/* applet state */
	guint cookie;
	/* the icon */
	GtkWidget *image;
	/* connection to gnome-session */
	DBusSessionManager *proxy;
	guint bus_watch_id;
	guint level;
};

#define GS_DBUS_SERVICE		"org.gnome.SessionManager"
#define GS_DBUS_PATH		"/org/gnome/SessionManager"

G_DEFINE_TYPE (InhibitApplet, inhibit_applet, GP_TYPE_APPLET)

#define GPM_INHIBIT_APPLET_ICON_INHIBIT		"gpm-inhibit"
#define GPM_INHIBIT_APPLET_ICON_INVALID		"gpm-inhibit-invalid"
#define GPM_INHIBIT_APPLET_ICON_UNINHIBIT	"gpm-uninhibit"
#define GPM_INHIBIT_APPLET_NAME			_("Inhibit Applet")

/** cookie is returned as an unsigned integer */
static gboolean
gpm_applet_inhibit (InhibitApplet *applet,
                    const char    *appname,
                    const char    *reason,
                    guint         *cookie)
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
gpm_applet_uninhibit (InhibitApplet *applet,
                      guint          cookie)
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
gpm_applet_update_icon (InhibitApplet *applet)
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
	InhibitApplet *applet;
	int size;

	applet = INHIBIT_APPLET (widget);
	size = 0;

	switch (gp_applet_get_orientation (GP_APPLET (applet))) {
		case GTK_ORIENTATION_VERTICAL:
			size = allocation->width;
			break;

		case GTK_ORIENTATION_HORIZONTAL:
			size = allocation->height;
			break;

		default:
			g_assert_not_reached ();
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
gpm_applet_update_tooltip (InhibitApplet *applet)
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
gpm_applet_click_cb (InhibitApplet  *applet,
                     GdkEventButton *event)
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

static void
gpm_applet_dialog_about_cb (GSimpleAction *action,
                            GVariant      *parameter,
                            gpointer       user_data)
{
  gp_applet_show_about (GP_APPLET (user_data));
}

static const GActionEntry menu_actions[] =
{
  { "about", gpm_applet_dialog_about_cb, NULL, NULL, NULL },
  { NULL }
};

static gboolean
gpm_inhibit_applet_dbus_connect (InhibitApplet *applet)
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

static gboolean
gpm_inhibit_applet_dbus_disconnect (InhibitApplet *applet)
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

static void
gpm_inhibit_applet_name_appeared_cb (GDBusConnection *connection,
                                     const char      *name,
                                     const char      *name_owner,
                                     InhibitApplet   *applet)
{
	gpm_inhibit_applet_dbus_connect (applet);
	gpm_applet_update_tooltip (applet);
	gpm_applet_update_icon (applet);
}

static void
gpm_inhibit_applet_name_vanished_cb (GDBusConnection *connection,
                                     const  char     *name,
                                     InhibitApplet   *applet)
{
	gpm_inhibit_applet_dbus_disconnect (applet);
	gpm_applet_update_tooltip (applet);
	gpm_applet_update_icon (applet);
}

static void
inhibit_applet_setup (InhibitApplet *applet)
{
	const char *menu_resource;

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
	gp_applet_set_flags (GP_APPLET (applet), GP_APPLET_FLAGS_EXPAND_MINOR);
	applet->image = gtk_image_new();
	gtk_container_add (GTK_CONTAINER (applet), applet->image);

	menu_resource = GRESOURCE_PREFIX "/ui/inhibit-applet-menu.ui";
	gp_applet_setup_menu_from_resource (GP_APPLET (applet),
	                                    menu_resource,
	                                    menu_actions);

	/* show */
	gtk_widget_show_all (GTK_WIDGET(applet));

	/* connect */
	g_signal_connect (G_OBJECT(applet), "button-release-event",
			  G_CALLBACK(gpm_applet_click_cb), NULL);

	g_signal_connect (G_OBJECT(applet), "size-allocate",
			  G_CALLBACK(gpm_applet_size_allocate_cb), NULL);
}

static void
inhibit_applet_constructed (GObject *object)
{
  G_OBJECT_CLASS (inhibit_applet_parent_class)->constructed (object);
  inhibit_applet_setup (INHIBIT_APPLET (object));
}

static void
inhibit_applet_dispose (GObject *object)
{
  InhibitApplet *self;

  self = INHIBIT_APPLET (object);

  if (self->bus_watch_id != 0)
    {
      g_bus_unwatch_name (self->bus_watch_id);
      self->bus_watch_id = 0;
    }

  G_OBJECT_CLASS (inhibit_applet_parent_class)->dispose (object);
}

static void
inhibit_applet_class_init (InhibitAppletClass *self_class)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (self_class);

  object_class->constructed = inhibit_applet_constructed;
  object_class->dispose = inhibit_applet_dispose;
}

static void
inhibit_applet_init (InhibitApplet *applet)
{
}

void
inhibit_applet_setup_about (GtkAboutDialog *dialog)
{
  const char *comments;
  const char **authors;
  const char *license;
  const char *copyright;

  comments = _("Allows user to inhibit automatic power saving.");

  authors = (const char *[])
    {
      "Benjamin Canou <bookeldor@gmail.com>",
      "Richard Hughes <richard@hughsie.com>",
      NULL
    };

  license = _("Licensed under the GNU General Public License Version 2\n\n"

              "Inhibit Applet is free software; you can redistribute it and/or\n"
              "modify it under the terms of the GNU General Public License\n"
              "as published by the Free Software Foundation; either version 2\n"
              "of the License, or (at your option) any later version.\n\n"

              "Inhibit Applet is distributed in the hope that it will be useful,\n"
              "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
              "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\n"
              "GNU General Public License for more details."

              "You should have received a copy of the GNU General Public License\n"
              "along with this program; if not, write to the Free Software\n"
              "Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA\n"
              "02110-1301, USA.\n");

  copyright = _("Copyright \xc2\xa9 2006-2007 Richard Hughes");

  gtk_about_dialog_set_comments (dialog, comments);

  gtk_about_dialog_set_authors (dialog, authors);
  gtk_about_dialog_set_license (dialog, license);
  gtk_about_dialog_set_translator_credits (dialog, _("translator-credits"));
  gtk_about_dialog_set_copyright (dialog, copyright);
}
