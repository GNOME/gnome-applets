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

extern void
CappletFillActivePluginList (GSwitchItPluginsCapplet * gswic)
{
	GtkWidget *activePlugins =
	    CappletGetGladeWidget (gswic, "activePlugins");
	GtkListStore *activePluginsModel =
	    GTK_LIST_STORE (gtk_tree_view_get_model
			    (GTK_TREE_VIEW (activePlugins)));
	GSList *pluginPathNode = gswic->appletConfig.enabledPlugins;
	GHashTable *allPluginRecs = gswic->pluginManager.allPluginRecs;

	gtk_list_store_clear (activePluginsModel);
	if (allPluginRecs == NULL)
		return;

	while (pluginPathNode != NULL) {
		GtkTreeIter iter;
		const char *fullPath = (const char *) pluginPathNode->data;
		const GSwitchItPlugin *plugin =
		    GSwitchItPluginManagerGetPlugin (&gswic->pluginManager,
						     fullPath);
		if (plugin != NULL) {
			gtk_list_store_append (activePluginsModel, &iter);
			gtk_list_store_set (activePluginsModel, &iter,
					    NAME_COLUMN, plugin->name,
					    FULLPATH_COLUMN, fullPath, -1);
		}

		pluginPathNode = g_slist_next (pluginPathNode);
	}
}

static char *
CappletGetSelectedActivePluginPath (GSwitchItPluginsCapplet * gswic)
{
	GtkTreeView *pluginsList =
	    GTK_TREE_VIEW (CappletGetGladeWidget (gswic, "activePlugins"));
	return CappletGetSelectedPluginPath (pluginsList, gswic);
}

char *
CappletGetSelectedPluginPath (GtkTreeView * pluginsList,
			      GSwitchItPluginsCapplet * gswic)
{
	GtkTreeModel *model =
	    gtk_tree_view_get_model (GTK_TREE_VIEW (pluginsList));
	GtkTreeSelection *selection =
	    gtk_tree_view_get_selection (GTK_TREE_VIEW (pluginsList));
	GtkTreeIter selectedIter;

	if (gtk_tree_selection_get_selected
	    (selection, NULL, &selectedIter)) {
		char *fullPath = NULL;

		gtk_tree_model_get (model, &selectedIter,
				    FULLPATH_COLUMN, &fullPath, -1);
		return fullPath;
	}
	return NULL;
}

static void
CappletActivePluginsSelectionChanged (GtkTreeSelection *
				      selection,
				      GSwitchItPluginsCapplet * gswic)
{
	GtkWidget *activePlugins =
	    CappletGetGladeWidget (gswic, "activePlugins");
	GtkTreeModel *model =
	    gtk_tree_view_get_model (GTK_TREE_VIEW (activePlugins));
	GtkTreeIter selectedIter;
	gboolean isAnythingSelected = FALSE;
	gboolean isFirstSelected = FALSE;
	gboolean isLastSelected = FALSE;
	gboolean hasConfigurationUi = FALSE;
	GtkWidget *btnRemove = CappletGetGladeWidget (gswic, "btnRemove");
	GtkWidget *btnUp = CappletGetGladeWidget (gswic, "btnUp");
	GtkWidget *btnDown = CappletGetGladeWidget (gswic, "btnDown");
	GtkWidget *btnProperties =
	    CappletGetGladeWidget (gswic, "btnProperties");
	GtkWidget *lblDescription =
	    CappletGetGladeWidget (gswic, "lblDescription");

	gtk_label_set_text (GTK_LABEL (lblDescription),
			    g_strconcat ("<small><i>",
					 _("No description."),
					 "</i></small>", NULL));
	gtk_label_set_use_markup (GTK_LABEL (lblDescription), TRUE);

	if (gtk_tree_selection_get_selected
	    (selection, NULL, &selectedIter)) {
		int counter = gtk_tree_model_iter_n_children (model, NULL);
		GtkTreePath *treePath =
		    gtk_tree_model_get_path (model, &selectedIter);
		gint *indices = gtk_tree_path_get_indices (treePath);
		char *fullPath =
		    CappletGetSelectedActivePluginPath (gswic);
		const GSwitchItPlugin *plugin =
		    GSwitchItPluginManagerGetPlugin (&gswic->pluginManager,
						     fullPath);

		isAnythingSelected = TRUE;

		isFirstSelected = indices[0] == 0;
		isLastSelected = indices[0] == counter - 1;

		if (plugin != NULL) {
			hasConfigurationUi =
			    (plugin->configurePropertiesCallback != NULL);
			gtk_label_set_text (GTK_LABEL (lblDescription),
					    g_strconcat ("<small><i>",
							 plugin->
							 description,
							 "</i></small>",
							 NULL));
			gtk_label_set_use_markup (GTK_LABEL
						  (lblDescription), TRUE);
		}
		g_free (fullPath);

		gtk_tree_path_free (treePath);
	}
	gtk_widget_set_sensitive (btnRemove, isAnythingSelected);
	gtk_widget_set_sensitive (btnUp, isAnythingSelected
				  && !isFirstSelected);
	gtk_widget_set_sensitive (btnDown, isAnythingSelected
				  && !isLastSelected);
	gtk_widget_set_sensitive (btnProperties, isAnythingSelected
				  && hasConfigurationUi);
}

static void
CappletPromotePlugin (GtkWidget * btnUp, GSwitchItPluginsCapplet * gswic)
{
	char *fullPath = CappletGetSelectedActivePluginPath (gswic);
	if (fullPath != NULL) {
		GSwitchItPluginManagerPromotePlugin (&gswic->pluginManager,
						     gswic->appletConfig.
						     enabledPlugins,
						     fullPath);
		g_free (fullPath);
		CappletFillActivePluginList (gswic);
		GSwitchItAppletConfigSaveToGConf (&gswic->appletConfig);
	}
}

static void
CappletDemotePlugin (GtkWidget * btnUp, GSwitchItPluginsCapplet * gswic)
{
	char *fullPath = CappletGetSelectedActivePluginPath (gswic);
	if (fullPath != NULL) {
		GSwitchItPluginManagerDemotePlugin (&gswic->pluginManager,
						    gswic->appletConfig.
						    enabledPlugins,
						    fullPath);
		g_free (fullPath);
		CappletFillActivePluginList (gswic);
		GSwitchItAppletConfigSaveToGConf (&gswic->appletConfig);
	}
}

static void
CappletDisablePlugin (GtkWidget * btnRemove,
		      GSwitchItPluginsCapplet * gswic)
{
	char *fullPath = CappletGetSelectedActivePluginPath (gswic);
	if (fullPath != NULL) {
		GSwitchItPluginManagerDisablePlugin (&gswic->pluginManager,
						     &gswic->appletConfig.
						     enabledPlugins,
						     fullPath);
		g_free (fullPath);
		CappletFillActivePluginList (gswic);
		GSwitchItAppletConfigSaveToGConf (&gswic->appletConfig);
	}
}

static void
CappletConfigurePlugin (GtkWidget * btnRemove,
			GSwitchItPluginsCapplet * gswic)
{
	char *fullPath = CappletGetSelectedActivePluginPath (gswic);
	if (fullPath != NULL) {
		GSwitchItPluginManagerConfigurePlugin (&gswic->
						       pluginManager,
						       &gswic->
						       pluginContainer,
						       fullPath,
						       GTK_WINDOW (gswic->
								   capplet));
		g_free (fullPath);
	}
}

static void
CappletResponse (GtkDialog * dialog, gint response)
{
	if (response == GTK_RESPONSE_HELP) {
		GSwitchItHelp (GTK_WIDGET (dialog),
			       "gswitchit-applet-plugins");
		return;
	}

	bonobo_main_quit ();
}

static void
CappletSetup (GSwitchItPluginsCapplet * gswic)
{
	GladeXML *data;
	GtkWidget *capplet;
	GtkWidget *activePlugins;
	GtkTreeModel *activePluginsModel;
	GtkCellRenderer *renderer =
	    GTK_CELL_RENDERER (gtk_cell_renderer_text_new ());
	GtkTreeViewColumn *column =
	    gtk_tree_view_column_new_with_attributes (NULL, renderer,
						      "text", 0,
						      NULL);
	GtkTreeSelection *selection;
	const char *iconFile;
	glade_gnome_init ();
	data = glade_xml_new (GNOME_GLADEDIR "/gswitchit-plugins.glade", "gswitchit_plugins", NULL);	// default domain!
	gswic->capplet = capplet =
	    glade_xml_get_widget (data, "gswitchit_plugins");
	iconFile = gnome_program_locate_file (NULL,
					      GNOME_FILE_DOMAIN_PIXMAP,
					      "gswitchit-properties-capplet.png",
					      TRUE, NULL);
	if (iconFile != NULL)
		gtk_window_set_icon_from_file (GTK_WINDOW (capplet),
					       iconFile, NULL);
	gtk_object_set_data (GTK_OBJECT (capplet), "gladeData", data);
	g_signal_connect_swapped (GTK_OBJECT (capplet),
				  "destroy", G_CALLBACK (g_object_unref),
				  data);
	g_signal_connect (G_OBJECT (capplet), "unrealize",
			  G_CALLBACK (bonobo_main_quit), data);

	g_signal_connect (GTK_OBJECT (capplet),
			  "response", G_CALLBACK (CappletResponse), NULL);

	glade_xml_signal_connect_data (data, "on_btnUp_clicked",
				       GTK_SIGNAL_FUNC
				       (CappletPromotePlugin), gswic);
	glade_xml_signal_connect_data (data,
				       "on_btnDown_clicked",
				       GTK_SIGNAL_FUNC
				       (CappletDemotePlugin), gswic);
	glade_xml_signal_connect_data (data,
				       "on_btnAdd_clicked",
				       GTK_SIGNAL_FUNC
				       (CappletEnablePlugin), gswic);
	glade_xml_signal_connect_data (data,
				       "on_btnRemove_clicked",
				       GTK_SIGNAL_FUNC
				       (CappletDisablePlugin), gswic);
	glade_xml_signal_connect_data (data,
				       "on_btnProperties_clicked",
				       GTK_SIGNAL_FUNC
				       (CappletConfigurePlugin), gswic);

	activePlugins = CappletGetGladeWidget (gswic, "activePlugins");
	activePluginsModel =
	    GTK_TREE_MODEL (gtk_list_store_new
			    (2, G_TYPE_STRING, G_TYPE_STRING));
	gtk_tree_view_set_model (GTK_TREE_VIEW (activePlugins),
				 activePluginsModel);
	gtk_tree_view_append_column (GTK_TREE_VIEW (activePlugins),
				     column);
	selection =
	    gtk_tree_view_get_selection (GTK_TREE_VIEW (activePlugins));
	gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);
	g_signal_connect (G_OBJECT (selection), "changed",
			  G_CALLBACK
			  (CappletActivePluginsSelectionChanged), gswic);
	CappletFillActivePluginList (gswic);
	CappletActivePluginsSelectionChanged (selection, gswic);
	gtk_widget_show_all (capplet);
}

int
main (int argc, char **argv)
{
	GSwitchItPluginsCapplet gswic;
	GError *gconf_error = NULL;
	GConfClient *confClient;

	bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
	memset (&gswic, 0, sizeof (gswic));
	gnome_program_init ("gswitchit", VERSION,
			    LIBGNOMEUI_MODULE, argc, argv,
			    GNOME_PROGRAM_STANDARD_PROPERTIES, NULL); 
	if (!gconf_init (argc, argv, &gconf_error)) {
		g_warning (_("Failed to init GConf: %s\n"),
			   gconf_error->message);
		g_error_free (gconf_error);
		return 1;
	}
	gconf_error = NULL;
	//GSwitchItInstallGlibLogAppender(  );
	XklInit (GDK_DISPLAY ());
	XklConfigInit ();
	XklConfigLoadRegistry ();

	confClient = gconf_client_get_default ();
	GSwitchItPluginContainerInit (&gswic.pluginContainer, confClient);
	g_object_unref (confClient);

	GSwitchItKbdConfigInit (&gswic.kbdConfig, confClient);
	GSwitchItAppletConfigInit (&gswic.appletConfig, confClient);
	GSwitchItPluginManagerInit (&gswic.pluginManager);
	GSwitchItKbdConfigLoadFromGConf (&gswic.kbdConfig);
	GSwitchItAppletConfigLoadFromGConf (&gswic.appletConfig);
	CappletSetup (&gswic);
	bonobo_main ();
	GSwitchItPluginManagerTerm (&gswic.pluginManager);
	GSwitchItAppletConfigTerm (&gswic.appletConfig);
	GSwitchItKbdConfigTerm (&gswic.kbdConfig);
	GSwitchItPluginContainerTerm (&gswic.pluginContainer);
	XklConfigFreeRegistry ();
	XklConfigTerm ();
	XklTerm ();
	return 0;
}

// functions just for plugins - otherwise ldd is not happy
void
GSwitchItPluginContainerReinitUi (GSwitchItPluginContainer * pc)
{
}

GSList *
GSwitchItPluginLoadLocalizedGroupNames (GSwitchItPluginContainer * pc)
{
	return GSwitchItConfigLoadGroupDescriptionsUtf8 (&(((GSwitchItPluginsCapplet *) pc)->config));
}
