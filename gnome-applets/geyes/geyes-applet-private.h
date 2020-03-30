/*
 * Copyright (C) 1999 Dave Camp <dave@davec.dhs.org>
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
 *  
 */

#ifndef GEYES_APPLET_PRIVATE_H
#define GEYES_APPLET_PRIVATE_H

#include "geyes-applet.h"

#include <gdk-pixbuf/gdk-pixbuf.h>

#define GEYES_PREFS_SCHEMA "org.gnome.gnome-applets.geyes"
#define KEY_THEME_PATH     "theme-path"

#define MAX_EYES 1000
typedef struct
{
	GtkWidget *pbox;
    
	gint selected_row;
} EyesPropertyBox;

struct _EyesApplet
{
	GpApplet parent;

	GtkWidget   *vbox;
	GtkWidget   *hbox;
	GtkWidget   **eyes;
	guint        timeout_id;
	gint 	    *pointer_last_x;
	gint 	    *pointer_last_y;

	/* Theme */
	GdkPixbuf *eye_image;
	GdkPixbuf *pupil_image;
	gchar *theme_dir;
	gchar *theme_name;
	gchar *eye_filename;
	gchar *pupil_filename;
	gint num_eyes;
	gint eye_height;
	gint eye_width;
	gint pupil_height;
	gint pupil_width;
	gint wall_thickness;

	/* Properties */
	EyesPropertyBox prop_box;

	GSettings *settings;
};

/* eyes.c */
void setup_eyes   (EyesApplet *eyes_applet);

void destroy_eyes (EyesApplet *eyes_applet);


/* theme.c */
void theme_dirs_create (void);

int load_theme    (EyesApplet        *eyes_applet,
		   const gchar       *theme_dir);

void properties_cb (GSimpleAction *action,
                    GVariant      *parameter,
                    gpointer       user_data);

#endif
