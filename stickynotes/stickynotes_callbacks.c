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


/* Sticky Window Callback : Lock/Unlock the window */
gboolean stickynote_toggle_lock_cb(GtkWidget *widget, StickyNote *note)
{
	stickynote_set_locked(note, !note->locked);

	return TRUE;
}
/* Sticky Window Callback : Close the window. */
gboolean stickynote_close_cb(GtkWidget *widget, StickyNote *note)
{
	stickynotes_remove(note);
	
	return TRUE;
}

/* Sticky Window Callback : Resize the window. */
gboolean stickynote_resize_cb(GtkWidget *widget, GdkEventButton *event, StickyNote *note)
{
	if (event->type == GDK_BUTTON_PRESS && event->button == 1) {
		if (widget == note->w_resize_se)
			gtk_window_begin_resize_drag(GTK_WINDOW(note->w_window), GDK_WINDOW_EDGE_SOUTH_EAST,
						     event->button, event->x_root, event->y_root, event->time);
		else /* if (widget == note->w_resize_sw) */
			gtk_window_begin_resize_drag(GTK_WINDOW(note->w_window), GDK_WINDOW_EDGE_SOUTH_WEST,
						     event->button, event->x_root, event->y_root, event->time);
	}
	else
		return FALSE;
			
	return TRUE;
}

/* Sticky Window Callback : Move the window or edit the title. */
gboolean stickynote_move_cb(GtkWidget *widget, GdkEventButton *event, StickyNote *note)
{
	if (event->type == GDK_BUTTON_PRESS && event->button == 1)
		gtk_window_begin_move_drag(GTK_WINDOW(note->w_window), event->button, event->x_root, event->y_root, event->time);
	else if (event->type == GDK_2BUTTON_PRESS && event->button == 1)
		stickynote_change_properties(note);
	else
		return FALSE;

	return TRUE;
}

/* Sticky Window Callback : Skip taskbar and pager when exposing the widow */
gboolean stickynote_expose_cb(GtkWidget *widget, GdkEventExpose *event, StickyNote *note)
{
	gtk_window_set_skip_taskbar_hint(GTK_WINDOW(note->w_window), TRUE);
	gtk_window_set_skip_pager_hint(GTK_WINDOW(note->w_window), TRUE);
	
	return FALSE;
}

/* Sticky Window Callback : Store settings when resizing/moving the window */
gboolean stickynote_configure_cb(GtkWidget *widget, GdkEventConfigure *event, StickyNote *note)
{
	note->x = event->x;
	note->y = event->y;
	note->w = event->width;
	note->h = event->height;

	stickynotes_save();

	return FALSE;
}

/* Sticky Window Callback : Get confirmation when deleting the window. */
gboolean stickynote_delete_cb(GtkWidget *widget, GdkEvent *event, StickyNote *note)
{
	stickynotes_remove(note);

	return TRUE;
}

/* Sticky Window Callback : Cross (leave or enter) the window. */
gboolean stickynote_cross_cb(GtkWidget *widget, GdkEventCrossing *event, StickyNote *note)
{
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(note->w_body));
	
	/* If the note was edited, save it. */
	if (event->type == GDK_LEAVE_NOTIFY && gtk_text_buffer_get_modified(buffer))
		stickynotes_save();
	
	return FALSE;
}

/* Sticky Window Callback : On focus (in and out) of the window */
gboolean stickynote_focus_cb(GtkWidget *widget, GdkEventFocus *event, StickyNote *note)
{
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(note->w_body));
	
	/* If the note was edited, save it. */
	if (!event->in && gtk_text_buffer_get_modified(buffer))
		stickynotes_save();

	/* Show the resize bar if in focus */
	if (event->in) {
		gtk_widget_show(note->w_lock);
		gtk_widget_show(note->w_close);
		gtk_widget_show(glade_xml_get_widget(note->window, "resize_bar"));
	}
	else {
		gtk_widget_hide(note->w_lock);
		gtk_widget_hide(note->w_close);
		gtk_widget_hide(glade_xml_get_widget(note->window, "resize_bar"));
	}

	return FALSE;
}

/* Popup Menu Callback : Create a new sticky note */
void popup_create_cb(GtkWidget *widget, StickyNote *note)
{
	stickynotes_add();
}

/* Popup Menu Callback : Destroy selected sticky note */
void popup_destroy_cb(GtkWidget *widget, StickyNote *note)
{
	stickynotes_remove(note);
}

/* Popup Menu Callback : Lock/Unlock selected sticky note */
void popup_toggle_lock_cb(GtkWidget *widget, StickyNote *note)
{
	stickynote_set_locked(note, gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget)));
}

/* Popup Menu Callback : Change sticky note properties */
void popup_properties_cb(GtkWidget *widget, StickyNote *note)
{
	stickynote_change_properties(note);
}

/* Properties Dialog Callback : Apply title */
void properties_apply_title_cb(StickyNote *note)
{
	stickynote_set_title(note, gtk_entry_get_text(GTK_ENTRY(note->w_entry)));
}

/* Properties Dialog Callback : Apply color */
void properties_apply_color_cb(StickyNote *note)
{
	g_free(note->color);
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(note->w_default)))
		note->color = NULL;
	else {
		GdkColor color;
		
		gnome_color_picker_get_i16(GNOME_COLOR_PICKER(note->w_color), &color.red, &color.green, &color.blue, NULL);
		note->color = g_strdup_printf("#%.2x%.2x%.2x", color.red / 256, color.green / 256, color.blue / 256);
	}
	
	gtk_widget_set_sensitive(glade_xml_get_widget(note->properties, "color_label"), note->color != NULL);
	gtk_widget_set_sensitive(note->w_color, note->color != NULL);

	stickynote_set_color(note, note->color);
}

/* Properties Dialog Callback : Color */
void properties_color_cb(GnomeColorPicker *cp, guint r, guint g, guint b, guint a, StickyNote *note)
{
	/* Reduce RGB from 16-bit to 8-bit values and calculate HTML-style hex specification for the color */
	g_free(note->color);
	note->color = g_strdup_printf("#%.2x%.2x%.2x", r / 256, g / 256, b / 256);

	stickynote_set_color(note, note->color);
}

/* Properties Dialog Callback : Activate */
void properties_activate_cb(GtkWidget *widget, StickyNote *note)
{
        gtk_dialog_response(GTK_DIALOG(note->w_properties), GTK_RESPONSE_CLOSE);
}

/* Properties Dialog Callback : Response. */
void properties_response_cb(GtkDialog *dialog, gint response, StickyNote *note)
{
	gtk_widget_hide(GTK_WIDGET(dialog));
}
