/* File: prop.c
 * Purpose: GNOME Keyboard switcher property box
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

#include <dirent.h>
#include "gkb.h"

typedef struct _PropWg PropWg;
struct _PropWg
{
  GdkPixmap *pix;

  char *name;
  char *command;
  char *iconpath;

  GtkWidget *diff_ch;
  GtkWidget *propbox;
  GtkWidget *notebook;
  GtkWidget *label1;
  GtkWidget *iconentry;
  GtkWidget *keymapname;
  GtkWidget *commandinput;
  GtkWidget *iconpathinput;
  GtkWidget *scrolledwin, *scrolledwinl;
  GtkWidget *hidebox, *hfa, *hfn;
  GtkWidget *frame21, *frame22, *label24, *label25, *entry21;
  GtkWidget *vbox1, *hbox1, *vbox2, *hbox2, *hbox3, *hboxmap;
  GtkWidget *frame1, *frame2, *frame3, *frame4, *frame6;
  GtkWidget *vbox21, *hbox21, *hbox22;
  GtkWidget *list, *tree, *iconentry21, *button21;
  GtkWidget *newkeymap, *delkeymap;
};

typedef struct _GKBpreset GKBpreset;
struct _GKBpreset
{
  gchar *name, *lang, *country; /* Names for items */
  gchar *command; /* Full switching command */
  gchar *codepage; /* Codepage, information only */
  gchar *flag; /* Flag filename */
  gchar *label; /* Label for label mode */
  gchar *arch; /* Sun, IBM, i386, or DEC */
  gchar *type; /* 101, 102, 105, microsoft, or any type */
};

typedef struct _CountryData CountryData;
struct _CountryData
{
  GtkWidget * widget, * listwg;
  GList * keymaps;
};

typedef struct _LangData LangData;
struct _LangData
{
  GtkWidget * widget, * listwg;
  GHashTable * countries;
};

static void prophelp_cb (AppletWidget * widget, gpointer data);
static void makenotebook (GKB * gkb, PropWg * actdata, int i);
static void advanced_show (GKB * gkb);
static PropWg *cp_prop (Prop * data);
static GList *copy_props (GKB * gkb);
static Prop *cp_propwg (PropWg * data);
static GList *copy_propwgs (GKB * gkb);
static GKBpreset * gkbpreset_load (const char *filename);
GtkWidget * defpage_create (GKB * gkb);

static gchar *prefixdir;

#include "prop_copy.h"
#include "prop_show.h"
#include "prop_cb.h"
#include "prop_preset.h"


static gint
country_select_cb(GtkWidget * widget, CountryData * cdata)
{
 /* TODO: Write this... */
}

static void
treeitems_create(GtkWidget *tree, GtkWidget *listwg)
{
	GList * list, * presets = NULL;
	GHashTable * langs = g_hash_table_new(g_str_hash, g_str_equal);
	LangData * ldata;
	CountryData * cdata;

	/* TODO: Error checking... */
	list=find_presets();

	while (list = g_list_next(list))
	{
	  GKBpreset * item = item = g_new0(GKBpreset, 1);

	  item = gkb_preset_load((gchar *)list->data);

	  if (ldata = g_hash_table_lookup (langs,item->lang))
	   {
	    /* There is lang */
	    if (cdata = g_hash_table_lookup (ldata->countries,item->country))
	     {
	      /* There is country */
	      g_list_append(cdata->keymaps,item);
	     }
	     else
	     {
	      /* There is no country */
	      GtkWidget * subtree, *subitem;

	      subtree = gtk_tree_new();
	      gtk_tree_set_selection_mode (GTK_TREE(subtree),                             
	                                       GTK_SELECTION_SINGLE);
	      gtk_tree_set_view_mode (GTK_TREE(subtree), GTK_TREE_VIEW_ITEM);
	      gtk_tree_item_set_subtree (GTK_TREE_ITEM(ldata->widget), subtree);
	      subitem = gtk_tree_item_new_with_label (item->country);
	      gtk_signal_connect (GTK_OBJECT(subitem), "select",                        
	                                GTK_SIGNAL_FUNC(country_select_cb), 
	                                cdata);
	      gtk_tree_append (GTK_TREE(subtree), subitem);
	      gtk_widget_show (subitem);

	      cdata = g_new0 (CountryData,1);

	      cdata->widget = subitem;
	      cdata->listwg = listwg;
	      cdata->keymaps = NULL;

	      g_hash_table_insert (ldata->countries, item->country, cdata);
	     }
	   }
	  else
	   {
	    /* There is no lang */
	    GtkWidget *titem, *subtree, *subitem;

	    titem = gtk_tree_item_new_with_label (item->lang);
	    gtk_tree_append (GTK_TREE(tree), titem);
	    gtk_widget_show (titem);
	    subtree = gtk_tree_new();
	    gtk_tree_set_selection_mode (GTK_TREE(subtree),                             
                                 GTK_SELECTION_SINGLE);
	    gtk_tree_set_view_mode (GTK_TREE(subtree), GTK_TREE_VIEW_ITEM);
	    gtk_tree_item_set_subtree (GTK_TREE_ITEM(titem), subtree);
	    subitem = gtk_tree_item_new_with_label (item->country);
            gtk_signal_connect (GTK_OBJECT(subitem), "select",     
                                  GTK_SIGNAL_FUNC(country_select_cb),
                                  cdata);
	    gtk_tree_append (GTK_TREE(subtree), subitem);
	    gtk_widget_show (subitem);
	    
	    ldata = g_new0 (LangData, 1);
            cdata = g_new0 (CountryData,1);

            cdata->widget = subitem;
            cdata->listwg = listwg; 
            cdata->keymaps = NULL;  
	    
	    ldata->listwg = listwg;
	    ldata->widget = titem;

	    ldata->countries = g_hash_table_new (g_str_hash, g_str_equal);
	    g_hash_table_insert (ldata->countries, item->country, cdata);

            g_hash_table_insert (langs, item->lang, ldata);
	   }
	  
	}
}

GtkWidget *
defpage_create (GKB * gkb)
{
  GtkWidget *frame1;
  GtkWidget *vbox1;
  GtkWidget *table1;
  GtkWidget *label5;
  GtkWidget *label6;
  GtkWidget *optionmenu2;
  GtkWidget *optionmenu2_menu;
  GtkWidget *menuitem;
  GtkWidget *optionmenu3;
  GtkWidget *optionmenu3_menu;
  GtkWidget *hbox1;
  gchar *pixmap1_filename;
  GtkWidget *pixmap1;
  GtkWidget *label4;
  GtkAccelGroup *accel_group;
  GtkTooltips *tooltips;

  tooltips = gtk_tooltips_new ();
  accel_group = gtk_accel_group_new ();

  frame1 = gtk_frame_new (_("Settings"));
  gtk_widget_ref (frame1);
  gtk_widget_show (frame1);

  vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (vbox1);
  gtk_widget_show (vbox1);
  gtk_container_add (GTK_CONTAINER (frame1), vbox1);

  table1 = gtk_table_new (2, 2, FALSE);
  gtk_widget_ref (table1);
  gtk_widget_show (table1);
  gtk_box_pack_start (GTK_BOX (vbox1), table1, TRUE, TRUE, 0);

  label5 = gtk_label_new (_("Applet size:"));
  gtk_widget_ref (label5);
  gtk_widget_show (label5);
  gtk_table_attach (GTK_TABLE (table1), label5, 0, 1, 0, 1,
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label5), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (label5), 0.95, 0.0100004);

  label6 = gtk_label_new (_("Applet style:"));
  gtk_widget_ref (label6);
  gtk_widget_show (label6);
  gtk_table_attach (GTK_TABLE (table1), label6, 0, 1, 1, 2,
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label6), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (label6), 0.95, 0.5);

  optionmenu2 = gtk_option_menu_new ();
  gtk_widget_ref (optionmenu2);
  gtk_widget_show (optionmenu2);
  gtk_table_attach (GTK_TABLE (table1), optionmenu2, 1, 2, 0, 1,
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		    (GtkAttachOptions) (GTK_EXPAND), 0, 0);
  gtk_tooltips_set_tip (tooltips, optionmenu2,
			_("Normal: Panel size. Small: Half panel size."),
			NULL);
  optionmenu2_menu = gtk_menu_new ();
  menuitem = gtk_menu_item_new_with_label (_("Normal"));
  gtk_signal_connect (GTK_OBJECT (menuitem), "activate",
		      (GtkSignalFunc) switch_normal_cb, gkb);
  gtk_widget_show (menuitem);
  gtk_menu_append (GTK_MENU (optionmenu2_menu), menuitem);
  menuitem = gtk_menu_item_new_with_label (_("Small"));
  gtk_signal_connect (GTK_OBJECT (menuitem), "activate",
		      (GtkSignalFunc) switch_small_cb, gkb);
  gtk_widget_show (menuitem);
  gtk_menu_append (GTK_MENU (optionmenu2_menu), menuitem);
  gtk_option_menu_set_menu (GTK_OPTION_MENU (optionmenu2), optionmenu2_menu);

  optionmenu3 = gtk_option_menu_new ();
  gtk_widget_ref (optionmenu3);
  gtk_widget_show (optionmenu3);
  gtk_table_attach (GTK_TABLE (table1), optionmenu3, 1, 2, 1, 2,
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		    (GtkAttachOptions) (GTK_EXPAND), 0, 0);
  gtk_tooltips_set_tip (tooltips, optionmenu3,
			_("Flag: Your flag. Label: Do not display flag."),
			NULL);
  optionmenu3_menu = gtk_menu_new ();
  menuitem = gtk_menu_item_new_with_label (_("Flag"));
  gtk_widget_show (menuitem);
  gtk_menu_append (GTK_MENU (optionmenu3_menu), menuitem);
  menuitem = gtk_menu_item_new_with_label (_("Label"));
  gtk_widget_show (menuitem);
  gtk_menu_append (GTK_MENU (optionmenu3_menu), menuitem);
  gtk_option_menu_set_menu (GTK_OPTION_MENU (optionmenu3), optionmenu3_menu);

  hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_widget_ref (hbox1);
  gtk_widget_show (hbox1);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox1, TRUE, TRUE, 0);

  pixmap1 = gtk_type_new (gnome_pixmap_get_type ());
  pixmap1_filename = gnome_pixmap_file ("gkb.png");
  if (pixmap1_filename)
    gnome_pixmap_load_file (GNOME_PIXMAP (pixmap1), pixmap1_filename);
  else
    g_warning (_("Couldn't find pixmap file: %s"), "gkb.png");
  g_free (pixmap1_filename);
  gtk_widget_ref (pixmap1);
  gtk_widget_show (pixmap1);
  gtk_box_pack_start (GTK_BOX (hbox1), pixmap1, FALSE, FALSE, 15);

  label4 =
    gtk_label_new (_("If you cannot set something and you are power user, click \n"
		    "the checkbox below. You will able to set the commands manually."));
  gtk_widget_ref (label4);
  gtk_widget_show (label4);
  gtk_box_pack_start (GTK_BOX (hbox1), label4, FALSE, FALSE, 3);
  gtk_label_set_justify (GTK_LABEL (label4), GTK_JUSTIFY_FILL);
  gtk_misc_set_alignment (GTK_MISC (label4), 0.0500001, 0.5);

  gkb->advanced = gtk_check_button_new_with_label (_("Advanced settings"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gkb->advanced),
				gkb->advconf);
  gtk_signal_connect (GTK_OBJECT (gkb->advanced), "toggled",
		      GTK_SIGNAL_FUNC (switch_adv_cb), gkb);
  gtk_widget_ref (gkb->advanced);
  gtk_widget_show (gkb->advanced);
  gtk_box_pack_start (GTK_BOX (vbox1), gkb->advanced, TRUE, TRUE, 0);
  gtk_tooltips_set_tip (tooltips, gkb->advanced,
			_("Advanced settings: Check this if you really know what you wanna do!"),
			NULL);
  gtk_widget_add_accelerator (gkb->advanced, "toggled", accel_group, GDK_a,
			      GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);

  return frame1;
}

static void
makenotebook (GKB * gkb, PropWg * actdata, int i)
{
  actdata->notebook = gkb->notebook;
  actdata->propbox = gkb->propbox;

  actdata->hidebox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (gkb->notebook), actdata->hidebox);

  actdata->hfa = gtk_frame_new (NULL);
  gtk_widget_ref (actdata->hfa);

  gtk_box_pack_start (GTK_BOX (actdata->hidebox), actdata->hfa, FALSE, TRUE,
		      0);
  gtk_container_set_border_width (GTK_CONTAINER (actdata->hfa), 0);
  gtk_frame_set_shadow_type (GTK_FRAME (actdata->hfa), GTK_SHADOW_NONE);

  actdata->vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (actdata->vbox1);

  gtk_widget_ref (actdata->vbox1);
  gtk_container_add (GTK_CONTAINER (actdata->hfa), actdata->vbox1);

  actdata->hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_widget_ref (actdata->hbox1);
  gtk_widget_show (actdata->hbox1);

  actdata->frame4 = gtk_frame_new (_("Keymap name"));
  gtk_widget_ref (actdata->frame4);
  gtk_widget_show (actdata->frame4);
  gtk_box_pack_start (GTK_BOX (actdata->hbox1), actdata->frame4, FALSE, TRUE,
		      0);
  gtk_container_set_border_width (GTK_CONTAINER (actdata->frame4), 2);

  gtk_container_add (GTK_CONTAINER (actdata->vbox1), actdata->hbox1);

  actdata->keymapname = gtk_entry_new ();
  gtk_widget_ref (actdata->keymapname);
  gtk_widget_show (actdata->keymapname);

  gtk_entry_set_text (GTK_ENTRY (actdata->keymapname), actdata->name);

  gtk_container_add (GTK_CONTAINER (actdata->frame4), actdata->keymapname);

  gtk_signal_connect (GTK_OBJECT (actdata->keymapname),
		      "changed", GTK_SIGNAL_FUNC (changed_cb), gkb);

  actdata->frame6 = gtk_frame_new (_("Keymap control"));
  gtk_widget_ref (actdata->frame6);
  gtk_widget_show (actdata->frame6);
  gtk_box_pack_start (GTK_BOX (actdata->hbox1), actdata->frame6, TRUE, TRUE,
		      0);
  gtk_container_set_border_width (GTK_CONTAINER (actdata->frame6), 2);

  actdata->hboxmap = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (actdata->hboxmap);

  actdata->newkeymap = gtk_button_new_with_label (_("New keymap"));

  gtk_widget_ref (actdata->newkeymap);
  gtk_widget_show (actdata->newkeymap);

  gtk_signal_connect (GTK_OBJECT (actdata->newkeymap),
		      "clicked", (GtkSignalFunc) newmap_cb, gkb);

  gtk_container_add (GTK_CONTAINER (actdata->hboxmap), actdata->newkeymap);
  gtk_container_set_border_width (GTK_CONTAINER (actdata->newkeymap), 2);

  actdata->delkeymap = gtk_button_new_with_label (_("Delete this keymap"));

  gtk_widget_ref (actdata->delkeymap);
  gtk_widget_show (actdata->delkeymap);

  gtk_signal_connect (GTK_OBJECT (actdata->delkeymap),
		      "clicked", (GtkSignalFunc) delmap_cb, gkb);

  gtk_container_add (GTK_CONTAINER (actdata->hboxmap), actdata->delkeymap);
  gtk_container_set_border_width (GTK_CONTAINER (actdata->delkeymap), 2);
  gtk_container_add (GTK_CONTAINER (actdata->frame6), actdata->hboxmap);

  actdata->hbox2 = gtk_hbox_new (FALSE, 0);
  gtk_widget_ref (actdata->hbox2);
  gtk_widget_show (actdata->hbox2);
  gtk_box_pack_start (GTK_BOX (actdata->vbox1), actdata->hbox2, TRUE, TRUE,
		      0);

  actdata->iconentry = gnome_icon_entry_new (NULL, _("Keymap icon"));
  gtk_widget_ref (actdata->iconentry);
  gtk_widget_show (actdata->iconentry);

  gnome_icon_entry_set_icon (GNOME_ICON_ENTRY (actdata->iconentry),
			     actdata->iconpath);
  gtk_signal_connect (GTK_OBJECT
		      (gnome_icon_entry_gtk_entry
		       (GNOME_ICON_ENTRY (actdata->iconentry))), "changed",
		      GTK_SIGNAL_FUNC (icontopath_cb), actdata);

  gtk_box_pack_end (GTK_BOX (actdata->hbox2), actdata->iconentry, FALSE,
		    FALSE, 0);
  gtk_widget_set_usize (actdata->iconentry, 60, 40);
  gtk_container_set_border_width (GTK_CONTAINER (actdata->iconentry), 2);

  actdata->vbox2 = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (actdata->vbox2);
  gtk_widget_show (actdata->vbox2);
  gtk_box_pack_start (GTK_BOX (actdata->hbox2), actdata->vbox2, TRUE, TRUE,
		      0);

  actdata->frame1 = gtk_frame_new (_("Iconpath"));
  gtk_widget_ref (actdata->frame1);
  gtk_widget_show (actdata->frame1);

  gtk_box_pack_start (GTK_BOX (actdata->vbox2), actdata->frame1, TRUE, TRUE,
		      0);
  gtk_container_set_border_width (GTK_CONTAINER (actdata->frame1), 2);

  actdata->iconpathinput = gtk_entry_new ();
  gtk_widget_ref (actdata->iconpathinput);
  gtk_widget_show (actdata->iconpathinput);

  gtk_signal_connect (GTK_OBJECT (actdata->iconpathinput), "changed",
		      GTK_SIGNAL_FUNC (pathtoicon_cb), actdata);

  gtk_entry_set_text (GTK_ENTRY (actdata->iconpathinput), actdata->iconpath);
  gtk_container_add (GTK_CONTAINER (actdata->frame1), actdata->iconpathinput);

  actdata->frame2 = gtk_frame_new (_("Full command"));
  gtk_widget_ref (actdata->frame2);
  gtk_widget_show (actdata->frame2);

  gtk_box_pack_start (GTK_BOX (actdata->vbox2), actdata->frame2, TRUE, TRUE,
		      0);
  gtk_container_set_border_width (GTK_CONTAINER (actdata->frame2), 2);

  actdata->commandinput = gtk_entry_new ();
  gtk_widget_ref (actdata->commandinput);
  gtk_widget_show (actdata->commandinput);
  gtk_entry_set_text (GTK_ENTRY (actdata->commandinput), actdata->command);

  gtk_signal_connect (GTK_OBJECT (actdata->commandinput), "changed",
		      GTK_SIGNAL_FUNC (changed_cb), gkb);
  gtk_container_add (GTK_CONTAINER (actdata->frame2), actdata->commandinput);

/*  Normal conf */

  actdata->hfn = gtk_frame_new (NULL);
  gtk_widget_ref (actdata->hfn);
  gtk_widget_show (actdata->hfn);
  gtk_frame_set_shadow_type (GTK_FRAME (actdata->hfn), GTK_SHADOW_NONE);

  gtk_container_add (GTK_CONTAINER (actdata->hidebox), actdata->hfn);

  gtk_container_set_border_width (GTK_CONTAINER (actdata->hfn), 0);

  actdata->hbox21 = gtk_hbox_new (FALSE, 0);
  gtk_widget_ref (actdata->hbox21);
  gtk_widget_show (actdata->hbox21);

  gtk_container_add (GTK_CONTAINER (actdata->hfn), actdata->hbox21);

  actdata->frame21 = gtk_frame_new (_("Language/Country"));
  gtk_widget_ref (actdata->frame21);
  gtk_widget_show (actdata->frame21);

  gtk_box_pack_start (GTK_BOX (actdata->hbox21), actdata->frame21, TRUE, TRUE,
		      0);
  gtk_container_set_border_width (GTK_CONTAINER (actdata->frame21), 1);

  actdata->scrolledwin = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (actdata->scrolledwin),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (actdata->frame21), actdata->scrolledwin);
  gtk_widget_set_usize (actdata->scrolledwin, 130, 140);
  gtk_widget_show (actdata->scrolledwin);

  actdata->tree = gtk_tree_new ();
  gtk_widget_ref (actdata->tree);


  gtk_widget_show (actdata->tree);

  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (actdata->scrolledwin), actdata->tree);
  gtk_tree_set_view_mode (GTK_TREE (actdata->tree), GTK_TREE_VIEW_ITEM);

  actdata->frame22 = gtk_frame_new (_("Keymap"));
  gtk_widget_ref (actdata->frame22);

  gtk_widget_show (actdata->frame22);
  gtk_box_pack_start (GTK_BOX (actdata->hbox21), actdata->frame22, TRUE, TRUE,
		      0);
  gtk_container_set_border_width (GTK_CONTAINER (actdata->frame22), 1);

  actdata->scrolledwinl = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (actdata->scrolledwinl),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (actdata->frame22), actdata->scrolledwinl);
  gtk_widget_set_usize (actdata->scrolledwinl, 130, 140);
  gtk_widget_show (actdata->scrolledwinl);

  actdata->list = gtk_list_new ();
  gtk_widget_ref (actdata->list);

  treeitems_create(actdata->tree,actdata->list);

  gtk_widget_show (actdata->list);
  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW(actdata->scrolledwinl), actdata->list);

  actdata->vbox21 = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (actdata->vbox21);

  gtk_widget_show (actdata->vbox21);
  gtk_box_pack_start (GTK_BOX (actdata->hbox21), actdata->vbox21, TRUE, TRUE,
		      0);

  actdata->hbox22 = gtk_hbox_new (FALSE, 0);
  gtk_widget_ref (actdata->hbox22);

  gtk_widget_show (actdata->hbox22);
  gtk_box_pack_start (GTK_BOX (actdata->vbox21), actdata->hbox22, FALSE,
		      FALSE, 0);

  actdata->label24 = gtk_label_new (_("Label:"));
  gtk_widget_ref (actdata->label24);

  gtk_widget_show (actdata->label24);
  gtk_box_pack_start (GTK_BOX (actdata->hbox22), actdata->label24, FALSE,
		      FALSE, 2);

  actdata->entry21 = gtk_entry_new ();
  gtk_widget_ref (actdata->entry21);
  gtk_widget_show (actdata->entry21);
  gtk_box_pack_start (GTK_BOX (actdata->hbox22), actdata->entry21, FALSE,
		      FALSE, 0);
  gtk_widget_set_usize (actdata->entry21, 89, -2);

  actdata->button21 = gtk_button_new_with_label (_("Set label"));
  gtk_widget_ref (actdata->button21);

  gtk_widget_show (actdata->button21);
  gtk_box_pack_start (GTK_BOX (actdata->hbox22), actdata->button21, FALSE,
		      FALSE, 0);

  actdata->iconentry21 = gnome_icon_entry_new (NULL, NULL);
  gtk_widget_ref (actdata->iconentry21);

  gtk_widget_show (actdata->iconentry21);
  gnome_icon_entry_set_icon (GNOME_ICON_ENTRY (actdata->iconentry21),
			     actdata->iconpath);
  gtk_box_pack_start (GTK_BOX (actdata->vbox21), actdata->iconentry21, FALSE,
		      FALSE, 0);


  actdata->label25 = gtk_label_new (_("Gnome Keyboard layout applet"));
  gtk_widget_ref (actdata->label25);

  gtk_widget_show (actdata->label25);
  gtk_box_pack_start (GTK_BOX (actdata->vbox21), actdata->label25, TRUE, TRUE,
		      0);

  actdata->label1 = gtk_label_new (actdata->name);
  gtk_widget_ref (actdata->label1);
  gtk_widget_show (actdata->label1);

  gtk_notebook_set_tab_label (GTK_NOTEBOOK (gkb->notebook),
			      gtk_notebook_get_nth_page (GTK_NOTEBOOK
							 (gkb->notebook),
							 i + 1),
			      actdata->label1);

  advanced_show (gkb);

}

void
properties_dialog (AppletWidget * applet, gpointer gkbx)
{

  GKB *gkb = (GKB *) gkbx;
  int i = 0;

  GtkWidget *defpage;

  GList *list;

  if (gkb->propbox)
    {
      gtk_widget_show_now (gkb->propbox);
      gdk_window_raise (gkb->propbox->window);
      return;
    }

  for (list = gkb->tempmaps; list != NULL; list = list->next)
    {
      PropWg *actdata = list->data;
      if (actdata)
	{
	  g_free (actdata->name);
	  g_free (actdata->iconpath);
	  g_free (actdata->command);
	  g_free (actdata);
	}
    }
  g_list_free (gkb->tempmaps);

  gkb->tempmaps = copy_props (gkb);
  gkb->tn = gkb->n;

  gkb->propbox = gnome_property_box_new ();
  gtk_object_set_data (GTK_OBJECT (gkb->propbox), "propbox", gkb->propbox);
  gtk_window_set_title (GTK_WINDOW (gkb->propbox), _("GKB Properties"));
  gtk_window_set_policy (GTK_WINDOW (gkb->propbox), FALSE, FALSE, TRUE);
  gtk_widget_show (GTK_WIDGET (gkb->propbox));

  gkb->notebook = GNOME_PROPERTY_BOX (gkb->propbox)->notebook;
  gtk_object_set_data (GTK_OBJECT (gkb->propbox), "notebook", gkb->notebook);
  gtk_widget_show (gkb->notebook);
  gtk_notebook_set_scrollable (GTK_NOTEBOOK (gkb->notebook), TRUE);
  gtk_notebook_popup_enable (GTK_NOTEBOOK (gkb->notebook));
  prop_show (gkb);

  defpage = defpage_create (gkb);

  gtk_notebook_append_page (GTK_NOTEBOOK (gkb->notebook), defpage,
			    gtk_label_new (_("General")));

  list = gkb->tempmaps;
  while (list)
    {
      if (list->data)
	makenotebook (gkb, list->data, i++);
      list = list->next;
    }

  gtk_signal_connect (GTK_OBJECT (gkb->propbox),
		      "apply", GTK_SIGNAL_FUNC (apply_cb), gkb);
  gtk_signal_connect (GTK_OBJECT (gkb->propbox),
		      "destroy", GTK_SIGNAL_FUNC (destroy_cb), gkb);
  gtk_signal_connect (GTK_OBJECT (gkb->propbox),
		      "help", GTK_SIGNAL_FUNC (prophelp_cb), NULL);
  prop_show (gkb);

  return;
  applet = NULL;
}

static void
prophelp_cb (AppletWidget * applet, gpointer data)
{
  GnomeHelpMenuEntry help_entry =
    { "gkb_applet", "index.html#GKBAPPLET-PREFS" };

  gnome_help_display (NULL, &help_entry);
}
