/*
#
#   GNotes!
#   Copyright (C) 1998-1999  spoon@ix.netcom.com
#   Copyright (C) 1999 dres@debian.org
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef _GNOTE_H_
#define _GNOTE_H_

#include "config.h"
#include <gnome.h>
#include <applet-widget.h>

typedef struct _Gnote
{
    GtkWidget *window;
    GtkWidget *hbox;
    GtkWidget *handle_box;
    GtkWidget *text;
    GtkWidget *menu;
    gboolean  hidden;
    time_t    timestamp;
    gchar    *title;
    gboolean  already_saved;
    gchar    *type;
} GNote;

void gnote_action(GtkWidget *, gpointer);
GtkWidget *gnote_create_menu(GNote*);
void gnote_menu(GtkWidget *, GdkEventButton *);
gint gnote_handle_button_cb(GtkWidget *, GdkEventButton *, gpointer);
void gnote_new(gint, gint, gint, gint, gboolean, const gchar *,
               time_t, const gchar *, gboolean, const gchar *);
void gnote_new_cb(AppletWidget *, gpointer);
void gnote_destroy_cb(GtkWidget *, gpointer);
void gnote_delete_cb(GtkWidget *, gpointer);
void gnote_signal_handler(int);
gint gnote_motion_cb(GtkWidget *, GdkEventButton *event, gpointer);
gint gnote_handle_button_cb(GtkWidget *, GdkEventButton *event, gpointer);

void gnotes_raise(AppletWidget*, gpointer);
void gnote_raise(gpointer);
void gnotes_lower(AppletWidget*, gpointer);
void gnote_lower(gpointer);
void gnotes_show(AppletWidget*, gpointer);
void gnote_show(gpointer);
void gnotes_hide(AppletWidget*, gpointer);
void gnote_hide(gpointer);
void gnotes_save(AppletWidget*, gpointer);
void gnote_save(gpointer);
void gnotes_load(AppletWidget*, gpointer);


#endif /* _GNOTE_H_ */

