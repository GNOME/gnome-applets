/*
 * Copyright (C) 2015 Alberts Muktupāvels
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

#ifndef WP_PREFERENCES_DIALOG_H
#define WP_PREFERENCES_DIALOG_H

#include <gtk/gtk.h>

#define KEY_SHOW_ALL_WINDOWS       "show-all-windows"
#define KEY_SHOW_APPLICATION_TITLE "show-application-title"
#define KEY_SHOW_HOME_TITLE        "show-home-title"
#define KEY_ICONS_GREYSCALE        "icons-greyscale"
#define KEY_EXPAND_TASK_LIST       "expand-task-list"

G_BEGIN_DECLS

#define WP_TYPE_PREFERENCES_DIALOG wp_preferences_dialog_get_type ()
G_DECLARE_FINAL_TYPE (WpPreferencesDialog, wp_preferences_dialog,
                      WP, PREFERENCES_DIALOG, GtkDialog)

GtkWidget *wp_preferences_dialog_new (GSettings *settings);

G_END_DECLS

#endif
