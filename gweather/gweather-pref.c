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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>

#include <gnome.h>
#include <panel-applet.h>
#include <gconf/gconf-client.h>
#include <egg-screen-help.h>

#include "gweather.h"
#include "gweather-pref.h"
#include "gweather-applet.h"
#include "location.h"


static void gweather_pref_set_accessibility (GWeatherApplet *gw_applet);
static void help_cb (GtkDialog *dialog);


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

    add_atk_relation (gw_applet->pref_basic_update_spin, gw_applet->pref_basic_update_btn,                                             ATK_RELATION_CONTROLLED_BY);

    /* Accessible Name and Description for the components in Preference Dialog */
   
    set_access_namedesc (gw_applet->pref_basic_update_spin, _("Update spin button"),                                      _("Spinbutton for updating"));

}

static void change_cb (GtkButton *button, gpointer user_data)
{
    GWeatherApplet *gw_applet = user_data;

    gtk_widget_set_sensitive(gw_applet->pref_basic_update_spin, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gw_applet->pref_basic_update_btn)));
    
    return;
}

/* FIXME: does this still work */
static void
auto_update_toggled (GtkToggleButton *button, gpointer data)
{
    GWeatherApplet *gw_applet = data;
    gboolean toggled;
    
    toggled = gtk_toggle_button_get_active(button);
    gw_applet->gweather_pref.update_enabled = toggled;
    gtk_widget_set_sensitive (gw_applet->pref_basic_update_spin, toggled);
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

    gweather_update (gw_applet);
 
}

static void
show_labels_toggled (GtkToggleButton *button, gpointer data)
{
    GWeatherApplet *gw_applet = data;
    gboolean toggled;
    gint i;
    
    toggled = gtk_toggle_button_get_active(button);
    if (gw_applet->gweather_pref.show_labels == toggled)
        return;
        
    gw_applet->gweather_pref.show_labels= toggled;
    panel_applet_gconf_set_bool(gw_applet->applet, "show_labels", toggled, NULL);

   for (i=0; i<=gw_applet->gweather_info->numforecasts; i++) {
	if (toggled)
		gtk_widget_show (gw_applet->labels[i]);
	else
		gtk_widget_hide (gw_applet->labels[i]);
    }
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

static void
num_forecasts_changed (GtkSpinButton *button, gpointer data)
{
    GWeatherApplet *applet = data;
    gint num;

    num = gtk_spin_button_get_value_as_int (button);
    applet->gweather_info->numforecasts = num;

    panel_applet_gconf_set_int(applet->applet, "num_forecasts", 
    		               			        num, NULL);
    place_widgets (applet);
    gweather_update (applet);

}


static void
response_cb (GtkDialog *dialog, gint id, gpointer data)
{
    GWeatherApplet *gw_applet = data;
  
    if(id == GTK_RESPONSE_HELP){
        help_cb (dialog);
	return;
    }
   
    if (gw_applet->locations_handle) {
	gnome_vfs_async_cancel (gw_applet->locations_handle);
	gw_applet->locations_handle = NULL;
    }
    gtk_widget_destroy (GTK_WIDGET (dialog));
    gw_applet->pref = NULL;

}

static void gweather_pref_create (GWeatherApplet *gw_applet)
{
    GtkWidget *pref_vbox;
    GtkWidget *pref_notebook;
    GtkObject *pref_basic_update_spin_adj;
    GtkWidget *pref_basic_update_sec_lbl;
    GtkWidget *pref_basic_note_lbl;
    GtkWidget *pref_loc_hbox;
    GtkWidget *pref_loc_note_lbl;
    GtkWidget *frame;
    GtkWidget *hbox, *hbox2;
    GtkWidget *vbox, *vbox2, *vbox3;
    GtkWidget *spin, *label;
    GtkWidget *scrolled, *check;
    GtkSizeGroup *size;
    gchar *tmp;
    
    gw_applet->pref = gtk_dialog_new_with_buttons (_("Weather Preferences"), NULL,
				      		   GTK_DIALOG_DESTROY_WITH_PARENT,
				      		   GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
				      		   GTK_STOCK_HELP, GTK_RESPONSE_HELP,
				      		   NULL);
    gtk_dialog_set_default_response (GTK_DIALOG (gw_applet->pref), GTK_RESPONSE_CLOSE);
    gtk_window_set_default_size(GTK_WINDOW (gw_applet->pref), 400,400);
    gtk_window_set_policy (GTK_WINDOW (gw_applet->pref), TRUE, TRUE, FALSE);
    gtk_window_set_screen (GTK_WINDOW (gw_applet->pref),
			   gtk_widget_get_screen (GTK_WIDGET (gw_applet->applet)));

    pref_vbox = GTK_DIALOG (gw_applet->pref)->vbox;
    gtk_widget_show (pref_vbox);


    pref_notebook = gtk_notebook_new ();
    gtk_widget_show (pref_notebook);
    gtk_box_pack_start (GTK_BOX (pref_vbox), pref_notebook, TRUE, TRUE, 0);

    size = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

   
  /*
   * General settings page.
   */
    
    vbox = gtk_vbox_new (FALSE, 18);
    pref_basic_note_lbl = gtk_label_new (_("General"));
    gtk_notebook_append_page (GTK_NOTEBOOK (pref_notebook), vbox, 
							 pref_basic_note_lbl);
    gtk_container_set_border_width (GTK_CONTAINER (vbox), 12);

    frame = gtk_frame_new (NULL);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);
    gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);	
    label = gtk_label_new (NULL);
    tmp = g_strdup_printf ("<b>%s</b>", _("Update"));
    gtk_label_set_markup (GTK_LABEL (label), tmp);
    g_free (tmp);
    gtk_frame_set_label_widget (GTK_FRAME (frame), label);

    vbox2 = gtk_vbox_new (FALSE, 6);
    gtk_container_add (GTK_CONTAINER (frame), vbox2);
    gtk_container_set_border_width (GTK_CONTAINER (vbox2), 12);

    hbox = gtk_hbox_new (FALSE, 6);
    gtk_box_pack_start (GTK_BOX (vbox2), hbox, FALSE, FALSE, 0);

    gw_applet->pref_basic_update_btn = gtk_check_button_new_with_mnemonic (_("_Automatically update every"));
    gtk_widget_show (gw_applet->pref_basic_update_btn);
    gtk_box_pack_start (GTK_BOX (hbox), gw_applet->pref_basic_update_btn, TRUE, TRUE, 0);
    g_signal_connect (G_OBJECT (gw_applet->pref_basic_update_btn), "toggled",
    		      G_CALLBACK (auto_update_toggled), gw_applet);

    hbox2 = gtk_hbox_new (FALSE, 6);
    gtk_box_pack_end (GTK_BOX (hbox), hbox2, FALSE, FALSE, 0);
    gtk_size_group_add_widget (size, hbox2);
    
    pref_basic_update_spin_adj = gtk_adjustment_new (30, 1, 60, 5, 25, 1);
    gw_applet->pref_basic_update_spin = gtk_spin_button_new (GTK_ADJUSTMENT (pref_basic_update_spin_adj), 1, 0);
    gtk_widget_show (gw_applet->pref_basic_update_spin);
    gtk_box_pack_start (GTK_BOX (hbox2), gw_applet->pref_basic_update_spin,
					  FALSE, FALSE, 0);
    gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (gw_applet->pref_basic_update_spin), TRUE);
    gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (gw_applet->pref_basic_update_spin), GTK_UPDATE_IF_VALID);
    g_signal_connect (G_OBJECT (gw_applet->pref_basic_update_spin),
 				      "value_changed",
    		      		       G_CALLBACK (update_interval_changed), gw_applet);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(gw_applet->pref_basic_update_spin), 
    			      gw_applet->gweather_pref.update_interval / 60);
    gtk_widget_set_sensitive(gw_applet->pref_basic_update_spin, 
    			     gw_applet->gweather_pref.update_enabled);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gw_applet->pref_basic_update_btn), 
    				 gw_applet->gweather_pref.update_enabled);

    pref_basic_update_sec_lbl = gtk_label_new (_("minute(s)"));
    gtk_widget_show (pref_basic_update_sec_lbl);
    gtk_box_pack_start (GTK_BOX (hbox2), pref_basic_update_sec_lbl, FALSE, FALSE, 0);

    frame = gtk_frame_new (NULL);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);
    gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);	
    label = gtk_label_new (NULL);
    tmp = g_strdup_printf ("<b>%s</b>", _("Display"));
    gtk_label_set_markup (GTK_LABEL (label), tmp);
    g_free (tmp);
    gtk_frame_set_label_widget (GTK_FRAME (frame), label);

    vbox2 = gtk_vbox_new (FALSE, 6);
    gtk_container_add (GTK_CONTAINER (frame), vbox2);
    gtk_container_set_border_width (GTK_CONTAINER (vbox2), 12);

    hbox = gtk_hbox_new (FALSE, 6);
    gtk_box_pack_start (GTK_BOX (vbox2), hbox, FALSE, FALSE, 0);
    gw_applet->pref_basic_metric_btn = gtk_check_button_new_with_mnemonic (_("Use _metric system units"));
    gtk_box_pack_start (GTK_BOX (vbox2), gw_applet->pref_basic_metric_btn, FALSE, FALSE, 0);
    gtk_widget_show (gw_applet->pref_basic_metric_btn);
    g_signal_connect (G_OBJECT (gw_applet->pref_basic_metric_btn), "toggled",
    		      G_CALLBACK (metric_toggled), gw_applet);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gw_applet->pref_basic_metric_btn), 
    				 gw_applet->gweather_pref.use_metric);

    hbox = gtk_hbox_new (FALSE, 6);
    gtk_box_pack_start (GTK_BOX (vbox2), hbox, FALSE, FALSE, 0);
    check = gtk_check_button_new_with_mnemonic (_("Display daily average temperatures"));
    gtk_box_pack_start (GTK_BOX (vbox2), check, FALSE, FALSE, 0);
    gtk_widget_show (gw_applet->pref_basic_metric_btn);
    g_signal_connect (G_OBJECT (check), "toggled",
    		      G_CALLBACK (show_labels_toggled), gw_applet);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check), 
    				 gw_applet->gweather_pref.show_labels);
    
    hbox = gtk_hbox_new (FALSE, 6);
    gtk_box_pack_start (GTK_BOX (vbox2), hbox, FALSE, FALSE, 0);

    label = gtk_label_new_with_mnemonic (_("Number of _forecast days to display:"));
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
    
    spin = gtk_spin_button_new_with_range (0, 5, 1);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (spin), gw_applet->gweather_info->numforecasts);
    g_signal_connect (G_OBJECT (spin), "value_changed",
    		      G_CALLBACK (num_forecasts_changed), gw_applet);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), spin);
    gtk_box_pack_end	(GTK_BOX (hbox), spin, FALSE, FALSE, 0);
    gtk_size_group_add_widget (size, spin);

  /*
   * Location page.
   */
    vbox = gtk_vbox_new (FALSE, 18);
    label = gtk_label_new (_("Location"));
    gtk_notebook_append_page (GTK_NOTEBOOK (pref_notebook), vbox, 
							 label);
    gtk_container_set_border_width (GTK_CONTAINER (vbox), 12);

    frame = gtk_frame_new (NULL);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);
    gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);	
    label = gtk_label_new (NULL);
    tmp = g_strdup_printf ("<b>%s</b>", _("Location"));
    gtk_label_set_markup (GTK_LABEL (label), tmp);
    g_free (tmp);
    gtk_frame_set_label_widget (GTK_FRAME (frame), label);

    vbox2 = gtk_vbox_new (FALSE, 6);
    gtk_container_add (GTK_CONTAINER (frame), vbox2);
    gtk_container_set_border_width (GTK_CONTAINER (vbox2), 12);

    hbox = gtk_hbox_new (FALSE, 6);
    gtk_box_pack_start (GTK_BOX (vbox2), hbox, FALSE, FALSE, 0);
    label = gtk_label_new (_("Current city:"));
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
    gw_applet->pref_location_city_label = gtk_label_new (gw_applet->gweather_pref.city);
    gtk_box_pack_start (GTK_BOX (hbox), gw_applet->pref_location_city_label, FALSE, FALSE, 0);

    hbox = gtk_hbox_new (TRUE, 6);
    gtk_box_pack_start (GTK_BOX (vbox2), hbox, TRUE, TRUE, 0);

    vbox3 = gtk_vbox_new (FALSE, 6);
    gtk_box_pack_start (GTK_BOX (hbox), vbox3, TRUE, TRUE, 0); 
    hbox2 = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox3), hbox2, FALSE, FALSE, 0); 
    label = gtk_label_new_with_mnemonic (_("Available c_ountries:"));
    gtk_box_pack_start (GTK_BOX (hbox2), label, FALSE, FALSE, 0);
    scrolled = create_countries_widget (gw_applet);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), gw_applet->country_tree);
    scrolled = create_countries_widget (gw_applet);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), gw_applet->country_tree);
    gtk_box_pack_start (GTK_BOX (vbox3), scrolled, TRUE, TRUE, 0);

    vbox3 = gtk_vbox_new (FALSE, 6);
    gtk_box_pack_start (GTK_BOX (hbox), vbox3, TRUE, TRUE, 0);
    hbox2 = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox3), hbox2, FALSE, FALSE, 0); 
    label = gtk_label_new_with_mnemonic (_("Available c_ities:"));    
    gtk_box_pack_start (GTK_BOX (hbox2), label, FALSE, FALSE, 0);
    scrolled = create_cities_widget (gw_applet);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), gw_applet->city_tree);
    gtk_box_pack_start (GTK_BOX (vbox3), scrolled, TRUE, TRUE, 0);

    g_signal_connect (G_OBJECT (gw_applet->pref), "response",
    		      G_CALLBACK (response_cb), gw_applet);
#if 0   
    gweather_pref_set_accessibility (gw_applet); 
    add_atk_relation (gw_applet->pref_basic_update_spin, pref_basic_update_sec_lbl,
                                            ATK_RELATION_LABELLED_BY);
    add_atk_relation (gw_applet->pref_basic_radar_url_entry, label, ATK_RELATION_LABELLED_BY);
 #endif

    gtk_widget_show_all (gw_applet->pref);
    fetch_countries (gw_applet);
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
    gw_applet->gweather_pref.update_interval = MAX (gw_applet->gweather_pref.update_interval, 1000);
    gw_applet->gweather_pref.update_enabled =
    	panel_applet_gconf_get_bool(gw_applet->applet, "auto_update", NULL);
    gw_applet->gweather_pref.use_metric = 
    	panel_applet_gconf_get_bool(gw_applet->applet, "enable_metric", NULL);
    
    gw_applet->gweather_info->numforecasts = panel_applet_gconf_get_int (gw_applet->applet,
    								    "num_forecasts",
    								    NULL);
    gw_applet->gweather_info->numforecasts = CLAMP (gw_applet->gweather_info->numforecasts, 0, 5);
    gw_applet->gweather_pref.url = NULL;
    gw_applet->gweather_pref.url = panel_applet_gconf_get_string(gw_applet->applet, "url", NULL);
    gw_applet->gweather_pref.city = panel_applet_gconf_get_string(gw_applet->applet, "city", NULL);
    g_print ("city %s name \n", gw_applet->gweather_pref.url);
    if (!gw_applet->gweather_pref.url || (strlen (gw_applet->gweather_pref.url)==0)) {
	gw_applet->gweather_pref.url=g_strdup ("http://weather.interceptvector.com/weather.xml?&state=PA&city=Pittsburgh");
	gw_applet->gweather_pref.city = g_strdup ("Pittsburgh");
    }
    gw_applet->gweather_pref.show_labels =
    	panel_applet_gconf_get_bool(gw_applet->applet, "show_labels", NULL);
}

static void help_cb (GtkDialog *dialog)
{
    GError *error = NULL;

    egg_screen_help_display (
		gtk_window_get_screen (GTK_WINDOW (dialog)),
		"gweather", "gweather-prefs", &error);

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

}

