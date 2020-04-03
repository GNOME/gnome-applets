/*
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
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * Author:
 *   James Henstridge <jamesh@canonical.com>
 */

#include "config.h"
#include "drivemount-applet.h"

#include <string.h>

#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>

#include "drive-list.h"

struct _DrivemountApplet
{
  GpApplet   parent;

  GtkWidget *drive_list;
};

G_DEFINE_TYPE (DrivemountApplet, drivemount_applet, GP_TYPE_APPLET)

static void
placement_changed_cb (GpApplet         *applet,
                      GtkOrientation    orientation,
                      GtkPositionType   position,
                      DrivemountApplet *self)
{
  drive_list_set_orientation (DRIVE_LIST (self->drive_list), orientation);
}

static void
size_allocate_cb (GtkWidget        *widget,
                  GtkAllocation    *allocation,
                  DrivemountApplet *self)
{
    int size;

    switch (gp_applet_get_orientation (GP_APPLET (self))) {
    case GTK_ORIENTATION_VERTICAL:
	size = allocation->width;
	break;
    case GTK_ORIENTATION_HORIZONTAL:
    default:
	size = allocation->height;
	break;
    }

    drive_list_set_panel_size (DRIVE_LIST (self->drive_list), size);
}

static void
display_about_dialog (GSimpleAction *action,
                      GVariant      *parameter,
                      gpointer       user_data)
{
  gp_applet_show_about (GP_APPLET (user_data));
}

static void
display_help (GSimpleAction *action,
              GVariant      *parameter,
              gpointer       user_data)
{
  gp_applet_show_help (GP_APPLET (user_data), NULL);
}

static const GActionEntry applet_menu_actions [] = {
	{ "help",  display_help,         NULL, NULL, NULL },
	{ "about", display_about_dialog, NULL, NULL, NULL },
	{ NULL }
};

static void
drivemount_applet_setup (DrivemountApplet *self)
{
  const char *menu_resource;
  AtkObject *ao;

  self->drive_list = drive_list_new ();
  gtk_container_add (GTK_CONTAINER (self), self->drive_list);

  g_signal_connect (self,
                    "placement-changed",
                    G_CALLBACK (placement_changed_cb),
                    self);

  g_signal_connect (self,
                    "size-allocate",
                    G_CALLBACK (size_allocate_cb),
                    self);

  drive_list_set_orientation (DRIVE_LIST (self->drive_list),
                              gp_applet_get_orientation (GP_APPLET (self)));

  menu_resource = GRESOURCE_PREFIX "/ui/drivemount-applet-menu.ui";
  gp_applet_setup_menu_from_resource (GP_APPLET (self),
                                      menu_resource,
                                      applet_menu_actions);

  ao = gtk_widget_get_accessible (GTK_WIDGET (self));
  atk_object_set_name (ao, _("Disk Mounter"));

  gtk_widget_show_all (GTK_WIDGET (self));
}

static void
drivemount_applet_constructed (GObject *object)
{
  G_OBJECT_CLASS (drivemount_applet_parent_class)->constructed (object);
  drivemount_applet_setup (DRIVEMOUNT_APPLET (object));
}

static void
drivemount_applet_class_init (DrivemountAppletClass *self_class)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (self_class);

  object_class->constructed = drivemount_applet_constructed;
}

static void
drivemount_applet_init (DrivemountApplet *self)
{
  gp_applet_set_flags (GP_APPLET (self), GP_APPLET_FLAGS_EXPAND_MINOR);
}

void
drivemount_applet_setup_about (GtkAboutDialog *dialog)
{
  const char *comments;
  const char **authors;
  const char **documenters;
  const char *copyright;

  comments = _("Applet for mounting and unmounting block volumes.");

  authors = (const char *[])
    {
      "James Henstridge <jamesh@canonical.com>",
      NULL
    };

  documenters = (const char *[])
    {
      "Dan Mueth <muet@alumni.uchicago.edu>",
      "John Fleck <jfleck@inkstain.net>",
      NULL
    };

  copyright = "Copyright \xC2\xA9 2004 Canonical Ltd";

  gtk_about_dialog_set_comments (dialog, comments);

  gtk_about_dialog_set_authors (dialog, authors);
  gtk_about_dialog_set_documenters (dialog, documenters);
  gtk_about_dialog_set_translator_credits (dialog, _("translator-credits"));
  gtk_about_dialog_set_copyright (dialog, copyright);
}
