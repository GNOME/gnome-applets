/*
 * Brightness Applet
 * Copyright (C) 2006 Benjamin Canou <bookeldor@gmail.com>
 * Copyright (C) 2007 Richard Hughes <richard@hughsie.com>
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
#include "brightness-applet.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib/gi18n-lib.h>
#include <gdk/gdkkeysyms.h>
#include <glib-object.h>

#include "dbus-brightness.h"

struct _GpmBrightnessApplet
{
	GpApplet parent;
	/* applet state */
	gboolean popped; /* the popup is shown */
	/* the popup and its widgets */
	GtkWidget *popup, *slider, *btn_plus, *btn_minus;
	/* the icon */
	GtkWidget *image;
	/* connection to g-s-d */
	DBusSettingsDaemonPowerScreen *proxy;
	guint bus_watch_id;
	gint level;
};

#define GSD_DBUS_SERVICE	"org.gnome.SettingsDaemon.Power"
#define GSD_DBUS_PATH_POWER	"/org/gnome/SettingsDaemon/Power"

G_DEFINE_TYPE (GpmBrightnessApplet, gpm_brightness_applet, GP_TYPE_APPLET)

static void      gpm_applet_update_tooltip        (GpmBrightnessApplet *applet);

#define GPM_BRIGHTNESS_APPLET_ICON		"gnome-brightness-applet"
#define GPM_BRIGHTNESS_APPLET_ICON_BRIGHTNESS	"gpm-brightness-lcd"
#define GPM_BRIGHTNESS_APPLET_ICON_DISABLED	"gpm-brightness-lcd-disabled"
#define GPM_BRIGHTNESS_APPLET_ICON_INVALID	"gpm-brightness-lcd-invalid"

/**
 * gpm_applet_get_brightness:
 * Return value: Success value, or zero for failure
 **/
static gboolean
gpm_applet_get_brightness (GpmBrightnessApplet *applet)
{
	if (applet->proxy == NULL) {
		g_warning ("not connected\n");
		return FALSE;
	}

	applet->level = dbus_settings_daemon_power_screen_get_brightness (applet->proxy);

	return TRUE;
}

/**
 * gpm_applet_update_icon:
 * @applet: Brightness applet instance
 *
 * retrieve an icon from stock with a size adapted to panel
 **/
static void
gpm_applet_update_icon (GpmBrightnessApplet *applet)
{
	const gchar *icon;

	/* get icon */
	if (applet->proxy == NULL) {
		icon = GPM_BRIGHTNESS_APPLET_ICON_INVALID;
	} else if (applet->level == -1) {
		icon = GPM_BRIGHTNESS_APPLET_ICON_DISABLED;
	} else {
		icon = GPM_BRIGHTNESS_APPLET_ICON_BRIGHTNESS;
	}

	gtk_image_set_from_icon_name (GTK_IMAGE(applet->image),
				      icon,
				      GTK_ICON_SIZE_BUTTON);
}

/**
 * gpm_applet_size_allocate_cb:
 * @applet: Brightness applet instance
 *
 * resize icon when panel size changed
 **/
static void
gpm_applet_size_allocate_cb (GtkWidget    *widget,
                             GdkRectangle *allocation)
{
	GpmBrightnessApplet *applet = GPM_BRIGHTNESS_APPLET (widget);
	int size = 0;

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
 * gpm_applet_destroy_popup_cb:
 * @applet: Brightness applet instance
 *
 * destroys the popup (called if orientation has changed)
 **/
static void
gpm_applet_destroy_popup_cb (GpmBrightnessApplet *applet)
{
	if (applet->popup != NULL) {
		gtk_widget_destroy (applet->popup);
		applet->popup = NULL;
		applet->popped = FALSE;
		gpm_applet_update_tooltip (applet);
	}
}

static void
placement_changed_cb (GpApplet            *applet,
                      GtkOrientation       orientation,
                      GtkPositionType      position,
                      GpmBrightnessApplet *self)
{
  gpm_applet_destroy_popup_cb (self);
}

/**
 * gpm_applet_update_tooltip:
 * @applet: Brightness applet instance
 *
 * sets tooltip's content (percentage or disabled)
 **/
static void
gpm_applet_update_tooltip (GpmBrightnessApplet *applet)
{
	gchar *buf = NULL;
	if (applet->popped == FALSE) {
		if (applet->proxy == NULL) {
			buf = g_strdup (_("Cannot connect to gnome-settings-daemon"));
		} else if (applet->level == -1) {
			buf = g_strdup (_("Cannot get laptop panel brightness"));
		} else {
			buf = g_strdup_printf (_("LCD brightness : %d%%"), applet->level);
		}
		gtk_widget_set_tooltip_text (GTK_WIDGET(applet), buf);
	} else {
		gtk_widget_set_tooltip_text (GTK_WIDGET(applet), NULL);
	}
	g_free (buf);
}

/**
 * gpm_applet_update_popup_level:
 * @applet: Brightness applet instance
 *
 * updates popup level of brightness
 **/
static void
gpm_applet_update_popup_level (GpmBrightnessApplet *applet)
{
	if (applet->popup != NULL) {
		gtk_widget_set_sensitive (applet->btn_plus, applet->level < 100);
		gtk_widget_set_sensitive (applet->btn_minus, applet->level > 0);
		gtk_range_set_value (GTK_RANGE(applet->slider), (guint) applet->level);
	}
	gpm_applet_update_tooltip (applet);
}

/**
 * gpm_applet_step_up_cb:
 * @source_object: the object the asynchronous operation was started with
 * @res: a GAsyncResult
 * @user_data: Brightness applet instance
 *
 * callback for the StepDown D-Bus response
 **/
static void
gpm_applet_step_up_cb (GObject *source_object, GAsyncResult *res, gpointer user_data)
{
	GpmBrightnessApplet *applet = GPM_BRIGHTNESS_APPLET (user_data);
	GError  *error = NULL;
	gboolean ret;

	ret = dbus_settings_daemon_power_screen_call_step_up_finish (applet->proxy, &applet->level, NULL, res, &error);
	if (error) {
		g_debug ("ERROR: %s\n", error->message);
		g_error_free (error);
	}
	if (ret) {
		gpm_applet_update_popup_level (applet);
	} else {
		/* abort as the DBUS method failed */
		g_warning ("StepUp brightness failed!");
	}

	return;
}

/**
 * gpm_applet_plus_cb:
 * @widget: The sending widget (plus button)
 * @applet: Brightness applet instance
 *
 * callback for the plus button
 **/
static gboolean
gpm_applet_plus_cb (GtkWidget *w, GpmBrightnessApplet *applet)
{
	if (applet->level == 100)
		return TRUE;

	if (applet->proxy == NULL) {
		g_warning ("not connected");
		return FALSE;
	}

	dbus_settings_daemon_power_screen_call_step_up (applet->proxy, NULL, gpm_applet_step_up_cb, applet);

	return TRUE;
}

/**
 * gpm_applet_step_down_cb:
 * @source_object: the object the asynchronous operation was started with
 * @res: a GAsyncResult
 * @user_data: Brightness applet instance
 *
 * callback for the StepDown D-Bus response
 **/
static void
gpm_applet_step_down_cb (GObject *source_object, GAsyncResult *res, gpointer user_data)
{
	GpmBrightnessApplet *applet = GPM_BRIGHTNESS_APPLET (user_data);
	GError  *error = NULL;
	gboolean ret;

	ret = dbus_settings_daemon_power_screen_call_step_down_finish (applet->proxy, &applet->level, NULL, res, &error);
	if (error) {
		g_debug ("ERROR: %s\n", error->message);
		g_error_free (error);
	}
	if (ret) {
		gpm_applet_update_popup_level (applet);
	} else {
		/* abort as the DBUS method failed */
		g_warning ("StepDown brightness failed!");
	}

	return;
}

/**
 * gpm_applet_minus_cb:
 * @widget: The sending widget (minus button)
 * @applet: Brightness applet instance
 *
 * callback for the minus button
 **/
static gboolean
gpm_applet_minus_cb (GtkWidget *w, GpmBrightnessApplet *applet)
{
	if (applet->level == 0)
		return TRUE;

	if (applet->proxy == NULL) {
		g_warning ("not connected");
		return FALSE;
	}

	dbus_settings_daemon_power_screen_call_step_down (applet->proxy, NULL, gpm_applet_step_down_cb, applet);

	return TRUE;
}

/**
 * gpm_applet_slide_cb:
 * @widget: The sending widget (slider)
 * @applet: Brightness applet instance
 *
 * callback for the slider
 **/
static gboolean
gpm_applet_slide_cb (GtkWidget *w, GpmBrightnessApplet *applet)
{
	if (applet->proxy == NULL) {
		g_warning ("not connected");
		return FALSE;
	}

	applet->level = gtk_range_get_value (GTK_RANGE(applet->slider));
	dbus_settings_daemon_power_screen_set_brightness (applet->proxy, applet->level);
	gpm_applet_update_popup_level (applet);

	return TRUE;
}

/**
 * gpm_applet_key_press_cb:
 * @applet: Brightness applet instance
 * @event: The key press event
 *
 * callback handling keyboard
 * escape to unpop
 **/
static gboolean
gpm_applet_key_press_cb (GtkWidget *popup, GdkEventKey *event, GpmBrightnessApplet *applet)
{
	switch (event->keyval) {
	case GDK_KEY_KP_Enter:
	case GDK_KEY_ISO_Enter:
	case GDK_KEY_3270_Enter:
	case GDK_KEY_Return:
	case GDK_KEY_space:
	case GDK_KEY_KP_Space:
	case GDK_KEY_Escape:
		/* if yet popped, hide */
		if (applet->popped) {
			gtk_widget_hide (applet->popup);
			applet->popped = FALSE;
			gpm_applet_update_tooltip (applet);
			return TRUE;
		} else {
			return FALSE;
		}
		break;
	default:
		return FALSE;
		break;
	}

	return FALSE;
}

/**
 * gpm_applet_scroll_cb:
 * @applet: Brightness applet instance
 * @event: The scroll event
 *
 * callback handling mouse scrolls, when the mouse is over
 * the applet.
 **/
static gboolean
gpm_applet_scroll_cb (GpmBrightnessApplet *applet, GdkEventScroll *event)
{
	if (event->type == GDK_SCROLL) {
		if (event->direction == GDK_SCROLL_UP) {
			gpm_applet_plus_cb (NULL, applet);
		} else {
			gpm_applet_minus_cb (NULL, applet);
		}
		return TRUE;
	}

	return FALSE;
}

/**
 * on_popup_button_press:
 * @applet: Brightness applet instance
 * @event: The button press event
 *
 * hide popup on focus loss.
 **/
static gboolean
on_popup_button_press (GtkWidget      *widget,
                       GdkEventButton *event,
                       GpmBrightnessApplet *applet)
{
        GtkWidget *event_widget;

        if (event->type != GDK_BUTTON_PRESS) {
                return FALSE;
        }
        event_widget = gtk_get_event_widget ((GdkEvent *)event);
        g_debug ("Button press: %p dock=%p", event_widget, widget);
        if (event_widget == widget) {
		gtk_widget_hide (applet->popup);
		applet->popped = FALSE;
		gpm_applet_update_tooltip (applet);
                return TRUE;
        }

        return FALSE;
}

/**
 * gpm_applet_create_popup:
 * @applet: Brightness applet instance
 *
 * cretes a new popup according to orientation of panel
 **/
static void
gpm_applet_create_popup (GpmBrightnessApplet *applet)
{
	static GtkWidget *box, *frame;
	GtkOrientation orientation = gp_applet_get_orientation (GP_APPLET (applet));

	gpm_applet_destroy_popup_cb (applet);

	/* slider */
	if (orientation == GTK_ORIENTATION_VERTICAL) {
		applet->slider = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, 0, 100, 1);
		gtk_widget_set_size_request (applet->slider, 100, -1);
	} else {
		applet->slider = gtk_scale_new_with_range (GTK_ORIENTATION_VERTICAL, 0, 100, 1);
		gtk_widget_set_size_request (applet->slider, -1, 100);
	}
	gtk_range_set_inverted (GTK_RANGE(applet->slider), TRUE);
	gtk_scale_set_draw_value (GTK_SCALE(applet->slider), FALSE);
	gtk_range_set_value (GTK_RANGE(applet->slider), (guint) applet->level);
	g_signal_connect (G_OBJECT(applet->slider), "value-changed", G_CALLBACK(gpm_applet_slide_cb), applet);

	/* minus button */
	applet->btn_minus = gtk_button_new_with_label ("\342\210\222"); /* U+2212 MINUS SIGN */
	gtk_button_set_relief (GTK_BUTTON(applet->btn_minus), GTK_RELIEF_NONE);
	gtk_widget_set_can_focus (applet->btn_minus, FALSE);
	g_signal_connect (G_OBJECT(applet->btn_minus), "pressed", G_CALLBACK(gpm_applet_minus_cb), applet);

	/* plus button */
	applet->btn_plus = gtk_button_new_with_label ("+");
	gtk_button_set_relief (GTK_BUTTON(applet->btn_plus), GTK_RELIEF_NONE);
	gtk_widget_set_can_focus (applet->btn_plus, FALSE);
	g_signal_connect (G_OBJECT(applet->btn_plus), "pressed", G_CALLBACK(gpm_applet_plus_cb), applet);

	/* box */
	if (orientation == GTK_ORIENTATION_VERTICAL) {
		box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 1);
	} else {
		box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 1);
	}
	gtk_box_pack_start (GTK_BOX(box), applet->btn_plus, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(box), applet->slider, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX(box), applet->btn_minus, FALSE, FALSE, 0);

	/* frame */
	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME(frame), GTK_SHADOW_OUT);
	gtk_container_add (GTK_CONTAINER(frame), box);

	/* window */
	applet->popup = gtk_window_new (GTK_WINDOW_POPUP);
	gtk_window_set_type_hint (GTK_WINDOW(applet->popup), GDK_WINDOW_TYPE_HINT_UTILITY);
	gtk_container_add (GTK_CONTAINER(applet->popup), frame);

	/* window events */
        g_signal_connect (G_OBJECT(applet->popup), "button-press-event",
                          G_CALLBACK (on_popup_button_press), applet);

	g_signal_connect (G_OBJECT(applet->popup), "key-press-event",
			  G_CALLBACK(gpm_applet_key_press_cb), applet);
}

/**
 * gpm_applet_popup_cb:
 * @applet: Brightness applet instance
 *
 * pops and unpops
 **/
static gboolean
gpm_applet_popup_cb (GpmBrightnessApplet *applet, GdkEventButton *event)
{
	GtkAllocation allocation, popup_allocation;
	GtkPositionType position;
	gint x, y;
	GdkWindow *window;
	GdkDisplay *display;
	GdkDeviceManager *device_manager;
	GdkDevice *pointer;
	GdkDevice *keyboard;

	/* react only to left mouse button */
	if (event->button != 1) {
		return FALSE;
	}

	/* if yet popped, release focus and hide */
	if (applet->popped) {
		gtk_widget_hide (applet->popup);
		applet->popped = FALSE;
		gpm_applet_update_tooltip (applet);
		return TRUE;
	}

	/* don't show the popup if brightness is unavailable */
	if (applet->level == -1) {
		return FALSE;
	}

	/* otherwise pop */
	applet->popped = TRUE;

	/* create a new popup (initial or if panel parameters changed) */
	if (applet->popup == NULL) {
		gpm_applet_create_popup (applet);
	}

	/* update UI for current brightness */
	gpm_applet_update_popup_level (applet);

	gtk_widget_show_all (applet->popup);

	/* retrieve geometry parameters and move window appropriately */
	position = gp_applet_get_position (GP_APPLET (applet));
	gdk_window_get_origin (gtk_widget_get_window (GTK_WIDGET(applet)), &x, &y);

	gtk_widget_get_allocation (GTK_WIDGET (applet), &allocation);
	gtk_widget_get_allocation (GTK_WIDGET (applet->popup), &popup_allocation);
	switch (position) {
	case GTK_POS_TOP:
		x += allocation.x + allocation.width/2;
		y += allocation.y + allocation.height;
		x -= popup_allocation.width/2;
		break;
	case GTK_POS_BOTTOM:
		x += allocation.x + allocation.width/2;
		y += allocation.y;
		x -= popup_allocation.width/2;
		y -= popup_allocation.height;
		break;
	case GTK_POS_LEFT:
		y += allocation.y + allocation.height/2;
		x += allocation.x + allocation.width;
		y -= popup_allocation.height/2;
		break;
	case GTK_POS_RIGHT:
		y += allocation.y + allocation.height/2;
		x += allocation.x;
		x -= popup_allocation.width;
		y -= popup_allocation.height/2;
		break;
	default:
		g_assert_not_reached ();
	}

	gtk_window_move (GTK_WINDOW (applet->popup), x, y);

	/* grab input */
        window = gtk_widget_get_window (GTK_WIDGET (applet->popup));
	display = gdk_window_get_display (window);
	device_manager = gdk_display_get_device_manager (display);
	pointer = gdk_device_manager_get_client_pointer (device_manager);
	keyboard = gdk_device_get_associated_device (pointer);
	gdk_device_grab (pointer, window,
			 GDK_OWNERSHIP_NONE, TRUE,
			 GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK,
			 NULL, GDK_CURRENT_TIME);
	gdk_device_grab (keyboard, window,
			 GDK_OWNERSHIP_NONE, TRUE,
			 GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK,
			 NULL, GDK_CURRENT_TIME);

	return TRUE;
}

static void
gpm_applet_dialog_about_cb (GSimpleAction *action,
                            GVariant      *parameter,
                            gpointer       user_data)
{
  gp_applet_show_about (GP_APPLET (user_data));
}

static const GActionEntry menu_actions [] =
{
	{ "about", gpm_applet_dialog_about_cb, NULL, NULL, NULL },
	{ NULL }
};

/**
 * gpm_applet_destroy_cb:
 * @widget: Class instance to destroy
 **/
static void
gpm_applet_destroy_cb (GtkWidget *widget)
{
	GpmBrightnessApplet *applet = GPM_BRIGHTNESS_APPLET (widget);

	g_bus_unwatch_name (applet->bus_watch_id);
}

static void
brightness_changed_cb (GDBusProxy *proxy,
		       GVariant   *changed_properties,
		       GVariant   *invalidated_properties,
		       GpmBrightnessApplet *applet)
{
	gpm_applet_get_brightness (applet);
	gpm_applet_update_tooltip (applet);
	gpm_applet_update_icon (applet);
}

/**
 * gpm_brightness_applet_dbus_connect:
 **/
static gboolean
gpm_brightness_applet_dbus_connect (GpmBrightnessApplet *applet)
{
	GError *error = NULL;

	if (applet->proxy == NULL) {
		g_debug ("get proxy\n");
		g_clear_error (&error);
		applet->proxy = dbus_settings_daemon_power_screen_proxy_new_for_bus_sync (G_BUS_TYPE_SESSION,
											  G_DBUS_PROXY_FLAGS_NONE,
											  GSD_DBUS_SERVICE,
											  GSD_DBUS_PATH_POWER,
											  NULL,
											  &error);
		if (error != NULL) {
			g_warning ("Cannot connect, maybe the daemon is not running: %s\n", error->message);
			g_error_free (error);
			applet->proxy = NULL;
			return FALSE;
		}
		g_signal_connect (applet->proxy, "g-properties-changed",
				  G_CALLBACK (brightness_changed_cb), applet);
		/* reset, we might be starting race */
		gpm_applet_get_brightness (applet);
	}
	return TRUE;
}

/**
 * gpm_brightness_applet_dbus_disconnect:
 **/
static gboolean
gpm_brightness_applet_dbus_disconnect (GpmBrightnessApplet *applet)
{
	if (applet->proxy != NULL) {
		g_debug ("removing proxy\n");
		g_object_unref (applet->proxy);
		applet->proxy = NULL;
	}
	applet->level = -1;
	return TRUE;
}

/**
 * gpm_brightness_applet_name_appeared_cb:
 **/
static void
gpm_brightness_applet_name_appeared_cb (GDBusConnection *connection, const gchar *name, const gchar *name_owner, GpmBrightnessApplet *applet)
{
	gpm_brightness_applet_dbus_connect (applet);
	gpm_applet_update_tooltip (applet);
	gpm_applet_update_icon (applet);
}

/**
 * gpm_brightness_applet_name_vanished_cb:
 **/
static void
gpm_brightness_applet_name_vanished_cb (GDBusConnection *connection, const gchar *name, GpmBrightnessApplet *applet)
{
	gpm_brightness_applet_dbus_disconnect (applet);
	gpm_applet_update_tooltip (applet);
	gpm_applet_update_icon (applet);
}

static void
gpm_brightness_applet_setup (GpmBrightnessApplet *applet)
{
	const char *menu_resource;

	/* initialize fields */
	applet->popped = FALSE;
	applet->popup = NULL;
	applet->level = -1;
	applet->image = NULL;
	applet->proxy = NULL;

	/* Add application specific icons to search path */
	gtk_icon_theme_append_search_path (gtk_icon_theme_get_default (),
                                           PKG_DATA_DIR G_DIR_SEPARATOR_S "icons");

	/* monitor the daemon */
	applet->bus_watch_id =
		g_bus_watch_name (G_BUS_TYPE_SESSION,
				  GSD_DBUS_SERVICE,
				  G_BUS_NAME_WATCHER_FLAGS_NONE,
				  (GBusNameAppearedCallback) gpm_brightness_applet_name_appeared_cb,
				  (GBusNameVanishedCallback) gpm_brightness_applet_name_vanished_cb,
				  applet, NULL);

	/* prepare */
	gp_applet_set_flags (GP_APPLET (applet), GP_APPLET_FLAGS_EXPAND_MINOR);
	gtk_widget_set_events (GTK_WIDGET (applet), GDK_SCROLL_MASK);
	applet->image = gtk_image_new();
	gtk_container_add (GTK_CONTAINER (applet), applet->image);

	/* menu */
	menu_resource = GRESOURCE_PREFIX "/ui/brightness-applet-menu.ui";
	gp_applet_setup_menu_from_resource (GP_APPLET (applet), menu_resource, menu_actions);

	/* show */
	gtk_widget_show_all (GTK_WIDGET(applet));

	/* connect */
	g_signal_connect (G_OBJECT(applet), "button-press-event",
			  G_CALLBACK(gpm_applet_popup_cb), NULL);

	g_signal_connect (G_OBJECT(applet), "scroll-event",
			  G_CALLBACK(gpm_applet_scroll_cb), NULL);

	g_signal_connect (G_OBJECT(applet), "placement-changed",
			  G_CALLBACK(placement_changed_cb), applet);

	g_signal_connect (G_OBJECT(applet), "size-allocate",
			  G_CALLBACK(gpm_applet_size_allocate_cb), NULL);

	g_signal_connect (G_OBJECT(applet), "destroy",
			  G_CALLBACK(gpm_applet_destroy_cb), NULL);
}

static void
gpm_brightness_applet_constructed (GObject *object)
{
	G_OBJECT_CLASS (gpm_brightness_applet_parent_class)->constructed (object);
	gpm_brightness_applet_setup (GPM_BRIGHTNESS_APPLET (object));
}

static void
gpm_brightness_applet_class_init (GpmBrightnessAppletClass *class)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (class);

	object_class->constructed = gpm_brightness_applet_constructed;
}

static void
gpm_brightness_applet_init (GpmBrightnessApplet *applet)
{
}

void
gpm_brightness_applet_setup_about (GtkAboutDialog *dialog)
{
  const char *comments;
  const char **authors;
  const char *license;
  const char *copyright;

  comments = _("Adjusts laptop panel brightness.");

  authors = (const char *[])
    {
      "Benjamin Canou <bookeldor@gmail.com>",
      "Richard Hughes <richard@hughsie.com>",
      NULL
    };

  license = _("Licensed under the GNU General Public License Version 2\n\n"

              "Brightness Applet is free software; you can redistribute it and/or\n"
              "modify it under the terms of the GNU General Public License\n"
              "as published by the Free Software Foundation; either version 2\n"
              "of the License, or (at your option) any later version.\n\n"

              "Brightness Applet is distributed in the hope that it will be useful,\n"
              "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
              "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\n"
              "GNU General Public License for more details.\n\n"

              "You should have received a copy of the GNU General Public License\n"
              "along with this program; if not, write to the Free Software\n"
              "Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA\n"
              "02110-1301, USA.\n");

  copyright = _("Copyright \xc2\xa9 2006 Benjamin Canou");

  gtk_about_dialog_set_comments (dialog, comments);

  gtk_about_dialog_set_authors (dialog, authors);
  gtk_about_dialog_set_translator_credits (dialog, _("translator-credits"));
  gtk_about_dialog_set_license (dialog, license);
  gtk_about_dialog_set_copyright (dialog, copyright);
}
