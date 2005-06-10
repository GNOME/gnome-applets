/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 2 -*- */
/* GNOME Volume Applet
 * Copyright (C) 2004 Ronald Bultje <rbultje@ronald.bitfreak.net>
 *
 * applet.h: the main applet
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

#ifndef __GVA_APPLET_H__
#define __GVA_APPLET_H__

#include <glib.h>

#include <gtk/gtkicontheme.h>
#include <gtk/gtkimage.h>
#include <gtk/gtktooltips.h>
#include <gconf/gconf-client.h>
#include <panel-applet-gconf.h>
#include <gst/gst.h>
#include <gst/mixer/mixer.h>

#include "dock.h"

G_BEGIN_DECLS

#define GNOME_TYPE_VOLUME_APPLET \
  (gnome_volume_applet_get_type ())
#define GNOME_VOLUME_APPLET(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GNOME_TYPE_VOLUME_APPLET, \
			       GnomeVolumeApplet))
#define GNOME_VOLUME_APPLET_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), GNOME_TYPE_VOLUME_APPLET, \
			    GnomeAppletAppletAppletClass))
#define GNOME_IS_VOLUME_APPLET(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GNOME_TYPE_VOLUME_APPLET))
#define GNOME_IS_VOLUME_APPLET_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), GNOME_TYPE_VOLUME_APPLET))

typedef struct _GnomeVolumeApplet {
  PanelApplet parent;

  /* our main icon, which is our panel user interface */
  GtkImage *image;

  /* the docked window containing the volume widget */
  GnomeVolumeAppletDock *dock;
  gboolean pop;

  /* list of volume control elements */
  GList *elements;

  /* gconf */
  GConfClient *client;

  /* element */
  GstMixer *mixer;
  gboolean lock;
  gint state;
  GList *tracks;

  /* timeout ID, used to decouple on disposal */
  guint timeout;

  /* preferences */
  GtkWidget *prefs;

  /* icon theme */
  GtkIconTheme *icon_theme;

  /* use same object for setting tooltop */
  GtkTooltips *tooltips;
} GnomeVolumeApplet;

typedef struct _GnomeVolumeAppletClass {
  PanelAppletClass klass;
} GnomeVolumeAppletClass;

void     gnome_volume_applet_adjust_volume (GstMixer      *mixer,
					    GstMixerTrack *track,
					    int            volume);
GType    gnome_volume_applet_get_type (void);
gboolean gnome_volume_applet_setup    (GnomeVolumeApplet *applet,
				       GList             *elements);

G_END_DECLS

#endif /* __GVA_APPLET_H__ */
