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
#include <gdk/gdkkeysyms.h>
#include <gconf/gconf-client.h>

#include <string.h>

enum {
    CMD_NONE,
    CMD_MOUNT_OR_PLAY,
    CMD_UNMOUNT,
    CMD_EJECT
};

#define GCONF_ROOT_AUTOPLAY  "/desktop/gnome/volume_manager/"

/* type registration boilerplate code */
G_DEFINE_TYPE(DriveButton, drive_button, GTK_TYPE_BUTTON)

static void     drive_button_set_drive    (DriveButton    *self,
				           GnomeVFSDrive  *drive);
static void     drive_button_set_volume   (DriveButton    *self,
				           GnomeVFSVolume *volume);
static void     drive_button_reset_popup  (DriveButton    *self);
static void     drive_button_ensure_popup (DriveButton    *self);

static void     drive_button_destroy      (GtkObject      *object);
#if 0
static void     drive_button_unrealize    (GtkWidget      *widget);
#endif /* 0 */
static gboolean drive_button_button_press (GtkWidget      *widget,
					   GdkEventButton *event);
static gboolean drive_button_key_press    (GtkWidget      *widget,
					   GdkEventKey    *event);

static void
drive_button_class_init (DriveButtonClass *class)
{
    GTK_OBJECT_CLASS(class)->destroy = drive_button_destroy;
    GTK_WIDGET_CLASS(class)->button_press_event = drive_button_button_press;
    GTK_WIDGET_CLASS(class)->key_press_event = drive_button_key_press;

    gtk_rc_parse_string ("\n"
			 "   style \"drive-button-style\"\n"
			 "   {\n"
			 "      GtkWidget::focus-line-width=0\n"
			 "      GtkWidget::focus-padding=0\n"
			 "      bg_pixmap[NORMAL] = \"<parent>\"\n"
			 "      bg_pixmap[ACTIVE] = \"<parent>\"\n"
			 "      bg_pixmap[PRELIGHT] = \"<parent>\"\n"
			 "      bg_pixmap[INSENSITIVE] = \"<parent>\"\n"
			 "   }\n"
			 "\n"
			 "    class \"DriveButton\" style \"drive-button-style\"\n"
			 "\n");
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

    self->popup_menu = NULL;
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
drive_button_destroy (GtkObject *object)
{
    DriveButton *self = DRIVE_BUTTON (object);

    drive_button_set_drive (self, NULL);

    if (self->tooltips)
	g_object_unref (self->tooltips);
    self->tooltips = NULL;

    if (self->update_tag)
	g_source_remove (self->update_tag);
    self->update_tag = 0;

    drive_button_reset_popup (self);

    if (GTK_OBJECT_CLASS (drive_button_parent_class)->destroy)
	(* GTK_OBJECT_CLASS (drive_button_parent_class)->destroy) (object);
}

#if 0
static void
drive_button_unrealize (GtkWidget *widget)
{
    DriveButton *self = DRIVE_BUTTON (widget);

    drive_button_reset_popup (self);

    if (GTK_WIDGET_CLASS (drive_button_parent_class)->unrealize)
	(* GTK_WIDGET_CLASS (drive_button_parent_class)->unrealize) (widget);
}
#endif /* 0 */

/* the following function is adapted from gtkmenuitem.c */
static void
position_menu (GtkMenu *menu, gint *x, gint *y,
	       gboolean *push_in, gpointer user_data)
{
    GtkWidget *widget = GTK_WIDGET (user_data);
    GdkScreen *screen;
    gint twidth, theight, tx, ty;
    GtkTextDirection direction;
    GdkRectangle monitor;
    gint monitor_num;

    g_return_if_fail (menu != NULL);
    g_return_if_fail (x != NULL);
    g_return_if_fail (y != NULL);

    if (push_in) *push_in = FALSE;

    direction = gtk_widget_get_direction (widget);

    twidth = GTK_WIDGET (menu)->requisition.width;
    theight = GTK_WIDGET (menu)->requisition.height;

    screen = gtk_widget_get_screen (GTK_WIDGET (menu));
    monitor_num = gdk_screen_get_monitor_at_window (screen, widget->window);
    if (monitor_num < 0) monitor_num = 0;
    gdk_screen_get_monitor_geometry (screen, monitor_num, &monitor);

    if (!gdk_window_get_origin (widget->window, &tx, &ty)) {
	g_warning ("Menu not on screen");
	return;
    }

    tx += widget->allocation.x;
    ty += widget->allocation.y;

    if (direction == GTK_TEXT_DIR_RTL)
	tx += widget->allocation.width - twidth;

    if ((ty + widget->allocation.height + theight) <= monitor.y + monitor.height)
	ty += widget->allocation.height;
    else if ((ty - theight) >= monitor.y)
	ty -= theight;
    else if (monitor.y + monitor.height - (ty + widget->allocation.height) > ty)
	ty += widget->allocation.height;
    else
	ty -= theight;

    *x = CLAMP (tx, monitor.x, MAX (monitor.x, monitor.x + monitor.width - twidth));
    *y = ty;
    gtk_menu_set_monitor (menu, monitor_num);
}

static gboolean
drive_button_button_press (GtkWidget      *widget,
			   GdkEventButton *event)
{
    DriveButton *self = DRIVE_BUTTON (widget);

    /* don't consume non-button1 presses */
    if (event->button == 1) {
	drive_button_ensure_popup (self);
	if (self->popup_menu) {
	    gtk_menu_popup (GTK_MENU (self->popup_menu), NULL, NULL,
			    position_menu, self,
			    event->button, event->time);
	}
	return TRUE;
    }
    return FALSE;
}

static gboolean
drive_button_key_press (GtkWidget      *widget,
			GdkEventKey    *event)
{
    DriveButton *self = DRIVE_BUTTON (widget);

    switch (event->keyval) {
    case GDK_KP_Space:
    case GDK_space:
    case GDK_KP_Enter:
    case GDK_Return:
	drive_button_ensure_popup (self);
	if (self->popup_menu) {
	    gtk_menu_popup (GTK_MENU (self->popup_menu), NULL, NULL,
			    position_menu, self,
			    0, event->time);
	}
	return TRUE;
    }
    return FALSE;
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
    GdkPixbuf *pixbuf, *scaled;
    GtkRequisition button_req, image_req;

    g_return_val_if_fail (DRIVE_IS_BUTTON (user_data), FALSE);
    self = DRIVE_BUTTON (user_data);
    self->update_tag = 0;

    drive_button_reset_popup (self);

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
		tip = g_strdup_printf ("%s\n%s", display_name, _("(mounted)"));
	    else if (gnome_vfs_drive_is_connected (self->drive))
		tip = g_strdup_printf ("%s\n%s", display_name, _("(not mounted)"));
	    else
		tip = g_strdup_printf ("%s\n%s", display_name, _("(not connected)"));
	} else {
	    display_name = gnome_vfs_volume_get_display_name (self->volume);
	    if (gnome_vfs_volume_is_mounted (self->volume))
		tip = g_strdup_printf ("%s\n%s", display_name, _("(mounted)"));
	    else
		tip = g_strdup_printf ("%s\n%s", display_name, _("(not mounted)"));
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

    scaled = gdk_pixbuf_scale_simple (pixbuf, width, height, GDK_INTERP_BILINEAR);
    if (scaled) {
	g_object_unref (pixbuf);
	pixbuf = scaled;
    }

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

static void
drive_button_reset_popup (DriveButton *self)
{
    if (self->popup_menu)
	gtk_object_destroy (GTK_OBJECT (self->popup_menu));
    self->popup_menu = NULL;
}

#if 0
static void
popup_menu_detach (GtkWidget *attach_widget, GtkMenu *menu)
{
    DRIVE_BUTTON (attach_widget)->popup_menu = NULL;
}
#endif /* 0 */

static char *
escape_underscores (const char *str)
{
    char *new_str;
    int i, j, count;

    /* count up how many underscores are in the string */
    count = 0;
    for (i = 0; str[i] != '\0'; i++) {
	if (str[i] == '_')
	    count++;
    }
    /* copy to new string, doubling up underscores */
    new_str = g_new (char, i + count + 1);
    for (i = j = 0; str[i] != '\0'; i++, j++) {
	new_str[j] = str[i];
	if (str[i] == '_')
	    new_str[++j] = '_';
    }
    new_str[j] = '\0';
    return new_str;
}
static GtkWidget *
create_menu_item (DriveButton *self, const gchar *stock_id,
		  const gchar *label, GCallback callback,
		  gboolean sensitive)
{
    GtkWidget *item, *image;

    item = gtk_image_menu_item_new_with_mnemonic (label);
    if (stock_id) {
	image = gtk_image_new_from_stock (stock_id, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item), image);
	gtk_widget_show (image);
    }
    if (callback)
	g_signal_connect_object (item, "activate", callback, self,
				 G_CONNECT_SWAPPED);
    gtk_widget_set_sensitive (item, sensitive);
    gtk_widget_show (item);
    return item;
}

static void
open_drive (DriveButton *self, GtkWidget *item)
{
    GdkScreen *screen;
    GtkWidget *dialog;
    GError *error = NULL;
    char *argv[3] = { "nautilus", NULL, NULL };

    screen = gtk_widget_get_screen (GTK_WIDGET (self));

    if (self->drive) {
	argv[1] = gnome_vfs_drive_get_activation_uri (self->drive);
	/* if the drive has no activation URI, get the activation URI
	 * of it's first mounted volume */
	if (!argv[1]) {
	    GList *volumes;
	    GnomeVFSVolume *volume;

	    volumes = gnome_vfs_drive_get_mounted_volumes (self->drive);
	    if (volumes) {
		volume = GNOME_VFS_VOLUME (volumes->data);

		argv[1] = gnome_vfs_volume_get_activation_uri (volume);
		g_list_foreach (volumes, (GFunc)gnome_vfs_volume_unref, NULL);
		g_list_free (volumes);
	    }
	}
    } else if (self->volume)
	argv[1] = gnome_vfs_volume_get_activation_uri (self->volume);
    else
	g_return_if_reached();

    if (!gdk_spawn_on_screen (screen, NULL, argv, NULL,
			      G_SPAWN_SEARCH_PATH,
			      NULL, NULL, NULL, &error)) {
	dialog = gtk_message_dialog_new (GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (self))),
						     GTK_DIALOG_DESTROY_WITH_PARENT,
						     GTK_MESSAGE_ERROR,
						     GTK_BUTTONS_OK,
						     _("Cannot execute '%s'"),
						     argv[0]);
	gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), error->message);
	g_signal_connect (dialog, "response",
			  G_CALLBACK (gtk_object_destroy), NULL);
	gtk_widget_show (dialog);
	g_error_free (error);
    }
    g_free (argv[1]);
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
	case CMD_MOUNT_OR_PLAY:
	    title = _("Mount Error");
	    break;
	case CMD_UNMOUNT:
	    title = _("Unmount Error");
	    break;
	case CMD_EJECT:
	    title = _("Eject Error");
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

/* copied from gnome-volume-manager/src/manager.c maybe there is a better way than
 * duplicating this code? */

/*
 * gvm_run_command - run the given command, replacing %d with the device node
 * and %m with the given path
 */
static void
gvm_run_command (const char *device, const char *command, const char *path)
{
	char *argv[4];
	gchar *new_command;
	GError *error = NULL;
	GString *exec = g_string_new (NULL);
	char *p, *q;

	/* perform s/%d/device/ and s/%m/path/ */
	new_command = g_strdup (command);
	q = new_command;
	p = new_command;
	while ((p = strchr (p, '%')) != NULL) {
		if (*(p + 1) == 'd') {
			*p = '\0';
			g_string_append (exec, q);
			g_string_append (exec, device);
			q = p + 2;
			p = p + 2;
		} else if (*(p + 1) == 'm') {
			*p = '\0';
			g_string_append (exec, q);
			g_string_append (exec, path);
			q = p + 2;
			p = p + 2;
		}
	}
	g_string_append (exec, q);

	argv[0] = "/bin/sh";
	argv[1] = "-c";
	argv[2] = exec->str;
	argv[3] = NULL;

	g_spawn_async (g_get_home_dir (), argv, NULL, 0, NULL, NULL,
		       NULL, &error);
	if (error) {
		g_warning ("failed to exec %s: %s\n", exec->str, error->message);
		g_error_free (error);
	}

	g_string_free (exec, TRUE);
	g_free (new_command);
}

/*
 * gvm_check_dvd_only - is this a Video DVD?
 *
 * Returns TRUE if this was a Video DVD and FALSE otherwise.
 * (the original in gvm was also running the autoplay action,
 * I removed that code, so I renamed from gvm_check_dvd to
 * gvm_check_dvd_only)
 */
static gboolean
gvm_check_dvd_only (const char *udi, const char *device, const char *mount_point)
{
	char *path;
	gboolean retval;
	
	path = g_build_path (G_DIR_SEPARATOR_S, mount_point, "video_ts", NULL);
	retval = g_file_test (path, G_FILE_TEST_IS_DIR);
	g_free (path);
	
	/* try the other name, if needed */
	if (retval == FALSE) {
		path = g_build_path (G_DIR_SEPARATOR_S, mount_point,
				     "VIDEO_TS", NULL);
		retval = g_file_test (path, G_FILE_TEST_IS_DIR);
		g_free (path);
	}
	
	return retval;
}
/* END copied from gnome-volume-manager/src/manager.c */

static gboolean
check_dvd_video (GnomeVFSVolume *volume)
{
	char *device_path = gnome_vfs_volume_get_device_path (volume);
	char *uri = gnome_vfs_volume_get_activation_uri (volume);
	char *mount_path = gnome_vfs_get_local_path_from_uri (uri);
	char *udi = gnome_vfs_volume_get_hal_udi (volume);

	gboolean result = gvm_check_dvd_only (udi, device_path, mount_path);

	g_free (device_path);
	g_free (udi);
	g_free (uri);
	g_free (mount_path);

	return result;
}

static gboolean
check_audio_cd (DriveButton *self, GnomeVFSVolume *volume)
{
	char *activation_uri = gnome_vfs_volume_get_activation_uri (volume);
	// we have an audioCD if the activation URI starts by 'cdda://'
	gboolean result = (strncmp ("cdda://", activation_uri, 7) == 0);
	g_free (activation_uri);
	return result;
}

static void
run_command (DriveButton *self, const char *command)
{
	char *uri, *mount_path;
	char *device_path = gnome_vfs_drive_get_device_path (self->drive);

	GList *volumes;
	GnomeVFSVolume *volume;

	volumes = gnome_vfs_drive_get_mounted_volumes (self->drive);
	volume = GNOME_VFS_VOLUME (volumes->data);
	uri = gnome_vfs_volume_get_activation_uri (volume);
	mount_path = gnome_vfs_get_local_path_from_uri (uri);
	g_free (uri);

	gnome_vfs_drive_get_display_name (self->drive);
	gvm_run_command (device_path, command,
				 mount_path);

	g_list_foreach (volumes, (GFunc)gnome_vfs_volume_unref, NULL);
	g_list_free (volumes);

	g_free (mount_path);
	g_free (device_path);
}

static void
mount_drive (DriveButton *self, GtkWidget *item)
{
    if (self->drive) {
	gnome_vfs_drive_mount (self->drive, mount_result,
			       GINT_TO_POINTER(CMD_MOUNT_OR_PLAY));
    } else {
	g_return_if_reached();
    }
}
static void
unmount_drive (DriveButton *self, GtkWidget *item)
{
    if (self->drive) {
	gnome_vfs_drive_unmount (self->drive, mount_result,
				 GINT_TO_POINTER(CMD_UNMOUNT));
    } else if (self->volume) {
	gnome_vfs_volume_unmount (self->volume, mount_result,
				  GINT_TO_POINTER(CMD_UNMOUNT));
    } else {
	g_return_if_reached();
    }
}
static void
eject_drive (DriveButton *self, GtkWidget *item)
{
    if (self->drive) {
	gnome_vfs_drive_eject (self->drive, mount_result,
			       GINT_TO_POINTER(CMD_EJECT));
    } else if (self->volume) {
	gnome_vfs_volume_eject (self->volume, mount_result,
				GINT_TO_POINTER(CMD_EJECT));
    } else {
	g_return_if_reached();
    }
}
static void
play_autoplay_media (DriveButton *self, const char *autoplay_key)
{
	GConfClient *gconf_client = gconf_client_get_default ();
	char *command = gconf_client_get_string (gconf_client,
			autoplay_key, NULL);
	run_command (self, command);
	g_free (command);
	g_object_unref (gconf_client);
}

static void
play_dvd (DriveButton *self, GtkWidget *item)
{
	play_autoplay_media (self, GCONF_ROOT_AUTOPLAY "autoplay_dvd_command");
}

static void
play_cda (DriveButton *self, GtkWidget *item)
{
	play_autoplay_media (self, GCONF_ROOT_AUTOPLAY "autoplay_cda_command");
}

static void
drive_button_ensure_popup (DriveButton *self)
{
    GnomeVFSDeviceType device_type;
    char *display_name, *tmp, *label;
    int action = CMD_NONE;
    GtkWidget *item;
    GCallback callback;
    const char *action_icon;
    gboolean ejectable;

    if (self->popup_menu) return;

    if (self->drive) {
	GnomeVFSVolume *volume = NULL;
	GList *volumes;

	if (!gnome_vfs_drive_is_connected (self->drive)) return;

	device_type = gnome_vfs_drive_get_device_type (self->drive);
	display_name = gnome_vfs_drive_get_display_name (self->drive);
	ejectable = gnome_vfs_drive_needs_eject (self->drive);

	if (gnome_vfs_drive_is_mounted (self->drive)) {
	    if (!ejectable)
		action = CMD_UNMOUNT;
	} else {
	    action = CMD_MOUNT_OR_PLAY;
	}
    
	volumes = gnome_vfs_drive_get_mounted_volumes (self->drive);
	if (volumes != NULL)
	{
		volume = GNOME_VFS_VOLUME (volumes->data);
		device_type = gnome_vfs_volume_get_device_type (volume);
	}

	/*
	 * For some reason, on my computer, I get GNOME_VFS_DEVICE_TYPE_CDROM
	 * also for DVDs and Audio CDs, while for DVD the icon is correctly the
	 * one for DVDs and for Audio CDs the description correctly that of an
	 * audio CD? So i have this hack.
	 */
	if (volume)
	{
		if (check_dvd_video (volume))
		{
			device_type = GNOME_VFS_DEVICE_TYPE_VIDEO_DVD;
		}
		if (check_audio_cd (self, volume))
		{
			device_type = GNOME_VFS_DEVICE_TYPE_AUDIO_CD;
		}
	}

	g_list_foreach (volumes, (GFunc)gnome_vfs_volume_unref, NULL);
	g_list_free (volumes);
    } else {
	GnomeVFSDrive* drive = gnome_vfs_volume_get_drive (self->volume);

	if (!gnome_vfs_volume_is_mounted (self->volume)) return;

	device_type = gnome_vfs_volume_get_device_type (self->volume);
	display_name = gnome_vfs_volume_get_display_name (self->volume);
	if (drive)
	    ejectable = gnome_vfs_drive_needs_eject (drive);
	else
	    ejectable = FALSE;
	if (!ejectable)
	    action = CMD_UNMOUNT;
    }

    self->popup_menu = gtk_menu_new ();

    /* make sure the display name doesn't look like a mnemonic */
    tmp = escape_underscores (display_name ? display_name : "(none)");
    g_free (display_name);
    display_name = tmp;

	callback = G_CALLBACK (open_drive);
	action_icon = GTK_STOCK_OPEN;

	switch (device_type) {
	case GNOME_VFS_DEVICE_TYPE_VIDEO_DVD:
		label = g_strdup (_("_Play DVD"));
		callback = G_CALLBACK (play_dvd);
		action_icon = GTK_STOCK_MEDIA_PLAY;
		break;
	case GNOME_VFS_DEVICE_TYPE_AUDIO_CD:
		label = g_strdup (_("_Play CD"));
		callback = G_CALLBACK (play_cda);
		action_icon = GTK_STOCK_MEDIA_PLAY;
		break;
	default:
		label = g_strdup_printf (_("_Open %s"), display_name);
	}

	item = create_menu_item (self, action_icon, label,
			     callback,
			     action != CMD_MOUNT_OR_PLAY);
    g_free (label);
    gtk_container_add (GTK_CONTAINER (self->popup_menu), item);

    switch (action) {
    case CMD_MOUNT_OR_PLAY:
	label = g_strdup_printf (_("_Mount %s"), display_name);
	item = create_menu_item (self, NULL, label,
				 G_CALLBACK (mount_drive), TRUE);
	g_free (label);
	gtk_container_add (GTK_CONTAINER (self->popup_menu), item);
	break;
    case CMD_UNMOUNT:
	label = g_strdup_printf (_("Un_mount %s"), display_name);
	item = create_menu_item (self, NULL, label,
				 G_CALLBACK (unmount_drive), TRUE);
	g_free (label);
	gtk_container_add (GTK_CONTAINER (self->popup_menu), item);
	break;
    default:
	break;
    }

    if (ejectable)
    {
	label = g_strdup_printf (_("_Eject %s"), display_name);
	item = create_menu_item (self, NULL, label,
				 G_CALLBACK (eject_drive), TRUE);
	g_free (label);
	gtk_container_add (GTK_CONTAINER (self->popup_menu), item);
    }

    g_free (display_name);
}
