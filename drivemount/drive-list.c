/* -*- mode: C; c-basic-offset: 4 -*-
 * Drive Mount Applet
 * Copyright (c) 2004 Canonical Ltd
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "drive-list.h"
#include "drive-button.h"
#include <glib/gi18n.h>

G_DEFINE_TYPE (DriveList, drive_list, GTK_TYPE_TABLE);

static GnomeVFSVolumeMonitor *volume_monitor = NULL;

static void drive_list_finalize (GObject *object);
static void drive_list_destroy  (GtkObject *object);
static void drive_list_add      (GtkContainer *container, GtkWidget *child);
static void drive_list_remove   (GtkContainer *container, GtkWidget *child);

static void volume_mounted     (GnomeVFSVolumeMonitor *monitor,
				GnomeVFSVolume *volume,
				DriveList *self);
static void volume_unmounted   (GnomeVFSVolumeMonitor *monitor,
				GnomeVFSVolume *volume,
				DriveList *self);
static void drive_connected    (GnomeVFSVolumeMonitor *monitor,
				GnomeVFSDrive *drive,
				DriveList *self);
static void drive_disconnected (GnomeVFSVolumeMonitor *monitor,
				GnomeVFSDrive *drive,
				DriveList *self);
static void add_drive          (DriveList *self,
				GnomeVFSDrive *drive);
static void remove_drive       (DriveList *self,
				GnomeVFSDrive *drive);
static void add_volume         (DriveList *self,
				GnomeVFSVolume *volume);
static void remove_volume      (DriveList *self,
				GnomeVFSVolume *volume);

static void
drive_list_class_init (DriveListClass *class)
{
    G_OBJECT_CLASS (class)->finalize = drive_list_finalize;
    GTK_OBJECT_CLASS (class)->destroy = drive_list_destroy;
    GTK_CONTAINER_CLASS (class)->add = drive_list_add;
    GTK_CONTAINER_CLASS (class)->remove = drive_list_remove;
}

static void
drive_list_init (DriveList *self)
{
    GList *connected_drives, *mounted_volumes, *tmp;

    gtk_table_set_homogeneous (GTK_TABLE (self), TRUE);

    self->drives = g_hash_table_new (NULL, NULL);
    self->volumes = g_hash_table_new (NULL, NULL);
    self->orientation = GTK_ORIENTATION_HORIZONTAL;
    self->tooltips = gtk_tooltips_new ();
    g_object_ref (self->tooltips);
    gtk_object_sink (GTK_OBJECT (self->tooltips));
    self->layout_tag = 0;
    self->icon_size = 24;

    /* listen for drive connects/disconnects, and add
     * currently connected drives. */
    if (!volume_monitor)
	volume_monitor = gnome_vfs_get_volume_monitor ();

    g_signal_connect_object (volume_monitor, "volume_mounted",
			     G_CALLBACK (volume_mounted), self, 0);
    g_signal_connect_object (volume_monitor, "volume_unmounted",
			     G_CALLBACK (volume_unmounted), self, 0);
    g_signal_connect_object (volume_monitor, "drive_connected",
			     G_CALLBACK (drive_connected), self, 0);
    g_signal_connect_object (volume_monitor, "drive_disconnected",
			     G_CALLBACK (drive_disconnected), self, 0);
    connected_drives = gnome_vfs_volume_monitor_get_connected_drives (volume_monitor);
    for (tmp = connected_drives; tmp != NULL; tmp = tmp->next) {
	GnomeVFSDrive *drive = tmp->data;

	add_drive (self, drive);
	gnome_vfs_drive_unref (drive);
    }
    g_list_free (connected_drives);

    mounted_volumes = gnome_vfs_volume_monitor_get_mounted_volumes (volume_monitor);
    for (tmp = mounted_volumes; tmp != NULL; tmp = tmp->next) {
	GnomeVFSVolume *volume = tmp->data;

	add_volume (self, volume);
	gnome_vfs_volume_unref (volume);
    }
    g_list_free (mounted_volumes);
}

GtkWidget *
drive_list_new (void)
{
    return g_object_new (DRIVE_TYPE_LIST, NULL);
}

static void
drive_list_finalize (GObject *object)
{
    DriveList *self = DRIVE_LIST (object);

    g_hash_table_destroy (self->drives);

    if (G_OBJECT_CLASS (drive_list_parent_class)->finalize)
	(* G_OBJECT_CLASS (drive_list_parent_class)->finalize) (object);
}

static void
drive_list_destroy (GtkObject *object)
{
    DriveList *self = DRIVE_LIST (object);

    if (self->tooltips)
	g_object_unref (self->tooltips);
    self->tooltips = NULL;

    g_signal_handlers_disconnect_by_func (volume_monitor,
					  G_CALLBACK (volume_mounted), self);
    g_signal_handlers_disconnect_by_func (volume_monitor,
					  G_CALLBACK (volume_unmounted), self);
    g_signal_handlers_disconnect_by_func (volume_monitor,
					  G_CALLBACK (drive_connected), self);
    g_signal_handlers_disconnect_by_func (volume_monitor,
					  G_CALLBACK (drive_disconnected), self);

    if (self->layout_tag)
	g_source_remove (self->layout_tag);
    self->layout_tag = 0;

    if (GTK_OBJECT_CLASS (drive_list_parent_class)->destroy)
	(* GTK_OBJECT_CLASS (drive_list_parent_class)->destroy) (object);
}

static void
drive_list_add (GtkContainer *container, GtkWidget *child)
{
    DriveList *self;
    DriveButton *button;

    g_return_if_fail (DRIVE_IS_LIST (container));
    g_return_if_fail (DRIVE_IS_BUTTON (child));

    if (GTK_CONTAINER_CLASS (drive_list_parent_class)->add)
	(* GTK_CONTAINER_CLASS (drive_list_parent_class)->add) (container,
								child);

    self = DRIVE_LIST (container);
    button = DRIVE_BUTTON (child);
    if (button->drive)
	g_hash_table_insert (self->drives, button->drive, button);
    else
	g_hash_table_insert (self->volumes, button->volume, button);
}

static void
drive_list_remove (GtkContainer *container, GtkWidget *child)
{
    DriveList *self;
    DriveButton *button;

    g_return_if_fail (DRIVE_IS_LIST (container));
    g_return_if_fail (DRIVE_IS_BUTTON (child));

    self = DRIVE_LIST (container);
    button = DRIVE_BUTTON (child);
    if (button->drive)
	g_hash_table_remove (self->drives, button->drive);
    else
	g_hash_table_remove (self->volumes, button->volume);

    if (GTK_CONTAINER_CLASS (drive_list_parent_class)->remove)
	(* GTK_CONTAINER_CLASS (drive_list_parent_class)->remove) (container,
								   child);
}

static void
list_buttons (gpointer key, gpointer value, gpointer user_data)
{
    GtkWidget *button = value;
    GList **sorted_buttons = user_data;

    *sorted_buttons = g_list_insert_sorted (*sorted_buttons, button,
					    (GCompareFunc)drive_button_compare);
}

static gboolean
relayout_buttons (gpointer data)
{
    DriveList *self = DRIVE_LIST (data);
    GList *sorted_buttons = NULL, *tmp;
    int i;


    self->layout_tag = 0;
    g_hash_table_foreach (self->drives, list_buttons, &sorted_buttons);
    g_hash_table_foreach (self->volumes, list_buttons, &sorted_buttons);

    /* position buttons in the table according to their sorted order */
    for (tmp = sorted_buttons, i = 0; tmp != NULL; tmp = tmp->next, i++) {
	GtkWidget *button = tmp->data;
    
	if (self->orientation == GTK_ORIENTATION_HORIZONTAL) {
	    gtk_container_child_set (GTK_CONTAINER (self), button,
				     "left_attach", i, "right_attach", i+1,
				     "top_attach", 0, "bottom_attach", 1,
				     "x_options", GTK_FILL,
				     "y_options", GTK_FILL,
				     NULL);
	} else {
	    gtk_container_child_set (GTK_CONTAINER (self), button,
				     "left_attach", 0, "right_attach", 1,
				     "top_attach", i, "bottom_attach", i+1,
				     "x_options", GTK_FILL,
				     "y_options", GTK_FILL,
				     NULL);
	}
    }
    /* shrink wrap the table */
    gtk_table_resize (GTK_TABLE (self), 1, 1);

    return FALSE;
}

static void
queue_relayout (DriveList *self)
{
    if (!self->layout_tag) {
	self->layout_tag = g_idle_add (relayout_buttons, self);
    }
}

static void
volume_mounted (GnomeVFSVolumeMonitor *monitor,
		GnomeVFSVolume *volume,
		DriveList *self)
{
    GnomeVFSDrive *drive;
    DriveButton *button = NULL;;

    add_volume (self, volume);

    drive = gnome_vfs_volume_get_drive (volume);
    if (drive) {
	button = g_hash_table_lookup (self->drives, drive);
	gnome_vfs_drive_unref (drive);
    } else {
	button = g_hash_table_lookup (self->volumes, volume);
    }
    if (button)
	drive_button_queue_update (button);
}

static void
volume_unmounted (GnomeVFSVolumeMonitor *monitor,
		  GnomeVFSVolume *volume,
		  DriveList *self)
{
    GnomeVFSDrive *drive;
    DriveButton *button = NULL;;

    remove_volume (self, volume);

    drive = gnome_vfs_volume_get_drive (volume);
    if (drive) {
	button = g_hash_table_lookup (self->drives, drive);
	gnome_vfs_drive_unref (drive);
    } else {
	button = g_hash_table_lookup (self->volumes, volume);
    }
    if (button)
	drive_button_queue_update (button);
}

static void
drive_connected (GnomeVFSVolumeMonitor *monitor,
		 GnomeVFSDrive *drive,
		 DriveList *self)
{
    add_drive (self, drive);
}

static void
drive_disconnected (GnomeVFSVolumeMonitor *monitor,
		    GnomeVFSDrive *drive,
		    DriveList *self)
{
    remove_drive (self, drive);
}

static void
add_drive (DriveList *self, GnomeVFSDrive *drive)
{
    GtkWidget *button;

    /* ignore non user visible drives */
    if (!gnome_vfs_drive_is_user_visible (drive))
	return;

    /* if the drive has already been added, return */
    if (g_hash_table_lookup (self->drives, drive) != NULL)
	return;

    button = drive_button_new (drive, self->tooltips);
    drive_button_set_size (DRIVE_BUTTON (button), self->icon_size);
    gtk_container_add (GTK_CONTAINER (self), button);
    gtk_widget_show (button);
    queue_relayout (self);
}

static void
remove_drive (DriveList *self, GnomeVFSDrive *drive)
{
    GtkWidget *button;

    /* if the drive has already been added, return */
    button = g_hash_table_lookup (self->drives, drive);
    if (button) {
	gtk_container_remove (GTK_CONTAINER (self), button);
	queue_relayout (self);
    }
}

static void
add_volume (DriveList *self, GnomeVFSVolume *volume)
{
    GtkWidget *button;
    GnomeVFSDrive *drive;

    /* ignore non user visible volumes */
    if (!gnome_vfs_volume_is_user_visible (volume))
	return;

    /* ignore "connected servers" style volumes */
    if (gnome_vfs_volume_get_volume_type (volume)
	== GNOME_VFS_VOLUME_TYPE_CONNECTED_SERVER)
	return;

    /* ignore volumes attached to a drive */
    drive = gnome_vfs_volume_get_drive (volume);
    if (drive) {
	gnome_vfs_drive_unref (drive);
	return;
    }

    /* if the volume has already been added, return */
    if (g_hash_table_lookup (self->volumes, volume) != NULL)
	return;

    button = drive_button_new_from_volume (volume, self->tooltips);
    drive_button_set_size (DRIVE_BUTTON (button), self->icon_size);
    gtk_container_add (GTK_CONTAINER (self), button);
    gtk_widget_show (button);
    queue_relayout (self);
}

static void
remove_volume (DriveList *self, GnomeVFSVolume *volume)
{
    GtkWidget *button;

    /* if the drive has already been added, return */
    button = g_hash_table_lookup (self->volumes, volume);
    if (button) {
	gtk_container_remove (GTK_CONTAINER (self), button);
	queue_relayout (self);
    }
}

void
drive_list_set_orientation (DriveList *self,
			    GtkOrientation orientation)
{
    g_return_if_fail (DRIVE_IS_LIST (self));

    if (orientation != self->orientation) {
	self->orientation = orientation;
	queue_relayout (self);
    }
}

static void
set_icon_size (gpointer key, gpointer value, gpointer user_data)
{
    DriveButton *button = value;
    DriveList *self = user_data;

    drive_button_set_size (button, self->icon_size);
}


void
drive_list_set_panel_size (DriveList *self, int panel_size)
{
    g_return_if_fail (DRIVE_IS_LIST (self));

    if (self->icon_size != panel_size) {
	self->icon_size = panel_size;
	g_hash_table_foreach (self->drives, set_icon_size, self);
	g_hash_table_foreach (self->volumes, set_icon_size, self);
    }
}
