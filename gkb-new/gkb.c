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

#include <config.h>
#include <string.h>
#include <gnome.h>
#include <applet-widget.h>

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <libart_lgpl/art_alphagamma.h>
#include <libart_lgpl/art_filterlevel.h>
#include <libart_lgpl/art_pixbuf.h>
#include <libart_lgpl/art_rgb_pixbuf_affine.h>
#include <libart_lgpl/art_affine.h>

#include <sys/types.h>
#include <dirent.h>		/* for opendir() et al. */
#include <string.h>		/* for strncmp() */

typedef struct _Prop Prop;
struct _Prop
{
  int i;  
  GdkPixmap *pix;

  char *name;
  char *command;
  char *iconpath;
  
  GtkWidget *notebook;
  GtkWidget *label1;
  GtkWidget *iconentry;
  GtkWidget *keymapname;
  GtkWidget *commandinput;
  GtkWidget *iconpathinput;
  GtkWidget *vbox1, *hbox1, *vbox2, *hbox2, *hbox3, *hboxmap;
  GtkWidget *frame1, *frame2, *frame3, *frame4, *frame6;
  GtkWidget *preset_opt, *preset_opt_menu, *gkb_menuitem;
  GtkWidget *newkeymap, *delkeymap;
  
  char tname[256],tcommand[256],ticonpath[256];
};

typedef struct _GKB GKB;
struct _GKB
{
  GtkWidget *applet;
  GtkWidget *frame;
  GtkWidget *darea;
  GtkWidget *propbox;
  GtkWidget *notebook;

  int n, cur, size, w, h;

  GList *maps;
  Prop *dact;
  PanelOrientType orient;

};

GtkWidget *bah_window = NULL;

static void gkb_button_press_event_cb (GtkWidget * widget, GdkEventButton * e,
				       GKB * gkb);
static void gkb_draw (GKB * gkb);
void properties_dialog (AppletWidget * applet, gpointer gkbx);
static void about_cb (AppletWidget * widget, gpointer gkbx);
static void help_cb (AppletWidget * widget, gpointer data);
static void prophelp_cb (AppletWidget * widget, gpointer data);
static void makenotebook(GKB * gkb,Prop * actdata, int i);
Prop * loadprop(GKB * gkb,int i);

static GList *
load_presets(GKB * gkb)
{
  int i, num_sets;
  char prefix[1024];
  GSList* list = 0;

  g_snprintf (prefix, sizeof(prefix), "gkb.presets/gkb");

  gnome_config_push_prefix (prefix);
  num_sets = gnome_config_get_int ("number_of_sets=0");
  gnome_config_pop_prefix ();

  for (i = 0; i < num_sets; ++i)
    {
     /* 
     g_snprintf (prefix, sizeof(prefix), "%s%s/%d,", file, session, i);
     */
    }
}

static void
makepix (Prop * actdata, char *fname, int w, int h)
{
  GdkPixbuf *pix;

  pix = gdk_pixbuf_new_from_file (fname);
  if (pix != NULL)
    {
      double affine[6];
      guchar *rgb;
      GdkGC *gc;

      g_assert (bah_window);
      g_assert (bah_window->window);

      affine[1] = affine[2] = affine[4] = affine[5] = 0;

      affine[0] = w / (double) (pix->art_pixbuf->width);
      affine[3] = h / (double) (pix->art_pixbuf->height);

      rgb = g_new0 (guchar, w * h * 3);

      art_rgb_pixbuf_affine (rgb,
			     0, 0, w, h, w * 3,
			     pix->art_pixbuf, affine,
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

static void
tell_panel(void)
{
    CORBA_Environment ev;
    GNOME_Panel panel_client = CORBA_OBJECT_NIL;

    panel_client = goad_server_activate_with_repo_id(NULL,
           "IDL:GNOME/Panel:1.0",
            GOAD_ACTIVATE_EXISTING_ONLY,
            NULL);
    if(!panel_client) return;
    CORBA_exception_init(&ev);
    GNOME_Panel_notice_config_changes(panel_client, &ev);
    CORBA_exception_free(&ev);
}

static void
sized_render (GKB * gkb)
{
  Prop *actdata;
  int i = 0;

  gtk_drawing_area_size (GTK_DRAWING_AREA (gkb->darea), gkb->w, gkb->h);
  gtk_widget_set_usize (GTK_WIDGET        (gkb->darea), gkb->w, gkb->h);
  gtk_widget_set_usize (GTK_WIDGET        (gkb->frame), gkb->w, gkb->h);
  gtk_widget_set_usize (GTK_WIDGET        (gkb->applet), gkb->w, gkb->h);

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

  while (actdata = g_list_nth_data (gkb->maps, i++))
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
  gkb_draw(gkb);
}

static void
gkb_change_pixel_size (GtkWidget * w, int size, gpointer data)
{
  GKB *gkb = data;

  gkb->size = size;
  sized_render (gkb);
  gkb_draw(gkb);
}

Prop *
loadprop(GKB * gkb, int i)
{
      char buf[256];
      
      Prop * actdata = g_new (Prop, 1);

      g_snprintf (buf, 256, "gkb/name%d=Hu",i);
      actdata->name = gnome_config_get_string (buf);

      g_snprintf (buf, 256, "gkb/image%d=%s",i,
		  gnome_unconditional_pixmap_file ("gkb/hu.png"));
      actdata->iconpath = gnome_config_get_string (buf);

      g_snprintf (buf, 256, "gkb/command%d=setxkbmap hu2",i);
      actdata->command = gnome_config_get_string (buf);

      actdata->pix = NULL;
      gkb->orient =
	applet_widget_get_panel_orient (APPLET_WIDGET (gkb->applet));
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

      gnome_config_pop_prefix ();

      return actdata;

}

static void
load_properties (GKB * gkb)
{
  char buf[256];
  int i;
  
  Prop *actdata;

  gnome_config_push_prefix (APPLET_WIDGET (gkb->applet)->privcfgpath);

  g_snprintf (buf, 256, "gkb/num=0");
  gkb->n = gnome_config_get_int (buf);

  if (gkb->n == 0){
      actdata = loadprop(gkb,0);
      gkb->maps = g_list_append (gkb->maps, actdata);
      gkb->n++;
      
      /* next one... TODO: remove this... or no? */

      actdata = loadprop(gkb,1);
      gkb->maps = g_list_append (gkb->maps, actdata);
      gkb->n++;
      } else 
	  for (i=0;i<gkb->n;i++) 
	  {
           actdata = loadprop(gkb,i);
           gkb->maps = g_list_append (gkb->maps, actdata);
          }
      applet_save_session (gkb->propbox,
                           APPLET_WIDGET(gkb->applet)->privcfgpath,
                           APPLET_WIDGET(gkb->applet)->globcfgpath,
                           gkb);
}

static void
delmap_cb (GnomePropertyBox * pb, Prop * data)
{
   /* TODO: write this

   g_list_remove((GKB *)(data->gkb)->maps,data);
    
   gtk_notebook_remove (GTK_NOTEBOOK (data->notebook),
        gtk_notebook_get_nth_page (GTK_NOTEBOOK(data->notebook),
	   data->i);
    */	   
}

static void
newmap_cb (GnomePropertyBox * pb, GKB * gkb)
{

      Prop * actdata = loadprop(gkb,0);
    
      makenotebook(gkb, actdata, gkb->n);

      gkb->maps = g_list_append (gkb->maps, actdata);

      gkb->n++;

}

static void
changed_cb (GnomePropertyBox * pb, GKB * gkb)
{
     gnome_property_box_changed (GNOME_PROPERTY_BOX (gkb->propbox));
}

static void
apply_cb (GtkWidget * pb, gint page, GKB * gkb)
{
  Prop * actdata;
  int i = 0;
    
  if (page != -1)
    return;			/* Thanks Havoc -- Julian7 */

  while ((actdata = g_list_nth_data (gkb->maps, i++)) != NULL)
    {
      /* TODO: tname, ticonpath, tcommand */
      /* contains the updated values after this... */

      actdata->name = gtk_entry_get_text(GTK_ENTRY (actdata->keymapname));
      
      actdata->iconpath = gnome_icon_entry_get_filename (GNOME_ICON_ENTRY(actdata->iconentry));
	  
      actdata->command = gtk_entry_get_text(GTK_ENTRY(actdata->commandinput));

      gtk_notebook_set_tab_label_text (GTK_NOTEBOOK (actdata->notebook),
        gtk_notebook_get_nth_page (GTK_NOTEBOOK(actdata->notebook),
	   actdata->i),gtk_entry_get_text (GTK_ENTRY (actdata->keymapname)));
  
      gtk_widget_show (actdata->notebook);
    }
  
  /* TODO: is this enough? No, I think..*/
  
  sized_render (gkb);
  gkb_draw(gkb);
  
  system (gkb->dact->command);
  
  tell_panel();
}

static void
destroy_cb (GtkWidget * widget, GKB * gkb)
{
  gkb->propbox = NULL;
}

static void
makenotebook(GKB * gkb,Prop * actdata, int i)
{
    
      actdata->notebook = gkb->notebook;
      
      actdata->i = i;
      
      actdata->vbox1 = gtk_vbox_new (FALSE, 0);
      
      gtk_widget_ref (actdata->vbox1);
      gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "vbox1",
				actdata->vbox1,
				(GtkDestroyNotify) gtk_widget_unref);
      gtk_widget_show (actdata->vbox1);
      gtk_container_add (GTK_CONTAINER (gkb->notebook), actdata->vbox1);

      actdata->hbox1 = gtk_hbox_new (FALSE, 0);
      gtk_widget_ref (actdata->hbox1);
      gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "hbox1",
				actdata->hbox1,
				(GtkDestroyNotify) gtk_widget_unref);
      gtk_widget_show (actdata->hbox1);

      actdata->frame4 = gtk_frame_new (_("Keymap name"));
      gtk_widget_ref (actdata->frame4);
      gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "frame4",
				actdata->frame4,
				(GtkDestroyNotify) gtk_widget_unref);
      gtk_widget_show (actdata->frame4);
      gtk_box_pack_start (GTK_BOX (actdata->hbox1), actdata->frame4, FALSE, TRUE, 0);
      gtk_container_set_border_width (GTK_CONTAINER (actdata->frame4), 2);

      gtk_container_add (GTK_CONTAINER (actdata->vbox1), actdata->hbox1);

      actdata->keymapname = gtk_entry_new();
      gtk_widget_ref (actdata->keymapname);
      
      memcpy (&actdata->tname,actdata->name,strlen(actdata->name)+1);

      gtk_entry_set_text(actdata->keymapname, &actdata->tname );
      
      gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "keymapname",
				actdata->keymapname,
				(GtkDestroyNotify) gtk_widget_unref);
      gtk_widget_show (actdata->keymapname);
      gtk_container_add (GTK_CONTAINER (actdata->frame4), actdata->keymapname);

      gtk_signal_connect (GTK_OBJECT (actdata->keymapname),
			  "changed", (GtkSignalFunc) changed_cb, gkb);

      actdata->frame6 = gtk_frame_new (_("Keymap control"));
      gtk_widget_ref (actdata->frame6);
      gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "frame6",
				actdata->frame6,
				(GtkDestroyNotify) gtk_widget_unref);
      gtk_widget_show (actdata->frame6);
      gtk_box_pack_start (GTK_BOX (actdata->hbox1), actdata->frame6, TRUE, TRUE, 0);
      gtk_container_set_border_width (GTK_CONTAINER (actdata->frame6), 2);


      actdata->hboxmap = gtk_hbox_new(FALSE,0);
      
      actdata->newkeymap = gtk_button_new_with_label ("New keymap");

      gtk_widget_ref (actdata->newkeymap);
      gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "newkeymap",
				actdata->newkeymap,
				(GtkDestroyNotify) gtk_widget_unref);
      gtk_widget_show (actdata->newkeymap);
      gtk_signal_connect (GTK_OBJECT (actdata->newkeymap),
			  "clicked", (GtkSignalFunc) newmap_cb, gkb);
      
      gtk_container_add (GTK_CONTAINER (actdata->hboxmap), actdata->newkeymap);
      gtk_container_set_border_width (GTK_CONTAINER (actdata->newkeymap), 2);

      actdata->delkeymap = gtk_button_new_with_label ("Delete this keymap");

      gtk_widget_ref (actdata->delkeymap);
      gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "delkeymap",
				actdata->delkeymap,
				(GtkDestroyNotify) gtk_widget_unref);
      gtk_widget_show (actdata->delkeymap);
      gtk_signal_connect (GTK_OBJECT (actdata->delkeymap),
			  "clicked", (GtkSignalFunc) delmap_cb, gkb);
      
      gtk_container_add (GTK_CONTAINER (actdata->hboxmap), actdata->delkeymap);
      gtk_container_set_border_width (GTK_CONTAINER (actdata->delkeymap), 2);
      gtk_container_add (GTK_CONTAINER (actdata->frame6), actdata->hboxmap);

      gtk_widget_show(actdata->hboxmap);
      
      actdata->frame3 = gtk_frame_new (_("Presets"));
      gtk_widget_ref (actdata->frame3);
      gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "frame3",
				actdata->frame3,
				(GtkDestroyNotify) gtk_widget_unref);
      gtk_widget_show (actdata->frame3);
      gtk_box_pack_start (GTK_BOX (actdata->vbox1), actdata->frame3, FALSE, FALSE, 0);
      gtk_container_set_border_width (GTK_CONTAINER (actdata->frame3), 2);

      actdata->preset_opt = gtk_option_menu_new ();
      gtk_widget_ref (actdata->preset_opt);
      gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "preset_opt",
				actdata->preset_opt,
				(GtkDestroyNotify) gtk_widget_unref);
      gtk_widget_show (actdata->preset_opt);
      gtk_container_add (GTK_CONTAINER (actdata->frame3), actdata->preset_opt);
      gtk_container_set_border_width (GTK_CONTAINER (actdata->preset_opt), 2);

      actdata->preset_opt_menu = gtk_menu_new ();
      
      actdata->gkb_menuitem = gtk_menu_item_new_with_label (_("US querty"));
      gtk_widget_show (actdata->gkb_menuitem);
      gtk_menu_append (GTK_MENU (actdata->preset_opt_menu), actdata->gkb_menuitem);
      actdata->gkb_menuitem = gtk_menu_item_new_with_label (_("Hungarian Latin 1 quertz"));
      gtk_widget_show (actdata->gkb_menuitem);
      gtk_menu_append (GTK_MENU (actdata->preset_opt_menu), actdata->gkb_menuitem);
      actdata->gkb_menuitem = gtk_menu_item_new_with_label (_("Hungarian Latin 2 quertz"));
      gtk_widget_show (actdata->gkb_menuitem);
      gtk_menu_append (GTK_MENU (actdata->preset_opt_menu), actdata->gkb_menuitem);
      gtk_option_menu_set_menu (GTK_OPTION_MENU (actdata->preset_opt), actdata->preset_opt_menu);
      
      actdata->hbox2 = gtk_hbox_new (FALSE, 0);
      gtk_widget_ref (actdata->hbox2);
      gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "hbox2",
				actdata->hbox2,
				(GtkDestroyNotify) gtk_widget_unref);
      gtk_widget_show (actdata->hbox2);
      gtk_box_pack_start (GTK_BOX (actdata->vbox1), actdata->hbox2, TRUE, TRUE, 0);

      actdata->iconentry = gnome_icon_entry_new (NULL, _("Keymap icon"));
      gtk_widget_ref (actdata->iconentry);
      gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "iconentry",
				actdata->iconentry,
				(GtkDestroyNotify) gtk_widget_unref);
      gnome_icon_entry_set_icon(actdata->iconentry, actdata->iconpath);
      gtk_widget_show (actdata->iconentry);
      
      gtk_box_pack_end (GTK_BOX (actdata->hbox2), actdata->iconentry, FALSE, FALSE,
			0);
      gtk_widget_set_usize (actdata->iconentry, 60, 40);
      gtk_container_set_border_width (GTK_CONTAINER (actdata->iconentry), 2);

      actdata->vbox2 = gtk_vbox_new (FALSE, 0);
      gtk_widget_ref (actdata->vbox2);
      gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "vbox2",
				actdata->vbox2,
				(GtkDestroyNotify) gtk_widget_unref);
      gtk_widget_show (actdata->vbox2);
      gtk_box_pack_start (GTK_BOX (actdata->hbox2), actdata->vbox2, TRUE, TRUE, 0);

      actdata->frame1 = gtk_frame_new (_("Iconpath"));
      gtk_widget_ref (actdata->frame1);
      gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "frame1",
				actdata->frame1,
				(GtkDestroyNotify) gtk_widget_unref);
      gtk_widget_show (actdata->frame1);
      gtk_box_pack_start (GTK_BOX (actdata->vbox2), actdata->frame1, TRUE, TRUE, 0);
      gtk_container_set_border_width (GTK_CONTAINER (actdata->frame1), 2);

      actdata->iconpathinput = gtk_entry_new ();
      gtk_widget_ref (actdata->iconpathinput);
      gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "iconpathinput",
				actdata->iconpathinput,
				(GtkDestroyNotify) gtk_widget_unref);
      gtk_signal_connect (GTK_OBJECT (actdata->iconpathinput), "changed",
			  GTK_SIGNAL_FUNC (changed_cb), gkb);

      memcpy (&actdata->ticonpath,actdata->iconpath,strlen(actdata->iconpath)+1);
      gtk_entry_set_text(actdata->iconpathinput,&actdata->ticonpath);
      gtk_widget_show (actdata->iconpathinput);
      gtk_container_add (GTK_CONTAINER (actdata->frame1), actdata->iconpathinput);

      actdata->frame2 = gtk_frame_new (_("Full command"));
      gtk_widget_ref (actdata->frame2);
      gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "frame2",
				actdata->frame2,
				(GtkDestroyNotify) gtk_widget_unref);
      gtk_widget_show (actdata->frame2);
      gtk_box_pack_start (GTK_BOX (actdata->vbox2), actdata->frame2, TRUE, TRUE, 0);
      gtk_container_set_border_width (GTK_CONTAINER (actdata->frame2), 2);

      actdata->commandinput = gtk_entry_new();
      gtk_widget_ref (actdata->commandinput);
      gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "commandinput",
				actdata->commandinput,
				(GtkDestroyNotify) gtk_widget_unref);

      memcpy (&actdata->tcommand,actdata->command,strlen(actdata->command)+1);
      gtk_entry_set_text(actdata->commandinput,&actdata->tcommand);
      
      gtk_signal_connect (GTK_OBJECT (actdata->commandinput), "changed",
			  GTK_SIGNAL_FUNC (changed_cb), gkb);
      gtk_widget_show (actdata->commandinput);
      gtk_container_add (GTK_CONTAINER (actdata->frame2), actdata->commandinput);

      actdata->label1 = gtk_label_new (actdata->name);
      gtk_widget_ref (actdata->label1);
      gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "label1",
				actdata->label1,
				(GtkDestroyNotify) gtk_widget_unref);
      gtk_widget_show (actdata->label1);
      gtk_notebook_set_tab_label (GTK_NOTEBOOK (gkb->notebook),
				  gtk_notebook_get_nth_page (
				  GTK_NOTEBOOK(gkb->notebook),i),
				  actdata->label1);
    
}

void
properties_dialog (AppletWidget * applet, gpointer gkbx)
{

  GKB *gkb = (GKB *) gkbx;

  Prop *actdata;
  int i = 0;

  gkb->propbox = gnome_property_box_new ();
  gtk_object_set_data (GTK_OBJECT (gkb->propbox), "propbox",
			   gkb->propbox);
  gtk_window_set_title (GTK_WINDOW (gkb->propbox), _("GKB Properties"));
  gtk_window_set_policy (GTK_WINDOW (gkb->propbox), FALSE, FALSE, TRUE);
  gtk_widget_show_all (GTK_WINDOW (gkb->propbox));

  gkb->notebook = GNOME_PROPERTY_BOX (gkb->propbox)->notebook;
  gtk_object_set_data (GTK_OBJECT (gkb->propbox), "notebook",
			   gkb->notebook);
  gtk_widget_show (gkb->notebook);
  gtk_notebook_set_scrollable (GTK_NOTEBOOK (gkb->notebook), TRUE);
  gtk_notebook_popup_enable (GTK_NOTEBOOK (gkb->notebook));
  gtk_widget_show_all (gkb->propbox);

  while (actdata = g_list_nth_data (gkb->maps, i))
    { 	
      makenotebook(gkb,actdata,i);
      i++;
    }

  gtk_signal_connect (GTK_OBJECT (gkb->propbox),
		      "apply", GTK_SIGNAL_FUNC (apply_cb), gkb);
  gtk_signal_connect (GTK_OBJECT (gkb->propbox),
		      "destroy", GTK_SIGNAL_FUNC (destroy_cb), gkb);
  gtk_signal_connect (GTK_OBJECT (gkb->propbox),
		      "help", GTK_SIGNAL_FUNC (prophelp_cb), NULL);
  gtk_widget_show_all (gkb->propbox);

  return;
  applet = NULL;
}

static void
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

  gkb->dact = g_list_nth_data(gkb->maps,0);

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
			     "her solidarity."), 
			     "gkb.png");

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

static void
prophelp_cb (AppletWidget * applet, gpointer data)
{
  GnomeHelpMenuEntry help_entry = { "gkb_applet",
    "index.html#gkb-prefs"
  };

  gnome_help_display (NULL, &help_entry);
}

static int
applet_save_session (GtkWidget * w,
		     const char *privcfgpath,
		     const char *globcfgpath, GKB * gkb)
{
  Prop *actdata;
  int i = 0;
  char str[100];

  gnome_config_push_prefix (privcfgpath);
  sprintf (str, "gkb/num");
  gnome_config_set_int (str, gkb->n);

  while (actdata = g_list_nth_data (gkb->maps, i))
    {
      sprintf (str, "gkb/name%d", i);
      gnome_config_set_string (str, actdata->name);
      sprintf (str, "gkb/image%d", i);
      gnome_config_set_string (str, actdata->iconpath);
      sprintf (str, "gkb/command%d", i);
      gnome_config_set_string (str, actdata->command);
      
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
  GKB * gkb;
  GList * list;

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

  list = load_presets (gkb);

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
					 "about",
					 GNOME_STOCK_MENU_ABOUT,
					 _("About..."), about_cb, NULL);

  applet_widget_register_stock_callback (APPLET_WIDGET (gkb->applet),
					 "help",
					 GNOME_STOCK_MENU_BOOK_OPEN,
					 _("Help"), help_cb, NULL);

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
