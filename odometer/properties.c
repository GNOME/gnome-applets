/*
 * GNOME odometer panel applet
 * (C) 1999 The Free software Foundation
 * 
 * Author: Fabrice Bellet <Fabrice.Bellet@creatis.insa-lyon.fr>
 *         adapted from kodo/Xodometer/Mouspedometa
 *
 * The property dialog box is heavily inpired from the
 * sound-monitor applet. Thanks to John Ellis.
 *
 */

#include <config.h>
#include "odo.h"

static int
auto_reset_selected_cb (GtkWidget *b, gpointer data)
{
   OdoApplet *oa = data;

   oa->auto_reset=GTK_TOGGLE_BUTTON (b)->active;
   
   panel_applet_gconf_set_bool (PANEL_APPLET(oa->applet), "auto_reset",
   				oa->auto_reset, NULL);
   return FALSE;
}

static int
use_metric_selected_cb (GtkWidget *b, gpointer data)
{
   OdoApplet *oa = data;

   oa->use_metric=GTK_TOGGLE_BUTTON (b)->active;
   panel_applet_gconf_set_bool (PANEL_APPLET(oa->applet), "use_metric",
   				oa->use_metric, NULL);
   return FALSE;
}

static int
enabled_selected_cb (GtkWidget *b, gpointer data)
{
   OdoApplet *oa = data;

   oa->enabled=GTK_TOGGLE_BUTTON (b)->active;
   panel_applet_gconf_set_bool (PANEL_APPLET(oa->applet), "enabled",
   				oa->enabled, NULL);
   return FALSE;
}

static int
digits_number_changed_cb (GtkWidget *widget, gpointer data)
{
   OdoApplet *oa = data;

   oa->digits_nb = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(oa->spinner));
   panel_applet_gconf_set_int (PANEL_APPLET(oa->applet), "digits_nb",
   			       oa->digits_nb, NULL);
   
   change_digits_nb (oa);
   refresh(oa);
   return FALSE;
}

static int
scale_applet_cb (GtkWidget *b, gpointer data)
{
   OdoApplet *oa = data;

   oa->scale_applet=GTK_TOGGLE_BUTTON (b)->active;
   change_theme(oa->theme_file, oa);
   panel_applet_gconf_set_bool (PANEL_APPLET(oa->applet), "scale_applet",
   				oa->scale_applet, NULL);
   return FALSE;
}

static void
theme_selected_cb (GtkTreeSelection *selection, gpointer data)
{
  OdoApplet *oa = data;
  GtkTreeModel *model;
  GtkTreeIter iter;
  gchar *theme;
  
  if (!gtk_tree_selection_get_selected (selection, &model, &iter))
		return;
		
  gtk_tree_model_get (model, &iter, 1, &theme, -1);
  
  if (theme && oa->theme_file && strcmp(theme, oa->theme_file) != 0) {
   	g_free (oa->theme_file);
   	oa->theme_file = g_strdup(theme);
   	if (!change_theme(oa->theme_file, oa))
   		change_theme(NULL, oa);
   } else if (theme && strlen(theme) != 0) {
   	if (oa->theme_file) g_free (oa->theme_file);
   	oa->theme_file = g_strdup(theme);
   	if (!change_theme(oa->theme_file, oa))
   		change_theme(NULL, oa);
   } else {
   	if (oa->theme_file) g_free (oa->theme_file);
   	oa->theme_file = NULL;
   	change_theme(NULL, oa);
   }
  
  if (theme) gtk_entry_set_text(GTK_ENTRY(oa->theme_entry),theme);
  
  refresh(oa);
  
  g_free (theme);
  
  return;
}

static gint
sort_theme_list_cb(void *a, void *b)
{
  return strcmp((gchar *)a, (gchar *)b);
}

static void
populate_theme_list (GtkWidget *tree)
{
   DIR *dp;
   struct dirent *dir;
   struct stat ent_sbuf;
   gchar *buf[] = { "x", };
   gint row;
   gchar *themepath;
   GList *theme_list = NULL;
   GList *list;
   GtkTreeIter iter;
   GtkListStore *model;
   
   model = GTK_LIST_STORE (gtk_tree_view_get_model (GTK_TREE_VIEW (tree)));

   /* add default theme */
   buf[0] = _("None (default)");
   gtk_list_store_insert (model, &iter, 0);
   gtk_list_store_set (model, &iter, 0, buf[0], -1);

   themepath = gnome_unconditional_datadir_file("odometer");

   if((dp = opendir(themepath))==NULL) {
   	/* dir not found */
   	g_free(themepath);
   	return;
   }

   while ((dir = readdir(dp)) != NULL) {
   	/* skips removed files and also skips the default dir */
   	if (dir->d_ino > 0
   		&& strcmp (dir->d_name, "default")) {
   		gchar *name;
   		gchar *path;

   		name = dir->d_name;
   		path = g_strconcat(themepath, "/", name, NULL);
   		if (stat(path,&ent_sbuf) >= 0 && S_ISDIR(ent_sbuf.st_mode)) {
   			theme_list = g_list_insert_sorted(theme_list,
   				g_strdup(name),
   				(GCompareFunc) sort_theme_list_cb);

   		}
   		g_free(path);
   	}
   }
   closedir(dp);

   list = theme_list;
   while (list) {
   	gchar *themedata_file = g_strconcat(themepath, "/", list->data, "/themedata", NULL);
   	if (g_file_exists(themedata_file)) {
   		gchar *theme_file = g_strconcat (themepath, "/", list->data, NULL);
   		buf[0] = list->data;
   		gtk_list_store_insert (model, &iter, 0);
   		gtk_list_store_set (model, &iter, 0, buf[0], 
   						  1, theme_file,
   						  -1);
   		g_free (theme_file);
	}
   	g_free(themedata_file);
   	g_free(list->data);
   	list = list->next;
   }
   g_list_free(theme_list);
   g_free(themepath);
}

static void
phelp_cb (GtkWidget *w, gint tab, gpointer data)
{
#ifdef FIXME
	GnomeHelpMenuEntry help_entry = { 
		"odometer_applet", "index.html#ODOMETER-PREFS"
	};
	gnome_help_display (NULL, &help_entry);
#endif
}

static void
response_cb (GtkDialog *dialog, gint id, gpointer data)
{
    OdoApplet *oa = data;
    gtk_widget_destroy (oa->pbox);
    oa->pbox = NULL;
	
}

/*
 * Callback to access properties of the applet: you can toggle:
 *   - the Metric mode
 *   - the trip auto-reset mode each time the applet is restarted
 *   - the enable/disable mode
 */
void
properties_cb (BonoboUIComponent *uic, gpointer data, const gchar *verbname)
{
   OdoApplet *oa = data;
   GtkWidget *notebook;
   GtkWidget *label;
   GtkWidget *frame;
   GtkWidget *frame2;
   GtkWidget *hbox;
   GtkWidget *vbox;
   GtkWidget *scrolled;
   GtkWidget *tree;
   GtkListStore *model;
   GtkTreeViewColumn *column;
   GtkCellRenderer *cell;
   GtkTreeSelection *selection;
   GtkAdjustment *adj;
   gchar *theme_title[] = { N_("Themes:"), NULL };

   if (oa->pbox) {
   	gdk_window_raise (oa->pbox->window);
   	return;
   }

   oa->pbox=gtk_dialog_new_with_buttons (_("Properties"), NULL,
					 GTK_DIALOG_DESTROY_WITH_PARENT,
					 GTK_STOCK_OK, GTK_RESPONSE_OK,
					 NULL);
					 
   notebook = gtk_notebook_new ();
   gtk_box_pack_start (GTK_BOX (GTK_DIALOG (oa->pbox)->vbox), notebook, TRUE, TRUE, 0);
   
   /* General Tab */

   frame = gtk_frame_new(NULL);
   gtk_container_set_border_width (GTK_CONTAINER (frame), GNOME_PAD_SMALL);

   vbox = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
   gtk_container_set_border_width (GTK_CONTAINER (vbox), GNOME_PAD_SMALL);
   gtk_container_add(GTK_CONTAINER(frame), vbox);

   label = gtk_check_button_new_with_label (_("Use Metric"));
   gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(label),oa->use_metric);
   gtk_signal_connect (GTK_OBJECT(label), "clicked", (GtkSignalFunc)
   	use_metric_selected_cb,oa);

   label = gtk_check_button_new_with_label (_("auto_reset"));
   gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(label),oa->auto_reset);
   gtk_signal_connect (GTK_OBJECT(label), "clicked", (GtkSignalFunc)
   	auto_reset_selected_cb,oa);

   label = gtk_check_button_new_with_label (_("enabled"));
   gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(label),oa->enabled);
   gtk_signal_connect (GTK_OBJECT(label), "clicked", (GtkSignalFunc)
   	enabled_selected_cb,oa);

   frame2 = gtk_frame_new (_("Digits number"));
   gtk_box_pack_start (GTK_BOX (vbox), frame2, FALSE, FALSE, 0);
   gtk_container_set_border_width (GTK_CONTAINER (frame2), 8);

   adj = (GtkAdjustment *) gtk_adjustment_new ((gdouble)oa->digits_nb, 1.0, 10.0, 1.0, 1.0, 0.0);
   oa->spinner = gtk_spin_button_new (adj,1.0,0);
   gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (oa->spinner),TRUE);
   gtk_container_add (GTK_CONTAINER (frame2), oa->spinner);
   gtk_signal_connect (GTK_OBJECT (adj),"value_changed",
   		       GTK_SIGNAL_FUNC (digits_number_changed_cb),oa);

   label = gtk_check_button_new_with_label (_("Scale size to panel"));
   gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(label),oa->scale_applet);
   gtk_signal_connect (GTK_OBJECT(label), "clicked", (GtkSignalFunc)
   	scale_applet_cb, oa);

   gtk_widget_show(frame);

   label = gtk_label_new(_("General"));
   gtk_notebook_append_page (GTK_NOTEBOOK (notebook), frame, label);

   /* Theme Tab */

   frame = gtk_frame_new(NULL);
   gtk_container_set_border_width (GTK_CONTAINER (frame), GNOME_PAD_SMALL);

   vbox = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
   gtk_container_set_border_width (GTK_CONTAINER (vbox), GNOME_PAD_SMALL);
   gtk_container_add(GTK_CONTAINER(frame), vbox);

   hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
   gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

   label = gtk_label_new(_("Theme file:"));
   gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

   oa->theme_entry = gtk_entry_new();
   if (oa->theme_file)
   	gtk_entry_set_text(GTK_ENTRY(oa->theme_entry), oa->theme_file);
   gtk_editable_set_editable (GTK_EDITABLE (oa->theme_entry), FALSE);
   gtk_box_pack_start(GTK_BOX(hbox),oa->theme_entry , TRUE, TRUE, 0);
   gtk_widget_show(oa->theme_entry);

   scrolled = gtk_scrolled_window_new(NULL, NULL);
   gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
   	GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
   gtk_box_pack_start(GTK_BOX(vbox), scrolled, TRUE, TRUE, 0);

   /* theme list */
   
   model = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);
   tree = gtk_tree_view_new_with_model (GTK_TREE_MODEL (model));
   gtk_container_add (GTK_CONTAINER (scrolled), tree);
   g_object_unref (model);
	
   cell = gtk_cell_renderer_text_new ();
   column = gtk_tree_view_column_new_with_attributes (theme_title[0], cell,
                                                      "text", 0, NULL);
   gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);
                                                           
   selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree));
   g_signal_connect (G_OBJECT (selection), "changed",
        	     G_CALLBACK (theme_selected_cb), oa);

   populate_theme_list(tree);

   label = gtk_label_new(_("Theme"));
   gtk_widget_show(frame);
   gtk_notebook_append_page (GTK_NOTEBOOK (notebook), frame, label);

   g_signal_connect (G_OBJECT (oa->pbox), "response",
   		     G_CALLBACK (response_cb), oa);
   gtk_widget_show_all(oa->pbox);
   return;
}
