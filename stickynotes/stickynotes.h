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

#ifndef __STICKYNOTES_H__
#define __STICKYNOTES_H__

#include <stickynotes_applet.h>

typedef struct
{
	GladeXML *window;		/* Glade object for note window */
	GladeXML *menu;			/* Glade object for popup menu */

	GtkWidget *w_window;		/* Sticky Note window */
	GtkWidget *w_menu;		/* Sticky Note menu */
	GtkWidget *w_title;		/* Sticky Note title */
	GtkWidget *w_body;		/* Sticky Note text body */
	GtkWidget *w_lock;		/* Sticky Note lock button */
	GtkWidget *w_close;		/* Sticky Note close button */
	GtkWidget *w_resize_se;		/* Sticky Note resize button (south east) */
	GtkWidget *w_resize_sw;		/* Sticky Note resize button (south west) */

	gchar *color;			/* Note color */
	gboolean locked;		/* Note locked state */
	gboolean visible;		/* Note visibility */

	gint x;				/* Note x-coordinate */
	gint y;				/* Note y-coordinate */
	gint w;				/* Note width */
	gint h;				/* Note height */

} StickyNote;

StickyNote * stickynote_new();
void stickynote_free(StickyNote *note);

gboolean stickynote_get_empty(const StickyNote *note);

void stickynote_set_color(StickyNote *note, const gchar* color_str);
void stickynote_set_title(StickyNote *note, const gchar* title);
void stickynote_set_locked(StickyNote *note, gboolean locked);
void stickynote_set_visible(StickyNote *note, gboolean visible);

void stickynote_change_title(StickyNote *note);
void stickynote_change_color(StickyNote *note);

void stickynotes_add();
void stickynotes_remove(StickyNote *note);
void stickynotes_save();
void stickynotes_load();

#endif /* __STICKYNOTES_H__ */
