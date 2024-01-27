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

#include <gdk/gdkx.h>
#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>
#include <string.h>
#include <X11/Xatom.h>

#include "sticky-notes.h"
#include "sticky-notes-preferences.h"
#include "gsettings.h"

G_DEFINE_TYPE (StickyNotesApplet, sticky_notes_applet, GP_TYPE_APPLET)

static void sticky_notes_init       (StickyNotesApplet *self);
static void sticky_notes_applet_new (StickyNotesApplet *self);

static void
stickynote_show_notes (StickyNotesApplet *self,
                       gboolean           visible)
{
	StickyNote *note;
	GList *l;

	if (self->visible == visible)
		return;
	self->visible = visible;

	for (l = self->notes; l; l = l->next)
	{
		note = l->data;
		stickynote_set_visible (note, visible);
	}
}

static void
stickynote_toggle_notes_visible (StickyNotesApplet *self)
{
  stickynote_show_notes (self, !self->visible);
}

static void
menu_new_note_cb (GSimpleAction *action,
                  GVariant      *parameter,
                  gpointer       user_data)
{
  StickyNotesApplet *self;

  self = STICKY_NOTES_APPLET (user_data);

  stickynotes_add (self);
}

static void
menu_hide_notes_cb (GSimpleAction *action,
                    GVariant      *parameter,
                    gpointer       user_data)
{
  StickyNotesApplet *self;

  self = STICKY_NOTES_APPLET (user_data);

  stickynote_show_notes (self, FALSE);
}

static void
menu_toggle_lock_cb (GSimpleAction *action,
                     GVariant      *parameter,
                     gpointer       user_data)
{
	GVariant *state = g_action_get_state (G_ACTION (action));
	g_action_change_state (G_ACTION (action), g_variant_new_boolean (!g_variant_get_boolean (state)));
	g_variant_unref (state);
}

static void
menu_toggle_lock_state (GSimpleAction *action,
                        GVariant      *value,
                        gpointer       user_data)
{
  StickyNotesApplet *self;
  gboolean locked;

  self = STICKY_NOTES_APPLET (user_data);
  locked = g_variant_get_boolean (value);

  if (g_settings_is_writable (self->settings, KEY_LOCKED))
    g_settings_set_boolean (self->settings, KEY_LOCKED, locked);
}

static void
destroy_all_response_cb (GtkDialog         *dialog,
                         gint               id,
                         StickyNotesApplet *applet)
{
	if (id == GTK_RESPONSE_OK) {
		g_list_free_full (applet->notes, (GDestroyNotify) stickynote_free);
		applet->notes = NULL;
	}

	stickynotes_applet_update_tooltips (applet);
	stickynotes_save (applet);

	gtk_widget_destroy (GTK_WIDGET (dialog));
	applet->destroy_all_dialog = NULL;
}

static void
menu_destroy_all_cb (GSimpleAction *action,
                     GVariant      *parameter,
                     gpointer       user_data)
{
	StickyNotesApplet *applet = (StickyNotesApplet *) user_data;
	GtkBuilder *builder;

	builder = gtk_builder_new ();
	gtk_builder_set_translation_domain (builder, GETTEXT_PACKAGE);
	gtk_builder_add_from_resource (builder,
	                               GRESOURCE_PREFIX "/ui/sticky-notes-delete-all.ui",
	                               NULL);

	if (applet->destroy_all_dialog != NULL) {
		gtk_window_set_screen (GTK_WINDOW (applet->destroy_all_dialog),
				       gtk_widget_get_screen (GTK_WIDGET (applet)));

		gtk_window_present (GTK_WINDOW (applet->destroy_all_dialog));
		return;
	}

	applet->destroy_all_dialog = GTK_WIDGET (gtk_builder_get_object (builder, "delete_all_dialog"));

	g_object_unref (builder);

	g_signal_connect (applet->destroy_all_dialog, "response",
			  G_CALLBACK (destroy_all_response_cb),
			  applet);

	gtk_window_set_screen (GTK_WINDOW (applet->destroy_all_dialog),
			gtk_widget_get_screen (GTK_WIDGET (applet)));

	gtk_widget_show_all (applet->destroy_all_dialog);
}

static void
preferences_response_cb (GtkWidget         *widget,
                         gint               response_id,
                         StickyNotesApplet *self)
{
  if (response_id == GTK_RESPONSE_HELP)
    gp_applet_show_help (GP_APPLET (self), "stickynotes-advanced-settings");
  else if (response_id == GTK_RESPONSE_CLOSE)
    gtk_widget_destroy (widget);
}

static void
menu_preferences_cb (GSimpleAction *action,
                     GVariant      *parameter,
                     gpointer       user_data)
{
  StickyNotesApplet *self;

  self = STICKY_NOTES_APPLET (user_data);

  if (self->w_prefs != NULL)
    {
      gtk_window_present (GTK_WINDOW (self->w_prefs));
      return;
    }

  self->w_prefs = sticky_notes_preferences_new (self->settings);
  g_object_add_weak_pointer (G_OBJECT (self->w_prefs),
                             (gpointer *) &self->w_prefs);

  g_signal_connect (self->w_prefs,
                    "response",
                    G_CALLBACK (preferences_response_cb),
                    self);

  gtk_window_present (GTK_WINDOW (self->w_prefs));
}

static void
menu_help_cb (GSimpleAction *action,
              GVariant      *parameter,
              gpointer       user_data)
{
  gp_applet_show_help (GP_APPLET (user_data), NULL);
}

static void
menu_about_cb (GSimpleAction *action,
               GVariant      *parameter,
               gpointer       user_data)
{
  gp_applet_show_about (GP_APPLET (user_data));
}

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
  sticky_notes_init (self);

  sticky_notes_applet_new (self);

  stickynotes_load (self);

  stickynotes_applet_update_menus (self);
  stickynotes_applet_update_tooltips (self);
}

static void
sticky_notes_applet_constructed (GObject *object)
{
  G_OBJECT_CLASS (sticky_notes_applet_parent_class)->constructed (object);
  sticky_notes_applet_setup (STICKY_NOTES_APPLET (object));
}

static void
sticky_notes_applet_dispose (GObject *object)
{
  StickyNotesApplet *self;

  self = STICKY_NOTES_APPLET (object);

  if (self->save_timeout_id != 0)
    {
      g_source_remove (self->save_timeout_id);
      self->save_timeout_id = 0;
    }

  if (self->notes != NULL)
    {
      stickynotes_save_now (self);

      g_list_free_full (self->notes, (GDestroyNotify) stickynote_free);
      self->notes = NULL;
    }

  g_clear_object (&self->icon_normal);
  g_clear_object (&self->icon_prelight);

  g_clear_pointer (&self->destroy_all_dialog, gtk_widget_destroy);
  g_clear_pointer (&self->w_prefs, gtk_widget_destroy);

  g_clear_object (&self->settings);

  G_OBJECT_CLASS (sticky_notes_applet_parent_class)->dispose (object);
}

static void
sticky_notes_applet_finalize (GObject *object)
{
  StickyNotesApplet *self;

  self = STICKY_NOTES_APPLET (object);

  g_clear_pointer (&self->filename, g_free);

  G_OBJECT_CLASS (sticky_notes_applet_parent_class)->finalize (object);
}

static void
sticky_notes_applet_class_init (StickyNotesAppletClass *self_class)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (self_class);

  object_class->constructed = sticky_notes_applet_constructed;
  object_class->dispose = sticky_notes_applet_dispose;
  object_class->finalize = sticky_notes_applet_finalize;
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

static void
filename_changed_cb (GSettings         *settings,
                     const gchar       *key,
                     StickyNotesApplet *self)
{
  char *filename;

  filename = g_settings_get_string (settings, key);

  if (g_strcmp0 (self->filename, filename) == 0)
    {
      g_free (filename);
      return;
    }

  g_free (filename);

  /* Save and remove existing notes */
  stickynotes_save_now (self);
  g_list_free_full (self->notes, (GDestroyNotify) stickynote_free);
  self->notes = NULL;

  /* Reload notes from new file */
  stickynotes_load (self);
}

static void
preferences_apply_cb (GSettings         *settings,
                      const gchar       *key,
                      StickyNotesApplet *self)
{
	GList *l;
	StickyNote *note;

	if (!strcmp (key, KEY_STICKY))
	{
		if (g_settings_get_boolean (settings, key))
			for (l = self->notes; l; l = l->next)
			{
				note = l->data;
				gtk_window_stick (GTK_WINDOW (note->w_window));
			}
		else
			for (l= self->notes; l; l = l->next)
			{
				note = l->data;
				gtk_window_unstick (GTK_WINDOW (
							note->w_window));
			}
	}

	else if (!strcmp (key, KEY_LOCKED))
	{
		for (l = self->notes; l; l = l->next)
		{
			note = l->data;
			stickynote_set_locked (note, g_settings_get_boolean (settings, key));
		}
		stickynotes_save (self);
	}

	else if (!strcmp (key, KEY_USE_SYSTEM_COLOR) ||
		 !strcmp (key, KEY_DEFAULT_FONT_COLOR) ||
		 !strcmp (key, KEY_DEFAULT_COLOR))
	{
		for (l = self->notes; l; l = l->next)
		{
			note = l->data;
			stickynote_set_color (note,
					note->color, note->font_color,
					FALSE);
		}
	}

	else if (!strcmp (key, KEY_USE_SYSTEM_FONT) ||
		 !strcmp (key, KEY_DEFAULT_FONT))
	{
		for (l = self->notes; l; l = l->next)
		{
			note = l->data;
			stickynote_set_font (note, note->font, FALSE);
		}
	}

	else if (!strcmp (key, KEY_FORCE_DEFAULT))
	{
		for (l = self->notes; l; l = l->next)
		{
			note = l->data;
			stickynote_set_color(note,
					note->color, note->font_color,
					FALSE);
			stickynote_set_font(note, note->font, FALSE);
		}
	}

	stickynotes_applet_update_menus (self);
}

static gboolean
get_desktop_window (Window *window)
{
	Window *desktop_window;
	GdkWindow *root_window;
	GdkAtom type_returned;
	int format_returned;
	int length_returned;

	root_window = gdk_screen_get_root_window (
				gdk_screen_get_default ());

	if (gdk_property_get (root_window,
			      gdk_atom_intern ("NAUTILUS_DESKTOP_WINDOW_ID", FALSE),
			      gdk_x11_xatom_to_atom (XA_WINDOW),
			      0, 4, FALSE,
			      &type_returned,
			      &format_returned,
			      &length_returned,
			      (guchar**) &desktop_window)) {
		*window = *desktop_window;
		g_free (desktop_window);
		return TRUE;
	}
	else {
		*window = 0;
		return FALSE;
	}
}

static GdkFilterReturn
desktop_window_event_filter (GdkXEvent *xevent,
                             GdkEvent  *event,
                             gpointer   data)
{
	StickyNotesApplet *self = data;
	gboolean desktop_hide = g_settings_get_boolean (self->settings, KEY_DESKTOP_HIDE);
	if (desktop_hide  &&
	    (((XEvent*)xevent)->xany.type == PropertyNotify) &&
	    (((XEvent*)xevent)->xproperty.atom == gdk_x11_get_xatom_by_name ("_NET_WM_USER_TIME"))) {
		stickynote_show_notes (self, FALSE);
	}
	return GDK_FILTER_CONTINUE;
}

static void
install_check_click_on_desktop (StickyNotesApplet *self)
{
	Window desktop_window;
	GdkWindow *window;
	Atom user_time_window;
	Atom user_time;

	if (!get_desktop_window (&desktop_window)) {
		return;
	}

	/* Access the desktop window. desktop_window is the root window for the
         * default screen, so we know using gdk_display_get_default() is correct. */
	window = gdk_x11_window_foreign_new_for_display (gdk_display_get_default (),
	                                                 desktop_window);

	/* It may contain an atom to tell us which other window to monitor */
	user_time_window = gdk_x11_get_xatom_by_name ("_NET_WM_USER_TIME_WINDOW");
	user_time = gdk_x11_get_xatom_by_name ("_NET_WM_USER_TIME");
	if (user_time != None  &&  user_time_window != None)
	{
		/* Looks like the atoms are there */
		Atom actual_type;
		int actual_format;
		gulong nitems;
		gulong bytes;
		Window *data;

		/* We only use this extra property if the actual user-time property's missing */
		XGetWindowProperty (GDK_DISPLAY_XDISPLAY (gdk_window_get_display (window)), desktop_window, user_time,
					0, 4, False, AnyPropertyType, &actual_type, &actual_format,
					&nitems, &bytes, (unsigned char **)&data );
		if (actual_type == None)
		{
			/* No user-time property, so look for the user-time-window */
			XGetWindowProperty (GDK_DISPLAY_XDISPLAY (gdk_window_get_display (window)), desktop_window, user_time_window,
					0, 4, False, AnyPropertyType, &actual_type, &actual_format,
					&nitems, &bytes, (unsigned char **)&data );
			if (actual_type != None)
			{
				/* We have another window to monitor */
				desktop_window = *data;
				window = gdk_x11_window_foreign_new_for_display (gdk_window_get_display (window),
				                                                 desktop_window);
			}
		}
	}

	gdk_window_set_events (window, GDK_PROPERTY_CHANGE_MASK);
	gdk_window_add_filter (window, desktop_window_event_filter, self);
}

static void
sticky_notes_init (StickyNotesApplet *self)
{
  self->settings = gp_applet_settings_new (GP_APPLET (self), STICKYNOTES_SCHEMA);

  g_signal_connect (self->settings,
                    "changed::" KEY_FILENAME,
                    G_CALLBACK (filename_changed_cb),
                    self);

  g_signal_connect (self->settings,
                    "changed",
                    G_CALLBACK (preferences_apply_cb),
                    self);

  self->max_height = 0.8 * gdk_screen_get_height (gdk_screen_get_default ());
  self->visible = TRUE;

  install_check_click_on_desktop (self);
}

static gboolean
applet_button_cb (GtkWidget         *widget,
                  GdkEventButton    *event,
                  StickyNotesApplet *applet)
{
	if (event->type == GDK_2BUTTON_PRESS)
	{
		stickynotes_add (applet);
		return TRUE;
	}
	else if (event->button == 1)
	{
		stickynote_toggle_notes_visible (applet);
		return TRUE;
	}

	return FALSE;
}

static gboolean
applet_key_cb (GtkWidget         *widget,
               GdkEventKey       *event,
               StickyNotesApplet *applet)
{
	switch (event->keyval)
	{
		case GDK_KEY_KP_Space:
		case GDK_KEY_space:
		case GDK_KEY_KP_Enter:
		case GDK_KEY_Return:
			stickynote_show_notes (applet, TRUE);
			return TRUE;

		default:
			break;
	}
	return FALSE;
}

static gboolean
applet_focus_cb (GtkWidget         *widget,
                 GdkEventFocus     *event,
                 StickyNotesApplet *applet)
{
	stickynotes_applet_update_icon(applet);

	return FALSE;
}

static gboolean
applet_cross_cb (GtkWidget         *widget,
                 GdkEventCrossing  *event,
                 StickyNotesApplet *applet)
{
	applet->prelighted = event->type == GDK_ENTER_NOTIFY || gtk_widget_has_focus(widget);

	stickynotes_applet_update_icon(applet);

	return FALSE;
}

static void
applet_size_allocate_cb (GtkWidget         *widget,
                         GtkAllocation     *allocation,
                         StickyNotesApplet *applet)
{
	if (applet->panel_orient == GTK_ORIENTATION_HORIZONTAL) {
	  if (applet->panel_size == allocation->height)
	    return;
	  applet->panel_size = allocation->height;
	} else {
	  if (applet->panel_size == allocation->width)
	    return;
	  applet->panel_size = allocation->width;
	}

	stickynotes_applet_update_icon(applet);
}

static void
applet_placement_changed_cb (GpApplet          *applet,
                             GtkOrientation     orientation,
                             GtkPositionType    position,
                             StickyNotesApplet *self)
{
	self->panel_orient = orientation;
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

	applet->icon_normal = gtk_icon_theme_load_icon (gtk_icon_theme_get_default (),
	                                                "gnome-sticky-notes-applet",
	                                                48,
	                                                0,
	                                                NULL);

	applet->icon_prelight = gdk_pixbuf_new (gdk_pixbuf_get_colorspace (applet->icon_normal),
	                                        gdk_pixbuf_get_has_alpha (applet->icon_normal),
	                                        gdk_pixbuf_get_bits_per_sample (applet->icon_normal),
	                                        gdk_pixbuf_get_width (applet->icon_normal),
	                                        gdk_pixbuf_get_height (applet->icon_normal));

	stickynotes_make_prelight_icon (applet->icon_prelight,
	                                applet->icon_normal,
	                                30);

	applet->destroy_all_dialog = NULL;
	applet->prelighted = FALSE;
	applet->pressed = FALSE;

	/* Add the applet icon */
	gtk_container_add(GTK_CONTAINER(applet), applet->w_image);
	applet->panel_orient = gp_applet_get_orientation (GP_APPLET (applet));

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
		pixbuf1 = gdk_pixbuf_scale_simple(applet->icon_prelight, size, size, GDK_INTERP_BILINEAR);
	else
		pixbuf1 = gdk_pixbuf_scale_simple(applet->icon_normal, size, size, GDK_INTERP_BILINEAR);

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
stickynotes_applet_update_menus (StickyNotesApplet *self)
{
  GAction *action;
  gboolean locked_writable;
  gboolean locked;

  action = gp_applet_menu_lookup_action (GP_APPLET (self), "lock");
  locked_writable = g_settings_is_writable (self->settings, KEY_LOCKED);
  locked = g_settings_get_boolean (self->settings, KEY_LOCKED);

  g_simple_action_set_enabled (G_SIMPLE_ACTION (action), locked_writable);
  g_simple_action_set_state (G_SIMPLE_ACTION (action), g_variant_new_boolean (locked));
}

void
stickynotes_applet_update_tooltips (StickyNotesApplet *self)
{
	int num;
	char *tooltip, *no_notes;

	num = g_list_length (self->notes);

	no_notes = g_strdup_printf (dngettext (GETTEXT_PACKAGE, "%d note", "%d notes", num), num);
	tooltip = g_strdup_printf ("%s\n%s", _("Show sticky notes"), no_notes);

	gtk_widget_set_tooltip_text (GTK_WIDGET (self), tooltip);

	g_free (tooltip);
	g_free (no_notes);
}

void
stickynotes_applet_panel_icon_get_geometry (StickyNotesApplet *applet,
                                            int               *x,
                                            int               *y,
                                            int               *width,
                                            int               *height)
{
	GtkWidget *widget;
        GtkAllocation allocation;
	GtkRequisition requisition;

	widget = GTK_WIDGET (applet->w_image);

	gtk_widget_get_preferred_size (widget, NULL, &requisition);

	gdk_window_get_origin (gtk_widget_get_window (widget), x, y);

	gtk_widget_get_allocation (widget, &allocation);
	*width = allocation.x;
	*height = allocation.y;
}

void
stickynotes_applet_setup_about (GtkAboutDialog *dialog)
{
  const char *comments;
  const char **authors;
  const char **documenters;
  const char *copyright;

  comments = _("Sticky Notes for the GNOME Desktop Environment");

  authors = (const char *[])
    {
      "Loban A Rahman <loban@earthling.net>",
      "Davyd Madeley <davyd@madeley.id.au>",
      NULL
    };

  documenters = (const char *[])
    {
      "Loban A Rahman <loban@earthling.net>",
      "Sun GNOME Documentation Team <gdocteam@sun.com>",
      NULL
    };

  copyright = "\xC2\xA9 2002-2003 Loban A Rahman, "
              "\xC2\xA9 2005 Davyd Madeley";

  gtk_about_dialog_set_comments (dialog, comments);

  gtk_about_dialog_set_authors (dialog, authors);
  gtk_about_dialog_set_documenters (dialog, documenters);
  gtk_about_dialog_set_translator_credits (dialog, _("translator-credits"));
  gtk_about_dialog_set_copyright (dialog, copyright);
}
