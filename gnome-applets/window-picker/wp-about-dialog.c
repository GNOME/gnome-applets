/*
 * Copyright (C) 2008 Canonical Ltd
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
 *
 * Authors:
 *     Alberts Muktupāvels <alberts.muktupavels@gmail.com>
 *     Neil Jagdish Patel <neil.patel@canonical.com>
 *     Sebastian Geiger <sbastig@gmx.net>
 */

#include "config.h"
#include "wp-about-dialog.h"

#include <glib/gi18n-lib.h>

struct _WpAboutDialog
{
  GtkAboutDialog  parent;
  GdkPixbuf      *logo;
};

static const gchar *authors[] = {
  "Neil J. Patel <neil.patel@canonical.com>",
  "Sebastian Geiger <sbastig@gmx.net>",
  NULL
};

G_DEFINE_TYPE (WpAboutDialog, wp_about_dialog, GTK_TYPE_ABOUT_DIALOG)

static void
wp_about_dialog_constructed (GObject *object)
{
  WpAboutDialog *dialog;
  const gchar *resource;

  G_OBJECT_CLASS (wp_about_dialog_parent_class)->constructed (object);

  dialog = WP_ABOUT_DIALOG (object);
  resource = GRESOURCE_PREFIX "/icons/wp-about-logo.png";

  dialog->logo = gdk_pixbuf_new_from_resource (resource, NULL);

  if (dialog->logo)
    gtk_about_dialog_set_logo (GTK_ABOUT_DIALOG (dialog), dialog->logo);
}

static void
wp_about_dialog_dispose (GObject *object)
{
  WpAboutDialog *dialog;

  dialog = WP_ABOUT_DIALOG (object);

  g_clear_object (&dialog->logo);

  G_OBJECT_CLASS (wp_about_dialog_parent_class)->dispose (object);
}

static void
wp_about_dialog_class_init (WpAboutDialogClass *dialog_class)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (dialog_class);

  object_class->constructed = wp_about_dialog_constructed;
  object_class->dispose = wp_about_dialog_dispose;
}

static void
wp_about_dialog_init (WpAboutDialog *dialog)
{
}

GtkWidget *
wp_about_dialog_new (void)
{
  const gchar *title;
  const gchar *copyright;

  title = _("Window Picker");
  copyright = "Copyright \xc2\xa9 2008 Canonical Ltd\nand Sebastian Geiger";

  return g_object_new (WP_TYPE_ABOUT_DIALOG,
                       "authors", authors,
                       "comments", title,
                       "copyright", copyright,
                       "name", title,
                       "program-name", "GNOME Applets",
                       "version", PACKAGE_VERSION,
                       NULL);
}
