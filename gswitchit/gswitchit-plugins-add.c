/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <config.h>

#include <string.h>
#include <sys/stat.h>

#include <gdk/gdkx.h>
#include <gnome.h>
#include <glade/glade.h>
#include <libbonobo.h>

#include <libxklavier/xklavier.h>
#include <libxklavier/xklavier_config.h>

#include "gswitchit-plugins.h"

static void
CappletAddAvailablePluginFunc (const char *fullPath,
			       GSwitchItPluginManagerRecord * rec,
			       GSwitchItPluginsCapplet * gswic)
{
	GtkListStore *availablePluginsModel;
	GtkTreeIter iter;
	const GSwitchItPlugin *plugin = rec->plugin;

	if (NULL !=
	    g_slist_find_custom (gswic->appletConfig.enabledPlugins,
				 fullPath, (GCompareFunc) strcmp))
		return;

	availablePluginsModel =
	    GTK_LIST_STORE (g_object_get_data (G_OBJECT (gswic->capplet),
					       "gswitchit_plugins_add.availablePluginsModel"));
	if (availablePluginsModel == NULL)
		return;

	if (plugin != NULL) {
		gtk_list_store_append (availablePluginsModel, &iter);
		gtk_list_store_set (availablePluginsModel, &iter,
				    NAME_COLUMN, plugin->name,
				    FULLPATH_COLUMN, fullPath, -1);
	}
}

static void
CappletFillAvailablePluginList (GtkTreeView *
				availablePluginsList,
				GSwitchItPluginsCapplet * gswic)
{
	GtkListStore *availablePluginsModel =
	    GTK_LIST_STORE (gtk_tree_view_get_model
			    (GTK_TREE_VIEW (availablePluginsList)));
	GSList *pluginPathNode = gswic->appletConfig.enabledPlugins;
	GHashTable *allPluginRecs = gswic->pluginManager.allPluginRecs;

	gtk_list_store_clear (availablePluginsModel);
	if (allPluginRecs == NULL)
		return;

	g_object_set_data (G_OBJECT (gswic->capplet),
			   "gswitchit_plugins_add.availablePluginsModel",
			   availablePluginsModel);
	g_hash_table_foreach (allPluginRecs,
			      (GHFunc) CappletAddAvailablePluginFunc,
			      gswic);
	g_object_set_data (G_OBJECT (gswic->capplet),
			   "gswitchit_plugins_add.availablePluginsModel",
			   NULL);
	pluginPathNode = g_slist_next (pluginPathNode);
}

static void
CappletAvailablePluginsSelectionChanged (GtkTreeSelection *
					 selection,
					 GSwitchItPluginsCapplet * gswic)
{
	GtkWidget *availablePluginsList =
	    GTK_WIDGET (gtk_tree_selection_get_tree_view (selection));
	gboolean isAnythingSelected = FALSE;
	GtkWidget *lblDescription =
	    GTK_WIDGET (g_object_get_data (G_OBJECT (gswic->capplet),
					   "gswitchit_plugins_add.lblDescription"));

	char *fullPath =
	    CappletGetSelectedPluginPath (GTK_TREE_VIEW
					  (availablePluginsList),
					  gswic);
	isAnythingSelected = fullPath != NULL;
	gtk_label_set_text (GTK_LABEL (lblDescription), 
	                    g_strconcat ("<small><i>", _("No description."), "</i></small>", NULL));
	gtk_label_set_use_markup (GTK_LABEL (lblDescription), TRUE);
	
	if (fullPath != NULL) {
		const GSwitchItPlugin *plugin =
		    GSwitchItPluginManagerGetPlugin (&gswic->pluginManager,
						     fullPath);
		if (plugin != NULL && plugin->description != NULL)
			gtk_label_set_text (GTK_LABEL (lblDescription),
			                    g_strconcat ("<small><i>", 
					    plugin->description, 
					    "</i></small>", NULL));
			gtk_label_set_use_markup (GTK_LABEL (lblDescription), TRUE);
	}
	gtk_widget_set_sensitive (GTK_WIDGET
				  (g_object_get_data
				   (G_OBJECT (gswic->capplet),
				    "gswitchit_plugins_add.btnOK")),
				  isAnythingSelected);
}

void
CappletEnablePlugin (GtkWidget * btnAdd, GSwitchItPluginsCapplet * gswic)
{
	GladeXML *data = glade_xml_new (GNOME_GLADEDIR "/gswitchit-plugins.glade", "gswitchit_plugins_add", NULL);	// default domain!
	GtkWidget *popup =
	    glade_xml_get_widget (data, "gswitchit_plugins_add");
	GtkWidget *availablePluginsList;
	GtkTreeModel *availablePluginsModel;
	GtkCellRenderer *renderer =
	    GTK_CELL_RENDERER (gtk_cell_renderer_text_new ());
	GtkTreeViewColumn *column =
	    gtk_tree_view_column_new_with_attributes (NULL,
						      renderer,
						      "text",
						      0,
						      NULL);
	GtkTreeSelection *selection;
	gint response;
	availablePluginsList = glade_xml_get_widget (data, "allPlugins");
	availablePluginsModel =
	    GTK_TREE_MODEL (gtk_list_store_new
			    (2, G_TYPE_STRING, G_TYPE_STRING));
	gtk_tree_view_set_model (GTK_TREE_VIEW (availablePluginsList),
				 availablePluginsModel);
	gtk_tree_view_append_column (GTK_TREE_VIEW (availablePluginsList),
				     column);
	selection =
	    gtk_tree_view_get_selection (GTK_TREE_VIEW
					 (availablePluginsList));
	gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);
	CappletFillAvailablePluginList (GTK_TREE_VIEW
					(availablePluginsList), gswic);
	g_signal_connect (G_OBJECT (selection), "changed",
			  G_CALLBACK
			  (CappletAvailablePluginsSelectionChanged),
			  gswic);
	g_object_set_data (G_OBJECT (gswic->capplet),
			   "gswitchit_plugins_add.btnOK",
			   glade_xml_get_widget (data, "btnOK"));
	g_object_set_data (G_OBJECT (gswic->capplet),
			   "gswitchit_plugins_add.lblDescription",
			   glade_xml_get_widget (data, "lblDescription"));
	CappletAvailablePluginsSelectionChanged (selection, gswic);
	response = gtk_dialog_run (GTK_DIALOG (popup));
	g_object_set_data (G_OBJECT (gswic->capplet),
			   "gswitchit_plugins_add.lblDescription", NULL);
	g_object_set_data (G_OBJECT (gswic->capplet),
			   "gswitchit_plugins_add.btnOK", NULL);
	gtk_widget_hide_all (popup);
	if (response == GTK_RESPONSE_OK) {
		char *fullPath =
		    CappletGetSelectedPluginPath (GTK_TREE_VIEW
						  (availablePluginsList),
						  gswic);
		if (fullPath != NULL) {
			GSwitchItPluginManagerEnablePlugin (&gswic->
							    pluginManager,
							    &gswic->
							    appletConfig.
							    enabledPlugins,
							    fullPath);
			CappletFillActivePluginList (gswic);
			g_free (fullPath);
			GSwitchItAppletConfigSaveToGConf (&gswic->appletConfig);
		}
	}
	gtk_widget_destroy (popup);
}
