/* -*- mode: C; c-basic-offset: 4 -*-
 * Drive Mount Applet
 * Copyright (c) 2004 Canonical Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License VERSION 2 as
 * published by the Free Software Foundation.  You are not allowed to
 * use any other version of the license; unless you got the explicit
 * permission from the author to do so.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author:
 *   James Henstridge <jamesh@canonical.com>
 */

#ifndef DRIVE_BUTTON_H
#define DRIVE_BUTTON_H

#include <gtk/gtk.h>
#include <libgnomevfs/gnome-vfs.h>

G_BEGIN_DECLS

#define DRIVE_TYPE_BUTTON         (drive_button_get_type ())
#define DRIVE_BUTTON(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), DRIVE_TYPE_BUTTON, DriveButton))
#define DRIVE_BUTTON_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST ((k), DRIVE_TYPE_BUTTON, DriveButtonClass))
#define DRIVE_IS_BUTTON(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), DRIVE_TYPE_BUTTON))
#define DRIVE_IS_BUTTON_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), DRIVE_TYPE_BUTTON))
#define DRIVE_BUTTON_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), DRIVE_TYPE_BUTTON, DriveButtonClass))

typedef struct _DriveButton      DriveButton;
typedef struct _DriveButtonClass DriveButtonClass;

struct _DriveButton
{
    GtkButton parent;

    GnomeVFSDrive *drive;
    GnomeVFSVolume *volume;
    int icon_size;
    GtkTooltips *tooltips;
    guint update_tag;
};

struct _DriveButtonClass
{
    GtkButtonClass parent;
};

GType      drive_button_get_type        (void);
GtkWidget *drive_button_new             (GnomeVFSDrive *drive,
					 GtkTooltips *tooltips);
GtkWidget *drive_button_new_from_volume (GnomeVFSVolume *volume,
					 GtkTooltips *tooltips);
void       drive_button_queue_update    (DriveButton *button);
void       drive_button_set_size        (DriveButton *button,
					 int          icon_size);

int        drive_button_compare         (DriveButton *button,
					 DriveButton *other_button);

G_END_DECLS

#endif /* DRIVE_BUTTON_H */
