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
#include <egg-screen-help.h>

typedef struct {
	GtkTreeIter iter;
	GHashTable *countries;
} LangData;
typedef struct {
	GtkTreeIter iter;
	GList *keymaps;
} CountryData;

enum {
	NAME_COL,
	COMMAND_COL,
	FLAG_COL,
	LABEL_COL,
	LANG_COL,
	COUNTRY_COL,
	NUM_COLS
};

static void
lang_data_free (LangData *lang)
{
	g_hash_table_destroy (lang->countries);
	g_free (lang);
}
static void
country_data_free (CountryData *country)
{
	g_list_free (country->keymaps); /* others have taken over the items */
	g_free (country);
}

static GtkWidget *
tree_create (GtkTreeStore *model)
{
	GList *ptr, *sets;
	GtkWidget *tree;
	GtkCellRenderer *cell;
	GtkTreeViewColumn *column;
	GtkTreeIter iter;
	CountryData *country;
	LangData    *lang;
	GHashTable  *langs = g_hash_table_new_full (
		g_str_hash, g_str_equal, NULL, (GDestroyNotify) lang_data_free);

	tree  = gtk_tree_view_new_with_model (GTK_TREE_MODEL (model));
	cell   = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (
		_("Keyboards (select and press add)"), cell,
		"text", 0, NULL);

	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (tree), FALSE);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);

	/* TODO: Error checking... */
	sets = gkb_preset_load (find_presets ());
	for (ptr = sets ; ptr != NULL ; ptr = ptr->next) {
		GkbKeymap *item = sets->data;

		/* Create lang if necessary */
		if ((lang = g_hash_table_lookup (langs, item->lang)) == NULL) {
			GtkTreeIter iter;
			gtk_tree_store_append (model, &iter, NULL);
			gtk_tree_store_set (model, &iter,
					    NAME_COL, item->lang,
					    COMMAND_COL, NULL,
					    FLAG_COL, item->flag,
					    LABEL_COL, item->label,
					    LANG_COL, item->lang,
					    COUNTRY_COL, item->country,
					    -1);

			lang = g_new0 (LangData, 1);
			lang->countries = g_hash_table_new_full (
				g_str_hash, g_str_equal, NULL, (GDestroyNotify) country_data_free);
			memcpy (&lang->iter, &iter, sizeof (lang->iter));
			g_hash_table_insert (langs, item->lang, lang);
		}

		/* Create country if necessary */
		if ((country = g_hash_table_lookup (lang->countries, item->country)) == NULL) {
			gtk_tree_store_append (model, &iter, &lang->iter);
			gtk_tree_store_set (model, &iter,
					    NAME_COL, item->country,
					    COMMAND_COL, NULL,
					    FLAG_COL, item->flag,
					    LABEL_COL, item->label,
					    LANG_COL, item->lang,
					    COUNTRY_COL, item->country,
					    -1);
			country = g_new0 (CountryData, 1);
			country->keymaps = NULL;
			memcpy (&country->iter, &iter, sizeof (lang->iter));
			g_hash_table_insert (lang->countries, item->country, country);
		}

		/* Add the item */
		gtk_tree_store_append (model, &iter, &country->iter);
		gtk_tree_store_set (GTK_TREE_STORE(model), &iter,
				    NAME_COL, item->name,
				    COMMAND_COL, item->command,
				    FLAG_COL, item->flag,
				    LABEL_COL, item->label,
				    LANG_COL, item->lang,
				    COUNTRY_COL, item->country,
				    -1);
		country->keymaps = g_list_append (country->keymaps, item);
	}

	g_hash_table_destroy (langs);

	return tree;
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

static gboolean
comparefunc (GkbKeymap *k1, GkbKeymap *k2)
{
	return ! ((k1->name == NULL || k2->name == NULL)
		? (k1->name == k2->name) : (strcmp (k1->name, k2->name) == 0))
	    || ! ((k1->country == NULL || k2->name == NULL)
		? (k1->country == k2->name) : (strcmp (k1->name, k2->name) == 0));
}

static gint
addwadd_cb (GtkWidget * addbutton, GkbPropertyBoxInfo * pbi)
{
	GKB *gkb = pbi->gkb;
	/* Do not add Language and Country rows */

	if (pbi->keymap_for_add != NULL && pbi->keymap_for_add->command != NULL) {
		if (g_list_find_custom (pbi->keymaps, pbi->keymap_for_add,
					(GCompareFunc) comparefunc) != NULL) { 
			gchar const *str = _("Keymap `%s' for the country %s already exists");
			GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (gkb->addwindow),
								    GTK_DIALOG_DESTROY_WITH_PARENT,
								    GTK_MESSAGE_INFO,
								    GTK_BUTTONS_OK,
								    str,
								    pbi->keymap_for_add->name,
								    pbi->keymap_for_add->country);

			g_signal_connect_swapped (GTK_OBJECT (dialog),
						  "response",
						  G_CALLBACK (gtk_widget_destroy),
						  GTK_OBJECT (dialog));

			gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
			gtk_widget_show (dialog);
			return FALSE;
		}

		pbi->keymaps = g_list_append (pbi->keymaps,
			gkb_keymap_copy (pbi->keymap_for_add));
	}

	gkb_prop_list_reload (pbi);
	gkb_apply (pbi);
	gkb_save_session (pbi->gkb);

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
addbutton_sensitive_cb (GtkTreeView * treeview, gpointer data)
{
	GkbPropertyBoxInfo * pbi = data;
	GKB * gkb = pbi->gkb;
	GtkWidget * addbutton = gtk_object_get_data (GTK_OBJECT (gkb->addwindow), "addbutton");

	gtk_widget_set_sensitive (addbutton, pbi->keymap_for_add->command != NULL);
}

static void
response_cb (GtkDialog *dialog, gint id, gpointer data)
{
	GkbPropertyBoxInfo * pbi = data;
	GError *error = NULL;

	switch (id) {
	case 100:
	/* Add response */
	addwadd_cb (NULL, pbi);
	break;

	case GTK_RESPONSE_HELP:
	egg_help_display_on_screen (
				    "gkb", "gkb-modify-list",
				    gtk_window_get_screen (GTK_WINDOW (dialog)),
				    &error);
	/* FIXME: display error to the user */
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
	GtkWidget *button;
	GtkWidget *scrolled1;
	GtkWidget *label;
	GtkTreeSelection *selection;
	GKB *gkb = pbi->gkb;

	if (gkb->addwindow)
	{
		gtk_window_set_screen (GTK_WINDOW (gkb->addwindow),
				       gtk_widget_get_screen (pbi->add_button));
		gtk_window_present (GTK_WINDOW (gkb->addwindow));
		return;
	}

	gkb->addwindow = gtk_dialog_new_with_buttons (_("Select Keyboard"), GTK_WINDOW (pbi->box),
						      GTK_DIALOG_DESTROY_WITH_PARENT,
						      GTK_STOCK_HELP, GTK_RESPONSE_HELP,
						      GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
						      NULL);

	button = gtk_dialog_add_button (GTK_DIALOG (gkb->addwindow), GTK_STOCK_ADD, 100);
	gtk_widget_set_sensitive (button, FALSE);

	gtk_dialog_set_default_response (GTK_DIALOG (gkb->addwindow), 100);
	gtk_dialog_set_has_separator (GTK_DIALOG (gkb->addwindow), FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (gkb->addwindow), 5);
	gtk_window_set_screen (GTK_WINDOW (gkb->addwindow),
			       gtk_widget_get_screen (pbi->add_button));
	gtk_object_set_data (GTK_OBJECT (gkb->addwindow), "addwindow",
			     gkb->addwindow);
	gtk_object_set_data (GTK_OBJECT (gkb->addwindow), "addbutton",
			     button);

	vbox1 = gtk_vbox_new (FALSE, 6); 
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (gkb->addwindow)->vbox), vbox1, FALSE, FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (vbox1), 5);
	gtk_widget_show (vbox1);

	label = gtk_label_new_with_mnemonic (_("_Keyboards (select and press add):"));
	gtk_box_pack_start (GTK_BOX (vbox1), label, FALSE, FALSE, 0);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_widget_show (label);

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
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), tree1);

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
	g_signal_connect (G_OBJECT (tree1), "cursor_changed",
			  G_CALLBACK (addbutton_sensitive_cb), pbi);

	g_signal_connect (G_OBJECT (gkb->addwindow), "response",
			  G_CALLBACK (response_cb), pbi);

	gtk_widget_show (gkb->addwindow);
}
