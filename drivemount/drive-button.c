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

#include "drive-button.h"
#include <glib/gi18n.h>

/* type registration boilerplate code */
G_DEFINE_TYPE(DriveButton, drive_button, GTK_TYPE_BUTTON);

static void     drive_button_set_drive    (DriveButton    *self,
				           GnomeVFSDrive  *drive);
static void     drive_button_set_volume   (DriveButton    *self,
				           GnomeVFSVolume *volume);
static void     drive_button_destroy      (GtkObject      *object);
static gboolean drive_button_button_press (GtkWidget      *widget,
					   GdkEventButton *event);
static void     drive_button_clicked      (GtkButton      *button);

static void
drive_button_class_init (DriveButtonClass *class)
{
    GTK_OBJECT_CLASS(class)->destroy = drive_button_destroy;
    GTK_WIDGET_CLASS(class)->button_press_event = drive_button_button_press;
    GTK_BUTTON_CLASS(class)->clicked = drive_button_clicked;
}

static void
drive_button_init (DriveButton *self)
{
    GtkWidget *image;

    image = gtk_image_new ();
    gtk_container_add (GTK_CONTAINER (self), image);
    gtk_widget_show(image);

    self->drive = NULL;
    self->volume = NULL;
    self->icon_size = 24;
    self->tooltips = NULL;
    self->update_tag = 0;
}

GtkWidget *
drive_button_new (GnomeVFSDrive *drive, GtkTooltips *tooltips)
{
    DriveButton *self;

    self = g_object_new (DRIVE_TYPE_BUTTON, NULL);
    drive_button_set_drive (self, drive);

    if (tooltips)
	self->tooltips = g_object_ref (tooltips);

    return (GtkWidget *)self;
}

GtkWidget *
drive_button_new_from_volume (GnomeVFSVolume *volume, GtkTooltips *tooltips)
{
    DriveButton *self;

    self = g_object_new (DRIVE_TYPE_BUTTON, NULL);
    drive_button_set_volume (self, volume);

    if (tooltips)
	self->tooltips = g_object_ref (tooltips);

    return (GtkWidget *)self;
}

static void
drive_button_destroy(GtkObject *object)
{
    DriveButton *self = DRIVE_BUTTON (object);

    drive_button_set_drive (self, NULL);

    if (self->tooltips)
	g_object_unref (self->tooltips);
    self->tooltips = NULL;

    if (self->update_tag)
	g_source_remove (self->update_tag);
    self->update_tag = 0;

    if (GTK_OBJECT_CLASS (drive_button_parent_class)->destroy)
	(* GTK_OBJECT_CLASS (drive_button_parent_class)->destroy) (object);
}

static void
mount_result (gboolean succeeded,
	      char *error,
	      char *detailed_error,
	      gpointer data)
{
    GtkWidget *dialog, *hbox, *vbox, *image, *label;
    char *title, *str;

    if (!succeeded) {
	switch (GPOINTER_TO_INT(data)) {
	case 1: /* eject error */
	    title = _("Eject Error");
	    break;
	case 2: /* unmount error */
	    title = _("Unmount Error");
	    break;
	case 3: /* mount error */
	    title = _("Mount Error");
	    break;
	default:
	    title = _("Error");
	}

	dialog = gtk_dialog_new ();
	atk_object_set_role (gtk_widget_get_accessible (dialog), ATK_ROLE_ALERT);
	gtk_window_set_title (GTK_WINDOW (dialog), title);
	gtk_dialog_set_has_separator (GTK_DIALOG (dialog), FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (dialog), 5);
	gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);

	gtk_box_set_spacing (GTK_BOX (GTK_DIALOG (dialog)->vbox), 14);
	hbox = gtk_hbox_new (FALSE, 12);
	gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
	gtk_widget_show (hbox);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), hbox,
			    FALSE, FALSE, 0);

	image = gtk_image_new_from_stock (GTK_STOCK_DIALOG_ERROR,
					  GTK_ICON_SIZE_DIALOG);
	gtk_misc_set_alignment (GTK_MISC (image), 0.5, 0.0);
	gtk_widget_show (image);
	gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);

	vbox = gtk_vbox_new (FALSE, 12);
	gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);
	gtk_widget_show (vbox);

	str = g_strconcat ("<span weight=\"bold\" size=\"larger\">",
			   error, "</span>", NULL);
	label = gtk_label_new (str);
	gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
	gtk_widget_show (label);
	g_free (str);

	label = gtk_label_new (detailed_error);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
	gtk_widget_show (label);

	gtk_dialog_add_button (GTK_DIALOG (dialog), GTK_STOCK_OK,
			       GTK_RESPONSE_OK);
	gtk_dialog_set_default_response (GTK_DIALOG (dialog),
					 GTK_RESPONSE_YES);

	gtk_widget_show (dialog);
	g_signal_connect (dialog, "response",
			  G_CALLBACK (gtk_object_destroy), NULL);

    }
}

static gboolean
drive_button_button_press (GtkWidget      *widget,
			   GdkEventButton *event)
{
    /* don't consume non-button1 presses */
    if (event->button != 1) {
	return FALSE;
    }

    return (* GTK_WIDGET_CLASS (drive_button_parent_class)->
	    button_press_event) (widget, event);
}

static void
drive_button_clicked (GtkButton *button)
{
    DriveButton *self = DRIVE_BUTTON (button);
    GnomeVFSDeviceType device_type;

    if (self->drive) {
	if (!gnome_vfs_drive_is_connected (self->drive)) return;

	device_type = gnome_vfs_drive_get_device_type (self->drive);
	if (gnome_vfs_drive_is_mounted (self->drive)) {
	    switch (device_type) {
	    case GNOME_VFS_DEVICE_TYPE_CDROM:
	    case GNOME_VFS_DEVICE_TYPE_ZIP:
	    case GNOME_VFS_DEVICE_TYPE_JAZ:
		gnome_vfs_drive_eject (self->drive, mount_result,
				       GINT_TO_POINTER(1));
		break;
	    default:
		gnome_vfs_drive_unmount (self->drive, mount_result,
					 GINT_TO_POINTER(2));
		break;
	    }
	} else {
	    gnome_vfs_drive_mount (self->drive, mount_result,
				   GINT_TO_POINTER(3));
	}
    } else if (self->volume) {
	device_type = gnome_vfs_volume_get_device_type (self->volume);
	if (gnome_vfs_volume_is_mounted (self->volume)) {
	    switch (device_type) {
	    case GNOME_VFS_DEVICE_TYPE_CDROM:
	    case GNOME_VFS_DEVICE_TYPE_ZIP:
	    case GNOME_VFS_DEVICE_TYPE_JAZ:
		gnome_vfs_volume_eject (self->volume, mount_result,
					GINT_TO_POINTER(1));
		break;
	    default:
		gnome_vfs_volume_unmount (self->volume, mount_result,
					  GINT_TO_POINTER(2));
		break;
	    }
	}
    }
}

static void
drive_button_set_drive (DriveButton *self, GnomeVFSDrive *drive)
{
    g_return_if_fail (DRIVE_IS_BUTTON (self));

    if (self->drive) {
	gnome_vfs_drive_unref (self->drive);
    }
    self->drive = NULL;
    if (self->volume) {
	gnome_vfs_volume_unref (self->volume);
    }
    self->volume = NULL;

    if (drive) {
	self->drive = gnome_vfs_drive_ref (drive);
    }
    drive_button_queue_update (self);
}

static void
drive_button_set_volume (DriveButton *self, GnomeVFSVolume *volume)
{
    g_return_if_fail (DRIVE_IS_BUTTON (self));

    if (self->drive) {
	gnome_vfs_drive_unref (self->drive);
    }
    self->drive = NULL;
    if (self->volume) {
	gnome_vfs_volume_unref (self->volume);
    }
    self->volume = NULL;

    if (volume) {
	self->volume = gnome_vfs_volume_ref (volume);
    }
    drive_button_queue_update (self);
}

static gboolean
drive_button_update (gpointer user_data)
{
    DriveButton *self;
    GdkScreen *screen;
    GtkIconTheme *icon_theme;
    char *icon_name;
    int width, height;
    GdkPixbuf *pixbuf;
    GtkRequisition button_req, image_req;

    g_return_val_if_fail (DRIVE_IS_BUTTON (user_data), FALSE);
    self = DRIVE_BUTTON (user_data);
    self->update_tag = 0;

    /* if no drive or volume, unset image */
    if (!self->drive && !self->volume) {
	if (GTK_BIN (self)->child != NULL)
	    gtk_image_set_from_pixbuf (GTK_IMAGE (GTK_BIN (self)->child), NULL);
	return FALSE;
    }


    if (self->tooltips) {
	char *display_name, *tip;

	if (self->drive) {
	    display_name = gnome_vfs_drive_get_display_name (self->drive);
	    if (gnome_vfs_drive_is_mounted (self->drive))
		tip = g_strdup_printf (_("%s\n(mounted)"), display_name);
	    else if (gnome_vfs_drive_is_connected (self->drive))
		tip = g_strdup_printf (_("%s\n(not mounted)"), display_name);
	    else
		tip = g_strdup_printf (_("%s\n(not connected)"), display_name);
	} else {
	    display_name = gnome_vfs_volume_get_display_name (self->volume);
	    if (gnome_vfs_volume_is_mounted (self->volume))
		tip = g_strdup_printf (_("%s\n(mounted)"), display_name);
	    else
		tip = g_strdup_printf (_("%s\n(not mounted)"), display_name);
	}

	gtk_tooltips_set_tip (self->tooltips, GTK_WIDGET (self), tip, NULL);
	g_free (tip);
	g_free (display_name);
    }

    /* get the icon for the first mounted volume, or the drive itself */
    if (self->drive) {
	if (gnome_vfs_drive_is_mounted (self->drive)) {
	    GList *volumes;
	    GnomeVFSVolume *volume;

	    volumes = gnome_vfs_drive_get_mounted_volumes (self->drive);
	    volume = GNOME_VFS_VOLUME (volumes->data);
	    icon_name = gnome_vfs_volume_get_icon (volume);
	    g_list_foreach (volumes, (GFunc)gnome_vfs_volume_unref, NULL);
	    g_list_free (volumes);
	} else {
	    icon_name = gnome_vfs_drive_get_icon (self->drive);
	}
    } else {
	icon_name = gnome_vfs_volume_get_icon (self->volume);
    }

    /* base the icon size on the desired button size */
    gtk_widget_size_request (GTK_WIDGET (self), &button_req);
    gtk_widget_size_request (GTK_BIN (self)->child, &image_req);
    width = self->icon_size - (button_req.width - image_req.width);
    height = self->icon_size - (button_req.height - image_req.height);

    screen = gtk_widget_get_screen (GTK_WIDGET (self));
    icon_theme = gtk_icon_theme_get_for_screen (screen);
    pixbuf = gtk_icon_theme_load_icon (icon_theme, icon_name,
				       MIN (width, height), 0, NULL);
    g_free (icon_name);

    if (!pixbuf)
	return FALSE;

    gtk_image_set_from_pixbuf (GTK_IMAGE (GTK_BIN (self)->child), pixbuf);
    g_object_unref (pixbuf);

    gtk_widget_size_request (GTK_WIDGET (self), &button_req);

    return FALSE;
}

void
drive_button_queue_update (DriveButton *self)
{
    if (!self->update_tag) {
	self->update_tag = g_idle_add (drive_button_update, self);
    }
}

void
drive_button_set_size (DriveButton *self, int icon_size)
{
    g_return_if_fail (DRIVE_IS_BUTTON (self));

    if (self->icon_size != icon_size) {
	self->icon_size = icon_size;
	drive_button_queue_update (self);
    }
}

int
drive_button_compare (DriveButton *button, DriveButton *other_button)
{
    /* sort drives before driveless volumes volumes */
    if (button->drive) {
	if (other_button->drive)
	    return gnome_vfs_drive_compare (button->drive, other_button->drive);
	else
	    return -1;
    } else {
	if (other_button->drive)
	    return 1;
	else
	    return gnome_vfs_volume_compare (button->volume, other_button->volume);
    }
}
