/* GNOME Volume Applet
 * Copyright (C) 2004 Ronald Bultje <rbultje@ronald.bitfreak.net>
 *
 * load.c: applet boilerplate code
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

#include <string.h>

#include <gtk/gtk.h>
#include <gst/propertyprobe/propertyprobe.h>

#include "applet.h"

/*
 * Probe for mixer elements. Set up GList * with elements,
 * where each element has a GObject data node set of the
 * name "gnome-volume-control-name" with the value being
 * the human-readable name of the element.
 *
 * All elements in the returned GList * are in state
 * GST_STATE_NULL.
 */

static GList *
create_mixer_collection (void)
{
  const GList *elements;
  GList *collection = NULL;
  gint num = 0;

  /* go through all elements of a certain class and check whether
   * they implement a mixer. If so, add it to the list. */
  elements = gst_registry_pool_feature_list (GST_TYPE_ELEMENT_FACTORY);
  for ( ; elements != NULL; elements = elements->next) {
    GstElementFactory *factory = GST_ELEMENT_FACTORY (elements->data);
    gchar *title = NULL, *name;
    const gchar *klass;
    GstElement *element = NULL;
    const GParamSpec *devspec;
    GstPropertyProbe *probe;
    GValueArray *array = NULL;
    gint n;

    /* check category */
    klass = gst_element_factory_get_klass (factory);
    if (strcmp (klass, "Generic/Audio"))
      goto next;

    /* FIXME:
     * - maybe we want to rename the element to its actual name
     *     if we've found that?
     */
#define _label N_("Unknown Volume Control %d")

    /* create element */
    title = g_strdup_printf (gettext("Unknown Volume Control %d"), num);
    element = gst_element_factory_create (factory, title);
    if (!element)
      goto next;

    if (!GST_IS_PROPERTY_PROBE (element))
      goto next;

    probe = GST_PROPERTY_PROBE (element);
    if (!(devspec = gst_property_probe_get_property (probe, "device")))
      goto next;
    if (!(array = gst_property_probe_probe_and_get_values (probe, devspec)))
      goto next;

    /* set all devices and test for mixer */
    for (n = 0; n < array->n_values; n++) {
      GValue *device = g_value_array_get_nth (array, n);

      /* set this device */
      g_object_set_property (G_OBJECT (element), "device", device);
      if (gst_element_set_state (element,
				 GST_STATE_READY) == GST_STATE_FAILURE)
        continue;

      /* is this device a mixer? */
      if (!GST_IS_MIXER (element)) {
        gst_element_set_state (element, GST_STATE_NULL);
        continue;
      }

      /* any tracks? */
      if (!gst_mixer_list_tracks (GST_MIXER (element))) {
        gst_element_set_state (element, GST_STATE_NULL);
        continue;
      }

      /* fetch name */
      if (g_object_class_find_property (G_OBJECT_GET_CLASS (G_OBJECT (element)),
					"device-name")) {
        gchar *devname;
        g_object_get (element, "device-name", &devname, NULL);
        name = g_strdup_printf ("%s (%s)", devname,
				gst_element_factory_get_longname (factory));
      } else {
        name = g_strdup_printf ("%s (%s)", title,
				gst_element_factory_get_longname (factory));
      }
      g_object_set_data (G_OBJECT (element), "gnome-volume-applet-name",
			 name);

      /* add to list */
      gst_element_set_state (element, GST_STATE_NULL);
      collection = g_list_append (collection, element);
      num++;

      /* and recreate this object, since we give it to the mixer */
      title = g_strdup_printf (gettext("Unknown Volume Control %d"), num);
      element = gst_element_factory_create (factory, title);
    }

next:
    if (element)
      gst_object_unref (GST_OBJECT (element));
    if (array)
      g_value_array_free (array);
    g_free (title);
  }

  return collection;
}

static gboolean
gnome_volume_applet_factory (PanelApplet *applet,
			     const gchar *iid,
			     gpointer     data)
{
  GList *elements;
  static gboolean init = FALSE;

  if (!init) {
    gst_init (NULL, NULL);
    init = TRUE;
  }

  elements = create_mixer_collection ();
  if (!elements) {
    GtkWidget *dialog;

    dialog = gtk_message_dialog_new (NULL, 0, GTK_MESSAGE_ERROR,
				     GTK_BUTTONS_CLOSE,
				     _("No volume control elements and/or devices found."));
    gtk_widget_show (dialog);
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);

    return FALSE;
  }
  gnome_volume_applet_setup (GNOME_VOLUME_APPLET (applet), elements);

  return TRUE;
}

PANEL_APPLET_BONOBO_FACTORY (
  "OAFIID:GNOME_MixerApplet_Factory",
  GNOME_TYPE_VOLUME_APPLET,
  "mixer_applet2",
  "0",
  gnome_volume_applet_factory,
  NULL
)
