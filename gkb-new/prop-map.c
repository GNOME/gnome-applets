/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 2 -*- */
/* File: prop-map.c
 * Purpose: GNOME Keyboard switcher keyprop editor property box
 *
 * Copyright (C) 1998-2000 Free Software Foundation
 * Authors: Szabolcs BAN <shooby@gnome.hu>
 *          Chema Celorio <chema@celorio.com>
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

#include <egg-screen-help.h>

extern gboolean gail_loaded;
static GtkWidget *label_prop_map = NULL;

typedef struct _GkbMapDialogInfo GkbMapDialogInfo;

struct _GkbMapDialogInfo
{
  GkbPropertyBoxInfo *pbi;
  GkbKeymap *keymap;
  GtkWidget *dialog;

  /* Buttons */
  GtkWidget *close_button;
  GtkWidget *help_button;

  /* Entries */
  GtkWidget *name_entry;
  GtkWidget *label_entry;
  GtkWidget *command_entry;
  GtkWidget *flag_entry;

};

/**
 * gkb_prop_map_close_clicked:
 * @mdi: 
 * 
 * Handle the close button clicked event
 **/
static void
gkb_prop_map_close_clicked (GkbMapDialogInfo * mdi)
{
  GkbKeymap *keymap;
  gchar *name, *label, *command, *flag;
                                                                                
  keymap = mdi->keymap;
                                                                                
  name = g_strdup (gtk_entry_get_text (GTK_ENTRY (mdi->name_entry)));
  if (keymap->name)
	g_free (keymap->name);
  if (name && name[0])
	keymap->name = g_strdup (name);
  else
	keymap->name = g_strdup (_("Unknown Keyboard"));
  if (name)
	g_free (name);

  label =
    g_strdup (gtk_entry_get_text (GTK_ENTRY (mdi->label_entry)));
  if (keymap->label)
	g_free (keymap->label);
  if (label && label[0])
	keymap->label = g_strdup (label);
  else  
	keymap->label = g_strdup (_("?"));
  if (label)
	g_free (label);

  command =
    g_strdup (gtk_entry_get_text (GTK_ENTRY (mdi->command_entry)));
  if (command) {
  	 if  (keymap->command)
  		g_free (keymap->command);
  	keymap->command = g_strdup (command);
	g_free (command);
  }
  flag =
    g_strdup (g_basename
              (gnome_icon_entry_get_filename
               (GNOME_ICON_ENTRY (mdi->flag_entry))));
  if (flag) {
  	 if  (keymap->flag)
  		g_free (keymap->flag);
  	keymap->flag = g_strdup (flag);
	g_free (flag);
  }
                                                                              
  mdi->pbi->selected_keymap = keymap;
  gkb_prop_list_reload (mdi->pbi);
  gkb_apply(mdi->pbi);
  gtk_widget_destroy (mdi->dialog);
  gkb_save_session (mdi->pbi->gkb);
  mdi->dialog = NULL;
}

/**
 * gkb_prop_map_help_clicked:
 * @mdi: 
 * 
 * Handle the help button clicked event
 **/
static void
gkb_prop_map_help_clicked (GkbMapDialogInfo * mdi)
{
	GError *error = NULL;

        egg_help_display_on_screen (
		"gkb", "gkb-edit-layout",
  		gtk_window_get_screen (GTK_WINDOW (mdi->dialog)),
		&error);

	/* FIXME: display error to the user */
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
gkb_prop_map_label_at (GtkWidget * table, gint row, gint col,
		       const gchar * label_text)
{
  label_prop_map = gtk_label_new_with_mnemonic (label_text);
  gtk_table_attach (GTK_TABLE (table), label_prop_map,
		    row, row + 1, col, col + 1,
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label_prop_map), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label_prop_map), 0.0, 0.5);
  gtk_label_set_use_markup (GTK_LABEL (label_prop_map), TRUE);
}

/**
 * gkb_prop_map_entry_at:
 * @table: 
 * @row: 
 * @col: 
 * @entry_text: 
 * 
 * Create a entry and add it to a table at a specified position
 * 
 * Return Value: 
 **/
static GtkWidget *
gkb_prop_map_entry_at (GtkWidget * table, gint row, gint col,
		       GkbMapDialogInfo * mdi, const gchar * entry_text)
{
  GtkWidget *entry;

  entry = gtk_entry_new ();
  if (entry_text)
    gtk_entry_set_text (GTK_ENTRY (entry), entry_text);

  gtk_table_attach (GTK_TABLE (table), entry,
		    row, row + 1, col, col + 1,
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		    (GtkAttachOptions) (0), 0, 0);

  gtk_label_set_mnemonic_widget( GTK_LABEL(label_prop_map), entry);
  gtk_entry_set_activates_default (GTK_ENTRY (entry), TRUE);
  if (gail_loaded)
    {
      add_atk_relation(entry, label_prop_map, ATK_RELATION_LABELLED_BY);
    }
  return entry;
}

/**
 * gkb_prop_map_icon_entry_at:
 * @table: 
 * @row: 
 * @col: 
 * @entry_text: 
 * 
 * Create an icon entry and add it to a table at a specified position
 * 
 * Return Value: 
 **/
static GtkWidget *
gkb_prop_map_icon_entry_at (GtkWidget * table, gint row, gint col,
                            GkbMapDialogInfo * mdi, const gchar * entry_text)
{
  GtkWidget *entry;
  GtkWidget *hbox;
  
  hbox = gtk_hbox_new (FALSE, 0);
  entry = gnome_icon_entry_new (NULL, NULL);
 
  gtk_table_attach (GTK_TABLE (table), hbox,
		    row, row + 1, col, col + 1,
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL), 0, 0);
 
  gtk_box_pack_start (GTK_BOX (hbox), entry, FALSE, FALSE, 0);

  gtk_label_set_mnemonic_widget( GTK_LABEL(label_prop_map), entry);
  if (gail_loaded)
    {
      add_atk_relation(entry, label_prop_map, ATK_RELATION_LABELLED_BY);
    }
  return entry;
}

static void
window_response (GtkWidget *w, int response, gpointer data)
{
  GkbMapDialogInfo * mdi = data;
  if (response == GTK_RESPONSE_HELP)
    gkb_prop_map_help_clicked (mdi);
  else {
   gkb_prop_map_close_clicked (mdi);
  }
}

/**
* gkb_prop_map_edit:
* @pbi:
*
* Implement the map edit dialog
**/
void
gkb_prop_map_edit (GkbPropertyBoxInfo * pbi)
{
  GkbMapDialogInfo *mdi;
  GtkWidget *vbox1;
  GtkWidget *table1;
  GkbKeymap *keymap;

  if (pbi->selected_keymap == NULL)
    {
      g_warning ("Why is the edit button active");
      return;
    }

  mdi = g_new0 (GkbMapDialogInfo, 1);
  mdi->pbi = pbi;
  mdi->keymap = pbi->selected_keymap;

  keymap = pbi->selected_keymap;

  mdi->dialog = gtk_dialog_new_with_buttons (_("Edit Keyboard"), NULL,
  				GTK_DIALOG_DESTROY_WITH_PARENT,
  				GTK_STOCK_HELP, GTK_RESPONSE_HELP,
  				GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
  				NULL);

  gtk_window_set_transient_for (GTK_WINDOW (mdi->dialog), GTK_WINDOW (pbi->box));
  gtk_window_set_modal (GTK_WINDOW (mdi->dialog), TRUE);

  gtk_dialog_set_has_separator (GTK_DIALOG (mdi->dialog), FALSE);
  gtk_dialog_set_default_response (GTK_DIALOG (mdi->dialog), GTK_RESPONSE_CLOSE);
  gtk_container_set_border_width (GTK_CONTAINER (mdi->dialog), 5);
  gtk_window_set_resizable (GTK_WINDOW (mdi->dialog), FALSE);

  vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (vbox1), 5);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (mdi->dialog)->vbox), vbox1,
                        TRUE, TRUE, 0);

  table1 = gtk_table_new (4, 2, FALSE);
  gtk_box_pack_start (GTK_BOX (vbox1), table1, TRUE, TRUE, 0);
  gtk_table_set_row_spacings (GTK_TABLE (table1), 6);
  gtk_table_set_col_spacings (GTK_TABLE (table1), 12);

  gkb_prop_map_label_at (table1, 0, 0, _("_Name:"));
  mdi->name_entry = gkb_prop_map_entry_at (table1, 1, 0, mdi, keymap->name);
  gtk_widget_set_size_request (mdi->name_entry, 225, -1);

  gkb_prop_map_label_at (table1, 0, 1, _("_Label:"));
  mdi->label_entry = gkb_prop_map_entry_at (table1, 1, 1, mdi, keymap->label);

  gkb_prop_map_label_at (table1, 0, 2, _("Co_mmand:"));
  mdi->command_entry = gkb_prop_map_entry_at (table1, 1, 2, mdi, keymap->command);

  gkb_prop_map_label_at (table1, 0, 3, _("_Flag:"));
  mdi->flag_entry = gkb_prop_map_icon_entry_at (table1, 1, 3, mdi, keymap->flag);

  gnome_icon_entry_set_pixmap_subdir (GNOME_ICON_ENTRY (mdi->flag_entry), "gkb");
  gnome_icon_entry_set_filename (GNOME_ICON_ENTRY (mdi->flag_entry), keymap->flag);

  gtk_widget_show_all (table1);

  if (gail_loaded)
   {
      add_atk_relation(mdi->flag_entry, label_prop_map, ATK_RELATION_LABELLED_BY);
      add_atk_relation(label_prop_map, mdi->flag_entry, ATK_RELATION_LABEL_FOR);
   }
                                                              
  g_signal_connect (G_OBJECT (mdi->dialog), "response",
                    G_CALLBACK (window_response),
                    mdi);

  gtk_widget_show_all (vbox1);

  /* Go, go go !! */
  gtk_widget_show_all (mdi->dialog);

}
