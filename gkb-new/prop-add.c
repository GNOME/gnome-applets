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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

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

static GtkWidget* 
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
 
  column = gtk_tree_view_column_new_with_attributes (_("Keyboards (select and press add)"), cell,
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
				COMMAND_COL, NULL,
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
				COMMAND_COL, NULL,
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
  g_free (value);

  gtk_tree_model_get (GTK_TREE_MODEL(pbi->model), &iter,
                              FLAG_COL, &value, -1);
  tdata->flag = g_strdup (value);
  g_free (value);

  gtk_tree_model_get (GTK_TREE_MODEL(pbi->model), &iter,
                              COMMAND_COL, &value, -1);
  tdata->command = g_strdup (value);
  g_free (value);

  gtk_tree_model_get (GTK_TREE_MODEL(pbi->model), &iter,
                              COUNTRY_COL, &value, -1);
  tdata->country = g_strdup (value);
  g_free (value);

  gtk_tree_model_get (GTK_TREE_MODEL(pbi->model), &iter,
                              LABEL_COL, &value, -1);
  tdata->label = g_strdup (value);
  g_free (value);

  gtk_tree_model_get (GTK_TREE_MODEL(pbi->model), &iter,
                              LANG_COL, &value, -1); 
  tdata->lang = g_strdup (value);
  g_free (value);

  pbi->keymap_for_add = tdata;

  return;
}


static gint
addwadd_cb (GtkWidget * addbutton, GkbPropertyBoxInfo * pbi)
{
  GkbKeymap *tdata;
  /* Do not add Language and Country rows */

  if (pbi->keymap_for_add) 
   {
   if (pbi->keymap_for_add->command != NULL)
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
   }

  gkb_prop_list_reload (pbi);

  gkb_apply(pbi);
  applet_save_session(pbi->gkb);

  return FALSE;
}

static gboolean
row_activated_cb (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
  GkbPropertyBoxInfo * pbi = data;
  
  if (event->type == GDK_2BUTTON_PRESS) {
  	addwadd_cb (NULL, pbi);
  	return TRUE;
  }
  
  return FALSE;
  
}

static void
response_cb (GtkDialog *dialog, gint id, gpointer data)
{
  GkbPropertyBoxInfo * pbi = data;

  switch (id) {
  case 100:
    /* Add response */
    addwadd_cb (NULL, pbi);
    break;
  case GTK_RESPONSE_HELP:
    addhelp_cb (NULL, NULL);
    break;
  default:
    gtk_widget_destroy (GTK_WIDGET (dialog));
    pbi->gkb->addwindow = NULL;
    break;
  }
 
}


void
gkb_prop_map_add (GkbPropertyBoxInfo * pbi)
{
  GtkWidget *vbox1;
  GtkWidget *tree1;
  GtkWidget *scrolled1;
  GtkTreeSelection *selection;
  GKB *gkb = pbi->gkb;
 
  if (gkb->addwindow)
    {
      gtk_window_present (GTK_WINDOW (gkb->addwindow));
      return;
    }

  gkb->addwindow = gtk_dialog_new_with_buttons (_("Select Keyboard"), NULL,
						GTK_DIALOG_DESTROY_WITH_PARENT,
						GTK_STOCK_HELP, GTK_RESPONSE_HELP,
						GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
						GTK_STOCK_ADD, 100,
						NULL);
  gtk_dialog_set_default_response (GTK_DIALOG (gkb->addwindow), 100);
  gtk_object_set_data (GTK_OBJECT (gkb->addwindow), "addwindow",
		       gkb->addwindow);
  
  vbox1 = GTK_DIALOG (gkb->addwindow)->vbox;
  gtk_widget_show (vbox1);
  
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

  gtk_container_add (GTK_CONTAINER (scrolled1), tree1);

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree1));

  gtk_tree_selection_set_mode (GTK_TREE_SELECTION (selection),
                          	GTK_SELECTION_BROWSE);

  g_signal_connect (selection, "changed",
		      G_CALLBACK (preadd_cb), pbi);
  /* Signal for double clicks or user pressing space */
  g_signal_connect (G_OBJECT (tree1), "button_press_event",
                    G_CALLBACK (row_activated_cb), pbi);
       
  g_signal_connect (G_OBJECT (gkb->addwindow), "response",
  		    G_CALLBACK (response_cb), pbi);
  		    
  gtk_widget_show (gkb->addwindow);

  return;
}

static void
addhelp_cb (PanelApplet * applet, gpointer data)
{
        GError *error = NULL;
        gnome_help_display("gkb","gkb-modify-list",&error);
}
