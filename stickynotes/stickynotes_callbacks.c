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

#include <config.h>
#include <stickynotes_callbacks.h>

/* Sticky Window Callback : Resize the window. */
gboolean window_resize_cb(GtkWidget *widget, StickyNote *note)
{
//	gtk_window_begin_resize_drag(GTK_WINDOW(note->window), GDK_WINDOW_EDGE_NORTH_WEST, event->button, event->x_root, event->y_root, event->time);
	
	return TRUE;
}

/* Sticky Window Callback : Close the window. */
gboolean window_close_cb(GtkWidget *widget, StickyNote *note)
{
	stickynotes_remove(note->stickynotes, note);
	
	return TRUE;
}

/* Sticky Window Callback : Move the window or edit the title. */
gboolean window_move_cb(GtkWidget *widget, GdkEventButton *event, StickyNote *note)
{
	if (event->type == GDK_BUTTON_PRESS && event->button == 1)
		gtk_window_begin_move_drag(GTK_WINDOW(note->window), event->button, event->x_root, event->y_root, event->time);
	else if (event->type == GDK_2BUTTON_PRESS && event->button == 1)
		stickynote_edit_title(note);

	return TRUE;
}

/* Sticky Window Callback : Skip taskbar and pager when exposing the widow */
gboolean window_expose_cb(GtkWidget *widget, GdkEventExpose *event, StickyNote *note)
{
	gtk_window_set_skip_taskbar_hint(GTK_WINDOW(note->window), TRUE);
	gtk_window_set_skip_pager_hint(GTK_WINDOW(note->window), TRUE);

	return FALSE;
}

/* Sticky Window Callback : Store settings when resizing/moving the window */
gboolean window_configure_cb(GtkWidget *widget, GdkEventConfigure *event, StickyNote *note)
{
	note->x = event->x;
	note->y = event->y;
	note->w = event->width;
	note->h = event->height;

	stickynotes_save(note->stickynotes);

	return FALSE;
}

/* Sticky Window Callback : Get confirmation when deleting the window. */
gboolean window_delete_cb(GtkWidget *widget, GdkEvent *event, StickyNote *note)
{
	stickynotes_remove(note->stickynotes, note);

	return TRUE;
}

/* Sticky Window Callback : Cross (leave or enter) the window. */
gboolean window_cross_cb(GtkWidget *widget, GdkEventCrossing *event, StickyNote *note)
{
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(note->body));
	
	/* If the note was edited, save it. */
	if (event->type == GDK_LEAVE_NOTIFY && gtk_text_buffer_get_modified(buffer))
		stickynotes_save(note->stickynotes);
	
	/* Let other handlers receive this event. */
	return FALSE;
}

/* Sticky Window Callback : On focus (in and out) of the window */
gboolean window_focus_cb(GtkWidget *widget, GdkEventFocus *event, StickyNote *note)
{
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(note->body));
	
	/* If the note was edited, save it. */
	if (!event->in && gtk_text_buffer_get_modified(buffer))
		stickynotes_save(note->stickynotes);

	stickynote_set_highlighted(note, event->in);
	
	/* Let other handlers receive this event. */
	return FALSE;
}

/* Popup Menu Callback : Create a new sticky note */
void popup_create_cb(GtkWidget *widget, StickyNote *note)
{
	stickynotes_add(note->stickynotes);
}

/* Popup Menu Callback : Destroy selected sticky note */
void popup_destroy_cb(GtkWidget *widget, StickyNote *note)
{
	stickynotes_remove(note->stickynotes, note);
}

/* Popup Menu Callback : Edit the title */
void popup_edit_cb(GtkWidget *widget, StickyNote *note)
{
	stickynote_edit_title(note);
}

/* Callback for Dialog to change title */
void dialog_apply_cb(GtkWidget *widget, GtkDialog *dialog)
{
        gtk_dialog_response(dialog, GTK_RESPONSE_OK);
}
