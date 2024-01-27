/*
 * Copyright (C) 2009, Nokia <ivan.frade@nokia.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "config.h"
#include "tracker-applet.h"

#include <string.h>
#include <stdlib.h>

#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "tracker-results-window.h"

struct _TrackerApplet
{
  GpApplet        parent;

  GtkWidget      *results;

  GtkWidget      *box;
  GtkWidget      *event_box;
  GtkWidget      *image;
  GtkWidget      *entry;

  guint           new_search_id;
  guint           idle_draw_id;

  GtkOrientation  orient;
  GdkPixbuf      *icon;
  gint            size;
};

G_DEFINE_TYPE (TrackerApplet, tracker_applet, GP_TYPE_APPLET)

static void
applet_about_cb (GSimpleAction *action,
                 GVariant      *parameter,
                 gpointer       user_data)
{
  gp_applet_show_about (GP_APPLET (user_data));
}

static const GActionEntry applet_menu_actions[] = {
	{ "about",
	  applet_about_cb,
	  NULL,
	  NULL,
	  NULL,
	},
	{
	  NULL
	}
};

static gboolean
applet_event_box_button_press_event_cb (GtkWidget      *widget,
                                        GdkEventButton *event,
                                        TrackerApplet  *applet)
{
	if (applet->results) {
		gtk_widget_destroy (applet->results);
		applet->results = NULL;
		return TRUE;
	}

	return FALSE;
}

static void
applet_entry_start_search (TrackerApplet *applet)
{
	const gchar *text;

	text = gtk_entry_get_text (GTK_ENTRY (applet->entry));

        if (!text || !*text) {
                gtk_widget_hide (applet->results);
                return;
        }

	g_print ("Searching for: '%s'\n", text);

	if (!applet->results) {
		applet->results = tracker_results_window_new (GTK_WIDGET (applet), text);
	} else {
		g_object_set (applet->results, "query", text, NULL);
	}

	if (!gtk_widget_get_visible (applet->results)) {
		tracker_results_window_popup (TRACKER_RESULTS_WINDOW (applet->results));
	}
}

static gboolean
applet_entry_start_search_cb (gpointer user_data)
{
	TrackerApplet *applet;

	applet = user_data;

	applet->new_search_id = 0;

	applet_entry_start_search (applet);

	return FALSE;
}

static void
applet_entry_activate_cb (GtkEntry      *entry,
                          TrackerApplet *applet)
{
	if (applet->new_search_id) {
		g_source_remove (applet->new_search_id);
		applet->new_search_id = 0;
	}

	applet_entry_start_search (applet);
}

static gboolean
applet_entry_button_press_event_cb (GtkWidget      *widget,
                                    GdkEventButton *event,
                                    TrackerApplet  *applet)
{
	gp_applet_request_focus (GP_APPLET (applet), event->time);

	return FALSE;
}

static void
applet_entry_editable_changed_cb (GtkWidget     *widget,
                                  TrackerApplet *applet)
{
        if (applet->new_search_id) {
                g_source_remove (applet->new_search_id);
        }

        applet->new_search_id =
                g_timeout_add (300,
                               applet_entry_start_search_cb,
                               applet);
}

static gboolean
applet_entry_key_press_event_cb (GtkWidget     *widget,
                                 GdkEventKey   *event,
                                 TrackerApplet *applet)
{
	if (event->keyval == GDK_KEY_Escape) {
		if (!applet->results) {
			return FALSE;
		}

		gtk_widget_destroy (applet->results);
		applet->results = NULL;
	} else if (event->keyval == GDK_KEY_Down) {
		if (!applet->results) {
			return FALSE;
		}

		gtk_widget_grab_focus (applet->results);
	}

	return FALSE;
}

static gboolean
applet_draw (gpointer user_data)
{
	TrackerApplet *applet = user_data;

	if (applet->box) {
		gtk_widget_destroy (applet->box);
	}

	switch (applet->orient) {
	case GTK_ORIENTATION_VERTICAL:
		applet->box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
		break;
	case GTK_ORIENTATION_HORIZONTAL:
		applet->box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
		break;
	default:
		g_assert_not_reached ();
		break;
	}

	gtk_container_add (GTK_CONTAINER (applet), applet->box);
	gtk_widget_show (applet->box);

	applet->event_box = gtk_event_box_new ();
	gtk_event_box_set_visible_window (GTK_EVENT_BOX (applet->event_box), FALSE);
	gtk_widget_show (applet->event_box);
	gtk_box_pack_start (GTK_BOX (applet->box), applet->event_box, FALSE, FALSE, 0);

	g_signal_connect (applet->event_box,
	                  "button_press_event",
	                  G_CALLBACK (applet_event_box_button_press_event_cb), applet);

	applet->image = gtk_image_new ();
	gtk_container_add (GTK_CONTAINER (applet->event_box), applet->image);
	gtk_image_set_from_icon_name (GTK_IMAGE (applet->image),
	                              "edit-find",
	                              GTK_ICON_SIZE_SMALL_TOOLBAR);
	gtk_widget_show (applet->image);

	applet->entry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (applet->box), applet->entry, TRUE, TRUE, 0);
	gtk_entry_set_width_chars (GTK_ENTRY (applet->entry), 12);
	gtk_widget_show (applet->entry);

	g_signal_connect (applet->entry,
	                  "activate",
	                  G_CALLBACK (applet_entry_activate_cb), applet);
	g_signal_connect (applet->entry,
	                  "button_press_event",
	                  G_CALLBACK (applet_entry_button_press_event_cb), applet);
        g_signal_connect (applet->entry,
                          "changed",
                          G_CALLBACK (applet_entry_editable_changed_cb), applet);
	g_signal_connect (applet->entry,
	                  "key_press_event",
	                  G_CALLBACK (applet_entry_key_press_event_cb), applet);

	applet->idle_draw_id = 0;

	return FALSE;
}

static void
applet_queue_draw (TrackerApplet *applet)
{
	if (!applet->idle_draw_id)
		applet->idle_draw_id = g_idle_add (applet_draw, applet);
}

static void
placement_changed_cb (GpApplet        *applet,
                      GtkOrientation   orientation,
                      GtkPositionType  position,
                      TrackerApplet   *self)
{
  GtkAllocation alloc;

  gtk_widget_get_allocation (GTK_WIDGET (self), &alloc);

  switch (orientation)
    {
      case GTK_ORIENTATION_VERTICAL:
        self->orient = orientation;
        self->size = alloc.width;
        break;

      case GTK_ORIENTATION_HORIZONTAL:
        self->orient = orientation;
        self->size = alloc.height;
        break;

      default:
        g_assert_not_reached ();
        break;
    }

  applet_queue_draw (self);
}

static void
applet_size_allocate_cb (GtkWidget     *widget,
                         GtkAllocation *allocation,
                         TrackerApplet *applet)
{
	gint new_size;

	if (gp_applet_get_orientation (GP_APPLET (applet)) == GTK_ORIENTATION_VERTICAL) {
		new_size = allocation->width;
	} else {
		new_size = allocation->height;
	}

	if (applet->image != NULL && applet->size != new_size) {
		applet->size = new_size;

		gtk_image_set_pixel_size (GTK_IMAGE (applet->image), applet->size - 2);

		/* re-scale the icon, if it was found */
		if (applet->icon)       {
			GdkPixbuf *scaled;

			scaled = gdk_pixbuf_scale_simple (applet->icon,
			                                  applet->size - 5,
			                                  applet->size - 5,
			                                  GDK_INTERP_BILINEAR);

			gtk_image_set_from_pixbuf (GTK_IMAGE (applet->image), scaled);
			g_object_unref (scaled);
		}
	}
}

static void
tracker_applet_setup (TrackerApplet *applet)
{
	const gchar *resource_name;

	applet->icon = gtk_icon_theme_load_icon (gtk_icon_theme_get_default (),
	                                         "edit-find",
	                                         48,
	                                         0,
	                                         NULL);

	applet_queue_draw (applet);

	gp_applet_set_flags (GP_APPLET (applet), GP_APPLET_FLAGS_EXPAND_MINOR);

	resource_name = GRESOURCE_PREFIX "/ui/tracker-search-bar-menu.ui";
	gp_applet_setup_menu_from_resource (GP_APPLET (applet),
	                                    resource_name,
	                                    applet_menu_actions);

	g_signal_connect (applet, "size-allocate",
	                  G_CALLBACK (applet_size_allocate_cb),
	                  applet);

	g_signal_connect (applet, "placement-changed",
	                  G_CALLBACK (placement_changed_cb),
	                  applet);
}

static void
tracker_applet_constructed (GObject *object)
{
  G_OBJECT_CLASS (tracker_applet_parent_class)->constructed (object);
  tracker_applet_setup (TRACKER_APPLET (object));
}

static void
tracker_applet_dispose (GObject *object)
{
  TrackerApplet *self;

  self = TRACKER_APPLET (object);

  if (self->idle_draw_id != 0)
    {
      g_source_remove (self->idle_draw_id);
      self->idle_draw_id = 0;
    }

  if (self->new_search_id != 0)
    {
      g_source_remove (self->new_search_id);
      self->new_search_id = 0;
    }

  g_clear_pointer (&self->results, gtk_widget_destroy);

  g_clear_object (&self->icon);

  G_OBJECT_CLASS (tracker_applet_parent_class)->dispose (object);
}

static void
tracker_applet_class_init (TrackerAppletClass *self_class)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (self_class);

  object_class->constructed = tracker_applet_constructed;
  object_class->dispose = tracker_applet_dispose;
}

static void
tracker_applet_init (TrackerApplet *self)
{
}

void
tracker_applet_setup_about (GtkAboutDialog *dialog)
{
  const char *comments;
  const char **authors;
  const char *copyright;

  comments = _("A search bar applet for finding content stored in Tracker");

  authors = (const char *[])
    {
      "Martyn Russell <martyn@lanedo.com>",
      "J&#xFC;rg Billeter <juerg.billeter@codethink.co.uk>",
      "Philip Van Hoof <pvanhoof@gnome.org>",
      "Carlos Garnacho <carlos@lanedo.com>",
      "Mikael Ottela <mikael.ottela@ixonos.com>",
      "Ivan Frade <ivan.frade@nokia.com>",
      "Jamie McCracken <jamiemcc@gnome.org>",
      "Adrien Bustany <abustany@gnome.org>",
      "Aleksander Morgado <aleksander@lanedo.com>",
      "Anders Aagaard <aagaande@gmail.com>",
      "Anders Rune Jensen <anders@iola.dk>",
      "Baptiste Mille-Mathias <baptist.millemathias@gmail.com>",
      "Christoph Laimburg <christoph.laimburg@rolmail.net>",
      "Dan Nicolaescu <dann@ics.uci.edu>",
      "Deji Akingunola <dakingun@gmail.com>",
      "Edward Duffy <eduffy@gmail.com>",
      "Eskil Bylund <eskil@letterboxes.org>",
      "Eugenio <me@eugesoftware.com>",
      "Fabien VALLON <fabien@sonappart.net>",
      "Gergan Penkov <gergan@gmail.com>",
      "Halton Huo <halton.huo@sun.com>",
      "Jaime Frutos Morales <acidborg@gmail.com>",
      "Jedy Wang <jedy.wang@sun.com>",
      "Jerry Tan <jerry.tan@sun.com>",
      "John Stowers <john.stowers@gmail.com>",
      "Julien <julienc@psychologie-fr.org>",
      "Laurent Aguerreche <laurent.aguerreche@free.fr>",
      "Luca Ferretti <elle.uca@libero.it>",
      "Marcus Fritzsch <fritschy@googlemail.com>",
      "Michael Biebl <mbiebl@gmail.com>",
      "Michal Pryc <michal.pryc@sun.com>",
      "Mikkel Kamstrup Erlandsen <mikkel.kamstrup@gmail.com>",
      "Nate Nielsen  <nielsen@memberwewbs.com>",
      "Neil Patel <njpatel@gmail.com>",
      "Richard Quirk <quirky@zoom.co.uk>",
      "Saleem Abdulrasool <compnerd@gentoo.org>",
      "Samuel Cormier-Iijima <sciyoshi@gmail.com>",
      "Tobutaz <tobutaz@gmail.com>",
      "Tom <tpgww@onepost.net>",
      "Tshepang Lekhonkhobe <tshepang@gmail.com>",
      "Ulrik Mikaelsson <ulrik.mikaelsson@gmail.com>",
      NULL
    };

  copyright = _("Copyright Tracker Authors 2005-2010");

  gtk_about_dialog_set_comments (dialog, comments);

  gtk_about_dialog_set_authors (dialog, authors);
  gtk_about_dialog_set_translator_credits (dialog, _("translator-credits"));
  gtk_about_dialog_set_copyright (dialog, copyright);
}
