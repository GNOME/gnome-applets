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
 * This program is free software; yo!@@u can redistribute it and/or
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
#include "gkb.h"


#define GKB_MENU_ITEM_TEXT "GtkMenuItemText"


typedef struct _KeymapData KeymapData;
struct _KeymapData
{
  GtkWidget *widget;
  GkbKeymap *preset;
};

static void
changed_cb (GtkEntry * entry, GnomePropertyBox * pb)
{
  gnome_property_box_changed (GNOME_PROPERTY_BOX (pb));
}







/**
 * gkb_prop_apply_clicked:
 * @pb: 
 * @page: 
 * @pbi: 
 * 
 * Apply settings.
 **/
static void
gkb_prop_apply_clicked (GtkWidget * pb, gint page, GkbPropertyBoxInfo * pbi)
{
  static guint key = 0;

  if (page != -1)
    return;

  /* Swap the lists of keymaps */
  gkb_keymap_free_list (gkb->maps);
  gkb->maps = gkb_keymap_copy_list (pbi->keymaps);

  /* recalculate new props */
  gkb->n = g_list_length (gkb->maps);
  gkb->cur = 0;
  gkb->keymap = g_list_nth_data (gkb->maps, 0);

  /* misc props from pbi */
  gkb->is_small = pbi->is_small;
  gkb->mode = pbi->mode;

  key = XKeysymToKeycode (GDK_DISPLAY (), gkb->keysym);

  XUngrabKey (GDK_DISPLAY (), key, gkb->state, GDK_ROOT_WINDOW ());

  gkb->key = g_strdup (gtk_entry_get_text (GTK_ENTRY (pbi->hotkey_entry)));
  convert_string_to_keysym_state (gkb->key, &gkb->keysym, &gkb->state);

  key = XKeysymToKeycode (GDK_DISPLAY (), gkb->keysym);

  XGrabKey (GDK_DISPLAY (), key, gkb->state,
	    GDK_ROOT_WINDOW (), True, GrabModeAsync, GrabModeAsync);

  applet_widget_sync_config (APPLET_WIDGET (gkb->applet));

  /* Render & update */
  gkb_sized_render (gkb);
  gkb_update (gkb, TRUE);

  /* We need keymap->parent to be valid, reload list */
  gkb_keymap_free_list (pbi->keymaps);
  pbi->keymaps = gkb_keymap_copy_list (gkb->maps);
  gkb_prop_list_reload (pbi);

}


/**
 * gkb_prop_map_label_at:
 * @table: 
 * @row: 
 * @col: 
 * @label_text: 
 * 
 * Add a label at a specified location
 **/
static void
gkb_prop_label_at (GtkWidget * table, gint row, gint col,
		   const gchar * label_text)
{
  GtkWidget *label;

  label = gtk_label_new (label_text);
  gtk_table_attach (GTK_TABLE (table), label,
		    row, row + 1, col, col + 1,
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_padding (GTK_MISC (label), 5, 0);
  gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);
}


/**
 * gkb_prop_get_sizes:
 * @void: 
 * 
 * Get the sizes for the Size option menu
 * 
 * Return Value: 
 **/
static GList *
gkb_prop_get_sizes (void)
{
  GList *list = NULL;

  list = g_list_prepend (list, _("Normal"));
  list = g_list_prepend (list, _("Big"));

  return g_list_reverse (list);
}

/**
 * gkb_prop_get_mode:
 * @void: 
 *
 *
 * 
 * Return Value: 
 **/
static GList *
gkb_prop_get_mode (void)
{
  GList *list = NULL;

  list = g_list_prepend (list, _("Flag"));
  list = g_list_prepend (list, _("Label"));
  list = g_list_prepend (list, _("Flag and Label"));

  return g_list_reverse (list);
}


/**
 * gkb_prop_mode_changed:
 * @menu_item: 
 * @pbi: 
 * 
 * 
 **/
static void
gkb_prop_mode_changed (GtkWidget * menu_item, GkbPropertyBoxInfo * pbi)
{
  gchar *text;

  text = gtk_object_get_data (GTK_OBJECT (menu_item), GKB_MENU_ITEM_TEXT);

  g_return_if_fail (text != NULL);

  pbi->mode = gkb_util_get_mode_from_text (text);

  gnome_property_box_changed (GNOME_PROPERTY_BOX (pbi->box));
}

/**
 * gkb_prop_size_changed:
 * @menu_item: 
 * @pbi: 
 * 
 * 
 **/
static void
gkb_prop_size_changed (GtkWidget * menu_item, GkbPropertyBoxInfo * pbi)
{
  gchar *text;

  text = gtk_object_get_data (GTK_OBJECT (menu_item), GKB_MENU_ITEM_TEXT);

  g_return_if_fail (text != NULL);

  if (strcmp (text, _("Normal")) == 0)
    {
      pbi->is_small = TRUE;
    }
  else if (strcmp (text, _("Big")) == 0)
    {
      pbi->is_small = FALSE;
    }
  else
    {
      g_warning ("Could not interpret size change [%s]\n", text);
    }

  gnome_property_box_changed (GNOME_PROPERTY_BOX (pbi->box));
}


/**
 * gkb_prop_option_menu_at:
 * @table: 
 * @row: 
 * @col: 
 * @list_in: 
 * @initval:
 * 
 * Create an option menu from the items in list and attach it to table
 **/
static void
gkb_prop_option_menu_at (GtkWidget * table, gint row, gint col,
			 GList * list_in, GtkSignalFunc function,
			 GkbPropertyBoxInfo * pbi, gint initval)
{
  GtkWidget *option_menu;
  GtkWidget *menu_item;
  GtkWidget *menu;
  GList *list;

  option_menu = gtk_option_menu_new ();

  menu = gtk_menu_new ();
  list = list_in;
  for (; list != NULL; list = list->next)
    {
      menu_item = gtk_menu_item_new_with_label ((gchar *) list->data);
      gtk_widget_show (menu_item);
      gtk_menu_append (GTK_MENU (menu), menu_item);
      gtk_signal_connect (GTK_OBJECT (menu_item), "activate", function, pbi);
      gtk_object_set_data (GTK_OBJECT (menu_item), GKB_MENU_ITEM_TEXT,
			   list->data);
    }

  gtk_option_menu_set_menu (GTK_OPTION_MENU (option_menu), menu);
  gtk_option_menu_set_history (GTK_OPTION_MENU (option_menu), initval);

  gtk_table_attach (GTK_TABLE (table), option_menu,
		    row, row + 1, col, col + 1,
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_EXPAND), 0, 0);

}


/**
 * gkb_prop_create_display_frame:
 * @GkbPropertyBoxInfo * pbi: PropBox Information
 * 
 * Implement the display frame
 * 
 * Return Value: the display frame widget
 **/
static GtkWidget *
gkb_prop_create_display_frame (GkbPropertyBoxInfo * pbi)
{
  GList *mode;
  GList *sizes;

  GtkWidget *frame;
  gchar *pixmap_filename;
  GtkWidget *hbox;
  GtkWidget *pixmap;
  GtkWidget *table;

  frame = gtk_frame_new (_("Display"));

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (frame), hbox);

  /* Load the pixmap */
  pixmap = gtk_type_new (gnome_pixmap_get_type ());
  pixmap_filename = gnome_pixmap_file ("gkb.png");
  if (pixmap_filename)
    gnome_pixmap_load_file (GNOME_PIXMAP (pixmap), pixmap_filename);
  g_free (pixmap_filename);
  gtk_box_pack_start (GTK_BOX (hbox), pixmap, FALSE, FALSE, 23);

  /* Create the table */
  table = gtk_table_new (2, 2, TRUE);
  gtk_box_pack_start (GTK_BOX (hbox), table, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table), 15);

  /* Labels */
  gkb_prop_label_at (table, 0, 0, _("Appearance "));
  gkb_prop_label_at (table, 0, 1, _("Applet size "));

  /* Option menus */
  mode = gkb_prop_get_mode ();
  gkb_prop_option_menu_at (table, 1, 0, mode,
			   gkb_prop_mode_changed, pbi,
			   gkb_util_get_int_from_mode (pbi->mode));
  g_list_free (mode);

  sizes = gkb_prop_get_sizes ();
  gkb_prop_option_menu_at (table, 1, 1, sizes,
			   gkb_prop_size_changed, pbi, pbi->is_small ? 0 : 1);
  g_list_free (sizes);

  return frame;
}


/**
 * gkb_prop_create_hotkey_frame:
 * @GkbProbertyBoxInfo * pbi: Propbox info
 * 
 * Implement the hotkey properties frame
 * 
 * Return Value: the hotkey frame widget
 **/
static GtkWidget *
gkb_prop_create_hotkey_frame (GkbPropertyBoxInfo * pbi, GnomePropertyBox * pb)
{
  GtkWidget *frame;
  GtkWidget *button;
  GtkWidget *hbox;

  frame = gtk_frame_new (_("Hotkey for switching between layouts"));
  hbox = gtk_hbox_new (TRUE, 0);

  gtk_container_add (GTK_CONTAINER (frame), hbox);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 10);

  if (pbi->hotkey_entry)
    gtk_widget_destroy (pbi->hotkey_entry);

  pbi->hotkey_entry = gtk_entry_new ();
  gtk_entry_set_text (GTK_ENTRY (pbi->hotkey_entry), gkb->key);

  button = gtk_button_new_with_label (_("Grab hotkey"));

  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), pbi->hotkey_entry, FALSE, TRUE, 0);

  gtk_signal_connect (GTK_OBJECT (pbi->hotkey_entry), "changed",
		      GTK_SIGNAL_FUNC (changed_cb), pb);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (grab_button_pressed),
		      pbi->hotkey_entry);

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
gkb_prop_create_property_box (GkbPropertyBoxInfo * pbi)
{
  GtkWidget *propbox;
  GtkWidget *propnotebook;
  GtkWidget *display_frame;
  GtkWidget *hotkey_frame;
  GtkWidget *buttons_vbox;
  GtkWidget *page_1_hbox;
  GtkWidget *page_2_vbox;
  GtkWidget *page;
  GtkWidget *scrolled_window;
  GtkWidget *page_1_label;
  GtkWidget *page_2_label;

  /* Create property box */
  propbox = gnome_property_box_new ();

  gtk_object_set_data (GTK_OBJECT (propbox), "propbox", propbox);
  gtk_window_set_title (GTK_WINDOW (propbox), _("GKB Properties"));
  gtk_window_set_position (GTK_WINDOW (propbox), GTK_WIN_POS_CENTER);
  gtk_window_set_policy (GTK_WINDOW (propbox), FALSE, FALSE, FALSE);

  propnotebook = GNOME_PROPERTY_BOX (propbox)->notebook;
  gtk_widget_show (propnotebook);

  /* Add page 1 */
  page_1_hbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (page_1_hbox);
  gtk_container_add (GTK_CONTAINER (propnotebook), page_1_hbox);
  page_1_label = gtk_label_new (_("Keymaps"));
  page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (propnotebook), 0);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (propnotebook), page,
			      page_1_label);

  /* Page 1 Frame */
  scrolled_window = gkb_prop_create_scrolled_window (pbi);
  buttons_vbox = gkb_prop_create_buttons_vbox (pbi);
  gtk_box_pack_start (GTK_BOX (page_1_hbox), scrolled_window, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (page_1_hbox), buttons_vbox, FALSE, TRUE, 0);

  /* Add page 2 */
  page_2_vbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (page_2_vbox);
  gtk_container_add (GTK_CONTAINER (propnotebook), page_2_vbox);
  page_2_label = gtk_label_new (_("Options"));
  page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (propnotebook), 1);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (propnotebook), page,
			      page_2_label);

  /* Page 2 Frames */
  display_frame = gkb_prop_create_display_frame (pbi);
  hotkey_frame =
    gkb_prop_create_hotkey_frame (pbi, GNOME_PROPERTY_BOX (propbox));
  gtk_box_pack_start (GTK_BOX (page_2_vbox), display_frame, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (page_2_vbox), hotkey_frame, TRUE, FALSE, 2);

  /* Connect the signals */
  gtk_signal_connect (GTK_OBJECT (propbox), "apply",
		      GTK_SIGNAL_FUNC (gkb_prop_apply_clicked), pbi);
  gtk_signal_connect (GTK_OBJECT (propbox), "help",
		      GTK_SIGNAL_FUNC (prophelp_cb), NULL);

  return propbox;
}

static gint
gkb_prop_box_destroy (GtkWidget * box, GkbPropertyBoxInfo * pbi)
{
  g_return_val_if_fail (pbi != NULL, TRUE);
  g_return_val_if_fail (box == pbi->box, TRUE);

  gtk_widget_destroy (box);
  /* This is hackish, but I could not find the correct
   * solution for it. The problem is that when we destroy
   * the list, the selection changes if there is a selected
   * row, so we need to clear the selection so that the
   * gkb_prop_list_selection_changed function is not reached.
   * if you know of a way to solve this, enlighten me. Chema */
  gtk_list_clear_items (pbi->list, 0, -1);
  g_free (pbi);

  return FALSE;
}

void
properties_dialog (AppletWidget * applet)
{
  GkbPropertyBoxInfo *pbi;

  pbi = g_new0 (GkbPropertyBoxInfo, 1);
  pbi->gkb = gkb;
  pbi->mode = gkb->mode;
  pbi->is_small = gkb->is_small;
  pbi->add_button = NULL;
  pbi->edit_button = NULL;
  pbi->delete_button = NULL;
  pbi->up_button = NULL;
  pbi->down_button = NULL;
  pbi->hotkey_entry = NULL;
  pbi->selected_keymap = NULL;

  pbi->box = gkb_prop_create_property_box (pbi);

  gtk_signal_connect (GTK_OBJECT (pbi->box), "destroy",
		      GTK_SIGNAL_FUNC (gkb_prop_box_destroy), pbi);

  gtk_widget_show_all (pbi->box);
  gdk_window_raise (pbi->box->window);

  return;
}
