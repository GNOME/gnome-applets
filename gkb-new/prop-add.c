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

#include "gkb.h"

static void addhelp_cb (AppletWidget * widget, gpointer data);

typedef struct _LangData LangData;
struct _LangData
{
  GtkWidget *widget;
  GHashTable *countries;
};

typedef struct _CountryData CountryData;
struct _CountryData
{
  GtkWidget *widget;
  GList *keymaps;
};




static void
treeitems_create (GtkWidget * tree)
{
  GList *sets = NULL;
  GList *retval = NULL;
  GtkWidget *sitem, *titem, *subitem, *subtree, *subtree2;

  GHashTable *langs = g_hash_table_new (g_str_hash, g_str_equal);
  LangData *ldata;
  CountryData *cdata;


  /* TODO: Error checking... */
  sets = gkb_preset_load (find_presets ());
  retval = sets;

  while ((sets = g_list_next (sets)) != NULL)
    {
      GkbKeymap *item;

      item = sets->data;

      if ((ldata = g_hash_table_lookup (langs, item->lang)) != NULL)
	{
	  /* There is lang */
	  if ((cdata = g_hash_table_lookup (ldata->countries, item->country))
	      != NULL)
	    {
	      /* There is country */
	      sitem = gtk_tree_item_new_with_label (item->name);
	      gtk_tree_append (GTK_TREE (cdata->widget), sitem);
	      gtk_widget_show (sitem);

	      cdata->keymaps = g_list_append (cdata->keymaps, item);

	      gtk_object_set_data (GTK_OBJECT (sitem), "item", item);

	    }
	  else
	    {
	      /* There is no country */

	      subitem = gtk_tree_item_new_with_label (item->country);
	      gtk_tree_append (GTK_TREE (ldata->widget), subitem);
	      gtk_widget_show (subitem);

	      subtree = gtk_tree_new ();
	      gtk_tree_set_selection_mode (GTK_TREE (subtree),
					   GTK_SELECTION_SINGLE);
	      gtk_tree_set_view_mode (GTK_TREE (subtree), GTK_TREE_VIEW_ITEM);
	      gtk_tree_item_set_subtree (GTK_TREE_ITEM (subitem), subtree);
	      subitem = gtk_tree_item_new_with_label (item->name);
	      gtk_tree_append (GTK_TREE (subtree), subitem);
	      gtk_widget_show (subitem);

	      cdata = g_new0 (CountryData, 1);

	      cdata->widget = subtree;
	      cdata->keymaps = NULL;

	      cdata->keymaps = g_list_append (cdata->keymaps, item);
	      gtk_object_set_data (GTK_OBJECT (subitem), "item", item);

	      g_hash_table_insert (ldata->countries, item->country, cdata);
	    }
	}
      else
	{
	  /* There is no lang */

	  titem = gtk_tree_item_new_with_label (item->lang);
	  gtk_tree_append (GTK_TREE (tree), titem);
	  gtk_widget_show (titem);

	  subtree = gtk_tree_new ();
	  gtk_tree_set_selection_mode (GTK_TREE (subtree),
				       GTK_SELECTION_SINGLE);
	  gtk_tree_set_view_mode (GTK_TREE (subtree), GTK_TREE_VIEW_ITEM);
	  gtk_tree_item_set_subtree (GTK_TREE_ITEM (titem), subtree);
	  subitem = gtk_tree_item_new_with_label (item->country);
	  gtk_tree_append (GTK_TREE (subtree), subitem);
	  gtk_widget_show (subitem);

	  subtree2 = gtk_tree_new ();
	  gtk_tree_set_selection_mode (GTK_TREE (subtree2),
				       GTK_SELECTION_SINGLE);
	  gtk_tree_set_view_mode (GTK_TREE (subtree2), GTK_TREE_VIEW_ITEM);
	  gtk_tree_item_set_subtree (GTK_TREE_ITEM (subitem), subtree2);
	  sitem = gtk_tree_item_new_with_label (item->name);
	  gtk_tree_append (GTK_TREE (subtree2), sitem);
	  gtk_widget_show (sitem);

	  ldata = g_new0 (LangData, 1);
	  cdata = g_new0 (CountryData, 1);

	  cdata->widget = subtree2;
	  cdata->keymaps = NULL;

	  ldata->widget = subtree;

	  ldata->countries = g_hash_table_new (g_str_hash, g_str_equal);

	  cdata->keymaps = g_list_append (cdata->keymaps, item);
	  gtk_object_set_data (GTK_OBJECT (sitem), "item", item);

	  g_hash_table_insert (ldata->countries, item->country, cdata);

	  g_hash_table_insert (langs, item->lang, ldata);
	}

    }
}

static void
preadd_cb (GtkWidget * tree, GkbPropertyBoxInfo * pbi)
{
  GList *i;
  GtkLabel *label;
  GtkWidget *item;

  g_return_if_fail (tree != NULL);

  if ((i = GTK_TREE_SELECTION (tree)) != NULL)
    {
      item = GTK_WIDGET (i->data);
      label = GTK_LABEL (GTK_BIN (item)->child);
      if (GTK_IS_LABEL (label))
	{
	  pbi->keymap_for_add =
	    gtk_object_get_data (GTK_OBJECT (item), "item");
	}
    }
  return;
}


static gint
addwadd_cb (GtkWidget * addbutton, GkbPropertyBoxInfo * pbi)
{
  GkbKeymap *tdata;

  if (pbi->keymap_for_add)
    {
      tdata = g_new0 (GkbKeymap, 1);

      tdata->name = g_strdup (pbi->keymap_for_add->name);
      tdata->flag = g_strdup (pbi->keymap_for_add->flag);
      tdata->command = g_strdup (pbi->keymap_for_add->command);
      tdata->country = g_strdup (pbi->keymap_for_add->country);
      tdata->label = g_strdup (pbi->keymap_for_add->label);
      tdata->lang = g_strdup (pbi->keymap_for_add->lang);

      pbi->keymaps = g_list_append (pbi->keymaps, tdata);
    }

  gkb_prop_list_reload (pbi);

  if (g_list_length (pbi->keymaps) > 1)
    gnome_property_box_changed (GNOME_PROPERTY_BOX (pbi->box));
  return FALSE;
}

static gint
wdestroy_cb (GtkWidget * closebutton, GtkWidget * window)
{
  if (window == gkb->addwindow)
    gkb->addwindow = NULL;
  gtk_widget_destroy (window);

  return FALSE;
}


void
gkb_prop_map_add (GkbPropertyBoxInfo * pbi)
{
  GtkWidget *vbox1;
  GtkWidget *tree1;
  GtkWidget *scrolled1;
  GtkWidget *hbuttonbox1;
  GtkWidget *button4;
  GtkWidget *button5;
  GtkWidget *button6;

  if (gkb->addwindow)
    {
      gtk_widget_show_now (gkb->addwindow);
      gdk_window_raise (gkb->addwindow->window);
      return;
    }

  gkb->addwindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_modal (GTK_WINDOW (gkb->addwindow), TRUE);
  gtk_object_set_data (GTK_OBJECT (gkb->addwindow), "addwindow",
		       gkb->addwindow);
  gtk_window_set_title (GTK_WINDOW (gkb->addwindow), _("Select layout"));

  vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox1);
  gtk_container_add (GTK_CONTAINER (gkb->addwindow), vbox1);

  scrolled1 = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled1),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_widget_set_usize (scrolled1, 315, 202);
  gtk_box_pack_start (GTK_BOX (vbox1), scrolled1, TRUE, TRUE, 0);
  gtk_widget_show (scrolled1);

  tree1 = gtk_tree_new ();

  gtk_widget_show (tree1);

  treeitems_create (tree1);

  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolled1),
					 tree1);

  hbuttonbox1 = gtk_hbutton_box_new ();
  gtk_widget_show (hbuttonbox1);
  gtk_box_pack_start (GTK_BOX (vbox1), hbuttonbox1, FALSE, TRUE, 0);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox1),
			     GTK_BUTTONBOX_SPREAD);

  button4 = gtk_button_new_with_label (_("Add"));

  gtk_widget_show (button4);
  gtk_container_add (GTK_CONTAINER (hbuttonbox1), button4);
  GTK_WIDGET_SET_FLAGS (button4, GTK_CAN_DEFAULT);

  button5 = gnome_stock_button (GNOME_STOCK_BUTTON_CLOSE);
  gtk_widget_show (button5);
  gtk_container_add (GTK_CONTAINER (hbuttonbox1), button5);
  GTK_WIDGET_SET_FLAGS (button5, GTK_CAN_DEFAULT);

  button6 = gnome_stock_button (GNOME_STOCK_BUTTON_HELP);
  gtk_widget_show (button6);
  gtk_container_add (GTK_CONTAINER (hbuttonbox1), button6);
  GTK_WIDGET_SET_FLAGS (button6, GTK_CAN_DEFAULT);

  gtk_signal_connect (GTK_OBJECT (tree1), "selection_changed",
		      GTK_SIGNAL_FUNC (preadd_cb), pbi);
  gtk_signal_connect (GTK_OBJECT (button4), "clicked",
		      GTK_SIGNAL_FUNC (addwadd_cb), pbi);
  gtk_signal_connect (GTK_OBJECT (button5), "clicked",
		      GTK_SIGNAL_FUNC (wdestroy_cb),
		      GTK_OBJECT (gkb->addwindow));
  gtk_signal_connect (GTK_OBJECT (button6), "clicked",
		      GTK_SIGNAL_FUNC (addhelp_cb), GTK_OBJECT (tree1));

  gtk_widget_show (gkb->addwindow);

  return;
}

static void
addhelp_cb (AppletWidget * applet, gpointer data)
{
  GnomeHelpMenuEntry help_entry =
    { "gkb_applet", "index.html#GKBAPPLET-PREFS-ADD" };

  gnome_help_display (NULL, &help_entry);
}
