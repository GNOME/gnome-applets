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
#include <panel-applet-gconf.h>
#include <gconf/gconf-client.h>
#include <egg-screen-help.h>

#include "gweather.h"
#include "gweather-pref.h"
#include "gweather-applet.h"

#define NEVER_SENSITIVE		"never_sensitive"

enum
{
    COL_LOC = 0,
    COL_POINTER,
    NUM_COLUMNS,
}; 


static void gweather_pref_set_accessibility (GWeatherApplet *gw_applet);
static void help_cb (GtkDialog *dialog);

/* set sensitive and setup NEVER_SENSITIVE appropriately */
static void
hard_set_sensitive (GtkWidget *w, gboolean sensitivity)
{
	gtk_widget_set_sensitive (w, sensitivity);
	g_object_set_data (G_OBJECT (w), NEVER_SENSITIVE,
			   GINT_TO_POINTER ( ! sensitivity));
}


/* set sensitive, but always insensitive if NEVER_SENSITIVE is set */
static void
soft_set_sensitive (GtkWidget *w, gboolean sensitivity)
{
	if (g_object_get_data (G_OBJECT (w), NEVER_SENSITIVE))
		gtk_widget_set_sensitive (w, FALSE);
	else
		gtk_widget_set_sensitive (w, sensitivity);
}


static gboolean
key_writable (PanelApplet *applet, const char *key)
{
	gboolean writable;
	char *fullkey;
	static GConfClient *client = NULL;
	if (client == NULL)
		client = gconf_client_get_default ();

	fullkey = panel_applet_gconf_get_full_key (applet, key);

	writable = gconf_client_key_is_writable (client, fullkey, NULL);

	g_free (fullkey);

	return writable;
}

/* sets up ATK Relation between the widgets */

void
add_atk_relation (GtkWidget *widget1, GtkWidget *widget2, AtkRelationType type)
{
    AtkObject *atk_obj1, *atk_obj2;
    AtkRelationSet *relation_set;
    AtkRelation *relation;
   
    /* If the relation is LABELLED_BY set LABEL_FOR also */

    if (type == ATK_RELATION_LABELLED_BY)
       gtk_label_set_mnemonic_widget (GTK_LABEL (widget2), widget1);

    atk_obj1 = gtk_widget_get_accessible (widget1);
    if (! GTK_IS_ACCESSIBLE (atk_obj1))
       return;
    atk_obj2 = gtk_widget_get_accessible (widget2);

    relation_set = atk_object_ref_relation_set (atk_obj1);
    relation = atk_relation_new (&atk_obj2, 1, type);
    atk_relation_set_add (relation_set, relation);
    g_object_unref (G_OBJECT (relation));
}

/* sets accessible name and description */

void
set_access_namedesc (GtkWidget *widget, const gchar *name, const gchar *desc)
{
    AtkObject *obj;

    obj = gtk_widget_get_accessible (widget);
    if (! GTK_IS_ACCESSIBLE (obj))
       return;

    if ( desc )
       atk_object_set_description (obj, desc);
    if ( name )
       atk_object_set_name (obj, name);
}

/* sets accessible name, description, CONTROLLED_BY 
 * and CONTROLLER_FOR relations for the components
 * in gweather preference dialog.
 */

static void gweather_pref_set_accessibility (GWeatherApplet *gw_applet)
{

    /* Relation between components in General page */

    add_atk_relation (gw_applet->pref_basic_update_btn, gw_applet->pref_basic_update_spin,                                             ATK_RELATION_CONTROLLER_FOR);
    add_atk_relation (gw_applet->pref_basic_radar_btn, gw_applet->pref_basic_radar_url_btn,                                            ATK_RELATION_CONTROLLER_FOR);
    add_atk_relation (gw_applet->pref_basic_radar_btn, gw_applet->pref_basic_radar_url_entry                                            , ATK_RELATION_CONTROLLER_FOR);

    add_atk_relation (gw_applet->pref_basic_update_spin, gw_applet->pref_basic_update_btn,                                             ATK_RELATION_CONTROLLED_BY);
    add_atk_relation (gw_applet->pref_basic_radar_url_btn, gw_applet->pref_basic_radar_btn,                                            ATK_RELATION_CONTROLLED_BY);
    add_atk_relation (gw_applet->pref_basic_radar_url_entry, gw_applet->pref_basic_radar_btn                                            , ATK_RELATION_CONTROLLED_BY);

    /* Accessible Name and Description for the components in Preference Dialog */
   
    set_access_namedesc (gw_applet->pref_tree, _("Location view"),                                                       _("Select Location from the list"));
    
    set_access_namedesc (gw_applet->pref_basic_update_spin, _("Update spin button"),                                      _("Spinbutton for updating"));
    set_access_namedesc (gw_applet->pref_basic_radar_url_entry, _("Address Entry"),                                        _("Enter the URL"));

}

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
    soft_set_sensitive(gw_applet->pref_basic_update_spin, 
    			     gw_applet->gweather_pref.update_enabled);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gw_applet->pref_basic_metric_btn), 
    				 gw_applet->gweather_pref.use_metric);
#if 0
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gw_applet->pref_basic_detailed_btn), 
    				 gw_applet->gweather_pref.detailed);
#endif
#ifdef RADARMAP
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gw_applet->pref_basic_radar_btn),
    				 gw_applet->gweather_pref.radar_enabled);
    				 
    soft_set_sensitive (gw_applet->pref_basic_radar_url_btn, 
    			      gw_applet->gweather_pref.radar_enabled);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gw_applet->pref_basic_radar_url_btn),
    				 gw_applet->gweather_pref.use_custom_radar_url);
    soft_set_sensitive (gw_applet->pref_basic_radar_url_hbox, 
    			      gw_applet->gweather_pref.radar_enabled);
    if (gw_applet->gweather_pref.radar)
    	gtk_entry_set_text (GTK_ENTRY (gw_applet->pref_basic_radar_url_entry),
    			    gw_applet->gweather_pref.radar);
#endif /* RADARMAP */
    
    
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

static void change_cb (GtkButton *button, gpointer user_data)
{
    GWeatherApplet *gw_applet = user_data;
    
    soft_set_sensitive(gw_applet->pref_basic_update_spin, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gw_applet->pref_basic_update_btn)));
    
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

    panel_applet_gconf_set_string(gw_applet->applet, "location0", loc->name, NULL);
    panel_applet_gconf_set_string(gw_applet->applet, "location1", loc->code, NULL);
    panel_applet_gconf_set_string(gw_applet->applet, "location2", loc->zone, NULL);
    panel_applet_gconf_set_string(gw_applet->applet, "location3", loc->radar, NULL);
    if (gw_applet->gweather_pref.location) {
       weather_location_free (gw_applet->gweather_pref.location);
    }
    gw_applet->gweather_pref.location = weather_location_config_read (gw_applet->applet);
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
    gchar *file;
    
    model = GTK_TREE_STORE (gtk_tree_view_get_model (tree));

    file = gnome_datadir_file("gweather/Locations");
    g_return_if_fail (file);
    path = g_strdup_printf("=%s=/", file);
    gnome_config_push_prefix(path);
    g_free(path);
    g_free (file);
    
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
auto_update_toggled (GtkToggleButton *button, gpointer data)
{
    GWeatherApplet *gw_applet = data;
    gboolean toggled;
    
    toggled = gtk_toggle_button_get_active(button);
    gw_applet->gweather_pref.update_enabled = toggled;
    soft_set_sensitive (gw_applet->pref_basic_update_spin, toggled);
    panel_applet_gconf_set_bool(gw_applet->applet, "auto_update", toggled, NULL);
    if (gw_applet->timeout_tag > 0)
        gtk_timeout_remove(gw_applet->timeout_tag);
    if (gw_applet->gweather_pref.update_enabled)
        gw_applet->timeout_tag =  
        	gtk_timeout_add (gw_applet->gweather_pref.update_interval * 1000,
                                 timeout_cb, gw_applet);
}

static void
metric_toggled (GtkToggleButton *button, gpointer data)
{
    GWeatherApplet *gw_applet = data;
    gboolean toggled;
    
    toggled = gtk_toggle_button_get_active(button);
    if (gw_applet->gweather_pref.use_metric == toggled)
        return;
        
    gw_applet->gweather_pref.use_metric = toggled;
    panel_applet_gconf_set_bool(gw_applet->applet, "enable_metric", toggled, NULL);
    
    if (!gw_applet->gweather_info)
        return;
    
    if (gw_applet->gweather_pref.use_metric) {
        weather_info_to_metric (gw_applet->gweather_info);
    }
    else {    
        weather_info_to_imperial (gw_applet->gweather_info); 
    }
    gtk_label_set_text(GTK_LABEL(gw_applet->label), 
    		       weather_info_get_temp_summary(gw_applet->gweather_info));
    gweather_dialog_update (gw_applet); 
}

static void
detailed_toggled (GtkToggleButton *button, gpointer data)
{
    GWeatherApplet *gw_applet = data;
    gboolean toggled;
    
    toggled = gtk_toggle_button_get_active(button);
    gw_applet->gweather_pref.detailed = toggled;
    panel_applet_gconf_set_bool(gw_applet->applet, "enable_detailed_forecast", 
    				toggled, NULL);    
}

static void
radar_toggled (GtkToggleButton *button, gpointer data)
{
    GWeatherApplet *gw_applet = data;
    gboolean toggled;
    
    toggled = gtk_toggle_button_get_active(button);
    gw_applet->gweather_pref.radar_enabled = toggled;
    panel_applet_gconf_set_bool(gw_applet->applet, "enable_radar_map", toggled, NULL);
    soft_set_sensitive (gw_applet->pref_basic_radar_url_btn, toggled);
    soft_set_sensitive (gw_applet->pref_basic_radar_url_hbox, toggled);
}

static void
use_radar_url_toggled (GtkToggleButton *button, gpointer data)
{
    GWeatherApplet *gw_applet = data;
    gboolean toggled;
    
    toggled = gtk_toggle_button_get_active(button);
    gw_applet->gweather_pref.use_custom_radar_url = toggled;
    panel_applet_gconf_set_bool(gw_applet->applet, "use_custom_radar_url", toggled, NULL);
}

static gboolean
radar_url_changed (GtkWidget *widget, GdkEventFocus *event, gpointer data)
{
    GWeatherApplet *gw_applet = data;
    gchar *text;
    
    text = gtk_editable_get_chars (GTK_EDITABLE (widget), 0, -1);
    
    if (gw_applet->gweather_pref.radar)
        g_free (gw_applet->gweather_pref.radar);
        
    if (text) {
    	gw_applet->gweather_pref.radar = g_strdup (text);
    	g_free (text);
    }
    else
    	gw_applet->gweather_pref.radar = NULL;
    	
    panel_applet_gconf_set_string (gw_applet->applet, "radar", 
    				   gw_applet->gweather_pref.radar, NULL);
    				   
    return FALSE;
    				   
}

static void
update_interval_changed (GtkSpinButton *button, gpointer data)
{
    GWeatherApplet *gw_applet = data;
    
    gw_applet->gweather_pref.update_interval = gtk_spin_button_get_value_as_int(button)*60;
    panel_applet_gconf_set_int(gw_applet->applet, "auto_update_interval", 
    		               gw_applet->gweather_pref.update_interval, NULL);
    if (gw_applet->timeout_tag > 0)
        gtk_timeout_remove(gw_applet->timeout_tag);
    if (gw_applet->gweather_pref.update_enabled)
        gw_applet->timeout_tag =  
        	gtk_timeout_add (gw_applet->gweather_pref.update_interval * 1000,
                                 timeout_cb, gw_applet);
}

static gboolean
free_data (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data)
{
   GWeatherApplet *gw_applet = data;
   WeatherLocation *location;
   
   gtk_tree_model_get (model, iter, COL_POINTER, &location, -1);
   if (!location)
   	return FALSE;

   if (location->name)
      	g_free (location->name);
   if (location->code)
      	g_free (location->code);
   if (location->zone)
      	g_free (location->zone);      	
   if (location->radar)
      	g_free (location->radar);	
   weather_location_free (location);
   return FALSE;
}

static void
free_tree (GWeatherApplet *gw_applet)
{
   GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (gw_applet->pref_tree));
   
   gtk_tree_model_foreach (model, free_data, gw_applet);
}

static void
response_cb (GtkDialog *dialog, gint id, gpointer data)
{
    GWeatherApplet *gw_applet = data;
  
    if(id == GTK_RESPONSE_HELP){
        help_cb (dialog);
	return;
    }
    free_tree (gw_applet);
    gtk_widget_destroy (GTK_WIDGET (dialog));
    gw_applet->pref = NULL;
#if 0
    /* refresh the applet incase the location has changed */
    gweather_update(gw_applet);
#endif
}

static void gweather_pref_create (GWeatherApplet *gw_applet)
{
    GtkWidget *pref_vbox;
    GtkWidget *pref_notebook;
    GtkWidget *pref_basic_metric_alignment;
    GtkWidget *pref_basic_detailed_alignment;
#ifdef RADARMAP
    GtkWidget *pref_basic_radar_alignment;
    GtkWidget *pref_basic_radar_url_alignment;
    GtkWidget *pref_basic_radar_url_entry_alignment;
#endif /* RADARMAP */
    GtkWidget *pref_basic_update_alignment;
    GtkWidget *pref_basic_update_lbl;
    GtkWidget *pref_basic_update_hbox;
    GtkObject *pref_basic_update_spin_adj;
    GtkWidget *pref_basic_update_sec_lbl;
    GtkWidget *pref_basic_note_lbl;
    GtkWidget *pref_net_note_lbl;    
    GtkWidget *pref_loc_hbox;
    GtkWidget *pref_loc_note_lbl;
    GtkWidget *scrolled_window;
    GtkWidget *label, *hbox;
    GtkTreeStore *model;
    GtkTreeSelection *selection;
    GtkWidget *pref_basic_vbox;
    GtkWidget *vbox;
    GtkWidget *frame;
    
    gw_applet->pref = gtk_dialog_new_with_buttons (_("Weather Preferences"), NULL,
				      		   GTK_DIALOG_DESTROY_WITH_PARENT,
				      		   GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
				      		   GTK_STOCK_HELP, GTK_RESPONSE_HELP,
				      		   NULL);
    gtk_dialog_set_default_response (GTK_DIALOG (gw_applet->pref), GTK_RESPONSE_CLOSE);
    gtk_window_set_default_size(GTK_WINDOW (gw_applet->pref), -1, 280);
    gtk_window_set_policy (GTK_WINDOW (gw_applet->pref), TRUE, TRUE, FALSE);
    gtk_window_set_screen (GTK_WINDOW (gw_applet->pref),
			   gtk_widget_get_screen (GTK_WIDGET (gw_applet->applet)));

    pref_vbox = GTK_DIALOG (gw_applet->pref)->vbox;
    gtk_widget_show (pref_vbox);

    pref_notebook = gtk_notebook_new ();
    gtk_widget_show (pref_notebook);
    gtk_box_pack_start (GTK_BOX (pref_vbox), pref_notebook, TRUE, TRUE, 0);

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
    pref_basic_radar_url_alignment = gtk_alignment_new (0.1, 0.5, 0, 1);
    gtk_widget_show (pref_basic_radar_url_alignment);
    pref_basic_radar_url_entry_alignment = gtk_alignment_new (0.1, 0.5, 0, 1);
    gtk_widget_show (pref_basic_radar_url_entry_alignment);
#endif /* RADARMAP */

    gw_applet->pref_basic_update_btn = gtk_check_button_new_with_mnemonic (_("_Automatically update every"));
    gtk_widget_show (gw_applet->pref_basic_update_btn);
    gtk_container_add (GTK_CONTAINER (pref_basic_update_alignment), gw_applet->pref_basic_update_btn);
    g_signal_connect (G_OBJECT (gw_applet->pref_basic_update_btn), "toggled",
    		      G_CALLBACK (auto_update_toggled), gw_applet);
    if ( ! key_writable (gw_applet->applet, "auto_update"))
	    hard_set_sensitive (gw_applet->pref_basic_update_btn, FALSE);

    gw_applet->pref_basic_metric_btn = gtk_check_button_new_with_mnemonic (_("Use _metric system units"));
    gtk_widget_show (gw_applet->pref_basic_metric_btn);
    gtk_container_add (GTK_CONTAINER (pref_basic_metric_alignment), gw_applet->pref_basic_metric_btn);
    g_signal_connect (G_OBJECT (gw_applet->pref_basic_metric_btn), "toggled",
    		      G_CALLBACK (metric_toggled), gw_applet);
    if ( ! key_writable (gw_applet->applet, "enable_metric"))
	    hard_set_sensitive (gw_applet->pref_basic_metric_btn, FALSE);

#ifdef RADARMAP
    gw_applet->pref_basic_radar_btn = gtk_check_button_new_with_mnemonic (_("Enable _radar map"));
    gtk_widget_show (gw_applet->pref_basic_radar_btn);
    gtk_container_add (GTK_CONTAINER (pref_basic_radar_alignment), gw_applet->pref_basic_radar_btn);
    g_signal_connect (G_OBJECT (gw_applet->pref_basic_radar_btn), "toggled",
    		      G_CALLBACK (radar_toggled), gw_applet);
    if ( ! key_writable (gw_applet->applet, "enable_radar_map"))
	    hard_set_sensitive (gw_applet->pref_basic_radar_btn, FALSE);
    		      
    gw_applet->pref_basic_radar_url_btn = gtk_check_button_new_with_mnemonic (_("Use cus_tom address for radar map"));
    gtk_widget_show (gw_applet->pref_basic_radar_url_btn);
    gtk_container_add (GTK_CONTAINER (pref_basic_radar_url_alignment), 
    		       gw_applet->pref_basic_radar_url_btn);
    g_signal_connect (G_OBJECT (gw_applet->pref_basic_radar_url_btn), "toggled",
    		      G_CALLBACK (use_radar_url_toggled), gw_applet);
    if ( ! key_writable (gw_applet->applet, "use_custom_radar_url"))
	    hard_set_sensitive (gw_applet->pref_basic_radar_url_btn, FALSE);
    		      
    gw_applet->pref_basic_radar_url_hbox = gtk_hbox_new (FALSE, 4);
    gtk_widget_show (gw_applet->pref_basic_radar_url_hbox);
    gtk_container_add (GTK_CONTAINER (pref_basic_radar_url_entry_alignment), 
    		       gw_applet->pref_basic_radar_url_hbox);
    label = gtk_label_new_with_mnemonic (_("A_ddress :"));
    gtk_widget_show (label);
    gtk_box_pack_start (GTK_BOX (gw_applet->pref_basic_radar_url_hbox),
    			label, FALSE, FALSE, 0); 
    gw_applet->pref_basic_radar_url_entry = gtk_entry_new ();
    gtk_widget_show (gw_applet->pref_basic_radar_url_entry);
    gtk_box_pack_start (GTK_BOX (gw_applet->pref_basic_radar_url_hbox),
    			gw_applet->pref_basic_radar_url_entry, TRUE, TRUE, 0);    
    g_signal_connect (G_OBJECT (gw_applet->pref_basic_radar_url_entry), "focus_out_event",
    		      G_CALLBACK (radar_url_changed), gw_applet);
    if ( ! key_writable (gw_applet->applet, "radar"))
	    hard_set_sensitive (gw_applet->pref_basic_radar_url_entry, FALSE);
#endif /* RADARMAP */

    frame = gtk_frame_new (_("Update"));
    gtk_container_border_width (GTK_CONTAINER (frame), GNOME_PAD_SMALL);

    pref_basic_update_hbox = gtk_hbox_new (FALSE, GNOME_PAD_SMALL);
    gtk_container_border_width (GTK_CONTAINER (pref_basic_update_hbox), GNOME_PAD_SMALL);

    pref_basic_update_lbl = gtk_label_new_with_mnemonic (_("_Automatically update every:"));
    gtk_widget_show (pref_basic_update_lbl);
    

/*
    gtk_label_set_justify (GTK_LABEL (pref_basic_update_lbl), GTK_JUSTIFY_RIGHT);
    gtk_misc_set_alignment (GTK_MISC (pref_basic_update_lbl), 1, 0.5);
*/

    gtk_widget_show (pref_basic_update_hbox);

    pref_basic_update_spin_adj = gtk_adjustment_new (30, 1, 3600, 5, 25, 1);
    gw_applet->pref_basic_update_spin = gtk_spin_button_new (GTK_ADJUSTMENT (pref_basic_update_spin_adj), 1, 0);
    gtk_widget_show (gw_applet->pref_basic_update_spin);

    gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (gw_applet->pref_basic_update_spin), TRUE);
    gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (gw_applet->pref_basic_update_spin), GTK_UPDATE_IF_VALID);
    g_signal_connect (G_OBJECT (gw_applet->pref_basic_update_spin), "value_changed",
    		      G_CALLBACK (update_interval_changed), gw_applet);
    
    pref_basic_update_sec_lbl = gtk_label_new (_("minutes"));
    gtk_widget_show (pref_basic_update_sec_lbl);
    if ( ! key_writable (gw_applet->applet, "auto_update_interval")) {
	    hard_set_sensitive (gw_applet->pref_basic_update_spin, FALSE);
	    hard_set_sensitive (pref_basic_update_sec_lbl, FALSE);
    }

    gtk_box_pack_start (GTK_BOX (pref_basic_update_hbox), pref_basic_update_alignment, FALSE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (pref_basic_update_hbox), gw_applet->pref_basic_update_spin, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (pref_basic_update_hbox), pref_basic_update_sec_lbl, FALSE, FALSE, 0);

    gtk_container_add (GTK_CONTAINER (frame), pref_basic_update_hbox);
    gtk_box_pack_start (GTK_BOX (pref_basic_vbox), frame, FALSE, TRUE, 0);

    /* The Miscellaneous frame */
    frame = gtk_frame_new (_("Display"));
    gtk_container_border_width (GTK_CONTAINER (frame), GNOME_PAD_SMALL);

    vbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
    gtk_container_border_width (GTK_CONTAINER (vbox), GNOME_PAD_SMALL);

    gtk_box_pack_start (GTK_BOX (vbox), pref_basic_metric_alignment, TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), pref_basic_detailed_alignment, TRUE, TRUE, 0);
#ifdef RADARMAP
    gtk_box_pack_start (GTK_BOX (vbox), pref_basic_radar_alignment, TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), pref_basic_radar_url_alignment, TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), pref_basic_radar_url_entry_alignment, TRUE, 
    			TRUE, 0);
#endif /* RADARMAP */

    gtk_container_add (GTK_CONTAINER (frame), vbox);
    gtk_box_pack_start (GTK_BOX (pref_basic_vbox), frame, FALSE, TRUE, 0);

    pref_basic_note_lbl = gtk_label_new (_("General"));
    gtk_widget_show (pref_basic_note_lbl);
    gtk_notebook_set_tab_label (GTK_NOTEBOOK (pref_notebook), 
    				gtk_notebook_get_nth_page (GTK_NOTEBOOK (pref_notebook), 0),
    				pref_basic_note_lbl);

  /*
   * Location page.
   */
    pref_loc_hbox = gtk_hbox_new (FALSE, 2);
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
    
    gtk_container_add (GTK_CONTAINER (scrolled_window), gw_applet->pref_tree);
    gtk_widget_show (gw_applet->pref_tree);
    gtk_widget_show (scrolled_window);
    gtk_box_pack_start (GTK_BOX (pref_loc_hbox), scrolled_window, TRUE, TRUE, 0);
    load_locations(gw_applet);
    
    if ( ! key_writable (gw_applet->applet, "location0")) {
	    hard_set_sensitive (scrolled_window, FALSE);
    }

    pref_loc_note_lbl = gtk_label_new (_("Location"));
    gtk_widget_show (pref_loc_note_lbl);
    gtk_notebook_set_tab_label (GTK_NOTEBOOK (pref_notebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (pref_notebook), 1), pref_loc_note_lbl);


    g_signal_connect (G_OBJECT (gw_applet->pref), "response",
    		      G_CALLBACK (response_cb), gw_applet);
   
    gweather_pref_set_accessibility (gw_applet); 
    add_atk_relation (gw_applet->pref_basic_update_spin, pref_basic_update_sec_lbl,
                                            ATK_RELATION_LABELLED_BY);
    add_atk_relation (gw_applet->pref_basic_radar_url_entry, label, ATK_RELATION_LABELLED_BY);
 


    gtk_widget_show_all (gw_applet->pref);
}


void gweather_pref_load (GWeatherApplet *gw_applet)
{
    GError *error = NULL;

    gw_applet->gweather_pref.update_interval = 
    	panel_applet_gconf_get_int(gw_applet->applet, "auto_update_interval", &error);
    if (error) {
	g_print ("%s \n", error->message);
	g_error_free (error);
	error = NULL;
    }
    gw_applet->gweather_pref.update_interval = MAX (gw_applet->gweather_pref.update_interval, 60);
    gw_applet->gweather_pref.update_enabled =
    	panel_applet_gconf_get_bool(gw_applet->applet, "auto_update", NULL);
    gw_applet->gweather_pref.use_metric = 
    	panel_applet_gconf_get_bool(gw_applet->applet, "enable_metric", NULL);
    gw_applet->gweather_pref.detailed =
    	panel_applet_gconf_get_bool(gw_applet->applet, "enable_detailed_forecast", NULL);
    gw_applet->gweather_pref.radar_enabled =
    	panel_applet_gconf_get_bool(gw_applet->applet, "enable_radar_map", NULL);
    gw_applet->gweather_pref.location = weather_location_config_read(gw_applet->applet);
    gw_applet->gweather_pref.use_custom_radar_url = 
    	panel_applet_gconf_get_bool(gw_applet->applet, "use_custom_radar_url", NULL);
    gw_applet->gweather_pref.radar = panel_applet_gconf_get_string (gw_applet->applet,
    								    "radar",
    								    NULL);
    return;
}

static void help_cb (GtkDialog *dialog)
{
    GError *error = NULL;

    egg_help_display_on_screen (
		"gweather", "gweather-prefs",
		gtk_window_get_screen (GTK_WINDOW (dialog)),
		&error);

    if (error) { /* FIXME: the user needs to see this error */
        g_warning ("help error: %s\n", error->message);
        g_error_free (error);
        error = NULL;
    }
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

