/* GNOME Volume Applet
 * Copyright (C) 2004 Ronald Bultje <rbultje@ronald.bitfreak.net>
 *
 * dock.c: floating window containing volume widgets
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gnome.h>

#include "dock.h"

static void	gnome_volume_applet_dock_class_init	(GnomeVolumeAppletDockClass *klass);
static void	gnome_volume_applet_dock_init		(GnomeVolumeAppletDock *applet);
static void	gnome_volume_applet_dock_dispose	(GObject *object);

static gboolean	cb_button_press				(GtkWidget *widget,
							 GdkEventButton *button,
							 gpointer   data);
static gboolean	cb_button_release			(GtkWidget *widget,
							 GdkEventButton *button,
							 gpointer   data);

static GtkWindowClass *parent_class = NULL;

GType
gnome_volume_applet_dock_get_type (void)
{
  static GType gnome_volume_applet_dock_type = 0;

  if (!gnome_volume_applet_dock_type) {
    static const GTypeInfo gnome_volume_applet_dock_info = {
      sizeof (GnomeVolumeAppletDockClass),
      NULL,
      NULL,
      (GClassInitFunc) gnome_volume_applet_dock_class_init,
      NULL,
      NULL,
      sizeof (GnomeVolumeAppletDock),
      0,
      (GInstanceInitFunc) gnome_volume_applet_dock_init,
      NULL
    };

    gnome_volume_applet_dock_type =
	g_type_register_static (GTK_TYPE_WINDOW, 
				"GnomeVolumeAppletDock",
				&gnome_volume_applet_dock_info, 0);
  }

  return gnome_volume_applet_dock_type;
}

static void
gnome_volume_applet_dock_class_init (GnomeVolumeAppletDockClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  parent_class = g_type_class_ref (GTK_TYPE_WINDOW);

  gobject_class->dispose = gnome_volume_applet_dock_dispose;
}

static void
gnome_volume_applet_dock_init (GnomeVolumeAppletDock *dock)
{
  dock->orientation = -1;
  dock->timeout = 0;
  gtk_window_set_decorated (GTK_WINDOW (dock), FALSE);
}

GtkWidget *
gnome_volume_applet_dock_new (GtkOrientation orientation)
{
  GtkWidget *table, *button, *scale, *frame;
  GnomeVolumeAppletDock *dock;
  struct {
    gint w, h;
    gint x[3], y[3];
    GtkWidget * (* sfunc) (GtkAdjustment *adj);
    gint sw, sh;
  } magic[2] = {
    { 3, 1, { 0, 1, 2 }, { 0, 0, 0 }, gtk_hscale_new, 100, -1 },
    { 1, 3, { 0, 0, 0 }, { 0, 1, 2 }, gtk_vscale_new, -1, 100 }
  };

  dock = g_object_new (GNOME_VOLUME_APPLET_TYPE_DOCK, NULL);
  dock->orientation = orientation;
  GTK_WINDOW (dock)->type = GTK_WINDOW_POPUP;
  GTK_WIDGET_UNSET_FLAGS (dock, GTK_TOPLEVEL);

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_OUT);

  table = gtk_table_new (magic[orientation].w,
			 magic[orientation].h, FALSE);

  button = gtk_button_new_with_label (_("+"));
  dock->plus = GTK_BUTTON (button);
  gtk_button_set_relief (dock->plus, GTK_RELIEF_NONE);
  gtk_table_attach_defaults (GTK_TABLE (table), button,
			     magic[orientation].x[0],
			     magic[orientation].x[0] + 1,
			     magic[orientation].y[0],
			     magic[orientation].y[0] + 1);
  g_signal_connect (button, "button-press-event",
		    G_CALLBACK (cb_button_press), dock);
  g_signal_connect (button, "button-release-event",
		    G_CALLBACK (cb_button_release), dock);
  gtk_widget_show (button);

  scale = magic[orientation].sfunc (NULL);
  dock->scale = GTK_RANGE (scale);
  gtk_widget_set_size_request (scale,
			       magic[orientation].sw,
			       magic[orientation].sh);
  gtk_scale_set_draw_value (GTK_SCALE (scale), FALSE);
  gtk_range_set_inverted (dock->scale, TRUE);
  gtk_table_attach_defaults (GTK_TABLE (table), scale,
			     magic[orientation].x[1],
			     magic[orientation].x[1] + 1,
			     magic[orientation].y[1],
			     magic[orientation].y[1] + 1);
  gtk_widget_show (scale);

  button = gtk_button_new_with_label (_("-"));
  dock->minus = GTK_BUTTON (button);
  gtk_button_set_relief (dock->minus, GTK_RELIEF_NONE);
  gtk_table_attach_defaults (GTK_TABLE (table), button,
			     magic[orientation].x[2],
			     magic[orientation].x[2] + 1,
			     magic[orientation].y[2],
			     magic[orientation].y[2] + 1);
  g_signal_connect (button, "button-press-event",
		    G_CALLBACK (cb_button_press), dock);
  g_signal_connect (button, "button-release-event",
		    G_CALLBACK (cb_button_release), dock);
  gtk_widget_show (button);

  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_widget_show (table);

  gtk_container_add (GTK_CONTAINER (dock), frame);
  gtk_widget_show (frame);

  return GTK_WIDGET (dock);
}

static void
gnome_volume_applet_dock_dispose (GObject *object)
{
  GnomeVolumeAppletDock *dock = GNOME_VOLUME_APPLET_DOCK (object);

  gnome_volume_applet_dock_change (dock, NULL);

  if (dock->timeout) {
    g_source_remove (dock->timeout);
    dock->timeout = 0;
  }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

/*
 * React if user presses +/- buttons.
 */

static gboolean
cb_timeout (gpointer data)
{
  GnomeVolumeAppletDock *dock = data;
  GtkAdjustment *adj;
  gint volume;
  gboolean res = TRUE;

  if (!dock->timeout)
    return FALSE;

  adj = gtk_range_get_adjustment (dock->scale);
  volume = gtk_range_get_value (dock->scale);
  volume += dock->direction * adj->step_increment;

  if (volume <= adj->lower) {
    volume = adj->lower;
    res = FALSE;
  } else if (volume >= adj->upper) {
    volume = adj->upper;
    res = FALSE;
  }
  gtk_range_set_value (dock->scale, volume);

  if (!res)
    dock->timeout = 0;

  return res;
}

static gboolean
cb_button_press (GtkWidget *widget,
		 GdkEventButton *button,
		 gpointer   data)
{
  GnomeVolumeAppletDock *dock = data;

  dock->direction = (GTK_BUTTON (widget) == dock->plus) ? 1 : -1;
  if (dock->timeout)
    g_source_remove (dock->timeout);
  dock->timeout = g_timeout_add (100, cb_timeout, data);
  cb_timeout (data);

  return TRUE;
}

static gboolean
cb_button_release (GtkWidget *widget,
		   GdkEventButton *button,
		   gpointer   data)
{
  GnomeVolumeAppletDock *dock = data;

  if (dock->timeout) {
    g_source_remove (dock->timeout);
    dock->timeout = 0;
  }

  return TRUE;
}

/*
 * Change the active element.
 */

void
gnome_volume_applet_dock_change (GnomeVolumeAppletDock *dock,
				 GtkAdjustment *adj)
{
  gtk_range_set_adjustment (dock->scale, adj);
}
