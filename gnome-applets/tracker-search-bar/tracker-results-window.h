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

#ifndef __TRACKER_RESULTS_WINDOW_H__
#define __TRACKER_RESULTS_WINDOW_H__

#include "tracker-aligned-window.h"

G_BEGIN_DECLS

#define TRACKER_TYPE_RESULTS_WINDOW (tracker_results_window_get_type ())
G_DECLARE_FINAL_TYPE (TrackerResultsWindow, tracker_results_window,
                      TRACKER, RESULTS_WINDOW, TrackerAlignedWindow)

GtkWidget * tracker_results_window_new      (GtkWidget   *parent,
                                             const gchar *query);
void        tracker_results_window_popup    (TrackerResultsWindow *window);

G_END_DECLS

#endif /* __TRACKER_RESULTS_WINDOW_H__ */
