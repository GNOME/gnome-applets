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

#ifndef __STICKYNOTES_CALLBACKS_H__
#define __STICKYNOTES_CALLBACKS_H__

#include <stickynotes.h>

/* Callbacks for the sticky notes windows */
gboolean window_expose_cb(GtkWidget *widget, GdkEventExpose *event, StickyNote *note);
gboolean window_delete_cb(GtkWidget *widget, GdkEvent *event, StickyNote *note);
gboolean window_move_edit_cb(GtkWidget *widget, GdkEventButton *event, StickyNote *note);
gboolean window_resize_cb(GtkWidget *widget, GdkEventButton *event, StickyNote *note);
gboolean window_close_cb(GtkWidget *widget, GdkEventButton *event, StickyNote *note);
gboolean window_cross_cb(GtkWidget *widget, GdkEventCrossing *event, StickyNote *note);
gboolean window_focus_cb(GtkWidget *widget, GdkEventFocus *event, StickyNote *note);

/* Callbacks for the sticky notes popup menu */
void popup_create_cb(GtkWidget *widget, StickyNote *note);
void popup_destroy_cb(GtkWidget *widget, StickyNote *note);
void popup_edit_cb(GtkWidget *widget, StickyNote *note);

/* Callbacks for dialogs */
void dialog_apply_cb(GtkWidget *widget, GtkDialog *dialog);

#endif /* __STICKYNOTES_CALLBACKS_H__ */
