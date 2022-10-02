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

#ifndef __STICKYNOTES_APPLET_H__
#define __STICKYNOTES_APPLET_H__

#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>
#include <libgnome-panel/gp-applet.h>

#define IS_STRING_EMPTY(x) ((x) == NULL || (x)[0] == '\0')

#define STICKY_NOTES_TYPE_APPLET (sticky_notes_applet_get_type ())
G_DECLARE_FINAL_TYPE (StickyNotesApplet, sticky_notes_applet,
                      STICKY_NOTES, APPLET, GpApplet)

struct _StickyNotesApplet
{
  GpApplet parent;

	GSettings *settings;

	char *filename;

	GtkWidget *w_image;		/* The applet icon */

	GdkPixbuf *icon_normal;		/* Normal applet icon */
	GdkPixbuf *icon_prelight;	/* Prelighted applet icon */

	GtkWidget *destroy_all_dialog;	/* The applet it's destroy all dialog */

	gboolean prelighted;		/* Whether applet is prelighted */
	gboolean pressed;		/* Whether applet is pressed */

	gint panel_size;
	GtkOrientation panel_orient;

	GtkWidget *w_prefs;

	guint save_timeout_id;

	GList *notes;

	gint max_height;

	gboolean visible;
};

void stickynotes_applet_update_icon             (StickyNotesApplet *self);
void stickynotes_applet_update_menus            (StickyNotesApplet *self);
void stickynotes_applet_update_tooltips         (StickyNotesApplet *self);

void stickynotes_applet_panel_icon_get_geometry (StickyNotesApplet *self,
                                                 int               *x,
                                                 int               *y,
                                                 int               *width,
                                                 int               *height);

void stickynotes_applet_setup_about (GtkAboutDialog *dialog);

#endif /* __STICKYNOTES_APPLET_H__ */
