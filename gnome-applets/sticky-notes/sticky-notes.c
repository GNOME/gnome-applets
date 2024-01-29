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
#include "sticky-notes.h"

#include <libxml/parser.h>
#include <X11/Xatom.h>
#include <gdk/gdkx.h>
#include <glib/gi18n-lib.h>
#include <glib/gstdio.h>
#define WNCK_I_KNOW_THIS_IS_UNSTABLE 1
#include <libwnck/libwnck.h>
#include <string.h>

#include "util.h"
#include "sticky-notes-applet.h"
#include "gsettings.h"

/* Stop gcc complaining about xmlChar's signedness */
#define XML_CHAR(str) ((xmlChar *) (str))

#define STICKYNOTES_ICON_SIZE 8

static void
popup_create_cb (GSimpleAction *action,
                 GVariant      *parameter,
                 gpointer       user_data)
{
	StickyNote *note = (StickyNote *) user_data;
	stickynotes_add (note->applet);
}

static void
popup_destroy_cb (GSimpleAction *action,
                  GVariant      *parameter,
                  gpointer       user_data)
{
	StickyNote *note = (StickyNote *) user_data;
	stickynotes_remove (note);
}

static void
popup_toggle_lock_cb (GSimpleAction *action,
                      GVariant      *parameter,
                      gpointer       user_data)
{
	GVariant *state = g_action_get_state (G_ACTION (action));
	g_action_change_state (G_ACTION (action), g_variant_new_boolean (!g_variant_get_boolean (state)));
	g_variant_unref (state);
}

static void
popup_toggle_lock_state (GSimpleAction *action,
                         GVariant      *value,
                         gpointer       user_data)
{
	StickyNote *note = (StickyNote *) user_data;
	gboolean locked = g_variant_get_boolean (value);

	stickynote_set_locked (note, locked);
}

static void
popup_properties_cb (GSimpleAction *action,
                     GVariant      *parameter,
                     gpointer       user_data)
{
	StickyNote *note = (StickyNote *) user_data;
	stickynote_change_properties (note);
}

/* Popup menu on the sticky note */
static const GActionEntry stickynotes_note_menu_actions [] = {
	{ "create",     popup_create_cb,      NULL, NULL,    NULL },
	{ "lock",       popup_toggle_lock_cb, NULL, "false", popup_toggle_lock_state },
	{ "destroy",    popup_destroy_cb,     NULL, NULL,    NULL },
	{ "properties", popup_properties_cb,  NULL, NULL,    NULL }
};

static void
setup_note_menu (StickyNote *note)
{
	GSimpleActionGroup *action_group;
	GtkBuilder *builder;
	GMenu *gmenu;
	const gchar *resource_name;

	action_group = g_simple_action_group_new ();
	g_action_map_add_action_entries (G_ACTION_MAP (action_group),
	                                 stickynotes_note_menu_actions,
	                                 G_N_ELEMENTS (stickynotes_note_menu_actions),
	                                 note);

	resource_name = GRESOURCE_PREFIX "/ui/sticky-notes-note-menu.ui";
	builder = gtk_builder_new ();
	gtk_builder_set_translation_domain (builder, GETTEXT_PACKAGE);
	gtk_builder_add_from_resource (builder,
	                               resource_name,
	                               NULL);

	gmenu = G_MENU (gtk_builder_get_object (builder, "note-popup"));
	note->w_menu = gtk_menu_new_from_model (G_MENU_MODEL (gmenu));
	g_object_unref (builder);

	gtk_menu_attach_to_widget (GTK_MENU (note->w_menu), GTK_WIDGET (note->w_window), NULL);

	gtk_widget_insert_action_group (GTK_WIDGET (note->w_window), "stickynote",
	                                G_ACTION_GROUP (action_group));
	g_object_unref (action_group);
}

static void
set_image_from_name (GtkImage   *image,
                     const char *name)
{
  char *resource_path;
  GdkPixbuf *pixbuf;

  resource_path = g_build_filename (GRESOURCE_PREFIX "/icons/", name, NULL);
  pixbuf = gdk_pixbuf_new_from_resource_at_scale (resource_path,
                                                  STICKYNOTES_ICON_SIZE,
                                                  STICKYNOTES_ICON_SIZE,
                                                  TRUE,
                                                  NULL);

  g_free (resource_path);

  gtk_image_set_from_pixbuf (image, pixbuf);
  g_object_unref (pixbuf);
}

/* Based on a function found in wnck */
static void
set_icon_geometry  (GdkWindow *window,
                  int        x,
                  int        y,
                  int        width,
                  int        height)
{
      gulong data[4];
      Display *dpy;

      dpy = gdk_x11_display_get_xdisplay (gdk_window_get_display (window));

      data[0] = x;
      data[1] = y;
      data[2] = width;
      data[3] = height;

      XChangeProperty (dpy,
                       GDK_WINDOW_XID (window),
                       gdk_x11_get_xatom_by_name_for_display (
			       gdk_window_get_display (window),
			       "_NET_WM_ICON_GEOMETRY"),
		       XA_CARDINAL, 32, PropModeReplace,
                       (guchar *)&data, 4);
}

static gboolean
stickynote_show_popup_menu (GtkWidget      *widget,
                            GdkEventButton *event,
                            GtkWidget      *popup_menu)
{
	if (event->type == GDK_BUTTON_PRESS && event->button == 3)
	{
		gtk_menu_popup (GTK_MENU (popup_menu),
				NULL, NULL,
				NULL, NULL,
				event->button, event->time);
	}

	return FALSE;
}

static void
response_cb (GtkWidget  *dialog,
             gint        response_id,
             StickyNote *self)
{
  if (response_id == GTK_RESPONSE_HELP)
    gp_applet_show_help (GP_APPLET (self->applet), "stickynotes-settings-individual");
  else if (response_id == GTK_RESPONSE_CLOSE)
    gtk_widget_hide (dialog);
}

static gboolean
stickynote_toggle_lock_cb (GtkWidget  *widget,
                           StickyNote *note)
{
	stickynote_set_locked (note, !note->locked);

	return TRUE;
}

static gboolean
stickynote_close_cb (GtkWidget  *widget,
                     StickyNote *note)
{
	stickynotes_remove (note);

	return TRUE;
}

static gboolean
stickynote_resize_cb (GtkWidget      *widget,
                      GdkEventButton *event,
                      StickyNote     *note)
{
	if (event->type == GDK_BUTTON_PRESS && event->button == 1) {
		if (widget == note->w_resize_se)
			gtk_window_begin_resize_drag (GTK_WINDOW (note->w_window), GDK_WINDOW_EDGE_SOUTH_EAST,
						     event->button, event->x_root, event->y_root, event->time);
		else /* if (widget == note->w_resize_sw) */
			gtk_window_begin_resize_drag (GTK_WINDOW (note->w_window), GDK_WINDOW_EDGE_SOUTH_WEST,
						     event->button, event->x_root, event->y_root, event->time);
	}
	else
		return FALSE;

	return TRUE;
}

static gboolean
stickynote_move_cb (GtkWidget      *widget,
                    GdkEventButton *event,
                    StickyNote     *note)
{
	if (event->type == GDK_BUTTON_PRESS && event->button == 1)
		gtk_window_begin_move_drag (GTK_WINDOW (note->w_window), event->button,
		                            event->x_root, event->y_root, event->time);
	else if (event->type == GDK_2BUTTON_PRESS && event->button == 1)
		stickynote_change_properties (note);
	else
		return FALSE;

	return TRUE;
}

static gboolean
stickynote_configure_cb (GtkWidget         *widget,
                         GdkEventConfigure *event,
                         StickyNote        *note)
{
	note->x = event->x;
	note->y = event->y;
	note->w = event->width;
	note->h = event->height;

	stickynotes_save (note->applet);

	return FALSE;
}

static gboolean
stickynote_delete_cb (GtkWidget  *widget,
                      GdkEvent   *event,
                      StickyNote *note)
{
	stickynotes_remove(note);

	return TRUE;
}

static void
properties_apply_title_cb (StickyNote *note)
{
	stickynote_set_title (note, gtk_entry_get_text (GTK_ENTRY (note->w_entry)));
}

static void
properties_apply_color_cb (StickyNote *note)
{
	char *color_str = NULL;
	char *font_color_str = NULL;

	if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (note->w_def_color)))
	{
		GdkRGBA color;
		GdkRGBA font_color;

		gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER (note->w_color), &color);
		gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER (note->w_font_color), &font_color);

		color_str = gdk_rgba_to_string (&color);
		font_color_str = gdk_rgba_to_string (&font_color);
	}

	stickynote_set_color (note, color_str, font_color_str, TRUE);

	g_free (color_str);
	g_free (font_color_str);
}

static void
properties_color_cb (GtkWidget  *button,
                     StickyNote *note)
{
	properties_apply_color_cb (note);
}

static void
properties_font_cb (GtkWidget  *button,
                    StickyNote *note)
{
	const gchar *font_str;

	font_str = gtk_font_button_get_font_name (GTK_FONT_BUTTON (button));

	stickynote_set_font (note, font_str, TRUE);
}

static void
properties_apply_font_cb (StickyNote *note)
{
	const gchar *font_str = NULL;

	if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (note->w_def_font)))
		font_str = gtk_font_button_get_font_name (GTK_FONT_BUTTON (note->w_font));

	stickynote_set_font (note, font_str, TRUE);
}

static void
properties_activate_cb (GtkWidget  *widget,
                        StickyNote *note)
{
	gtk_dialog_response (GTK_DIALOG (note->w_properties), GTK_RESPONSE_CLOSE);
}

static gboolean
timeout_cb (gpointer user_data)
{
  StickyNote *note;

  note = user_data;
  note->buffer_changed_id = 0;

  stickynotes_save (note->applet);

  return G_SOURCE_REMOVE;
}

/* Called when a text buffer is changed.  */
static void
buffer_changed (GtkTextBuffer *buffer, StickyNote *note)
{
	if ((note->h + note->y) > note->applet->max_height)
		gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (note->w_scroller),
		                                GTK_POLICY_NEVER,
		                                GTK_POLICY_AUTOMATIC);

	/* When a buffer is changed, we set a 10 second timer.  When
	   the timer triggers, we will save the buffer if there have
	   been no subsequent changes.  */
	if (note->buffer_changed_id != 0)
		g_source_remove (note->buffer_changed_id);

	note->buffer_changed_id = g_timeout_add_seconds (10, timeout_cb, note);
}

/* Create a new (empty) Sticky Note at a specific position
   and with specific size */
static StickyNote *
stickynote_new_aux (StickyNotesApplet *applet,
                    gint               x,
                    gint               y,
                    gint               w,
                    gint               h)
{
	GdkScreen *screen;
	StickyNote *note;
	GtkBuilder *builder;
	static guint id = 0;

	screen = gtk_widget_get_screen (GTK_WIDGET (applet));

	note = g_new (StickyNote, 1);
	note->applet = applet;

	note->buffer_changed_id = 0;

	builder = gtk_builder_new ();
	gtk_builder_set_translation_domain (builder, GETTEXT_PACKAGE);
	gtk_builder_add_from_resource (builder,
	                               GRESOURCE_PREFIX "/ui/sticky-notes-note.ui",
	                               NULL);
	gtk_builder_add_from_resource (builder,
	                               GRESOURCE_PREFIX "/ui/sticky-notes-properties.ui",
	                               NULL);

	note->w_window = GTK_WIDGET (gtk_builder_get_object (builder, "stickynote_window"));
	gtk_window_set_screen(GTK_WINDOW(note->w_window),screen);
	gtk_window_set_decorated (GTK_WINDOW (note->w_window), FALSE);
	gtk_window_set_skip_taskbar_hint (GTK_WINDOW (note->w_window), TRUE);
	gtk_window_set_skip_pager_hint (GTK_WINDOW (note->w_window), TRUE);
	gtk_widget_add_events (note->w_window, GDK_BUTTON_PRESS_MASK);

	note->w_title = GTK_WIDGET (gtk_builder_get_object (builder, "title_label"));
	note->w_body = GTK_WIDGET (gtk_builder_get_object (builder, "body_text"));
	note->w_scroller = GTK_WIDGET (gtk_builder_get_object (builder, "body_scroller"));
	note->w_lock = GTK_WIDGET (gtk_builder_get_object (builder, "lock_button"));
	gtk_widget_add_events (note->w_lock, GDK_BUTTON_PRESS_MASK);

	note->w_close = GTK_WIDGET (gtk_builder_get_object (builder, "close_button"));
	gtk_widget_add_events (note->w_close, GDK_BUTTON_PRESS_MASK);
	note->w_resize_se = GTK_WIDGET (gtk_builder_get_object (builder, "resize_se_box"));
	gtk_widget_add_events (note->w_resize_se, GDK_BUTTON_PRESS_MASK);
	note->w_resize_sw = GTK_WIDGET (gtk_builder_get_object (builder, "resize_sw_box"));
	gtk_widget_add_events (note->w_resize_sw, GDK_BUTTON_PRESS_MASK);

	note->img_lock = GTK_IMAGE (gtk_builder_get_object (builder,
	                "lock_img"));
	note->img_close = GTK_IMAGE (gtk_builder_get_object (builder,
	                "close_img"));
	note->img_resize_se = GTK_IMAGE (gtk_builder_get_object (builder,
	                "resize_se_img"));
	note->img_resize_sw = GTK_IMAGE (gtk_builder_get_object (builder,
	                "resize_sw_img"));

	/* deal with RTL environments */
	gtk_widget_set_direction (GTK_WIDGET (gtk_builder_get_object (builder, "resize_bar")),
			GTK_TEXT_DIR_LTR);

	setup_note_menu (note);

	note->w_properties = GTK_WIDGET (gtk_builder_get_object (builder,
			"stickynote_properties"));
	gtk_window_set_screen (GTK_WINDOW (note->w_properties), screen);

	note->w_entry = GTK_WIDGET (gtk_builder_get_object (builder, "title_entry"));
	note->w_color = GTK_WIDGET (gtk_builder_get_object (builder, "note_color"));
	note->w_color_label = GTK_WIDGET (gtk_builder_get_object (builder, "color_label"));
	note->w_font_color = GTK_WIDGET (gtk_builder_get_object (builder, "font_color"));
	note->w_font_color_label = GTK_WIDGET (gtk_builder_get_object (builder,
			"font_color_label"));
	note->w_font = GTK_WIDGET (gtk_builder_get_object (builder, "note_font"));
	note->w_font_label = GTK_WIDGET (gtk_builder_get_object (builder, "font_label"));
	note->w_def_color = GTK_WIDGET (&GTK_CHECK_BUTTON (
				gtk_builder_get_object (builder,
					"def_color_check"))->toggle_button);
	note->w_def_font = GTK_WIDGET (&GTK_CHECK_BUTTON (
				gtk_builder_get_object (builder,
					"def_font_check"))->toggle_button);

	note->color = NULL;
	note->font_color = NULL;
	note->font = NULL;
	note->locked = FALSE;
	note->x = x;
	note->y = y;
	note->w = w;
	note->h = h;

	/* Customize the window */
	if (g_settings_get_boolean (applet->settings, KEY_STICKY))
		gtk_window_stick(GTK_WINDOW(note->w_window));

	if (w == 0 || h == 0)
		gtk_window_resize (GTK_WINDOW(note->w_window),
				g_settings_get_int (applet->settings, KEY_DEFAULT_WIDTH),
				g_settings_get_int (applet->settings, KEY_DEFAULT_HEIGHT));
	else
		gtk_window_resize (GTK_WINDOW(note->w_window),
				note->w,
				note->h);

	if (x != -1 && y != -1)
		gtk_window_move (GTK_WINDOW(note->w_window),
				note->x,
				note->y);

	/* Set the button images */
	set_image_from_name (note->img_close, "sticky-notes-stock-close.png");
	set_image_from_name (note->img_resize_se, "sticky-notes-stock-resize-se.png");
	set_image_from_name (note->img_resize_sw, "sticky-notes-stock-resize-sw.png");

	gtk_widget_show(note->w_lock);
	gtk_widget_show(note->w_close);
	gtk_widget_show(GTK_WIDGET (gtk_builder_get_object (builder, "resize_bar")));

	note->name = g_strdup_printf ("sticky-note-%d", id++);
	gtk_widget_set_name (note->w_window, note->name);

	note->css = gtk_css_provider_new ();
	gtk_style_context_add_provider_for_screen (screen,
	                                           GTK_STYLE_PROVIDER (note->css),
	                                           GTK_STYLE_PROVIDER_PRIORITY_USER + 100);

	/* Customize the title and colors, hide and unlock */
	stickynote_set_title(note, NULL);
	stickynote_set_color(note, NULL, NULL, TRUE);
	stickynote_set_font(note, NULL, TRUE);
	stickynote_set_locked(note, FALSE);

	gtk_widget_realize (note->w_window);

	g_signal_connect (G_OBJECT (note->w_window), "button-press-event",
			G_CALLBACK (stickynote_show_popup_menu), note->w_menu);

	g_signal_connect (G_OBJECT (note->w_lock), "button-press-event",
			G_CALLBACK (stickynote_show_popup_menu), note->w_menu);

	g_signal_connect (G_OBJECT (note->w_close), "button-press-event",
			G_CALLBACK (stickynote_show_popup_menu), note->w_menu);

	g_signal_connect (G_OBJECT (note->w_resize_se), "button-press-event",
			G_CALLBACK (stickynote_show_popup_menu), note->w_menu);

	g_signal_connect (G_OBJECT (note->w_resize_sw), "button-press-event",
			G_CALLBACK (stickynote_show_popup_menu), note->w_menu);

	/* Connect a properties dialog to the note */
	gtk_window_set_transient_for (GTK_WINDOW(note->w_properties),
			GTK_WINDOW(note->w_window));
	gtk_dialog_set_default_response (GTK_DIALOG(note->w_properties),
			GTK_RESPONSE_CLOSE);
	g_signal_connect (G_OBJECT (note->w_properties), "response",
			G_CALLBACK (response_cb), note);

	/* Connect signals to the sticky note */
	g_signal_connect (G_OBJECT (note->w_lock), "clicked",
			G_CALLBACK (stickynote_toggle_lock_cb), note);
	g_signal_connect (G_OBJECT (note->w_close), "clicked",
			G_CALLBACK (stickynote_close_cb), note);
	g_signal_connect (G_OBJECT (note->w_resize_se), "button-press-event",
			G_CALLBACK (stickynote_resize_cb), note);
	g_signal_connect (G_OBJECT (note->w_resize_sw), "button-press-event",
			G_CALLBACK (stickynote_resize_cb), note);

	g_signal_connect (G_OBJECT (note->w_window), "button-press-event",
			G_CALLBACK (stickynote_move_cb), note);
	g_signal_connect (G_OBJECT (note->w_window), "configure-event",
			G_CALLBACK (stickynote_configure_cb), note);
	g_signal_connect (G_OBJECT (note->w_window), "delete-event",
			G_CALLBACK (stickynote_delete_cb), note);

	g_signal_connect_swapped (G_OBJECT (note->w_entry), "changed",
			G_CALLBACK (properties_apply_title_cb), note);
	g_signal_connect (G_OBJECT (note->w_color), "color-set",
			G_CALLBACK (properties_color_cb), note);
	g_signal_connect (G_OBJECT (note->w_font_color), "color-set",
			G_CALLBACK (properties_color_cb), note);
	g_signal_connect_swapped (G_OBJECT (note->w_def_color), "toggled",
			G_CALLBACK (properties_apply_color_cb), note);
	g_signal_connect (G_OBJECT (note->w_font), "font-set",
			G_CALLBACK (properties_font_cb), note);
	g_signal_connect_swapped (G_OBJECT (note->w_def_font), "toggled",
			G_CALLBACK (properties_apply_font_cb), note);
	g_signal_connect (G_OBJECT (note->w_entry), "activate",
			G_CALLBACK (properties_activate_cb), note);
	g_signal_connect (G_OBJECT (note->w_properties), "delete-event",
			G_CALLBACK (gtk_widget_hide), note);

	g_object_unref(builder);

	g_signal_connect_after (note->w_body, "button-press-event",
	                        G_CALLBACK (gtk_true), note);

	g_signal_connect (gtk_text_view_get_buffer(GTK_TEXT_VIEW(note->w_body)),
			  "changed",
			  G_CALLBACK (buffer_changed), note);

	return note;
}

/* Create a new (empty) Sticky Note */
StickyNote *
stickynote_new (StickyNotesApplet *applet)
{
  return stickynote_new_aux (applet, -1, -1, 0, 0);
}

/* Destroy a Sticky Note */
void stickynote_free(StickyNote *note)
{
	gtk_widget_destroy(note->w_properties);
	gtk_widget_destroy(note->w_menu);
	gtk_widget_destroy(note->w_window);

	g_free (note->name);

	g_clear_object (&note->css);

	g_free(note->color);
	g_free(note->font_color);
	g_free(note->font);

	g_free(note);
}

/* Change the sticky note title and color */
void stickynote_change_properties (StickyNote *note)
{
	StickyNotesApplet *applet;
	GdkRGBA color, font_color;
	char *color_str = NULL;

	applet = note->applet;

	gtk_entry_set_text(GTK_ENTRY(note->w_entry),
			gtk_label_get_text (GTK_LABEL (note->w_title)));

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(note->w_def_color),
			note->color == NULL);

	if (note->color)
		color_str = g_strdup (note->color);
	else
	{
		color_str = g_settings_get_string (applet->settings, KEY_DEFAULT_COLOR);
	}

	if (!IS_STRING_EMPTY (color_str))
	{
		gdk_rgba_parse (&color, color_str);
		g_free (color_str);
		gtk_color_chooser_set_rgba (GTK_COLOR_CHOOSER (note->w_color), &color);
	}

	if (note->font_color)
		color_str = g_strdup (note->font_color);
	else
	{
		color_str = g_settings_get_string (applet->settings, KEY_DEFAULT_FONT_COLOR);
	}

	if (!IS_STRING_EMPTY (color_str))
	{
		gdk_rgba_parse (&font_color, color_str);
		g_free (color_str);
		gtk_color_chooser_set_rgba (GTK_COLOR_CHOOSER (note->w_font_color), &font_color);
	}

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(note->w_def_font),
			note->font == NULL);
	if (note->font)
		gtk_font_button_set_font_name (GTK_FONT_BUTTON (note->w_font),
				note->font);

	gtk_widget_show (note->w_properties);

	stickynotes_save (applet);
}

/* Check if a sticky note is empty */
gboolean stickynote_get_empty(const StickyNote *note)
{
	return gtk_text_buffer_get_char_count(gtk_text_view_get_buffer(GTK_TEXT_VIEW(note->w_body))) == 0;
}

/* Set the sticky note title */
void stickynote_set_title(StickyNote *note, const gchar *title)
{
	/* If title is NULL, use the current date as the title. */
	if (!title) {
		gchar *date_title, *tmp;
		gchar *date_format = g_settings_get_string (note->applet->settings, KEY_DATE_FORMAT);
		if (IS_STRING_EMPTY (date_format)) {
			g_free (date_format);
			date_format = g_strdup ("%x");
		}
		tmp = get_current_date (date_format);
		date_title = g_locale_to_utf8 (tmp, -1, NULL, NULL, NULL);

		gtk_window_set_title(GTK_WINDOW(note->w_window), date_title);
		gtk_label_set_text(GTK_LABEL (note->w_title), date_title);

		g_free (tmp);
		g_free(date_title);
		g_free(date_format);
	}
	else {
		gtk_window_set_title(GTK_WINDOW(note->w_window), title);
		gtk_label_set_text(GTK_LABEL (note->w_title), title);
	}
}

static void
append_background_color (StickyNote *note,
                         GSettings  *settings,
                         GString    *string)
{
  gchar *color;

  color = NULL;

  if (!note->color || g_settings_get_boolean (settings, KEY_FORCE_DEFAULT))
    {
      if (!g_settings_get_boolean (settings, KEY_USE_SYSTEM_COLOR))
        color = g_settings_get_string (settings, KEY_DEFAULT_COLOR);
    }
  else
    {
      color = g_strdup (note->color);
    }

  if (color == NULL)
    return;

  {
    GdkRGBA rgba;
    gchar *tmp;

    gdk_rgba_parse (&rgba, color);

    /* get darker color from the original */
    rgba.red = rgba.red * 9 / 10;
    rgba.green = rgba.green * 9 / 10;
    rgba.blue = rgba.blue * 9 / 10;

    tmp = gdk_rgba_to_string (&rgba);

    g_string_append_printf (string, "#%s,\n", note->name);
    g_string_append_printf (string, "#%s:backdrop {\n", note->name);
    g_string_append_printf (string, "\tbackground: %s;\n", tmp);
    g_string_append (string, "}\n");

    g_free (tmp);
  }

  g_string_append_printf (string, "#%s textview text {\n", note->name);
  g_string_append_printf (string, "\tbackground: %s;\n", color);
  g_string_append (string, "}\n");

  g_free (color);
}

static void
append_font_color (StickyNote *note,
                   GSettings  *settings,
                   GString    *string)
{
  gchar *font_color;

  font_color = NULL;

  if (!note->font_color || g_settings_get_boolean (settings, KEY_FORCE_DEFAULT))
    {
      if (!g_settings_get_boolean (settings, KEY_USE_SYSTEM_COLOR))
        font_color = g_settings_get_string (settings, KEY_DEFAULT_FONT_COLOR);
    }
  else
    {
      font_color = g_strdup (note->font_color);
    }

  if (font_color == NULL)
    return;

  g_string_append_printf (string, "#%s .title,\n", note->name);
  g_string_append_printf (string, "#%s .title:backdrop,\n", note->name);
  g_string_append_printf (string, "#%s textview text,\n", note->name);
  g_string_append_printf (string, "#%s textview text:backdrop {\n", note->name);
  g_string_append_printf (string, "\tcolor: %s;\n", font_color);
  g_string_append_printf (string, "\tcaret-color: %s;\n", font_color);
  g_string_append (string, "}\n");

  g_free (font_color);
}

static const gchar *
get_font_style_from_pango (PangoFontDescription *font_desc)
{
  PangoStyle style;
  const gchar *font_style;

  style = pango_font_description_get_style (font_desc);

  switch (style)
    {
      case PANGO_STYLE_OBLIQUE:
        font_style = "oblique";
        break;

      case PANGO_STYLE_ITALIC:
        font_style = "italic";
        break;

      case PANGO_STYLE_NORMAL:
      default:
        font_style = "normal";
        break;
    }

  return font_style;
}

static const gchar *
get_font_weight_from_pango (PangoFontDescription *font_desc)
{
  PangoWeight weight;
  const gchar *font_weight;

  weight = pango_font_description_get_weight (font_desc);

  switch (weight)
    {
      case PANGO_WEIGHT_THIN:
        font_weight = "100";
        break;

      case PANGO_WEIGHT_ULTRALIGHT:
        font_weight = "200";
        break;

      case PANGO_WEIGHT_LIGHT:
      case PANGO_WEIGHT_SEMILIGHT:
        font_weight = "300";
        break;

      case PANGO_WEIGHT_MEDIUM:
        font_weight = "500";
        break;

      case PANGO_WEIGHT_SEMIBOLD:
        font_weight = "600";
        break;

      case PANGO_WEIGHT_BOLD:
        font_weight = "700";
        break;

      case PANGO_WEIGHT_ULTRABOLD:
        font_weight = "800";
        break;

      case PANGO_WEIGHT_HEAVY:
      case PANGO_WEIGHT_ULTRAHEAVY:
        font_weight = "900";
        break;

      case PANGO_WEIGHT_NORMAL:
      case PANGO_WEIGHT_BOOK:
      default:
        font_weight = "400";
        break;
    }

  return font_weight;
}

static const gchar *
get_font_variant_from (PangoFontDescription *font_desc)
{
  PangoVariant variant;
  const gchar *font_variant;

  variant = pango_font_description_get_variant (font_desc);

  switch (variant)
    {
      case PANGO_VARIANT_SMALL_CAPS:
        font_variant = "small-caps";
        break;

#ifdef HAVE_PANGO_1_50_0
      case PANGO_VARIANT_ALL_SMALL_CAPS:
        font_variant = "all-small-caps";
        break;

      case PANGO_VARIANT_PETITE_CAPS:
        font_variant = "petite-caps";
        break;

      case PANGO_VARIANT_ALL_PETITE_CAPS:
        font_variant = "all-petite-caps";
        break;

      case PANGO_VARIANT_UNICASE:
        font_variant = "unicase";
        break;

      case PANGO_VARIANT_TITLE_CAPS:
        font_variant = "title-caps";
        break;
#endif

      case PANGO_VARIANT_NORMAL:
      default:
        font_variant = "normal";
        break;
    }

  return font_variant;
}

static void
append_font (StickyNote *note,
             GSettings  *settings,
             GString    *string)
{
  gchar *font;
  PangoFontDescription *font_desc;
  const gchar *font_family;
  gdouble font_size;
  const gchar *font_style;
  const gchar *font_weight;
  const gchar *font_variant;

  font = NULL;

  if (!note->font || g_settings_get_boolean (settings, KEY_FORCE_DEFAULT))
    {
      if (!g_settings_get_boolean (settings, KEY_USE_SYSTEM_FONT))
        font = g_settings_get_string (settings, KEY_DEFAULT_FONT);
    }
  else
    {
      font = g_strdup (note->font);
    }

  if (font == NULL)
    return;

  font_desc = pango_font_description_from_string (font);
  g_free (font);

  font_family = pango_font_description_get_family (font_desc);
  font_size = (gdouble) pango_font_description_get_size (font_desc) / PANGO_SCALE;
  font_style = get_font_style_from_pango (font_desc);
  font_weight = get_font_weight_from_pango (font_desc);
  font_variant = get_font_variant_from (font_desc);

  g_string_append_printf (string, "#%s .title,\n", note->name);
  g_string_append_printf (string, "#%s .title:backdrop,\n", note->name);
  g_string_append_printf (string, "#%s textview,\n", note->name);
  g_string_append_printf (string, "#%s textview:backdrop,\n", note->name);
  g_string_append_printf (string, "#%s textview text,\n", note->name);
  g_string_append_printf (string, "#%s textview text:backdrop {\n", note->name);
  g_string_append_printf (string, "\tfont-family: %s;\n", font_family);
  g_string_append_printf (string, "\tfont-size: %fpx;\n", font_size);
  g_string_append_printf (string, "\tfont-style: %s;\n", font_style);
  g_string_append_printf (string, "\tfont-weight: %s;\n", font_weight);
  g_string_append_printf (string, "\tfont-variant: %s;\n", font_variant);
  g_string_append (string, "}\n");

  pango_font_description_free (font_desc);
}

static void
update_css (StickyNote *note)
{
  GString *string;
  gchar *css;

  string = g_string_new (NULL);

  append_background_color (note, note->applet->settings, string);
  append_font_color (note, note->applet->settings, string);
  append_font (note, note->applet->settings, string);

  css = g_string_free (string, FALSE);
  gtk_css_provider_load_from_data (note->css, css, -1, NULL);
  g_free (css);
}

/* Set the sticky note color */
void
stickynote_set_color (StickyNote  *note,
		      const gchar *color_str,
		      const gchar *font_color_str,
		      gboolean     save)
{
	if (save) {
		if (note->color)
			g_free (note->color);
		if (note->font_color)
			g_free (note->font_color);

		note->color = color_str ?
			g_strdup (color_str) : NULL;
		note->font_color = font_color_str ?
			g_strdup (font_color_str) : NULL;

		gtk_widget_set_sensitive (note->w_color_label,
				note->color != NULL);
		gtk_widget_set_sensitive (note->w_font_color_label,
				note->font_color != NULL);
		gtk_widget_set_sensitive (note->w_color,
				note->color != NULL);
		gtk_widget_set_sensitive (note->w_font_color,
				note->color != NULL);
	}

	update_css (note);
}

/* Set the sticky note font */
void
stickynote_set_font (StickyNote *note, const gchar *font_str, gboolean save)
{
	if (save) {
		g_free (note->font);
		note->font = font_str ? g_strdup (font_str) : NULL;

		gtk_widget_set_sensitive (note->w_font_label, note->font != NULL);
		gtk_widget_set_sensitive(note->w_font, note->font != NULL);
	}

	update_css (note);
}

/* Lock/Unlock a sticky note from editing */
void
stickynote_set_locked (StickyNote *note,
                       gboolean    locked)
{
	note->locked = locked;

	/* Set cursor visibility and editability */
	gtk_text_view_set_editable(GTK_TEXT_VIEW(note->w_body), !locked);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(note->w_body), !locked);

	/* Show appropriate icon and tooltip */
	if (locked) {
		set_image_from_name (note->img_lock, "sticky-notes-stock-locked.png");
		gtk_widget_set_tooltip_text(note->w_lock, _("This note is locked."));
	}
	else {
		set_image_from_name (note->img_lock, "sticky-notes-stock-unlocked.png");
		gtk_widget_set_tooltip_text(note->w_lock, _("This note is unlocked."));
	}

	gtk_image_set_pixel_size (note->img_lock, STICKYNOTES_ICON_SIZE);

	stickynotes_applet_update_menus (note->applet);
}

/* Show/Hide a sticky note */
void
stickynote_set_visible (StickyNote *note, gboolean visible)
{
	if (visible)
	{
		gtk_window_present (GTK_WINDOW (note->w_window));

		if (note->x != -1 || note->y != -1)
			gtk_window_move (GTK_WINDOW (note->w_window),
					note->x, note->y);
		/* Put the note on all workspaces if necessary. */
		if (g_settings_get_boolean (note->applet->settings, KEY_STICKY))
			gtk_window_stick(GTK_WINDOW(note->w_window));
		else if (note->workspace > 0)
		{
#if 0
			WnckWorkspace *wnck_ws;
			gulong xid;
			WnckWindow *wnck_win;
			WnckScreen *wnck_screen;

			g_print ("set_visible(): workspace = %i\n",
					note->workspace);

			xid = GDK_WINDOW_XID (note->w_window->window);
			wnck_screen = wnck_screen_get_default ();
			wnck_win = wnck_window_get (xid);
			wnck_ws = wnck_screen_get_workspace (
					wnck_screen,
					note->workspace - 1);
			if (wnck_win && wnck_ws)
				wnck_window_move_to_workspace (
						wnck_win, wnck_ws);
			else
				g_print ("set_visible(): errr\n");
#endif
			xstuff_change_workspace (GTK_WINDOW (note->w_window),
					note->workspace - 1);
		}
	}
	else {
		/* Hide sticky note */
		int x, y, width, height;
		stickynotes_applet_panel_icon_get_geometry (note->applet, &x, &y, &width, &height);
		set_icon_geometry (gtk_widget_get_window (GTK_WIDGET (note->w_window)),
				   x, y, width, height);
		gtk_window_iconify(GTK_WINDOW (note->w_window));
	}
}

/* Add a sticky note */
void
stickynotes_add (StickyNotesApplet *applet)
{
	StickyNote *note;

	note = stickynote_new (applet);

	applet->notes = g_list_append(applet->notes, note);
	stickynotes_applet_update_tooltips(applet);
	stickynotes_save (applet);
	stickynote_set_visible (note, TRUE);
}

/* Remove a sticky note with confirmation, if needed */
void stickynotes_remove(StickyNote *note)
{
	GtkBuilder *builder;
	GtkWidget *dialog;

	builder = gtk_builder_new ();
	gtk_builder_set_translation_domain (builder, GETTEXT_PACKAGE);
	gtk_builder_add_from_resource (builder,
	                               GRESOURCE_PREFIX "/ui/sticky-notes-delete.ui",
	                               NULL);

	dialog = GTK_WIDGET (gtk_builder_get_object (builder, "delete_dialog"));

	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(note->w_window));

	if (stickynote_get_empty(note)
	    || !g_settings_get_boolean (note->applet->settings, KEY_CONFIRM_DELETION)
	    || gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {

		/* Remove the note from the linked-list of all notes */
		note->applet->notes = g_list_remove (note->applet->notes, note);

		/* Update tooltips */
		stickynotes_applet_update_tooltips(note->applet);

		/* Save notes */
		stickynotes_save (note->applet);

		stickynote_free (note);
	}

	gtk_widget_destroy(dialog);
	g_object_unref(builder);
}

/* Save all sticky notes in an XML configuration file */
void
stickynotes_save_now (StickyNotesApplet *applet)
{
	char *path;
	char *notes_file;
	xmlDocPtr doc;
	xmlNodePtr root;
	WnckHandle *wnck_handle;
	WnckScreen *wnck_screen;
	const gchar *title;
	GtkTextBuffer *buffer;
	GtkTextIter start, end;
	gchar *body;
	guint i;

	path = g_build_filename (g_get_user_config_dir (),
	                         "gnome-applets",
	                         "sticky-notes",
	                         NULL);

	g_mkdir_with_parents (path, 0700);
	notes_file = g_build_filename (path, applet->filename, NULL);
	g_free (path);

	if (applet->notes == NULL) {
		g_unlink (notes_file);
		g_free (notes_file);
		return;
	}

	/* Create a new XML document */
	doc = xmlNewDoc(XML_CHAR ("1.0"));
	root = xmlNewDocNode(doc, NULL, XML_CHAR ("stickynotes"), NULL);

	xmlDocSetRootElement(doc, root);
	xmlNewProp(root, XML_CHAR("version"), XML_CHAR (VERSION));

	wnck_handle = wnck_handle_new (WNCK_CLIENT_TYPE_APPLICATION);
	wnck_screen = wnck_handle_get_default_screen (wnck_handle);
	wnck_screen_force_update (wnck_screen);

	/* For all sticky notes */
	for (i = 0; i < g_list_length (applet->notes); i++) {
		WnckWindow *wnck_win;
		gulong xid = 0;

		/* Access the current note in the list */
		StickyNote *note = g_list_nth_data (applet->notes, i);

		/* Retrieve the window size of the note */
		gchar *w_str = g_strdup_printf("%d", note->w);
		gchar *h_str = g_strdup_printf("%d", note->h);

		/* Retrieve the window position of the note */
		gchar *x_str = g_strdup_printf("%d", note->x);
		gchar *y_str = g_strdup_printf("%d", note->y);

		xid = GDK_WINDOW_XID (gtk_widget_get_window (note->w_window));
		wnck_win = wnck_handle_get_window (wnck_handle, xid);

		if (!g_settings_get_boolean (note->applet->settings, KEY_STICKY) &&
			wnck_win)
			note->workspace = 1 +
				wnck_workspace_get_number (
				wnck_window_get_workspace (wnck_win));
		else
			note->workspace = 0;

		/* Retrieve the title of the note */
		title = gtk_label_get_text(GTK_LABEL(note->w_title));

		/* Retrieve body contents of the note */
		buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(note->w_body));

		gtk_text_buffer_get_bounds(buffer, &start, &end);
		body = gtk_text_iter_get_text(&start, &end);

		/* Save the note as a node in the XML document */
		{
			xmlNodePtr node = xmlNewTextChild(root, NULL, XML_CHAR ("note"),
					XML_CHAR (body));
			xmlNewProp(node, XML_CHAR ("title"), XML_CHAR (title));
			if (note->color)
				xmlNewProp (node, XML_CHAR ("color"), XML_CHAR (note->color));
			if (note->font_color)
				xmlNewProp (node, XML_CHAR ("font_color"),
						XML_CHAR (note->font_color));
			if (note->font)
				xmlNewProp (node, XML_CHAR ("font"), XML_CHAR (note->font));
			if (note->locked)
				xmlNewProp (node, XML_CHAR ("locked"), XML_CHAR ("true"));
			xmlNewProp (node, XML_CHAR ("x"), XML_CHAR (x_str));
			xmlNewProp (node, XML_CHAR ("y"), XML_CHAR (y_str));
			xmlNewProp (node, XML_CHAR ("w"), XML_CHAR (w_str));
			xmlNewProp (node, XML_CHAR ("h"), XML_CHAR (h_str));
			if (note->workspace > 0)
			{
				char *workspace_str;

				workspace_str = g_strdup_printf ("%i",
						note->workspace);
				xmlNewProp (node, XML_CHAR ("workspace"), XML_CHAR (workspace_str));
				g_free (workspace_str);
			}
		}

		/* Now that it has been saved, reset the modified flag */
		gtk_text_buffer_set_modified(buffer, FALSE);

		g_free(x_str);
		g_free(y_str);
		g_free(w_str);
		g_free(h_str);
		g_free(body);
	}

	g_clear_object (&wnck_handle);

	{
		char *tmp_file;

		tmp_file = g_strdup_printf ("%s.tmp", notes_file);
		if (xmlSaveFormatFile (tmp_file, doc, 1) == -1 ||
			g_rename (tmp_file, notes_file) == -1) {
			g_warning ("Failed to save notes");
			g_unlink (tmp_file);
		}

		g_free (tmp_file);
	}

	g_free (notes_file);
	xmlFreeDoc(doc);
}

static gboolean
stickynotes_save_cb (gpointer user_data)
{
  StickyNotesApplet *applet;

  applet = STICKY_NOTES_APPLET (user_data);
  applet->save_timeout_id = 0;

  stickynotes_save_now (applet);

  return G_SOURCE_REMOVE;
}

void
stickynotes_save (StickyNotesApplet *applet)
{
  /* If a save isn't already schedules, save everything a minute from now. */
  if (applet->save_timeout_id != 0)
    return;

  applet->save_timeout_id = g_timeout_add_seconds (60,
                                                   stickynotes_save_cb,
                                                   applet);
}

static char *
get_unique_filename (void)
{
  int number;
  char *filename;
  char *path;

  number = 1;
  filename = NULL;
  path = NULL;

  do
    {
      g_free (filename);
      filename = g_strdup_printf ("sticky-notes-%d.xml", number++);

      g_free (path);
      path = g_build_filename (g_get_user_config_dir (),
                               "gnome-applets",
                               "sticky-notes",
                               filename,
                               NULL);
    }
  while (g_file_test (path, G_FILE_TEST_EXISTS));

  g_free (path);

  return filename;
}

static char *
get_notes_file (StickyNotesApplet *applet)
{
  const char *dir;
  char *filename;
  char *notes_file;

  dir = g_get_user_config_dir ();
  filename = g_settings_get_string (applet->settings, KEY_FILENAME);

  g_free (applet->filename);

  if (*filename == '\0')
    {
      g_free (filename);
      filename = get_unique_filename ();

      notes_file = g_build_filename (dir,
                                     "gnome-applets",
                                     "sticky-notes",
                                     filename,
                                     NULL);



      applet->filename = filename;
      g_settings_set_string (applet->settings, KEY_FILENAME, filename);
    }
  else
    {
      applet->filename = filename;
      notes_file = g_build_filename (dir,
                                     "gnome-applets",
                                     "sticky-notes",
                                     filename,
                                     NULL);
    }

  return notes_file;
}

/**
 * The sticky-notes file had various locations in past versions of the applet.
 *
 * Until version 3.24 the location was: `~/.config/gnome-applets/stickynotes`.
 * In version 3.26 the location changed to: `<XDG_CONFIG_HOME>/gnome-applets/sticky-notes/sticky-notes.xml`.
 * In version 3.38 the location changed to: `<XDG_CONFIG_HOME>/gnome-applets/sticky-notes/sticky-notes-%d.xml`
 * where `%d` is incremented for each notes applet on the panel.
 */
static void
migrate_legacy_note_file (StickyNotesApplet *applet,
                          const char        *notes_file)
{
  const char *dir;
  char *old_file;

  dir = g_get_user_config_dir ();

  old_file = g_build_filename (dir,
                               "gnome-applets",
                               "sticky-notes",
                               "sticky-notes.xml",
                               NULL);

  if (g_file_test (old_file, G_FILE_TEST_EXISTS))
    {
      g_rename (old_file, notes_file);

      g_free (old_file);
      old_file = g_build_filename (dir,
                                   "gnome-applets",
                                   "stickynotes",
                                   NULL);

      if (g_file_test (old_file, G_FILE_TEST_EXISTS))
        g_unlink (old_file);
    }
  else
    {
      g_free (old_file);
      old_file = g_build_filename (dir,
                                   "gnome-applets",
                                   "stickynotes",
                                   NULL);

      if (g_file_test (old_file, G_FILE_TEST_EXISTS))
        g_rename (old_file, notes_file);
    }

  g_free (old_file);
}

/* Load all sticky notes from an XML configuration file */
void
stickynotes_load (StickyNotesApplet *applet)
{
	char *notes_file;
	xmlDocPtr doc;
	xmlNodePtr root;
	xmlNodePtr node;

	GList *new_notes, *tmp1;  /* Lists of StickyNote*'s */
	int x, y, w, h;

	notes_file = get_notes_file (applet);

	if (!g_file_test (notes_file, G_FILE_TEST_EXISTS))
		migrate_legacy_note_file (applet, notes_file);

	if (!g_file_test (notes_file, G_FILE_TEST_EXISTS)) {
		g_free (notes_file);
		return;
	}

	doc = xmlParseFile (notes_file);
	g_free (notes_file);

	/* If the XML file is corrupted/incorrect, create a blank one */
	root = xmlDocGetRootElement(doc);
	if (!root || xmlStrcmp(root->name, XML_CHAR ("stickynotes")))
	{
		xmlFreeDoc(doc);
		stickynotes_save (applet);
		return;
	}

	node = root->xmlChildrenNode;

	/* For all children of the root node (ie all sticky notes) */
	new_notes = NULL;

	while (node) {
		if (!xmlStrcmp(node->name, (const xmlChar *) "note"))
		{
			StickyNote *note;

			/* Retrieve and set the window size of the note */
			{
				gchar *w_str = (gchar *)xmlGetProp (node, XML_CHAR ("w"));
				gchar *h_str = (gchar *)xmlGetProp (node, XML_CHAR ("h"));
				if (w_str && h_str)
				{
					w = atoi (w_str);
					h = atoi (h_str);
				}
				else
				{
					w = 0;
					h = 0;
				}

				g_free (w_str);
				g_free (h_str);
			}

			/* Retrieve and set the window position of the note */
			{
				gchar *x_str = (gchar *)xmlGetProp (node, XML_CHAR ("x"));
				gchar *y_str = (gchar *)xmlGetProp (node, XML_CHAR ("y"));

				if (x_str && y_str)
				{
					x = atoi (x_str);
					y = atoi (y_str);
				}
				else
				{
					x = -1;
					y = -1;
				}

				g_free (x_str);
				g_free (y_str);
			}

			/* Create a new note */
			note = stickynote_new_aux (applet, x, y, w, h);
			applet->notes = g_list_append (applet->notes, note);
			new_notes = g_list_append (new_notes, note);

			/* Retrieve and set title of the note */
			{
				gchar *title = (gchar *)xmlGetProp(node, XML_CHAR ("title"));
				if (title)
					stickynote_set_title (note, title);
				g_free (title);
			}

			/* Retrieve and set the color of the note */
			{
				gchar *color_str;
				gchar *font_color_str;

				color_str = (gchar *)xmlGetProp (node, XML_CHAR ("color"));
				font_color_str = (gchar *)xmlGetProp (node, XML_CHAR ("font_color"));

				if (color_str || font_color_str)
					stickynote_set_color (note,
							color_str,
							font_color_str,
							TRUE);
				g_free (color_str);
				g_free (font_color_str);
			}

			/* Retrieve and set the font of the note */
			{
				gchar *font_str = (gchar *)xmlGetProp(node, XML_CHAR ("font"));
				if (font_str)
					stickynote_set_font (note, font_str,
							TRUE);
				g_free(font_str);
			}

			/* Retrieve the workspace */
			{
				char *workspace_str;

				workspace_str = (gchar *)xmlGetProp (node, XML_CHAR ("workspace"));
				if (workspace_str)
				{
					note->workspace = atoi (workspace_str);
					g_free (workspace_str);
				}
			}

			/* Retrieve and set (if any) the body contents of the
			 * note */
			{
				gchar *body = (gchar *)xmlNodeListGetString(doc,
						node->xmlChildrenNode, 1);
				if (body) {
					GtkTextBuffer *buffer;
					GtkTextIter start, end;

					buffer = gtk_text_view_get_buffer(
						GTK_TEXT_VIEW(note->w_body));
					gtk_text_buffer_get_bounds(
							buffer, &start, &end);
					gtk_text_buffer_insert(buffer,
							&start, body, -1);
				}
				g_free(body);
			}

			/* Retrieve and set the locked state of the note,
			 * by default unlocked */
			{
				gchar *locked = (gchar *)xmlGetProp(node, XML_CHAR ("locked"));
				if (locked)
					stickynote_set_locked(note,
						!strcmp(locked, "true"));
				g_free(locked);
			}
		}

		node = node->next;
	}

	tmp1 = new_notes;

	while (tmp1)
	{
		StickyNote *note = tmp1->data;

		stickynote_set_visible (note, applet->visible);
		tmp1 = tmp1->next;
	}

	g_list_free (new_notes);

	xmlFreeDoc(doc);
}
