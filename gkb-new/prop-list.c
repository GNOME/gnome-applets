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

#include <sys/stat.h>

#include "gkb.h"

#define GKB_KEYMAP_TAG  "GkbKeymapTag"

/**
 * gkb_util_get_pixmap_name:
 * @keymap: 
 * 
 * return a newly allocated string to the path of the flag for a given keymap
 * 
 * Return Value: 
 **/
static gchar *
gkb_util_get_pixmap_name (GkbKeymap * keymap)
{
  struct stat tempbuf;
  gchar *pixmap_name_pre;
  gchar *pixmap_name;

  pixmap_name_pre = g_strdup_printf ("gkb/%s", keymap->flag);
  pixmap_name = gnome_unconditional_pixmap_file (pixmap_name_pre);
  if (stat (pixmap_name, &tempbuf))
    {
      g_free (pixmap_name);
      pixmap_name = gnome_unconditional_pixmap_file ("gkb/gkb-foot.png");
    }
  g_free (pixmap_name_pre);

  return pixmap_name;
}

/**
 * gkb_prop_list_create_item:
 * @keymap: 
 * 
 * Create a list item from a keymap
 * 
 * Return Value: 
 **/
static GtkWidget *
gkb_prop_list_create_item (GkbKeymap * keymap)
{
  GtkWidget *list_item;
  GtkWidget *label;
  GtkWidget *hbox;
  GtkWidget *pixmap;
  gchar *pixmap_name;

  hbox = gtk_hbox_new (FALSE, 0);

  /* Pixmap */
  pixmap = gtk_type_new (gnome_pixmap_get_type ());
  pixmap_name = gkb_util_get_pixmap_name (keymap);
  gnome_pixmap_load_file_at_size (GNOME_PIXMAP (pixmap), pixmap_name, 28, 20);
  g_free (pixmap_name);
  gtk_box_pack_start (GTK_BOX (hbox), pixmap, FALSE, TRUE, 0);

  /* Label */
  label = gtk_label_new (keymap->name);
  gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label), 3, 0);

  /* List item */
  list_item = gtk_list_item_new ();
  gtk_container_add (GTK_CONTAINER (list_item), hbox);
  gtk_object_set_data (GTK_OBJECT (list_item), GKB_KEYMAP_TAG, keymap);

  return list_item;
}


/**
 * gkb_prop_list_reload:
 * @pbi: 
 * 
 * Reloads the items from pbi->keymaps into the widget. Call after
 * any change is made to the list, (add, remove, move, delete etc)
 **/
void
gkb_prop_list_reload (GkbPropertyBoxInfo * pbi)
{
  GkbKeymap *keymap;
  GkbKeymap *selected_keymap;
  GtkWidget *item;
  GtkWidget *selected_child = NULL;
  GList *list;

  g_return_if_fail (pbi != NULL);

  selected_keymap = pbi->selected_keymap;

  gtk_list_clear_items (pbi->list, 0, -1);

  list = pbi->keymaps;
  for (; list != NULL; list = list->next)
    {
      keymap = (GkbKeymap *) list->data;
      item = gkb_prop_list_create_item (keymap);
      gtk_container_add (GTK_CONTAINER (pbi->list), item);
      if (keymap == selected_keymap)
	selected_child = item;
    }

  if (selected_child)
    gtk_list_select_child (pbi->list, selected_child);

  gtk_widget_show_all (GTK_WIDGET (pbi->list));
}


/**
 * gkb_prop_list_delete_clicked:
 * @pbi: 
 * 
 * Deletes the selected item from the gtk_list
 **/
static void
gkb_prop_list_delete_clicked (GkbPropertyBoxInfo * pbi)
{
  g_return_if_fail (pbi != NULL);
  g_return_if_fail (GTK_IS_WIDGET (pbi->add_button));

  if (!pbi->selected_keymap)
    {
      g_warning ("Why is the DELETE button sensitive ???");
      return;
    }

  pbi->keymaps = g_list_remove (pbi->keymaps, pbi->selected_keymap);
  gkb_keymap_free (pbi->selected_keymap);
  pbi->selected_keymap = NULL;

  gkb_prop_list_reload (pbi);

  gnome_property_box_changed (GNOME_PROPERTY_BOX (pbi->box));

  return;
}


/**
 * gkb_util_g_list_swap:
 * @item1: 
 * @item2: 
 * 
 * Swap the data of two list items
 **/
static inline void
gkb_util_g_list_swap (GList * item1, GList * item2)
{
  gpointer temp;

  temp = item1->data;
  item1->data = item2->data;
  item2->data = temp;
}

/**
 * gkb_prop_list_up_down_clicked:
 * @pbi: 
 * @up: 
 * 
 * 
 **/
static void
gkb_prop_list_up_down_clicked (GkbPropertyBoxInfo * pbi, gboolean up)
{
  GList *keymap_node;

  g_return_if_fail (pbi != NULL);
  g_return_if_fail (GTK_IS_WIDGET (pbi->add_button));

  if (!pbi->selected_keymap)
    {
      g_warning ("Why is the UP/DOWN button sensitive ???");
      return;
    }

  keymap_node = g_list_find (pbi->keymaps, pbi->selected_keymap);

  if (up)
    gkb_util_g_list_swap (keymap_node->prev, keymap_node);
  else
    gkb_util_g_list_swap (keymap_node, keymap_node->next);

  gkb_prop_list_reload (pbi);

  gnome_property_box_changed (GNOME_PROPERTY_BOX (pbi->box));

  return;
}

/**
 * gkb_prop_list_update_sensitivity:
 * @pbi: 
 * 
 * Updates the buttons sensitivity, it is called after the selection
 * is changed or an action is taken.
 **/
static void
gkb_prop_list_update_sensitivity (GkbPropertyBoxInfo * pbi)
{
  gboolean row_selected = FALSE;
  gboolean is_top = FALSE;
  gboolean is_bottom = FALSE;

  g_return_if_fail (pbi != NULL);
  g_return_if_fail (GTK_IS_WIDGET (pbi->add_button));

  if (pbi->selected_keymap)
    {
      GList *list_item;

      row_selected = TRUE;

      list_item = g_list_find (pbi->keymaps, pbi->selected_keymap);
      g_return_if_fail (list_item);

      if (list_item->next == NULL)
	is_bottom = TRUE;

      if (list_item->prev == NULL)
	is_top = TRUE;
    }

  gtk_widget_set_sensitive (pbi->add_button, TRUE);
  gtk_widget_set_sensitive (pbi->edit_button, row_selected);
  gtk_widget_set_sensitive (pbi->up_button, row_selected && !is_top);
  gtk_widget_set_sensitive (pbi->down_button, row_selected && !is_bottom);
  gtk_widget_set_sensitive (pbi->delete_button, row_selected
			    && !(is_bottom && is_top));

}

/**
 * gkb_prop_list_selection_changed:
 * @list: 
 * @pbi: 
 * 
 * The list's selection has changed. The main purpose it to set pbi->selected_keymap
 **/
static void
gkb_prop_list_selection_changed (GtkWidget * list, GkbPropertyBoxInfo * pbi)
{
  GList *selection;

  g_return_if_fail (GTK_IS_WIDGET (list));
  g_return_if_fail (pbi != NULL);
  g_return_if_fail (GTK_IS_WIDGET (pbi->add_button));

  selection = GTK_LIST (pbi->list)->selection;

  if (selection)
    {
      GkbKeymap *keymap;
      GtkListItem *list_item;

      list_item = GTK_LIST_ITEM (selection->data);
      keymap = gtk_object_get_data (GTK_OBJECT (list_item), GKB_KEYMAP_TAG);
      g_return_if_fail (keymap != NULL);
      pbi->selected_keymap = keymap;
      gkb->keymap = keymap->parent;
      gkb_update (gkb, TRUE);
    }
  else
    {
      pbi->selected_keymap = NULL;
    }

  gkb_prop_list_update_sensitivity (pbi);

  return;
}

/**
 * gkb_prop_list_button_clicked_cb:
 * @button: 
 * @pbi: 
 * 
 * Takes care of executing the appropiate action when a button is pressed
 **/
static void
gkb_prop_list_button_clicked_cb (GtkWidget * button, GkbPropertyBoxInfo * pbi)
{
  if (button == pbi->add_button)
    gkb_prop_map_add (pbi);
  else if (button == pbi->edit_button)
    gkb_prop_map_edit (pbi);
  else if (button == pbi->delete_button)
    gkb_prop_list_delete_clicked (pbi);
  else if (button == pbi->up_button)
    gkb_prop_list_up_down_clicked (pbi, TRUE);
  else if (button == pbi->down_button)
    gkb_prop_list_up_down_clicked (pbi, FALSE);

}

/**
 * gkb_prop_list_create_button:
 * @name: 
 * @pbi: 
 * 
 * Function to create a button for the keymaps list
 * 
 * Return Value: 
 **/
static GtkWidget *
gkb_prop_list_create_button (const gchar * name, GkbPropertyBoxInfo * pbi)
{
  GtkWidget *button;

  button = gtk_button_new_with_label (name);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (gkb_prop_list_button_clicked_cb), pbi);
  gtk_container_add (GTK_CONTAINER (pbi->buttons_vbox), button);
  /* FIXME, we don't want to flag the GTK_CAN_DEFAULT but if
   * we don't the widgets look ugly. This a hacky solution for
   * a cosmetic problem. */
  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);

  return button;
}

/**
 * gkb_prop_create_buttons_vbox:
 * @pbi: 
 * 
 * Creates the buttons vbox for the list of keymaps installed
 * 
 * Return Value: 
 **/
GtkWidget *
gkb_prop_create_buttons_vbox (GkbPropertyBoxInfo * pbi)
{
  GtkWidget *vbox;

  vbox = gtk_vbutton_box_new ();
  gtk_button_box_set_spacing (GTK_BUTTON_BOX (vbox), 0);
  gtk_button_box_set_child_size (GTK_BUTTON_BOX (vbox), 75, 25);
  gtk_button_box_set_child_ipadding (GTK_BUTTON_BOX (vbox), 2, 0);

  pbi->buttons_vbox = vbox;
  pbi->add_button = gkb_prop_list_create_button (_("Add"), pbi);
  pbi->edit_button = gkb_prop_list_create_button (_("Edit"), pbi);
  pbi->up_button = gkb_prop_list_create_button (_("Up"), pbi);
  pbi->down_button = gkb_prop_list_create_button (_("Down"), pbi);
  pbi->delete_button = gkb_prop_list_create_button (_("Delete"), pbi);

  gkb_prop_list_update_sensitivity (pbi);

  return vbox;
}


/**
 * gkb_prop_list_load_keymaps:
 * @void: 
 * 
 * Load the keymaps into the PropertyBoxInfo struct
 **/
static void
gkb_prop_list_load_keymaps (GkbPropertyBoxInfo * pbi)
{
  if (pbi->keymaps)
    {
      g_warning ("Dude ! you forgot to free the keymaps list somewhere.");
      gkb_keymap_free_list (pbi->keymaps);
    }

  pbi->keymaps = gkb_keymap_copy_list (gkb->maps);

  gkb_prop_list_reload (pbi);
}

/**
 * gkb_prop_create_scrolled_window:
 * @void: 
 * 
 * Creates the scrolled window where the currently installed keymaps
 * live
 * 
 * Return Value: 
 **/
GtkWidget *
gkb_prop_create_scrolled_window (GkbPropertyBoxInfo * pbi)
{
  GtkWidget *scrolled_window;

  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  pbi->list = GTK_LIST (gtk_list_new ());
  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW
					 (scrolled_window),
					 GTK_WIDGET (pbi->list));
  gkb_prop_list_load_keymaps (pbi);

  gtk_signal_connect (GTK_OBJECT (pbi->list),
		      "selection_changed",
		      GTK_SIGNAL_FUNC (gkb_prop_list_selection_changed), pbi);

  return scrolled_window;
}
