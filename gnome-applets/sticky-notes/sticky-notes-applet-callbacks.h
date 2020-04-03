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

#ifndef __STICKYNOTES_APPLET_CALLBACKS_H__
#define __STICKYNOTES_APPLET_CALLBACKS_H__

#include "sticky-notes-applet.h"

/* Callbacks for the sticky notes applet */
gboolean applet_button_cb(GtkWidget *widget, GdkEventButton *event, StickyNotesApplet *applet);
gboolean applet_key_cb(GtkWidget *widget, GdkEventKey *event, StickyNotesApplet *applet);
gboolean applet_cross_cb(GtkWidget *widget, GdkEventCrossing *event, StickyNotesApplet *applet);
gboolean applet_focus_cb(GtkWidget *widget, GdkEventFocus *event, StickyNotesApplet *applet);
void install_check_click_on_desktop (void);
void applet_placement_changed_cb(GpApplet *applet, GtkOrientation orientation, GtkPositionType position, StickyNotesApplet *self);
void applet_size_allocate_cb(GtkWidget *widget, GtkAllocation *allocation, StickyNotesApplet *applet);
void applet_destroy_cb (GtkWidget *widget, StickyNotesApplet *applet);
/* Callbacks for sticky notes applet menu */
void menu_create_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data);
void menu_new_note_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data);
void menu_hide_notes_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data);
void menu_destroy_all_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data);
void menu_toggle_lock_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data);
void menu_toggle_lock_state(GSimpleAction *action, GVariant *value, gpointer user_data);
void menu_preferences_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data);
void menu_help_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data);

/* Callbacks for sticky notes preferences dialog */
void preferences_save_cb(gpointer data);
void preferences_color_cb (GtkWidget *button, gpointer data);
void preferences_font_cb (GtkWidget *button, gpointer data);
void preferences_apply_cb(GSettings *settings, const gchar *key, gpointer user_data);
void preferences_response_cb(GtkWidget *dialog, gint response, gpointer data);
gboolean preferences_delete_cb(GtkWidget *widget, GdkEvent *event, gpointer data);

#endif /* __STICKYNOTES_APPLET_CALLBACKS_H__ */
