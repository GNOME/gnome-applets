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

#include <gnome.h>
#include <glade/glade.h>
#include <stickynotes_applet.h>

#ifndef __STICKYNOTES_APPLET_CALLBACKS_H__
#define __STICKYNOTES_APPLET_CALLBACKS_H__

/* Callbacks for the sticky notes applet */
gboolean applet_click_cb(GtkWidget *widget, GdkEventButton *event, PanelApplet *applet);
gboolean applet_resize_cb(GtkWidget *widget, gint size, PanelApplet *applet);
gboolean applet_cross_cb(GtkWidget *widget, GdkEventCrossing *event, PanelApplet *applet);
gboolean applet_focus_cb(GtkWidget *widget, GdkEventFocus *event, PanelApplet *applet);
gboolean applet_save_cb(PanelApplet *applet);

/* Callbacks for sticky notes applet menu */
void menu_create_cb(BonoboUIComponent *uic, StickyNotesApplet *sticky, const gchar *verbname);
void menu_destroy_all_cb(BonoboUIComponent *uic, StickyNotesApplet *sticky, const gchar *verbname);
void menu_hide_cb(BonoboUIComponent *uic, StickyNotesApplet *sticky, const gchar *verbname);
void menu_show_cb(BonoboUIComponent *uic, StickyNotesApplet *sticky, const gchar *verbname);
void menu_preferences_cb(BonoboUIComponent *uic, StickyNotesApplet *sticky, const gchar *verbname);
void menu_help_cb(BonoboUIComponent *uic, StickyNotesApplet *sticky, const gchar *verbname);
void menu_about_cb(BonoboUIComponent *uic, StickyNotesApplet *sticky, const gchar *verbname);

/* Callbacks for sticky notes preferences dialog */
void preferences_save_cb(GladeXML *glade);
void preferences_color_cb(GnomeColorPicker *cp, guint r, guint g, guint b, guint a, GladeXML *glade);
void preferences_response_cb(GtkDialog *dialog, gint response, GladeXML *glade);
void preferences_apply_cb(GConfClient *client, guint cnxn_id, GConfEntry *entry, StickyNotesApplet *sticky);

#endif /* __STICKYNOTES_APPLET_CALLBACKS_H__ */
