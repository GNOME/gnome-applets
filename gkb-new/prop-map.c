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

typedef struct _GkbMapDialogInfo GkbMapDialogInfo;

struct _GkbMapDialogInfo
{
  GkbPropertyBoxInfo *pbi;
  GkbKeymap *keymap;
  GtkWidget *dialog;
  gboolean changed;

  /* Buttons */
  GtkWidget *ok_button;
  GtkWidget *apply_button;
  GtkWidget *close_button;
  GtkWidget *help_button;

  /* Entries */
  GtkWidget *name_entry;
  GtkWidget *label_entry;
  GtkWidget *language_entry;
  GtkWidget *country_entry;
  GtkWidget *codepage_entry;
  GtkWidget *type_entry;
  GtkWidget *arch_entry;
  GtkWidget *command_entry;

  GtkWidget *icon_entry;

};




/**
 * gkb_prop_map_apply_clicked:
 * @mdi: 
 * 
 * Update the selected keymap settings when the apply button is clicked
 **/
static void
gkb_prop_map_apply_clicked (GkbMapDialogInfo * mdi)
{
  GkbKeymap *keymap;

  keymap = mdi->keymap;

  gkb_keymap_free_internals (keymap);

  keymap->name = g_strdup (gtk_entry_get_text (GTK_ENTRY (mdi->name_entry)));
  keymap->label =
    g_strdup (gtk_entry_get_text (GTK_ENTRY (mdi->label_entry)));
  keymap->lang =
    g_strdup (gtk_entry_get_text (GTK_ENTRY (mdi->language_entry)));
  keymap->country =
    g_strdup (gtk_entry_get_text (GTK_ENTRY (mdi->country_entry)));
  keymap->codepage =
    g_strdup (gtk_entry_get_text (GTK_ENTRY (mdi->codepage_entry)));
  keymap->type = g_strdup (gtk_entry_get_text (GTK_ENTRY (mdi->type_entry)));
  keymap->arch = g_strdup (gtk_entry_get_text (GTK_ENTRY (mdi->arch_entry)));
  keymap->command =
    g_strdup (gtk_entry_get_text (GTK_ENTRY (mdi->command_entry)));
  keymap->flag =
    g_strdup (g_basename
	      (gnome_icon_entry_get_filename
	       (GNOME_ICON_ENTRY (mdi->icon_entry))));

  gkb_prop_list_reload (mdi->pbi);

  gnome_property_box_changed (GNOME_PROPERTY_BOX (mdi->pbi->box));

  gtk_widget_set_sensitive (mdi->apply_button, FALSE);
  mdi->changed = FALSE;
}

/**
 * gkb_prop_map_ok_clicked:
 * @mdi: 
 * 
 * Handle the clos button clicked event
 **/
static void
gkb_prop_map_close_clicked (GkbMapDialogInfo * mdi)
{
  gtk_widget_destroy (mdi->dialog);
  mdi->dialog = NULL;
}


/**
 * gkb_prop_map_ok_clicked:
 * @mdi: 
 * 
 * Handle the ok button clicked event
 **/
static void
gkb_prop_map_ok_clicked (GkbMapDialogInfo * mdi)
{
  if (mdi->changed)
    gkb_prop_map_apply_clicked (mdi);
  gkb_prop_map_close_clicked (mdi);
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
  GnomeHelpMenuEntry help_entry =
    { "gkb_applet", "index.html#GKBAPPLET-PREFS-EDIT" };

  gnome_help_display (NULL, &help_entry);
}


/**
 * gkb_prop_map_button_clicked:
 * @widget: 
 * @dummy: 
 * 
 * Handle the button clicked event for the map dialog
 **/
static void
gkb_prop_map_button_clicked (GtkWidget * widget, GkbMapDialogInfo * mdi)
{
  if (widget == mdi->help_button)
    gkb_prop_map_help_clicked (mdi);
  else if (widget == mdi->apply_button)
    gkb_prop_map_apply_clicked (mdi);
  else if (widget == mdi->ok_button)
    gkb_prop_map_ok_clicked (mdi);
  else if (widget == mdi->close_button)
    gkb_prop_map_close_clicked (mdi);
}

/**
 * gkb_prop_map_get_countries:
 * @pbi: 
 * 
 * Returns a list of const gchar pointers of the selectable countires
 * 
 * Return Value: 
 **/
static GList *
gkb_prop_map_get_countries (GkbPropertyBoxInfo * pbi)
{
  GList *list = NULL;

  list = g_list_prepend (list, _("Armenia"));
  list = g_list_prepend (list, _("Australia"));
  list = g_list_prepend (list, _("Austria"));
  list = g_list_prepend (list, _("Belgium"));
  list = g_list_prepend (list, _("Brazil"));
  list = g_list_prepend (list, _("Bulgaria"));
  list = g_list_prepend (list, _("Canada"));
  list = g_list_prepend (list, _("Caribic"));
  list = g_list_prepend (list, _("France"));
  list = g_list_prepend (list, _("Georgia"));
  list = g_list_prepend (list, _("Germany"));
  list = g_list_prepend (list, _("Great Britain"));
  list = g_list_prepend (list, _("Hungary"));
  list = g_list_prepend (list, _("Jamaica"));
  list = g_list_prepend (list, _("New Zealand"));
  list = g_list_prepend (list, _("Norway"));
  list = g_list_prepend (list, _("Poland"));
  list = g_list_prepend (list, _("Portugal"));
  list = g_list_prepend (list, _("Russia"));
  list = g_list_prepend (list, _("Slovak Republic"));
  list = g_list_prepend (list, _("Slovenia"));
  list = g_list_prepend (list, _("South Africa"));
  list = g_list_prepend (list, _("Spain"));
  list = g_list_prepend (list, _("Sweden"));
  list = g_list_prepend (list, _("Switzerland"));
  list = g_list_prepend (list, _("Thailand"));
  list = g_list_prepend (list, _("Turkey"));
  list = g_list_prepend (list, _("United Kingdom"));
  list = g_list_prepend (list, _("United States"));
  list = g_list_prepend (list, _("Yugoslavia"));

  return g_list_reverse (list);
}



/**
 * gkb_prop_map_get_languages:
 * @pbi: 
 * 
 * Returns a list of const gchar pointers of the selectable languages
 * 
 * Return Value: 
 **/
static GList *
gkb_prop_map_get_languages (GkbPropertyBoxInfo * pbi)
{
  GList *list = NULL;

  list = g_list_prepend (list, _("Armenian"));
  list = g_list_prepend (list, _("Basque"));
  list = g_list_prepend (list, _("Bulgarian"));
  list = g_list_prepend (list, _("Dutch"));
  list = g_list_prepend (list, _("English"));
  list = g_list_prepend (list, _("French"));
  list = g_list_prepend (list, _("Georgian"));
  list = g_list_prepend (list, _("German"));
  list = g_list_prepend (list, _("Hungarian"));
  list = g_list_prepend (list, _("Norwegian"));
  list = g_list_prepend (list, _("Polish"));
  list = g_list_prepend (list, _("Portuguese"));
  list = g_list_prepend (list, _("Russian"));
  list = g_list_prepend (list, _("Slovak"));
  list = g_list_prepend (list, _("Slovenian"));
  list = g_list_prepend (list, _("Swedish"));
  list = g_list_prepend (list, _("Thai"));
  list = g_list_prepend (list, _("Turkish"));
  list = g_list_prepend (list, _("Wallon"));
  list = g_list_prepend (list, _("Yugoslavian"));

  return g_list_reverse (list);
}

/**
 * gkb_prop_map_get_types:
 * @pbi: 
 * 
 * Returns a list of const gchar pointers of the selectable keyboar types
 * 
 * Return Value: 
 **/
static GList *
gkb_prop_map_get_types (GkbPropertyBoxInfo * pbi)
{
  GList *list = NULL;

  list = g_list_prepend (list, _("105"));
  list = g_list_prepend (list, _("101"));
  list = g_list_prepend (list, _("102"));
  list = g_list_prepend (list, _("450"));
  list = g_list_prepend (list, _("84"));
  list = g_list_prepend (list, _("mklinux"));
  list = g_list_prepend (list, _("type5"));

  return g_list_reverse (list);
}


/**
 * gkb_prop_map_get_codepages:
 * @pbi: 
 * 
 *
 * Returns a list of const gchar pointers of the selectable codepagess
 * 
 * Return Value: 
 **/
static GList *
gkb_prop_map_get_codepages (GkbPropertyBoxInfo * pbi)
{
  GList *list = NULL;

  list = g_list_prepend (list, _("iso-8859-1"));
  list = g_list_prepend (list, _("iso-8859-2"));
  list = g_list_prepend (list, _("iso-8859-9"));
  list = g_list_prepend (list, _("am-armscii8"));
  list = g_list_prepend (list, _("be-latin1"));
  list = g_list_prepend (list, _("cp1251"));
  list = g_list_prepend (list, _("georgian-academy"));
  list = g_list_prepend (list, _("koi8-r"));
  list = g_list_prepend (list, _("tis620"));

  return g_list_reverse (list);
}

/**
 * gkb_prop_map_get_arquitectures:
 * @pbi: 
 * 
 * Returns a list of const gchar pointers of the selectable arquitectures
 * 
 * Return Value: 
 **/
static GList *
gkb_prop_map_get_arquitectures (GkbPropertyBoxInfo * pbi)
{
  GList *list = NULL;

  list = g_list_prepend (list, _("ix86"));
  list = g_list_prepend (list, _("sun"));
  list = g_list_prepend (list, _("mac"));
  list = g_list_prepend (list, _("sgi"));
  list = g_list_prepend (list, _("dec"));
  list = g_list_prepend (list, _("ibm"));

  return g_list_reverse (list);
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
  GtkWidget *label;

  label = gtk_label_new (label_text);
  gtk_table_attach (GTK_TABLE (table), label,
		    row, row + 1, col, col + 1,
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (label), 1, 0.5);
}

/**
 * gkb_prop_map_load_stock_button:
 * @stock_button: 
 * @container: 
 * 
 * Load a stock buttin
 * 
 * Return Value: The newly created button, NULL on error
 **/
static GtkWidget *
gkb_prop_map_load_stock_button (const gchar * stock_button,
				GtkContainer * container,
				GkbMapDialogInfo * mdi)
{
  GtkWidget *button;

  button = gnome_stock_button (stock_button);

  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (gkb_prop_map_button_clicked), mdi);

  gtk_container_add (GTK_CONTAINER (container), button);
  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);

  return button;
}



/**
 * gkb_prop_map_data_changed:
 * @mdi: 
 * 
 * Call whenever data is changed so that we can update the apply
 * button sensitivity
 **/
static void
gkb_prop_map_data_changed (GtkWidget * unused, GkbMapDialogInfo * mdi)
{
  if (mdi->changed)
    return;

  mdi->changed = TRUE;

  gtk_widget_set_sensitive (mdi->apply_button, TRUE);
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

  gtk_signal_connect (GTK_OBJECT (entry), "changed",
		      GTK_SIGNAL_FUNC (gkb_prop_map_data_changed), mdi);

  return entry;
}

/**
 * gkb_prop_map_combo_at:
 * @table: 
 * @row: 
 * @col: 
 * @list: 
 * @text: 
 * 
 * Create a combo menu item and add it to a table in a specifid location
 * 
 * Return Value: 
 **/
static GtkWidget *
gkb_prop_map_combo_at (GtkWidget * table, gint row, gint col,
		       GList * list, GkbMapDialogInfo * mdi,
		       const gchar * text)
{
  GtkWidget *combo;
  GtkWidget *entry;

  combo = gtk_combo_new ();

  gtk_table_attach (GTK_TABLE (table), combo,
		    row, row + 1, col, col + 1,
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		    (GtkAttachOptions) (0), 0, 0);

  gtk_combo_set_popdown_strings (GTK_COMBO (combo), list);

  entry = GTK_COMBO (combo)->entry;

  if (text)
    gtk_entry_set_text (GTK_ENTRY (entry), text);

  gtk_signal_connect (GTK_OBJECT (entry), "changed",
		      GTK_SIGNAL_FUNC (gkb_prop_map_data_changed), mdi);

  return entry;
}

/**
 * gkb_prop_map_pixmap_at:
 * @table: 
 * @row: 
 * @col: 
 * @flag: 
 * 
 * Load a pixmap and add it to a label at a specified position
 **/
static GtkWidget *
gkb_prop_map_pixmap_at (GtkWidget * table, gint row, gint col,
			GkbMapDialogInfo * mdi, const gchar * flag)
{
  GtkWidget *icon_entry;

  icon_entry = gnome_icon_entry_new (NULL, NULL);

  gnome_icon_entry_set_pixmap_subdir (GNOME_ICON_ENTRY (icon_entry), "gkb");

  gtk_table_attach (GTK_TABLE (table), icon_entry,
		    row, row + 1, col, col + 1,
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL), 0, 0);

  gnome_icon_entry_set_icon (GNOME_ICON_ENTRY (icon_entry), flag);

  gtk_signal_connect (GTK_OBJECT (GNOME_ICON_ENTRY (icon_entry)->pickbutton),
		      "clicked", GTK_SIGNAL_FUNC (gkb_prop_map_data_changed),
		      mdi);

  return icon_entry;
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
  GtkContainer *button_box;
  GkbKeymap *keymap;
  GtkWidget *dialog;
  GtkWidget *left_table;
  GtkWidget *right_table;
  GtkWidget *vseparator;
  GtkWidget *entry;
  GtkWidget *label;
  GtkWidget *hbox1;
  GtkWidget *vbox2;
  GtkWidget *vbox3;
  GtkWidget *hseparator1;
  GList *list;

  if (pbi->selected_keymap == NULL)
    {
      g_warning ("Why is the edit button active");
      return;
    }

  mdi = g_new0 (GkbMapDialogInfo, 1);
  mdi->pbi = pbi;
  mdi->keymap = pbi->selected_keymap;
  mdi->changed = FALSE;

  keymap = pbi->selected_keymap;

  dialog = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  mdi->dialog = dialog;
  gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
  gtk_object_set_data (GTK_OBJECT (dialog), "mapedit", dialog);
  gtk_window_set_title (GTK_WINDOW (dialog), _("Edit keymap"));

  vbox2 = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (dialog), vbox2);

  hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox2), hbox1, TRUE, TRUE, 0);
  vseparator = gtk_vseparator_new ();
  gtk_box_pack_start (GTK_BOX (hbox1), vseparator, TRUE, TRUE, 0);

  /* Create the right table */
  right_table = gtk_table_new (5, 2, FALSE);
  gtk_box_pack_start (GTK_BOX (hbox1), right_table, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (right_table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (right_table), 8);
  gtk_table_set_col_spacings (GTK_TABLE (right_table), 5);

  /* Create the left table */
  vbox3 = gtk_vbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox1), vbox3, TRUE, TRUE, 0);

  left_table = gtk_table_new (4, 2, FALSE);
  gtk_box_pack_start (GTK_BOX (vbox3), left_table, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (left_table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (left_table), 8);
  gtk_table_set_col_spacings (GTK_TABLE (left_table), 5);

  /* Add the labels */
  gkb_prop_map_label_at (right_table, 0, 0, _("Name"));
  gkb_prop_map_label_at (right_table, 0, 1, _("Label"));
  gkb_prop_map_label_at (right_table, 0, 2, _("Language"));
  gkb_prop_map_label_at (right_table, 0, 3, _("Country"));
  gkb_prop_map_label_at (right_table, 0, 4, _("Flag\nPixmap"));

  gkb_prop_map_label_at (left_table, 0, 0, _("Architecture"));
  gkb_prop_map_label_at (left_table, 0, 1, _("Type"));
  gkb_prop_map_label_at (left_table, 0, 2, _("Codepage"));
  gkb_prop_map_label_at (left_table, 0, 3, _("Command"));

  /* Add the entries */
  mdi->name_entry =
    gkb_prop_map_entry_at (right_table, 1, 0, mdi, keymap->name);
  mdi->label_entry =
    gkb_prop_map_entry_at (right_table, 1, 1, mdi, keymap->label);
  mdi->command_entry =
    gkb_prop_map_entry_at (left_table, 1, 3, mdi, keymap->command);

  /* Add the combos */
  list = gkb_prop_map_get_languages (pbi);
  entry = gkb_prop_map_combo_at (right_table, 1, 2, list, mdi, keymap->lang);
  mdi->language_entry = entry;
  g_list_free (list);

  list = gkb_prop_map_get_countries (pbi);
  entry =
    gkb_prop_map_combo_at (right_table, 1, 3, list, mdi, keymap->country);
  mdi->country_entry = entry;
  g_list_free (list);

  list = gkb_prop_map_get_arquitectures (pbi),
    entry = gkb_prop_map_combo_at (left_table, 1, 0, list, mdi, keymap->arch);
  mdi->arch_entry = entry;
  g_list_free (list);

  list = gkb_prop_map_get_types (pbi);
  entry = gkb_prop_map_combo_at (left_table, 1, 1, list, mdi, keymap->type);
  mdi->type_entry = entry;
  g_list_free (list);

  list = gkb_prop_map_get_codepages (pbi);
  entry =
    gkb_prop_map_combo_at (left_table, 1, 2, list, mdi, keymap->codepage);
  mdi->codepage_entry = entry;
  g_list_free (list);

  /* Add the flag pixmap */
  mdi->icon_entry =
    gkb_prop_map_pixmap_at (right_table, 1, 4, mdi, keymap->flag);

  /* Add the where comand can be label */
  label = gtk_label_new ("");
  gtk_label_parse_uline (GTK_LABEL (label),
			 _("Where command can be:\n"
			   "* xmodmap /full/path/xmodmap.hu\n"
			   "* gkb__xmmap hu\n" "* setxkbmap hu"));
  gtk_box_pack_start (GTK_BOX (vbox3), label, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);

  /* Load buttons */
  hseparator1 = gtk_hseparator_new ();
  gtk_box_pack_start (GTK_BOX (vbox2), hseparator1, TRUE, TRUE, 0);

  button_box = GTK_CONTAINER (gtk_hbutton_box_new ());
  gtk_box_pack_start (GTK_BOX (vbox2), GTK_WIDGET (button_box), TRUE, TRUE,
		      0);
  mdi->ok_button =
    gkb_prop_map_load_stock_button (GNOME_STOCK_BUTTON_OK, button_box, mdi);
  mdi->apply_button =
    gkb_prop_map_load_stock_button (GNOME_STOCK_BUTTON_APPLY, button_box,
				    mdi);
  mdi->close_button =
    gkb_prop_map_load_stock_button (GNOME_STOCK_BUTTON_CLOSE, button_box,
				    mdi);
  mdi->help_button =
    gkb_prop_map_load_stock_button (GNOME_STOCK_BUTTON_HELP, button_box, mdi);
  gtk_widget_set_sensitive (mdi->apply_button, FALSE);

  /* Go, go go !! */
  gtk_widget_show_all (dialog);

  return;
}
