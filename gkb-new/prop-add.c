/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 2 -*- */
/* File: prop-add.c
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

static void addhelp_cb (PanelApplet * widget, gpointer data);

typedef struct _LangData LangData;
struct _LangData
{
  GtkTreeIter iter;
  GHashTable *countries;
};

typedef struct _CountryData CountryData;
struct _CountryData
{
  GtkTreeIter iter;
  GList *keymaps;
};

enum {
 NAME_COL,
 COMMAND_COL,
 FLAG_COL,
 LABEL_COL,
 LANG_COL,
 COUNTRY_COL,
 NUM_COLS
};

GtkWidget* 
tree_create (GtkTreeStore *model)
{
  GList *sets = NULL;
  GList *retval = NULL;
  GtkWidget *tree1;
  GtkCellRenderer *cell;
  GtkTreeViewColumn *column;

  GHashTable *langs = g_hash_table_new (g_str_hash, g_str_equal);
  LangData *ldata;
  CountryData *cdata;

  /* TODO: Error checking... */
  sets = gkb_preset_load (find_presets ());
  retval = sets;

  tree1 = gtk_tree_view_new_with_model (GTK_TREE_MODEL (model));
  
  cell = gtk_cell_renderer_text_new ();
 
  column = gtk_tree_view_column_new_with_attributes (_("Keymaps (select and press add)"), cell,
                                                           "text", 0, NULL);

  gtk_tree_view_append_column (GTK_TREE_VIEW (tree1), column);
 
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
              GtkTreeIter iter;
              
              gtk_tree_store_append (GTK_TREE_STORE(model), &iter, 
              				&cdata->iter);

	      gtk_tree_store_set (GTK_TREE_STORE(model), &iter,
				NAME_COL, item->name,
				COMMAND_COL, item->command,
				FLAG_COL, item->flag,
				LABEL_COL, item->label,
				LANG_COL, item->lang,
				COUNTRY_COL, item->country,
		                -1);
              cdata->keymaps = g_list_append (cdata->keymaps, item);	    
	    }
	  else
	    {
	      /* There is no country */

              GtkTreeIter citer;
              GtkTreeIter iter; 
                           
              gtk_tree_store_append (GTK_TREE_STORE(model), &citer, 
              				&ldata->iter);

	      gtk_tree_store_set (GTK_TREE_STORE(model), &citer,
				NAME_COL, item->country,
				COMMAND_COL, NULL,
				FLAG_COL, NULL,
				LABEL_COL, item->label,
				LANG_COL, item->lang,
				COUNTRY_COL, item->country,
		                -1);
	      
	      cdata = g_new0 (CountryData, 1);

              gtk_tree_store_append (GTK_TREE_STORE(model), &iter, 
              				&cdata->iter);

	      gtk_tree_store_set (GTK_TREE_STORE(model), &iter,
				NAME_COL, item->name,
				COMMAND_COL, item->command,
				FLAG_COL, item->flag,
				LABEL_COL, item->label,
				LANG_COL, item->lang,
				COUNTRY_COL, item->country,
		                -1);

	      memcpy(&cdata->iter,&citer,sizeof(GtkTreeIter));
	      cdata->keymaps = NULL;

	      cdata->keymaps = g_list_append (cdata->keymaps, item);

	      g_hash_table_insert (ldata->countries, item->country, cdata);
	    }
	}
      else
	{
	  /* There is no lang */

          GtkTreeIter liter;
          GtkTreeIter citer;
          GtkTreeIter iter; 

	  ldata = g_new0 (LangData, 1);

          gtk_tree_store_append (GTK_TREE_STORE(model), &liter,
              				NULL);

          gtk_tree_store_set (GTK_TREE_STORE(model), &liter,
				NAME_COL, item->lang,
				COMMAND_COL, item->command,
				FLAG_COL, item->flag,
				LABEL_COL, item->label,
				LANG_COL, item->lang,
				COUNTRY_COL, item->country,
		                -1);


	  memcpy(&ldata->iter,&liter,sizeof(GtkTreeIter));

	  ldata->countries = g_hash_table_new (g_str_hash, g_str_equal);
          gtk_tree_store_append (GTK_TREE_STORE(model), &citer, 
              				&ldata->iter);
     
          gtk_tree_store_set (GTK_TREE_STORE(model), &citer,
				NAME_COL, item->country,
				COMMAND_COL, item->command,
				FLAG_COL, item->flag,
				LABEL_COL, item->label,
				LANG_COL, item->lang,
				COUNTRY_COL, item->country,
		                -1);
	                
          cdata = g_new0 (CountryData, 1);

	  cdata->keymaps = g_list_append (cdata->keymaps, item);

	  memcpy(&cdata->iter,&citer,sizeof(GtkTreeIter));
	  cdata->keymaps = NULL;

          gtk_tree_store_append (GTK_TREE_STORE(model), &iter, 
              				&cdata->iter);

          gtk_tree_store_set (GTK_TREE_STORE(model), &iter,
				NAME_COL, item->name,
				COMMAND_COL, item->command,
				FLAG_COL, item->flag,
				LABEL_COL, item->label,
				LANG_COL, item->lang,
				COUNTRY_COL, item->country,
		                -1);

          memcpy(&cdata->iter,&citer,sizeof(GtkTreeIter));
	  cdata->keymaps = NULL;

	  cdata->keymaps = g_list_append (cdata->keymaps, item);

	  g_hash_table_insert (ldata->countries, item->country, cdata);

	  g_hash_table_insert (langs, item->lang, ldata);
	}

    }
    return tree1;
}

static void
preadd_cb (GtkTreeSelection *selection,
              GkbPropertyBoxInfo  *pbi)
{
  GkbKeymap *tdata;
  GtkTreeIter iter;
  gchar *value;
         
  tdata = g_new0 (GkbKeymap,1);
  
  if (! gtk_tree_selection_get_selected (selection, NULL, &iter))
     return;

  if (!pbi->keymap_for_add) g_free(pbi->keymap_for_add); 
  /* TODO: free them all */

  gtk_tree_model_get (GTK_TREE_MODEL(pbi->model), &iter,
                              NAME_COL, &value, -1);
  tdata->name = g_strdup (value);

  gtk_tree_model_get (GTK_TREE_MODEL(pbi->model), &iter,
                              FLAG_COL, &value, -1);
  tdata->flag = g_strdup (value);

  gtk_tree_model_get (GTK_TREE_MODEL(pbi->model), &iter,
                              COMMAND_COL, &value, -1);
  tdata->command = g_strdup (value);

  gtk_tree_model_get (GTK_TREE_MODEL(pbi->model), &iter,
                              COUNTRY_COL, &value, -1);
  tdata->country = g_strdup (value);

  gtk_tree_model_get (GTK_TREE_MODEL(pbi->model), &iter,
                              LABEL_COL, &value, -1);
  tdata->label = g_strdup (value);

  gtk_tree_model_get (GTK_TREE_MODEL(pbi->model), &iter,
                              LANG_COL, &value, -1); 
  tdata->lang = g_strdup (value);

  pbi->keymap_for_add = tdata;

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
  GtkTreeSelection *selection;
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

  pbi->model = gtk_tree_store_new (NUM_COLS, G_TYPE_STRING,
               G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
               G_TYPE_STRING, G_TYPE_STRING); 

  tree1 = tree_create (pbi->model);

  gtk_widget_show (tree1);

  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolled1),
					 tree1);

  hbuttonbox1 = gtk_hbutton_box_new ();
  gtk_widget_show (hbuttonbox1);
  gtk_box_pack_start (GTK_BOX (vbox1), hbuttonbox1, FALSE, TRUE, 0);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox1),
			     GTK_BUTTONBOX_SPREAD);

  button4 = gtk_button_new_from_stock (GTK_STOCK_ADD);

  gtk_widget_show (button4);
  GTK_WIDGET_SET_FLAGS (button4, GTK_CAN_DEFAULT);

  button5 = gtk_button_new_from_stock (GNOME_STOCK_BUTTON_CLOSE);
  gtk_widget_show (button5);
  GTK_WIDGET_SET_FLAGS (button5, GTK_CAN_DEFAULT);

  button6 = gtk_button_new_from_stock (GNOME_STOCK_BUTTON_HELP);
  gtk_widget_show (button6);
  GTK_WIDGET_SET_FLAGS (button6, GTK_CAN_DEFAULT);

  /* ability suxx :) */
  gtk_container_add (GTK_CONTAINER (hbuttonbox1), button6);
  gtk_container_add (GTK_CONTAINER (hbuttonbox1), button5);
  gtk_container_add (GTK_CONTAINER (hbuttonbox1), button4);

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree1));

  gtk_tree_selection_set_mode (GTK_TREE_SELECTION (selection),
                          	GTK_SELECTION_BROWSE);

  g_signal_connect (selection, "changed",
		      G_CALLBACK (preadd_cb), pbi);
  g_signal_connect (button4, "clicked",
		      G_CALLBACK (addwadd_cb), pbi);
  g_signal_connect (button5, "clicked",
		      G_CALLBACK (wdestroy_cb),
		      GTK_OBJECT (gkb->addwindow));
  g_signal_connect (button6, "clicked",
		      G_CALLBACK (addhelp_cb), 
	              GTK_OBJECT (tree1));

  gtk_widget_show (gkb->addwindow);

  return;
}

static void
addhelp_cb (PanelApplet * applet, gpointer data)
{
/*
  GnomeHelpMenuEntry help_entry =
    { "gkb_applet", "index.html#GKBAPPLET-PREFS-ADD" };

  gnome_help_display (NULL, &help_entry);
*/
}
