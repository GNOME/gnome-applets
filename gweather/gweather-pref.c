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
    NUM_COLUMNS
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

static void load_region (GWeatherApplet *gw_applet, GtkTreeIter *parent_iter, const gchar *region_key)
{
    GtkTreeView *tree;
    GtkTreeStore *model;
    GtkTreeIter region_iter;
    gchar *subregions_key;
    gint isubregions, nsubregions;
    gchar **subregions;
    gchar *name_key;
    gchar *name;
    void *iter;
    gchar *iter_key, *iter_val;
    
    tree = GTK_TREE_VIEW (gw_applet->pref_tree);
    model = GTK_TREE_STORE (gtk_tree_view_get_model (tree));
    
    name_key = g_strconcat (region_key, "/name", NULL);
    name = gnome_config_get_string (name_key);
    
    gtk_tree_store_insert (model, &region_iter, parent_iter, 0);
    gtk_tree_store_set (model, &region_iter, COL_LOC, name, -1);
    
    subregions_key = g_strconcat (region_key, "/regions", NULL);
    gnome_config_get_vector (subregions_key, &nsubregions, &subregions);
    
    for (isubregions = nsubregions-1;  isubregions >= 0;  isubregions--) {
        gchar *subregion_key = g_strconcat (region_key, "_", subregions[isubregions], NULL);
        
        load_region (gw_applet, &region_iter, subregion_key);
        
        g_free (subregion_key);
    }
    
    iter = gnome_config_init_iterator (region_key);
    while ((iter = gnome_config_iterator_next (iter, &iter_key, &iter_val)) != NULL) {
        if (strstr (iter_key, "loc") != NULL) {
            GtkTreeIter loc_iter;
            gchar **locdata;
            gint nlocdata;
            WeatherLocation *weather_location;
            
            gnome_config_make_vector (iter_val, &nlocdata, &locdata);
            g_return_if_fail (nlocdata == 4);
            
            weather_location = weather_location_new (locdata[0], 
                                                     locdata[1], 
                                                     locdata[2], 
                                                     locdata[3]);
            gtk_tree_store_insert (model, &loc_iter, &region_iter, 0);
            gtk_tree_store_set (model, &loc_iter, COL_LOC, locdata[0], 
        			COL_POINTER, weather_location, -1);
            
            if (gw_applet->gweather_pref.location && weather_location_equal (weather_location, gw_applet->gweather_pref.location)) {
                GtkTreePath *path;
                path = gtk_tree_model_get_path (GTK_TREE_MODEL (model), 
                                                &loc_iter);
                gtk_tree_view_expand_to_path (tree, path);
                gtk_tree_view_set_cursor (tree, path, NULL, FALSE);
                gtk_tree_view_scroll_to_cell (tree, path, NULL, TRUE, 0.5, 0.5);
                gtk_tree_path_free (path);
            }
            
            g_strfreev (locdata);
        }
        
        g_free(iter_key);
        g_free(iter_val);
    }
    
    g_free (name);
    g_free (name_key);
    g_free (subregions_key);
    g_strfreev (subregions);
}

static void load_locations (GWeatherApplet *gw_applet)
{
    GtkTreeView *tree = GTK_TREE_VIEW(gw_applet->pref_tree);
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell_renderer;
    gint iregions, nregions;
    gchar **regions;
    gchar *pp[1], *path;
    gchar *file;
    
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
    
    gnome_config_get_vector ("Main/regions", &nregions, &regions);

    for (iregions = nregions-1;  iregions >= 0;  iregions--) {
        load_region (gw_applet, NULL, regions[iregions]);
    }
    
    g_strfreev (regions);
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
    if (toggled == FALSE || gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (gw_applet->pref_basic_radar_url_btn)) == TRUE)
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
    soft_set_sensitive (gw_applet->pref_basic_radar_url_hbox, toggled);
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

static GtkWidget *
create_hig_catagory (GtkWidget *main_box, gchar *title)
{
	GtkWidget *vbox, *vbox2, *hbox;
	GtkWidget *label;
	gchar *tmp;
	
	vbox = gtk_vbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (main_box), vbox, FALSE, FALSE, 0);

	tmp = g_strdup_printf ("<b>%s</b>", title);
	label = gtk_label_new (NULL);
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_label_set_markup (GTK_LABEL (label), tmp);
	g_free (tmp);
	gtk_box_pack_start (GTK_BOX (vbox), label, TRUE, FALSE, 0);
	
	hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
	
	label = gtk_label_new ("    ");
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	
	vbox2 = gtk_vbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (hbox), vbox2, TRUE, TRUE, 0);

	return vbox2;		
}

static void gweather_pref_create (GWeatherApplet *gw_applet)
{
    GtkWidget *pref_vbox;
    GtkWidget *pref_notebook;
#ifdef RADARMAP
    GtkWidget *radar_toggle_hbox;
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
    GtkWidget *label, *value_hbox, *tree_label;
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
    gtk_dialog_set_has_separator (GTK_DIALOG (gw_applet->pref), FALSE);
    gtk_container_set_border_width (GTK_CONTAINER (gw_applet->pref), 5);
    gtk_window_set_default_size(GTK_WINDOW (gw_applet->pref), -1, 325);
    gtk_window_set_policy (GTK_WINDOW (gw_applet->pref), TRUE, TRUE, FALSE);
    gtk_window_set_screen (GTK_WINDOW (gw_applet->pref),
			   gtk_widget_get_screen (GTK_WIDGET (gw_applet->applet)));

    pref_vbox = GTK_DIALOG (gw_applet->pref)->vbox;
    gtk_box_set_spacing (GTK_BOX (pref_vbox), 2);
    gtk_widget_show (pref_vbox);

    pref_notebook = gtk_notebook_new ();
    gtk_container_set_border_width (GTK_CONTAINER (pref_notebook), 5);
    gtk_widget_show (pref_notebook);
    gtk_box_pack_start (GTK_BOX (pref_vbox), pref_notebook, TRUE, TRUE, 0);

  /*
   * General settings page.
   */

    pref_basic_vbox = gtk_vbox_new (FALSE, 18);
    gtk_container_set_border_width (GTK_CONTAINER (pref_basic_vbox), 12);
    gtk_container_add (GTK_CONTAINER (pref_notebook), pref_basic_vbox);

    pref_basic_update_alignment = gtk_alignment_new (0, 0.5, 0, 1);
    gtk_widget_show (pref_basic_update_alignment);

    gw_applet->pref_basic_update_btn = gtk_check_button_new_with_mnemonic (_("_Automatically update every:"));
    gtk_widget_show (gw_applet->pref_basic_update_btn);
    gtk_container_add (GTK_CONTAINER (pref_basic_update_alignment), gw_applet->pref_basic_update_btn);
    g_signal_connect (G_OBJECT (gw_applet->pref_basic_update_btn), "toggled",
    		      G_CALLBACK (auto_update_toggled), gw_applet);
    if ( ! key_writable (gw_applet->applet, "auto_update"))
	    hard_set_sensitive (gw_applet->pref_basic_update_btn, FALSE);

    gw_applet->pref_basic_metric_btn = gtk_check_button_new_with_mnemonic (_("Use _metric system units"));
    gtk_widget_show (gw_applet->pref_basic_metric_btn);
    g_signal_connect (G_OBJECT (gw_applet->pref_basic_metric_btn), "toggled",
    		      G_CALLBACK (metric_toggled), gw_applet);
    if ( ! key_writable (gw_applet->applet, "enable_metric"))
	    hard_set_sensitive (gw_applet->pref_basic_metric_btn, FALSE);

#ifdef RADARMAP
    gw_applet->pref_basic_radar_btn = gtk_check_button_new_with_mnemonic (_("Enable _radar map"));
    gtk_widget_show (gw_applet->pref_basic_radar_btn);
    g_signal_connect (G_OBJECT (gw_applet->pref_basic_radar_btn), "toggled",
    		      G_CALLBACK (radar_toggled), gw_applet);
    if ( ! key_writable (gw_applet->applet, "enable_radar_map"))
	    hard_set_sensitive (gw_applet->pref_basic_radar_btn, FALSE);
    
    radar_toggle_hbox = gtk_hbox_new (FALSE, 12);
    gtk_widget_show (radar_toggle_hbox);
    
    label = gtk_label_new ("    ");
    gtk_widget_show (label);
    gtk_box_pack_start (GTK_BOX (radar_toggle_hbox), label, FALSE, FALSE, 0); 
					      
    gw_applet->pref_basic_radar_url_btn = gtk_check_button_new_with_mnemonic (_("Use cus_tom address for radar map"));
    gtk_widget_show (gw_applet->pref_basic_radar_url_btn);
    gtk_box_pack_start (GTK_BOX (radar_toggle_hbox), gw_applet->pref_basic_radar_url_btn, FALSE, FALSE, 0);

    g_signal_connect (G_OBJECT (gw_applet->pref_basic_radar_url_btn), "toggled",
    		      G_CALLBACK (use_radar_url_toggled), gw_applet);
    if ( ! key_writable (gw_applet->applet, "use_custom_radar_url"))
	    hard_set_sensitive (gw_applet->pref_basic_radar_url_btn, FALSE);
    		      
    gw_applet->pref_basic_radar_url_hbox = gtk_hbox_new (FALSE, 12);
    gtk_widget_show (gw_applet->pref_basic_radar_url_hbox);

    label = gtk_label_new ("    ");
    gtk_widget_show (label);
    gtk_box_pack_start (GTK_BOX (gw_applet->pref_basic_radar_url_hbox),
    			label, FALSE, FALSE, 0); 
			
    label = gtk_label_new_with_mnemonic (_("A_ddress:"));
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

    frame = create_hig_catagory (pref_basic_vbox, _("Update"));

    pref_basic_update_hbox = gtk_hbox_new (FALSE, 12);

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

    value_hbox = gtk_hbox_new (FALSE, 6);

    gtk_box_pack_start (GTK_BOX (pref_basic_update_hbox), pref_basic_update_alignment, FALSE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (pref_basic_update_hbox), value_hbox, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (value_hbox), gw_applet->pref_basic_update_spin, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (value_hbox), pref_basic_update_sec_lbl, FALSE, FALSE, 0);

    gtk_container_add (GTK_CONTAINER (frame), pref_basic_update_hbox);

    frame = create_hig_catagory (pref_basic_vbox, _("Display"));

    vbox = gtk_vbox_new (FALSE, 6);

    gtk_box_pack_start (GTK_BOX (vbox), gw_applet->pref_basic_metric_btn, TRUE, TRUE, 0);
#ifdef RADARMAP
    gtk_box_pack_start (GTK_BOX (vbox), gw_applet->pref_basic_radar_btn, TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), radar_toggle_hbox, TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), gw_applet->pref_basic_radar_url_hbox, TRUE, 
    			TRUE, 0);
#endif /* RADARMAP */

    gtk_container_add (GTK_CONTAINER (frame), vbox);

    pref_basic_note_lbl = gtk_label_new (_("General"));
    gtk_widget_show (pref_basic_note_lbl);
    gtk_notebook_set_tab_label (GTK_NOTEBOOK (pref_notebook), 
    				gtk_notebook_get_nth_page (GTK_NOTEBOOK (pref_notebook), 0),
    				pref_basic_note_lbl);

  /*
   * Location page.
   */
    pref_loc_hbox = gtk_vbox_new (FALSE, 6);
    gtk_container_set_border_width (GTK_CONTAINER (pref_loc_hbox), 12);
    gtk_container_add (GTK_CONTAINER (pref_notebook), pref_loc_hbox);

    tree_label = gtk_label_new_with_mnemonic (_("_Select a location:"));
    gtk_misc_set_alignment (GTK_MISC (tree_label), 0.0, 0.5);
    gtk_box_pack_start (GTK_BOX (pref_loc_hbox), tree_label, FALSE, FALSE, 0);

    scrolled_window = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
				    GTK_POLICY_AUTOMATIC,
				    GTK_POLICY_AUTOMATIC);

    model = gtk_tree_store_new (NUM_COLUMNS, G_TYPE_STRING, G_TYPE_POINTER);
    gw_applet->pref_tree = gtk_tree_view_new_with_model (GTK_TREE_MODEL (model));
    gtk_label_set_mnemonic_widget (GTK_LABEL (tree_label), GTK_WIDGET (gw_applet->pref_tree));
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
    gtk_label_set_mnemonic_widget (GTK_LABEL (pref_basic_update_sec_lbl), gw_applet->pref_basic_update_spin);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), gw_applet->pref_basic_radar_url_entry);

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
		"gweather", "gweather-settings",
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

