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
#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#include <sys/stat.h>
#include "gkb.h"

GtkWidget *bah_window = NULL;

static void gkb_button_press_event_cb (GtkWidget * widget,
				       GdkEventButton * e);
static void about_cb (AppletWidget * widget);
static void help_cb (AppletWidget * widget);
static GdkFilterReturn
event_filter (GdkXEvent * gdk_xevent, GdkEvent * event, gpointer data);

static void
makepix (GkbKeymap *keymap, char *fname, int w, int h)
{
  GdkPixbuf *pix;
  int width, height;

  debug (FALSE, "");

  g_print ("Making pixmap for \"%s\" with file \"%s\"\n", keymap->label, fname);

  pix = gdk_pixbuf_new_from_file (fname);
  if (pix != NULL)
    {
      double affine[6];
      guchar *rgb;
      GdkGC *gc;

      g_assert (bah_window);
      g_assert (bah_window->window);

      width  = gdk_pixbuf_get_width (pix);
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

#if 0	
      gdk_pixbuf_unref (pix);
#endif	

      keymap->pix = gdk_pixmap_new (bah_window->window, w, h, -1);

      gc = gdk_gc_new (keymap->pix);
      gdk_draw_rgb_image (keymap->pix, gc,
			  0, 0, w, h, GDK_RGB_DITHER_NORMAL, rgb, w * 3);
      gdk_gc_destroy (gc);

      g_free (rgb);
    }
  else
    {
      if (keymap->pix) {
	g_print ("Unrefing ..\n");
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
  debug (FALSE, "");

  g_return_if_fail (gkb->darea != NULL);
  g_return_if_fail (gkb->keymap != NULL);
  g_return_if_fail (gkb->keymap->pix != NULL);
  g_return_if_fail (GTK_WIDGET_REALIZED (gkb->darea));

  gdk_draw_pixmap (gkb->darea->window,
		   gkb->darea->style->fg_gc [GTK_WIDGET_STATE (gkb->darea)],
		   gkb->keymap->pix, 0, 0, 0, 0, gkb->w, gkb->h);

  applet_widget_set_tooltip (APPLET_WIDGET (gkb->applet), gkb->keymap->name);
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
  gint size;

  debug (FALSE, "");

  if (gkb->is_small)
    size = applet_widget_get_panel_pixel_size (APPLET_WIDGET (gkb->applet)) / 2;
  else
    size = applet_widget_get_panel_pixel_size (APPLET_WIDGET (gkb->applet));

  if (gkb->orient == ORIENT_UP || gkb->orient == ORIENT_DOWN)
    {
      gkb->h = size;
      gkb->w = (int) gkb->h * 1.5;
    }
  else
    {
      gkb->w = size;
      gkb->h = (int) gkb->w / 1.5;
    }


  gtk_widget_queue_resize (gkb->darea);
  gtk_widget_queue_resize (gkb->darea->parent);
  gtk_widget_queue_resize (gkb->darea->parent->parent);

  list = gkb->maps;
  for (; list != NULL; list = list->next) {
    gchar *name;
    gchar *real_name;
    keymap = (GkbKeymap *)list->data;
    name = g_strdup_printf ("gkb/%s", keymap->flag);
    real_name = gnome_unconditional_pixmap_file (name);
    if (g_file_exists (real_name)) {
      makepix (keymap, real_name, gkb->w - 4, gkb->h - 4);
    } else {
      g_free (real_name);
      real_name = gnome_unconditional_pixmap_file ("gkb/gkb-foot.png");
      makepix (keymap, real_name, gkb->w - 4, gkb->h - 4);
    }
    g_free (name);
    g_free (real_name);
  }
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
  debug (FALSE, "");

  g_return_if_fail (gkb->maps);

  /* When the size is changed, we don't need to change the actual
   * keymap, so when the set_commadn is false, it means that we
   * have changed size. In other words, we can't change size &
   * keymap at the same time */
  if (gkb->is_small)
   {
    if (gkb->appearance == GKB_FLAG)
     {
      gtk_widget_show_all (gkb->darea_frame);
      gtk_widget_hide (gkb->label_frame);
      if (!set_command)
       gkb_sized_render (gkb);  
      gtk_widget_set_usize (GTK_WIDGET (gkb->applet), gkb->w, gkb->h);
      gtk_drawing_area_size (GTK_DRAWING_AREA (gkb->darea), gkb->w, gkb->h);
      gtk_widget_set_usize (GTK_WIDGET (gkb->darea), gkb->w, gkb->h);
      gkb_draw (gkb);
     }
    else if (gkb->appearance == GKB_LABEL)
     {
      gtk_widget_set_usize (GTK_WIDGET (gkb->applet), gkb->w, gkb->h);
      gtk_widget_set_usize (GTK_WIDGET (gkb->label), gkb->w, gkb->h);
      gtk_widget_show_all (gkb->label_frame);
      gtk_widget_hide (gkb->darea_frame);
      gtk_label_set_text(GTK_LABEL(gkb->label),g_strdup(gkb->keymap->label));
     } 
   else 
    {
     gtk_widget_show_all (gkb->label_frame);
     gtk_widget_show_all (gkb->darea_frame);
     gtk_widget_set_usize (GTK_WIDGET (gkb->label), gkb->w, gkb->h);
     gtk_widget_set_usize (GTK_WIDGET (gkb->applet), gkb->w, gkb->h * 2);
     gtk_drawing_area_size (GTK_DRAWING_AREA (gkb->darea), gkb->w, gkb->h);
     gtk_widget_set_usize (GTK_WIDGET (gkb->darea), gkb->w, gkb->h);
     gtk_label_set_text(GTK_LABEL(gkb->label),g_strdup(gkb->keymap->label));
     if (!set_command)
      gkb_sized_render (gkb); 
     gkb_draw (gkb);
    }
   }
  else
   {
     gtk_widget_show_all (gkb->darea_frame);
     gtk_widget_hide (gkb->label_frame);
     if (!set_command)
      gkb_sized_render (gkb);  
     gtk_widget_set_usize (GTK_WIDGET (gkb->applet), gkb->w, gkb->h);
     gtk_drawing_area_size (GTK_DRAWING_AREA (gkb->darea), gkb->w, gkb->h);
     gtk_widget_set_usize (GTK_WIDGET (gkb->darea), gkb->w, gkb->h);
     gkb_draw (gkb);
   }
 
  if (set_command)
     gkb_system_set_keymap (gkb);

}

static void
gkb_change_orient (GtkWidget * w, PanelOrientType new_orient, gpointer data)
{
  GKB *gkb = data;

  debug (FALSE, "");

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

  debug (FALSE, "");

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

#if 0
static void
gkb_switch_normal (AppletWidget * applet, gpointer gkbx)
{
  GKB *gkb = (GKB *) gkbx;

  debug (FALSE, "");

  if (gkb->is_small)
    {
      gkb->is_small = FALSE;
      gkb->size =
	applet_widget_get_panel_pixel_size (APPLET_WIDGET (gkb->applet));
      gkb_update (gkb, FALSE);
    }
}

static void
gkb_switch_small (AppletWidget * applet, gpointer gkbx)
{
  GKB *gkb = (GKB *) gkbx;

  debug (FALSE, "");

  if (!gkb->is_small)
    {
      gkb->is_small = TRUE;
      gkb_update (gkb, FALSE);
    }
}
#endif


static gboolean
applet_save_session (GtkWidget * w,
		     const char *privcfgpath, const char *globcfgpath)
{
  const gchar *text;
  GkbKeymap *actdata;
  GList *list = gkb->maps;
  gchar str[100];
  int i = 0;

  debug (FALSE, "");

  gnome_config_push_prefix (privcfgpath);
  gnome_config_set_int ("gkb/num", gkb->n);
  gnome_config_set_int ("gkb/small", gkb->is_small);
  gnome_config_set_string ("gkb/key", gkb->key);
  text = gkb_util_get_text_from_appearance (gkb->appearance);
  gnome_config_set_string ("gkb/appearance", text);

  while (list)
    {
      actdata = list->data;
      if (actdata)
	{
	  g_snprintf (str, sizeof (str), "keymap_%d/name", i);
	  gnome_config_set_string (str, actdata->name);
	  g_snprintf (str, sizeof (str), "keymap_%d/country", i);
	  gnome_config_set_string (str, actdata->country);
	  g_snprintf (str, sizeof (str), "keymap_%d/lang", i);
	  gnome_config_set_string (str, actdata->lang);
	  g_snprintf (str, sizeof (str), "keymap_%d/label", i);
	  gnome_config_set_string (str, actdata->label);
	  g_snprintf (str, sizeof (str), "keymap_%d/flag", i);
	  gnome_config_set_string (str, actdata->flag);
	  g_snprintf (str, sizeof (str), "keymap_%d/command", i);
	  gnome_config_set_string (str, actdata->command);
	}

      list = list->next;

      i++;
    }

  gnome_config_pop_prefix ();
  gnome_config_sync ();
  gnome_config_drop_all ();

  return FALSE;
}

GkbKeymap *
loadprop (int i)
{
  struct stat tempbuf;
  GkbKeymap *actdata;
  gint size;
  char buf[256];
  char *pixmapname;

  debug (FALSE, "");

  actdata = g_new0 (GkbKeymap, 1);

  if (i == 0)
    {
      g_snprintf (buf, 256, _("keymap_%d/name=Hungarian 105 keys latin2"), i);
      actdata->name = gnome_config_get_string (buf);

      g_snprintf (buf, 256, _("keymap_%d/label=hu"), i);
      actdata->label = gnome_config_get_string (buf);

      g_snprintf (buf, 256, _("keymap_%d/country=Hungary"), i);
      actdata->country = gnome_config_get_string (buf);

      g_snprintf (buf, 256, _("keymap_%d/lang=Hungarian"), i);
      actdata->lang = gnome_config_get_string (buf);

      g_snprintf (buf, 256, _("keymap_%d/image=hu.png"), i);
      actdata->flag = gnome_config_get_string (buf);

      g_snprintf (buf, 256, _("keymap_%d/command=gkb_xmmap hu"), i);
      actdata->command = gnome_config_get_string (buf);
    }
  else
    {
      g_snprintf (buf, 256,
		  _
		  ("keymap_%d/name=US 105 key keyboard (with windows keys)"),
		  i);
      actdata->name = gnome_config_get_string (buf);

      g_snprintf (buf, 256, _("keymap_%d/label=us"), i);
      actdata->label = gnome_config_get_string (buf);

      g_snprintf (buf, 256, _("keymap_%d/country=United States"), i);
      actdata->country = gnome_config_get_string (buf);

      g_snprintf (buf, 256, _("keymap_%d/lang=English"), i);
      actdata->lang = gnome_config_get_string (buf);

      g_snprintf (buf, 256, _("keymap_%d/flag=us.png"), i);
      actdata->flag = gnome_config_get_string (buf);

      g_snprintf (buf, 256, _("keymap_%d/command=gkb_xmmap us"), i);
      actdata->command = gnome_config_get_string (buf);
    }

  actdata->pix = NULL;
  gkb->orient = applet_widget_get_panel_orient (APPLET_WIDGET (gkb->applet));

  if (gkb->is_small)
    size = applet_widget_get_panel_pixel_size (APPLET_WIDGET (gkb->applet)) / 2;
  else
    size = applet_widget_get_panel_pixel_size (APPLET_WIDGET (gkb->applet));

  if (gkb->orient == ORIENT_UP || gkb->orient == ORIENT_DOWN)
    {
      gkb->h = size;
      gkb->w = gkb->h * 1.5;
    }
  else
    {
      gkb->w = size;
      gkb->h = (int) gkb->w / 1.5;
    }

  sprintf (buf, "gkb/%s", actdata->flag);
  pixmapname = gnome_unconditional_pixmap_file (buf);
  if (stat (pixmapname, &tempbuf))
    {
      pixmapname = gnome_unconditional_pixmap_file ("gkb/gkb-foot.png");
      makepix (actdata, pixmapname, gkb->w - 4, gkb->h - 4);
    }
  else
    {
      makepix (actdata, pixmapname, gkb->w - 4, gkb->h - 4);
    }
  g_free (pixmapname);

  return actdata;

}

static void
load_properties ()
{
  GkbKeymap *actdata;
  gchar *text;
  int i;

  debug (FALSE, "");

  gkb->maps = NULL;

  gnome_config_push_prefix (APPLET_WIDGET (gkb->applet)->privcfgpath);

  gkb->n = gnome_config_get_int ("gkb/num=0");

  gkb->key = gnome_config_get_string ("gkb/key=Mod1-Shift_L");
  convert_string_to_keysym_state (gkb->key, &gkb->keysym, &gkb->state);

  gkb->is_small = gnome_config_get_int ("gkb/small=0");
  text = gnome_config_get_string ("gkb/appearance=Flag");
  gkb->appearance = gkb_util_get_appearance_from_text (text);
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

  gnome_config_pop_prefix ();

  /* tell the panel to save our configuration data */
  applet_widget_sync_config (APPLET_WIDGET (gkb->applet));
}


static void
gkb_button_press_event_cb (GtkWidget * widget, GdkEventButton * event)
{
  debug (FALSE, "");

  if (event->button != 1)	/* Ignore mouse buttons 2 and 3 */
    return;

  if (gkb->cur + 1 < gkb->n)
    gkb->keymap = g_list_nth_data (gkb->maps, ++gkb->cur);
  else
    {
      gkb->cur = 0;
      gkb->keymap = g_list_nth_data (gkb->maps, gkb->cur);
    }

  gkb_update (gkb, TRUE);
}

static int
gkb_expose (GtkWidget * darea, GdkEventExpose * event)
{
  GkbKeymap *d = gkb->keymap;

  debug (FALSE, "");

  gdk_draw_pixmap (gkb->darea->window,
		   gkb->darea->style->fg_gc[GTK_WIDGET_STATE (gkb->darea)],
		   d->pix, event->area.x, event->area.y, event->area.x,
		   event->area.y, event->area.width, event->area.height);

  return FALSE;
}

static void
create_gkb_widget ()
{
  GtkStyle *style;

  debug (FALSE, "");

  gtk_widget_push_visual (gdk_rgb_get_visual ());
  gtk_widget_push_colormap (gdk_rgb_get_cmap ());

  style = gtk_widget_get_style (gkb->applet);

  gkb->eventbox = gtk_event_box_new ();
  gtk_widget_show (gkb->eventbox);

  gkb->box = gtk_vbox_new (FALSE, 0);
  gtk_widget_show(gkb->box);

  gtk_container_add (GTK_CONTAINER (gkb->eventbox), gkb->box);

  gkb->darea = gtk_drawing_area_new ();

  gtk_drawing_area_size (GTK_DRAWING_AREA (gkb->darea), gkb->w, gkb->h);

  gtk_widget_set_events (gkb->darea,
			 gtk_widget_get_events (gkb->darea) |
			 GDK_BUTTON_PRESS_MASK);

  gtk_signal_connect (GTK_OBJECT (gkb->eventbox), "button_press_event",
		      GTK_SIGNAL_FUNC (gkb_button_press_event_cb), NULL);
  gtk_signal_connect (GTK_OBJECT (gkb->eventbox), "expose_event",
		      GTK_SIGNAL_FUNC (gkb_expose), NULL);

  gtk_widget_show (gkb->darea);

  gkb->darea_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (gkb->darea_frame), GTK_SHADOW_IN);
  gtk_container_add (GTK_CONTAINER (gkb->darea_frame), gkb->darea);

  gtk_box_pack_start (GTK_BOX (gkb->box), gkb->darea_frame, TRUE, TRUE, 0);

  gkb->label = gtk_label_new (_("GKB"));

  gkb->label_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (gkb->label_frame), GTK_SHADOW_IN);
  gtk_container_add (GTK_CONTAINER (gkb->label_frame), gkb->label);
  gtk_container_add (GTK_CONTAINER (gkb->box), gkb->label_frame);

  gtk_widget_pop_colormap ();
  gtk_widget_pop_visual ();

}

static void
about_cb (AppletWidget * widget)
{
  static GtkWidget *about;
  const char *authors[2];
  GtkWidget *link;

  debug (FALSE, "");

  if (about)
    {
      gtk_widget_show (about);
      gdk_window_raise (about->window);
      return;
    }

  authors[0] = "Szabolcs BAN <shooby@gnome.hu>";
  authors[1] = NULL;

  about = gnome_about_new (_("The GNOME KeyBoard Switcher Applet"),
			   VERSION,
			   _
			   ("(C) 1998-2000 LSC - Linux Support Center"),
			   authors,
			   _
			   ("This applet switches between keyboard maps. "
			    "It uses setxkbmap, or xmodmap.\n"
			    "Mail me your flag, please (60x40 size), "
			    "I will put it to CVS.\n"
			    "So long, and thanks for all the fish.\n"
			    "Thanks for Balazs Nagy (Kevin) "
			    "<julian7@kva.hu> for his help "
			    "and Emese Kovacs <emese@eik.bme.hu> for "
			    "her solidarity."), "gkb-icon.png");

  link = gnome_href_new ("http://projects.gnome.hu/gkb",
			 _("GKB Home Page (http://projects.gnome.hu/gkb)"));
  gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (about)->vbox), link, TRUE,
		      FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (about), "destroy",
		      GTK_SIGNAL_FUNC (gtk_widget_destroyed), &about);
  gtk_widget_show_all (about);

  return;
}

static void
help_cb (AppletWidget * applet)
{
  GnomeHelpMenuEntry help_entry = { "gkb_applet", "index.html" };

  debug (FALSE, "");

  gnome_help_display (NULL, &help_entry);
}

static GdkFilterReturn
global_key_filter (GdkXEvent * gdk_xevent, GdkEvent * event)
{
  debug (FALSE, "");

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

  debug (FALSE, "");

  xevent = (XEvent *) gdk_xevent;

  if (xevent->type == KeyRelease)
    {
      return global_key_filter (gdk_xevent, event);
    }
  return GDK_FILTER_CONTINUE;
}

/**
 * gkb_activator_connect_signals:
 * @gkb: 
 * 
 * Connect the signaals for gkb->applet
 **/
static void
gkb_activator_connect_signals (GKB * gkb)
{
  debug (FALSE, "");

  g_return_if_fail (gkb != NULL);
  g_return_if_fail (GTK_IS_WIDGET (gkb->applet));

  gtk_signal_connect (GTK_OBJECT (gkb->applet), "save_session",
		      GTK_SIGNAL_FUNC (applet_save_session), NULL);

  gtk_signal_connect (GTK_OBJECT (gkb->applet), "change_orient",
		      GTK_SIGNAL_FUNC (gkb_change_orient), gkb);

  gtk_signal_connect (GTK_OBJECT (gkb->applet), "change_pixel_size",
		      GTK_SIGNAL_FUNC (gkb_change_pixel_size), gkb);
}

/**
 * gkb_applet_widget_register_callbacks:
 * @gkb: 
 * 
 * Register the callbacks for the applet widget
 **/
static void
gkb_activator_register_callbacks (GKB * gkb)
{
  g_return_if_fail (gkb != NULL);
  g_return_if_fail (GTK_IS_WIDGET (gkb->applet));

  debug (FALSE, "");

  applet_widget_register_stock_callback (APPLET_WIDGET (gkb->applet),
					 "properties",
					 GNOME_STOCK_MENU_PROP,
					 _("Properties..."),
					 GTK_SIGNAL_FUNC
					 (properties_dialog), NULL);
  applet_widget_register_stock_callback (APPLET_WIDGET (gkb->applet),
					 "help",
					 GNOME_STOCK_MENU_BOOK_OPEN,
					 _("Help"),
					 GTK_SIGNAL_FUNC (help_cb), NULL);
  applet_widget_register_stock_callback (APPLET_WIDGET (gkb->applet),
					 "about",
					 GNOME_STOCK_MENU_ABOUT,
					 _("About..."),
					 GTK_SIGNAL_FUNC (about_cb), NULL);

}

static CORBA_Object
gkb_activator (CORBA_Object poa_in,
	       const char *goad_id,
	       const char **params,
	       gpointer * impl_ptr, CORBA_Environment * ev)
{
  PortableServer_POA poa = (PortableServer_POA) poa_in;

  debug (FALSE, "");

  gkb = g_new0 (GKB, 1);

  gkb->applet = applet_widget_new (goad_id);

  create_gkb_widget ();

  gkb->n = 0;
  gkb->cur = 0;

  gtk_widget_push_visual (gdk_rgb_get_visual ());
  gtk_widget_push_colormap (gdk_rgb_get_cmap ());

  bah_window = gtk_window_new (GTK_WINDOW_POPUP);

  gtk_widget_pop_visual ();
  gtk_widget_pop_colormap ();

  gtk_widget_set_uposition (bah_window, gdk_screen_width () + 1,
			    gdk_screen_height () + 1);
  gtk_widget_show_now (bah_window);

  load_properties ();
  gkb_activator_connect_signals (gkb);

  gkb->keymap = g_list_nth_data (gkb->maps, 0);

  gkb_activator_register_callbacks (gkb);

  gtk_widget_show (gkb->darea_frame);

  applet_widget_add (APPLET_WIDGET (gkb->applet), gkb->eventbox);
  gtk_widget_show (gkb->applet);

  gdk_window_add_filter (GDK_ROOT_PARENT (), event_filter, NULL);

  gkb_sized_render (gkb);
  gkb_update (gkb, TRUE);
  
  return applet_widget_corba_activate (gkb->applet, poa, goad_id,
				       params, impl_ptr, ev);
}

static void
gkb_deactivator (CORBA_Object poa_in,
		 const char *goad_id,
		 gpointer impl_ptr, CORBA_Environment * ev)
{
  PortableServer_POA poa = (PortableServer_POA) poa_in;
/*  gdk_window_remove_filter(GDK_ROOT_PARENT(), event_filter, NULL);
*/
  debug (FALSE, "");

  applet_widget_corba_deactivate (poa, goad_id, impl_ptr, ev);
}

#if 0 || defined(SHLIB_APPLETS)
static const char *repo_id[] = { "IDL:GNOME/Applet:1.0", NULL };
static GnomePluginObject applets_list[] = {
  {repo_id, "gkb_applet", NULL, "GNOME Keyboard switcher",
   &gkb_activator, &gkb_deactivator},
  {NULL}
};

GnomePlugin GNOME_Plugin_info = {
  applets_list, NULL
};
#else

int
main (int argc, char *argv[])
{
  gpointer gkb_impl;

  /* Initialize the i18n stuff */

  debug (FALSE, "");

  bindtextdomain (PACKAGE, GNOMELOCALEDIR);
  textdomain (PACKAGE);

  applet_widget_init ("gkb_applet", VERSION, argc, argv, NULL, 0, NULL);

  APPLET_ACTIVATE (gkb_activator, "gkb_applet", &gkb_impl);

  applet_widget_gtk_main ();

  APPLET_DEACTIVATE (gkb_deactivator, "gkb_applet", gkb_impl);

  return 0;
}
#endif
