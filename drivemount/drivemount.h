/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <glib.h>

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>
#include <panel-applet.h>

#ifndef _DRIVEMOUNT_APPLET_H_
#define _DRIVEMOUNT_APPLET_H_

G_BEGIN_DECLS

#define ICON_HEIGHT 10
#define ICON_WIDTH  40

typedef struct _DriveData DriveData;

struct _DriveData
{
	GtkWidget *applet;
	GtkWidget *button;
	GtkWidget *button_pixmap;
	gint device_pixmap;
	gint timeout_id;
	gint interval;
	gint mounted;
	gchar *mount_base;
	gchar *mount_point;
	gint scale_applet;
	gint auto_eject;
	gint autofs_friendly;
	PanelAppletOrient orient;
	GtkTooltips *tooltips;
	gint sizehint;

	gchar *custom_icon_in;
	gchar *custom_icon_out;

	GtkWidget *error_dialog;
	GtkWidget *about_dialog;
};

void redraw_pixmap(DriveData *dd);
void start_callback_update(DriveData *dd);
GtkWidget* hig_dialog_new (GtkWindow      *parent,
			   GtkDialogFlags flags,
			   GtkMessageType type,
			   GtkButtonsType buttons,
			   const gchar    *header,
			   const gchar    *message);

G_END_DECLS
#endif
