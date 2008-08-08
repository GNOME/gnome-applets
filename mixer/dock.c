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

#include <glib-object.h>
#include <glib/gi18n.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkframe.h>
#include <gtk/gtkrange.h>
#include <gtk/gtkhscale.h>
#include <gtk/gtktable.h>
#include <gtk/gtkvscale.h>
#include <gtk/gtkwidget.h>


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

G_DEFINE_TYPE (GnomeVolumeAppletDock, gnome_volume_applet_dock, GTK_TYPE_WINDOW)

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

  /* We can't use a simple GDK_WINDOW_TYPE_HINT_DOCK here since
   * the dock windows don't accept input by default. Instead we use 
   * the popup menu type. In the end we set everything by hand anyway
   * since what happens depends very heavily on the window manager. */
  gtk_window_set_type_hint (GTK_WINDOW (dock), 
      			    GDK_WINDOW_TYPE_HINT_POPUP_MENU);
  gtk_window_set_keep_above (GTK_WINDOW (dock), TRUE);
  gtk_window_set_decorated (GTK_WINDOW (dock), FALSE);
  gtk_window_set_skip_taskbar_hint (GTK_WINDOW (dock), TRUE);
  gtk_window_set_skip_pager_hint (GTK_WINDOW (dock), TRUE);
  gtk_window_set_resizable (GTK_WINDOW (dock), FALSE);
  gtk_window_stick (GTK_WINDOW (dock));
}

GtkWidget *
gnome_volume_applet_dock_new (GtkOrientation orientation)
{
  GtkWidget *table, *button, *scale, *frame;
  GnomeVolumeAppletDock *dock;
  gint i;
  static struct {
    gint w, h;
    gint x[3], y[3]; /* Locations for widgets in the table. The widget 
			coordinate order is '+', '-', then the slider. */
    GtkWidget * (* sfunc) (GtkAdjustment *adj);
    gint sw, sh;
    gboolean inverted;
  } magic[2] = {
    { 3, 1, { 2, 0, 1 }, { 0, 0, 0 }, gtk_hscale_new, 100, -1, FALSE},
    { 1, 3, { 0, 0, 0 }, { 0, 2, 1 }, gtk_vscale_new, -1, 100, TRUE}
  };

  dock = g_object_new (GNOME_VOLUME_APPLET_TYPE_DOCK,
		       NULL);
  dock->orientation = orientation;

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_OUT);

  table = gtk_table_new (magic[orientation].w,
			 magic[orientation].h, FALSE);

  button = gtk_button_new_with_label (_("-"));
  dock->minus = GTK_BUTTON (button);
  button = gtk_button_new_with_label (_("+")); /* The value of button falls
						  through into the loop. */
  dock->plus = GTK_BUTTON (button);
  for (i = 0; i<2; i++) { /* for button in (dock->plus, dock->minus): */
    gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
    gtk_table_attach_defaults (GTK_TABLE (table), button,
			       magic[orientation].x[i],
			       magic[orientation].x[i] + 1,
			       magic[orientation].y[i],
			       magic[orientation].y[i] + 1);
    g_signal_connect (button, "button-press-event",
		      G_CALLBACK (cb_button_press), dock);
    g_signal_connect (button, "button-release-event",
		      G_CALLBACK (cb_button_release), dock);
    button = GTK_WIDGET (dock->minus);
  }

  scale = magic[orientation].sfunc (NULL);
  dock->scale = GTK_RANGE (scale);
  gtk_widget_set_size_request (scale,
			       magic[orientation].sw,
			       magic[orientation].sh);
  gtk_scale_set_draw_value (GTK_SCALE (scale), FALSE);
  gtk_range_set_inverted (dock->scale, magic[orientation].inverted);
  gtk_table_attach_defaults (GTK_TABLE (table), scale,
			     magic[orientation].x[2],
			     magic[orientation].x[2] + 1,
			     magic[orientation].y[2],
			     magic[orientation].y[2] + 1);

  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_container_add (GTK_CONTAINER (dock), frame);

  return GTK_WIDGET (dock);
}

static destroy_source (GnomeVolumeAppletDock *dock)
{
  if (dock->timeout) {
    g_source_remove (dock->timeout);
    dock->timeout = 0;
  }
}

static void
gnome_volume_applet_dock_dispose (GObject *object)
{
  GnomeVolumeAppletDock *dock = GNOME_VOLUME_APPLET_DOCK (object);

  destroy_source (dock);

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

/*
 * Change the value of the slider. This is called both from a direct
 * call from the +/- button callbacks and via a timer so holding down the 
 * buttons changes the volume.
 */

static gboolean
cb_timeout (gpointer data)
{
  GnomeVolumeAppletDock *dock = data;
  GtkAdjustment *adj;
  gfloat volume;
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

/*
 * React if user presses +/- buttons.
 */

static gboolean
cb_button_press (GtkWidget *widget,
		 GdkEventButton *button,
		 gpointer   data)
{
  GnomeVolumeAppletDock *dock = data;

  dock->direction = (GTK_BUTTON (widget) == dock->plus) ? 1 : -1;
  destroy_source (dock);
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

  destroy_source (dock);

  return TRUE;
}

/*
 * Set the adjustment for the slider.
 */

void
gnome_volume_applet_dock_change (GnomeVolumeAppletDock *dock,
				 GtkAdjustment *adj)
{
  gtk_range_set_adjustment (dock->scale, adj);
}

void
gnome_volume_applet_dock_set_focus (GnomeVolumeAppletDock *dock)
{
  gtk_widget_grab_focus (GTK_WIDGET (dock->scale));
}
