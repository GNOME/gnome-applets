/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 2 -*- */
/* File: prop.c
 * Purpose: GNOME Keyboard switcher property box
 *
 * Copyright (C) 1998-2000 Free Software Foundation
 * Authors: Szabolcs BAN <shooby@gnome.hu>
 *          Chema Celorio <chema@celorio.com>
 *
 * Some of functions came from Helixcode's keyboard grabbing sections.
 * Other functions, ideas stolen from other applets, for example
 * from the fish applet, Wanda.
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

#include <gdk/gdkx.h>
#include <sys/stat.h>
#include <X11/Xlib.h>
#include <dirent.h>
#include "gkb.h"



typedef struct _KeymapData KeymapData;
struct _KeymapData
{
 GtkWidget * widget;
 GkbKeymap * preset;
};

static void
switch_normal_cb (GnomePropertyBox * pb)
{
 gkb->tempsmall = 0;
 gkb->tempsize =
    applet_widget_get_panel_pixel_size (APPLET_WIDGET (gkb->applet));
 gnome_property_box_changed (GNOME_PROPERTY_BOX (gkb->propbox));
}

static void
changed_cb (GnomePropertyBox * pb)
{
 gnome_property_box_changed (GNOME_PROPERTY_BOX (gkb->propbox));
}

static void
switch_small_cb (GnomePropertyBox * pb)
{
 gkb->tempsmall = 1;
 gnome_property_box_changed (GNOME_PROPERTY_BOX (gkb->propbox));
}

static GtkWidget *
gkb_prop_create_display_frame ()
{
  GtkWidget *frame;
  gchar *pixmap1_filename;
  GtkWidget *prop_menuitem;
  GtkWidget *hbox11;
  GtkWidget *label23, *label24;
  GtkWidget *optionmenu1, *optionmenu1_menu;
  GtkWidget *optionmenu2, *optionmenu2_menu;
  GtkWidget *pixmap1;
  GtkWidget *table1;
  
  frame = gtk_frame_new (_("Display"));

  hbox11 = gtk_hbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (frame), hbox11);

  pixmap1 = gtk_type_new (gnome_pixmap_get_type ());
  pixmap1_filename = gnome_pixmap_file ("gkb.png");
  if (pixmap1_filename)
    gnome_pixmap_load_file (GNOME_PIXMAP (pixmap1), pixmap1_filename);
  g_free (pixmap1_filename);
  gtk_box_pack_start (GTK_BOX (hbox11), pixmap1, FALSE, FALSE, 23);

  table1 = gtk_table_new (2, 2, TRUE);
  gtk_box_pack_start (GTK_BOX (hbox11), table1, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (table1), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table1), 15);

  optionmenu1 = gtk_option_menu_new ();
  gtk_table_attach (GTK_TABLE (table1), optionmenu1, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  optionmenu1_menu = gtk_menu_new ();
  prop_menuitem = gtk_menu_item_new_with_label (_("Normal"));
  gtk_widget_show (prop_menuitem);  
  gtk_signal_connect (GTK_OBJECT (prop_menuitem), "activate",
                        (GtkSignalFunc) switch_normal_cb, NULL);
  gtk_menu_append (GTK_MENU (optionmenu1_menu), prop_menuitem);
  prop_menuitem = gtk_menu_item_new_with_label (_("Small"));
  gtk_widget_show (prop_menuitem);
  gtk_signal_connect (GTK_OBJECT (prop_menuitem), "activate",
                        (GtkSignalFunc) switch_small_cb, NULL);

  gtk_menu_append (GTK_MENU (optionmenu1_menu), prop_menuitem);
  gtk_option_menu_set_menu (GTK_OPTION_MENU (optionmenu1), optionmenu1_menu);
  gtk_option_menu_set_history (GTK_OPTION_MENU (optionmenu1), 0);

  optionmenu2 = gtk_option_menu_new ();
  gtk_table_attach (GTK_TABLE (table1), optionmenu2, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND), 0, 0);
  optionmenu2_menu = gtk_menu_new ();
  prop_menuitem = gtk_menu_item_new_with_label (_("Label"));
  gtk_widget_show (prop_menuitem);  
  gtk_menu_append (GTK_MENU (optionmenu2_menu), prop_menuitem);
  prop_menuitem = gtk_menu_item_new_with_label (_("Flag"));
  gtk_widget_show (prop_menuitem);  
  gtk_menu_append (GTK_MENU (optionmenu2_menu), prop_menuitem);
  prop_menuitem = gtk_menu_item_new_with_label (_("Flag and label"));
  gtk_widget_show (prop_menuitem);  
  gtk_menu_append (GTK_MENU (optionmenu2_menu), prop_menuitem);
  gtk_option_menu_set_menu (GTK_OPTION_MENU (optionmenu2), optionmenu2_menu);
  gtk_option_menu_set_history (GTK_OPTION_MENU (optionmenu2), 1);

  label23 = gtk_label_new (_("Appearance"));
  gtk_table_attach (GTK_TABLE (table1), label23, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 10, 0);
  gtk_label_set_justify (GTK_LABEL (label23), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (label23), 1, 0.5);

  label24 = gtk_label_new (_("Applet size"));
  gtk_table_attach (GTK_TABLE (table1), label24, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 10, 0);
  gtk_label_set_justify (GTK_LABEL (label24), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (label24), 1, 0.5);

  return frame;
}


/**
 * gkb_prop_create_hotkey_frame:
 * @void: 
 * 
 * Implement the hotkey properties frame
 * 
 * Return Value: 
 **/
static GtkWidget *
gkb_prop_create_hotkey_frame (void)
{
  GtkWidget *frame;
  GtkWidget *button;
  GtkWidget *entry;
  GtkWidget *hbox;

  frame = gtk_frame_new (_("Hotkey for switching between layouts"));
  hbox  = gtk_hbox_new (TRUE, 0);
  
  gtk_container_add (GTK_CONTAINER (frame), hbox);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 10);

  entry = gtk_entry_new ();
  gtk_entry_set_text (GTK_ENTRY(entry), gkb->key);

  button = gtk_button_new_with_label (_("Grab hotkey"));

  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), entry, FALSE, TRUE, 0);

  gtk_signal_connect (GTK_OBJECT (entry), "changed", 
                      GTK_SIGNAL_FUNC (changed_cb),
		      NULL);
  gtk_signal_connect (GTK_OBJECT (button), "clicked", 
                      GTK_SIGNAL_FUNC (grab_button_pressed),
		      entry);

  return frame;
}

static void
prophelp_cb (AppletWidget * applet, gpointer data)
{
  GnomeHelpMenuEntry help_entry =
    { "gkb_applet", "index.html#GKBAPPLET-PREFS" };

  gnome_help_display (NULL, &help_entry);
}


static GtkWidget *
gkb_prop_create_property_box (GkbPropertyBoxInfo *pbi)
{
  GtkWidget *propbox;
  GtkWidget *propnotebook;
  GtkWidget *display_frame;
  GtkWidget *hotkey_frame;
  GtkWidget *buttons_vbox;
  GtkWidget *page_1_vbox;
  GtkWidget *page_2_hbox;
  GtkWidget *page;
  GtkWidget *scrolled_window;
  GtkWidget *page_1_label;
  GtkWidget *page_2_label;

  /* Create property box */
  propbox = gnome_property_box_new ();
  gkb->propbox = propbox;

  gtk_object_set_data (GTK_OBJECT (propbox), "propbox", propbox);
  gtk_window_set_title (GTK_WINDOW (propbox), _("GKB Properties"));
  gtk_window_set_position (GTK_WINDOW (propbox), GTK_WIN_POS_CENTER);
  gtk_window_set_policy (GTK_WINDOW (propbox), FALSE, FALSE, FALSE);

  propnotebook = GNOME_PROPERTY_BOX (propbox)->notebook;
  gtk_widget_show (propnotebook);

  /* Add page 1 */
  page_1_vbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (page_1_vbox);
  gtk_container_add (GTK_CONTAINER (propnotebook), page_1_vbox);
  page_1_label = gtk_label_new (_("General"));
  page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (propnotebook), 0);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (propnotebook), page, page_1_label);

  /* Page 1 Frames */
  display_frame = gkb_prop_create_display_frame ();
  hotkey_frame  = gkb_prop_create_hotkey_frame ();
  gtk_box_pack_start (GTK_BOX (page_1_vbox), display_frame, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (page_1_vbox), hotkey_frame, TRUE, FALSE, 2);

  /* Add page 2 */
  page_2_hbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (page_2_hbox);
  gtk_container_add (GTK_CONTAINER (propnotebook), page_2_hbox);
  page_2_label = gtk_label_new (_("Keymaps"));
  page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (propnotebook), 1);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (propnotebook), page, page_2_label);

  /* Page 2 Frame */
  scrolled_window = gkb_prop_create_scrolled_window (pbi);
  buttons_vbox    = gkb_prop_create_buttons_vbox (pbi);
  gtk_box_pack_start (GTK_BOX (page_2_hbox), scrolled_window, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (page_2_hbox), buttons_vbox, FALSE, TRUE, 0);

  /* Connect the signals */
  gtk_signal_connect (GTK_OBJECT (propbox), "destroy",
		      GTK_SIGNAL_FUNC (gtk_widget_destroyed), &gkb->propbox);
  gtk_signal_connect (GTK_OBJECT (propbox), "help",
		      GTK_SIGNAL_FUNC (prophelp_cb), NULL);
  
  return propbox;
}

void
properties_dialog (AppletWidget * applet)
{
  GkbPropertyBoxInfo *pbi;
  
  if (gkb->propbox)
    {
	gtk_widget_destroy (gkb->propbox);
	gkb->propbox= NULL;
    }

#if 0	/* FIXME now */
  gkb_prop_list_free_keymaps (pbi);
#endif	

  
  pbi = g_new0 (GkbPropertyBoxInfo, 1);
  pbi->add_button    = NULL;
  pbi->edit_button   = NULL;
  pbi->delete_button = NULL;
  pbi->up_button     = NULL;
  pbi->down_button   = NULL;
  pbi->selected_keymap = NULL;
  
  gkb->tn = gkb->n;
  gkb->propbox = gkb_prop_create_property_box (pbi);
  
  pbi->box    = gkb->propbox;
  
  gtk_widget_show_all (pbi->box);
  gdk_window_raise    (pbi->box->window);

  return;
}

