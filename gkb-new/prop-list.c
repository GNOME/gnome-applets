/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 2 -*- */
/* File: prop.c
 * Purpose: GNOME Keyboard switcher property box
 *
 * Copyright (C) 1999 Free Software Foundation
 * Authors: Szabolcs BAN <shooby@gnome.hu>, 1998-2000
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


#include <gnome.h>
#include <sys/stat.h>

#include "gkb.h"
/* --- list --- */

static void addhelp_cb (AppletWidget * widget, gpointer data);
static void edithelp_cb (AppletWidget * widget);


#define GKB_KEYMAP_TAG  "GkbKeymapTag"

/* Phew.. Globals... */
GkbKeymap * a_keymap;


typedef struct _LangData LangData;
struct _LangData
{
  GtkWidget * widget;
  GHashTable * countries;
};

typedef struct _CountryData CountryData;
struct _CountryData
{
  GtkWidget * widget;
  GList * keymaps;
};

static void
gkb_prop_list_reload (GkbPropertyBoxInfo *pbi)
{
  GList * list;


  GtkWidget * hbox1;
  GtkWidget * label3, * pixmap1, * list_item;
  GkbKeymap * tdata;
  gchar *pixmapname;
  gint counter;
  
  gtk_list_clear_items (pbi->list, 0, -1);
 
  counter=0;

  list = pbi->keymaps;
  for (; list != NULL ; list = list->next)
    {
      char buf[30];
      struct stat tempbuf;
      
      tdata = list->data;

      hbox1 = gtk_hbox_new (FALSE, 0);
      gtk_widget_ref (hbox1);
      gtk_widget_show (hbox1);
     
      pixmap1 = gtk_type_new (gnome_pixmap_get_type ());
      sprintf(buf,"gkb/%s",tdata->flag);
      pixmapname = gnome_unconditional_pixmap_file (buf);
      if (stat (pixmapname, &tempbuf))
	pixmapname = gnome_unconditional_pixmap_file ("gkb/gkb-foot.png");
      gnome_pixmap_load_file_at_size (GNOME_PIXMAP (pixmap1), pixmapname, 28, 20); 
      g_free (pixmapname);
      gtk_widget_ref (pixmap1); 
      gtk_widget_show (pixmap1);
      gtk_box_pack_start (GTK_BOX (hbox1), pixmap1, FALSE, TRUE, 0);
      
      label3 = gtk_label_new (tdata->name);
      gtk_widget_ref (label3);
      gtk_widget_show (label3);
      gtk_box_pack_start (GTK_BOX (hbox1), label3, TRUE, TRUE, 0);
      gtk_misc_set_alignment (GTK_MISC (label3), 0, 0.5);
      gtk_misc_set_padding (GTK_MISC (label3), 3, 0);
      
      list_item = gtk_list_item_new ();
      gtk_container_add(GTK_CONTAINER(list_item), hbox1);
#if 0	
      gtk_object_set_data (GTK_OBJECT(list_item),"hbox",hbox1);
      gtk_object_set_data (GTK_OBJECT(list_item),"flag",tdata->flag);
      gtk_object_set_data (GTK_OBJECT(list_item),"name",tdata->name);
#else	
      gtk_object_set_data (GTK_OBJECT (list_item), GKB_KEYMAP_TAG, tdata);
#endif
      gtk_container_add (GTK_CONTAINER (pbi->list), list_item);
      gtk_widget_show(list_item);
    }

  gtk_widget_show (GTK_WIDGET (pbi->list));
}




static void
treeitems_create(GtkWidget *tree)
{
	GList * sets = NULL;
	GList * retval = NULL;
        GtkWidget *sitem, *titem, *subitem, *subtree, *subtree2;

	GHashTable * langs = g_hash_table_new(g_str_hash, g_str_equal);
	LangData * ldata;
	CountryData * cdata;

	/* TODO: Error checking... */
	sets = gkb_preset_load(find_presets());
	retval = sets;
        
        while ((sets = g_list_next(sets)) != NULL)
         {
	  GkbKeymap * item; 

	  item = sets->data;

	  if ((ldata = g_hash_table_lookup (langs,item->lang)) != NULL)
	   {
	    /* There is lang */
	    if ((cdata = g_hash_table_lookup (ldata->countries,item->country)) != NULL)
	     {
	      /* There is country */
             sitem = gtk_tree_item_new_with_label (item->name);
             gtk_tree_append (GTK_TREE(cdata->widget), sitem);
             gtk_widget_show(sitem);

             cdata->keymaps = g_list_append(cdata->keymaps,item);

             gtk_object_set_data (GTK_OBJECT(sitem),"item",item);

	     }
	     else
	     {
	      /* There is no country */

	      subitem = gtk_tree_item_new_with_label (item->country);
	      gtk_tree_append (GTK_TREE(ldata->widget), subitem);
	      gtk_widget_show (subitem);

              subtree = gtk_tree_new();
              gtk_tree_set_selection_mode (GTK_TREE(subtree),
                                 GTK_SELECTION_SINGLE);
              gtk_tree_set_view_mode (GTK_TREE(subtree), GTK_TREE_VIEW_ITEM);
              gtk_tree_item_set_subtree (GTK_TREE_ITEM(subitem), subtree);
              subitem = gtk_tree_item_new_with_label (item->name);
              gtk_tree_append (GTK_TREE(subtree), subitem);
	      gtk_widget_show (subitem);

	      cdata = g_new0 (CountryData,1);

	      cdata->widget = subtree;
	      cdata->keymaps = NULL;

              cdata->keymaps = g_list_append (cdata->keymaps,item);
              gtk_object_set_data (GTK_OBJECT(subitem),"item",item);

	      g_hash_table_insert (ldata->countries, item->country, cdata);
	     }
	   }
	  else
	   {
	    /* There is no lang */

	    titem = gtk_tree_item_new_with_label (item->lang);
	    gtk_tree_append (GTK_TREE(tree), titem);
	    gtk_widget_show (titem);

	    subtree = gtk_tree_new();
	    gtk_tree_set_selection_mode (GTK_TREE(subtree),
                                 GTK_SELECTION_SINGLE);
	    gtk_tree_set_view_mode (GTK_TREE(subtree), GTK_TREE_VIEW_ITEM);
	    gtk_tree_item_set_subtree (GTK_TREE_ITEM(titem), subtree);
	    subitem = gtk_tree_item_new_with_label (item->country);
	    gtk_tree_append (GTK_TREE(subtree), subitem);
	    gtk_widget_show (subitem);

	    subtree2 = gtk_tree_new();
	    gtk_tree_set_selection_mode (GTK_TREE(subtree2),
                                 GTK_SELECTION_SINGLE);
	    gtk_tree_set_view_mode (GTK_TREE(subtree2), GTK_TREE_VIEW_ITEM);
	    gtk_tree_item_set_subtree (GTK_TREE_ITEM(subitem), subtree2);
            sitem = gtk_tree_item_new_with_label (item->name);
            gtk_tree_append (GTK_TREE(subtree2), sitem);
            gtk_widget_show (sitem);

	    ldata = g_new0 (LangData, 1);
            cdata = g_new0 (CountryData,1);

            cdata->widget = subtree2;
            cdata->keymaps = NULL;
	    
	    ldata->widget = subtree;

	    ldata->countries = g_hash_table_new (g_str_hash, g_str_equal);

            cdata->keymaps = g_list_append(cdata->keymaps,item);
            gtk_object_set_data (GTK_OBJECT(sitem),"item",item);

	    g_hash_table_insert (ldata->countries, item->country, cdata);

            g_hash_table_insert (langs, item->lang, ldata);
	   }
	  
	}
}

static void
preadd_cb (GtkWidget * tree, GtkWidget * button)
{
  GList *i;
  GtkLabel *label;
  GtkWidget *item;

  g_return_if_fail (tree != NULL);
  g_return_if_fail (button != NULL);

  gtk_widget_set_sensitive (button, FALSE);
 
  if((i = GTK_TREE_SELECTION(tree)) != NULL)
  {
   item = GTK_WIDGET (i->data);
   label = GTK_LABEL (GTK_BIN (item)->child);
   if (GTK_IS_LABEL(label))
    {
     gtk_widget_set_sensitive (button, TRUE);
     a_keymap = gtk_object_get_data (GTK_OBJECT(item),"item");
    }
   }
 return;
}


static gint 
addwadd_cb (GtkWidget * addbutton, GtkWidget * ctree, GkbPropertyBoxInfo *pbi)
{
 GkbKeymap * tdata;

 if (a_keymap)
 {
  tdata = g_new0 (GkbKeymap,1);

  tdata->name = g_strdup (a_keymap->name);
  tdata->flag = g_strdup (a_keymap->flag);
  tdata->command = g_strdup (a_keymap->command);
  tdata->country = g_strdup (a_keymap->country);
  tdata->label = g_strdup (a_keymap->label);
  tdata->lang = g_strdup (a_keymap->lang);

  pbi->keymaps = g_list_append (pbi->keymaps, tdata);
 }

 gkb_prop_list_reload (pbi);
 
 if ( g_list_length(pbi->keymaps) > 1 )
  gnome_property_box_changed (GNOME_PROPERTY_BOX (gkb->propbox));
 return FALSE;
}

static gint 
wdestroy_cb (GtkWidget * closebutton, GtkWidget * window)
{
 if (window == gkb->addwindow)
  gkb->addwindow=NULL;
 else
  gkb->mapedit=NULL;
 gtk_widget_destroy(window);
  
 return FALSE;
}


static void
addwindow_cb (GtkWidget *addbutton)
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
  gtk_window_set_modal(GTK_WINDOW(gkb->addwindow), TRUE);
  gtk_object_set_data (GTK_OBJECT (gkb->addwindow), "addwindow", gkb->addwindow);
  gtk_window_set_title (GTK_WINDOW (gkb->addwindow), _("Select layout"));

  vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox1);
  gtk_container_add (GTK_CONTAINER (gkb->addwindow), vbox1);

  scrolled1 = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled1),
                                  GTK_POLICY_AUTOMATIC,
	                	  GTK_POLICY_AUTOMATIC);
  gtk_widget_set_usize (scrolled1, 315, 202);
  gtk_box_pack_start (GTK_BOX (vbox1), scrolled1, TRUE, TRUE, 0);
  gtk_widget_show (scrolled1);

  tree1 = gtk_tree_new ();

  gtk_widget_show (tree1);

  treeitems_create (tree1);

  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW(scrolled1),
                                         tree1);

  hbuttonbox1 = gtk_hbutton_box_new ();
  gtk_widget_show (hbuttonbox1);
  gtk_box_pack_start (GTK_BOX (vbox1), hbuttonbox1, FALSE, TRUE, 0);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox1), GTK_BUTTONBOX_SPREAD);

  button4 = gtk_button_new_with_label (_("Add"));

  gtk_widget_show (button4);
  gtk_container_add (GTK_CONTAINER (hbuttonbox1), button4);
  GTK_WIDGET_SET_FLAGS (button4, GTK_CAN_DEFAULT);
  gtk_widget_set_sensitive (button4, FALSE);

  button5 = gnome_stock_button (GNOME_STOCK_BUTTON_CLOSE);
  gtk_widget_show (button5);
  gtk_container_add (GTK_CONTAINER (hbuttonbox1), button5);
  GTK_WIDGET_SET_FLAGS (button5, GTK_CAN_DEFAULT);

  button6 = gnome_stock_button (GNOME_STOCK_BUTTON_HELP);
  gtk_widget_show (button6);
  gtk_container_add (GTK_CONTAINER (hbuttonbox1), button6);
  GTK_WIDGET_SET_FLAGS (button6, GTK_CAN_DEFAULT);

  gtk_signal_connect (GTK_OBJECT(tree1), "selection_changed",
                      GTK_SIGNAL_FUNC(preadd_cb),
                      GTK_OBJECT(button4));
  gtk_signal_connect (GTK_OBJECT(button4), "clicked",
                      GTK_SIGNAL_FUNC(addwadd_cb),
                      GTK_OBJECT(tree1));
  gtk_signal_connect (GTK_OBJECT(button5), "clicked",
                      GTK_SIGNAL_FUNC(wdestroy_cb),
                      GTK_OBJECT(gkb->addwindow));
  gtk_signal_connect (GTK_OBJECT(button6), "clicked",
                      GTK_SIGNAL_FUNC(addhelp_cb),
                      GTK_OBJECT(tree1));

  gtk_widget_show(gkb->addwindow);

  return;
}

static void
apply_edited_cb (GtkWidget * button, gint pos, GkbPropertyBoxInfo *pbi)
{
 GtkWidget * nameentry, * labelentry;
 GtkWidget * langentry, * countryentry;
 GtkWidget * iconentry, * commandentry;
 GtkWidget * codepageentry, * archentry;
 GtkWidget * typeentry;

 GkbKeymap * data = g_list_nth_data (pbi->keymaps, pos);
 
 g_free(data->name);
 g_free(data->label);
 g_free(data->country);
 g_free(data->flag);
 g_free(data->lang);
 g_free(data->type);
 g_free(data->command);
 g_free(data->codepage);
 g_free(data->arch);

 nameentry = gtk_object_get_data ( GTK_OBJECT(gkb->mapedit), "entry41");
 data->name = g_strdup (gtk_entry_get_text(GTK_ENTRY(nameentry)));

 labelentry = gtk_object_get_data (GTK_OBJECT(gkb->mapedit), "entry42");
 data->label = g_strdup (gtk_entry_get_text(GTK_ENTRY(labelentry)));

 langentry = gtk_object_get_data (GTK_OBJECT(gkb->mapedit), "entry43");
 data->lang = g_strdup (gtk_entry_get_text(GTK_ENTRY(langentry)));

 countryentry = gtk_object_get_data (GTK_OBJECT(gkb->mapedit), "entry26");
 data->country = g_strdup (gtk_entry_get_text(GTK_ENTRY(countryentry)));

 iconentry = gtk_object_get_data (GTK_OBJECT(gkb->mapedit), "iconentry5");
 data->flag = g_strdup( g_basename (
     gnome_icon_entry_get_filename (GNOME_ICON_ENTRY (iconentry))));

 printf ("Applied flag: %s\n", data->flag); fflush (stdout);

 archentry = gtk_object_get_data (GTK_OBJECT(gkb->mapedit), "entry37");
 data->arch = g_strdup (gtk_entry_get_text(GTK_ENTRY(archentry)));

 typeentry = gtk_object_get_data (GTK_OBJECT(gkb->mapedit), "entry38");
 data->type = g_strdup (gtk_entry_get_text(GTK_ENTRY(typeentry)));

 codepageentry = gtk_object_get_data (GTK_OBJECT(gkb->mapedit), "entry39");
 data->codepage = g_strdup (gtk_entry_get_text(GTK_ENTRY(codepageentry)));

 commandentry = gtk_object_get_data (GTK_OBJECT(gkb->mapedit), "entry40");
 data->command = g_strdup (gtk_entry_get_text(GTK_ENTRY(commandentry)));

 gkb_prop_list_reload (pbi);
}


static void
addhelp_cb (AppletWidget * applet, gpointer data)
{
  GnomeHelpMenuEntry help_entry =
    { "gkb_applet", "index.html#GKBAPPLET-PREFS-ADD" };

  gnome_help_display (NULL, &help_entry);
}




static void
edithelp_cb (AppletWidget * applet)
{
  GnomeHelpMenuEntry help_entry =
    { "gkb_applet", "index.html#GKBAPPLET-PREFS-EDIT" };

  gnome_help_display (NULL, &help_entry);
}
static void
ok_edited_cb (GtkWidget * button, gint pos, GkbPropertyBoxInfo *pbi)
{
  apply_edited_cb (button, pos, pbi);
  gtk_widget_destroy (gkb->mapedit);
  gkb->mapedit = NULL;
}

static void
mapedit_cb (GkbPropertyBoxInfo *pbi)
{
  GList *combo28_items = NULL;
  GList *combo34_items = NULL;
  GList *combo35_items = NULL;
  GList *combo36_items = NULL;
  GList *combo37_items = NULL;
  GtkWidget *button4, *button5, *button6, *button7;
  GtkWidget *combo28, *combo34, *combo35, *combo36;
  GtkWidget *combo37;
  GtkWidget *entry26, *entry37, *entry38, *entry39;
  GtkWidget *entry40, *entry41, *entry42, *entry43;
  GtkWidget *hbox1;
  GtkWidget *hbuttonbox1;
  GtkWidget *hseparator1;
  GtkWidget *iconentry5;
  GtkWidget *label46, *label51, *label52, *label53;
  GtkWidget *label54, *label55, *label56, *label57;
  GtkWidget *label58, *label59;
  GtkWidget *table6, *table7;
  GtkWidget *vbox2, *vbox3;
  GtkWidget *vseparator1;
  GList * mlist;
  gint pos;
  GkbKeymap * data;

  mlist = GTK_LIST(pbi->list)->selection;

  if (mlist){

  pos = gtk_list_child_position (GTK_LIST(pbi->list), GTK_WIDGET(mlist->data));

  data = g_list_nth_data (pbi->keymaps, pos);

  if (gkb->mapedit)
    {
      gtk_widget_destroy (gkb->mapedit);
    }
    
  gkb->mapedit = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_modal(GTK_WINDOW (gkb->mapedit), TRUE);
  gtk_object_set_data (GTK_OBJECT (gkb->mapedit), "mapedit", gkb->mapedit);
  gtk_window_set_title (GTK_WINDOW (gkb->mapedit), _("Edit keymap"));

  vbox2 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox2);
  gtk_container_add (GTK_CONTAINER (gkb->mapedit), vbox2);

  hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox1);
  gtk_box_pack_start (GTK_BOX (vbox2), hbox1, TRUE, TRUE, 0);

  table7 = gtk_table_new (5, 2, FALSE);
  gtk_widget_show (table7);
  gtk_box_pack_start (GTK_BOX (hbox1), table7, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (table7), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table7), 8);
  gtk_table_set_col_spacings (GTK_TABLE (table7), 5);

  label55 = gtk_label_new (_("Name"));
  gtk_widget_show (label55);
  gtk_table_attach (GTK_TABLE (table7), label55, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label55), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (label55), 1, 0.5);

  label56 = gtk_label_new (_("Label"));
  gtk_widget_show (label56);
  gtk_table_attach (GTK_TABLE (table7), label56, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label56), 1, 0.5);

  label57 = gtk_label_new (_("Language"));
  gtk_widget_show (label57);
  gtk_table_attach (GTK_TABLE (table7), label57, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label57), 1, 0.5);

  label58 = gtk_label_new (_("Country"));
  gtk_widget_show (label58);
  gtk_table_attach (GTK_TABLE (table7), label58, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label58), 1, 0.5);

  label59 = gtk_label_new (_("Flag\nPixmap"));
  gtk_widget_show (label59);
  gtk_table_attach (GTK_TABLE (table7), label59, 0, 1, 4, 5,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label59), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (label59), 1, 0.5);

  entry41 = gtk_entry_new ();
  gtk_entry_set_text(GTK_ENTRY(entry41), strdup(data->name));

  gtk_widget_show (entry41);
  gtk_table_attach (GTK_TABLE (table7), entry41, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  entry42 = gtk_entry_new ();
  gtk_entry_set_text(GTK_ENTRY(entry42), strdup(data->label));
  gtk_widget_show (entry42);
  gtk_table_attach (GTK_TABLE (table7), entry42, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  combo37 = gtk_combo_new ();
  gtk_widget_show (combo37);
  gtk_table_attach (GTK_TABLE (table7), combo37, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  combo37_items = g_list_append (combo37_items, _("Armenian"));
  combo37_items = g_list_append (combo37_items, _("Basque"));
  combo37_items = g_list_append (combo37_items, _("Bulgarian"));
  combo37_items = g_list_append (combo37_items, _("Dutch"));
  combo37_items = g_list_append (combo37_items, _("English"));
  combo37_items = g_list_append (combo37_items, _("French"));
  combo37_items = g_list_append (combo37_items, _("Georgian"));
  combo37_items = g_list_append (combo37_items, _("German"));
  combo37_items = g_list_append (combo37_items, _("Hungarian"));
  combo37_items = g_list_append (combo37_items, _("Norwegian"));
  combo37_items = g_list_append (combo37_items, _("Polish"));
  combo37_items = g_list_append (combo37_items, _("Portuglese"));
  combo37_items = g_list_append (combo37_items, _("Portuguese"));
  combo37_items = g_list_append (combo37_items, _("Russian"));
  combo37_items = g_list_append (combo37_items, _("Slovak"));
  combo37_items = g_list_append (combo37_items, _("Slovenian"));
  combo37_items = g_list_append (combo37_items, _("Swedish"));
  combo37_items = g_list_append (combo37_items, _("Thai"));
  combo37_items = g_list_append (combo37_items, _("Turkish"));
  combo37_items = g_list_append (combo37_items, _("Wallon"));
  combo37_items = g_list_append (combo37_items, _("Yugoslavian"));
  gtk_combo_set_popdown_strings (GTK_COMBO (combo37), combo37_items);
  g_list_free (combo37_items);

  entry43 = GTK_COMBO (combo37)->entry;
  gtk_widget_show (entry43);
  gtk_entry_set_text (GTK_ENTRY (entry43), data->lang);

  combo28 = gtk_combo_new ();
  gtk_widget_show (combo28);
  gtk_table_attach (GTK_TABLE (table7), combo28, 1, 2, 3, 4,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  combo28_items = g_list_append (combo28_items, _("Armenia"));
  combo28_items = g_list_append (combo28_items, _("Australia"));
  combo28_items = g_list_append (combo28_items, _("Austria"));
  combo28_items = g_list_append (combo28_items, _("Belgium"));
  combo28_items = g_list_append (combo28_items, _("Brazil"));
  combo28_items = g_list_append (combo28_items, _("Bulgaria"));
  combo28_items = g_list_append (combo28_items, _("Canada"));
  combo28_items = g_list_append (combo28_items, _("Caribic"));
  combo28_items = g_list_append (combo28_items, _("France"));
  combo28_items = g_list_append (combo28_items, _("Georgia"));
  combo28_items = g_list_append (combo28_items, _("Germany"));
  combo28_items = g_list_append (combo28_items, _("Great Britain"));
  combo28_items = g_list_append (combo28_items, _("Hungary"));
  combo28_items = g_list_append (combo28_items, _("Jamaica"));
  combo28_items = g_list_append (combo28_items, _("New Zealand"));
  combo28_items = g_list_append (combo28_items, _("Norway"));
  combo28_items = g_list_append (combo28_items, _("Poland"));
  combo28_items = g_list_append (combo28_items, _("Portugal"));
  combo28_items = g_list_append (combo28_items, _("Russia"));
  combo28_items = g_list_append (combo28_items, _("Slovak Republic"));
  combo28_items = g_list_append (combo28_items, _("Slovenia"));
  combo28_items = g_list_append (combo28_items, _("South Africa"));
  combo28_items = g_list_append (combo28_items, _("Spain"));
  combo28_items = g_list_append (combo28_items, _("Sweden"));
  combo28_items = g_list_append (combo28_items, _("Switzerland"));
  combo28_items = g_list_append (combo28_items, _("Thailand"));
  combo28_items = g_list_append (combo28_items, _("Turkey"));
  combo28_items = g_list_append (combo28_items, _("United States"));
  combo28_items = g_list_append (combo28_items, _("Yugoslavia"));
  gtk_combo_set_popdown_strings (GTK_COMBO (combo28), combo28_items);
  g_list_free (combo28_items);

  entry26 = GTK_COMBO (combo28)->entry;
  gtk_widget_show (entry26);
  gtk_entry_set_text (GTK_ENTRY (entry26), data->country);

  iconentry5 = gnome_icon_entry_new (NULL, NULL);

  gnome_icon_entry_set_pixmap_subdir (GNOME_ICON_ENTRY(iconentry5),"gkb");

  gtk_widget_show (iconentry5);
  gtk_table_attach (GTK_TABLE (table7), iconentry5, 1, 2, 4, 5,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  gnome_icon_entry_set_icon (GNOME_ICON_ENTRY (iconentry5),
  		    data->flag);

  vseparator1 = gtk_vseparator_new ();
  gtk_widget_show (vseparator1);
  gtk_box_pack_start (GTK_BOX (hbox1), vseparator1, TRUE, TRUE, 0);

  vbox3 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox3);
  gtk_box_pack_start (GTK_BOX (hbox1), vbox3, TRUE, TRUE, 0);

  table6 = gtk_table_new (4, 2, FALSE);
  gtk_widget_show (table6);
  gtk_box_pack_start (GTK_BOX (vbox3), table6, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (table6), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table6), 8);
  gtk_table_set_col_spacings (GTK_TABLE (table6), 5);

  label51 = gtk_label_new (_("Architecture"));
  gtk_widget_show (label51);
  gtk_table_attach (GTK_TABLE (table6), label51, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label51), 1, 0.5);

  label52 = gtk_label_new (_("Type"));
  gtk_widget_show (label52);
  gtk_table_attach (GTK_TABLE (table6), label52, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label52), 1, 0.5);

  label53 = gtk_label_new (_("Codepage"));
  gtk_widget_show (label53);
  gtk_table_attach (GTK_TABLE (table6), label53, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label53), 1, 0.5);

  label54 = gtk_label_new (_("Command"));
  gtk_widget_show (label54);
  gtk_table_attach (GTK_TABLE (table6), label54, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label54), 1, 0.5);

  combo34 = gtk_combo_new ();
  gtk_widget_show (combo34);
  gtk_table_attach (GTK_TABLE (table6), combo34, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  combo34_items = g_list_append (combo34_items, _("ix86"));
  combo34_items = g_list_append (combo34_items, _("sun"));
  combo34_items = g_list_append (combo34_items, _("mac"));
  combo34_items = g_list_append (combo34_items, _("sgi"));
  combo34_items = g_list_append (combo34_items, _("dec"));
  combo34_items = g_list_append (combo34_items, _("ibm"));
  gtk_combo_set_popdown_strings (GTK_COMBO (combo34), combo34_items);
  g_list_free (combo34_items);

  entry37 = GTK_COMBO (combo34)->entry;
  gtk_widget_show (entry37);
  gtk_entry_set_text (GTK_ENTRY (entry37), data->arch);

  combo35 = gtk_combo_new ();
  gtk_widget_show (combo35);
  gtk_table_attach (GTK_TABLE (table6), combo35, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  combo35_items = g_list_append (combo35_items, _("105"));
  combo35_items = g_list_append (combo35_items, _("101"));
  combo35_items = g_list_append (combo35_items, _("102"));
  combo35_items = g_list_append (combo35_items, _("450"));
  combo35_items = g_list_append (combo35_items, _("84"));
  combo35_items = g_list_append (combo35_items, _("mklinux"));
  combo35_items = g_list_append (combo35_items, _("type5"));
  gtk_combo_set_popdown_strings (GTK_COMBO (combo35), combo35_items);
  g_list_free (combo35_items);

  entry38 = GTK_COMBO (combo35)->entry;
  gtk_widget_show (entry38);
  gtk_entry_set_text (GTK_ENTRY (entry38), data->type);

  combo36 = gtk_combo_new ();
  gtk_widget_show (combo36);
  gtk_table_attach (GTK_TABLE (table6), combo36, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  combo36_items = g_list_append (combo36_items, _("iso-8859-1"));
  combo36_items = g_list_append (combo36_items, _("iso-8859-2"));
  combo36_items = g_list_append (combo36_items, _("iso-8859-9"));
  combo36_items = g_list_append (combo36_items, _("am-armscii8"));
  combo36_items = g_list_append (combo36_items, _("be-latin1"));
  combo36_items = g_list_append (combo36_items, _("cp1251"));
  combo36_items = g_list_append (combo36_items, _("georgian-academy"));
  combo36_items = g_list_append (combo36_items, _("koi8-r"));
  combo36_items = g_list_append (combo36_items, _("tis620"));
  gtk_combo_set_popdown_strings (GTK_COMBO (combo36), combo36_items);
  g_list_free (combo36_items);

  entry39 = GTK_COMBO (combo36)->entry;
  gtk_widget_show (entry39);
  gtk_entry_set_text (GTK_ENTRY (entry39), data->codepage);

  entry40 = gtk_entry_new ();
  gtk_entry_set_text (GTK_ENTRY (entry40), data->command);
  gtk_widget_show (entry40);
  gtk_table_attach (GTK_TABLE (table6), entry40, 1, 2, 3, 4,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  label46 = gtk_label_new ("");
  gtk_label_parse_uline (GTK_LABEL (label46),
                         _("Where command can be:\n"
                         "* xmodmap /full/path/xmodmap.hu\n"
                         "* gkb__xmmap hu\n"
                         "* setxkbmap hu"));
  gtk_widget_show (label46);
  gtk_box_pack_start (GTK_BOX (vbox3), label46, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (label46), GTK_JUSTIFY_LEFT);

  hseparator1 = gtk_hseparator_new ();
  gtk_widget_show (hseparator1);
  gtk_box_pack_start (GTK_BOX (vbox2), hseparator1, TRUE, TRUE, 0);

  hbuttonbox1 = gtk_hbutton_box_new ();
  gtk_widget_show (hbuttonbox1);
  gtk_box_pack_start (GTK_BOX (vbox2), hbuttonbox1, TRUE, TRUE, 0);

  button4 = gnome_stock_button (GNOME_STOCK_BUTTON_OK);
  gtk_signal_connect (GTK_OBJECT (button4), "clicked",
  		      GTK_SIGNAL_FUNC(ok_edited_cb),
  		      GINT_TO_POINTER(pos));

  gtk_widget_show (button4);
  gtk_container_add (GTK_CONTAINER (hbuttonbox1), button4);
  GTK_WIDGET_SET_FLAGS (button4, GTK_CAN_DEFAULT);

  button5 = gnome_stock_button (GNOME_STOCK_BUTTON_APPLY);
  gtk_signal_connect (GTK_OBJECT (button5), "clicked",
  		      GTK_SIGNAL_FUNC (apply_edited_cb), 
  		      GINT_TO_POINTER(pos));

  gtk_widget_show (button5);
  gtk_container_add (GTK_CONTAINER (hbuttonbox1), button5);
  GTK_WIDGET_SET_FLAGS (button5, GTK_CAN_DEFAULT);

  button6 = gnome_stock_button (GNOME_STOCK_BUTTON_CLOSE);
  gtk_signal_connect (GTK_OBJECT(button6), "clicked",
                      GTK_SIGNAL_FUNC(wdestroy_cb),
                      GTK_OBJECT(gkb->mapedit));

  gtk_widget_show (button6);
  gtk_container_add (GTK_CONTAINER (hbuttonbox1), button6);
  GTK_WIDGET_SET_FLAGS (button6, GTK_CAN_DEFAULT);

  button7 = gnome_stock_button (GNOME_STOCK_BUTTON_HELP);
  gtk_signal_connect (GTK_OBJECT (button7), "clicked",
  		      GTK_SIGNAL_FUNC (edithelp_cb), 
  		      NULL);

  gtk_widget_show (button7);
  gtk_container_add (GTK_CONTAINER (hbuttonbox1), button7);
  GTK_WIDGET_SET_FLAGS (button7, GTK_CAN_DEFAULT);

  gtk_widget_show(gkb->mapedit);
  }
  return;
}

#if 0
static void
apply_cb (GtkWidget * pb, gint page)
{
  GList * list;
  GkbKeymap * tdata;
  GkbKeymap * data;

  if (page != -1)
    return;

  for(list = gkb->maps; list != NULL; list = list->next) {
	  GkbKeymapWg *p = list->data;
	  if(p) {
		  g_free(p->name);
		  g_free(p->command);
		  g_free(p->flag);
		  g_free(p);
	  }
  }
  
  g_list_free(gkb->maps);
  gkb->maps = NULL;

  for(list = pbi->keymaps; list != NULL; list = list->next) 
   {

    tdata = list->data;   
    data = g_new0 (GkbKeymap,1);
    data->name = g_strdup (tdata->name);
    data->flag = g_strdup (tdata->flag);
    data->command = g_strdup (tdata->command);
    data->type = g_strdup (tdata->type);
    data->arch = g_strdup (tdata->arch);
    data->codepage = g_strdup (tdata->codepage);
    data->lang = g_strdup (tdata->lang);
    data->label = g_strdup (tdata->label);
    data->country = g_strdup (tdata->country);

    gkb->maps = g_list_append(gkb->maps, data);

    g_free(tdata->name);
    g_free(tdata->command);
    g_free(tdata->flag);
    g_free(tdata->lang);
    g_free(tdata->label);
    g_free(tdata->codepage);
    g_free(tdata->country);
    g_free(tdata->type);
    g_free(tdata->arch);
    g_free(tdata);

  }

  if (pbi->keymaps)
   g_list_free (pbi->keymaps);

  gkb->n = g_list_length(gkb->maps);

  gkb->cur = 0;

  gkb->dact = g_list_nth_data (gkb->maps, 0);

  gkb->small = gkb->tempsmall;
  gkb->size = gkb->tempsize;

/*  entry1 = gtk_object_get_data (GTK_OBJECT(gkb->propbox), "entry1");
  gkb->key = g_strdup (gtk_entry_get_text(GTK_ENTRY(entry1)));
  convert_string_to_keysym_state(gkb->key,
                                &gkb->keysym,
                                &gkb->state);		
                                                                                                               
*/

  gkb_update (gkb, FALSE);
  gkb_update (gkb, TRUE);

  applet_widget_sync_config(APPLET_WIDGET(gkb->applet));
}
#endif




static void
del_select_cb (GtkWidget * button, GkbPropertyBoxInfo *pbi)
{ 
 GtkWidget * deletebutton;
 GList *mlist;
 GtkObject *list_item;
 GtkWidget *hbox;
 gint pos;
 
 mlist=GTK_LIST(pbi->list)->selection;

 g_return_if_fail (mlist != NULL);

 if (mlist){
  list_item=GTK_OBJECT(mlist->data);
  hbox = gtk_object_get_data (GTK_OBJECT(list_item),"hbox");
  pos = gtk_list_child_position (GTK_LIST(pbi->list), GTK_WIDGET(mlist->data));

  pbi->keymaps = g_list_remove (pbi->keymaps,g_list_nth_data(pbi->keymaps, pos));

  deletebutton = gtk_object_get_data(GTK_OBJECT(gkb->propbox),"deletebutton");
  gtk_widget_set_sensitive (deletebutton, FALSE); 
 }

 gkb_prop_list_reload (pbi);

 if ( g_list_length(pbi->keymaps) > 1 )
  gnome_property_box_changed (GNOME_PROPERTY_BOX (gkb->propbox));
 
 return;
}



























/* -------------------------------------------------------- EVERY THING BELOW THIS POINT HAS BEEN CLEANED ---------------------------- */


/**
 * gkb_prop_list_move_clicked:
 * @pbi: 
 * @up: 
 * 
 * 
 **/
static void
gkb_prop_list_move_clicked (GkbPropertyBoxInfo *pbi, gboolean up)
{
 
#warning FIXME  
#if 0
 GtkWidget * upbutton;
 GtkWidget * downbutton;
 GtkWidget * list_item;
 GList *mlist;
 GkbKeymap * data;
 gint pos;
 GList * row, * nextr, * prevr;


 mlist=GTK_LIST(pbi->list)->selection;
 g_return_if_fail (mlist != NULL);

 downbutton = gtk_object_get_data(GTK_OBJECT(gkb->propbox),"downbutton");
 upbutton = gtk_object_get_data(GTK_OBJECT(gkb->propbox),"upbutton");
 
 if (mlist) {
  list_item=GTK_WIDGET(mlist->data);
  pos = gtk_list_child_position (GTK_LIST(pbi->list), list_item);

  row = g_list_nth(pbi->keymaps,pos);
  data = g_list_nth_data(pbi->keymaps,pos);

  printf("POS:%d, N: %s\n",pos, data->name);fflush(stdout);

  if (button == upbutton) 
   { 
    prevr = row->prev;
    if ((row->prev = prevr->prev) == NULL)
     {
      pbi->keymaps = row;
     }
     else
     {
      prevr->prev->next = row;
     }

    if ((nextr = row->next) != NULL) {
      nextr->prev = prevr;
    }

    row->next = prevr;

    prevr->prev = row;

    prevr->next = nextr;

    pos--;
   }
   else 
   {
    nextr = row->next;
    if ((prevr = row->prev) != NULL) {
     prevr->next = nextr;
    } else {
     pbi->keymaps = nextr;
    }
    row->next = nextr->next;
    row->prev = nextr;
    nextr->next = row;
    nextr->prev = prevr;
    pos++;
   }

  list_show(pos);

  if (pos == 0) 
    {
     gtk_widget_set_sensitive (downbutton, TRUE);
     gtk_widget_set_sensitive (upbutton, FALSE);
    } else 
    {
     gtk_widget_set_sensitive (upbutton, TRUE);
     gtk_widget_set_sensitive (downbutton, FALSE);
     if (pos < (g_list_length(pbi->keymaps) - 1)) 
      {
      gtk_widget_set_sensitive (downbutton, TRUE);
      }
    }
  }

 if ( g_list_length(pbi->keymaps) > 1 )
  gnome_property_box_changed (GNOME_PROPERTY_BOX (gkb->propbox));

#endif	
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
gkb_prop_list_update_sensitivity (GkbPropertyBoxInfo *pbi)
{
  gboolean row_selected = FALSE;
  gboolean one_row = FALSE; /* True if there is only 1 row */
  
  debug (FALSE, "");

  g_return_if_fail (pbi != NULL);
  g_return_if_fail (GTK_IS_WIDGET (pbi->add_button));

  if (GTK_LIST (pbi->list)->selection)
    row_selected = TRUE;
  if (g_list_length (GTK_LIST (pbi->list)->children) == 1)
    one_row = TRUE;

  gtk_widget_set_sensitive (pbi->add_button,    TRUE);
  gtk_widget_set_sensitive (pbi->edit_button,   row_selected);
  gtk_widget_set_sensitive (pbi->up_button,     row_selected && !one_row);
  gtk_widget_set_sensitive (pbi->down_button,   row_selected && !one_row);
  gtk_widget_set_sensitive (pbi->delete_button, row_selected && !one_row);
}

/**
 * gkb_prop_list_selection_changed:
 * @list: 
 * @pbi: 
 * 
 * The list's selection has changed
 **/
static void
gkb_prop_list_selection_changed (GtkWidget *list, GkbPropertyBoxInfo *pbi)
{
  GList *selection;
  
  debug (FALSE, "");

  g_return_if_fail (GTK_IS_WIDGET (list));
  g_return_if_fail (pbi != NULL);
  g_return_if_fail (GTK_IS_WIDGET (pbi->add_button));

  gkb_prop_list_update_sensitivity (pbi);

  selection = GTK_LIST(pbi->list)->selection;
  
  if (selection) {
    GkbKeymap *keymap;
    GtkListItem *list_item;

    list_item = GTK_LIST_ITEM (selection->data);
    keymap = gtk_object_get_data (GTK_OBJECT (list_item), GKB_KEYMAP_TAG);
    g_return_if_fail (keymap != NULL);
    pbi->selected_keymap = keymap;
  } else {
    pbi->selected_keymap = NULL;
  }
  
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
gkb_prop_list_button_clicked_cb (GtkWidget *button, GkbPropertyBoxInfo *pbi)
{
  if (button == pbi->add_button)
    addwindow_cb (button);
  else if (button == pbi->edit_button)
    mapedit_cb (pbi);
  else if (button == pbi->delete_button)
    del_select_cb (button, pbi);
  else if (button == pbi->up_button)
    gkb_prop_list_move_clicked (pbi, TRUE);
  else if (button == pbi->down_button)
    gkb_prop_list_move_clicked (pbi, FALSE);
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
gkb_prop_list_create_button (const gchar *name, GkbPropertyBoxInfo *pbi)
{
  GtkWidget *button;
  
  button = gtk_button_new_with_label (name);
  gtk_signal_connect (GTK_OBJECT (button), "clicked", 
                      GTK_SIGNAL_FUNC (gkb_prop_list_button_clicked_cb),
                      pbi);
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
gkb_prop_create_buttons_vbox (GkbPropertyBoxInfo *pbi)
{
  GtkWidget *vbox;

  vbox = gtk_vbutton_box_new ();
  gtk_button_box_set_spacing (GTK_BUTTON_BOX (vbox), 0);
  gtk_button_box_set_child_size (GTK_BUTTON_BOX (vbox), 75, 25);
  gtk_button_box_set_child_ipadding (GTK_BUTTON_BOX (vbox), 2, 0);

  pbi->buttons_vbox  = vbox;
  pbi->add_button    = gkb_prop_list_create_button (_("Add"),    pbi);
  pbi->edit_button   = gkb_prop_list_create_button (_("Edit"),   pbi);
  pbi->up_button     = gkb_prop_list_create_button (_("Up"),     pbi);
  pbi->down_button   = gkb_prop_list_create_button (_("Down"),   pbi);
  pbi->delete_button = gkb_prop_list_create_button (_("Delete"), pbi);

  return vbox;
}



/**
 * gkb_utils_copy_keymap:
 * @keymap: 
 * 
 * Copy an incoming keymap and return a pointer to a newly allocated one.
 * 
 * Return Value: 
 **/
static GkbKeymap *
gkb_utils_copy_keymap (GkbKeymap *keymap)
{
  GkbKeymap *new_keymap;

  debug (FALSE, "");
  
  new_keymap = g_new0 (GkbKeymap, 1);
  new_keymap->name    = g_strdup (keymap->name);
  new_keymap->command = g_strdup (keymap->name);
  new_keymap->flag    = g_strdup (keymap->flag);
  new_keymap->country = g_strdup (keymap->country);
  new_keymap->command = g_strdup (keymap->command);
  new_keymap->lang    = g_strdup (keymap->lang);
  new_keymap->label   = g_strdup (keymap->label);

  return new_keymap;
}


/**
 * gkb_utils_free_keymap:
 * @keymap: 
 * 
 * Free a keymap
 **/
static void
gkb_utils_free_keymap (GkbKeymap *keymap)
{

  g_free (keymap->name);
  g_free (keymap->flag);
  g_free (keymap->country);
  g_free (keymap->command);
  g_free (keymap->lang);
  g_free (keymap->label);
  g_free (keymap);
}
  

/**
 * gkb_prop_list_free_keymaps:
 * @gkb: 
 * 
 * Take care of freeing the gkb->tempas list (including contents)
 **/
void
gkb_prop_list_free_keymaps (GkbPropertyBoxInfo *pbi)
{
  GList *list;

  list = pbi->keymaps;
  for (; list != NULL; list = list->next)
      gkb_utils_free_keymap ((GkbKeymap *) list->data);

  g_list_free (pbi->keymaps);
  pbi->keymaps = NULL;
}


/**
 * gkb_prop_list_load_keymaps:
 * @void: 
 * 
 * Load the keymaps into the PropertyBoxInfo struct
 **/
static void
gkb_prop_list_load_keymaps (GkbPropertyBoxInfo *pbi)
{
  GkbKeymap *new_keymap;
  GkbKeymap *keymap;
  GList *new_list = NULL;
  GList *list;
 
  if (pbi->keymaps) {
    g_warning ("Dude ! you forgot to free the keymaps list somewhere.");
    gkb_prop_list_free_keymaps (pbi);
  }

  list = gkb->maps;
  for (; list != NULL; list = list->next) {
    keymap = list->data;
    new_keymap = gkb_utils_copy_keymap ((GkbKeymap *) list->data);
    new_list = g_list_prepend (new_list, new_keymap);
  }

  pbi->keymaps = g_list_reverse (new_list);
  
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
gkb_prop_create_scrolled_window (GkbPropertyBoxInfo *pbi)
{
  GtkWidget *scrolled_window;

  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
				  GTK_POLICY_AUTOMATIC,
	                	  GTK_POLICY_AUTOMATIC);
  pbi->list = GTK_LIST (gtk_list_new ());
  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolled_window),
					 GTK_WIDGET (pbi->list));
  gkb_prop_list_load_keymaps (pbi);

  gtk_signal_connect (GTK_OBJECT(pbi->list),
		      "selection_changed",
		      GTK_SIGNAL_FUNC (gkb_prop_list_selection_changed),
		      pbi);

  return scrolled_window;
}

