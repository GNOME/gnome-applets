/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 2 -*- */
/* File: gkb.c
 * Purpose: GNOME Keyboard switcher
 *
 * Copyright (C) 1998-2000 Free Software Foundation
 * Authors: Szabolcs Ban  <shooby@gnome.hu>
 *          Chema Celorio <chema@celorio.com>
 *
 * Thanks for aid of George Lebl <jirka@5z.com> and solidarity
 * Balazs Nagy <js@lsc.hu>, Charles Levert <charles@comm.polymtl.ca>
 * and Emese Kovacs <emese@gnome.hu> for her docs and ideas.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include <X11/keysym.h>
#include <X11/Xmd.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <gdk/gdkx.h>
#include <sys/stat.h>

#include <panel-applet.h>

#include <gtk/gtk.h>
#include <libbonobo.h>
#include <libgnomeui/libgnomeui.h>
#include <libgnome/libgnome.h>
#include <panel-applet-gconf.h>

#include "gkb.h"

int NumLockMask, CapsLockMask, ScrollLockMask;

GtkWidget *bah_window = NULL;

gchar *
gkb_load_pref(const gchar * key, const gchar * defaultv);

static gint gkb_button_press_event_cb (GtkWidget * widget,
				       GdkEventButton * e);
static void about_cb (BonoboUIComponent *uic,
			GKB	  *gkb,
			const gchar	  *verbname);
static void help_cb (BonoboUIComponent *uic,
		        GKB       *gkb,
	            const gchar	  *verbname);
static GdkFilterReturn
event_filter (GdkXEvent * gdk_xevent, GdkEvent * event, gpointer data);

static void gkb_destroy (GtkWidget * widget, gpointer data);

void alert(const gchar * str){
 GtkWidget * window;
 window = gnome_message_box_new (str,
               GNOME_MESSAGE_BOX_INFO,
               GNOME_STOCK_BUTTON_OK, NULL);
 gtk_widget_show_all(window);
}

static void
makepix (GkbKeymap * keymap, char *fname, int w, int h)
{
  GdkPixbuf *pix;
  int width, height;

  pix = gdk_pixbuf_new_from_file (fname,NULL);
  if (pix != NULL)
    {
      double affine[6];
      guchar *rgb;
      GdkGC *gc;

      g_assert (bah_window);
      g_assert (bah_window->window);

      width = gdk_pixbuf_get_width (pix);
      height = gdk_pixbuf_get_height (pix);

      affine[1] = affine[2] = affine[4] = affine[5] = 0;

      affine[0] = w / (double) width;
      affine[3] = h / (double) height;

      rgb = g_new0 (guchar, w * h * 3);

      art_rgb_rgba_affine (rgb,
			   0, 0, w, h, w * 3,
			   gdk_pixbuf_get_pixels (pix),
			   width, height,
			   gdk_pixbuf_get_rowstride (pix),
			   affine, ART_FILTER_NEAREST, NULL);

      gdk_pixbuf_unref (pix);

      keymap->pix = gdk_pixmap_new (bah_window->window, w, h,
#if 0
				    gtk_widget_get_visual (gkb->darea)->depth
#else
				    -1
#endif
	);

      gc = gdk_gc_new (keymap->pix);
      gdk_draw_rgb_image (keymap->pix, gc,
			  0, 0, w, h, GDK_RGB_DITHER_NORMAL, rgb, w * 3);
      gdk_gc_destroy (gc);

      g_free (rgb);
    }
  else
    {
      if (keymap->pix)
	{
	  gdk_pixmap_unref (keymap->pix);
	}
      keymap->pix = gdk_pixmap_new (bah_window->window, w, h, -1);
    }
}

/**
 * gkb_draw:
 * @gkb: 
 * 
 * Draw the applet from the pixmap loaded in gkb->dact
 **/
static void
gkb_draw (GKB * gkb)
{
  g_return_if_fail (gkb->darea != NULL);
  g_return_if_fail (gkb->keymap != NULL);
  g_return_if_fail (GTK_WIDGET_REALIZED (gkb->darea));

  if (gkb->mode != GKB_LABEL)
    {
      g_return_if_fail (gkb != NULL);
      g_return_if_fail (gkb->keymap != NULL);
      g_return_if_fail (gkb->keymap->pix != NULL);
      g_return_if_fail (gkb->darea);
      g_return_if_fail (gkb->darea->window);

      gdk_draw_pixmap (gkb->darea->window,
		       gkb->darea->
		       style->fg_gc[GTK_WIDGET_STATE (gkb->darea)],
		       gkb->keymap->pix, 0, 0, 0, 0, gkb->w, gkb->h);

    }

  if (gkb->mode != GKB_FLAG)
    {
      gtk_label_set_text (GTK_LABEL (gkb->label1), gkb->keymap->label);
      gtk_label_set_text (GTK_LABEL (gkb->label2), gkb->keymap->label);
    }

 /* TODO: tooltip support
  applet_widget_set_tooltip (PANEL_APPLET (gkb->applet), gkb->keymap->name);
  */
}


#define GKB_SMALL_PANEL_SIZE 25	/* less than */
/**
 * count_sizes:
 * @gkb:
 * 
 * Calculates applet, flag, label sizes, mode mode
 */
static gint
gkb_count_sizes (GKB * gkb)
{
  gboolean small_panel = FALSE;
  gboolean label_in_vbox = TRUE;	/* If FALSE label is in hbox */
  gint panel_width = 0;		/* Zero means we have not determined it */
  gint panel_height = 0;
  gint applet_width = 0;
  gint applet_height = 0;
  gint flag_width = 0;
  gint flag_height = 0;
  gint label_height = 0;
  gint label_width = 0;

  gint size;

  size = panel_applet_get_size (PANEL_APPLET (gkb->applet));
  size = panel_applet_get_size (PANEL_APPLET (gkb->applet));

  /* Determine if this pannel requires different handling because it is very small */
  switch (gkb->orient)
    {
    case PANEL_APPLET_ORIENT_UP:
    case PANEL_APPLET_ORIENT_DOWN:
      panel_height = size;
      if (size < GKB_SMALL_PANEL_SIZE)
	small_panel = TRUE;
      break;
    case PANEL_APPLET_ORIENT_RIGHT:
    case PANEL_APPLET_ORIENT_LEFT:
      panel_width = size;
      if (size < GKB_SMALL_PANEL_SIZE)
	small_panel = TRUE;
      break;
    default:
      g_assert_not_reached ();
    }

  flag_height =
    (gint) panel_height / ((gkb->is_small && !small_panel) ? 2 : 1);
  flag_width = (gint) panel_width / ((gkb->is_small && !small_panel) ? 2 : 1);

  /* This are the cases in which we change the label to be side by side
   * v.s. beeing top-bottom */
  if (gkb->mode == GKB_FLAG_AND_LABEL)
    {
      if (gkb->orient == PANEL_APPLET_ORIENT_UP && small_panel)
	label_in_vbox = FALSE;
      if (gkb->orient == PANEL_APPLET_ORIENT_UP && !gkb->is_small)
	label_in_vbox = FALSE;
      if (gkb->orient == PANEL_APPLET_ORIENT_DOWN && small_panel)
	label_in_vbox = FALSE;
      if (gkb->orient == PANEL_APPLET_ORIENT_DOWN && !gkb->is_small)
	label_in_vbox = FALSE;
      if (gkb->orient == PANEL_APPLET_ORIENT_RIGHT && !small_panel)
	if (gkb->is_small)
	  label_in_vbox = FALSE;
      if (gkb->orient == PANEL_APPLET_ORIENT_LEFT && !small_panel)
	if (gkb->is_small)
	  label_in_vbox = FALSE;
    }

  /* Flag has either the with or the height set */
  if (flag_width == 0)
    flag_width = (gint) flag_height *1.5;
  else
    flag_height = (gint) flag_width / 1.5;

  label_width = flag_width;
  label_height = flag_height;

  if (gkb->mode != GKB_LABEL)
    {
      applet_width += flag_width;
      applet_height += flag_height;
    }

  if (gkb->mode != GKB_FLAG)
    {
      if (label_in_vbox)
	applet_height += label_height;
      else
	applet_width += label_width;
    }

  gtk_widget_set_usize (GTK_WIDGET (gkb->applet), applet_width,
			applet_height);

  if (flag_width > 0)
    {
#if 1
      gtk_widget_set_usize (GTK_WIDGET (gkb->darea), flag_width, flag_height);
#endif
      gtk_drawing_area_size (GTK_DRAWING_AREA (gkb->darea), flag_width,
			     flag_height);
    }

  gtk_widget_set_usize (GTK_WIDGET (gkb->label1), label_width, label_height);
  gtk_widget_set_usize (GTK_WIDGET (gkb->label2), label_width, label_height);

  gkb->w = flag_width;
  gkb->h = flag_height;

  return label_in_vbox;
}

/**
 * gkb_sized_render:
 * @void: 
 * 
 * When a size request is made, we need to redraw the pixbufs with the
 * new size. This function updates all the pixmaps with the new param.
 **/
void
gkb_sized_render (GKB * gkb)
{
  GkbKeymap *keymap;
  GList *list;
  gboolean label_in_vbox;

  label_in_vbox = gkb_count_sizes (gkb);

  /* Hide or show the flag */
  if (gkb->mode == GKB_LABEL)
    gtk_widget_hide (gkb->darea_frame);
  else
    gtk_widget_show_all (gkb->darea_frame);

  /* Hide or show the labels */
  switch (gkb->mode)
    {
    case GKB_LABEL:
    case GKB_FLAG_AND_LABEL:
      if (label_in_vbox)
	{
	  gtk_widget_hide (gkb->label_frame2);
	  gtk_widget_show_all (gkb->label_frame1);
	}
      else
	{
	  gtk_widget_hide (gkb->label_frame1);
	  gtk_widget_show_all (gkb->label_frame2);
	}
      break;
    case GKB_FLAG:
      gtk_widget_hide (gkb->label_frame1);
      gtk_widget_hide (gkb->label_frame2);
    }

  list = gkb->maps;
  for (; list != NULL; list = list->next)
    {
      gchar *name;
      gchar *real_name;
      keymap = (GkbKeymap *) list->data;
      name = g_strdup_printf ("gkb/%s", keymap->flag);
      real_name = gnome_unconditional_pixmap_file (name);
      if (g_file_exists (real_name))
	{
	  makepix (keymap, real_name, gkb->w - 4, gkb->h - 4);
	}
      else
	{
	  g_free (real_name);
	  real_name = gnome_unconditional_pixmap_file ("gkb/gkb-foot.png");
	  makepix (keymap, real_name, gkb->w - 4, gkb->h - 4);
	}
      g_free (name);
      g_free (real_name);
    }

#if 0
  gtk_widget_queue_resize (gkb->darea);
  gtk_widget_queue_resize (gkb->darea->parent);
  gtk_widget_queue_resize (gkb->darea->parent->parent);
#endif
}

/**
 * gkb_update:
 * @gkb: the GKB. global, so redundant
 * @set_command: do the command, or no
 * 
 * The size or orientation has been changed, update the widget
 **/
void
gkb_update (GKB * gkb, gboolean set_command)
{
  g_return_if_fail (gkb->maps);

  /* When the size is changed, we don't need to change the actual
   * keymap, so when the set_commadn is false, it means that we
   * have changed size. In other words, we can't change size &
   * keymap at the same time */
  if (set_command)
    gkb_system_set_keymap (gkb);
  else
    gkb_sized_render (gkb);

  gkb_draw (gkb);
}

static void
gkb_change_orient (GtkWidget * w, PanelAppletOrient new_orient, gpointer data)
{
  GKB *gkb = data;

  if (gkb->orient != new_orient)
    {
      gkb->orient = new_orient;
      gkb_update (gkb, FALSE);
    }
}

static void
gkb_change_pixel_size (GtkWidget * w, gint new_size, gpointer data)
{
  GKB *gkb = data;

#if 0
  if (gkb->size != new_size)
    {
      gkb->size = new_size;
#endif
      gkb_update (gkb, FALSE);
#if 0
    }
#endif
}

void
applet_save_session ()
{
  const gchar *text;
  GkbKeymap *actdata;
  GList *list = gkb->maps;
  gchar str[100];
  int i = 0;

  panel_applet_gconf_set_int (PANEL_APPLET (gkb->applet), "num", gkb->n, NULL);
  panel_applet_gconf_set_bool (PANEL_APPLET (gkb->applet), "small", gkb->is_small, NULL);
  panel_applet_gconf_set_string (PANEL_APPLET (gkb->applet), "key", gkb->key, NULL);
  text = gkb_util_get_text_from_mode (gkb->mode);
  panel_applet_gconf_set_string (PANEL_APPLET (gkb->applet), "mode", text, NULL);

  while (list)
    {
      actdata = list->data;
      if (actdata)
	{
	  g_snprintf (str, sizeof (str), "name_%d", i);
	  panel_applet_gconf_set_string (PANEL_APPLET (gkb->applet), str, actdata->name, NULL);
	  g_snprintf (str, sizeof (str), "country_%d", i);
	  panel_applet_gconf_set_string (PANEL_APPLET (gkb->applet), str, actdata->country, NULL);
	  g_snprintf (str, sizeof (str), "lang_%d", i);
	  panel_applet_gconf_set_string (PANEL_APPLET (gkb->applet), str, actdata->lang, NULL);
	  g_snprintf (str, sizeof (str), "label_%d", i);
	  panel_applet_gconf_set_string (PANEL_APPLET (gkb->applet), str, actdata->label, NULL);
	  g_snprintf (str, sizeof (str), "flag_%d", i);
	  panel_applet_gconf_set_string (PANEL_APPLET (gkb->applet), str, actdata->flag, NULL);
	  g_snprintf (str, sizeof (str), "command_%d", i);
	  panel_applet_gconf_set_string (PANEL_APPLET (gkb->applet), str, actdata->command, NULL);
	}

      list = list->next;

      i++;
    }
}

gchar *
gkb_load_pref(const gchar * key, const gchar * defaultv)
{
 gchar * value;

 value = panel_applet_gconf_get_string (PANEL_APPLET(gkb->applet), key, NULL);
 
 if (value == NULL) {
  value = g_strdup (defaultv);
 }
 return value;
}

GkbKeymap *
loadprop (int i)
{
  GkbKeymap *actdata;
  gchar *buf;

  actdata = g_new0 (GkbKeymap, 1);

  buf = g_strdup_printf (_("name_%d"),i);
  actdata->name = gkb_load_pref (buf, (i?"Hungarian 105 keys keyboard":"US 105 key keyboard"));

  buf = g_strdup_printf (_("label_%d"), i);
  actdata->label = gkb_load_pref (buf, (i?"hu":"us"));

  buf = g_strdup_printf (_("country_%d"), i);
  actdata->country = gkb_load_pref (buf, (i?"Hungary":"United States"));

  buf = g_strdup_printf (_("lang_%d"), i);
  actdata->lang = gkb_load_pref (buf, (i?"Hungarian":"English"));

  buf = g_strdup_printf (("flag_%d"), i);
  actdata->flag = gkb_load_pref (buf, (i?"hu.png":"us.png"));

  buf = g_strdup_printf (_("command_%d"), i);
  actdata->command = gkb_load_pref (buf, (i?"gkb_xmmap hu":"gkb_xmmap us"));

  g_free(buf);

  actdata->pix = NULL;
  gkb->orient = panel_applet_get_orient (PANEL_APPLET (gkb->applet));

  return actdata;
}

static void
load_properties (GKB * gkb)
{
  GkbKeymap *actdata;
  gchar *text;
  int i;

  gkb->maps = NULL;

  gkb->n = panel_applet_gconf_get_int (PANEL_APPLET(gkb->applet), "num", NULL);

  gkb->key = gkb_load_pref ("key", "Mod1-Shift_L");
  convert_string_to_keysym_state (gkb->key, &gkb->keysym, &gkb->state);

  gkb->is_small = panel_applet_gconf_get_bool (PANEL_APPLET(gkb->applet), "small", NULL);

  text = gkb_load_pref ("mode", "Flag and Label");
  gkb->mode = gkb_util_get_mode_from_text (text);
  g_free (text);

  if (gkb->n == 0)
    {
      actdata = loadprop (0);
      gkb->maps = g_list_append (gkb->maps, actdata);
      gkb->n++;

      actdata = loadprop (1);
      gkb->maps = g_list_append (gkb->maps, actdata);
      gkb->n++;
    }
  else
    for (i = 0; i < gkb->n; i++)
      {
	actdata = loadprop (i);
	gkb->maps = g_list_append (gkb->maps, actdata);
      }

  applet_save_session ();
}


static gint
gkb_button_press_event_cb (GtkWidget * widget, GdkEventButton * event)
{
   if (event->button != 1)	
    return FALSE;


  if (gkb->cur + 1 < gkb->n)
    gkb->keymap = g_list_nth_data (gkb->maps, ++gkb->cur);
  else
    {
      gkb->cur = 0;
      gkb->keymap = g_list_nth_data (gkb->maps, gkb->cur);
    }

  gkb_update (gkb, TRUE);
  return TRUE;
}

static int
gkb_expose (GtkWidget * darea, GdkEventExpose * event)
{
  gdk_draw_pixmap (gkb->darea->window,
		   gkb->darea->style->fg_gc[GTK_WIDGET_STATE (gkb->darea)],
		   gkb->keymap->pix,
		   event->area.x, event->area.y,
		   event->area.x, event->area.y,
		   event->area.width, event->area.height);

  return FALSE;
}

static void
create_gkb_widget ()
{
  GtkStyle *style;

  gtk_widget_push_visual (gdk_rgb_get_visual ());
  gtk_widget_push_colormap (gdk_rgb_get_cmap ());

  style = gtk_widget_get_style (gkb->applet);

  gkb->eventbox = gtk_event_box_new ();
  gtk_widget_show (gkb->eventbox);

  gkb->vbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (gkb->vbox);
  gkb->hbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (gkb->hbox);

  gtk_container_add (GTK_CONTAINER (gkb->eventbox), gkb->hbox);
  gtk_container_add (GTK_CONTAINER (gkb->hbox), gkb->vbox);

  gkb->darea = gtk_drawing_area_new ();

  gtk_drawing_area_size (GTK_DRAWING_AREA (gkb->darea), gkb->w, gkb->h);

  gtk_widget_set_events (gkb->darea,
			 gtk_widget_get_events (gkb->darea) |
			 GDK_BUTTON_PRESS_MASK);

  g_signal_connect (gkb->eventbox, "button_press_event",
		      G_CALLBACK (gkb_button_press_event_cb), NULL);
	      
#if 0
  g_signal_connect (gkb->eventbox, "expose_event",
		      G_CALLBACK (gkb_expose), NULL);
#else
  g_signal_connect (gkb->darea, "expose_event",
		      G_CALLBACK (gkb_expose), NULL);
#endif

  gtk_widget_show (gkb->darea);

  gkb->darea_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (gkb->darea_frame), GTK_SHADOW_IN);
  gtk_container_add (GTK_CONTAINER (gkb->darea_frame), gkb->darea);
  gtk_box_pack_start (GTK_BOX (gkb->vbox), gkb->darea_frame, TRUE, TRUE, 0);
#if 0
  g_print ("c\n");
  gtk_box_pack_start (GTK_BOX (gkb->hbox), gkb->vbox, TRUE, TRUE, 0);
  g_print ("d\n");
#endif

  gkb->label1 = gtk_label_new (_("GKB"));

  gkb->label_frame1 = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (gkb->label_frame1), GTK_SHADOW_IN);
  gtk_container_add (GTK_CONTAINER (gkb->label_frame1), gkb->label1);
  gtk_container_add (GTK_CONTAINER (gkb->vbox), gkb->label_frame1);

  gkb->label2 = gtk_label_new (_("GKB"));

  gkb->label_frame2 = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (gkb->label_frame2), GTK_SHADOW_IN);
  gtk_container_add (GTK_CONTAINER (gkb->label_frame2), gkb->label2);
  gtk_container_add (GTK_CONTAINER (gkb->hbox), gkb->label_frame2);

  gtk_widget_pop_colormap ();
  gtk_widget_pop_visual ();

  gkb_sized_render (gkb);

  gkb_update (gkb, TRUE);
}

static void
about_cb (BonoboUIComponent *uic,
          GKB     *gkb,
          const gchar     *verbname)
{
  static GtkWidget *about;
  GtkWidget *link;
  gchar *file;	
  GdkPixbuf *pixbuf;

  static const gchar *authors[] = {
		"Chema Celorio <chema@celorio.com>",
	        "Szabolcs Ban <shooby@gnome.hu>",
		NULL
	};
  static const gchar *docauthors[] = {
	        "Szabolcs Ban <shooby@gnome.hu>",
		"Emese Kovacs <emese@gnome.hu>",
		"David Mason <dcm@redhat.com>",
		"Alexander Kirillov <kirillov@math.sunysb.edu>",
		NULL
	};
	

  pixbuf = NULL;
  file = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_PIXMAP, "gkb-icon.png", TRUE, NULL);
  if (!file) {
         g_warning (G_STRLOC ": gkb-icon.png cannot be found");
         pixbuf = gdk_pixbuf_new_from_file (file, NULL);
  }

  about = gnome_about_new (_("The GNOME KeyBoard Switcher Applet"),
			   VERSION,
			   _("(C) 1998-2000 Free Software Foundation"),
 			   _("This applet switches between keyboard maps. "
			    "It uses setxkbmap, or xmodmap.\n"
			    "Mail me your flag, please (60x40 size), "
			    "I will put it into CVS.\n"
			    "So long, and thanks for all the fish.\n"
			    "Thanks for Balazs Nagy (Kevin) "
			    "<julian7@iksz.hu> for his help "
			    "and Emese Kovacs <emese@gnome.hu> for "
			    "her solidarity."),
			    authors,
			    docauthors,
			    NULL,
			    pixbuf);

  link = gnome_href_new ("http://projects.gnome.hu/gkb",
			 _("GKB Home Page (http://projects.gnome.hu/gkb)"));

  gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (about)->vbox), link, TRUE,
		      FALSE, 0);

  gtk_window_set_wmclass (GTK_WINDOW (about), "gkb", "Gkb");

  if (pixbuf) {
                gtk_window_set_icon (GTK_WINDOW (about), pixbuf);
                g_object_unref (pixbuf);
  }

  g_signal_connect (G_OBJECT(about), "destroy",
                          (GCallback)gtk_widget_destroyed, &about);

  gtk_widget_show_all (about);

  return;
}

static void
help_cb (BonoboUIComponent *uic,
         GKB	 *gkb,
         const gchar	 *verbname)
{
/* TODO:
  GnomeHelpMenuEntry help_entry = { "gkb_applet", "index.html" };

  gnome_help_display (NULL, &help_entry);
*/
}

static GdkFilterReturn
global_key_filter (GdkXEvent * gdk_xevent, GdkEvent * event)
{
  if (event->key.keyval == gkb->keysym && event->key.state == gkb->state)
    {
      if (gkb->cur + 1 < gkb->n)
	gkb->keymap = g_list_nth_data (gkb->maps, ++gkb->cur);
      else
	{
	  gkb->cur = 0;
	  gkb->keymap = g_list_nth_data (gkb->maps, gkb->cur);
	}

      gkb_update (gkb, TRUE);

      return GDK_FILTER_CONTINUE;
    }
  return GDK_FILTER_CONTINUE;
}

static GdkFilterReturn
event_filter (GdkXEvent * gdk_xevent, GdkEvent * event, gpointer data)
{
  XEvent *xevent;

  xevent = (XEvent *) gdk_xevent;

  if (xevent->type == KeyRelease)
    {
      return global_key_filter (gdk_xevent, event);
    }
  return GDK_FILTER_CONTINUE;
}

static gboolean
check_client (Display * display, Window xwindow, Atom state_atom)
{
  gboolean valid_client = TRUE;

  if (valid_client)
    {
      Atom dummy1;
      int format = 0;
      unsigned long dummy2, nitems = 0, *prop = NULL;

      XGetWindowProperty (display, xwindow, state_atom, 0, 1024, False,
			  state_atom, &dummy1, &format, &nitems, &dummy2,
			  (unsigned char **) &prop);
      if (prop)
	{
	  valid_client = format == 32 && nitems > 0 && (prop[0] == NormalState
							|| prop[0] ==
							IconicState);
	  XFree (prop);
	}
    }

  if (valid_client)
    {
      XWindowAttributes attributes = { 0, };

      XGetWindowAttributes (display, xwindow, &attributes);
      valid_client = (attributes.class == InputOutput &&
		      attributes.map_state == IsViewable);
    }

  if (valid_client)
    {
      XWMHints *hints = XGetWMHints (display, xwindow);

      valid_client &= hints && (hints->flags & InputHint) && hints->input;
      if (hints)
	XFree (hints);
    }

  return valid_client;
}


static Window
find_input_client (Display * display, Window xwindow, Atom state_atom)
{
  Window dummy1, dummy2, *children = NULL;
  unsigned int n_children = 0;
  guint i;

  if (check_client (display, xwindow, state_atom))
    return xwindow;

  if (!XQueryTree (display, xwindow, &dummy1, &dummy2, &children, &n_children)
      || !children)
    return None;

  for (i = 0; i < n_children; i++)
    {
      xwindow = find_input_client (display, children[i], state_atom);
      if (xwindow)
	break;
    }
  XFree (children);

  return xwindow;
}

static const char gkb_menu_xml [] =
	"<popup name=\"button3\">\n"
	"   <menuitem name=\"GKB Properties Item\" verb=\"GKBProperties\" _label=\"Properties ...\"\n"
	"             pixtype=\"stock\" pixname=\"gtk-properties\"/>\n"
	"   <menuitem name=\"GKB Help Item\" verb=\"GKBHelp\" _label=\"Help\"\n"
	"             pixtype=\"stock\" pixname=\"gtk-help\"/>\n"
	"   <menuitem name=\"GKB About Item\" verb=\"GKBAbout\" _label=\"About ...\"\n"
	"             pixtype=\"stock\" pixname=\"gnome-stock-about\"/>\n"
	"</popup>\n";

static const BonoboUIVerb gkb_menu_verbs [] = {
	BONOBO_UI_UNSAFE_VERB ("GKBProperties",   properties_dialog),
	BONOBO_UI_UNSAFE_VERB ("GKBHelp",	  help_cb),
	BONOBO_UI_UNSAFE_VERB ("GKBAbout",	  about_cb),
	BONOBO_UI_VERB_END
};

static void gkb_destroy (GtkWidget * widget, gpointer data) {
	g_free(gkb);
}

gboolean fill_gkb_applet(PanelApplet *applet)
{
  int keycode, modifiers;

  gkb = g_new0 (GKB, 1);

  gkb->n = 0;
  gkb->cur = 0;
  gkb->applet = GTK_WIDGET (applet);

  panel_applet_add_preferences (gkb->applet, "/schemas/apps/gkb-applet/prefs", NULL);

  gtk_widget_push_visual (gdk_rgb_get_visual ());
  gtk_widget_push_colormap (gdk_rgb_get_cmap ());

  bah_window = gtk_window_new (GTK_WINDOW_POPUP);

  gtk_widget_pop_visual ();
  gtk_widget_pop_colormap ();

  gtk_widget_set_uposition (bah_window, gdk_screen_width () + 1,
			    gdk_screen_height () + 1);
  gtk_widget_show (bah_window);

  load_properties (gkb);

  gkb->keymap = g_list_nth_data (gkb->maps, 0);

  create_gkb_widget ();

  gtk_widget_show (gkb->darea_frame);
  gtk_container_add (GTK_CONTAINER (gkb->applet), gkb->eventbox);

  keycode = XKeysymToKeycode(GDK_DISPLAY(), gkb->keysym);

  modifiers = gkb->state;
  
  XGrabKey (GDK_DISPLAY(), keycode, modifiers,
            GDK_ROOT_WINDOW(), True, GrabModeAsync, GrabModeAsync);
  XGrabKey (GDK_DISPLAY(), keycode, modifiers|NumLockMask,
            GDK_ROOT_WINDOW(), True, GrabModeAsync, GrabModeAsync);
  XGrabKey (GDK_DISPLAY(), keycode, modifiers|CapsLockMask,
            GDK_ROOT_WINDOW(), True, GrabModeAsync, GrabModeAsync);
  XGrabKey (GDK_DISPLAY(), keycode, modifiers|ScrollLockMask,
            GDK_ROOT_WINDOW(), True, GrabModeAsync, GrabModeAsync);
  XGrabKey (GDK_DISPLAY(), keycode, modifiers|NumLockMask|CapsLockMask,
            GDK_ROOT_WINDOW(), True, GrabModeAsync, GrabModeAsync);
  XGrabKey (GDK_DISPLAY(), keycode, modifiers|NumLockMask|ScrollLockMask,
            GDK_ROOT_WINDOW(), True, GrabModeAsync, GrabModeAsync);
  XGrabKey (GDK_DISPLAY(), keycode, modifiers|NumLockMask|CapsLockMask|ScrollLockMask,
            GDK_ROOT_WINDOW(), True, GrabModeAsync, GrabModeAsync);

  gdk_window_add_filter (GDK_ROOT_PARENT (), event_filter, NULL);

  g_signal_connect (G_OBJECT(gkb->applet),
  			  "destroy", 
  			  G_CALLBACK (gkb_destroy), 
  			  NULL);

  g_signal_connect (G_OBJECT (gkb->applet),
                          "change_orient",
                          G_CALLBACK (gkb_change_orient),
                          gkb);

  g_signal_connect (G_OBJECT (gkb->applet),
                          "change_size",
                          G_CALLBACK (gkb_change_pixel_size),
                          gkb);

  panel_applet_setup_menu (PANEL_APPLET (gkb->applet), 
  				gkb_menu_xml, 
  				gkb_menu_verbs, 
  				gkb);

  gtk_widget_show (GTK_WIDGET(gkb->applet));

  gkb_update(gkb,TRUE);

  return TRUE;
}

static gboolean
gkb_factory (PanelApplet *applet,
		const gchar *iid,
		gpointer     data)
{
 	gboolean retval = FALSE;

        if (!strcmp (iid, "OAFIID:GNOME_KeyboardApplet"))
		retval = fill_gkb_applet (applet);

	return retval;
}

PANEL_APPLET_BONOBO_FACTORY ("OAFIID:GNOME_KeyboardApplet_Factory",
                             "GKB_keyboard_layout_switcher",
                             "0",
                             gkb_factory,
                             NULL)
