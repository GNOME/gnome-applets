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

#include <glib/glist.h>
#ifdef HAVE_GST10
#include <gst/audio/mixerutils.h>
#else
#include <gst/propertyprobe/propertyprobe.h>
#endif

#include "applet.h"

#ifdef HAVE_GST10
typedef struct _FilterHelper {
   GList *names_list;
   gint count;
} FilterHelper;


static gboolean
_filter_func (GstMixer *mixer, gpointer data) {
   GstElementFactory *factory;
   const gchar *longname;
   FilterHelper *helper = (FilterHelper *)data;
   gchar *device = NULL;
   gchar *name, *original;

   /* fetch name */
   if (g_object_class_find_property (G_OBJECT_GET_CLASS (G_OBJECT (mixer)), "device-name")) {
      g_object_get (mixer, "device-name", &device, NULL);
      GST_DEBUG ("device-name: %s", GST_STR_NULL (device));
   } else {
      GST_DEBUG ("no 'device-name' property: device name unknown");
   }

   factory = gst_element_get_factory (GST_ELEMENT (mixer));
   longname = gst_element_factory_get_longname (factory);

   if (device) {
      GList *list;
      gint duplicates = 0;

      name = g_strdup_printf ("%s (%s)", device, longname);
      g_free (device);

      /* Devices are sorted by names, we must ensure that the names are unique */
      for (list = helper->names_list; list != NULL; list = list->next) {
         if (strcmp ((const gchar *) list->data, name) == 0)
            duplicates++;
      }

      if (duplicates > 0) {
         /*
          * There is a duplicate name, so append an index to make the name
          * unique within the list
          */
          original = name;
          name = g_strdup_printf("%s #%d", name, duplicates + 1);
      } else {
         original = g_strdup(name);
      }
   } else {
      gchar *title;

      (helper->count)++;

      title = g_strdup_printf (_("Unknown Volume Control %d"), helper->count);
      name = g_strdup_printf ("%s (%s)", title, longname);
      original = g_strdup(name);
      g_free (title);
   }

   helper->names_list = g_list_prepend (helper->names_list, name);

   g_object_set_data_full (G_OBJECT (mixer),
                           "gnome-volume-applet-name",
                           name,
                           (GDestroyNotify) g_free);

   /* Is this even used anywhere? */
   g_object_set_data_full (G_OBJECT(mixer),
                           "gnome-volume-applet-origname",
                           original,
                           (GDestroyNotify) g_free);

   GST_DEBUG ("Adding '%s' to the list of available mixers", name);

   gst_element_set_state (GST_ELEMENT (mixer), GST_STATE_NULL);

   return TRUE;
}

static GList *
create_mixer_collection (void)
{
   FilterHelper helper;
   GList *mixer_list;

   helper.count = 0;
   helper.names_list = NULL;

   mixer_list = gst_audio_default_registry_mixer_filter(_filter_func, FALSE, &helper);
   g_free (helper.names_list);

   return mixer_list;
}

#else
static gint
sort_by_rank (GstElement * a, GstElement * b)
{
#define gst_element_rank(x) \
  gst_plugin_feature_get_rank (GST_PLUGIN_FEATURE ( \
      gst_element_get_factory (x)))
  return gst_element_rank (b) - gst_element_rank (a);
}

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
  const GList *elements, *item;
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
    gint n, samenamenr;

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

      /* there may be devices with the same name, and since we sort based
       * on unique names, we need to make sure they really are. */
      samenamenr = 0;
      for (item = collection; item != NULL; item = item->next) {
        const gchar *tname;

        tname = g_object_get_data (G_OBJECT (item->data),
				   "gnome-volume-applet-origname");
        if (!strcmp (tname, name)) {
          samenamenr++;
        }
      }
      if (samenamenr) {
        gchar *tname;

        /* name already exists, so append a number to make it unique */
        tname = g_strdup_printf ("%s #%d", name, samenamenr + 1);
        g_object_set_data (G_OBJECT (element), "gnome-volume-applet-origname",
			   name);
        name = tname;
      } else {
        g_object_set_data (G_OBJECT (element), "gnome-volume-applet-origname",
			   name);
      }

      g_object_set_data (G_OBJECT (element), "gnome-volume-applet-name",
			 name);

      /* add to list */
      gst_element_set_state (element, GST_STATE_NULL);
      collection = g_list_append (collection, element);
      num++;

      /* and recreate this object, since we give it to the mixer */
      g_free (title);
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

  return g_list_sort (collection, (GCompareFunc) sort_by_rank);
}
#endif

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
