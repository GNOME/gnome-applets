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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include <stickynotes_applet.h>

#ifndef __STICKYNOTES_APPLET_CALLBACKS_H__
#define __STICKYNOTES_APPLET_CALLBACKS_H__

/* Callbacks for the sticky notes applet */
gboolean applet_button_cb(GtkWidget *widget, GdkEventButton *event, StickyNotesApplet *applet);
gboolean applet_key_cb(GtkWidget *widget, GdkEventKey *event, StickyNotesApplet *applet);
gboolean applet_cross_cb(GtkWidget *widget, GdkEventCrossing *event, StickyNotesApplet *applet);
gboolean applet_focus_cb(GtkWidget *widget, GdkEventFocus *event, StickyNotesApplet *applet);
gboolean applet_save_cb(StickyNotesApplet *applet);
void applet_change_orient_cb(PanelApplet *panel_applet, PanelAppletOrient orient, StickyNotesApplet *applet);
void applet_size_allocate_cb(GtkWidget *widget, GtkAllocation *allocation, StickyNotesApplet *applet);
gboolean applet_change_bg_cb(PanelApplet *panel_applet, PanelAppletBackgroundType type, GdkColor *color, GdkPixmap *pixmap,
			     StickyNotesApplet *applet);
void applet_destroy_cb (PanelApplet *panel_applet, StickyNotesApplet *applet);

/* Callbacks for sticky notes applet menu */
void menu_create_cb(BonoboUIComponent *uic, StickyNotesApplet *applet, const gchar *verbname);
void menu_destroy_all_cb(BonoboUIComponent *uic, StickyNotesApplet *applet, const gchar *verbname);
void menu_toggle_show_cb(BonoboUIComponent *uic, const gchar *path, Bonobo_UIComponent_EventType type, const gchar *state, StickyNotesApplet *applet);
void menu_toggle_lock_cb(BonoboUIComponent *uic, const gchar *path, Bonobo_UIComponent_EventType type, const gchar *state, StickyNotesApplet *applet);
void menu_preferences_cb(BonoboUIComponent *uic, StickyNotesApplet *applet, const gchar *verbname);
void menu_help_cb(BonoboUIComponent *uic, StickyNotesApplet *applet, const gchar *verbname);
void menu_about_cb(BonoboUIComponent *uic, StickyNotesApplet *applet, const gchar *verbname);

/* Callbacks for sticky notes preferences dialog */
void preferences_save_cb(gpointer data);
void preferences_color_cb(GnomeColorPicker *cp, guint r, guint g, guint b, guint a, gpointer data);
void preferences_font_cb(GnomeFontPicker *fp, gchar *font_str, gpointer data);
void preferences_apply_cb(GConfClient *client, guint cnxn_id, GConfEntry *entry, gpointer data);
void preferences_response_cb(GtkDialog *dialog, gint response, gpointer data);
gboolean preferences_delete_cb(GtkWidget *widget, GdkEvent *event, gpointer data);

#endif /* __STICKYNOTES_APPLET_CALLBACKS_H__ */
