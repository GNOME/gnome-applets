/* File: gkb.c
 * Purpose: GNOME Keyboard switcher
 *
 * Copyright (C) 1999 Free Software Foundation
 * Author: Szabolcs BAN <shooby@gnome.hu>, 1998-2000
 *
 * Thanks for aid of Balazs Nagy <julian7@kva.hu>,
 * Charles Levert <charles@comm.polymtl.ca>,
 * George Lebl <jirka@5z.com> and solidarity
 * Emese Kovacs <emese@eik.bme.hu>.
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

#include "gkb.h"

GtkWidget *bah_window = NULL;

static void gkb_button_press_event_cb (GtkWidget * widget, GdkEventButton * e,
				       GKB * gkb);
static void about_cb (AppletWidget * widget, gpointer gkbx);
static void help_cb (AppletWidget * widget, gpointer data);

static void
makepix (Prop * actdata, char *fname, int w, int h)
{
  GdkPixbuf *pix;
  int width,height;

  pix = gdk_pixbuf_new_from_file (fname);
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

      art_rgb_rgba_affine    (rgb,
			     0, 0, w, h, w * 3,
			     gdk_pixbuf_get_pixels(pix), 
			     width, height,
			     gdk_pixbuf_get_rowstride (pix),
			     affine,
			     ART_FILTER_NEAREST, NULL);

      gdk_pixbuf_unref (pix);

      actdata->pix = gdk_pixmap_new (bah_window->window, w, h, -1);

      gc = gdk_gc_new (actdata->pix);

      gdk_draw_rgb_image (actdata->pix, gc,
			  0, 0, w, h, GDK_RGB_DITHER_NORMAL, rgb, w * 3);

      g_free (rgb);
    }
  else
    {
      if (actdata->pix)
	gdk_pixmap_unref (actdata->pix);
      actdata->pix = gdk_pixmap_new (bah_window->window, w, h, -1);
    }
}

void
sized_render (GKB * gkb)
{
  Prop *actdata;
  int i = 0;

  gtk_drawing_area_size (GTK_DRAWING_AREA (gkb->darea), gkb->w, gkb->h);
  gtk_widget_set_usize (GTK_WIDGET (gkb->darea), gkb->w, gkb->h);
  gtk_widget_set_usize (GTK_WIDGET (gkb->frame), gkb->w, gkb->h);
  gtk_widget_set_usize (GTK_WIDGET (gkb->applet), gkb->w, gkb->h);

  if (gkb->orient == ORIENT_UP)
    {
      gkb->h = gkb->size;
      gkb->w = gkb->h * 1.5;
    }
  else
    {
      gkb->w = gkb->size;
      gkb->h = (int) gkb->w / 1.5;
    }

  gtk_widget_queue_resize (gkb->darea);
  gtk_widget_queue_resize (gkb->darea->parent);
  gtk_widget_queue_resize (gkb->darea->parent->parent);

  while ((actdata = g_list_nth_data (gkb->maps, i++)) != NULL)
    {
      makepix (actdata, actdata->iconpath, gkb->w - 4, gkb->h - 4);
    }
}

static void
gkb_change_orient (GtkWidget * w, PanelOrientType o, gpointer data)
{
  GKB *gkb = data;

  gkb->orient = o;
  sized_render (gkb);
  gkb_draw (gkb);
}

static void
gkb_change_pixel_size (GtkWidget * w, int size, gpointer data)
{
  GKB *gkb = data;

  gkb->size = size;
  sized_render (gkb);
  gkb_draw (gkb);
}

Prop *
loadprop (GKB * gkb, int i)
{
  char buf[256];

  Prop *actdata = g_new (Prop, 1);

  g_snprintf (buf, 256, "map_%d/name=Us", i);
  actdata->name = gnome_config_get_string (buf);

  g_snprintf (buf, 256, "map_%d/image=%s", i,
	      gnome_unconditional_pixmap_file ("gkb/us.png"));
  actdata->iconpath = gnome_config_get_string (buf);

  g_snprintf (buf, 256, "map_%d/command=setxkbmap us", i);
  actdata->command = gnome_config_get_string (buf);

  actdata->pix = NULL;
  gkb->orient = applet_widget_get_panel_orient (APPLET_WIDGET (gkb->applet));
  gkb->size =
    applet_widget_get_panel_pixel_size (APPLET_WIDGET (gkb->applet));

  if (gkb->orient == ORIENT_UP)
    {
      gkb->h = gkb->size;
      gkb->w = gkb->h * 1.5;
    }
  else
    {
      gkb->w = gkb->size;
      gkb->h = (int) gkb->w / 1.5;
    }

  makepix (actdata, actdata->iconpath, gkb->w, gkb->h);


  return actdata;

}

static void
load_properties (GKB * gkb)
{
  char buf[256];
  int i;

  Prop *actdata;

  gkb->maps = g_list_alloc();

  gnome_config_push_prefix (APPLET_WIDGET (gkb->applet)->privcfgpath);

  g_snprintf (buf, 256, "gkb/num=0");
  gkb->n = gnome_config_get_int (buf);

  if (gkb->n == 0)
    {
      actdata = loadprop (gkb, 0);
      gkb->maps->data = actdata;
      gkb->n++;

      actdata = loadprop (gkb, 1);
      gkb->maps = g_list_append (gkb->maps, actdata);
      gkb->n++;
    }
  else
    for (i = 0; i < gkb->n; i++)
      {
	actdata = loadprop (gkb, i);
        if (i == 0)
          gkb->maps->data = actdata;
         else
	  gkb->maps = g_list_append (gkb->maps, actdata);
      }

  gnome_config_pop_prefix ();

  applet_save_session (gkb->propbox,
		       APPLET_WIDGET (gkb->applet)->privcfgpath,
		       APPLET_WIDGET (gkb->applet)->globcfgpath, gkb);
}

void
gkb_draw (GKB * gkb)
{

  g_return_if_fail (gkb->darea != NULL);
  g_return_if_fail (GTK_WIDGET_REALIZED (gkb->darea));

  gdk_draw_pixmap (gkb->darea->window,
		   gkb->darea->style->fg_gc[GTK_WIDGET_STATE (gkb->darea)],
		   gkb->dact->pix, 0, 0, 0, 0, gkb->w, gkb->h);
}

static void
gkb_button_press_event_cb (GtkWidget * widget,
			   GdkEventButton * event, GKB * gkb)
{
  if (event->button != 1)	/* Ignore mouse buttons 2 and 3 */
    return;

  if (gkb->cur + 1 < gkb->n)
    gkb->dact = g_list_nth_data (gkb->maps, ++gkb->cur);
  else
    {
      gkb->cur = 0;
      gkb->dact = g_list_nth_data (gkb->maps, gkb->cur);
    }

  gkb_draw (gkb);
  system (gkb->dact->command);
}

static int
gkb_expose (GtkWidget * darea, GdkEventExpose * event, GKB * gkb)
{
  Prop *d = gkb->dact;

  gdk_draw_pixmap (gkb->darea->window,
		   gkb->darea->style->fg_gc[GTK_WIDGET_STATE (gkb->darea)],
		   d->pix,
		   event->area.x, event->area.y,
		   event->area.x, event->area.y,
		   event->area.width, event->area.height);

  return FALSE;
}

static void
create_gkb_widget (GKB * gkb)
{
  GtkStyle *style;

  gtk_widget_push_visual (gdk_imlib_get_visual ());
  gtk_widget_push_colormap (gdk_imlib_get_colormap ());
  style = gtk_widget_get_style (gkb->applet);

  gkb->darea = gtk_drawing_area_new ();
  gtk_drawing_area_size (GTK_DRAWING_AREA (gkb->darea), gkb->w, gkb->h);
  gtk_widget_set_events (gkb->darea,
			 gtk_widget_get_events (gkb->darea) |
			 GDK_BUTTON_PRESS_MASK);

  gtk_widget_show (gkb->darea);


  gtk_signal_connect (GTK_OBJECT (gkb->darea), "button_press_event",
		      GTK_SIGNAL_FUNC (gkb_button_press_event_cb), gkb);
  gtk_signal_connect (GTK_OBJECT (gkb->darea), "expose_event",
		      GTK_SIGNAL_FUNC (gkb_expose), gkb);

  gkb->dact = g_list_nth_data (gkb->maps, 0);

  gkb->frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (gkb->frame), GTK_SHADOW_IN);
  gtk_container_add (GTK_CONTAINER (gkb->frame), gkb->darea);
  gtk_widget_pop_colormap ();
  gtk_widget_pop_visual ();
}

static void
about_cb (AppletWidget * widget, gpointer gkbx)
{
  static GtkWidget *about;
  const char *authors[2];
  GtkWidget *link;

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
			   _("(C) 1998-2000 LSC - Linux Support Center"),
			   authors,
			   _("This applet switches between keyboard maps. "
			     "It uses setxkbmap, or xmodmap.\n"
			     "Mail me your flag, please (60x40 size), "
			     "I will put it to CVS.\n"
			     "So long, and thanks for all the fish.\n"
			     "Thanks for Balazs Nagy (Kevin) "
			     "<julian7@kva.hu> for his help "
			     "and Emese Kovacs <emese@eik.bme.hu> for "
			     "her solidarity."), "gkb.png");

  link = gnome_href_new ("http://projects.gnome.hu/gkb",
			 _("GKB Home Page (http://projects.gnome.hu/gkb)"));
  gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (about)->vbox),
		      link, TRUE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (about), "destroy",
		      GTK_SIGNAL_FUNC (gtk_widget_destroyed), &about);
  gtk_widget_show_all (about);

  return;
}

static void
help_cb (AppletWidget * applet, gpointer data)
{
  GnomeHelpMenuEntry help_entry = { "gkb_applet", "index.html" };

  gnome_help_display (NULL, &help_entry);
}

int
applet_save_session (GtkWidget * w,
		     const char *privcfgpath,
		     const char *globcfgpath, GKB * gkb)
{
  Prop *actdata;
  int i = 0;
  char str[100];
  GList * list = gkb->maps;

  gnome_config_push_prefix (privcfgpath);
  sprintf (str, "gkb/num");
  gnome_config_set_int (str, gkb->n);

  while (list)
    {
      if(list->data) actdata = list->data;
      sprintf (str, "map_%d/name", i);
      gnome_config_set_string (str, actdata->name);
      sprintf (str, "map_%d/image", i);
      gnome_config_set_string (str, actdata->iconpath);
      sprintf (str, "map_%d/command", i);
      gnome_config_set_string (str, actdata->command);

      list=list->next;

      i++;
    }

  gnome_config_pop_prefix ();
  gnome_config_sync ();
  gnome_config_drop_all ();

  return FALSE;
}

static CORBA_Object
gkb_activator (PortableServer_POA poa,
	       const char *goad_id,
	       const char **params,
	       gpointer * impl_ptr, CORBA_Environment * ev)
{
  GKB *gkb;

  gkb = g_new0 (GKB, 1);

  gkb->applet = applet_widget_new (goad_id);

  create_gkb_widget (gkb);

  gtk_widget_show (gkb->frame);
  applet_widget_add (APPLET_WIDGET (gkb->applet), gkb->frame);
  gtk_widget_show (gkb->applet);

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

  load_properties (gkb);

  gtk_signal_connect (GTK_OBJECT (gkb->applet), "save_session",
		      GTK_SIGNAL_FUNC (applet_save_session), gkb);

  gtk_signal_connect (GTK_OBJECT (gkb->applet), "change_orient",
		      GTK_SIGNAL_FUNC (gkb_change_orient), gkb);

  gtk_signal_connect (GTK_OBJECT (gkb->applet), "change_pixel_size",
		      GTK_SIGNAL_FUNC (gkb_change_pixel_size), gkb);

  gkb->dact = g_list_nth_data (gkb->maps, 0);

  system (gkb->dact->command);

  applet_widget_register_stock_callback (APPLET_WIDGET (gkb->applet),
					 "properties",
					 GNOME_STOCK_MENU_PROP,
					 _("Properties..."),
					 properties_dialog, gkb);

  applet_widget_register_stock_callback (APPLET_WIDGET (gkb->applet),
					 "help",
					 GNOME_STOCK_MENU_BOOK_OPEN,
					 _("Help"), help_cb, NULL);

  applet_widget_register_stock_callback (APPLET_WIDGET (gkb->applet),
					 "about",
					 GNOME_STOCK_MENU_ABOUT,
					 _("About..."), about_cb, NULL);

  return applet_widget_corba_activate (gkb->applet, poa, goad_id, params,
				       impl_ptr, ev);
}

static void
gkb_deactivator (PortableServer_POA poa,
		 const char *goad_id,
		 gpointer impl_ptr, CORBA_Environment * ev)
{
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

  bindtextdomain (PACKAGE, GNOMELOCALEDIR);
  textdomain (PACKAGE);

  applet_widget_init ("gkb_applet", VERSION, argc, argv, NULL, 0, NULL);

  APPLET_ACTIVATE (gkb_activator, "gkb_applet", &gkb_impl);

  applet_widget_gtk_main ();

  APPLET_DEACTIVATE (gkb_deactivator, "gkb_applet", gkb_impl);

  return 0;
}
#endif
