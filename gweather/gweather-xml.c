/* gweather-xml.c - Locations.xml parsing code
 *
 * Copyright (C) 2004 Gareth Owen
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


/* There is very little error checking in the parsing code below, it relies 
 * heavily on the locations file being in the correct format.
 * It also does not enforce a limit on the frequency or ordering of
 * the elements within a particular element.
 * The format is as follows: 
 * 
 * <gweather format="1.0">
 * <region>
 *  <name>Name of the region</name>
 *  <country>
 *   <name>Name of the country</name>
 *   <location>
 *    <name>Name of the location</name>
 *    <code>IWIN code</code>
 *    <zone>Forecast code (North America, Australia, UK only)</zone>
 *    <radar>Weather.com radar map code (North America only)</radar>
 *    <coordinates>Latitude and longitude as DD-MM[-SS][H] pair</coordinates>
 *   </location>
 *   <state>
 *     <location>
 *       ....
 *     </location>
 *     <city>
 *      <name>Name of city with multiple locations</city>
 *      <zone>Forecast code</zone>
 *      <radar>Radar Map code</radar>
 *      <location>
 *        ...
 *      </location>
 *     </city>
 *   </state>
 *  </country>
 * </region>
 * <gweather>
 *
 * The thing to note is that each country can either contain different locations
 * or be split into "states" which in turn contain a list of locations.
 *
 */

#include <string.h>
#include <locale.h>
#include <gtk/gtk.h>
#include <libxml/xmlreader.h>

#include "weather.h"
#include "gweather-pref.h"


/* Location of the xml file */
#define GWEATHER_XML_LOCATION "gweather/Locations.xml"

/* If you change the format of the Locations.xml file that would break the current
 * parsing then make sure you also add an extra define and maintain support for
 * old format.
 */ 
#define GWEATHER_XML_FORMAT_1_0   "1.0"


/* XML Node names */
#define GWEATHER_XML_NODE_GWEATHER   "gweather"
#define GWEATHER_XML_NODE_REGION     "region"
#define GWEATHER_XML_NODE_COUNTRY    "country"
#define GWEATHER_XML_NODE_STATE      "state"
#define GWEATHER_XML_NODE_CITY       "city"
#define GWEATHER_XML_NODE_LOC        "location"
#define GWEATHER_XML_NODE_NAME       "name"
#define GWEATHER_XML_NODE_CODE       "code"
#define GWEATHER_XML_NODE_ZONE       "zone"
#define GWEATHER_XML_NODE_RADAR      "radar"
#define GWEATHER_XML_NODE_COORD      "coordinates"

/* XML Attributes */
#define GWEATHER_XML_ATTR_FORMAT    "format"

/*****************************************************************************
 * Func:    gweather_xml_location_sort_func()
 * Desc:    compare two locales to see if they match
 * Parm:    model:      tree 
 *          a,b:        iterators to sort
 *          user_data:  user data (unused)
 */
static gint gweather_xml_location_sort_func(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer user_data)
{
    gint res;
    gchar *name_a, *name_b;
    gchar *fold_a, *fold_b;
        
    gtk_tree_model_get (model, a, GWEATHER_PREF_COL_LOC, &name_a, -1);
    gtk_tree_model_get (model, b, GWEATHER_PREF_COL_LOC, &name_b, -1);
        
    fold_a = g_utf8_casefold(name_a, -1);
    fold_b = g_utf8_casefold(name_b, -1);
        
    res = g_utf8_collate(fold_a, fold_b);
    
    g_free(name_a);
    g_free(name_b);
    g_free(fold_a);
    g_free(fold_b);
    
    return res;
}
 
/*****************************************************************************
 * Func:    gweather_xml_compare_locale()
 * Desc:    compare two locales to see if they match
 * Parm:    a, b: the two locales to compare
 */
static gint gweather_xml_compare_locale(gconstpointer a, gconstpointer b)
{
    return (strcmp(a, b));
}

/*****************************************************************************
 * Func:    gweather_xml_get_value()
 * Desc:    Extract the value from the xml
 * Parm:    reader: xml text reader
 */
static xmlChar* gweather_xml_get_value (xmlTextReaderPtr reader)
{
    int ret, type;
    xmlChar* value;

    /* check for null node */
    if ( xmlTextReaderIsEmptyElement (reader) ) {
        return NULL;
    }
    
    value = NULL;
    /* the next "node" is the text node containing the value we want to get */
    if (xmlTextReaderRead (reader) == 1) {
        value = xmlTextReaderValue (reader);
    }

    /* move on to the end of this node */
    while ( (xmlTextReaderRead(reader) == 1) && 
            (xmlTextReaderNodeType (reader) != XML_READER_TYPE_END_ELEMENT) ) {
    }
    
    return value;
}

/*****************************************************************************
 * Func:    gweather_xml_parse_name()
 * Desc:    Extract the name from the xml and add it to the tree
 * Parm:
 *      *locale         current locale
 *      *tree:          tree to view locations
 *      *iter:          iterator to named node
 *      reader:         xml text reader
 *      **untrans_name: returns untranslated name
 *      **trans_name:   returns translated name
 *      *pref:          the preference of the current translation, the smaller the better 
 */
static void gweather_xml_parse_name (const GList *locale, GtkTreeStore *tree, GtkTreeIter* iter, xmlTextReaderPtr reader,
				     xmlChar **untrans_name, xmlChar **trans_name, gint *pref)
{
    int ret, type;
    xmlChar *lang;
    GList   *found_locale;
    gint    position;
    
    /* First let's get the language */
    lang = xmlTextReaderXmlLang (reader);
    
    /* the next "node" is text node containing the actual name */
    ret = xmlTextReaderRead (reader);
    if (ret == 1) {
        
        /* Get the name, if this is the untranslated name, or the locale matches
         * the language
         */
        if ( lang == NULL ) {
            if (*untrans_name == NULL) {
	        *untrans_name = xmlTextReaderValue (reader);
	    }
            /* set the column entry, incase there is no translation */
	    gtk_tree_store_set (tree, iter, GWEATHER_PREF_COL_LOC, *untrans_name, -1);
        } else {
            found_locale = g_list_find_custom((GList*)locale, lang, gweather_xml_compare_locale);
            if (found_locale) {
                position = g_list_position((GList*)locale, found_locale);
                
                /* If this is higer up the preference list then use it */
                if ( (*pref == -1) || (*pref > position) ) {
		    if (*trans_name == NULL) {
		        *trans_name = xmlTextReaderValue (reader);
		    }
                    gtk_tree_store_set (tree, iter, GWEATHER_PREF_COL_LOC, *trans_name, -1);
                    *pref = position;
                }
            }
        }
    }

    xmlFree (lang);
    
    /* move on to the end of this node */
    while ( (xmlTextReaderRead(reader) == 1) && 
            (xmlTextReaderNodeType (reader) != XML_READER_TYPE_END_ELEMENT) ) {
    }
}


/*****************************************************************************
 * Func:    gweather_xml_parse_location()
 * Desc:    Parse a location node, creating a leaf for it in the tree
 * Parm:
 *      *locale         current locale
 *      *tree:          tree to view locations
 *      *loc:           currently selected location
 *      reader:         xml text reader
 *      *iter:          Changed to iterator for this node
 *      *r_iter:        parent node's iterator 
 *      *c_iter:        sibling node's iterator (who this node appears after)
 *      *dfltZone       default forecast zone for the enclosing element
 *      *dfltRadar      default radar map code
 *      *untransCity    if set, the untranslated name for encompassing city
 *      *transCity      if set, the translated name for encompassing city
 */
static void gweather_xml_parse_location (const GList* locale, GtkTreeView *tree, WeatherLocation *loc, xmlTextReaderPtr reader,
					 GtkTreeIter* iter, GtkTreeIter* r_iter, GtkTreeIter* c_iter,
					 xmlChar *dfltZone, xmlChar *dfltRadar, xmlChar *untransCity, xmlChar *transCity)
{
    int type;
    xmlChar *name, *untrans_name, *trans_name, *code, *zone, *radar, *coordinates;
    GtkTreeStore *store;
    WeatherLocation* new_loc;
    gint pref;
    
    /* check for an empty element */
    if ( xmlTextReaderIsEmptyElement (reader) ) {
        return;
    }
    
    /* initialise variables */
    store = GTK_TREE_STORE (gtk_tree_view_get_model (tree));
    untrans_name = NULL;
    trans_name = NULL;
    code = NULL;
    zone = dfltZone;
    radar = dfltRadar;
    coordinates = NULL;
    pref = -1;

    /* create a node in the tree for this region */
    gtk_tree_store_insert_after(store, iter, r_iter, c_iter);

    /* Loop through any sub elements (ie until we get to a close for this element */
    type = XML_READER_TYPE_ELEMENT;
    while ( (xmlTextReaderRead(reader) == 1) && (type != XML_READER_TYPE_END_ELEMENT) ) {
               
        type = xmlTextReaderNodeType (reader);
        
        /* skip non-element types */       
        if ( type == XML_READER_TYPE_ELEMENT ) {
            name = xmlTextReaderName (reader);
            
            if ( untrans_name == NULL && trans_name == NULL && strcmp (name, GWEATHER_XML_NODE_NAME) == 0 ) {
                gweather_xml_parse_name (locale, store, iter, reader, &untrans_name, &trans_name, &pref);
            } else if ( code == NULL && strcmp (name, GWEATHER_XML_NODE_CODE) == 0 ) {
                code = gweather_xml_get_value (reader);
            } else if ( strcmp (name, GWEATHER_XML_NODE_ZONE) == 0 ) {
	        zone = gweather_xml_get_value (reader);
            } else if ( strcmp (name, GWEATHER_XML_NODE_RADAR) == 0 ) {
                radar = gweather_xml_get_value (reader);
	    } else if ( strcmp (name, GWEATHER_XML_NODE_COORD) == 0 ) {
		coordinates = gweather_xml_get_value (reader);
	    } else {
	        xmlFree (gweather_xml_get_value (reader) );
	    }
            
            xmlFree (name);
        }
    }
    
    /* Add an entry in the tree for the location */
    new_loc = weather_location_new((untransCity ? untransCity : untrans_name), (transCity ? transCity : trans_name),
				   code, zone, radar, coordinates);
    gtk_tree_store_set (store, iter, GWEATHER_PREF_COL_POINTER, new_loc, -1);

    /* Free xml attributes */
    xmlFree(code);
    xmlFree(untrans_name);
    xmlFree(trans_name);
    if (zone != dfltZone)
        xmlFree(zone);
    if (radar != dfltRadar)
        xmlFree(radar);
    xmlFree(coordinates);

    /* If this location is actually the currently selected one, select it */
    if ( loc && weather_location_equal (new_loc, loc) ) {
        
        GtkTreePath *path;

        path = gtk_tree_model_get_path (GTK_TREE_MODEL (store), iter);
        gtk_tree_view_expand_to_path (tree, path);
        gtk_tree_view_set_cursor (tree, path, NULL, FALSE);
        gtk_tree_view_scroll_to_cell (tree, path, NULL, TRUE, 0.5, 0.5);
        gtk_tree_path_free (path);
    }
}


/*****************************************************************************
 * Func:    gweather_xml_parse_city()
 * Desc:    Parse a city node, creating a leaf for it in the tree
 *          then parsing any sub-nodes.
 *          This is called recursively as each node parses it's sub-nodes
 * Parm:
 *      *locale         current_locale
 *      *tree:          tree to view locations
 *      *loc:           currently selected location
 *      reader:         xml text reader
 *      *iter:          Changed to iterator for this node
 *      *p_iter:        iterator to parent node
 *      *s_iter:        iterator to last sibling node
 */
static void gweather_xml_parse_city (const GList *locale, GtkTreeView *tree, WeatherLocation *loc,
				     xmlTextReaderPtr reader, GtkTreeIter* iter, GtkTreeIter* p_iter, GtkTreeIter* s_iter)
{
    int type;
    xmlChar *name, *untrans_name, *trans_name;
    GtkTreeStore *model;
    GtkTreeIter last_child;
    gboolean first, isCity;
    gint pref;
    xmlChar *zone, *radar;
    
    /* check for an empty element */
    if ( xmlTextReaderIsEmptyElement (reader) ) {
        return;
    }
       
    /* initialise variables */
    model = GTK_TREE_STORE (gtk_tree_view_get_model (tree));
    first = TRUE;
    isCity = FALSE;
    pref = -1;
    untrans_name = NULL;
    trans_name = NULL;
    zone = NULL;
    radar = NULL;
    
    /* create a node in the tree for this region */
    gtk_tree_store_insert_after(model, iter, p_iter, s_iter);

    while ( xmlTextReaderRead(reader) == 1 ) {

        type = xmlTextReaderNodeType (reader);

        /* skip non-element types */
        if ( type == XML_READER_TYPE_ELEMENT ) {
            name = xmlTextReaderName (reader);

	    if ( untrans_name == NULL && trans_name == NULL && strcmp (name, GWEATHER_XML_NODE_NAME) == 0 ) {
	        gweather_xml_parse_name (locale, model, iter, reader,
					 &untrans_name, &trans_name, &pref);
	    } else if (zone == NULL && strcmp (name, GWEATHER_XML_NODE_ZONE) == 0) {
	        zone = gweather_xml_get_value (reader);
	    } else if (radar == NULL && strcmp (name, GWEATHER_XML_NODE_RADAR) == 0 ) {
	        radar = gweather_xml_get_value (reader);
            } else if ( strcmp (name, GWEATHER_XML_NODE_LOC) == 0 ) {
	        gweather_xml_parse_location (locale, tree, loc, reader, &last_child, iter,
					     (first ? NULL : &last_child), zone, radar,
					     untrans_name, trans_name);
		first = FALSE;
	    } else {
	        xmlFree (gweather_xml_get_value (reader) );
            }
            
            xmlFree (name);

        } else if ( type == XML_READER_TYPE_END_ELEMENT ) {
	    break;
	}
    }

    xmlFree (untrans_name);
    xmlFree (trans_name);
    xmlFree (zone);
    xmlFree (radar);
}

/*****************************************************************************
 * Func:    gweather_xml_parse_node()
 * Desc:    Parse a region/country/state/city node, creating a leaf for it in the tree
 *          then parsing any sub-nodes.
 *          This is called recursively as each node parses it's sub-nodes
 * Parm:
 *      *locale         current_locale
 *      *tree:          tree to view locations
 *      *loc:           currently selected location
 *      reader:         xml text reader
 *      *iter:          Changed to iterator for this node
 *      *p_iter:        iterator to parent node
 *      *s_iter:        iterator to last sibling node
 */

static void gweather_xml_parse_node (const GList *locale, GtkTreeView *tree, WeatherLocation *loc,
				     xmlTextReaderPtr reader, GtkTreeIter* iter, GtkTreeIter* p_iter, GtkTreeIter* s_iter)
{
    int type;
    xmlChar *name, *untrans_name, *trans_name;
    GtkTreeStore *model;
    GtkTreeIter last_child;
    gboolean first, isCity;
    gint pref;
    
    /* check for an empty element */
    if ( xmlTextReaderIsEmptyElement (reader) ) {
        return;
    }
       
    /* initialise variables */
    model = GTK_TREE_STORE (gtk_tree_view_get_model (tree));
    first = TRUE;
    isCity = FALSE;
    pref = -1;
    untrans_name = NULL;
    trans_name = NULL;
    
    /* create a node in the tree for this region */
    gtk_tree_store_insert_after(model, iter, p_iter, s_iter);

    while ( xmlTextReaderRead(reader) == 1 ) {

        type = xmlTextReaderNodeType (reader);

        /* skip non-element types */
        if ( type == XML_READER_TYPE_ELEMENT ) {
            name = xmlTextReaderName (reader);

	    if ( strcmp (name, GWEATHER_XML_NODE_NAME) == 0 ) {
	        gweather_xml_parse_name (locale, model, iter, reader,
					 &untrans_name, &trans_name, &pref);
	    } else if ( strcmp (name, GWEATHER_XML_NODE_CITY) == 0 ) {
		gweather_xml_parse_city (locale, tree, loc, reader, &last_child, iter,
					 (first ? NULL : &last_child));
		first = FALSE;
	    } else if ( strcmp (name, GWEATHER_XML_NODE_COUNTRY) == 0 || 
			strcmp (name, GWEATHER_XML_NODE_STATE)   == 0)  {
		gweather_xml_parse_node (locale, tree, loc, reader, &last_child, iter,
					 (first ? NULL : &last_child) );
		first = FALSE;
            } else if ( strcmp (name, GWEATHER_XML_NODE_LOC) == 0 ) {
	        gweather_xml_parse_location (locale, tree, loc, reader, &last_child, iter,
					     (first ? NULL : &last_child), NULL, NULL, NULL, NULL);
		first = FALSE;
	    } else {
	        xmlFree (gweather_xml_get_value (reader) );
            }
            
            xmlFree (name);

        } else if ( type == XML_READER_TYPE_END_ELEMENT ) {
	    break;
	}
    }
}

/*****************************************************************************
 * Func:    gweather_xml_parse()
 * Desc:    Top of the xml parser
 * Parm:
 *      *tree:          tree to view locations
 *      *loc:           currently selected location
 *      reader:         xml text reader
 *      locale:         current locale
 */
static void gweather_xml_parse (GtkTreeView *tree, WeatherLocation *loc, xmlTextReaderPtr reader, const GList* locale)
{
    int     ret, type;
    xmlChar *name;
    GtkTreeIter iter;
    gboolean first;
        
    /* at the start there are no regions in the tree */
    first = TRUE;
    
    /* We are expecting a region element anything else we simply skip */
    for (ret = xmlTextReaderRead(reader); ret == 1; ret = xmlTextReaderRead(reader) ) {
        type = xmlTextReaderNodeType (reader);
        if (type == XML_READER_TYPE_ELEMENT) {
            name = xmlTextReaderName (reader);
            if ( strcmp(name, GWEATHER_XML_NODE_REGION) == 0 ) {
	        gweather_xml_parse_node (locale, tree, loc, reader, &iter, NULL,
					 (first ? NULL : &iter));
		first = FALSE;
	    } else {
	        xmlFree (gweather_xml_get_value (reader) );
            }
            xmlFree (name);
        }
    }
}

/*****************************************************************************
 * Func:    gweather_xml_load_locations()
 * Desc:    Main entry point for loading the locations from the XML file
 * Parm:
 *      *tree:  tree to view locations
 *      *loc:   currently selected location
 */
void gweather_xml_load_locations (GtkTreeView *tree, WeatherLocation *loc)
{
    xmlTextReaderPtr reader;
    gchar *file;
    int ret;
    xmlChar *name, *format;
	const GList *locale;
    GtkTreeSortable *sortable;

    /* Get the current locale */
    locale = gnome_i18n_get_language_list("LC_MESSAGES");
    
    /* Open the xml file containing the different locations */
    file = gnome_datadir_file (GWEATHER_XML_LOCATION);
    g_return_if_fail (file);
    reader = xmlNewTextReaderFilename (file);
    g_return_if_fail (reader);
    
    /* The first node that is read is <gweather> */
    ret = xmlTextReaderRead (reader);
    if ( ret == 1 ) {
        /* check the name and format */
        name = xmlTextReaderName(reader);
        if ( strcmp(GWEATHER_XML_NODE_GWEATHER, name) == 0 ) {
            format = xmlTextReaderGetAttribute(reader, GWEATHER_XML_ATTR_FORMAT);
            if ( format != NULL ) {
                /* Parse depending on the format */
                if ( strcmp (format, GWEATHER_XML_FORMAT_1_0) == 0 ) {
                    gweather_xml_parse (tree, loc, reader, locale);
                }
                else {
                    g_warning ("Unknown gweather xml format %s", format);
                }
                xmlFree (format);
            }
        }
        xmlFree (name);
    }
    xmlFreeTextReader (reader);
    
    /* Sort the tree */
    sortable = GTK_TREE_SORTABLE (GTK_TREE_STORE(gtk_tree_view_get_model (tree)));
    gtk_tree_sortable_set_default_sort_func(sortable, &gweather_xml_location_sort_func, NULL, NULL);
    gtk_tree_sortable_set_sort_column_id(sortable,GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID,GTK_SORT_ASCENDING);
}
