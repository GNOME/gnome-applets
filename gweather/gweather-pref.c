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
#include <langinfo.h>
#include <locale.h>

#include <gnome.h>
#include <panel-applet.h>
#include <panel-applet-gconf.h>
#include <gconf/gconf-client.h>
#include <egg-screen-help.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

#include "gweather.h"
#include "gweather-pref.h"
#include "gweather-applet.h"
#include "gweather-xml.h"

#define NEVER_SENSITIVE		"never_sensitive"


/* gconf keys */
#define GCONF_TEMP_UNIT     "temperature_unit"
#define GCONF_SPEED_UNIT    "speed_unit"
#define GCONF_PRESSURE_UNIT "pressure_unit"
#define GCONF_DISTANCE_UNIT "distance_unit"

enum
{
    COL_LOC = 0,
    COL_POINTER,
    NUM_COLUMNS
}; 


static GConfEnumStringPair temp_unit_enum_map [] = {
    { TEMP_UNIT_DEFAULT,    "Default" },
    { TEMP_UNIT_KELVIN,     "K"       },
    { TEMP_UNIT_CENTIGRADE, "C"       },
    { TEMP_UNIT_FAHRENHEIT, "F"       },
    { 0, NULL }
};	

static GConfEnumStringPair speed_unit_enum_map [] = {
    { SPEED_UNIT_DEFAULT, "Default" },
    { SPEED_UNIT_MS,      "m/s"     },
    { SPEED_UNIT_KPH,     "km/h"    },
    { SPEED_UNIT_MPH,     "mph"     },
    { SPEED_UNIT_KNOTS,   "knots"   },
	{ 0, NULL }
};

static GConfEnumStringPair pressure_unit_enum_map [] = {
    { PRESSURE_UNIT_DEFAULT, "Default" },
	{ PRESSURE_UNIT_HPA,     "hPa"     },
	{ PRESSURE_UNIT_MB,      "mb"      },
    { PRESSURE_UNIT_MM_HG,   "mmHg"    },
	{ PRESSURE_UNIT_INCH_HG, "inHg"    },
    { 0, NULL }
};

static GConfEnumStringPair distance_unit_enum_map [] = {
    { DISTANCE_UNIT_DEFAULT, "Default" },
    { DISTANCE_UNIT_METERS,  "m"       },
    { DISTANCE_UNIT_KM,      "km"      },
    { DISTANCE_UNIT_MILES,   "mi"      },
    { 0, NULL }
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

    if ( gw_applet->gweather_pref.use_temperature_default) {
        gtk_combo_box_set_active (GTK_COMBO_BOX(gw_applet->pref_basic_temp_combo), 0);
    } else {
		gtk_combo_box_set_active (GTK_COMBO_BOX(gw_applet->pref_basic_temp_combo), 
	                 gw_applet->gweather_pref.temperature_unit -1);
    }
	
    if ( gw_applet->gweather_pref.use_speed_default) {
        gtk_combo_box_set_active (GTK_COMBO_BOX(gw_applet->pref_basic_speed_combo), 0);
    } else {
        gtk_combo_box_set_active (GTK_COMBO_BOX(gw_applet->pref_basic_speed_combo), 
	                 gw_applet->gweather_pref.speed_unit -1);
    }
	
    if ( gw_applet->gweather_pref.use_pressure_default) {
        gtk_combo_box_set_active (GTK_COMBO_BOX(gw_applet->pref_basic_pres_combo), 0);
    } else {
        gtk_combo_box_set_active (GTK_COMBO_BOX(gw_applet->pref_basic_pres_combo), 
	                 gw_applet->gweather_pref.pressure_unit -1);
    }
    if ( gw_applet->gweather_pref.use_distance_default) {
        gtk_combo_box_set_active (GTK_COMBO_BOX(gw_applet->pref_basic_dist_combo), 0);
    } else {
        gtk_combo_box_set_active (GTK_COMBO_BOX(gw_applet->pref_basic_dist_combo), 
	                 gw_applet->gweather_pref.distance_unit -1);
    }

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
    	
    gtk_tree_model_get (model, &iter, GWEATHER_PREF_COL_POINTER, &loc, -1);
    
    if (!loc)
    	return;

    panel_applet_gconf_set_string(gw_applet->applet, "location0", loc->untrans_name, NULL);
    panel_applet_gconf_set_string(gw_applet->applet, "location1", loc->code, NULL);
    panel_applet_gconf_set_string(gw_applet->applet, "location2", loc->zone, NULL);
    panel_applet_gconf_set_string(gw_applet->applet, "location3", loc->radar, NULL);
    panel_applet_gconf_set_string(gw_applet->applet, "location4", loc->trans_name, NULL);
    if (gw_applet->gweather_pref.location) {
       weather_location_free (gw_applet->gweather_pref.location);
    }
    
    gw_applet->gweather_pref.location = weather_location_config_read (gw_applet->applet);
    gweather_update (gw_applet);
} 

static void load_locations (GWeatherApplet *gw_applet)
{
    GtkTreeView *tree = GTK_TREE_VIEW(gw_applet->pref_tree);
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell_renderer;	
        
    /* Add a colum for the locations */
    cell_renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes ("not used", cell_renderer,
						       "text", GWEATHER_PREF_COL_LOC, NULL);
    gtk_tree_view_append_column (tree, column);
    gtk_tree_view_set_expander_column (GTK_TREE_VIEW (tree), column);
    
    /* load locations from xml file */
    gweather_xml_load_locations(tree, gw_applet->gweather_pref.location);
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
detailed_toggled (GtkToggleButton *button, gpointer data)
{
    GWeatherApplet *gw_applet = data;
    gboolean toggled;
    
    toggled = gtk_toggle_button_get_active(button);
    gw_applet->gweather_pref.detailed = toggled;
    panel_applet_gconf_set_bool(gw_applet->applet, "enable_detailed_forecast", 
    				toggled, NULL);    
}

static void parse_temp_string (const gchar *gconf_str, GWeatherPrefs *prefs)
{
    gint value = 0;
    char *imperial = NULL;
	
    prefs->use_temperature_default = TRUE;
	
    if ( gconf_str && gconf_string_to_enum (temp_unit_enum_map, gconf_str, &value) ) {
        prefs->temperature_unit = value;
		
		if ((prefs->temperature_unit == TEMP_UNIT_DEFAULT) &&
		    (gconf_string_to_enum (temp_unit_enum_map, _("DEFAULT_TEMP_UNIT"), &value)) ) {
              prefs->temperature_unit = value;
		} else {
            prefs->use_temperature_default = FALSE;
        }
    }	
    else {
        /* TRANSLATOR: This is the default unit to use for temperature measurements. */
        /* Valid values are: "K" (Kelvin), "C" (Centigrade) and "F" (Fahrenheit) */
        if (gconf_string_to_enum (temp_unit_enum_map, _("DEFAULT_TEMP_UNIT"), &value) ) {
            prefs->temperature_unit = value;
        }
    }
    if (!prefs->temperature_unit || prefs->temperature_unit == TEMP_UNIT_DEFAULT ) {
        imperial = nl_langinfo(_NL_MEASUREMENT_MEASUREMENT);
        if ( imperial && imperial[0] == 2 )  {
            /* imperial */
            prefs->temperature_unit = TEMP_UNIT_FAHRENHEIT;
        } else {
            prefs->temperature_unit = TEMP_UNIT_CENTIGRADE;
        }
    }
}

static void parse_speed_string (const gchar *gconf_str, GWeatherPrefs *prefs)
{
    gint value = 0;
    char *imperial = NULL;
	
    prefs->use_speed_default = TRUE;
	
    if ( gconf_str && gconf_string_to_enum (speed_unit_enum_map, gconf_str, &value) ) {
        prefs->speed_unit = value;
		if ((prefs->speed_unit == SPEED_UNIT_DEFAULT) &&
			(gconf_string_to_enum (speed_unit_enum_map, _("DEFAULT_SPEED_UNIT"), &value)) ) {
              prefs->speed_unit = value;
		} else {
            prefs->use_speed_default = FALSE;
        }
    }	
    else {
        /* TRANSLATOR: This is the default unit to use for wind speed. */
        /* Valid values are: "m/s" (meters per second), "km/h" (kilometers per hour), */
        /* "mph" (miles per hour) and "knots"  */
        if (gconf_string_to_enum (speed_unit_enum_map, _("DEFAULT_SPEED_UNIT"), &value) ) {
            prefs->speed_unit = value;
        }
    }
    if ( (!prefs->speed_unit) || prefs->speed_unit == SPEED_UNIT_DEFAULT ) {
        imperial = nl_langinfo(_NL_MEASUREMENT_MEASUREMENT);
        if ( imperial && imperial[0] == 2 )  {
            /* imperial */
            prefs->speed_unit = SPEED_UNIT_KNOTS;
        } else {
            prefs->speed_unit = SPEED_UNIT_MS;
        }
    }
}


static void parse_pressure_string (const gchar *gconf_str, GWeatherPrefs *prefs)
{
    gint value = 0;
    char *imperial = NULL;
	
    prefs->use_pressure_default = TRUE;
	
    if ( gconf_str && gconf_string_to_enum (pressure_unit_enum_map, gconf_str, &value) ) {
        prefs->pressure_unit = value;

        if ((prefs->pressure_unit == PRESSURE_UNIT_DEFAULT) &&
			(gconf_string_to_enum (pressure_unit_enum_map, _("DEFAULT_PRESSURE_UNIT"), &value)) ) {
              prefs->pressure_unit = value;
		} else {
            prefs->use_pressure_default = FALSE;
        }
    }	
    else {
        /* TRANSLATOR: This is the default unit to use for atmospheric pressure. */
        /* Valid values are: "hPa" (hectoPascals), "mb" (millibars), "mmHg"  */
		/* (millimeters of mercury) and "inHg" (inches of mercury) */
        if (gconf_string_to_enum (pressure_unit_enum_map, _("DEFAULT_PRESSURE_UNIT"), &value) ) {
            prefs->pressure_unit = value;
        }
    }
    if ( (!prefs->pressure_unit) || prefs->pressure_unit == PRESSURE_UNIT_DEFAULT ) {
        imperial = nl_langinfo(_NL_MEASUREMENT_MEASUREMENT);
        if ( imperial && imperial[0] == 2 )  {
            /* imperial */
            prefs->pressure_unit = PRESSURE_UNIT_INCH_HG;
        } else {
            prefs->pressure_unit = PRESSURE_UNIT_HPA;
        }
    }	
}

static void parse_distance_string (const gchar *gconf_str, GWeatherPrefs *prefs)
{
    gint value = 0;
    char *imperial = NULL;
	
    prefs->use_distance_default = TRUE;
    if ( gconf_str && gconf_string_to_enum (distance_unit_enum_map, gconf_str, &value) ) {
        prefs->distance_unit = value;

		if ((prefs->distance_unit == DISTANCE_UNIT_DEFAULT) &&
			(gconf_string_to_enum (distance_unit_enum_map, _("DEFAULT_DISTANCE_UNIT"), &value)) ) {
              prefs->distance_unit = value;
		} else {
            prefs->use_distance_default = FALSE;
        }
    }	
    else {
        /* TRANSLATOR: This is the default unit to use for visibility distance. */
        /* Valid values are: "m" (meters), "km" (kilometers) and "mi" (miles)   */
        if (gconf_string_to_enum (distance_unit_enum_map, _("DEFAULT_DISTANCE_UNIT"), &value) ) {
            prefs->distance_unit = value;
        }
    }

	if ((!prefs->distance_unit) || prefs->distance_unit == DISTANCE_UNIT_DEFAULT ) {
        imperial = nl_langinfo(_NL_MEASUREMENT_MEASUREMENT);
        if ( imperial && imperial[0] == 2 )  {
            /* imperial */
            prefs->distance_unit = DISTANCE_UNIT_MILES;
        } else {
            prefs->distance_unit = DISTANCE_UNIT_METERS;
        }
    }

    return;
}	
static void temp_combo_changed_cb (GtkComboBox *combo, gpointer data)
{
    GWeatherApplet *gw_applet = data;
    TempUnit       new_unit, *old_unit;
	
	g_return_if_fail(gw_applet != NULL);
    
	old_unit = &gw_applet->gweather_pref.temperature_unit;	
	new_unit = gtk_combo_box_get_active(combo) + 1;

	if (new_unit == *old_unit)
        return;

    parse_temp_string (gconf_enum_to_string (temp_unit_enum_map, new_unit), &(gw_applet->gweather_pref));

    panel_applet_gconf_set_string(gw_applet->applet, GCONF_TEMP_UNIT,
                                  gconf_enum_to_string (temp_unit_enum_map, new_unit),
                                  NULL);
	
    gtk_label_set_text(GTK_LABEL(gw_applet->label), 
	                   weather_info_get_temp_summary(gw_applet->gweather_info));
    gweather_dialog_update (gw_applet);
}

static void speed_combo_changed_cb (GtkComboBox *combo, gpointer data)
{
    GWeatherApplet *gw_applet = data;
    SpeedUnit      new_unit, *old_unit;
	
	g_return_if_fail(gw_applet != NULL);
    
	old_unit = &gw_applet->gweather_pref.speed_unit;	
	new_unit = gtk_combo_box_get_active(combo) + 1;

	if (new_unit == *old_unit)
        return;

    parse_speed_string (gconf_enum_to_string (speed_unit_enum_map, new_unit), &(gw_applet->gweather_pref));

    panel_applet_gconf_set_string(gw_applet->applet, GCONF_SPEED_UNIT,
                                  gconf_enum_to_string (speed_unit_enum_map, new_unit),
                                  NULL);
	
    gweather_dialog_update (gw_applet);
}

static void pres_combo_changed_cb (GtkComboBox *combo, gpointer data)
{
    GWeatherApplet *gw_applet = data;
    PressureUnit   new_unit, *old_unit;
	
	g_return_if_fail(gw_applet != NULL);
    
	old_unit = &gw_applet->gweather_pref.pressure_unit;	
	new_unit = gtk_combo_box_get_active(combo) + 1;

	if (new_unit == *old_unit)
        return;

    parse_pressure_string (gconf_enum_to_string (pressure_unit_enum_map, new_unit), &(gw_applet->gweather_pref));

    panel_applet_gconf_set_string(gw_applet->applet, GCONF_PRESSURE_UNIT,
                                  gconf_enum_to_string (pressure_unit_enum_map, new_unit),
                                  NULL);
	
    gweather_dialog_update (gw_applet);
}

static void dist_combo_changed_cb (GtkComboBox *combo, gpointer data)
{
    GWeatherApplet *gw_applet = data;
    DistanceUnit   new_unit, *old_unit;
	
	g_return_if_fail(gw_applet != NULL);
    
	old_unit = &gw_applet->gweather_pref.distance_unit;	
	new_unit = gtk_combo_box_get_active(combo) + 1;

	if (new_unit == *old_unit)
        return;

    parse_distance_string (gconf_enum_to_string (distance_unit_enum_map, new_unit), &(gw_applet->gweather_pref));

    panel_applet_gconf_set_string(gw_applet->applet, GCONF_DISTANCE_UNIT,
                                  gconf_enum_to_string (distance_unit_enum_map, new_unit),
                                  NULL);
	
    gweather_dialog_update (gw_applet);
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
   
   gtk_tree_model_get (model, iter, GWEATHER_PREF_COL_POINTER, &location, -1);
   if (!location)
   	return FALSE;

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
    GtkWidget *temp_label;
    GtkWidget *temp_combo;
    GtkWidget *speed_label;
    GtkWidget *speed_combo;	
    GtkWidget *pres_label;
    GtkWidget *pres_combo;
    GtkWidget *dist_label;
    GtkWidget *dist_combo;
    GtkWidget *unit_table;	
    
    gw_applet->pref = gtk_dialog_new_with_buttons (_("Weather Preferences"), NULL,
				      		   GTK_DIALOG_DESTROY_WITH_PARENT,
				      		   GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
				      		   GTK_STOCK_HELP, GTK_RESPONSE_HELP,
				      		   NULL);

    gtk_dialog_set_default_response (GTK_DIALOG (gw_applet->pref), GTK_RESPONSE_CLOSE);
    gtk_dialog_set_has_separator (GTK_DIALOG (gw_applet->pref), FALSE);
    gtk_container_set_border_width (GTK_CONTAINER (gw_applet->pref), 5);
    gtk_window_set_resizable (GTK_WINDOW (gw_applet->pref), FALSE);
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

    /* Temperature Unit */
    temp_label = gtk_label_new_with_mnemonic (_("_Temperature Unit:"));
    gtk_label_set_use_markup (GTK_LABEL (temp_label), TRUE);
    gtk_label_set_justify (GTK_LABEL (temp_label), GTK_JUSTIFY_LEFT);
    gtk_misc_set_alignment (GTK_MISC (temp_label), 0, 0.5);
    gtk_widget_show (temp_label);

    temp_combo = gtk_combo_box_new_text ();
	gw_applet->pref_basic_temp_combo = temp_combo;
    gtk_label_set_mnemonic_widget (GTK_LABEL (temp_label), temp_combo);
    gtk_combo_box_append_text (GTK_COMBO_BOX (temp_combo), _("Default"));
    gtk_combo_box_append_text (GTK_COMBO_BOX (temp_combo), _("Kelvin"));
    gtk_combo_box_append_text (GTK_COMBO_BOX (temp_combo), _("Centigrade"));
    gtk_combo_box_append_text (GTK_COMBO_BOX (temp_combo), _("Fahrenheit"));
	gtk_widget_show (temp_combo);
		
    if ( ! key_writable (gw_applet->applet, GCONF_TEMP_UNIT))
        hard_set_sensitive (gw_applet->pref_basic_temp_combo, FALSE);
	
    /* Speed Unit */
    speed_label = gtk_label_new_with_mnemonic (_("_Wind Speed Unit:"));
    gtk_label_set_use_markup (GTK_LABEL (speed_label), TRUE);
    gtk_label_set_justify (GTK_LABEL (speed_label), GTK_JUSTIFY_LEFT);
    gtk_misc_set_alignment (GTK_MISC (speed_label), 0, 0.5);
    gtk_widget_show (speed_label);

    speed_combo = gtk_combo_box_new_text ();
    gw_applet->pref_basic_speed_combo = speed_combo;
    gtk_label_set_mnemonic_widget (GTK_LABEL (speed_label), speed_combo);
    gtk_combo_box_append_text (GTK_COMBO_BOX (speed_combo), _("Default"));
    /* TRANSLATOR: The wind speed unit "meters per second" */    
    gtk_combo_box_append_text (GTK_COMBO_BOX (speed_combo), _("m/s"));
    /* TRANSLATOR: The wind speed unit "kilometers per hour" */
    gtk_combo_box_append_text (GTK_COMBO_BOX (speed_combo), _("km/h"));
    /* TRANSLATOR: The wind speed unit "miles per hour" */
    gtk_combo_box_append_text (GTK_COMBO_BOX (speed_combo), _("mph"));
    /* TRANSLATOR: The wind speed unit "knots" */
	gtk_combo_box_append_text (GTK_COMBO_BOX (speed_combo), _("knots"));
	gtk_widget_show (speed_combo);

    if ( ! key_writable (gw_applet->applet, GCONF_SPEED_UNIT))
        hard_set_sensitive (gw_applet->pref_basic_speed_combo, FALSE);

    /* Pressure Unit */
    pres_label = gtk_label_new_with_mnemonic (_("_Pressure Unit:"));
    gtk_label_set_use_markup (GTK_LABEL (pres_label), TRUE);
    gtk_label_set_justify (GTK_LABEL (pres_label), GTK_JUSTIFY_LEFT);
    gtk_misc_set_alignment (GTK_MISC (pres_label), 0, 0.5);
    gtk_widget_show (pres_label);

    pres_combo = gtk_combo_box_new_text ();
	gw_applet->pref_basic_pres_combo = pres_combo;
    gtk_label_set_mnemonic_widget (GTK_LABEL (pres_label), pres_combo);
    gtk_combo_box_append_text (GTK_COMBO_BOX (pres_combo), _("Default"));
    /* TRANSLATOR: The pressure unit "hectoPascals" */
    gtk_combo_box_append_text (GTK_COMBO_BOX (pres_combo), _("hPa"));
    /* TRANSLATOR: The pressure unit "millibars" */
    gtk_combo_box_append_text (GTK_COMBO_BOX (pres_combo), _("mb"));
    /* TRANSLATOR: The pressure unit "millibars of mercury" */
    gtk_combo_box_append_text (GTK_COMBO_BOX (pres_combo), _("mmHg"));
    /* TRANSLATOR: The pressure unit "inches of mercury" */
    gtk_combo_box_append_text (GTK_COMBO_BOX (pres_combo), _("inHg"));
    gtk_widget_show (pres_combo);

    if ( ! key_writable (gw_applet->applet, GCONF_PRESSURE_UNIT))
        hard_set_sensitive (gw_applet->pref_basic_pres_combo, FALSE);

    /* Distance Unit */
    dist_label = gtk_label_new_with_mnemonic (_("_Visibility Unit:"));
    gtk_label_set_use_markup (GTK_LABEL (dist_label), TRUE);
    gtk_label_set_justify (GTK_LABEL (dist_label), GTK_JUSTIFY_LEFT);
    gtk_misc_set_alignment (GTK_MISC (dist_label), 0, 0.5);
    gtk_widget_show (dist_label);

    dist_combo = gtk_combo_box_new_text ();
	gw_applet->pref_basic_dist_combo = dist_combo;
    gtk_label_set_mnemonic_widget (GTK_LABEL (dist_label), dist_combo);
    gtk_combo_box_append_text (GTK_COMBO_BOX (dist_combo), _("Default"));
    /* TRANSLATOR: The distance unit "meters" */
    gtk_combo_box_append_text (GTK_COMBO_BOX (dist_combo), _("meters"));
    /* TRANSLATOR: The distance unit "kilometers" */
    gtk_combo_box_append_text (GTK_COMBO_BOX (dist_combo), _("km"));
    /* TRANSLATOR: The distance unit "miles" */
    gtk_combo_box_append_text (GTK_COMBO_BOX (dist_combo), _("miles"));
	gtk_widget_show (dist_combo);

    if ( ! key_writable (gw_applet->applet, GCONF_DISTANCE_UNIT))
        hard_set_sensitive (gw_applet->pref_basic_dist_combo, FALSE);
	
	unit_table = gtk_table_new(5, 3, FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(unit_table), 6);
	gtk_table_attach_defaults(GTK_TABLE(unit_table), temp_label,  0, 1, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(unit_table), temp_combo,  1, 2, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(unit_table), speed_label, 0, 1, 1, 2);
	gtk_table_attach_defaults(GTK_TABLE(unit_table), speed_combo, 1, 2, 1, 2);
	gtk_table_attach_defaults(GTK_TABLE(unit_table), pres_label,  0, 1, 2, 3);
	gtk_table_attach_defaults(GTK_TABLE(unit_table), pres_combo,  1, 2, 2, 3);
	gtk_table_attach_defaults(GTK_TABLE(unit_table), dist_label,  0, 1, 3, 4);
	gtk_table_attach_defaults(GTK_TABLE(unit_table), dist_combo,  1, 2, 3, 4);
	gtk_widget_show(unit_table);
	
	g_signal_connect (temp_combo, "changed", G_CALLBACK (temp_combo_changed_cb), gw_applet);
	g_signal_connect (speed_combo, "changed", G_CALLBACK (speed_combo_changed_cb), gw_applet);
	g_signal_connect (dist_combo, "changed", G_CALLBACK (dist_combo_changed_cb), gw_applet);
	g_signal_connect (pres_combo, "changed", G_CALLBACK (pres_combo_changed_cb), gw_applet);

	
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
					      
    gw_applet->pref_basic_radar_url_btn = gtk_check_button_new_with_mnemonic (_("Use _custom address for radar map"));
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

	gtk_box_pack_start (GTK_BOX (vbox), unit_table, TRUE, TRUE, 0);

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
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window), GTK_SHADOW_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
				    GTK_POLICY_AUTOMATIC,
				    GTK_POLICY_AUTOMATIC);

    model = gtk_tree_store_new (GWEATHER_PREF_NUM_COLUMNS, G_TYPE_STRING, G_TYPE_POINTER);
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
	gchar  *gconf_str = NULL;
	gint   value = 0;
	GWeatherPrefs *prefs = &(gw_applet->gweather_pref);

	/* Assume we use unit defaults */
	prefs->use_temperature_default = TRUE;
    prefs->use_speed_default = TRUE;
    prefs->use_pressure_default = TRUE;
    prefs->use_distance_default = TRUE;
	
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
    
    gconf_str = panel_applet_gconf_get_string (gw_applet->applet, GCONF_TEMP_UNIT, NULL);
    parse_temp_string(gconf_str, prefs);
	g_free (gconf_str);
    
		
    gconf_str = panel_applet_gconf_get_string (gw_applet->applet, GCONF_SPEED_UNIT, NULL);
	parse_speed_string(gconf_str, prefs);
	g_free (gconf_str);
			
    gconf_str = panel_applet_gconf_get_string (gw_applet->applet, GCONF_PRESSURE_UNIT, NULL);
	parse_pressure_string(gconf_str, prefs);
	g_free (gconf_str);

    gconf_str = panel_applet_gconf_get_string (gw_applet->applet, GCONF_DISTANCE_UNIT, NULL);
	parse_distance_string(gconf_str, prefs);
	g_free (gconf_str);
	
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
