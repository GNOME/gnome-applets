/* $Id$ */

/*
 *  Papadimitriou Spiros <spapadim+@cs.cmu.edu>
 *
 *  This code released under the GNU GPL.
 *  Read the file COPYING for more information.
 *
 *  Preferences dialog
 *
 */

/* Radar map on by default. */
#define RADARMAP

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>

#include <gnome.h>
#include <panel-applet.h>

#include "gweather.h"
#include "gweather-pref.h"
#include "gweather-applet.h"

enum
{
    COL_LOC = 0,
    COL_POINTER,
    NUM_COLUMNS,
}; 


static gint cmp_loc (const WeatherLocation *l1, const WeatherLocation *l2)
{
    return (l1 && l2 && weather_location_equal(l1, l2)) ? 0 : 1;
}

/* Update pref dialog from gweather_pref */
static gboolean update_dialog (GWeatherApplet *gw_applet)
{
    GtkCTreeNode *node;

    g_return_val_if_fail(gw_applet->gweather_pref.location != NULL, FALSE);

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(gw_applet->pref_basic_update_spin), 
    			      gw_applet->gweather_pref.update_interval / 60);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gw_applet->pref_basic_update_btn), 
    				 gw_applet->gweather_pref.update_enabled);
    gtk_widget_set_sensitive(gw_applet->pref_basic_update_spin, 
    			     gw_applet->gweather_pref.update_enabled);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gw_applet->pref_basic_metric_btn), 
    				 gw_applet->gweather_pref.use_metric);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gw_applet->pref_basic_detailed_btn), 
    				 gw_applet->gweather_pref.detailed);
#ifdef RADARMAP
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gw_applet->pref_basic_radar_btn),
    				 gw_applet->gweather_pref.radar_enabled);
#endif /* RADARMAP */

    gtk_entry_set_text(GTK_ENTRY(gw_applet->pref_net_proxy_url_entry), 
    		       gw_applet->gweather_pref.proxy_url ? 
    		       gw_applet->gweather_pref.proxy_url : "");
    gtk_entry_set_text(GTK_ENTRY(gw_applet->pref_net_proxy_passwd_entry), 
    		       gw_applet->gweather_pref.proxy_passwd ? 
    		       gw_applet->gweather_pref.proxy_passwd : "");
    gtk_entry_set_text(GTK_ENTRY(gw_applet->pref_net_proxy_user_entry), 
    		       gw_applet->gweather_pref.proxy_user ? 
    		       gw_applet->gweather_pref.proxy_user : "");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gw_applet->pref_net_proxy_btn), 
    				 gw_applet->gweather_pref.use_proxy);
    
    gtk_widget_set_sensitive (gw_applet->pref_net_proxy_url_entry,
    			      gw_applet->gweather_pref.use_proxy);
    gtk_widget_set_sensitive (gw_applet->pref_net_proxy_passwd_entry,
    			      gw_applet->gweather_pref.use_proxy);
    gtk_widget_set_sensitive (gw_applet->pref_net_proxy_user_entry,
    			      gw_applet->gweather_pref.use_proxy);
    
    return TRUE;
}

static inline void update_string (gchar *val, gchar **str)
{
    g_return_if_fail(str != NULL);

    g_free(*str);
    *str = NULL;
    if (val && (strlen(val) > 0))
        *str = g_strdup(val);
}

static gboolean check_proxy_uri (const gchar *uri)
{
    char *pcolon, *pslash;

    g_return_val_if_fail(uri != NULL, FALSE);

    /* Check if uri starts with "http://" */
    if ((strlen(uri) < 7) || (strncmp(uri, "http://", 7) != 0))
        return FALSE;

    /* Check for slashes */
    pslash = strchr(uri+7, '/');
    if (pslash && (strcmp(pslash, "/") != 0))
        return FALSE;

    /* Check for port number after second colon */
    if ((pcolon = strchr(uri+7, ':')) != NULL) {
        if (!isdigit(pcolon[1]))
            return FALSE;
        for (++pcolon ;  *pcolon;  pcolon++)
            if (!isdigit(*pcolon))
                break;
        return (!strcmp(pcolon, "") || !strcmp(pcolon, "/"));
    }

    return TRUE;
}

/* Update gweather_pref from pref dialog */
static gboolean update_pref (GWeatherApplet *gw_applet)
{
    gchar *proxy_url;
    
    proxy_url = (gchar *)gtk_entry_get_text(GTK_ENTRY(gw_applet->pref_net_proxy_url_entry));

   if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gw_applet->pref_net_proxy_btn))
	&& proxy_url && strlen(proxy_url) && !check_proxy_uri(proxy_url)) {
        gnome_error_dialog(_("Proxy URL is not of the form http://host:port/\nProperties remain unchanged."));
        return FALSE;
    }

    
#ifdef RADARMAP
    
#endif /* RADARMAP */

    
    update_string(proxy_url, &(gw_applet->gweather_pref.proxy_url));
    update_string((gchar *)gtk_entry_get_text(GTK_ENTRY(gw_applet->pref_net_proxy_passwd_entry)), &(gw_applet->gweather_pref.proxy_passwd));
    update_string((gchar *)gtk_entry_get_text(GTK_ENTRY(gw_applet->pref_net_proxy_user_entry)), &(gw_applet->gweather_pref.proxy_user));

    /* fprintf(stderr, "Set location to: %s\n", loc->name); */

    return TRUE;
}

static void change_cb (GtkButton *button, gpointer user_data)
{
    GWeatherApplet *gw_applet = user_data;
    
    gtk_widget_set_sensitive(gw_applet->pref_basic_update_spin, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gw_applet->pref_basic_update_btn)));
    
    return;
}

static void row_selected_cb (GtkTreeSelection *selection, gpointer data)
{
    GWeatherApplet *gw_applet = data;
    GtkTreeModel *model;
    WeatherLocation *loc = NULL;
    GtkTreeIter iter;
    
    if (!gtk_tree_selection_get_selected (selection, &model, &iter))
    	return;
    	
    gtk_tree_model_get (model, &iter, COL_POINTER, &loc, -1);
    
    if (!loc)
    	return;
    	
    weather_location_free(gw_applet->gweather_pref.location);
    gw_applet->gweather_pref.location = weather_location_clone(loc);
    panel_applet_gconf_set_string(gw_applet->applet, "location0", loc->name, NULL);
    panel_applet_gconf_set_string(gw_applet->applet, "location1", loc->code, NULL);
    panel_applet_gconf_set_string(gw_applet->applet, "location2", loc->zone, NULL);
    panel_applet_gconf_set_string(gw_applet->applet, "location3", loc->radar, NULL);
     
} 

static void row_activated_cb (GtkTreeView *tree, 
			      GtkTreePath *path, 
			      GtkTreeViewColumn *column,
			      gpointer data)
{
    GWeatherApplet *gw_applet = data;
    GtkTreeModel *model = gtk_tree_view_get_model (tree);
    WeatherLocation *loc = NULL;
    GtkTreeIter iter;
    
    if (!gtk_tree_model_get_iter (model, &iter, path))
    	return;
    	
    gtk_tree_model_get (model, &iter, COL_POINTER, &loc, -1);
    
    if (loc)
        gweather_update (gw_applet);

}

static void load_locations (GWeatherApplet *gw_applet)
{
    GtkTreeView *tree = GTK_TREE_VIEW(gw_applet->pref_tree);
    GtkTreeStore *model;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell_renderer;

    gchar *pp[1], *path;
    gint nregions, iregions;
    gchar **regions;
    
    model = GTK_TREE_STORE (gtk_tree_view_get_model (tree));

    path = g_strdup_printf("=%s=/", gnome_datadir_file("gweather/Locations"));
    gnome_config_push_prefix(path);
    g_free(path);
    
    cell_renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes ("not used", cell_renderer,
						       "text", COL_LOC, NULL);
    gtk_tree_view_append_column (tree, column);
    gtk_tree_view_set_expander_column (GTK_TREE_VIEW (tree), column);

    gnome_config_get_vector("Main/regions", &nregions, &regions);

    for (iregions = nregions-1;  iregions >= 0;  iregions--) {
        GtkTreeIter region_iter;
        gint nstates, istates;
        gchar **states;
        gchar *region_name;
        gchar *region_name_key = g_strconcat(regions[iregions], "/name", NULL);
        gchar *states_key = g_strconcat(regions[iregions], "/states", NULL);

        region_name = gnome_config_get_string(region_name_key);

        gtk_tree_store_insert (model, &region_iter, NULL, 0);
        gtk_tree_store_set (model, &region_iter, COL_LOC, region_name, -1);
        
        gnome_config_get_vector(states_key, &nstates, &states);

        for (istates = nstates - 1;  istates >= 0;  istates--) {
            GtkTreeIter state_iter;
            void *iter;
            gchar *iter_key, *iter_val;
            gchar *state_path = g_strconcat(regions[iregions], "_", states[istates], "/", NULL);
            gchar *state_name_key = g_strconcat(state_path, "name", NULL);
            gchar *state_name = gnome_config_get_string(state_name_key);

	    gtk_tree_store_insert (model, &state_iter, &region_iter, 0);
            gtk_tree_store_set (model, &state_iter, COL_LOC, state_name, -1);
            
            iter = gnome_config_init_iterator(state_path);
            while ((iter = gnome_config_iterator_next(iter, &iter_key, &iter_val)) != NULL) {
                if (strstr(iter_key, "loc") != NULL) {
                    GtkTreeIter loc_iter;
                    gchar **locdata;
                    gint nlocdata;
                    WeatherLocation *weather_location;

                    gnome_config_make_vector(iter_val, &nlocdata, &locdata);
                    g_return_if_fail(nlocdata == 4);
                    
		    weather_location = weather_location_new(locdata[0], 
		    					    locdata[1], 
		    					    locdata[2], 
		    					    locdata[3]);
                    gtk_tree_store_insert (model, &loc_iter, &state_iter, 0);
                    gtk_tree_store_set (model, &loc_iter, COL_LOC, locdata[0], 
                    			COL_POINTER, weather_location, -1);                 
                    
                    
                    if (gw_applet->gweather_pref.location && weather_location_equal(weather_location, gw_applet->gweather_pref.location)) {
                        GtkTreePath *path;
                        path = gtk_tree_model_get_path (GTK_TREE_MODEL (model), 
                                                        &region_iter);
                        gtk_tree_view_expand_row (tree, path, FALSE);
                        gtk_tree_path_free (path);
                        path = gtk_tree_model_get_path (GTK_TREE_MODEL (model), 
                                                        &state_iter);
                        gtk_tree_view_expand_row (tree, path, FALSE);
                        gtk_tree_path_free (path);
                        path = gtk_tree_model_get_path (GTK_TREE_MODEL (model), 
                                                        &loc_iter);
                        gtk_tree_view_set_cursor (tree, path, NULL, FALSE);
                        gtk_tree_view_scroll_to_cell (tree, path, NULL, TRUE, 0.5, 0.5);
                        gtk_tree_path_free (path);
                    }
		    
                    g_strfreev(locdata);
                }

                g_free(iter_key);
                g_free(iter_val);
            }

            g_free(state_name);
            g_free(state_path);
            g_free(state_name_key);
        }

        g_strfreev(states);
        g_free(region_name);
        g_free(region_name_key);
        g_free(states_key);
    }
    g_strfreev(regions);

    gnome_config_pop_prefix();
}

static void
proxy_toggled (GtkToggleButton *button, gpointer data)
{
    GWeatherApplet *gw_applet = data;
    gboolean toggled;
	
    toggled = gtk_toggle_button_get_active(button);
    gw_applet->gweather_pref.use_proxy = toggled;
    gtk_widget_set_sensitive(gw_applet->pref_net_proxy_url_entry, toggled);
    gtk_widget_set_sensitive(gw_applet->pref_net_proxy_user_entry, toggled);
    gtk_widget_set_sensitive(gw_applet->pref_net_proxy_passwd_entry, toggled);
    
    panel_applet_gconf_set_bool(gw_applet->applet, "enable_proxy", toggled, NULL);
}

static void
proxy_url_changed (GtkWidget *entry, GdkEventFocus *event, gpointer data)
{
    GWeatherApplet *gw_applet = data;
    gchar *text;
	g_print("url_changed\n");    
    text = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
    
   if (!text)
   	return;
   
   panel_applet_gconf_set_string(gw_applet->applet, "proxy_url", text, NULL);
   g_free(gw_applet->gweather_pref.proxy_url);
   gw_applet->gweather_pref.proxy_url = text;   	
}

static void
proxy_user_changed (GtkWidget *entry, GdkEventFocus *event, gpointer data)
{
    GWeatherApplet *gw_applet = data;
    gchar *text;
    
    text = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
    
   if (!text)
   	return;
   
   panel_applet_gconf_set_string(gw_applet->applet, "proxy_user", text, NULL);
   g_free(gw_applet->gweather_pref.proxy_user);
   gw_applet->gweather_pref.proxy_user = text;   
}

static void
proxy_password_changed (GtkWidget *entry, GdkEventFocus *event, gpointer data)
{
    GWeatherApplet *gw_applet = data;
    gchar *text;
    
    text = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
    
   if (!text)
   	return;

	panel_applet_gconf_set_string(gw_applet->applet, "proxy_passwd", text, NULL);
	g_free(gw_applet->gweather_pref.proxy_passwd);
	gw_applet->gweather_pref.proxy_passwd = text;      	
}

static void
auto_update_toggled (GtkToggleButton *button, gpointer data)
{
    GWeatherApplet *gw_applet = data;
    gboolean toggled;
    
    toggled = gtk_toggle_button_get_active(button);
    gw_applet->gweather_pref.update_enabled = toggled;
    gtk_widget_set_sensitive (gw_applet->pref_basic_update_spin, toggled);
    panel_applet_gconf_set_bool(gw_applet->applet, "auto_update", toggled, NULL);
}

static void
metric_toggled (GtkToggleButton *button, gpointer data)
{
    GWeatherApplet *gw_applet = data;
    gboolean toggled;
    
    toggled = gtk_toggle_button_get_active(button);
    gw_applet->gweather_pref.use_metric = toggled;
    panel_applet_gconf_set_bool(gw_applet->applet, "enable_metric", toggled, NULL);
}

static void
detailed_toggled (GtkToggleButton *button, gpointer data)
{
    GWeatherApplet *gw_applet = data;
    gboolean toggled;
    
    toggled = gtk_toggle_button_get_active(button);
    gw_applet->gweather_pref.detailed = toggled;
    panel_applet_gconf_set_bool(gw_applet->applet, "enable_detailed_forecast", toggled, NULL);    
}

static void
radar_toggled (GtkToggleButton *button, gpointer data)
{
    GWeatherApplet *gw_applet = data;
    gboolean toggled;
    
    toggled = gtk_toggle_button_get_active(button);
    gw_applet->gweather_pref.radar_enabled = toggled;
    panel_applet_gconf_set_bool(gw_applet->applet, "enable_radar_map", toggled, NULL);
}

static void
update_interval_changed (GtkSpinButton *button, gpointer data)
{
    GWeatherApplet *gw_applet = data;
    
    gw_applet->gweather_pref.update_interval = gtk_spin_button_get_value_as_int(button)*60;
    panel_applet_gconf_set_int(gw_applet->applet, "auto_update_intervarl", 
    		gw_applet->gweather_pref.update_interval, NULL);
}

static void
response_cb (GtkDialog *dialog, gint id, gpointer data)
{
    GWeatherApplet *gw_applet = data;
    gtk_widget_destroy (GTK_WIDGET (dialog));
    gw_applet->pref = NULL;
     
}

static void gweather_pref_create (GWeatherApplet *gw_applet)
{
    GtkWidget *pref_vbox;
    GtkWidget *pref_notebook;
    GtkWidget *pref_basic_metric_alignment;
    GtkWidget *pref_basic_detailed_alignment;
#ifdef RADARMAP
    GtkWidget *pref_basic_radar_alignment;
#endif /* RADARMAP */
    GtkWidget *pref_basic_update_alignment;
    GtkWidget *pref_basic_update_lbl;
    GtkWidget *pref_basic_update_hbox;
    GtkObject *pref_basic_update_spin_adj;
    GtkWidget *pref_basic_update_sec_lbl;
    GtkWidget *pref_basic_note_lbl;
    GtkWidget *pref_net_table;
    GtkWidget *pref_net_note_lbl;
    GtkWidget *pref_net_proxy_url_lbl;
    GtkWidget *pref_net_proxy_user_lbl;
    GtkWidget *pref_net_proxy_passwd_lbl;
    GtkWidget *pref_net_proxy_alignment;
    GtkWidget *pref_net_caveat_lbl;
    GtkWidget *pref_loc_hbox;
    GtkWidget *pref_loc_note_lbl;
    GtkWidget *scrolled_window;
    GtkTreeStore *model;
    GtkTreeSelection *selection;
    GtkWidget *pref_basic_vbox;
    GtkWidget *vbox;
    GtkWidget *frame;

    gw_applet->pref = gtk_dialog_new_with_buttons (_("Weather Properties"), NULL,
				      		   GTK_DIALOG_DESTROY_WITH_PARENT,
				      		   GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
				      		   NULL);
    gtk_widget_set_usize (gw_applet->pref, -2, 280);
    gtk_window_set_policy (GTK_WINDOW (gw_applet->pref), TRUE, TRUE, FALSE);
    
    pref_vbox = GTK_DIALOG (gw_applet->pref)->vbox;
    gtk_widget_show (pref_vbox);

    pref_notebook = gtk_notebook_new ();
    gtk_widget_show (pref_notebook);
    gtk_box_pack_start (GTK_BOX (pref_vbox), pref_notebook, TRUE, TRUE, 0);

  /*
   * Location page.
   */
    pref_loc_hbox = gtk_hbox_new (FALSE, 2);
    gtk_widget_show (pref_loc_hbox);
    gtk_container_add (GTK_CONTAINER (pref_notebook), pref_loc_hbox);

    scrolled_window = gtk_scrolled_window_new (NULL, NULL);
    gtk_container_set_border_width (GTK_CONTAINER (scrolled_window), GNOME_PAD_SMALL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
				    GTK_POLICY_AUTOMATIC,
				    GTK_POLICY_AUTOMATIC);

    model = gtk_tree_store_new (NUM_COLUMNS, G_TYPE_STRING, G_TYPE_POINTER);
    gw_applet->pref_tree = gtk_tree_view_new_with_model (GTK_TREE_MODEL (model));
    gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (gw_applet->pref_tree), FALSE);
    g_object_unref (G_OBJECT (model));
    
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (gw_applet->pref_tree));
    g_signal_connect (G_OBJECT (selection), "changed",
    		      G_CALLBACK (row_selected_cb), gw_applet);
    /* Update applet when user double clicks */
    g_signal_connect (G_OBJECT (gw_applet->pref_tree), "row_activated",
    		      G_CALLBACK (row_activated_cb), gw_applet);
    
    gtk_container_add (GTK_CONTAINER (scrolled_window), gw_applet->pref_tree);
    gtk_widget_show (gw_applet->pref_tree);
    gtk_widget_show (scrolled_window);
    gtk_box_pack_start (GTK_BOX (pref_loc_hbox), scrolled_window, TRUE, TRUE, 0);
    load_locations(gw_applet);

    pref_loc_note_lbl = gtk_label_new (_("Location"));
    gtk_widget_show (pref_loc_note_lbl);
    gtk_notebook_set_tab_label (GTK_NOTEBOOK (pref_notebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (pref_notebook), 0), pref_loc_note_lbl);

    /*
   * Network settings page.
   */
    pref_net_table = gtk_table_new (5, 2, FALSE);
    gtk_widget_show (pref_net_table);
    gtk_container_add (GTK_CONTAINER (pref_notebook), pref_net_table);
    gtk_container_set_border_width (GTK_CONTAINER (pref_net_table), 8);
    gtk_table_set_row_spacings (GTK_TABLE (pref_net_table), 4);
    gtk_table_set_col_spacings (GTK_TABLE (pref_net_table), 4);

    pref_net_proxy_alignment = gtk_alignment_new (0, 0.5, 0, 1);
    gtk_widget_show (pref_net_proxy_alignment);
    gtk_table_attach (GTK_TABLE (pref_net_table), pref_net_proxy_alignment, 0, 2, 0, 1,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);

    gw_applet->pref_net_proxy_btn = gtk_check_button_new_with_label (_("Use proxy"));
    gtk_widget_show (gw_applet->pref_net_proxy_btn);
    gtk_container_add (GTK_CONTAINER (pref_net_proxy_alignment), gw_applet->pref_net_proxy_btn);
    g_signal_connect (G_OBJECT (gw_applet->pref_net_proxy_btn), "toggled",
    		       G_CALLBACK (proxy_toggled), gw_applet);

    pref_net_proxy_url_lbl = gtk_label_new (_("Proxy URL:"));
    gtk_widget_show (pref_net_proxy_url_lbl);
    gtk_table_attach (GTK_TABLE (pref_net_table), pref_net_proxy_url_lbl, 0, 1, 1, 2,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify (GTK_LABEL (pref_net_proxy_url_lbl), GTK_JUSTIFY_RIGHT);
    gtk_misc_set_alignment (GTK_MISC (pref_net_proxy_url_lbl), 1, 0.5);
    
    gw_applet->pref_net_proxy_url_entry = gtk_entry_new ();
    gtk_widget_show (gw_applet->pref_net_proxy_url_entry);
    gtk_table_attach (GTK_TABLE (pref_net_table), gw_applet->pref_net_proxy_url_entry, 1, 2, 1, 2,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (GTK_FILL), 0, 0);
    g_signal_connect (G_OBJECT (gw_applet->pref_net_proxy_url_entry), "focus_out_event",
    		      G_CALLBACK (proxy_url_changed), gw_applet);

    pref_net_proxy_user_lbl = gtk_label_new (_("Username:"));
    gtk_widget_show (pref_net_proxy_user_lbl);
    gtk_table_attach (GTK_TABLE (pref_net_table), pref_net_proxy_user_lbl, 0, 1, 2, 3,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify (GTK_LABEL (pref_net_proxy_user_lbl), GTK_JUSTIFY_RIGHT);
    gtk_misc_set_alignment (GTK_MISC (pref_net_proxy_user_lbl), 1, 0.5);

    gw_applet->pref_net_proxy_user_entry = gtk_entry_new ();
    gtk_widget_show (gw_applet->pref_net_proxy_user_entry);
    gtk_table_attach (GTK_TABLE (pref_net_table), gw_applet->pref_net_proxy_user_entry, 1, 2, 2, 3,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (GTK_FILL), 0, 0);
    g_signal_connect (G_OBJECT (gw_applet->pref_net_proxy_user_entry), "focus_out_event",
    		      G_CALLBACK (proxy_user_changed), gw_applet);

    pref_net_proxy_passwd_lbl = gtk_label_new (_("Password:"));
    gtk_widget_show (pref_net_proxy_passwd_lbl);
    gtk_table_attach (GTK_TABLE (pref_net_table), pref_net_proxy_passwd_lbl, 0, 1, 3, 4,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify (GTK_LABEL (pref_net_proxy_passwd_lbl), GTK_JUSTIFY_RIGHT);
    gtk_misc_set_alignment (GTK_MISC (pref_net_proxy_passwd_lbl), 1, 0.5);

    gw_applet->pref_net_proxy_passwd_entry = gtk_entry_new ();
    gtk_entry_set_visibility (GTK_ENTRY (gw_applet->pref_net_proxy_passwd_entry), FALSE);
    gtk_widget_show (gw_applet->pref_net_proxy_passwd_entry);
    gtk_table_attach (GTK_TABLE (pref_net_table), gw_applet->pref_net_proxy_passwd_entry, 1, 2, 3, 4,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (GTK_FILL), 0, 0);
    g_signal_connect (G_OBJECT (gw_applet->pref_net_proxy_passwd_entry), "focus_out_event",
    		      G_CALLBACK (proxy_password_changed), gw_applet);

    pref_net_caveat_lbl = gtk_label_new (_("Warning: Your password will be saved as\nunencrypted text in a private configuration\nfile."));
    gtk_widget_show (pref_net_caveat_lbl);
    gtk_table_attach (GTK_TABLE (pref_net_table), pref_net_caveat_lbl, 0, 2, 4, 5,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify (GTK_LABEL (pref_net_caveat_lbl), GTK_JUSTIFY_LEFT);
    gtk_misc_set_alignment (GTK_MISC (pref_net_caveat_lbl), 1, 0.5);

    pref_net_note_lbl = gtk_label_new (_("Network"));
    gtk_widget_show (pref_net_note_lbl);
    gtk_notebook_set_tab_label (GTK_NOTEBOOK (pref_notebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (pref_notebook), 1), pref_net_note_lbl);

  /*
   * General settings page.
   */

    pref_basic_vbox = gtk_vbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (pref_notebook), pref_basic_vbox);

    pref_basic_update_alignment = gtk_alignment_new (0, 0.5, 0, 1);
    gtk_widget_show (pref_basic_update_alignment);

    pref_basic_metric_alignment = gtk_alignment_new (0, 0.5, 0, 1);
    gtk_widget_show (pref_basic_metric_alignment);

    pref_basic_detailed_alignment = gtk_alignment_new (0, 0.5, 0, 1);
    gtk_widget_show (pref_basic_detailed_alignment);

#ifdef RADARMAP
    pref_basic_radar_alignment = gtk_alignment_new (0, 0.5, 0, 1);
    gtk_widget_show (pref_basic_radar_alignment);
#endif /* RADARMAP */

    gw_applet->pref_basic_update_btn = gtk_check_button_new_with_label (_("Automatically update every"));
    gtk_widget_show (gw_applet->pref_basic_update_btn);
    gtk_container_add (GTK_CONTAINER (pref_basic_update_alignment), gw_applet->pref_basic_update_btn);
    g_signal_connect (G_OBJECT (gw_applet->pref_basic_update_btn), "toggled",
    		      G_CALLBACK (auto_update_toggled), gw_applet);

    gw_applet->pref_basic_metric_btn = gtk_check_button_new_with_label (_("Use metric system units"));
    gtk_widget_show (gw_applet->pref_basic_metric_btn);
    gtk_container_add (GTK_CONTAINER (pref_basic_metric_alignment), gw_applet->pref_basic_metric_btn);
    g_signal_connect (G_OBJECT (gw_applet->pref_basic_metric_btn), "toggled",
    		      G_CALLBACK (metric_toggled), gw_applet);
    		      
    gw_applet->pref_basic_detailed_btn = gtk_check_button_new_with_label (_("Enable detailed forecast"));
    gtk_widget_show (gw_applet->pref_basic_detailed_btn);
    gtk_container_add (GTK_CONTAINER (pref_basic_detailed_alignment), gw_applet->pref_basic_detailed_btn);
    g_signal_connect (G_OBJECT (gw_applet->pref_basic_detailed_btn), "toggled",
    		      G_CALLBACK (detailed_toggled), gw_applet);

#ifdef RADARMAP
    gw_applet->pref_basic_radar_btn = gtk_check_button_new_with_label (_("Enable radar map"));
    gtk_widget_show (gw_applet->pref_basic_radar_btn);
    gtk_container_add (GTK_CONTAINER (pref_basic_radar_alignment), gw_applet->pref_basic_radar_btn);
    g_signal_connect (G_OBJECT (gw_applet->pref_basic_radar_btn), "toggled",
    		      G_CALLBACK (radar_toggled), gw_applet);
#endif /* RADARMAP */

    frame = gtk_frame_new (_("Updates"));
    gtk_container_border_width (GTK_CONTAINER (frame), GNOME_PAD_SMALL);

    pref_basic_update_hbox = gtk_hbox_new (FALSE, GNOME_PAD_SMALL);
    gtk_container_border_width (GTK_CONTAINER (pref_basic_update_hbox), GNOME_PAD_SMALL);

    pref_basic_update_lbl = gtk_label_new (_("Automatically update every "));
    gtk_widget_show (pref_basic_update_lbl);
    

/*
    gtk_label_set_justify (GTK_LABEL (pref_basic_update_lbl), GTK_JUSTIFY_RIGHT);
    gtk_misc_set_alignment (GTK_MISC (pref_basic_update_lbl), 1, 0.5);
*/

    gtk_widget_show (pref_basic_update_hbox);

    pref_basic_update_spin_adj = gtk_adjustment_new (30, 1, 60, 5, 25, 1);
    gw_applet->pref_basic_update_spin = gtk_spin_button_new (GTK_ADJUSTMENT (pref_basic_update_spin_adj), 1, 0);
    gtk_widget_show (gw_applet->pref_basic_update_spin);

    gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (gw_applet->pref_basic_update_spin), TRUE);
    gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (gw_applet->pref_basic_update_spin), GTK_UPDATE_IF_VALID);
    g_signal_connect (G_OBJECT (gw_applet->pref_basic_update_spin), "value_changed",
    		      G_CALLBACK (update_interval_changed), gw_applet);

    pref_basic_update_sec_lbl = gtk_label_new (_("minute(s)"));
    gtk_widget_show (pref_basic_update_sec_lbl);

    gtk_box_pack_start (GTK_BOX (pref_basic_update_hbox), pref_basic_update_alignment, FALSE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (pref_basic_update_hbox), gw_applet->pref_basic_update_spin, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (pref_basic_update_hbox), pref_basic_update_sec_lbl, FALSE, FALSE, 0);

    gtk_container_add (GTK_CONTAINER (frame), pref_basic_update_hbox);
    gtk_box_pack_start (GTK_BOX (pref_basic_vbox), frame, FALSE, TRUE, 0);

    /* The Miscellaneous frame */
    frame = gtk_frame_new (_("Miscellaneous"));
    gtk_container_border_width (GTK_CONTAINER (frame), GNOME_PAD_SMALL);

    vbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
    gtk_container_border_width (GTK_CONTAINER (vbox), GNOME_PAD_SMALL);

    gtk_box_pack_start (GTK_BOX (vbox), pref_basic_metric_alignment, FALSE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), pref_basic_detailed_alignment, FALSE, TRUE, 0);
#ifdef RADARMAP
    gtk_box_pack_start (GTK_BOX (vbox), pref_basic_radar_alignment, FALSE, TRUE, 0);
#endif /* RADARMAP */

    gtk_container_add (GTK_CONTAINER (frame), vbox);
    gtk_box_pack_start (GTK_BOX (pref_basic_vbox), frame, FALSE, TRUE, 0);

    pref_basic_note_lbl = gtk_label_new (_("General"));
    gtk_widget_show (pref_basic_note_lbl);
    gtk_notebook_set_tab_label (GTK_NOTEBOOK (pref_notebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (pref_notebook), 2), pref_basic_note_lbl);


    g_signal_connect (G_OBJECT (gw_applet->pref), "response",
    		      G_CALLBACK (response_cb), gw_applet);

    gtk_widget_show_all (gw_applet->pref);
}


void gweather_pref_load (GWeatherApplet *gw_applet)
{

    gw_applet->gweather_pref.update_interval = 
    	panel_applet_gconf_get_int(gw_applet->applet, "auto_update_interval", NULL);
    gw_applet->gweather_pref.update_enabled =
    	panel_applet_gconf_get_bool(gw_applet->applet, "auto_update", NULL);
    gw_applet->gweather_pref.use_metric = 
    	panel_applet_gconf_get_bool(gw_applet->applet, "enable_metric", NULL);
    gw_applet->gweather_pref.detailed =
    	panel_applet_gconf_get_bool(gw_applet->applet, "enable_detailed_forecast", NULL);
    gw_applet->gweather_pref.radar_enabled =
    	panel_applet_gconf_get_bool(gw_applet->applet, "enable_radar_map", NULL);
    gw_applet->gweather_pref.proxy_url =
    	panel_applet_gconf_get_string(gw_applet->applet, "proxy_url", NULL);
    gw_applet->gweather_pref.proxy_user =
    	panel_applet_gconf_get_string(gw_applet->applet, "proxy_user", NULL);
    gw_applet->gweather_pref.proxy_passwd =
    	panel_applet_gconf_get_string(gw_applet->applet, "proxy_passwd", NULL);
    gw_applet->gweather_pref.use_proxy =
    	panel_applet_gconf_get_bool(gw_applet->applet, "enable_proxy", NULL);
    gw_applet->gweather_pref.location = weather_location_config_read(gw_applet->applet);

	return;
}

static void help_cb (void)
{
/*
    GnomeHelpMenuEntry help_entry = { "gweather_applet",
				      "index.html#GWEATHER-PREFS" };

    gnome_help_display (NULL, &help_entry);
*/
}

void gweather_pref_run (GWeatherApplet *gw_applet)
{
    /* Only one preferences window at a time */
    if (gw_applet->pref) {
        gtk_window_present (GTK_WINDOW (gw_applet->pref));
	return;
    }

    gweather_pref_create(gw_applet);
    update_dialog(gw_applet);

    
}

