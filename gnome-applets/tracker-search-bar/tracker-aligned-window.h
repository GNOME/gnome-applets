/*
 * Copyright (C) 2009, Nokia <ivan.frade@nokia.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef __TRACKER_ALIGNED_WINDOW_H__
#define __TRACKER_ALIGNED_WINDOW_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define TRACKER_TYPE_ALIGNED_WINDOW (tracker_aligned_window_get_type ())
G_DECLARE_DERIVABLE_TYPE (TrackerAlignedWindow, tracker_aligned_window,
                          TRACKER, ALIGNED_WINDOW, GtkWindow)

struct _TrackerAlignedWindowClass
{
	GtkWindowClass parent_class;
};

GtkWidget *tracker_aligned_window_new        (GtkWidget            *align_widget);
void       tracker_aligned_window_set_widget (TrackerAlignedWindow *aligned_window,
                                              GtkWidget            *align_widget);
GtkWidget *tracker_aligned_window_get_widget (TrackerAlignedWindow *aligned_window);

G_END_DECLS

#endif /* __TRACKER_ALIGNED_WINDOW_H__ */
