/* GNOME Volume Applet
 * Copyright (C) 2004 Ronald Bultje <rbultje@ronald.bitfreak.net>
 *
 * preferences.h: preferences screen
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

#ifndef __GVA_PREFERENCES_H__
#define __GVA_PREFERENCES_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <panel-applet.h>
#include <panel-applet-gconf.h>
#include <gst/interfaces/mixer.h>

G_BEGIN_DECLS

#define GNOME_VOLUME_APPLET_TYPE_PREFERENCES \
  (gnome_volume_applet_preferences_get_type ())
#define GNOME_VOLUME_APPLET_PREFERENCES(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GNOME_VOLUME_APPLET_TYPE_PREFERENCES, \
			       GnomeVolumeAppletPreferences))
#define GNOME_VOLUME_APPLET_PREFERENCES_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), GNOME_VOLUME_APPLET_TYPE_PREFERENCES, \
			    GnomeVolumeAppletPreferencesClass))
#define GNOME_VOLUME_APPLET_IS_PREFERENCES(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GNOME_VOLUME_APPLET_TYPE_PREFERENCES))
#define GNOME_VOLUME_APPLET_IS_PREFERENCES_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), GNOME_VOLUME_APPLET_TYPE_PREFERENCES))

typedef struct _GnomeVolumeAppletPreferences {
  GtkDialog parent;

  /* all elements */
  GList *elements;

  /* current element that we're working on */
  GstMixer *mixer;

  /* is the track list currently locked */
  gboolean track_lock;

  /* for gconf */
  PanelApplet *applet;

  /* treeview inside us */
  GtkWidget *optionmenu, *treeview;
} GnomeVolumeAppletPreferences;

typedef struct _GnomeVolumeAppletPreferencesClass {
  GtkDialogClass klass;
} GnomeVolumeAppletPreferencesClass;

GType	gnome_volume_applet_preferences_get_type (void);
GtkWidget *gnome_volume_applet_preferences_new	(PanelApplet *applet,
						 GList       *elements,
						 GstMixer    *mixer,
						 GList       *track);
void	gnome_volume_applet_preferences_change	(GnomeVolumeAppletPreferences *prefs,
						 GstMixer    *mixer,
						 GList       *tracks);

G_END_DECLS

#endif /* __GVA_PREFERENCES_H__ */
