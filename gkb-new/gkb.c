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

#include <config.h>

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
#include <egg-screen-help.h>
#include "gkb.h"

int NumLockMask, CapsLockMask, ScrollLockMask;

gboolean gail_loaded = FALSE;

gchar *
gkb_load_pref(GKB *gkb, const gchar * key, const gchar * defaultv);

static gint gkb_button_press_event_cb (GtkWidget * widget,
				       GdkEventButton * e,
			               GKB *gkb);
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

/**
 * gkb_draw:
 * @gkb: 
 * 
 * Draw the applet from the pixmap loaded in gkb->dact
 **/
static void
gkb_draw (GKB * gkb)
{
  g_return_if_fail (gkb->image != NULL);
  g_return_if_fail (gkb->keymap != NULL);

  if (gkb->mode != GKB_LABEL)
    {
      GdkPixbuf *tmp;
      
      g_return_if_fail (gkb != NULL);
      g_return_if_fail (gkb->keymap != NULL);
      g_return_if_fail (gkb->keymap->pixbuf != NULL);
      
      tmp = gdk_pixbuf_scale_simple (gkb->keymap->pixbuf, gkb->w * 0.9, 
      				     gkb->h * 0.9, GDK_INTERP_HYPER); 
      if (tmp) {     
        gtk_image_set_from_pixbuf (GTK_IMAGE (gkb->image), tmp);
        g_object_unref (tmp);
      }

    }

  if (gkb->mode != GKB_FLAG)
    {
      gtk_label_set_text (GTK_LABEL (gkb->label1), gkb->keymap->label);
      gtk_label_set_text (GTK_LABEL (gkb->label2), gkb->keymap->label);
    }

  gtk_tooltips_set_tip (gkb->tooltips, gkb->applet, gkb->keymap->name, NULL);
  add_atk_namedesc(gkb->applet, gkb->keymap->name, _("Press the hotkey to switch between layouts. The hotkey can be set through Properties dialog"));

}

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

  gkb->w = flag_width - 1;
  /* FIXME the applet is just a little bigger than panel, so I add the -2 here*/
  gkb->h = flag_height - 1;
  gtk_widget_set_size_request (gkb->label_frame1, gkb->w, gkb->h);
  gtk_widget_set_size_request (gkb->label_frame2, gkb->w, gkb->h);
  gtk_widget_set_size_request (gkb->darea_frame, gkb->w, gkb->h);

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
	  if (keymap->pixbuf) 
	    g_object_unref (keymap->pixbuf);
	  keymap->pixbuf = gdk_pixbuf_new_from_file (real_name,NULL);
	}
      else
	{
	  g_free (real_name);
	  real_name = gnome_unconditional_pixmap_file ("gkb/gkb-foot.png");
	  if (keymap->pixbuf)
	    g_object_unref (keymap->pixbuf);
	  keymap->pixbuf = gdk_pixbuf_new_from_file (real_name,NULL);
	}
      g_free (name);
      g_free (real_name);
    }

}

void gkb_update_handlers (GKB *gkb, gboolean disconnect)
{
  GdkWindow *root_window;

  root_window = gdk_get_default_root_window ();

  if (disconnect)
  {
    if (gkb->button_press_id != -1)
      g_signal_handler_disconnect (gkb->eventbox, gkb->button_press_id);
    gkb->button_press_id = -1;

    gdk_window_remove_filter (root_window, event_filter, gkb);
  }
  else {
    gdk_window_add_filter (root_window, event_filter, gkb);

    if (gkb->button_press_id == -1)
      gkb->button_press_id = g_signal_connect (gkb->eventbox, "button_press_event",
                                               G_CALLBACK (gkb_button_press_event_cb), gkb);

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
  g_return_if_fail (gkb->maps);

  /* When the size is changed, we don't need to change the actual
   * keymap, so when the set_commadn is false, it means that we
   * have changed size. In other words, we can't change size &
   * keymap at the same time */
  if (set_command) {
    gkb_update_handlers (gkb, TRUE);
    gkb_system_set_keymap (gkb);
  }
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
applet_save_session (GKB *gkb)
{
  const gchar *text;
  GkbKeymap *actdata;
  GList *list = gkb->maps;
  gchar str[100];
  int i = 0;

  gconf_applet_set_int (PANEL_APPLET (gkb->applet), "num", gkb->n, NULL);
  gconf_applet_set_bool (PANEL_APPLET (gkb->applet), "small", gkb->is_small, NULL);
  gconf_applet_set_string (PANEL_APPLET (gkb->applet), "key", gkb->key, NULL);
  text = gkb_util_get_text_from_mode (gkb->mode);
  gconf_applet_set_string (PANEL_APPLET (gkb->applet), "mode", text, NULL);

  while (list)
    {
      actdata = list->data;
      if (actdata)
	{
	  g_snprintf (str, sizeof (str), "name_%d", i);
	  gconf_applet_set_string (PANEL_APPLET (gkb->applet), str, actdata->name, NULL);
	  g_snprintf (str, sizeof (str), "country_%d", i);
	  gconf_applet_set_string (PANEL_APPLET (gkb->applet), str, actdata->country, NULL);
	  g_snprintf (str, sizeof (str), "lang_%d", i);
	  gconf_applet_set_string (PANEL_APPLET (gkb->applet), str, actdata->lang, NULL);
	  g_snprintf (str, sizeof (str), "label_%d", i);
	  gconf_applet_set_string (PANEL_APPLET (gkb->applet), str, actdata->label, NULL);
	  g_snprintf (str, sizeof (str), "flag_%d", i);
	  gconf_applet_set_string (PANEL_APPLET (gkb->applet), str, actdata->flag, NULL);
	  g_snprintf (str, sizeof (str), "command_%d", i);
	  gconf_applet_set_string (PANEL_APPLET (gkb->applet), str, actdata->command, NULL);
	}

      list = list->next;

      i++;
    }
}

gchar *
gkb_load_pref(GKB *gkb, const gchar * key, const gchar * defaultv)
{
 gchar * value;

 value = gconf_applet_get_string (PANEL_APPLET(gkb->applet), key, NULL);
 
 if (value == NULL) {
  value = g_strdup (defaultv);
 }
 return value;
}

GkbKeymap *
loadprop (GKB *gkb, int i)
{
  GkbKeymap *actdata;
  gchar *buf;
  gchar *default_cmd;

  actdata = g_new0 (GkbKeymap, 1);


  buf = g_strdup_printf ("name_%d",i);
  actdata->name = gkb_load_pref (gkb, buf, (i?"US 105 key keyboard":"Gnome Keyboard default"));
  g_free (buf);

  buf = g_strdup_printf ("label_%d", i);
  actdata->label = gkb_load_pref (gkb, buf, (i?"us":"gkb"));
  g_free (buf);
  
  buf = g_strdup_printf ("country_%d", i);
  actdata->country = gkb_load_pref (gkb, buf, (i?"United States":"Gnome Keyboard"));
  g_free (buf);
  
  buf = g_strdup_printf ("lang_%d", i);
  actdata->lang = gkb_load_pref (gkb, buf, (i?"English":"International"));
  g_free (buf);
  
  buf = g_strdup_printf ("flag_%d", i);
  actdata->flag = gkb_load_pref (gkb, buf, (i?"us.png":"gkb.png"));
  g_free (buf);
  
  buf = g_strdup_printf ("command_%d", i);
  default_cmd = g_strdup_printf ("xmodmap %s/.gkb_default.xmm", g_get_home_dir ()); 
  actdata->command = gkb_load_pref (gkb, buf, (i?"gkb_xmmap us":default_cmd));
  g_free (default_cmd);
  g_free(buf);

  actdata->pixbuf = NULL;
  gkb->orient = panel_applet_get_orient (PANEL_APPLET (gkb->applet));

  return actdata;
}

static void
do_at_first_run (GKB * gkb) 
{
 gboolean firstrun;
 gchar *cmd;
 firstrun = gconf_applet_get_bool (PANEL_APPLET(gkb->applet), 
                                      "first_run", NULL);
 if (!firstrun) {
  cmd = g_strdup_printf ("echo \"! This file has generated by Gnome Keyboard applet\" > %s/.gkb_default.xmm", g_get_home_dir ());
  system (cmd);
  g_free (cmd);

  cmd = g_strdup_printf ("xmodmap -pke >> %s/.gkb_default.xmm", g_get_home_dir ());
  system (cmd);
  g_free (cmd);

  gconf_applet_set_bool (PANEL_APPLET (gkb->applet), "first_run", FALSE, NULL);
 }
 
}

static void
load_properties (GKB * gkb)
{
  GkbKeymap *actdata;
  gchar *text;
  int i;

  gkb->maps = NULL;

  gkb->n = gconf_applet_get_int (PANEL_APPLET(gkb->applet), "num", NULL);

  gkb->key = gkb_load_pref (gkb, "key", "Alt-Shift_L");
  gkb->old_key = g_strdup (gkb->key);

  convert_string_to_keysym_state (gkb->key, &gkb->keysym, &gkb->state);
  gkb->old_keysym = gkb->keysym;
  gkb->old_state = gkb->state;

  gkb->is_small = gconf_applet_get_bool (PANEL_APPLET(gkb->applet), "small", NULL);
#ifdef THIS_IS_UNNECESSARY
  if (gkb->is_small == NULL) {
    gkb->is_small = TRUE;
  }
#endif 
  text = gkb_load_pref (gkb, "mode", "Flag and Label");
  gkb->mode = gkb_util_get_mode_from_text (text);
  g_free (text);

  if (gkb->n == 0)
    {
      actdata = loadprop (gkb, 0);
      gkb->maps = g_list_append (gkb->maps, actdata);
      gkb->n++;

      actdata = loadprop (gkb, 1);
      gkb->maps = g_list_append (gkb->maps, actdata);
      gkb->n++;
    }
  else
    for (i = 0; i < gkb->n; i++)
      {
	actdata = loadprop (gkb, i);
	gkb->maps = g_list_append (gkb->maps, actdata);
      }

  applet_save_session (gkb);
}


static gint
gkb_button_press_event_cb (GtkWidget * widget, GdkEventButton * event, GKB *gkb)
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

static void
create_gkb_widget (GKB *gkb)
{
  gkb->eventbox = gtk_event_box_new ();
  gtk_widget_show (gkb->eventbox);

  gkb->vbox = gtk_vbox_new (TRUE, 0);
  gtk_widget_show (gkb->vbox);
  gkb->hbox = gtk_hbox_new (TRUE, 0);
  gtk_widget_show (gkb->hbox);

  gtk_container_add (GTK_CONTAINER (gkb->eventbox), gkb->hbox);
  gtk_container_add (GTK_CONTAINER (gkb->hbox), gkb->vbox);

  gkb->image = gtk_image_new ();

  gkb->button_press_id = g_signal_connect (gkb->eventbox, "button_press_event",
                                           G_CALLBACK (gkb_button_press_event_cb), gkb);

  gkb->darea_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (gkb->darea_frame), GTK_SHADOW_OUT);
  gtk_container_add (GTK_CONTAINER (gkb->darea_frame), gkb->image);
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
  
  gkb->tooltips = gtk_tooltips_new ();

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
  GError    *error = NULL;

  static const gchar *authors[] = {
	        "Szabolcs Ban <shooby@gnome.hu>",
		"Chema Celorio <chema@celorio.com>",
		NULL
	};
  static const gchar *docauthors[] = {
		"Alexander Kirillov <kirillov@math.sunysb.edu>",
		"Emese Kovacs <emese@gnome.hu>",
		"David Mason <dcm@redhat.com>",
	        "Szabolcs Ban <shooby@gnome.hu>",
		NULL
	};

  const gchar *translator_credits = _("translator_credits");

  file = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_PIXMAP, "gkb-icon.png", FALSE, NULL);
  pixbuf = gdk_pixbuf_new_from_file (file, &error);
  g_free (file);

  if (error) {
         g_warning (G_STRLOC ": cannot open %s: %s", file, error->message);
         g_error_free (error);
  }

  about = gnome_about_new (_("Keyboard Layout Switcher"),
			   VERSION,
			   _("(C) 1998-2000 Free Software Foundation"),
                           _("This applet switches between keyboard maps "
                             "using setxkbmap or xmodmap.\n"
                             "Mail me your flag and keyboard layout "
                             "if you want support for your locale "
                             "(my email address is shooby@gnome.hu).\n"
                             "So long, and thanks for all the fish.\n"
                             "Thanks for Balazs Nagy (Kevin) "
                              "<julian7@iksz.hu> for his help "
                              "and Emese Kovacs <emese@gnome.hu> for "
                              "her solidarity, help from guys like KevinV."
                              "\nShooby Ban <shooby@gnome.hu>"),
                            authors,
			    docauthors,
			    strcmp (translator_credits, "translator_credits") != 0 ? translator_credits : NULL,
			    pixbuf);

  link = gnome_href_new ("http://projects.gnome.hu/gkb",
			 _("GKB Home Page (http://projects.gnome.hu/gkb)"));

  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (about)->vbox), link, TRUE,
		      FALSE, 0);

  if (pixbuf)
  	g_object_unref (pixbuf);

  gtk_window_set_wmclass (GTK_WINDOW (about), "keyboard layout switcher", "Keyboard Layout Switcher");
  gtk_window_set_screen (GTK_WINDOW (about), gtk_widget_get_screen (gkb->applet));
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
	GError *error = NULL;

	egg_help_display_on_screen (
			"gkb", NULL,
			gtk_widget_get_screen (gkb->applet),
			&error);

	/* FIXME: display error to the user */
}

static GdkFilterReturn
global_key_filter (GKB *gkb, GdkXEvent * gdk_xevent, GdkEvent * event)
{
  XKeyEvent * xkevent = (XKeyEvent *) gdk_xevent;
  gint keysym = XKeycodeToKeysym(GDK_DISPLAY(), xkevent->keycode,0);

 if (strcmp (gkb->key, convert_keysym_state_to_string (XKeycodeToKeysym(GDK_DISPLAY (), xkevent->keycode, 0), xkevent->state)) == 0)
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
  GKB *gkb = (GKB *)data;

  xevent = (XEvent *) gdk_xevent;

  if (xevent->type == KeyPress)
    {
      return global_key_filter (gkb, gdk_xevent, event);
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

static const BonoboUIVerb gkb_menu_verbs [] = {
	BONOBO_UI_UNSAFE_VERB ("GKBProperties",   properties_dialog),
	BONOBO_UI_UNSAFE_VERB ("GKBHelp",	  help_cb),
	BONOBO_UI_UNSAFE_VERB ("GKBAbout",	  about_cb),
	BONOBO_UI_VERB_END
};

static void gkb_destroy (GtkWidget * widget, gpointer data)
{
	GKB *gkb = (GKB *)data;
	
	if (gkb->propwindow)
		gtk_widget_destroy (gkb->propwindow);
	gkb_update_handlers (gkb, TRUE);
	g_free(gkb);
}

gboolean fill_gkb_applet(PanelApplet *applet)
{
  GdkWindow *root_window;
  gint keycode, modifiers;
  GKB *gkb;

  gkb = g_new0 (GKB, 1);

  gkb->n = 0;
  gkb->cur = 0;
  gkb->applet = GTK_WIDGET (applet);
  
  gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gkb.png");
  
  panel_applet_add_preferences (applet, "/schemas/apps/gkb-applet/prefs", NULL);

  do_at_first_run(gkb);

  load_properties (gkb);

  gkb->keymap = g_list_nth_data (gkb->maps, 0);

  create_gkb_widget (gkb);

  gtk_widget_show (gkb->darea_frame);
  gtk_container_add (GTK_CONTAINER (gkb->applet), gkb->eventbox);


  keycode = XKeysymToKeycode(GDK_DISPLAY(), gkb->keysym);

  modifiers = gkb->state;

  root_window = gdk_get_default_root_window ();

  gkb->keycode = keycode; 
  /* Incase the key is already grabbed by another application */ 
  gdk_error_trap_push ();
  gkb_xgrab (keycode, modifiers, root_window);
  gdk_flush ();
  gdk_error_trap_pop ();
  gdk_window_add_filter (root_window, event_filter, gkb);

  g_signal_connect (G_OBJECT(gkb->applet),
  			  "destroy", 
  			  G_CALLBACK (gkb_destroy), 
  			  gkb);

  g_signal_connect (G_OBJECT (gkb->applet),
                          "change_orient",
                          G_CALLBACK (gkb_change_orient),
                          gkb);

  g_signal_connect (G_OBJECT (gkb->applet),
                          "change_size",
                          G_CALLBACK (gkb_change_pixel_size),
                          gkb);

  panel_applet_setup_menu_from_file (PANEL_APPLET (gkb->applet),
                                     NULL,
                                     "GNOME_KeyboardApplet.xml",
  				     NULL,
  				     gkb_menu_verbs, 
  				     gkb);

  gtk_widget_show (GTK_WIDGET(gkb->applet));
  gkb_sized_render (gkb);
  gkb_update(gkb,TRUE);

  return TRUE;
}

static gboolean
gkb_factory (PanelApplet *applet,
		const gchar *iid,
		gpointer     data)
{
 	gboolean retval = FALSE;
	if ( GTK_IS_ACCESSIBLE(gtk_widget_get_accessible(GTK_WIDGET(applet))))
	  {
            gail_loaded = TRUE;
          }
        if (!strcmp (iid, "OAFIID:GNOME_KeyboardApplet"))
		retval = fill_gkb_applet (applet);

	return retval;
}

PANEL_APPLET_BONOBO_FACTORY ("OAFIID:GNOME_KeyboardApplet_Factory",
			     PANEL_TYPE_APPLET,
                             "gkb",
                             "0",
                             gkb_factory,
                             NULL)

/* Add AtkRelation */                                                           
void                                                                            
add_atk_relation(GtkWidget *obj1, GtkWidget *obj2, AtkRelationType rel_type)
{
    AtkObject *atk_obj1, *atk_obj2;
    AtkRelationSet *relation_set;
    AtkRelation *relation;

    atk_obj1 = gtk_widget_get_accessible(obj1);

    atk_obj2 = gtk_widget_get_accessible(obj2);

    relation_set = atk_object_ref_relation_set (atk_obj1);
    relation = atk_relation_new(&atk_obj2, 1, rel_type);
    atk_relation_set_add(relation_set, relation);
    g_object_unref(G_OBJECT (relation));
}

/* Accessible name and description */                                           
void                                                                            
add_atk_namedesc(GtkWidget *widget, const gchar *name, const gchar *desc)       
{                                                                               
    AtkObject *atk_widget;                                                      
    atk_widget = gtk_widget_get_accessible(widget);                             
    if (name)                                                                   
    {                                                                           
      atk_object_set_name(atk_widget, name);                                    
    }                                                                           
    if (desc)                                                                   
    {                                                                           
      atk_object_set_description(atk_widget, desc);                             
    }                                                                           
}                                                                               
