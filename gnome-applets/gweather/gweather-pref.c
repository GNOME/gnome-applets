/*
 * Copyright (C) Papadimitriou Spiros <spapadim+@cs.cmu.edu>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <locale.h>

#include <libgweather/gweather.h>

#include "gweather-xml.h"
#include "gweather-pref.h"
#include "gweather-applet.h"
#include "gweather-dialog.h"

struct _GWeatherPref
{
	GtkDialog  parent;

	GtkWidget *basic_temp_combo;
	GtkWidget *basic_speed_combo;
	GtkWidget *basic_dist_combo;
	GtkWidget *basic_pres_combo;
	GtkWidget *find_entry;
	GtkWidget *find_next_btn;

	GtkWidget *basic_update_spin;
	GtkWidget *basic_update_btn;
	GtkWidget *tree;

	GtkTreeModel *model;

	GWeatherApplet *applet;
};

enum
{
	PROP_0,
	PROP_GWEATHER_APPLET,
};

G_DEFINE_TYPE (GWeatherPref, gweather_pref, GTK_TYPE_DIALOG)

/* sets up ATK Relation between the widgets */

static void
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

static void
gweather_pref_set_accessibility (GWeatherPref *pref)
{
    /* Relation between components in General page */

    add_atk_relation (pref->basic_update_btn, pref->basic_update_spin, ATK_RELATION_CONTROLLER_FOR);

    add_atk_relation (pref->basic_update_spin, pref->basic_update_btn, ATK_RELATION_CONTROLLED_BY);

    /* Accessible Name and Description for the components in Preference Dialog */

    set_access_namedesc (pref->tree, _("Location view"), _("Select Location from the list"));
    set_access_namedesc (pref->basic_update_spin, _("Update spin button"), _("Spinbutton for updating"));
}

static gboolean
bind_update_interval_get (GValue   *value,
			  GVariant *variant,
			  gpointer  user_data)
{
  g_value_set_double (value, g_variant_get_int32 (variant) / 60);
  return TRUE;
}

static GVariant*
bind_update_interval_set (const GValue       *value,
			  const GVariantType *variant,
			  gpointer            user_data)
{
  return g_variant_new_int32 (g_value_get_double (value) * 60);
}

static gboolean
string_to_enum (GValue *value,
                GVariant *variant,
                gpointer user_data)
{
	GType (* get_type) (void) = user_data;
	GEnumClass *klass;
	GEnumValue *eval = NULL;
	const char *s;
	guint i;

	g_variant_get (variant, "&s", &s);

	klass = g_type_class_ref (get_type ());
	for (i = 0; i < klass->n_values; ++i) {
		if (strcmp (klass->values[i].value_nick, s) != 0)
			continue;

		eval = &klass->values[i];
		break;
	}

	if (eval)
		g_value_set_int (value, eval->value - 1);

	g_type_class_unref (klass);

	return eval != NULL;
}

static GVariant *
enum_to_string (const GValue *value,
                const GVariantType *expected_type,
                gpointer user_data)
{
	GType (* get_type) (void) = user_data;
	GEnumClass *klass;
	GEnumValue *eval = NULL;
	int val;
	guint i;
	GVariant *variant = NULL;

	val = g_value_get_int (value) + 1;

	klass = g_type_class_ref (get_type ());
	for (i = 0; i < klass->n_values; ++i) {
		if (klass->values[i].value != val)
			continue;

		eval = &klass->values[i];
		break;
	}

	if (eval)
		variant = g_variant_new_string (eval->value_nick);

	g_type_class_unref (klass);

	return variant;
}

/* Update pref dialog from gweather_pref */
static gboolean update_dialog (GWeatherPref *pref)
{
    GWeatherApplet *gw_applet = pref->applet;

    g_settings_bind_with_mapping (gw_applet->applet_settings, "auto-update-interval",
				  pref->basic_update_spin, "value",
				  G_SETTINGS_BIND_DEFAULT | G_SETTINGS_BIND_NO_SENSITIVITY,
				  bind_update_interval_get,
				  bind_update_interval_set,
				  NULL, NULL);

    g_settings_bind (gw_applet->applet_settings, "auto-update",
		     pref->basic_update_btn, "active",
		     G_SETTINGS_BIND_DEFAULT);

    gtk_widget_set_sensitive(pref->basic_update_spin,
			      g_settings_get_boolean (gw_applet->applet_settings, "auto-update"));

    g_settings_bind_with_mapping (gw_applet->lib_settings, "temperature-unit",
		     pref->basic_temp_combo, "active",
		     G_SETTINGS_BIND_DEFAULT,
		     string_to_enum, enum_to_string,
		     gweather_temperature_unit_get_type, NULL);

    g_settings_bind_with_mapping (gw_applet->lib_settings, "speed-unit",
		     pref->basic_speed_combo, "active",
		     G_SETTINGS_BIND_DEFAULT,
		     string_to_enum, enum_to_string,
		     gweather_speed_unit_get_type, NULL);

    g_settings_bind_with_mapping (gw_applet->lib_settings, "pressure-unit",
		     pref->basic_pres_combo, "active",
		     G_SETTINGS_BIND_DEFAULT,
		     string_to_enum, enum_to_string,
		     gweather_pressure_unit_get_type, NULL);

    g_settings_bind_with_mapping (gw_applet->lib_settings, "distance-unit",
		     pref->basic_dist_combo, "active",
		     G_SETTINGS_BIND_DEFAULT,
		     string_to_enum, enum_to_string,
		     gweather_distance_unit_get_type, NULL);

    return TRUE;
}

static void row_selected_cb (GtkTreeSelection *selection, GWeatherPref *pref)
{
    GWeatherApplet *gw_applet = pref->applet;
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *name, *code;
    gboolean has_latlon;
    gdouble latitude, longitude;
    
    if (!gtk_tree_selection_get_selected (selection, &model, &iter))
    	return;
    	
    gtk_tree_model_get (model, &iter, GWEATHER_XML_COL_METAR_CODE, &code, -1);
    
    if (!code)
    	return;

    gtk_tree_model_get (model, &iter,
			GWEATHER_XML_COL_LOCATION_NAME, &name,
			GWEATHER_XML_COL_LATLON_VALID, &has_latlon,
			GWEATHER_XML_COL_LATITUDE, &latitude,
			GWEATHER_XML_COL_LONGITUDE, &longitude,
			-1);

    g_settings_set (gw_applet->lib_settings, "default-location", "(ssm(dd))",
		    name, code, has_latlon, latitude, longitude);

    g_free (name);
    g_free (code);
    
    gweather_update (gw_applet);
} 

static gboolean
compare_location (GtkTreeModel *model,
                  GtkTreePath  *path,
                  GtkTreeIter  *iter,
                  gpointer      user_data)
{
    GWeatherPref *pref = user_data;
    GtkTreeView *view;
    gchar *name = NULL;
    gchar *default_loc = NULL;
    gboolean retval = TRUE;

    gtk_tree_model_get (model, iter, GWEATHER_XML_COL_LOCATION_NAME, &name, -1);
    if (!name)
	retval = FALSE;

    g_settings_get (pref->applet->lib_settings, "default-location", "(ssm(dd))",
		    &default_loc, NULL, NULL, NULL, NULL);

    if (g_strcmp0(name, default_loc))
	retval = FALSE;

    if (retval) {
      view = GTK_TREE_VIEW (pref->tree);
      gtk_tree_view_expand_to_path (view, path);
      gtk_tree_view_set_cursor (view, path, NULL, FALSE);
      gtk_tree_view_scroll_to_cell (view, path, NULL, TRUE, 0.5, 0.5);
    }

    g_free(name);
    g_free(default_loc);
    return retval;
}

static void load_locations (GWeatherPref *pref)
{
    GtkTreeView *tree = GTK_TREE_VIEW (pref->tree);
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell_renderer;
        
    /* Add a colum for the locations */
    cell_renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes ("not used", cell_renderer,
						       "text", GWEATHER_XML_COL_LOCATION_NAME, NULL);
    gtk_tree_view_append_column (tree, column);
    gtk_tree_view_set_expander_column (GTK_TREE_VIEW (tree), column);
    
    /* load locations from xml file */
    pref->model = gweather_xml_load_locations ();
    if (!pref->model)
    {
        GtkWidget *d;

        d = gtk_message_dialog_new (NULL, 0, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                    _("Failed to load the Locations XML "
                                      "database.  Please report this as "
                                      "a bug."));
        gtk_dialog_run (GTK_DIALOG (d));
	gtk_widget_destroy (d);
    }
    gtk_tree_view_set_model (tree, pref->model);

    /* Select the current (default) location */
    gtk_tree_model_foreach (GTK_TREE_MODEL (pref->model), compare_location, pref);
}

static void
auto_update_toggled (GtkToggleButton *button, GWeatherPref *pref)
{
    GWeatherApplet *gw_applet = pref->applet;
    gboolean toggled;
    gint nxtSunEvent;

    toggled = gtk_toggle_button_get_active(button);
    gtk_widget_set_sensitive (pref->basic_update_spin, toggled);
    if (gw_applet->timeout_tag > 0)
        g_source_remove(gw_applet->timeout_tag);
    if (gw_applet->suncalc_timeout_tag > 0)
        g_source_remove(gw_applet->suncalc_timeout_tag);
    if (toggled) {
      gw_applet->timeout_tag = g_timeout_add_seconds (g_settings_get_int (gw_applet->applet_settings, "auto-update-interval"),
						      timeout_cb, gw_applet);
	nxtSunEvent = gweather_info_next_sun_event(gw_applet->gweather_info);
	if (nxtSunEvent >= 0)
	    gw_applet->suncalc_timeout_tag = g_timeout_add_seconds (
	    						nxtSunEvent,
							suncalc_timeout_cb,
							gw_applet);
    }
}

static void temp_combo_changed_cb (GtkComboBox *combo, GWeatherPref *pref)
{
    GWeatherApplet *gw_applet = pref->applet;
    gchar *temp_summary;

    temp_summary = gweather_info_get_temp_summary (gw_applet->gweather_info);
    gtk_label_set_text (GTK_LABEL (gw_applet->label), temp_summary);
    g_free (temp_summary);

    if (gw_applet->details_dialog)
        gweather_dialog_update (GWEATHER_DIALOG (gw_applet->details_dialog));
}

static void speed_combo_changed_cb (GtkComboBox *combo, GWeatherPref *pref)
{
    GWeatherApplet *gw_applet = pref->applet;

    if (gw_applet->details_dialog)
        gweather_dialog_update (GWEATHER_DIALOG (gw_applet->details_dialog));
}

static void pres_combo_changed_cb (GtkComboBox *combo, GWeatherPref *pref)
{
    GWeatherApplet *gw_applet = pref->applet;

    if (gw_applet->details_dialog)
        gweather_dialog_update (GWEATHER_DIALOG (gw_applet->details_dialog));
}

static void dist_combo_changed_cb (GtkComboBox *combo, GWeatherPref *pref)
{
    GWeatherApplet *gw_applet = pref->applet;

    if (gw_applet->details_dialog)
        gweather_dialog_update (GWEATHER_DIALOG (gw_applet->details_dialog));
}

static void
update_interval_changed (GtkSpinButton *button, GWeatherPref *pref)
{
    GWeatherApplet *gw_applet = pref->applet;

    if (gw_applet->timeout_tag > 0)
        g_source_remove(gw_applet->timeout_tag);
    if (g_settings_get_boolean (gw_applet->applet_settings, "auto-update"))
        gw_applet->timeout_tag =  
	  g_timeout_add_seconds (g_settings_get_int (gw_applet->applet_settings, "auto-update-interval"),
                                 timeout_cb, gw_applet);
}

static GtkWidget *
create_hig_catagory (GtkWidget *main_box, gchar *title)
{
	GtkWidget *vbox, *vbox2, *hbox;
	GtkWidget *label;
	gchar *tmp;

	vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
	gtk_box_pack_start (GTK_BOX (main_box), vbox, FALSE, FALSE, 0);

	tmp = g_strdup_printf ("<b>%s</b>", title);
	label = gtk_label_new (NULL);
	gtk_label_set_xalign (GTK_LABEL (label), 0.0);
	gtk_label_set_markup (GTK_LABEL (label), tmp);
	g_free (tmp);
	gtk_box_pack_start (GTK_BOX (vbox), label, TRUE, FALSE, 0);
	
	hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
	
	label = gtk_label_new ("    ");
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	
	vbox2 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
	gtk_box_pack_start (GTK_BOX (hbox), vbox2, TRUE, TRUE, 0);

	return vbox2;		
}

static gboolean
find_location (GtkTreeModel *model, GtkTreeIter *iter, const gchar *location, gboolean go_parent)
{
	GtkTreeIter iter_child;
	GtkTreeIter iter_parent;
	gchar *aux_loc;
	gboolean valid;
	int len;

	len = strlen (location);

	if (len <= 0) {
		return FALSE;
	}
	
	do {
		
	  gtk_tree_model_get (model, iter, GWEATHER_XML_COL_LOCATION_NAME, &aux_loc, -1);

		if (g_ascii_strncasecmp (aux_loc, location, len) == 0) {
			g_free (aux_loc);
			return TRUE;
		}

		if (gtk_tree_model_iter_has_child (model, iter)) {
			gtk_tree_model_iter_nth_child (model, &iter_child, iter, 0);

			if (find_location (model, &iter_child, location, FALSE)) {
				/* Manual copying of the iter */
				iter->stamp = iter_child.stamp;
				iter->user_data = iter_child.user_data;
				iter->user_data2 = iter_child.user_data2;
				iter->user_data3 = iter_child.user_data3;

				g_free (aux_loc);
				
				return TRUE;
			}
		}
		g_free (aux_loc);

		valid = gtk_tree_model_iter_next (model, iter);		
	} while (valid);

	if (go_parent) {
		iter_parent = *iter;
                while (gtk_tree_model_iter_parent (model, iter, &iter_parent)) {
			if (gtk_tree_model_iter_next (model, iter))
				return find_location (model, iter, location, TRUE);
			iter_parent = *iter;
		}
	}

	return FALSE;
}

static void
find_next_clicked (GtkButton *button, GWeatherPref *pref)
{
	GtkTreeView *tree;
	GtkTreeModel *model;
	GtkEntry *entry;
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	GtkTreeIter iter_parent;
	GtkTreePath *path;
	const gchar *location;

	tree = GTK_TREE_VIEW (pref->tree);
	model = gtk_tree_view_get_model (tree);
	entry = GTK_ENTRY (pref->find_entry);

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree));

	if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
		/* Select next or select parent */
		if (!gtk_tree_model_iter_next (model, &iter)) {
			iter_parent = iter;
			if (!gtk_tree_model_iter_parent (model, &iter, &iter_parent) || !gtk_tree_model_iter_next (model, &iter))
				gtk_tree_model_get_iter_first (model, &iter);
		}
	} else {
		gtk_tree_model_get_iter_first (model, &iter);
	}

	location = gtk_entry_get_text (entry);

	if (find_location (model, &iter, location, TRUE)) {
		gtk_widget_set_sensitive (pref->find_next_btn, TRUE);

		path = gtk_tree_model_get_path (model, &iter);
		gtk_tree_view_expand_to_path (tree, path);
		gtk_tree_selection_select_path (selection, path);
		gtk_tree_view_scroll_to_cell (tree, path, NULL, TRUE, 0.5, 0);

		gtk_tree_path_free (path);
	} else {
		gtk_widget_set_sensitive (pref->find_next_btn, FALSE);
	}
}

static void
find_entry_changed (GtkEditable *entry, GWeatherPref *pref)
{
	GtkTreeView *tree;
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	GtkTreePath *path;
	const gchar *location;

	tree = GTK_TREE_VIEW (pref->tree);
	model = gtk_tree_view_get_model (tree);

	g_return_if_fail (model != NULL);

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree));
	gtk_tree_model_get_iter_first (model, &iter);

	location = gtk_entry_get_text (GTK_ENTRY (entry));

	if (find_location (model, &iter, location, TRUE)) {
		gtk_widget_set_sensitive (pref->find_next_btn, TRUE);

		path = gtk_tree_model_get_path (model, &iter);
		gtk_tree_view_expand_to_path (tree, path);
		gtk_tree_selection_select_iter (selection, &iter);
		gtk_tree_view_scroll_to_cell (tree, path, NULL, TRUE, 0.5, 0);

		gtk_tree_path_free (path);
	} else {
		gtk_widget_set_sensitive (pref->find_next_btn, FALSE);
	}
}

static void
response_cb (GtkDialog *dialog, gint id, GWeatherPref *pref)
{
    if (id == GTK_RESPONSE_HELP) {
        gp_applet_show_help (GP_APPLET (pref->applet), "gweather-settings");
    } else {
        gtk_widget_destroy (GTK_WIDGET (dialog));
    }
}

static void
gweather_pref_create (GWeatherPref *pref)
{
    GtkWidget *pref_vbox;
    GtkWidget *pref_notebook;
    GtkWidget *pref_basic_update_alignment;
    GtkWidget *pref_basic_update_lbl;
    GtkWidget *pref_basic_update_hbox;
    GtkAdjustment *pref_basic_update_spin_adj;
    GtkWidget *pref_basic_update_sec_lbl;
    GtkWidget *pref_basic_note_lbl;
    GtkWidget *pref_loc_hbox;
    GtkWidget *pref_loc_note_lbl;
    GtkWidget *scrolled_window;
    GtkWidget *value_hbox, *tree_label;
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
    GtkWidget *unit_grid;
    GtkWidget *pref_find_label;
    GtkWidget *pref_find_hbox;
    GtkWidget *image;

    g_object_set (pref, "destroy-with-parent", TRUE, NULL);
    gtk_window_set_title (GTK_WINDOW (pref), _("Weather Preferences"));
    gtk_dialog_add_buttons (GTK_DIALOG (pref),
			    GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
			    GTK_STOCK_HELP, GTK_RESPONSE_HELP,
			    NULL);
    gtk_dialog_set_default_response (GTK_DIALOG (pref), GTK_RESPONSE_CLOSE);
    gtk_container_set_border_width (GTK_CONTAINER (pref), 5);
    gtk_window_set_resizable (GTK_WINDOW (pref), TRUE);
    gtk_window_set_screen (GTK_WINDOW (pref),
			   gtk_widget_get_screen (GTK_WIDGET (pref->applet)));

    pref_vbox = gtk_dialog_get_content_area (GTK_DIALOG (pref));
    gtk_box_set_spacing (GTK_BOX (pref_vbox), 2);
    gtk_widget_show (pref_vbox);

    pref_notebook = gtk_notebook_new ();
    gtk_container_set_border_width (GTK_CONTAINER (pref_notebook), 5);
    gtk_widget_show (pref_notebook);
    gtk_box_pack_start (GTK_BOX (pref_vbox), pref_notebook, TRUE, TRUE, 0);

  /*
   * General settings page.
   */

    pref_basic_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 18);
    gtk_container_set_border_width (GTK_CONTAINER (pref_basic_vbox), 12);
    gtk_container_add (GTK_CONTAINER (pref_notebook), pref_basic_vbox);

    pref_basic_update_alignment = gtk_alignment_new (0, 0.5, 0, 1);
    gtk_widget_show (pref_basic_update_alignment);

    pref->basic_update_btn = gtk_check_button_new_with_mnemonic (_("_Automatically update every:"));
    gtk_widget_show (pref->basic_update_btn);
    gtk_container_add (GTK_CONTAINER (pref_basic_update_alignment), pref->basic_update_btn);
    g_signal_connect (G_OBJECT (pref->basic_update_btn), "toggled",
    		      G_CALLBACK (auto_update_toggled), pref);

  /*
   * Units settings page.
   */

  /* Temperature Unit */
  temp_label = gtk_label_new_with_mnemonic (_("_Temperature unit:"));
  gtk_label_set_use_markup (GTK_LABEL (temp_label), TRUE);
  gtk_label_set_justify (GTK_LABEL (temp_label), GTK_JUSTIFY_LEFT);
  gtk_label_set_xalign (GTK_LABEL (temp_label), 0.0);
  gtk_widget_show (temp_label);

  temp_combo = gtk_combo_box_text_new ();
  pref->basic_temp_combo = temp_combo;
  gtk_label_set_mnemonic_widget (GTK_LABEL (temp_label), temp_combo);
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (temp_combo), _("Default"));
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (temp_combo), _("Kelvin"));
  /* TRANSLATORS: Celsius is sometimes referred Centigrade */
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (temp_combo), _("Celsius"));
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (temp_combo), _("Fahrenheit"));
  gtk_widget_set_hexpand (temp_combo, TRUE);
  gtk_widget_show (temp_combo);

  /* Speed Unit */
  speed_label = gtk_label_new_with_mnemonic (_("_Wind speed unit:"));
  gtk_label_set_use_markup (GTK_LABEL (speed_label), TRUE);
  gtk_label_set_justify (GTK_LABEL (speed_label), GTK_JUSTIFY_LEFT);
  gtk_label_set_xalign (GTK_LABEL (speed_label), 0.0);
  gtk_widget_show (speed_label);

  speed_combo = gtk_combo_box_text_new ();
  pref->basic_speed_combo = speed_combo;
  gtk_label_set_mnemonic_widget (GTK_LABEL (speed_label), speed_combo);
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (speed_combo), _("Default"));
  /* TRANSLATOR: The wind speed unit "meters per second" */
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (speed_combo), _("m/s"));
  /* TRANSLATOR: The wind speed unit "kilometers per hour" */
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (speed_combo), _("km/h"));
  /* TRANSLATOR: The wind speed unit "miles per hour" */
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (speed_combo), _("mph"));
  /* TRANSLATOR: The wind speed unit "knots" */
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (speed_combo), _("knots"));
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (speed_combo), _("Beaufort scale"));
  gtk_widget_set_hexpand (speed_combo, TRUE);
  gtk_widget_show (speed_combo);

  /* Pressure Unit */
  pres_label = gtk_label_new_with_mnemonic (_("_Pressure unit:"));
  gtk_label_set_use_markup (GTK_LABEL (pres_label), TRUE);
  gtk_label_set_justify (GTK_LABEL (pres_label), GTK_JUSTIFY_LEFT);
  gtk_label_set_xalign (GTK_LABEL (pres_label), 0.0);
  gtk_widget_show (pres_label);

  pres_combo = gtk_combo_box_text_new ();
  pref->basic_pres_combo = pres_combo;
  gtk_label_set_mnemonic_widget (GTK_LABEL (pres_label), pres_combo);
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (pres_combo), _("Default"));
  /* TRANSLATOR: The pressure unit "kiloPascals" */
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (pres_combo), _("kPa"));
  /* TRANSLATOR: The pressure unit "hectoPascals" */
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (pres_combo), _("hPa"));
  /* TRANSLATOR: The pressure unit "millibars" */
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (pres_combo), _("mb"));
  /* TRANSLATOR: The pressure unit "millibars of mercury" */
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (pres_combo), _("mmHg"));
  /* TRANSLATOR: The pressure unit "inches of mercury" */
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (pres_combo), _("inHg"));
  /* TRANSLATOR: The pressure unit "atmospheres" */
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (pres_combo), _("atm"));
  gtk_widget_set_hexpand (pres_combo, TRUE);
  gtk_widget_show (pres_combo);

  /* Distance Unit */
  dist_label = gtk_label_new_with_mnemonic (_("_Visibility unit:"));
  gtk_label_set_use_markup (GTK_LABEL (dist_label), TRUE);
  gtk_label_set_justify (GTK_LABEL (dist_label), GTK_JUSTIFY_LEFT);
  gtk_label_set_xalign (GTK_LABEL (dist_label), 0.0);
  gtk_widget_show (dist_label);

  dist_combo = gtk_combo_box_text_new ();
  pref->basic_dist_combo = dist_combo;
  gtk_label_set_mnemonic_widget (GTK_LABEL (dist_label), dist_combo);
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (dist_combo), _("Default"));
  /* TRANSLATOR: The distance unit "meters" */
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (dist_combo), _("meters"));
  /* TRANSLATOR: The distance unit "kilometers" */
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (dist_combo), _("km"));
  /* TRANSLATOR: The distance unit "miles" */
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (dist_combo), _("miles"));
  gtk_widget_set_hexpand (dist_combo, TRUE);
  gtk_widget_show (dist_combo);

  unit_grid = gtk_grid_new ();
  gtk_grid_set_row_spacing (GTK_GRID (unit_grid), 6);
  gtk_grid_set_column_spacing (GTK_GRID (unit_grid), 12);
  gtk_grid_attach (GTK_GRID (unit_grid), temp_label, 0, 0, 1, 1);
  gtk_grid_attach (GTK_GRID (unit_grid), temp_combo,  1, 0, 1, 1);
  gtk_grid_attach (GTK_GRID (unit_grid), speed_label, 0, 1, 1, 1);
  gtk_grid_attach (GTK_GRID (unit_grid), speed_combo, 1, 1, 1, 1);
  gtk_grid_attach (GTK_GRID (unit_grid), pres_label, 0, 2, 1, 1);
  gtk_grid_attach (GTK_GRID (unit_grid), pres_combo,  1, 2, 1, 1);
  gtk_grid_attach (GTK_GRID (unit_grid), dist_label, 0, 3, 1, 1);
  gtk_grid_attach (GTK_GRID (unit_grid), dist_combo,  1, 3, 1, 1);
  gtk_widget_show (unit_grid);

	g_signal_connect (temp_combo, "changed", G_CALLBACK (temp_combo_changed_cb), pref);
	g_signal_connect (speed_combo, "changed", G_CALLBACK (speed_combo_changed_cb), pref);
	g_signal_connect (dist_combo, "changed", G_CALLBACK (dist_combo_changed_cb), pref);
	g_signal_connect (pres_combo, "changed", G_CALLBACK (pres_combo_changed_cb), pref);

    frame = create_hig_catagory (pref_basic_vbox, _("Update"));

    pref_basic_update_hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);

    pref_basic_update_lbl = gtk_label_new_with_mnemonic (_("_Automatically update every:"));
    gtk_widget_show (pref_basic_update_lbl);

/*
    gtk_label_set_justify (GTK_LABEL (pref_basic_update_lbl), GTK_JUSTIFY_RIGHT);
    gtk_label_set_xalign (GTK_LABEL (prefs_basic_update_lbl), 0.0);
*/

    gtk_widget_show (pref_basic_update_hbox);

    pref_basic_update_spin_adj = gtk_adjustment_new (30, 1, 3600, 5, 25, 1);
    pref->basic_update_spin = gtk_spin_button_new (GTK_ADJUSTMENT (pref_basic_update_spin_adj), 1, 0);
    gtk_widget_show (pref->basic_update_spin);

    gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (pref->basic_update_spin), TRUE);
    gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (pref->basic_update_spin), GTK_UPDATE_IF_VALID);
    g_signal_connect (G_OBJECT (pref->basic_update_spin), "value_changed",
    		      G_CALLBACK (update_interval_changed), pref);
    
    pref_basic_update_sec_lbl = gtk_label_new (_("minutes"));
    gtk_widget_show (pref_basic_update_sec_lbl);

    value_hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);

    gtk_box_pack_start (GTK_BOX (pref_basic_update_hbox), pref_basic_update_alignment, FALSE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (pref_basic_update_hbox), value_hbox, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (value_hbox), pref->basic_update_spin, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (value_hbox), pref_basic_update_sec_lbl, FALSE, FALSE, 0);

    gtk_container_add (GTK_CONTAINER (frame), pref_basic_update_hbox);

    frame = create_hig_catagory (pref_basic_vbox, _("Display"));

    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);

  gtk_box_pack_start (GTK_BOX (vbox), unit_grid, TRUE, TRUE, 0);

    gtk_container_add (GTK_CONTAINER (frame), vbox);

    pref_basic_note_lbl = gtk_label_new (_("General"));
    gtk_widget_show (pref_basic_note_lbl);
    gtk_notebook_set_tab_label (GTK_NOTEBOOK (pref_notebook), 
    				gtk_notebook_get_nth_page (GTK_NOTEBOOK (pref_notebook), 0),
    				pref_basic_note_lbl);

  /*
   * Location page.
   */
    pref_loc_hbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
    gtk_container_set_border_width (GTK_CONTAINER (pref_loc_hbox), 12);
    gtk_container_add (GTK_CONTAINER (pref_notebook), pref_loc_hbox);

    tree_label = gtk_label_new_with_mnemonic (_("_Select a location:"));
    gtk_label_set_xalign (GTK_LABEL (tree_label), 0.0);
    gtk_box_pack_start (GTK_BOX (pref_loc_hbox), tree_label, FALSE, FALSE, 0);

    scrolled_window = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window), GTK_SHADOW_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
				    GTK_POLICY_AUTOMATIC,
				    GTK_POLICY_AUTOMATIC);

    pref->tree = gtk_tree_view_new ();
    gtk_label_set_mnemonic_widget (GTK_LABEL (tree_label), GTK_WIDGET (pref->tree));
    gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (pref->tree), FALSE);
    
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (pref->tree));
    g_signal_connect (G_OBJECT (selection), "changed",
    		      G_CALLBACK (row_selected_cb), pref);

    gtk_container_add (GTK_CONTAINER (scrolled_window), pref->tree);
    gtk_widget_show (pref->tree);
    gtk_widget_show (scrolled_window);
    gtk_box_pack_start (GTK_BOX (pref_loc_hbox), scrolled_window, TRUE, TRUE, 0);
    load_locations(pref);

    pref_find_hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
    pref_find_label = gtk_label_new (_("_Find:"));
    gtk_label_set_use_underline (GTK_LABEL (pref_find_label), TRUE);

    pref->find_entry = gtk_entry_new ();
    gtk_label_set_mnemonic_widget (GTK_LABEL (pref_find_label),
		    pref->find_entry);
    
    pref->find_next_btn = gtk_button_new_with_mnemonic (_("Find _Next"));
    gtk_widget_set_sensitive (pref->find_next_btn, FALSE);
    
    image = gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_BUTTON); 
    gtk_button_set_image (GTK_BUTTON (pref->find_next_btn), image);

    g_signal_connect (G_OBJECT (pref->find_next_btn), "clicked",
		      G_CALLBACK (find_next_clicked), pref);
    g_signal_connect (G_OBJECT (pref->find_entry), "changed",
		      G_CALLBACK (find_entry_changed), pref);

    gtk_container_set_border_width (GTK_CONTAINER (pref_find_hbox), 0);
    gtk_box_pack_start (GTK_BOX (pref_find_hbox), pref_find_label, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (pref_find_hbox), pref->find_entry, TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (pref_find_hbox), pref->find_next_btn, FALSE, FALSE, 0);

    gtk_box_pack_start (GTK_BOX (pref_loc_hbox), pref_find_hbox, FALSE, FALSE, 0);

    pref_loc_note_lbl = gtk_label_new (_("Location"));
    gtk_widget_show (pref_loc_note_lbl);
    gtk_notebook_set_tab_label (GTK_NOTEBOOK (pref_notebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (pref_notebook), 1), pref_loc_note_lbl);

    g_signal_connect (G_OBJECT (pref), "response",
    		      G_CALLBACK (response_cb), pref);

    gweather_pref_set_accessibility (pref); 
    gtk_label_set_mnemonic_widget (GTK_LABEL (pref_basic_update_sec_lbl), pref->basic_update_spin);
}

static void
gweather_pref_set_property (GObject *object,
                            guint prop_id,
                            const GValue *value,
                            GParamSpec *pspec)
{
    GWeatherPref *pref = GWEATHER_PREF (object);

    switch (prop_id) {
	case PROP_GWEATHER_APPLET:
	    pref->applet = g_value_get_pointer (value);
	    break;

	default:
	    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	    break;
    }
}

static void
gweather_pref_get_property (GObject *object,
                            guint prop_id,
                            GValue *value,
                            GParamSpec *pspec)
{
    GWeatherPref *pref = GWEATHER_PREF (object);

    switch (prop_id) {
	case PROP_GWEATHER_APPLET:
	    g_value_set_pointer (value, pref->applet);
	    break;

	default:
	    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	    break;
    }
}

static void
gweather_pref_init (GWeatherPref *self)
{
}

static GObject *
gweather_pref_constructor (GType type,
			   guint n_construct_params,
			   GObjectConstructParam *construct_params)
{
    GObject *object;
    GWeatherPref *self;

    object = G_OBJECT_CLASS (gweather_pref_parent_class)->
        constructor (type, n_construct_params, construct_params);
    self = GWEATHER_PREF (object);

    gweather_pref_create (self);
    update_dialog (self);

    return object;
}

GtkWidget *
gweather_pref_new (GWeatherApplet *applet)
{
    return g_object_new (GWEATHER_TYPE_PREF,
			 "gweather-applet", applet,
			 NULL);
}

static void
gweather_pref_finalize (GObject *object)
{
   GWeatherPref *self = GWEATHER_PREF (object);

   g_object_unref (G_OBJECT (self->model));

   G_OBJECT_CLASS (gweather_pref_parent_class)->finalize(object);
}

static void
gweather_pref_class_init (GWeatherPrefClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    gweather_pref_parent_class = g_type_class_peek_parent (klass);

    object_class->set_property = gweather_pref_set_property;
    object_class->get_property = gweather_pref_get_property;
    object_class->constructor = gweather_pref_constructor;
    object_class->finalize = gweather_pref_finalize;

    /* This becomes an OBJECT property when GWeatherApplet is redone */
    g_object_class_install_property (object_class,
				     PROP_GWEATHER_APPLET,
				     g_param_spec_pointer ("gweather-applet",
							   "GWeather Applet",
							   "The GWeather Applet",
							   G_PARAM_READWRITE |
							   G_PARAM_CONSTRUCT_ONLY));
}
