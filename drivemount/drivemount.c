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

#include <string.h>

#include <gtk/gtk.h>
#include <libgnomevfs/gnome-vfs.h>
#include <panel-applet.h>

#include "drive-list.h"

static const char drivemount_iid[] = "OAFIID:GNOME_DriveMountApplet";
static const char factory_iid[] = "OAFIID:GNOME_DriveMountApplet_Factory";

static void
change_orient (PanelApplet *applet, PanelAppletOrient o, DriveList *drive_list)
{
    GtkOrientation orientation;

    switch (o) {
    case PANEL_APPLET_ORIENT_LEFT:
    case PANEL_APPLET_ORIENT_RIGHT:
	orientation = GTK_ORIENTATION_VERTICAL;
	break;
    case PANEL_APPLET_ORIENT_UP:
    case PANEL_APPLET_ORIENT_DOWN:
    default:
	orientation = GTK_ORIENTATION_HORIZONTAL;
	break;
    }
    drive_list_set_orientation (drive_list, orientation);
}

static void
size_allocate (PanelApplet  *applet,
	       GdkRectangle *allocation,
	       DriveList    *drive_list)
{
    int size;

    switch (panel_applet_get_orient (applet)) {
    case PANEL_APPLET_ORIENT_LEFT:
    case PANEL_APPLET_ORIENT_RIGHT:
	size = allocation->width;
	break;
    case PANEL_APPLET_ORIENT_UP:
    case PANEL_APPLET_ORIENT_DOWN:
    default:
	size = allocation->height;
	break;
    }
    drive_list_set_panel_size (drive_list, size);
}

static void
change_background (PanelApplet               *applet,
		   PanelAppletBackgroundType  type,
		   GdkColor                  *colour,
		   GdkPixmap                 *pixmap)
{
    GtkRcStyle *rc_style;
    GtkStyle *style;

    /* reset style */
    gtk_widget_set_style (GTK_WIDGET (applet), NULL);
    rc_style = gtk_rc_style_new ();
    gtk_widget_modify_style (GTK_WIDGET (applet), rc_style);
    g_object_unref (rc_style);

    switch (type) {
    case PANEL_NO_BACKGROUND:
	break;
    case PANEL_COLOR_BACKGROUND:
	gtk_widget_modify_bg (GTK_WIDGET (applet),
			      GTK_STATE_NORMAL, colour);
	break;
    case PANEL_PIXMAP_BACKGROUND:
	style = gtk_style_copy (GTK_WIDGET (applet)->style);
	if (style->bg_pixmap[GTK_STATE_NORMAL])
	    g_object_unref (style->bg_pixmap[GTK_STATE_NORMAL]);
	style->bg_pixmap[GTK_STATE_NORMAL] = g_object_ref (pixmap);
	gtk_widget_set_style (GTK_WIDGET (applet), style);
	break;
    }
}

static void
display_about_dialog (BonoboUIComponent *uic,
		      DriveList *drive_list,
		      const gchar *verbname)
{
    const gchar *authors[] = {
	"James Henstridge <jamesh@canonical.com>",
	NULL
    };
    const gchar *documenters[] = {
	"Dan Mueth <muet@alumni.uchicago.edu>",
	"John Fleck <jfleck@inkstain.net>",
	NULL
    };

    gtk_show_about_dialog (NULL,
	"name",        _("Disk Mounter"),
	"version",     VERSION,
	"copyright",   "Copyright \xC2\xA9 2004 Canonical Ltd",
	"comments",    _("Applet for mounting and unmounting block volumes."),
	"authors",     authors,
	"documenters", documenters,
	"translator-credits", _("translator_credits"),
	"logo_icon_name",     "gnome-dev-jazdisk",
	NULL);
}

static void
display_help (BonoboUIComponent *uic,
	      DriveList *drive_list,
	      const gchar *verbname)
{
    GdkScreen *screen;
    GError *error = NULL;

    screen = gtk_widget_get_screen (GTK_WIDGET (drive_list));
    gnome_help_display_desktop_on_screen (
		NULL, "drivemount", "drivemount", NULL,
		screen, &error);
    if (error) {
	GtkWidget *dialog;

	dialog = gtk_message_dialog_new (NULL,
					 GTK_DIALOG_MODAL,
					 GTK_MESSAGE_ERROR,
					 GTK_BUTTONS_OK,
					 _("There was an error displaying help: %s"),
					 error->message);
	g_signal_connect (dialog, "response",
			  G_CALLBACK (gtk_widget_destroy), NULL);
	gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
	gtk_window_set_screen (GTK_WINDOW (dialog), screen);
	gtk_widget_show (dialog);
	g_error_free (error);
    }
}

static const BonoboUIVerb applet_menu_verbs[] = {
    BONOBO_UI_UNSAFE_VERB ("About", display_about_dialog),
    BONOBO_UI_UNSAFE_VERB ("Help",  display_help),
    BONOBO_UI_VERB_END
};

static gboolean
applet_factory (PanelApplet *applet,
		const char  *iid,
		gpointer     user_data)
{
    gboolean ret = FALSE;
    GtkWidget *drive_list;

    if (!strcmp (iid, drivemount_iid)) {
	gtk_window_set_default_icon_name ("gnome-dev-jazdisk");

	panel_applet_set_flags (applet, PANEL_APPLET_EXPAND_MINOR);
	panel_applet_add_preferences (applet,
				      "/schemas/apps/drivemount-applet/prefs",
				      NULL);

	drive_list = drive_list_new ();
	gtk_container_add (GTK_CONTAINER (applet), drive_list);

	g_signal_connect_object (applet, "change_orient",
				 G_CALLBACK (change_orient), drive_list, 0);
	g_signal_connect_object (applet, "size_allocate",
				 G_CALLBACK (size_allocate), drive_list, 0);
	g_signal_connect (applet, "change_background",
			  G_CALLBACK (change_background), NULL);

	/* set initial state */
	change_orient (applet,
		       panel_applet_get_orient (applet),
		       DRIVE_LIST (drive_list));

	panel_applet_setup_menu_from_file (applet,
					   DATADIR,
					   "GNOME_DriveMountApplet.xml",
					   NULL, applet_menu_verbs,
					   drive_list);

	gtk_widget_show_all (GTK_WIDGET (applet));

	ret = TRUE;
    }

    return ret;
}

PANEL_APPLET_BONOBO_FACTORY (factory_iid,
			     PANEL_TYPE_APPLET,
			     "Drive-Mount-Applet", "0",
			     applet_factory, NULL)
