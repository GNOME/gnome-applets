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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gdk/gdkx.h>
#include <sys/stat.h>
#include <X11/Xlib.h>
#include <panel-applet-gconf.h>
#include <egg-screen-help.h>
#include "gkb.h"


#define GKB_MENU_ITEM_TEXT "GtkMenuItemText"

extern gboolean gail_loaded;

typedef struct _KeymapData KeymapData;
struct _KeymapData
{
  GtkWidget *widget;
  GkbKeymap *preset;
};

static GtkWidget *label = NULL;

/**
 * gkb_apply:
 * @pbi: Propbox information
 * 
 * Apply settings.
 **/
void
gkb_apply (GkbPropertyBoxInfo * pbi)
{
  gint selected;
  GKB *gkb = pbi->gkb;
  /* Swap the lists of keymaps */
  gkb_keymap_free_list (gkb->maps);
  gkb->maps = gkb_keymap_copy_list (pbi->keymaps);

  /* recalculate new props */
  gkb->n = g_list_length (gkb->maps);
  gkb->cur = 0;
  gkb->keymap = g_list_nth_data (gkb->maps, 0);
 
  gkb_update (gkb, TRUE);

  selected = g_list_index(pbi->keymaps,pbi->selected_keymap);

  /* We need keymap->parent to be valid, reload list */
  gkb_keymap_free_list (pbi->keymaps);
  pbi->keymaps = gkb_keymap_copy_list (gkb->maps);
  pbi->selected_keymap = g_list_nth_data (pbi->keymaps,selected);
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
  label = gtk_label_new_with_mnemonic (label_text);
  gtk_table_attach (GTK_TABLE (table), label,
		    row, row + 1, col, col + 1,
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
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
  GKB *gkb = pbi->gkb;

  text = gtk_object_get_data (GTK_OBJECT (menu_item), GKB_MENU_ITEM_TEXT);

  g_return_if_fail (text != NULL);

  gkb->mode = gkb_util_get_mode_from_text (text);
  gkb_update (gkb, TRUE);
  gconf_applet_set_string ("mode", text);
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
      g_signal_connect (menu_item, "activate", function, pbi);
      gtk_object_set_data (GTK_OBJECT (menu_item), GKB_MENU_ITEM_TEXT,
			   list->data);
    }

  gtk_option_menu_set_menu (GTK_OPTION_MENU (option_menu), menu);
  gtk_option_menu_set_history (GTK_OPTION_MENU (option_menu), initval);

  gtk_table_attach (GTK_TABLE (table), option_menu,
		    row, row + 1, col, col + 1,
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		    (GtkAttachOptions) (0), 0, 0);

  gtk_label_set_mnemonic_widget(GTK_LABEL(label), option_menu );                
  if (gail_loaded)                                                              
    {                                                                           
      add_atk_relation(option_menu, label, ATK_RELATION_LABELLED_BY);               }
}

static GtkWidget *
create_hig_category (GtkWidget *main_box, gchar *title)
{
	GtkWidget *vbox, *vbox2, *hbox;
	GtkWidget *label;
	gchar *tmp;
		
	vbox = gtk_vbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (main_box), vbox, FALSE, FALSE, 0);
	
	tmp = g_strdup_printf ("<b>%s</b>", title);
	label = gtk_label_new (NULL);
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_label_set_markup (GTK_LABEL (label), tmp);
	g_free (tmp);
	gtk_box_pack_start (GTK_BOX (vbox), label, TRUE, FALSE, 0);

	hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
	
	label = gtk_label_new ("    ");
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	
	vbox2 = gtk_vbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (hbox), vbox2, TRUE, TRUE, 0);

	return vbox2;
		
}

/**
 * gkb_prop_create_display_category:
 * @GkbPropertyBoxInfo * pbi: PropBox Information
 * 
 * Implement the display category
 * 
 * Return Value: the display category widget
 **/
static GtkWidget *
gkb_prop_create_display_category (GkbPropertyBoxInfo * pbi)
{
  GList *mode;

  GtkWidget *vbox;
  GtkWidget *vbox2;
  GtkWidget *hbox;
  GtkWidget *table;
  gint size;

  vbox = gtk_vbox_new (FALSE, 6);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);
  gtk_widget_show (vbox);
  
  vbox2 = create_hig_category (vbox, _("Display"));

  hbox = gtk_hbox_new (FALSE, 6);
  gtk_box_pack_start (GTK_BOX (vbox2), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  /* Create the table */
  table = gtk_table_new (2, 2, FALSE);
  gtk_box_pack_start (GTK_BOX (hbox), table, TRUE, TRUE, 0);
  gtk_table_set_row_spacings (GTK_TABLE (table), 6);
  gtk_table_set_col_spacings (GTK_TABLE (table), 12);

  /* Labels and option Menus */
  gkb_prop_label_at (table, 0, 0, _("_Appearance: "));
  mode = gkb_prop_get_mode ();
  gkb_prop_option_menu_at (table, 1, 0, mode,
			   GTK_SIGNAL_FUNC (gkb_prop_mode_changed), pbi,
			   gkb_util_get_int_from_mode (pbi->mode));
  g_list_free (mode);
  size = panel_applet_get_size (PANEL_APPLET (pbi->gkb->applet));

  return vbox;
}


/**
 * gkb_prop_create_hotkey_category:
 * @GkbProbertyBoxInfo * pbi: Propbox info
 * 
 * Implement the hotkey properties category
 * 
 * Return Value: the hotkey category widget
 **/
static GtkWidget *
gkb_prop_create_hotkey_category (GkbPropertyBoxInfo * pbi, GtkWidget * widget)
{
  GtkWidget *category;
  GtkWidget *button;
  GtkWidget *hbox;
  GtkWidget *vbox;
  GtkWidget *label;
  GKB *gkb = pbi->gkb;

  vbox = gtk_vbox_new (FALSE, 6);
  gtk_container_set_border_width (GTK_CONTAINER(vbox), 0);
  gtk_widget_show (vbox);
  
  category = create_hig_category (vbox, _("Keyboard Shortcuts"));
  hbox = gtk_hbox_new (FALSE, 12);

  gtk_container_add (GTK_CONTAINER (category), hbox);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  
  label = gtk_label_new_with_mnemonic (_("S_witch layout:"));
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

  if (pbi->hotkey_entry)
    gtk_widget_destroy (pbi->hotkey_entry);

  pbi->hotkey_entry = gtk_entry_new ();
  gtk_entry_set_text (GTK_ENTRY (pbi->hotkey_entry), gkb->key);
  gtk_editable_set_editable (GTK_EDITABLE (pbi->hotkey_entry), FALSE);
  gtk_entry_set_activates_default (GTK_ENTRY (pbi->hotkey_entry), TRUE);
  gtk_label_set_mnemonic_widget (GTK_LABEL(label), pbi->hotkey_entry);

  button = gtk_button_new_with_mnemonic (_("_Grab keys"));
  if (gail_loaded)                                                              
    {                                                                           
      add_atk_relation(GTK_WIDGET(pbi->hotkey_entry),
			button, ATK_RELATION_CONTROLLED_BY);                                                                      
      add_atk_relation(button, GTK_WIDGET(pbi->hotkey_entry),
			ATK_RELATION_CONTROLLER_FOR );                              }
                                                                           
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);                                                                         
  gtk_box_pack_start (GTK_BOX (hbox), pbi->hotkey_entry, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);

/*  g_signal_connect (pbi->hotkey_entry, "changed",
		      G_CALLBACK (changed_cb), pb);
*/
  g_signal_connect (button,
	"clicked",
	G_CALLBACK (grab_button_pressed), pbi);

  return vbox;
}

static void
prophelp_cb (GtkWidget *widget, gpointer data)
{
	GError *error = NULL;

        egg_help_display_on_screen (
		"gkb", "gkb-prefs",
		gtk_widget_get_screen (widget),
		&error);

	/* FIXME: display error to the user */
}

static void
window_response (GtkWidget *w, int response, gpointer data)
{
  GkbPropertyBoxInfo * pbi = data;
  GKB *gkb = pbi->gkb;

  if (response == GTK_RESPONSE_HELP)
    prophelp_cb (w, data);
  else {
    gtk_list_store_clear (GTK_LIST_STORE (gtk_tree_view_get_model 
                          (GTK_TREE_VIEW (pbi->list))));
    g_free (pbi);
    gtk_widget_destroy (w);
    gkb->addwindow = NULL;/*The add window gets destroyed with parent if open */
    gkb->propwindow = NULL;
         
    if (gkb->applet == NULL)
	gtk_main_quit ();
  }
}


static void
gkb_prop_create_property_box (GkbPropertyBoxInfo *pbi,
			      gboolean include_applet_options)
{
  GtkWidget *propnotebook;
  GtkWidget *display_category;
  GtkWidget *hotkey_category;
  GtkWidget *buttons_vbox;
  GtkWidget *page_1_vbox;
  GtkWidget *page_2_vbox;
  GtkWidget *scrolled_window;
  GtkWidget *page_1_label;
  GtkWidget *page_2_label;
  GtkWidget *label;
  GtkWidget *hbox;

  /* Create property box */
  pbi->box = pbi->gkb->propwindow =
      gtk_dialog_new_with_buttons ( include_applet_options
				    ? _("Keyboard Layout Switcher Preferences")
				    : _("Keyboard Layout Selector"),
				    NULL,
                                            GTK_DIALOG_DESTROY_WITH_PARENT,
                                            GTK_STOCK_HELP, GTK_RESPONSE_HELP,
                                            GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
                                            NULL);
  gtk_window_set_default_size (GTK_WINDOW (pbi->gkb->propwindow), 350, -1);
  if (pbi->gkb->applet)
      gtk_window_set_screen (GTK_WINDOW (pbi->gkb->propwindow),
			     gtk_widget_get_screen (pbi->gkb->applet));

  gtk_dialog_set_default_response (GTK_DIALOG (pbi->gkb->propwindow), GTK_RESPONSE_CLOSE);
  gtk_dialog_set_has_separator (GTK_DIALOG (pbi->gkb->propwindow), FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (pbi->gkb->propwindow), 5);

  propnotebook =  gtk_notebook_new ();
  gtk_container_set_border_width (GTK_CONTAINER (propnotebook), 5);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (pbi->gkb->propwindow)->vbox), propnotebook,
                      TRUE, TRUE, 0);
                              
  gtk_widget_show (propnotebook);

  /* Add page 1 */
  page_1_vbox = gtk_vbox_new (FALSE, 6);
  gtk_container_set_border_width (GTK_CONTAINER (page_1_vbox), 12);
  gtk_widget_show (page_1_vbox);
  page_1_label = gtk_label_new (_("Keyboards"));
  gtk_notebook_append_page (GTK_NOTEBOOK (propnotebook), page_1_vbox, page_1_label);

  /* Page 1 Frame */
  label = gtk_label_new_with_mnemonic (_("_Keyboards:"));
  scrolled_window = gkb_prop_create_scrolled_window (pbi, label);
  buttons_vbox = gkb_prop_create_buttons_vbox (pbi);
  hbox = gtk_hbox_new (FALSE, 12); 
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5); 
  gtk_box_pack_start (GTK_BOX (page_1_vbox), label, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), scrolled_window, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (page_1_vbox), hbox, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), buttons_vbox, FALSE, TRUE, 0);

  /* Add page 2 */
  if (include_applet_options) {
    page_2_vbox = gtk_vbox_new (FALSE, 18);
    gtk_container_set_border_width (GTK_CONTAINER (page_2_vbox), 12);
    gtk_widget_show (page_2_vbox);
    page_2_label = gtk_label_new_with_mnemonic (_("_Options"));
    gtk_notebook_append_page (GTK_NOTEBOOK (propnotebook), page_2_vbox, page_2_label);

    /* Page 2 Frames */
    display_category = gkb_prop_create_display_category (pbi);
    hotkey_category =
      gkb_prop_create_hotkey_category (pbi, propnotebook);
    gtk_box_pack_start (GTK_BOX (page_2_vbox), display_category, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (page_2_vbox), hotkey_category, FALSE, FALSE, 0);
  } else {
    gtk_notebook_set_show_border (GTK_NOTEBOOK (propnotebook), FALSE);
    gtk_notebook_set_show_tabs (GTK_NOTEBOOK (propnotebook), FALSE);
  }
  
  g_signal_connect (G_OBJECT (pbi->gkb->propwindow),
	"response",
	G_CALLBACK (window_response), pbi);
}

void	
properties_dialog (BonoboUIComponent *uic,
	           GKB       *gkb,
	           const gchar	  *verbname)
{
  GkbPropertyBoxInfo *pbi;
  
  if (gkb->propwindow) {
	gtk_window_set_screen (GTK_WINDOW (gkb->propwindow),
			       gtk_widget_get_screen (gkb->applet));
  	gtk_window_present (GTK_WINDOW (gkb->propwindow));
  	return;
  }
  
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

  gkb_prop_create_property_box (pbi, uic != NULL);
  gtk_widget_show_all (pbi->box);
}
