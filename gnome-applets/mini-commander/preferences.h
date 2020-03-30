/*
 * Copyright (C) 1998, 1999 Oliver Maruhn <oliver@maruhn.com>
 *               2002 Sun Microsystems Inc.
 *
 * Authors: Oliver Maruhn <oliver@maruhn.com>
 *          Mark McLoughlin <mark@skynet.ie>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __PREFERENCES_H__
#define __PREFERENCES_H__

#include <glib.h>

G_BEGIN_DECLS

#include <sys/types.h>
#include <regex.h>

#include <gtk/gtk.h>

typedef struct {
    char    *pattern;
    char    *command;
    regex_t  regex;
} MCMacro;

typedef struct {
    int     show_default_theme;
    int     auto_complete_history;

    int     normal_size_x;
    int     normal_size_y;
    int     panel_size_x;

    gchar  *cmd_line_color_fg;
    gchar  *cmd_line_color_bg;

    GSList *macros;

    guint   idle_macros_loader_id;

} MCPreferences;

typedef struct {
    GtkWidget    *dialog;
    GtkWidget    *auto_complete_history_toggle;
    GtkWidget    *size_spinner;
    GtkWidget    *use_default_theme_toggle;
    GtkWidget    *fg_color_picker;
    GtkWidget    *bg_color_picker;
    GtkWidget    *macros_tree;
    GtkWidget    *delete_button;
    GtkWidget    *add_button;

    GtkListStore *macros_store;

    GtkWidget    *macro_add_dialog;
    GtkWidget    *pattern_entry;
    GtkWidget    *command_entry;
} MCPrefsDialog;

#include "mini-commander-applet-private.h"

void       mc_load_preferences (MCData            *mc);
void       mc_show_preferences (GSimpleAction *action,
                                GVariant      *parameter,
                                gpointer       user_data);
void       mc_macros_free      (GSList            *macros);

G_END_DECLS

#endif
