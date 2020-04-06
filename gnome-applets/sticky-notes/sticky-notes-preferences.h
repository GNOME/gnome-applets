/*
 * Copyright (C) 2020 Alberts Muktupāvels
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef STICKY_NOTES_PREFERENCES_H
#define STICKY_NOTES_PREFERENCES_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define STICKY_NOTES_TYPE_PREFERENCES (sticky_notes_preferences_get_type ())
G_DECLARE_FINAL_TYPE (StickyNotesPreferences, sticky_notes_preferences,
                      STICKY_NOTES, PREFERENCES, GtkDialog)

GtkWidget *sticky_notes_preferences_new (GSettings *settings);

G_END_DECLS

#endif
