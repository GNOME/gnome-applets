/* GNOME Volume Applet
 * Copyright (C) 2004 Ronald Bultje <rbultje@ronald.bitfreak.net>
 *
 * applet.c: the main applet
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

#include <libgnomeui/libgnomeui.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "applet.h"
#include "keys.h"
#include "preferences.h"

#define IS_PANEL_HORIZONTAL(o) \
  (o == PANEL_APPLET_ORIENT_UP || o == PANEL_APPLET_ORIENT_DOWN)

static void	gnome_volume_applet_class_init	(GnomeVolumeAppletClass *klass);
static void	gnome_volume_applet_init	(GnomeVolumeApplet *applet);
static void	gnome_volume_applet_dispose	(GObject   *object);

static void	gnome_volume_applet_popup_dock	(GnomeVolumeApplet *applet);
static void	gnome_volume_applet_popdown_dock (GnomeVolumeApplet *applet);

static gboolean	gnome_volume_applet_scroll	(GtkWidget *widget,
						 GdkEventScroll *event);
static gboolean	gnome_volume_applet_button	(GtkWidget *widget,
						 GdkEventButton *event);
static gboolean	gnome_volume_applet_key		(GtkWidget *widget,
						 GdkEventKey *event);

static void	gnome_volume_applet_background	(PanelApplet *panel_applet,
						 PanelAppletBackgroundType type,
						 GdkColor  *colour,
						 GdkPixmap *pixmap);
static void	gnome_volume_applet_orientation	(PanelApplet *applet,
						 PanelAppletOrient orient);
static void	gnome_volume_applet_size	(PanelApplet *applet,
						 guint      size);

static void	gnome_volume_applet_refresh	(GnomeVolumeApplet *applet);
static gboolean	cb_check			(gpointer   data);

static void	cb_volume			(GtkAdjustment *adj,
						 gpointer   data);

static void	cb_gconf			(GConfClient     *client,
						 guint            connection_id,
						 GConfEntry      *entry,
						 gpointer         data);

static void	cb_verb				(BonoboUIComponent *uic,
						 gpointer   data,
						 const gchar *verbname);

static void	cb_ui_event			(BonoboUIComponent *comp,
						 const gchar       *path,
						 Bonobo_UIComponent_EventType type,
						 const gchar       *state_string,
						 gpointer           data);

static PanelAppletClass *parent_class = NULL;
static struct {
  gchar *filename;
  GdkPixbuf *pixbuf;
} pix[] = {
  { GNOME_PIXMAPSDIR "/mixer/volume-mute.png",   NULL },
  { GNOME_PIXMAPSDIR "/mixer/volume-zero.png",   NULL },
  { GNOME_PIXMAPSDIR "/mixer/volume-min.png",    NULL },
  { GNOME_PIXMAPSDIR "/mixer/volume-medium.png", NULL },
  { GNOME_PIXMAPSDIR "/mixer/volume-max.png",    NULL },
  { NULL, NULL }
};

GType
gnome_volume_applet_get_type (void)
{
  static GType gnome_volume_applet_type = 0;

  if (!gnome_volume_applet_type) {
    static const GTypeInfo gnome_volume_applet_info = {
      sizeof (GnomeVolumeAppletClass),
      NULL,
      NULL,
      (GClassInitFunc) gnome_volume_applet_class_init,
      NULL,
      NULL,
      sizeof (GnomeVolumeApplet),
      0,
      (GInstanceInitFunc) gnome_volume_applet_init,
      NULL
    };

    gnome_volume_applet_type =
	g_type_register_static (PANEL_TYPE_APPLET, 
				"GnomeVolumeApplet",
				&gnome_volume_applet_info, 0);
  }

  return gnome_volume_applet_type;
}

/*
 * Hi, I'm slow.
 */

static inline void
flip_byte (gchar *one, gchar *two)
{
  gint temp;

  temp = *one;
  *one = *two;
  *two = temp;
}

static inline void
flip_pixel (gchar *line, gint pixel, gint width, gint bpp)
{
  gint n;

  for (n = 0; n < bpp; n++) {
    flip_byte (&line[pixel * bpp + n], &line[(width - 1 - pixel) * bpp + n]);
  }
}

static void
flip (GdkPixbuf *pix)
{
  gint w = gdk_pixbuf_get_width (pix),
       h = gdk_pixbuf_get_height (pix),
       x, y, stride = gdk_pixbuf_get_rowstride (pix),
       bpp = gdk_pixbuf_get_n_channels (pix);
  gchar *data = gdk_pixbuf_get_pixels (pix);

  for (y = 0; y < h; y++) {
    for (x = 0; x < (w / 2); x++) {
      flip_pixel (data, x, w, bpp);
    }
    data += stride;
  }
}

static void
gnome_volume_applet_class_init (GnomeVolumeAppletClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *gtkwidget_class = GTK_WIDGET_CLASS (klass);
  PanelAppletClass *panelapplet_class = PANEL_APPLET_CLASS (klass);
  gint n;

  parent_class = g_type_class_ref (PANEL_TYPE_APPLET);

  gobject_class->dispose = gnome_volume_applet_dispose;
  gtkwidget_class->key_press_event = gnome_volume_applet_key;
  gtkwidget_class->button_press_event = gnome_volume_applet_button;
  gtkwidget_class->scroll_event = gnome_volume_applet_scroll;
  panelapplet_class->change_orient = gnome_volume_applet_orientation;
  panelapplet_class->change_size = gnome_volume_applet_size;
  panelapplet_class->change_background = gnome_volume_applet_background;

  /* FIXME:
   * - style-set.
   */

  /* init pixbufs */
  for (n = 0; pix[n].filename != NULL; n++) {
    pix[n].pixbuf = gdk_pixbuf_new_from_file (pix[n].filename, NULL);
    if (gtk_widget_get_default_direction () == GTK_TEXT_DIR_RTL) {
      flip (pix[n].pixbuf);
    }
  }
}

static void
gnome_volume_applet_init (GnomeVolumeApplet *applet)
{
  GtkWidget *image, *dock;
  GtkTooltips *tooltips;

  applet->timeout = 0;
  applet->elements = NULL;
  applet->client = gconf_client_get_default ();
  applet->mixer = NULL;
  applet->track = NULL;
  applet->lock = FALSE;
  applet->state = -1;
  applet->prefs = NULL;

  /* icon (our main UI) */
  image = gtk_image_new ();
  applet->image = GTK_IMAGE (image);
  gtk_container_add (GTK_CONTAINER (applet), image);
  gtk_widget_show (image);

  /* dock window (expanded UI) */
  applet->pop = FALSE;

  /* tooltip over applet */
  tooltips = gtk_tooltips_new ();
  gtk_tooltips_set_tip (tooltips, GTK_WIDGET (applet),
			_("Volume Control"), NULL);

  /* other stuff */
  panel_applet_add_preferences (PANEL_APPLET (applet),
				"/schemas/apps/mixer_applet/prefs",
				NULL);
  panel_applet_set_flags (PANEL_APPLET (applet),
			  PANEL_APPLET_EXPAND_MINOR);
}

void
gnome_volume_applet_setup (GnomeVolumeApplet *applet,
			   GList *elements)
{
  GConfValue *value;
  GtkObject *adj;
  GstElement *active_element;
  GstMixerTrack *active_track;
  const GList *item;
  gint page;
  BonoboUIComponent *component;
  static const BonoboUIVerb verbs[] = {
    BONOBO_UI_UNSAFE_VERB ("RunMixer", cb_verb),
    BONOBO_UI_UNSAFE_VERB ("Mute",     cb_verb),
    BONOBO_UI_UNSAFE_VERB ("Help",     cb_verb),
    BONOBO_UI_UNSAFE_VERB ("About",    cb_verb),
    BONOBO_UI_UNSAFE_VERB ("Pref",     cb_verb),
    BONOBO_UI_VERB_END
  };

  /* default element to first */
  g_return_if_fail (elements != NULL);
  active_element = elements->data;

  applet->elements = elements;

  /* get active element, if any (otherwise we use the default) */
  if ((value = gconf_client_get (applet->client,
				 GNOME_VOLUME_APPLET_KEY_ACTIVE_ELEMENT,
				 NULL)) != NULL &&
      value->type == GCONF_VALUE_STRING) {
    const gchar *active_el_str, *cur_el_str;

    active_el_str = gconf_value_get_string (value);
    for (item = elements; item != NULL; item = item->next) {
      cur_el_str = g_object_get_data (item->data, "gnome-volume-applet-name");
      if (!strcmp (active_el_str, cur_el_str)) {
        active_element = item->data;
        break;
      }
    }
  }

  /* use the active element */
  gst_element_set_state (active_element, GST_STATE_READY);
  applet->mixer = GST_MIXER (active_element);
  gst_object_ref (GST_OBJECT (active_element));

  /* default track: first */
  item = gst_mixer_list_tracks (applet->mixer);
  active_track = item->data;
  /* FIXME:
   * - what if first has 0 channels?
   */

  /* get active track, if any (otherwise we use the default) */
  if ((value = gconf_client_get (applet->client,
				 GNOME_VOLUME_APPLET_KEY_ACTIVE_TRACK,
				 NULL)) != NULL &&
      value->type == GCONF_VALUE_STRING) {
    const gchar *track_name = gconf_value_get_string (value);

    for ( ; item != NULL; item = item->next) {
      GstMixerTrack *track = item->data;

      if (!track->num_channels)
        continue;

      if (!strcmp (track_name, track->label)) {
        active_track = item->data;
        break;
      } else if (!track &&
		 GST_MIXER_TRACK_HAS_FLAG (track,
					   GST_MIXER_TRACK_MASTER)) {
        active_track = item->data;
      }
    }
  }

  /* use the active track */
  applet->track = active_track;
  g_object_ref (G_OBJECT (active_track));

  /* tell the dock */
  page = (applet->track->max_volume - applet->track->min_volume) / 10;
  adj = gtk_adjustment_new (0, applet->track->min_volume,
			    applet->track->max_volume,
			    (page / 5 > 0) ? page / 5 : 1,
			    page, 0);
  gnome_volume_applet_orientation (PANEL_APPLET (applet),
				   panel_applet_get_orient (PANEL_APPLET (applet)));
  gnome_volume_applet_dock_change (applet->dock,
				   GTK_ADJUSTMENT (adj));
  g_signal_connect (adj, "value-changed",
		    G_CALLBACK (cb_volume), applet);

  gnome_volume_applet_refresh (applet);
  applet->timeout = g_timeout_add (100, cb_check, applet);

  /* menu - done here because bonobo is intialized now */
  panel_applet_setup_menu_from_file (PANEL_APPLET (applet), 
				     DATADIR,
				     "GNOME_MixerApplet.xml",
				     NULL, verbs, applet);
  component = panel_applet_get_popup_component (PANEL_APPLET (applet));
  g_signal_connect (component, "ui-event", G_CALLBACK (cb_ui_event), applet);

  /* gconf */
  gconf_client_add_dir (applet->client, GNOME_VOLUME_APPLET_KEY_DIR,
			GCONF_CLIENT_PRELOAD_RECURSIVE, NULL);
  gconf_client_notify_add (applet->client, GNOME_VOLUME_APPLET_KEY_DIR,
			   cb_gconf, applet, NULL, NULL);

  gtk_widget_show (GTK_WIDGET (applet));
}

static void
gnome_volume_applet_dispose (GObject *object)
{
  GnomeVolumeApplet *applet = GNOME_VOLUME_APPLET (object);

  gnome_volume_applet_popdown_dock (applet);

  if (applet->elements) {
    GList *item;

    for (item = applet->elements; item != NULL; item = item->next) {
      GstElement *element = GST_ELEMENT (item->data);

      gst_element_set_state (element, GST_STATE_NULL);
      g_free (g_object_get_data (G_OBJECT (element),
				 "gnome-volume-applet-name"));
      gst_object_unref (GST_OBJECT (element));
    }
    g_list_free (applet->elements);
    applet->elements = NULL;
  }

  if (applet->track) {
    g_object_unref (G_OBJECT (applet->track));
    applet->track = NULL;
  }

  if (applet->mixer) {
    gst_object_unref (GST_OBJECT (applet->mixer));
    applet->mixer = NULL;
  }

  if (applet->timeout) {
    g_source_remove (applet->timeout);
    applet->timeout = 0;
  }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

/*
 * get position that the dock should get based on applet position.
 */

static void
gnome_volume_applet_get_dock_position (GnomeVolumeApplet *applet,
				       gint *_x, gint *_y)
{
  GtkWidget *widget = GTK_WIDGET (applet);
  gint x, y;

  gdk_window_get_origin (widget->window, &x, &y);
  switch (panel_applet_get_orient (PANEL_APPLET (applet))) {
    case PANEL_APPLET_ORIENT_DOWN:
      x += widget->allocation.x;
      x -= (GTK_WIDGET (applet->dock)->allocation.width -
          widget->allocation.width) / 2;
      y += widget->allocation.height + widget->allocation.y;
      break;
    case PANEL_APPLET_ORIENT_UP:
      x += widget->allocation.x;
      x -= (GTK_WIDGET (applet->dock)->allocation.width -
          widget->allocation.width) / 2;
      y += widget->allocation.y;
      y -= GTK_WIDGET (applet->dock)->allocation.height;
      break;
    case PANEL_APPLET_ORIENT_RIGHT:
      x += widget->allocation.width + widget->allocation.x;
      y += widget->allocation.y;
      y -= (GTK_WIDGET (applet->dock)->allocation.height -
          widget->allocation.height) / 2;
      break;
    case PANEL_APPLET_ORIENT_LEFT:
      x += widget->allocation.x;
      x -= GTK_WIDGET (applet->dock)->allocation.width;
      y += widget->allocation.y;
      y -= (GTK_WIDGET (applet->dock)->allocation.height -
          widget->allocation.height) / 2;
      break;
    default:
      g_assert_not_reached ();
  }

  *_x = x;
  *_y = y;
}

/*
 * popup (show) or popdown (hide) the dock.
 */

static void
gnome_volume_applet_popup_dock (GnomeVolumeApplet *applet)
{
  GtkWidget *widget = GTK_WIDGET (applet);
  gint x, y;

  /* show (before reposition, so size allocation is done) */
  gtk_widget_show (GTK_WIDGET (applet->dock));

  /* reposition */
  gnome_volume_applet_get_dock_position (applet, &x, &y);
  gtk_window_move (GTK_WINDOW (applet->dock), x, y);

  /* grab input */
  gtk_widget_grab_focus (widget);
  gtk_grab_add (widget);
  gdk_pointer_grab (widget->window, TRUE,
		    GDK_BUTTON_PRESS_MASK |
		    GDK_BUTTON_RELEASE_MASK |
		    GDK_POINTER_MOTION_MASK,
		    NULL, NULL, GDK_CURRENT_TIME);
  gdk_keyboard_grab (widget->window, TRUE, GDK_CURRENT_TIME);

  /* set menu item as active */
  gtk_widget_set_state (GTK_WIDGET (applet), GTK_STATE_SELECTED);

  /* keep state */
  applet->pop = TRUE;
}

static void
gnome_volume_applet_popdown_dock (GnomeVolumeApplet *applet)
{
  GtkWidget *widget = GTK_WIDGET (applet);

  if (!applet->pop)
    return;

  /* release input */
  gdk_keyboard_ungrab (GDK_CURRENT_TIME);
  gdk_pointer_ungrab (GDK_CURRENT_TIME);
  gtk_grab_remove (widget);

  /* hide */
  gtk_widget_hide (GTK_WIDGET (applet->dock));

  /* set menu item as active */
  gtk_widget_set_state (GTK_WIDGET (applet), GTK_STATE_NORMAL);

  /* keep state */
  applet->pop = FALSE;
}

static void
gnome_volume_applet_pop_dock (GnomeVolumeApplet *applet)
{
  if (applet->pop) {
    gnome_volume_applet_popdown_dock (applet);
  } else {
    gnome_volume_applet_popup_dock (applet);
  }
}

/*
 * Toggle mute.
 */

static void
gnome_volume_applet_toggle_mute (GnomeVolumeApplet *applet)
{
  BonoboUIComponent *component;
  gboolean mute = (GST_MIXER_TRACK_HAS_FLAG (applet->track,
			GST_MIXER_TRACK_MUTE));

  gst_mixer_set_mute (applet->mixer, applet->track, !mute);

  /* update component */
  component = panel_applet_get_popup_component (PANEL_APPLET (applet));
  bonobo_ui_component_set_prop (component,
			        "/commands/Mute",
			        "state", mute ? "1" : "0", NULL);

  /* update graphic - this should happen automagically after the next
   * idle call, but apparently doesn't for some people... */
  gnome_volume_applet_refresh (applet);
}

/*
 * Run g-v-c.
 */

static void
gnome_volume_applet_run_mixer (GnomeVolumeApplet *applet)
{
  GError *error = NULL;

  gdk_spawn_command_line_on_screen (
      gtk_widget_get_screen (GTK_WIDGET (applet)),
      "gnome-volume-control", &error);

  if (error) {
    GtkWidget *dialog;

    dialog = gtk_message_dialog_new (NULL, 0, GTK_MESSAGE_ERROR,
				     GTK_BUTTONS_CLOSE,
				     _("Failed to start Volume Control: %s"),
				     error->message);
    g_signal_connect (dialog, "response",
		      G_CALLBACK (gtk_widget_destroy), NULL);
    gtk_widget_show (dialog);
    g_error_free (error);
  }
}

/*
 * Control events, change volume and so on.
 */

static gboolean
gnome_volume_applet_scroll (GtkWidget      *widget,
			    GdkEventScroll *event)
{
  GnomeVolumeApplet *applet = GNOME_VOLUME_APPLET (widget);

  if (event->type == GDK_SCROLL) {
    switch (event->direction) {
      case GDK_SCROLL_UP:
      case GDK_SCROLL_DOWN: {
        GtkAdjustment *adj = gtk_range_get_adjustment (applet->dock->scale);
        gint volume = adj->value;

        if (event->direction == GDK_SCROLL_UP) {
          volume += adj->step_increment;
          if (volume > adj->upper)
            volume = adj->upper;
        } else {
          volume -= adj->step_increment;
          if (volume < adj->lower)
            volume = adj->lower;
        }

        gtk_range_set_value (applet->dock->scale, volume);
        return TRUE;
      }
      default:
        break;
    }
  }

  return GTK_WIDGET_CLASS (parent_class)->scroll_event (widget, event);
}

static gboolean
gnome_volume_applet_button (GtkWidget      *widget,
			    GdkEventButton *event)
{
  GnomeVolumeApplet *applet = GNOME_VOLUME_APPLET (widget);

  if (event->window != GTK_WIDGET (applet)->window) {
    gnome_volume_applet_popdown_dock (applet);
    return TRUE;
  } else {
    switch (event->button) {
      case 1:
        switch (event->type) {
          case GDK_BUTTON_PRESS:
            gnome_volume_applet_pop_dock (applet);
            return TRUE;
          case GDK_2BUTTON_PRESS:
            gnome_volume_applet_popdown_dock (applet);
            gnome_volume_applet_run_mixer (applet);
            return TRUE;
          default:
            break;
        }
        break;
      case 2: /* move */
      case 3: /* menu */
        if (applet->pop) {
          gnome_volume_applet_popdown_dock (applet);
          return TRUE;
        }
        break;
      default:
        break;
    }
  }

  if (GTK_WIDGET_CLASS (parent_class)->button_press_event)
    return GTK_WIDGET_CLASS (parent_class)->button_press_event (widget, event);

  return FALSE;
}

static gboolean
gnome_volume_applet_key (GtkWidget   *widget,
			 GdkEventKey *event)
{
  GnomeVolumeApplet *applet = GNOME_VOLUME_APPLET (widget);

  switch (event->keyval) {
    case GDK_KP_Enter:
    case GDK_ISO_Enter:
    case GDK_3270_Enter:
    case GDK_Return:
    case GDK_space:
    case GDK_KP_Space:
      gnome_volume_applet_pop_dock (applet);
      return TRUE;
    case GDK_m:
      if (event->state == GDK_CONTROL_MASK) {
        gnome_volume_applet_toggle_mute (applet);
        return TRUE;
      }
      break;
    case GDK_o:
      if (event->state == GDK_CONTROL_MASK) {
        gnome_volume_applet_run_mixer (applet);
        return TRUE;
      }
      break;
    case GDK_Escape:
      gnome_volume_applet_popdown_dock (applet);
      return TRUE;
    case GDK_Page_Up:
    case GDK_Page_Down:
    case GDK_Up:
    case GDK_Down: {
      GtkAdjustment *adj = gtk_range_get_adjustment (applet->dock->scale);
      gint volume = adj->value, increment;

      if (event->keyval == GDK_Up || event->keyval == GDK_Down)
        increment = adj->step_increment;
      else
        increment = adj->page_increment;

      if (event->keyval == GDK_Page_Up || event->keyval == GDK_Up) {
        volume += increment;
        if (volume > adj->upper)
          volume = adj->upper;
      } else {
        volume -= increment;
        if (volume < adj->lower)
          volume = adj->lower;
      }

      gtk_range_set_value (applet->dock->scale, volume);
      return TRUE;
    }
    default:
      break;
  }

  return GTK_WIDGET_CLASS (parent_class)->key_press_event (widget, event);
}

/*
 * Change orientation or size of panel.
 */

static void
gnome_volume_applet_orientation	(PanelApplet *_applet,
				 PanelAppletOrient orientation)
{
  GnomeVolumeApplet *applet = GNOME_VOLUME_APPLET (_applet);
  GtkAdjustment *adj = NULL;
  GtkWidget *dock;

  if (applet->dock) {
    adj = gtk_range_get_adjustment (applet->dock->scale);
    g_object_ref (G_OBJECT (adj));
    /* FIXME:
     * - we need to unset the parent in some way, because else Gtk+
     *    thinks that the child of the applet (image) is us (dock),
     *    which is not the case.
     */
    gtk_widget_destroy (GTK_WIDGET (applet->dock));
  }
  dock = gnome_volume_applet_dock_new (IS_PANEL_HORIZONTAL (orientation) ?
      GTK_ORIENTATION_VERTICAL : GTK_ORIENTATION_HORIZONTAL);
  /* parent, for signal forwarding */
  gtk_widget_set_parent (dock, GTK_WIDGET (applet));
  applet->dock = GNOME_VOLUME_APPLET_DOCK (dock);
  gnome_volume_applet_dock_change (applet->dock, adj);

  if (PANEL_APPLET_CLASS (parent_class)->change_orient)
    PANEL_APPLET_CLASS (parent_class)->change_orient (_applet, orientation);
}

static void
gnome_volume_applet_size (PanelApplet *applet,
			  guint        size)
{
  gnome_volume_applet_refresh (GNOME_VOLUME_APPLET (applet));

  if (PANEL_APPLET_CLASS (parent_class)->change_size)
    PANEL_APPLET_CLASS (parent_class)->change_size (applet, size);
}

static void
gnome_volume_applet_background (PanelApplet *_applet,
				PanelAppletBackgroundType type,
				GdkColor  *colour,
				GdkPixmap *pixmap)
{
  GnomeVolumeApplet *applet = GNOME_VOLUME_APPLET (_applet);
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

/*
 * Volume changed.
 */

static void
cb_volume (GtkAdjustment *adj,
	   gpointer data)
{
  GnomeVolumeApplet *applet = data;
  gint *volumes, n;

  if (applet->lock)
    return;
  applet->lock = TRUE;

  volumes = g_new (gint, applet->track->num_channels);
  for (n = 0; n < applet->track->num_channels; n++)
    volumes[n] = gtk_adjustment_get_value (adj);
  gst_mixer_set_volume (applet->mixer, applet->track, volumes);
  g_free (volumes);

  applet->lock = FALSE;
}

/*
 * Automatic timer. Check for changes.
 */

#define STATE(vol,m) ((vol << 1) | m)

static void
gnome_volume_applet_refresh (GnomeVolumeApplet *applet)
{
  BonoboUIComponent *component;
  GdkPixbuf *scaled, *orig;
  guint size;
  gint n, *volumes, volume = 0;
  gboolean mute;

  /* build-up phase */
  if (!applet->track)
    return;

  volumes = g_new (gint, applet->track->num_channels);
  gst_mixer_get_volume (applet->mixer, applet->track, volumes);
  for (n = 0; n < applet->track->num_channels; n++)
    volume += volumes[n];
  volume /= applet->track->num_channels;
  g_free (volumes);
  mute = GST_MIXER_TRACK_HAS_FLAG (applet->track,
				   GST_MIXER_TRACK_MUTE);

  n = 4 * volume / (applet->track->max_volume -
      applet->track->min_volume) + 1;
  if (n <= 0)
    n = 1;
  if (n >= 5)
    n = 4;

  if (STATE (n, mute) != applet->state) {
    if (mute) {
      orig = gdk_pixbuf_copy (pix[n].pixbuf);
      gdk_pixbuf_composite (pix[0].pixbuf, orig, 0, 0,
			    gdk_pixbuf_get_width (orig),
			    gdk_pixbuf_get_height (orig),
			    0, 0, 1.0, 1.0,
			    GDK_INTERP_BILINEAR, 255);
    } else {
      orig = pix[n].pixbuf;
    }

    size = panel_applet_get_size (PANEL_APPLET (applet));
    scaled = gdk_pixbuf_scale_simple (orig, size, size,
				      GDK_INTERP_BILINEAR);
    gtk_image_set_from_pixbuf (applet->image, scaled);
    if (mute)
      gdk_pixbuf_unref (orig);
    gdk_pixbuf_unref (scaled);

    applet->state = STATE (n, mute);
  }

  applet->lock = TRUE;
  gtk_range_set_value (applet->dock->scale, volume);
  applet->lock = FALSE;

  /* update mute status (bonobo) */
  component = panel_applet_get_popup_component (PANEL_APPLET (applet));
  bonobo_ui_component_set_prop (component,
				"/commands/Mute",
				"state", mute ? "1" : "0", NULL);
}

static gboolean
cb_check (gpointer data)
{
  gnome_volume_applet_refresh (GNOME_VOLUME_APPLET (data));

  return TRUE;
}

/*
 * GConf callback.
 */

static void
cb_gconf (GConfClient *client,
	  guint        connection_id,
	  GConfEntry  *entry,
	  gpointer     data)
{
  GnomeVolumeApplet *applet = data;
  GConfValue *value;
  const gchar *str;
  const GList *item;
  GstMixerTrack *active_track = NULL;
  gboolean newdevice = FALSE;

  if ((value = gconf_entry_get_value (entry)) != NULL &&
      (value->type == GCONF_VALUE_STRING) &&
      (str = gconf_value_get_string (value)) != NULL) {
    if (!strcmp (gconf_entry_get_key (entry),
		 GNOME_VOLUME_APPLET_KEY_ACTIVE_ELEMENT)) {
      for (item = applet->elements; item != NULL; item = item->next) {
        gchar *cur_el_str = g_object_get_data (item->data,
					       "gnome-volume-applet-name");

        if (!strcmp (cur_el_str, str)) {
          GstElement *old_element = GST_ELEMENT (applet->mixer);

          /* change element */
          gst_element_set_state (item->data, GST_STATE_READY);
          gst_object_replace ((GstObject **) &applet->mixer, item->data);
          gst_element_set_state (old_element, GST_STATE_NULL);

          newdevice = TRUE;
          active_track = gst_mixer_list_tracks (applet->mixer)->data;
          str = applet->track->label;
          break;
        }
      }
    }

    if (!strcmp (gconf_entry_get_key (entry),
		 GNOME_VOLUME_APPLET_KEY_ACTIVE_TRACK) ||
        newdevice) {
      for (item = gst_mixer_list_tracks (applet->mixer);
           item != NULL; item = item->next) {
        GstMixerTrack *track = item->data;

        if (track->num_channels <= 0)
          continue;

        if (!strcmp (str, track->label)) {
          active_track = track;
          break;
        } else if (newdevice && !active_track &&
		   GST_MIXER_TRACK_HAS_FLAG (track,
					     GST_MIXER_TRACK_MASTER)) {
          active_track = item->data;
        }
      }

      if (active_track) {
        GtkObject *adj;
        gint page;

        g_object_unref (G_OBJECT (applet->track));
        applet->track = active_track;
        g_object_ref (G_OBJECT (active_track));

        /* dock */
        page = (applet->track->max_volume - applet->track->min_volume) / 10;
        adj = gtk_adjustment_new (0, applet->track->min_volume,
				  applet->track->max_volume,
				  (page / 5 > 0) ? page / 5 : 1,
				  page, 0);
        gnome_volume_applet_dock_change (applet->dock,
					 GTK_ADJUSTMENT (adj));
        g_signal_connect (adj, "value-changed",
			  G_CALLBACK (cb_volume), applet);

        /* if preferences window is open, update */
        if (applet->prefs) {
          gnome_volume_applet_preferences_change (
	      GNOME_VOLUME_APPLET_PREFERENCES (applet->prefs),
	      applet->mixer, applet->track);
        }
      }
    }
  }
}

/*
 * bonobo verb callback.
 */

static void
cb_prefs_destroy (GtkWidget *widget,
		  gpointer   data)
{
  GNOME_VOLUME_APPLET (data)->prefs = NULL;
}

static void
cb_verb (BonoboUIComponent *uic,
	 gpointer           data,
	 const gchar       *verbname)
{
  GnomeVolumeApplet *applet = GNOME_VOLUME_APPLET (data);

  if (!strcmp (verbname, "RunMixer")) {
    gnome_volume_applet_run_mixer (applet);
  } else if (!strcmp (verbname, "Help")) {
    GError *error = NULL;

    gnome_help_display_on_screen ("mixer_applet2", NULL,
				  gtk_widget_get_screen (GTK_WIDGET (applet)),
				  &error);

    if (error) {
      GtkWidget *dialog;

      dialog = gtk_message_dialog_new (NULL, 0, GTK_MESSAGE_ERROR,
				       GTK_BUTTONS_CLOSE,
				       _("Failed to display help: %s"),
				       error->message);
      g_signal_connect (dialog, "response",
			G_CALLBACK (gtk_widget_destroy), NULL);
      gtk_widget_show (dialog);
      g_error_free (error);
    }
  } else if (!strcmp (verbname, "About")) {
	  
    const gchar *authors[] = { "Ronald Bultje <rbultje@ronald.bitfreak.net>",
			     NULL };

    gtk_show_about_dialog (NULL,
		"name",		_("Volume Applet"),
		"version",	VERSION,
		"copyright",	"\xC2\xA9 2004 Ronald Bultje",
		"comments",	_("A GNOME/GStreamer-based volume control "
				  "applet"),
		"authors",	authors,
		"translator-credits",	_("translator-credits"),
		"logo-icon-name",	"gnome-mixer-applet",
		NULL);

  } else if (!strcmp (verbname, "Pref")) {
    if (applet->prefs)
      return;

    applet->prefs = gnome_volume_applet_preferences_new (applet->client,
							 applet->elements,
							 applet->mixer,
							 applet->track);
    g_signal_connect (applet->prefs, "destroy",
		      G_CALLBACK (cb_prefs_destroy), applet);
    gtk_widget_show (applet->prefs);
  } else {
    g_warning ("Unknown bonobo command '%s'", verbname);
  }
}

static void
cb_ui_event (BonoboUIComponent *comp,
	     const gchar       *verbname,
	     Bonobo_UIComponent_EventType type,
	     const gchar       *state_string,
	     gpointer           data)
{
  GnomeVolumeApplet *applet = GNOME_VOLUME_APPLET (data);

  if (!strcmp (verbname, "Mute")) {
    /* mute will have a value of 4 without the ? TRUE : FALSE bit... */
    gboolean mute = (GST_MIXER_TRACK_HAS_FLAG (applet->track,
                        GST_MIXER_TRACK_MUTE)) ? TRUE : FALSE,
             want_mute = !strcmp (state_string, "1") ? TRUE : FALSE;

    if (mute != want_mute)
      gnome_volume_applet_toggle_mute (applet);
  } else {
    g_warning ("Unknown bonobo command '%s'", verbname);
  }
}
