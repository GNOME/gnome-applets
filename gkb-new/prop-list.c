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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/stat.h>

#include "gkb.h"

#define GKB_KEYMAP_TAG  "GkbKeymapTag"

extern gboolean gail_loaded;

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
 * @treeview 
 * Create a list item from a keymap
 * 
 * Return Value: 
 **/
static void
gkb_prop_list_create_item (GkbKeymap * keymap, GtkTreeView *treeview, GtkTreeIter *iter)
{
  GdkPixbuf  *pixbuf;
  gchar *pixmap_name;
  GtkTreeModel *store;	

  store = gtk_tree_view_get_model (treeview);  
  /* PixBuf */
  pixmap_name = gkb_util_get_pixmap_name (keymap);
  pixbuf = gdk_pixbuf_scale_simple  (gdk_pixbuf_new_from_file 
                                               (pixmap_name, NULL),
                                               28,
                                               20,
                                               GDK_INTERP_HYPER);
   
  gtk_list_store_append (GTK_LIST_STORE(store), iter);
  gtk_list_store_set (GTK_LIST_STORE(store), iter, 0, pixbuf, 1, keymap->name, 2, keymap, -1); 	
  g_free (pixmap_name);
  g_object_unref (G_OBJECT (pixbuf));
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
  GtkTreeIter item;
  GtkTreeIter *selected_child = NULL;
  GList *list;

  g_return_if_fail (pbi != NULL);
  
  selected_keymap = pbi->selected_keymap;
  
  gtk_list_store_clear (GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(pbi->list))));

  list = pbi->keymaps;
  for (; list != NULL; list = list->next)
    {
      keymap = (GkbKeymap *) list->data;
      gkb_prop_list_create_item (keymap, pbi->list, &item);
      if (keymap == selected_keymap) {
	selected_child = gtk_tree_iter_copy (&item);
      }
    }

  if (selected_child) {
      GtkTreePath *path;
      GtkTreeSelection *selection;
  	
      path = gtk_tree_model_get_path (gtk_tree_view_get_model(GTK_TREE_VIEW(pbi->list)),
      				      selected_child);
      if (path) {
        gtk_tree_view_set_cursor (GTK_TREE_VIEW(pbi->list), path, NULL, FALSE);
        selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(pbi->list));
        gtk_tree_selection_select_path (selection, path);
      } 

      gtk_tree_path_free (path);
      gtk_tree_iter_free (selected_child);
  }

  gtk_widget_grab_focus (GTK_WIDGET (pbi->list));

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
  gkb_apply(pbi);
  gkb_save_session (pbi->gkb);  
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
 * Property box Up or Down button clicked
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
  
  /* FIXME 

  pbi->selected_keymap = NULL;

  */

  gkb_prop_list_reload (pbi);
  gkb_apply(pbi);
  gkb_save_session(pbi->gkb);
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
 * @Selection: 
 * @data: 
 * 
 * The list's selection has changed. The main purpose it to set pbi->selected_keymap
 **/

static void
gkb_prop_list_selection_changed  (GtkTreeSelection *selection, gpointer data)

{
  GkbPropertyBoxInfo * pbi = data;
  GtkTreeView *treeview; 
  GkbKeymap *keymap;
  GtkTreeIter iter;
  GtkTreeModel *model;
  g_return_if_fail (pbi != NULL);
  g_return_if_fail (GTK_IS_WIDGET (pbi->add_button));
  treeview = gtk_tree_selection_get_tree_view (selection);
  if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
        gtk_tree_model_get (model, &iter, 2, &keymap, -1);
        if (keymap != NULL)
                pbi->selected_keymap = keymap;
        else

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
   else
  if (button == pbi->edit_button) {
    gkb_prop_map_edit (pbi); 
    gkb_prop_list_reload (pbi);
    }
  else if (button == pbi->delete_button)
    gkb_prop_list_delete_clicked (pbi);
  else if (button == pbi->up_button)
    gkb_prop_list_up_down_clicked (pbi, TRUE);
  else if (button == pbi->down_button)
    gkb_prop_list_up_down_clicked (pbi, FALSE);

  gkb_prop_list_update_sensitivity(pbi);

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

  button = gtk_button_new_from_stock (name);
  g_signal_connect (button, "clicked",
		      G_CALLBACK (gkb_prop_list_button_clicked_cb), pbi);
  gtk_container_add (GTK_CONTAINER (pbi->buttons_vbox), button);
  /* FIXME, we don't want to flag the GTK_CAN_DEFAULT but if
   * we don't the widgets look ugly. This a hacky solution for
   * a cosmetic problem. 
  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
*/
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
  gtk_button_box_set_spacing (GTK_BUTTON_BOX (vbox), 6);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (vbox), GTK_BUTTONBOX_START);
      
  pbi->buttons_vbox = vbox;
  pbi->add_button = gkb_prop_list_create_button ("gtk-add", pbi);
  pbi->edit_button = gkb_prop_list_create_button ("gtk-properties", pbi);
  pbi->up_button = gkb_prop_list_create_button ("gtk-go-up", pbi);
  pbi->down_button = gkb_prop_list_create_button ("gtk-go-down", pbi);
  pbi->delete_button = gkb_prop_list_create_button ("gtk-delete", pbi);

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
  GKB *gkb = pbi->gkb;
  if (pbi->keymaps)
    {
      g_warning ("Dude ! you forgot to free the keymaps list somewhere.");
      gkb_keymap_free_list (pbi->keymaps);
    }

  pbi->keymaps = gkb_keymap_copy_list (gkb->maps);

  gkb_prop_list_reload (pbi);
  gtk_widget_show (GTK_WIDGET(pbi->list));
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
gkb_prop_create_scrolled_window (GkbPropertyBoxInfo * pbi, GtkWidget *label)
{
  GtkWidget *scrolled_window;
  GtkTreeModel *model;
  GtkWidget    *treeview;
  GtkCellRenderer *cell_renderer;
  GtkTreeViewColumn *column;
  GtkListStore *store;
  scrolled_window = gtk_scrolled_window_new (NULL, NULL);

  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  store = gtk_list_store_new (3, GDK_TYPE_PIXBUF, G_TYPE_STRING,G_TYPE_POINTER);	
  model = GTK_TREE_MODEL(store);	
  treeview = gtk_tree_view_new_with_model (model);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), treeview);
  GTK_WIDGET_SET_FLAGS (treeview, GTK_CAN_DEFAULT);
    
  pbi->list = GTK_TREE_VIEW (treeview);
  
  column = gtk_tree_view_column_new ();
  
  cell_renderer = gtk_cell_renderer_pixbuf_new  ();
  gtk_tree_view_column_pack_start (column, cell_renderer, FALSE);
  gtk_tree_view_column_set_attributes (column, cell_renderer,
				       "pixbuf", 0, NULL);
  
  cell_renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (column, cell_renderer, FALSE);
  gtk_tree_view_column_set_attributes (column, cell_renderer,
				       "text", 1, NULL);
   
  gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
  
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview), FALSE);			
  gtk_tree_selection_set_mode ((GtkTreeSelection *) gtk_tree_view_get_selection
			(GTK_TREE_VIEW (treeview)),
			GTK_SELECTION_SINGLE);
  g_signal_connect (G_OBJECT (gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview))),
                          "changed",
                          G_CALLBACK (gkb_prop_list_selection_changed), pbi);

  g_object_unref (G_OBJECT (model));
  gtk_container_add (GTK_CONTAINER (scrolled_window), GTK_WIDGET(treeview));	


  gtk_widget_show (GTK_WIDGET(treeview));
  gkb_prop_list_load_keymaps (pbi);
  return scrolled_window;
}
