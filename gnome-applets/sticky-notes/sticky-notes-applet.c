/* Sticky Notes
 * Copyright (C) 2002-2003 Loban A Rahman
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
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
#include "sticky-notes-applet.h"

#include <string.h>

#include "sticky-notes-applet-callbacks.h"
#include "sticky-notes.h"
#include "gsettings.h"

#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>

G_DEFINE_TYPE (StickyNotesApplet, sticky_notes_applet, GP_TYPE_APPLET)

StickyNotes *stickynotes = NULL;

static void sticky_notes_init       (GpApplet          *applet);
static void sticky_notes_init_prefs (void);

static void sticky_notes_applet_new (StickyNotesApplet *self);

/* Popup menu on the applet */
static const GActionEntry stickynotes_applet_menu_actions [] = {
	{ "new-note",    menu_new_note_cb,    NULL, NULL,    NULL },
	{ "hide-notes",  menu_hide_notes_cb,  NULL, NULL,    NULL },
	{ "lock",        menu_toggle_lock_cb, NULL, "false", menu_toggle_lock_state },
	{ "destroy-all", menu_destroy_all_cb, NULL, NULL,    NULL },
	{ "preferences", menu_preferences_cb, NULL, NULL,    NULL },
	{ "help",        menu_help_cb,        NULL, NULL,    NULL },
	{ "about",       menu_about_cb,       NULL, NULL,    NULL },
	{ NULL }
};

static void
sticky_notes_applet_setup (StickyNotesApplet *self)
{
  if (stickynotes == NULL)
    sticky_notes_init (GP_APPLET (self));

  sticky_notes_applet_new (self);

  /* Add applet to linked list of all applets */
  stickynotes->applets = g_list_append (stickynotes->applets, self);

  stickynotes_applet_update_menus ();
  stickynotes_applet_update_tooltips ();
}

static void
sticky_notes_applet_constructed (GObject *object)
{
  G_OBJECT_CLASS (sticky_notes_applet_parent_class)->constructed (object);
  sticky_notes_applet_setup (STICKY_NOTES_APPLET (object));
}

static void
sticky_notes_applet_class_init (StickyNotesAppletClass *self_class)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (self_class);

  object_class->constructed = sticky_notes_applet_constructed;
}

static void
sticky_notes_applet_init (StickyNotesApplet *self)
{
  gp_applet_set_flags (GP_APPLET (self), GP_APPLET_FLAGS_EXPAND_MINOR);
}

/* colorshift a pixbuf */
static void
stickynotes_make_prelight_icon (GdkPixbuf *dest, GdkPixbuf *src, int shift)
{
	gint i, j;
	gint width, height, has_alpha, srcrowstride, destrowstride;
	guchar *target_pixels;
	guchar *original_pixels;
	guchar *pixsrc;
	guchar *pixdest;
	int val;
	guchar r,g,b;

	has_alpha = gdk_pixbuf_get_has_alpha (src);
	width = gdk_pixbuf_get_width (src);
	height = gdk_pixbuf_get_height (src);
	srcrowstride = gdk_pixbuf_get_rowstride (src);
	destrowstride = gdk_pixbuf_get_rowstride (dest);
	target_pixels = gdk_pixbuf_get_pixels (dest);
	original_pixels = gdk_pixbuf_get_pixels (src);

	for (i = 0; i < height; i++) {
		pixdest = target_pixels + i*destrowstride;
		pixsrc = original_pixels + i*srcrowstride;
		for (j = 0; j < width; j++) {
			r = *(pixsrc++);
			g = *(pixsrc++);
			b = *(pixsrc++);
			val = r + shift;
			*(pixdest++) = CLAMP(val, 0, 255);
			val = g + shift;
			*(pixdest++) = CLAMP(val, 0, 255);
			val = b + shift;
			*(pixdest++) = CLAMP(val, 0, 255);
			if (has_alpha)
				*(pixdest++) = *(pixsrc++);
		}
	}
}

/* Create and initalize global sticky notes instance */
static void
sticky_notes_init (GpApplet *applet)
{
	stickynotes = g_new(StickyNotes, 1);

	stickynotes->notes = NULL;
	stickynotes->applets = NULL;
	stickynotes->settings = gp_applet_settings_new (applet, STICKYNOTES_SCHEMA);
	stickynotes->last_timeout_data = 0;

	stickynotes->icon_normal = gtk_icon_theme_load_icon (
			gtk_icon_theme_get_default (),
			"gnome-sticky-notes-applet",
			48, 0, NULL);

	stickynotes->icon_prelight = gdk_pixbuf_new (
			gdk_pixbuf_get_colorspace (stickynotes->icon_normal),
			gdk_pixbuf_get_has_alpha (stickynotes->icon_normal),
			gdk_pixbuf_get_bits_per_sample (
				stickynotes->icon_normal),
			gdk_pixbuf_get_width (stickynotes->icon_normal),
			gdk_pixbuf_get_height (stickynotes->icon_normal));
	stickynotes_make_prelight_icon (stickynotes->icon_prelight,
			stickynotes->icon_normal, 30);
	stickynotes->visible = TRUE;

	sticky_notes_init_prefs ();

	g_signal_connect (stickynotes->settings, "changed",
	                  G_CALLBACK (preferences_apply_cb), NULL);

	/* Max height for large notes*/
	stickynotes->max_height = 0.8*gdk_screen_get_height( gdk_screen_get_default() );

	/* Load sticky notes */
	stickynotes_load (gtk_widget_get_screen (GTK_WIDGET (applet)));

	install_check_click_on_desktop ();
}

static void
sticky_notes_init_prefs (void)
{
	stickynotes->builder = gtk_builder_new ();

	gtk_builder_add_from_resource (stickynotes->builder,
	                               GRESOURCE_PREFIX "/ui/sticky-notes-preferences.ui",
	                               NULL);

	stickynotes->w_prefs = GTK_WIDGET (gtk_builder_get_object (stickynotes->builder,
			"preferences_dialog"));

	stickynotes->w_prefs_width = gtk_spin_button_get_adjustment (
			GTK_SPIN_BUTTON (gtk_builder_get_object (
                                         stickynotes->builder, "width_spin")));
	stickynotes->w_prefs_height = gtk_spin_button_get_adjustment (
			GTK_SPIN_BUTTON (gtk_builder_get_object (
                                         stickynotes->builder, "height_spin")));
	stickynotes->w_prefs_color = GTK_WIDGET (gtk_builder_get_object (stickynotes->builder,
			"default_color"));
	stickynotes->w_prefs_font_color = GTK_WIDGET (gtk_builder_get_object (stickynotes->builder,
			"prefs_font_color"));
	stickynotes->w_prefs_sys_color = GTK_WIDGET (&GTK_CHECK_BUTTON (
				        gtk_builder_get_object (stickynotes->builder,
					"sys_color_check"))->toggle_button);
	stickynotes->w_prefs_font = GTK_WIDGET (gtk_builder_get_object (stickynotes->builder,
			"default_font"));
	stickynotes->w_prefs_sys_font = GTK_WIDGET (&GTK_CHECK_BUTTON (
				        gtk_builder_get_object (stickynotes->builder,
					"sys_font_check"))->toggle_button);
	stickynotes->w_prefs_sticky = GTK_WIDGET (&GTK_CHECK_BUTTON (
				        gtk_builder_get_object (stickynotes->builder,
					"sticky_check"))->toggle_button);
	stickynotes->w_prefs_force = GTK_WIDGET (&GTK_CHECK_BUTTON (
				        gtk_builder_get_object (stickynotes->builder,
					"force_default_check"))->toggle_button);
	stickynotes->w_prefs_desktop = GTK_WIDGET (&GTK_CHECK_BUTTON (
				        gtk_builder_get_object (stickynotes->builder,
					"desktop_hide_check"))->toggle_button);

	g_signal_connect (G_OBJECT (stickynotes->w_prefs), "response",
			G_CALLBACK (preferences_response_cb), NULL);
	g_signal_connect (G_OBJECT (stickynotes->w_prefs), "delete-event",
			G_CALLBACK (preferences_delete_cb), NULL);
	g_signal_connect_swapped (G_OBJECT (stickynotes->w_prefs_width),
			"value-changed",
			G_CALLBACK (preferences_save_cb), NULL);
	g_signal_connect_swapped (G_OBJECT (stickynotes->w_prefs_height),
			"value-changed",
			G_CALLBACK (preferences_save_cb), NULL);
	g_signal_connect_swapped (G_OBJECT (stickynotes->w_prefs_sys_color),
			"toggled",
			G_CALLBACK (preferences_save_cb), NULL);
	g_signal_connect_swapped (G_OBJECT(stickynotes->w_prefs_sys_font),
			"toggled", G_CALLBACK (preferences_save_cb), NULL);
	g_signal_connect (G_OBJECT (stickynotes->w_prefs_color),
			"color-set", G_CALLBACK (preferences_color_cb), NULL);
	g_signal_connect (G_OBJECT (stickynotes->w_prefs_font_color),
			"color-set", G_CALLBACK (preferences_color_cb), NULL);
	g_signal_connect (G_OBJECT (stickynotes->w_prefs_font),
			"font-set", G_CALLBACK (preferences_font_cb), NULL);
	g_signal_connect_swapped (G_OBJECT (stickynotes->w_prefs_sticky),
			"toggled", G_CALLBACK (preferences_save_cb), NULL);
	g_signal_connect_swapped (G_OBJECT (stickynotes->w_prefs_force),
			"toggled", G_CALLBACK (preferences_save_cb), NULL);
	g_signal_connect_swapped (G_OBJECT (stickynotes->w_prefs_desktop),
			"toggled", G_CALLBACK (preferences_save_cb), NULL);

	{
		GtkSizeGroup *group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);

		gtk_size_group_add_widget(group, GTK_WIDGET (gtk_builder_get_object (stickynotes->builder, "width_label")));
		gtk_size_group_add_widget(group, GTK_WIDGET (gtk_builder_get_object (stickynotes->builder, "height_label")));
		gtk_size_group_add_widget(group, GTK_WIDGET (gtk_builder_get_object (stickynotes->builder, "prefs_color_label")));

		g_object_unref(group);
	}

	if (!g_settings_is_writable (stickynotes->settings, KEY_DEFAULT_WIDTH))
	{
		gtk_widget_set_sensitive (GTK_WIDGET (gtk_builder_get_object (
					stickynotes->builder, "width_label")),
				FALSE);
		gtk_widget_set_sensitive (GTK_WIDGET (gtk_builder_get_object (
					stickynotes->builder, "width_spin")),
				FALSE);
	}
	if (!g_settings_is_writable (stickynotes->settings, KEY_DEFAULT_HEIGHT))
	{
		gtk_widget_set_sensitive (GTK_WIDGET (gtk_builder_get_object (
					stickynotes->builder, "height_label")),
				FALSE);
		gtk_widget_set_sensitive (GTK_WIDGET (gtk_builder_get_object (
					stickynotes->builder, "height_spin")),
				FALSE);
	}
	if (!g_settings_is_writable (stickynotes->settings, KEY_DEFAULT_COLOR))
	{
		gtk_widget_set_sensitive (GTK_WIDGET (gtk_builder_get_object (
					stickynotes->builder, "prefs_color_label")),
				FALSE);
		gtk_widget_set_sensitive (stickynotes->w_prefs_color, FALSE);
	}
	if (!g_settings_is_writable (stickynotes->settings, KEY_DEFAULT_FONT_COLOR))
	{
		gtk_widget_set_sensitive (GTK_WIDGET (gtk_builder_get_object (
					stickynotes->builder, "prefs_font_color_label")),
				FALSE);
		gtk_widget_set_sensitive (stickynotes->w_prefs_font_color,
				FALSE);
	}
	if (!g_settings_is_writable (stickynotes->settings, KEY_USE_SYSTEM_COLOR))
		gtk_widget_set_sensitive (stickynotes->w_prefs_sys_color,
				FALSE);
	if (!g_settings_is_writable (stickynotes->settings, KEY_DEFAULT_FONT))
	{
		gtk_widget_set_sensitive (GTK_WIDGET (gtk_builder_get_object (
					stickynotes->builder, "prefs_font_label")),
				FALSE);
		gtk_widget_set_sensitive (stickynotes->w_prefs_font, FALSE);
	}
	if (!g_settings_is_writable (stickynotes->settings, KEY_USE_SYSTEM_FONT))
		gtk_widget_set_sensitive (stickynotes->w_prefs_sys_font,
				FALSE);
	if (!g_settings_is_writable (stickynotes->settings, KEY_STICKY))
		gtk_widget_set_sensitive (stickynotes->w_prefs_sticky, FALSE);
	if (!g_settings_is_writable (stickynotes->settings, KEY_FORCE_DEFAULT))
		gtk_widget_set_sensitive (stickynotes->w_prefs_force, FALSE);

	stickynotes_applet_update_prefs();
}

/* Create a Sticky Notes applet */
static void
sticky_notes_applet_new (StickyNotesApplet *applet)
{
	AtkObject *atk_obj;
	const gchar *resource_name;
	GAction *action;

	/* Initialize Sticky Notes Applet */
	applet->w_image = gtk_image_new();
	applet->destroy_all_dialog = NULL;
	applet->prelighted = FALSE;
	applet->pressed = FALSE;

	applet->menu_tip = NULL;

	/* Add the applet icon */
	gtk_container_add(GTK_CONTAINER(applet), applet->w_image);
	applet->panel_orient = gp_applet_get_orientation (GP_APPLET (applet));
	stickynotes_applet_update_icon(applet);

	/* Add the popup menu */
	resource_name = GRESOURCE_PREFIX "/ui/sticky-notes-applet-menu.ui";
	gp_applet_setup_menu_from_resource (GP_APPLET (applet),
	                                    resource_name,
	                                    stickynotes_applet_menu_actions);

	action = gp_applet_menu_lookup_action (GP_APPLET (applet), "preferences");
	g_object_bind_property (applet, "locked-down",
	                        action, "enabled",
	                        G_BINDING_DEFAULT|G_BINDING_INVERT_BOOLEAN|G_BINDING_SYNC_CREATE);

	/* Connect all signals for applet management */
	g_signal_connect(applet, "button-press-event",
			G_CALLBACK(applet_button_cb), applet);
	g_signal_connect(applet, "key-press-event",
			G_CALLBACK(applet_key_cb), applet);
	g_signal_connect(applet, "focus-in-event",
			G_CALLBACK(applet_focus_cb), applet);
	g_signal_connect(applet, "focus-out-event",
			G_CALLBACK(applet_focus_cb), applet);
	g_signal_connect(applet, "enter-notify-event",
			G_CALLBACK(applet_cross_cb), applet);
	g_signal_connect(applet, "leave-notify-event",
			G_CALLBACK(applet_cross_cb), applet);
	g_signal_connect(applet, "size-allocate",
			G_CALLBACK(applet_size_allocate_cb), applet);
	g_signal_connect(applet, "placement-changed",
			G_CALLBACK(applet_placement_changed_cb), applet);
	g_signal_connect(applet, "destroy",
			G_CALLBACK(applet_destroy_cb), applet);

	atk_obj = gtk_widget_get_accessible (GTK_WIDGET (applet));
	atk_object_set_name (atk_obj, _("Sticky Notes"));

	/* Show the applet */
	gtk_widget_show_all (GTK_WIDGET (applet));
}

void stickynotes_applet_update_icon(StickyNotesApplet *applet)
{
	GdkPixbuf *pixbuf1, *pixbuf2;

	gint size = applet->panel_size;

        if (size > 3)
           size = size -3;

	/* Choose appropriate icon and size it */
	if (applet->prelighted)
		pixbuf1 = gdk_pixbuf_scale_simple(stickynotes->icon_prelight, size, size, GDK_INTERP_BILINEAR);
	else
		pixbuf1 = gdk_pixbuf_scale_simple(stickynotes->icon_normal, size, size, GDK_INTERP_BILINEAR);

	if (!pixbuf1)
		return;

	/* Shift the icon if pressed */
	pixbuf2 = gdk_pixbuf_copy(pixbuf1);
	if (applet->pressed)
		gdk_pixbuf_scale(pixbuf1, pixbuf2, 0, 0, size, size, 1, 1, 1, 1, GDK_INTERP_BILINEAR);

	/* Apply the finished pixbuf to the applet image */
	gtk_image_set_from_pixbuf(GTK_IMAGE(applet->w_image), pixbuf2);

	g_object_unref(pixbuf1);
	g_object_unref(pixbuf2);
}

void
stickynotes_applet_update_prefs (void)
{
	gint width, height;
	gboolean sys_color, sys_font, sticky, force_default, desktop_hide;
	char *font_str;
	char *color_str, *font_color_str;
	GdkRGBA color, font_color;

	width = g_settings_get_int (stickynotes->settings, KEY_DEFAULT_WIDTH);
	width = MAX (width, 1);

	height = g_settings_get_int (stickynotes->settings, KEY_DEFAULT_HEIGHT);
	height = MAX (height, 1);

	sys_color = g_settings_get_boolean (stickynotes->settings, KEY_USE_SYSTEM_COLOR);
	sys_font = g_settings_get_boolean (stickynotes->settings, KEY_USE_SYSTEM_FONT);
	sticky = g_settings_get_boolean (stickynotes->settings, KEY_STICKY);
	force_default = g_settings_get_boolean (stickynotes->settings, KEY_FORCE_DEFAULT);
	desktop_hide = g_settings_get_boolean (stickynotes->settings, KEY_DESKTOP_HIDE);

	font_str = g_settings_get_string (stickynotes->settings, KEY_DEFAULT_FONT);
	if (IS_STRING_EMPTY (font_str)) {
		g_free (font_str);
		font_str = g_strdup ("Sans 10");
	}

	color_str = g_settings_get_string (stickynotes->settings, KEY_DEFAULT_COLOR);
	if (IS_STRING_EMPTY (color_str)) {
		g_free (color_str);
		color_str = g_strdup ("#ECF833");
	}

	font_color_str = g_settings_get_string (stickynotes->settings, KEY_DEFAULT_FONT_COLOR);
	if (IS_STRING_EMPTY (font_color_str)) {
		g_free (font_color_str);
		font_color_str = g_strdup ("#000000");
	}

	gdk_rgba_parse (&color, color_str);
	g_free (color_str);

	gdk_rgba_parse (&font_color, font_color_str);
	g_free (font_color_str);

	gtk_adjustment_set_value (stickynotes->w_prefs_width, width);
	gtk_adjustment_set_value (stickynotes->w_prefs_height, height);
	gtk_toggle_button_set_active (
			GTK_TOGGLE_BUTTON (stickynotes->w_prefs_sys_color),
			sys_color);
	gtk_toggle_button_set_active (
			GTK_TOGGLE_BUTTON(stickynotes->w_prefs_sys_font),
			sys_font);
	gtk_toggle_button_set_active (
			GTK_TOGGLE_BUTTON (stickynotes->w_prefs_sticky),
			sticky);
	gtk_toggle_button_set_active (
			GTK_TOGGLE_BUTTON (stickynotes->w_prefs_force),
			force_default);
	gtk_toggle_button_set_active (
			GTK_TOGGLE_BUTTON (stickynotes->w_prefs_desktop),
			desktop_hide);

	gtk_color_chooser_set_rgba (GTK_COLOR_CHOOSER (stickynotes->w_prefs_color), &color);
	gtk_color_chooser_set_rgba (GTK_COLOR_CHOOSER (stickynotes->w_prefs_font_color), &font_color);
	gtk_font_button_set_font_name (
			GTK_FONT_BUTTON (stickynotes->w_prefs_font), font_str);
	g_free (font_str);

	if (g_settings_is_writable (stickynotes->settings, KEY_DEFAULT_COLOR))
	{
		gtk_widget_set_sensitive (GTK_WIDGET (gtk_builder_get_object (
				stickynotes->builder, "prefs_color_label")),
				!sys_color);
		gtk_widget_set_sensitive (stickynotes->w_prefs_color,
				!sys_color);
	}
	if (g_settings_is_writable (stickynotes->settings, KEY_DEFAULT_FONT_COLOR))
	{
		gtk_widget_set_sensitive (GTK_WIDGET (gtk_builder_get_object (
				stickynotes->builder, "prefs_font_color_label")),
				!sys_color);
		gtk_widget_set_sensitive (stickynotes->w_prefs_font_color,
				!sys_color);
	}
	if (g_settings_is_writable (stickynotes->settings, KEY_DEFAULT_FONT))
	{
		gtk_widget_set_sensitive (GTK_WIDGET (gtk_builder_get_object (
				stickynotes->builder, "prefs_font_label")),
				!sys_font);
		gtk_widget_set_sensitive (stickynotes->w_prefs_font,
				!sys_font);
	}
}

void stickynotes_applet_update_menus(void)
{
	GList *l;

	gboolean locked = g_settings_get_boolean (stickynotes->settings, KEY_LOCKED);
	gboolean locked_writable = g_settings_is_writable (stickynotes->settings, KEY_LOCKED);

	for (l = stickynotes->applets; l != NULL; l = l->next) {
		StickyNotesApplet *applet = l->data;

		GAction *action = gp_applet_menu_lookup_action (GP_APPLET (applet), "lock");
		g_simple_action_set_enabled (G_SIMPLE_ACTION (action), locked_writable);
		g_simple_action_set_state (G_SIMPLE_ACTION (action), g_variant_new_boolean (locked));
	}
}

void
stickynotes_applet_update_tooltips (void)
{
	int num;
	char *tooltip, *no_notes;
	StickyNotesApplet *applet;
	GList *l;

	num = g_list_length (stickynotes->notes);

	no_notes = g_strdup_printf (ngettext ("%d note", "%d notes", num), num);
	tooltip = g_strdup_printf ("%s\n%s", _("Show sticky notes"), no_notes);

	for (l = stickynotes->applets; l; l = l->next)
	{
		applet = l->data;
		gtk_widget_set_tooltip_text (GTK_WIDGET (applet), tooltip);

		if (applet->menu_tip)
			gtk_label_set_text (GTK_LABEL (gtk_bin_get_child (
						GTK_BIN (applet->menu_tip))),
				no_notes);
	}

	g_free (tooltip);
	g_free (no_notes);
}

void
stickynotes_applet_panel_icon_get_geometry (int *x, int *y, int *width, int *height)
{
	GtkWidget *widget;
        GtkAllocation allocation;
	GtkRequisition requisition;
	StickyNotesApplet *applet;

	applet = stickynotes->applets->data;

	widget = GTK_WIDGET (applet->w_image);

	gtk_widget_get_preferred_size (widget, NULL, &requisition);

	gdk_window_get_origin (gtk_widget_get_window (widget), x, y);

	gtk_widget_get_allocation (widget, &allocation);
	*width = allocation.x;
	*height = allocation.y;
}