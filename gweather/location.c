
#include <gnome.h>
#include <libgnomevfs/gnome-vfs.h>
#include <libxml/parser.h>
#include "gweather.h"

#define DATA_SIZE 5000

static void
insert_us_states (xmlNodePtr cur,  GWeatherApplet *applet)
{
	xmlNodePtr curtemp;
	GtkTreeIter row;
	curtemp =  cur->xmlChildrenNode;

	while (curtemp) {
		gchar *name = NULL, *url, *tmp;
		if (!xmlStrcmp(curtemp->name, (const xmlChar *)"state")) {
			name =  xmlGetProp(curtemp, "name");
			url = xmlGetProp(curtemp, "url");
			tmp = g_strdup_printf ("United States / %s", name);
			gtk_tree_store_append (applet->country_model, &row, NULL);
			gtk_tree_store_set (GTK_TREE_STORE (applet->country_model), &row, 0, tmp, 1, url, -1);
			xmlFree (name);
			xmlFree (url);
			g_free (tmp);
		}
		curtemp = curtemp->next;
	}
}

static void
insert_other_states (xmlNodePtr cur,  GWeatherApplet *applet)
{
	xmlNodePtr curtemp;
	GtkTreeIter row;
	curtemp =  cur->xmlChildrenNode;

	while (curtemp) {
		gchar *name = NULL, *url;
		if (!xmlStrcmp(curtemp->name, (const xmlChar *)"state")) {
			name =  xmlGetProp(curtemp, "name");
			url = xmlGetProp(curtemp, "url");
			gtk_tree_store_append (applet->country_model, &row, NULL);
			gtk_tree_store_set (GTK_TREE_STORE (applet->country_model), &row, 0, name, 1, url, -1);
			xmlFree (name);
			xmlFree (url);
		}
		curtemp = curtemp->next;
	}
}

static gboolean
parse_xml (GWeatherApplet *applet)
{
	gchar *xml = applet->locations_xml;
	xmlDocPtr doc;
	xmlNodePtr cur;
	gboolean country=TRUE;
	GtkTreeIter row;

	doc = xmlParseMemory (xml, strlen(xml));
	if (!doc) {
		return FALSE;
	}

	cur = xmlDocGetRootElement(doc);
	if (cur == NULL) {
		g_print ("empty xml \n");
		xmlFreeDoc(doc);
		return FALSE;
	}
	
	if (!xmlStrcmp(cur->name, (const xmlChar *) "countries")) {
		country = TRUE;
		gtk_tree_store_clear (GTK_TREE_STORE (applet->country_model));
	}
	else if (!xmlStrcmp(cur->name, (const xmlChar *) "state")) {
		gtk_tree_store_clear (GTK_TREE_STORE (applet->city_model));
		country = FALSE;
		
	}
	else {
		xmlFreeDoc(doc);
		return FALSE;
	}
	
	cur = cur->xmlChildrenNode;

	if (country == TRUE) {
		while (cur != NULL) {
			if ((!xmlStrcmp(cur->name, (const xmlChar *)"country"))) {
				gchar *name = NULL;
				name =  xmlGetProp(cur, "name");
				if (!g_strcasecmp (name, "United States")) {
					insert_us_states (cur, applet);
				}
				else {
					insert_other_states (cur, applet);
				}
				xmlFree (name);
			}
			cur = cur->next;
		}
	}
	else {
		while (cur != NULL) {
			gchar *name, *url;
			if (!xmlStrcmp(cur->name, (const xmlChar *)"city")) {
				gchar *name = NULL, *url;
				name =  xmlGetProp(cur, "name");
				url = xmlGetProp (cur, "url");
				gtk_tree_store_append (applet->city_model, &row, NULL);
				gtk_tree_store_set (GTK_TREE_STORE (applet->city_model), &row, 0, name, 1, url, -1);
				xmlFree (name);
				xmlFree (url);
			}
			cur = cur->next;
		}
	}
	
 	return TRUE;
}

static void 
finish_read(GnomeVFSAsyncHandle *handle, GnomeVFSResult result, 
		      gpointer buffer, GnomeVFSFileSize requested, 
		      GnomeVFSFileSize body_len, gpointer data)
{
	GWeatherApplet *applet = data;
	gchar *body, *temp;

	body = (gchar *)buffer;
	body[body_len] = '\0';
	if (applet->locations_xml == NULL)
        	applet->locations_xml = g_strdup(body);
    	else
    	{
       		temp = g_strdup(applet->locations_xml);
		g_free(applet->locations_xml);
		applet->locations_xml = g_strdup_printf("%s%s", temp, body);
		g_free(temp);
    	}

	if (result == GNOME_VFS_ERROR_EOF)
    	{
		parse_xml (applet);
		applet->fetchid = -1;
		applet->locations_handle = NULL;
    	}
    	else if (result != GNOME_VFS_OK) {
		g_print("%s", gnome_vfs_result_to_string(result));
		g_print("Failed to read data.\n");
		applet->fetchid = -1;
		applet->locations_handle = NULL;
    	} else {
		gnome_vfs_async_read(handle, body, DATA_SIZE - 1, 
							    finish_read, applet);
		return;
    	}

	 g_free (buffer);
}


static void 
url_opened (GnomeVFSAsyncHandle *handle, GnomeVFSResult result, 			gpointer data)
{
	GWeatherApplet *applet = data;
	gchar *body;
	gint body_len;
	
	if (result != GNOME_VFS_OK) {
		GtkTreeIter iter;
		GtkTreeModel *model;
        	g_print("%s", gnome_vfs_result_to_string(result));
		if (applet->fetchid == 0)
			model = GTK_TREE_MODEL (applet->country_model);
		else
			model = GTK_TREE_MODEL (applet->city_model);
		gtk_tree_store_clear (GTK_TREE_STORE (model));
		gtk_tree_store_append (GTK_TREE_STORE (model), &iter, NULL);
		if (applet->fetchid == 0)
			gtk_tree_store_set (GTK_TREE_STORE (model), &iter, 0, _("Could not download countries"), -1);
		else
			gtk_tree_store_set (GTK_TREE_STORE (model), &iter, 0, _("Could not download cities"), -1);
		applet->fetchid = -1;
		applet->locations_handle = NULL;
    	} else {
		body = g_malloc0(DATA_SIZE);
        	gnome_vfs_async_read(handle, body, DATA_SIZE -1, 
							   finish_read, applet); 
    	}
     return;

}



static void
get_xml (GWeatherApplet *applet, gchar *url)
{
	if (applet->locations_handle != NULL)
		return;

	if (applet->locations_xml) {
		g_free (applet->locations_xml);
		applet->locations_xml = NULL;
	}

	gnome_vfs_async_open(&applet->locations_handle, url,
						    GNOME_VFS_OPEN_READ, 
    						     0, url_opened, applet);

}

static gboolean
cb_row_selected (GtkTreeSelection *selection,
                                 gpointer data)
{
	GWeatherApplet *applet = data;
	GtkTreeIter iter, row;
	gchar *url = NULL;

	if (!gtk_tree_selection_get_selected (selection, NULL, &iter)) 
		return FALSE;

	if (applet->fetchid == 0)
		return FALSE;
	else if (applet->fetchid == 1) {
		gnome_vfs_async_cancel (applet->locations_handle);
		applet->locations_handle = NULL;
	}

	gtk_tree_model_get (GTK_TREE_MODEL (applet->country_model), &iter, 
					      1, &url, -1);
	if (!url)
		return;
	applet->fetchid = 1;
	
	gtk_tree_store_clear (GTK_TREE_STORE (applet->city_model));
	gtk_tree_store_append (applet->city_model, &row, NULL);
	gtk_tree_store_set (GTK_TREE_STORE (applet->city_model), &row, 0, _("Downloading Cities..."), -1);

	get_xml (applet, url);
	g_free (url);
	
	return FALSE;
}

static gboolean
cb_city_selected (GtkTreeSelection *selection,
                                 gpointer data)
{
	GWeatherApplet *applet = data;
	GtkTreeIter iter;
	GtkTreeModel *model;
	gchar *url = NULL, *name;
	
	if (!gtk_tree_selection_get_selected (selection, &model, &iter))
		return FALSE;

	gtk_tree_model_get (model, &iter, 0, &name, 1, &url, -1);
	if (!url)
		return FALSE;

	if (applet->gweather_pref.city)
		g_free (applet->gweather_pref.city);
	applet->gweather_pref.city = name;
	if (applet->gweather_pref.url)
		g_free (applet->gweather_pref.url);
	applet->gweather_pref.url = gnome_vfs_escape_host_and_path_string (url);
	g_free (url);
	gtk_label_set_text (GTK_LABEL (applet->pref_location_city_label),
					   applet->gweather_pref.city);
	gweather_update (applet);

	panel_applet_gconf_set_string (applet->applet, "url", applet->gweather_pref.url, NULL);
	panel_applet_gconf_set_string (applet->applet, "city", applet->gweather_pref.city, NULL);

	return FALSE;

}

GtkWidget *
create_countries_widget (GWeatherApplet *applet)
{
	GtkWidget *scrolled;
	GtkTreeStore *model;
	GtkWidget *tree;
	GtkCellRenderer *cell;
	GtkTreeViewColumn *column;

	scrolled = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled)	,
		GTK_POLICY_ALWAYS, GTK_POLICY_ALWAYS);

	model = gtk_tree_store_new (2, G_TYPE_STRING,  G_TYPE_STRING);

	tree = gtk_tree_view_new_with_model (GTK_TREE_MODEL (model));
	g_object_unref (G_OBJECT (model));
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (tree), FALSE);
	set_access_namedesc (tree, 
				         _("Countries list"),
				         _("Select the country from the list"));
				         
	cell = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Country"),
						    		   cell,
						     		   "text", 0,
						     		   NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);

	cell = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Url",
						    		   cell,
						     		   "text", 1,
						     		   NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);
	gtk_tree_view_column_set_visible (column, FALSE);
	
	gtk_container_add (GTK_CONTAINER (scrolled), tree);

	applet->country_tree = tree;
	applet->country_model = model;

	g_signal_connect (G_OBJECT (gtk_tree_view_get_selection 					(GTK_TREE_VIEW (tree))), 
			  		"changed",
			  		G_CALLBACK (cb_row_selected), applet);

	applet->fetchid = -1;

	return scrolled;

}

GtkWidget *
create_cities_widget (GWeatherApplet *applet)
{
	GtkWidget *scrolled;
	GtkTreeStore *model;
	GtkWidget *tree;
	GtkCellRenderer *cell;
	GtkTreeViewColumn *column;

	scrolled = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled)	,
		GTK_POLICY_ALWAYS, GTK_POLICY_ALWAYS);

	model = gtk_tree_store_new (2, G_TYPE_STRING,  G_TYPE_STRING);

	tree = gtk_tree_view_new_with_model (GTK_TREE_MODEL (model));
	g_object_unref (G_OBJECT (model));
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (tree), FALSE);
	set_access_namedesc (tree, 
				         _("Cities list"),
				         _("Select the city from the list"));

	cell = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Location"),
						    		   cell,
						     		   "text", 0,
						     		   NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);

	cell = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Url",
						    		   cell,
						     		   "text", 1,
						     		   NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);
	gtk_tree_view_column_set_visible (column, FALSE);
	
	gtk_container_add (GTK_CONTAINER (scrolled), tree);

	applet->city_tree = tree;
	applet->city_model = model;

	g_signal_connect (G_OBJECT (gtk_tree_view_get_selection 					(GTK_TREE_VIEW (tree))), 
			  		"changed",
			  		G_CALLBACK (cb_city_selected), applet);

	return scrolled;

}

void fetch_countries (GWeatherApplet *applet)
{
	GtkTreeIter row;

	applet->fetchid = 0;
	get_xml (applet, "http://weather.interceptvector.com/cities/index.xml");
	gtk_tree_store_append (applet->country_model, &row, NULL);
	gtk_tree_store_set (GTK_TREE_STORE (applet->country_model), &row, 0, _("Downloading Countries..."),1, NULL, -1);

}
	



