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
	GladeXML *glade;		/* Glade object */

	GtkWidget *window;		/* Sticky Note window */
	GtkWidget *title;		/* Note title */
	GtkWidget *body;		/* Note text body */

	gint x;				/* Note x-coordinate */
	gint y;				/* Note y-coordinate */

	StickyNotesApplet *stickynotes;	/* The sticky notes applet */

} StickyNote;

StickyNote * stickynote_new(StickyNotesApplet *stickynotes);
void stickynote_free(StickyNote *note);

gboolean stickynote_get_empty(const StickyNote *note);

void stickynote_set_highlighted(StickyNote *note, gboolean highlighted);
void stickynote_set_title(StickyNote *note);

void stickynotes_add(StickyNotesApplet *stickynotes);
void stickynotes_remove(StickyNotesApplet *stickynotes, StickyNote *note);

void stickynotes_set_visible(StickyNotesApplet *stickynotes, gboolean visible);
void stickynotes_set_locked(StickyNotesApplet *stickynotes, gboolean locked);
void stickynotes_save(StickyNotesApplet *stickynotes);
void stickynotes_load(StickyNotesApplet *stickynotes);

#endif /* __STICKYNOTES_H__ */
