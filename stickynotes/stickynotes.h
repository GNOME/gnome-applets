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

#ifndef __STICKYNOTES_H__
#define __STICKYNOTES_H__

typedef struct
{
	GladeXML *glade;		/* Glade object */

	GtkWidget *window;		/* Sticky Note window */
	GtkWidget *title;		/* Note title */
	GtkWidget *body;		/* Note text body */

	gint x;				/* Note x-coordinate */
	gint y;				/* Note y-coordinate */
} StickyNote;

StickyNote * stickynote_new();
void stickynote_free(StickyNote *note);

gboolean stickynote_get_empty(const StickyNote *note);

void stickynote_set_highlighted(StickyNote *note, gboolean highlighted);
void stickynote_edit_title(StickyNote *note);
void stickynote_remove(StickyNote *note);
    
void stickynotes_hide_all();
void stickynotes_show_all();
void stickynotes_lock_all();
void stickynotes_unlock_all();
void stickynotes_save_all();
void stickynotes_load_all();

#endif /* __STICKYNOTES_H__ */
