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

static void close_cb (GtkButton *button, gpointer user_data);

static gint cmp_loc (const WeatherLocation *l1, const WeatherLocation *l2)
{
    return (l1 && l2 && weather_location_equal(l1, l2)) ? 0 : 1;
}

/* Update pref dialog from gweather_pref */
static gboolean update_dialog (GWeatherApplet *gw_applet)
{
    GtkCTreeNode *node;

    g_return_val_if_fail(gw_applet->gweather_pref.location != NULL, FALSE);

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(gw_applet->pref_basic_update_spin), gw_applet->gweather_pref.update_interval / 60);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gw_applet->pref_basic_update_btn), gw_applet->gweather_pref.update_enabled);
    gtk_widget_set_sensitive(gw_applet->pref_basic_update_spin, gw_applet->gweather_pref.update_enabled);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gw_applet->pref_basic_metric_btn), gw_applet->gweather_pref.use_metric);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gw_applet->pref_basic_detailed_btn), gw_applet->gweather_pref.detailed);
#ifdef RADARMAP
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gw_applet->pref_basic_radar_btn), gw_applet->gweather_pref.radar_enabled);
#endif /* RADARMAP */

    gtk_entry_set_text(GTK_ENTRY(gw_applet->pref_net_proxy_url_entry), gw_applet->gweather_pref.proxy_url ? gw_applet->gweather_pref.proxy_url : "");
    gtk_entry_set_text(GTK_ENTRY(gw_applet->pref_net_proxy_passwd_entry), gw_applet->gweather_pref.proxy_passwd ? gw_applet->gweather_pref.proxy_passwd : "");
    gtk_entry_set_text(GTK_ENTRY(gw_applet->pref_net_proxy_user_entry), gw_applet->gweather_pref.proxy_user ? gw_applet->gweather_pref.proxy_user : "");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gw_applet->pref_net_proxy_btn), gw_applet->gweather_pref.use_proxy);

    node = gtk_ctree_find_by_row_data_custom(GTK_CTREE(gw_applet->pref_loc_ctree),
                                     gw_applet->pref_loc_root, gw_applet->gweather_pref.location,
                                     (GCompareFunc)cmp_loc);
    g_return_val_if_fail(node != NULL, FALSE);
    gtk_ctree_select(GTK_CTREE(gw_applet->pref_loc_ctree), node);

    return TRUE;
}

static __inline void update_string (gchar *val, gchar **str)
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
    WeatherLocation *loc = (WeatherLocation *)gtk_ctree_node_get_row_data(GTK_CTREE(gw_applet->pref_loc_ctree), GTK_CTREE_NODE(gw_applet->pref_loc_sel_node));
    gchar *proxy_url;
    
    proxy_url = (gchar *)gtk_entry_get_text(GTK_ENTRY(gw_applet->pref_net_proxy_url_entry));

    if (!loc) {
        gnome_error_dialog(_("Invalid location chosen!\nProperties remain unchanged."));
        return FALSE;
    }

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gw_applet->pref_net_proxy_btn))
	&& proxy_url && strlen(proxy_url) && !check_proxy_uri(proxy_url)) {
        gnome_error_dialog(_("Proxy URL is not of the form http://host:port/\nProperties remain unchanged."));
        return FALSE;
    }

    weather_location_free(gw_applet->gweather_pref.location);
    gw_applet->gweather_pref.location = weather_location_clone(loc);

    gw_applet->gweather_pref.update_interval = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(gw_applet->pref_basic_update_spin)) * 60;
    gw_applet->gweather_pref.update_enabled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gw_applet->pref_basic_update_btn));
    gw_applet->gweather_pref.use_metric = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gw_applet->pref_basic_metric_btn));
    gw_applet->gweather_pref.detailed = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gw_applet->pref_basic_detailed_btn));
#ifdef RADARMAP
    gw_applet->gweather_pref.radar_enabled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gw_applet->pref_basic_radar_btn));
#endif /* RADARMAP */

    gw_applet->gweather_pref.use_proxy = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gw_applet->pref_net_proxy_btn));
    update_string(proxy_url, &(gw_applet->gweather_pref.proxy_url));
    update_string((gchar *)gtk_entry_get_text(GTK_ENTRY(gw_applet->pref_net_proxy_passwd_entry)), &(gw_applet->gweather_pref.proxy_passwd));
    update_string((gchar *)gtk_entry_get_text(GTK_ENTRY(gw_applet->pref_net_proxy_user_entry)), &(gw_applet->gweather_pref.proxy_user));

    /* fprintf(stderr, "Set location to: %s\n", loc->name); */

    return TRUE;
}

static void change_cb (GtkButton *button, gpointer user_data)
{
    GWeatherApplet *gw_applet = user_data;
    
    gnome_dialog_set_sensitive(GNOME_DIALOG(gw_applet->pref), 0, TRUE);
    gnome_dialog_set_sensitive(GNOME_DIALOG(gw_applet->pref), 1, TRUE);
    gtk_widget_set_sensitive(gw_applet->pref_basic_update_spin, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gw_applet->pref_basic_update_btn)));
    gtk_widget_set_sensitive(gw_applet->pref_net_proxy_url_entry, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gw_applet->pref_net_proxy_btn)));
    gtk_widget_set_sensitive(gw_applet->pref_net_proxy_user_entry, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gw_applet->pref_net_proxy_btn)));
    gtk_widget_set_sensitive(gw_applet->pref_net_proxy_passwd_entry, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gw_applet->pref_net_proxy_btn)));
    return;
}

static void tree_select_row_cb (GtkCTree     *ctree,
                                GtkCTreeNode *row,
                                gint          column)
{
	GWeatherApplet *gw_applet = g_object_get_data(G_OBJECT(ctree), "user_data");
	
    gw_applet->pref_loc_sel_node = row;
    if (gtk_ctree_node_get_row_data(GTK_CTREE(gw_applet->pref_loc_ctree), GTK_CTREE_NODE(gw_applet->pref_loc_sel_node)) != NULL)
        change_cb(NULL, gw_applet);
    return;
}

static void load_locations (GWeatherApplet *gw_applet)
{
    GtkCTreeNode *region, *state, *location;
    GtkCTree *ctree = GTK_CTREE(gw_applet->pref_loc_ctree);

    gchar *pp[1], *path;
    gint nregions, iregions;
    gchar **regions;

    path = g_strdup_printf("=%s=/", gnome_datadir_file("gweather/Locations"));
    gnome_config_push_prefix(path);
    g_free(path);

    pp[0] = _("Regions");
    gw_applet->pref_loc_root = gtk_ctree_insert_node (ctree, NULL, NULL, pp, 0,
                                           NULL, NULL, NULL, NULL,
                                           FALSE, TRUE);

    gnome_config_get_vector("Main/regions", &nregions, &regions);

    region = NULL;
    for (iregions = nregions-1;  iregions >= 0;  iregions--) {
        gint nstates, istates;
        gchar **states;
        gchar *region_name;
        gchar *region_name_key = g_strconcat(regions[iregions], "/name", NULL);
        gchar *states_key = g_strconcat(regions[iregions], "/states", NULL);

        region_name = gnome_config_get_string(region_name_key);

        pp[0] = region_name;
        region = gtk_ctree_insert_node (ctree, gw_applet->pref_loc_root, region, pp, 0,
                                        NULL, NULL, NULL, NULL,
                                        FALSE, FALSE);

        gnome_config_get_vector(states_key, &nstates, &states);

        state = NULL;
        for (istates = nstates - 1;  istates >= 0;  istates--) {
            void *iter;
            gchar *iter_key, *iter_val;
            gchar *state_path = g_strconcat(regions[iregions], "_", states[istates], "/", NULL);
            gchar *state_name_key = g_strconcat(state_path, "name", NULL);
            gchar *state_name = gnome_config_get_string(state_name_key);

            pp[0] = state_name;
            state = gtk_ctree_insert_node (ctree, region, state, pp, 0,
                                            NULL, NULL, NULL, NULL,
                                            FALSE, FALSE);

            location = NULL;
            iter = gnome_config_init_iterator(state_path);
            while ((iter = gnome_config_iterator_next(iter, &iter_key, &iter_val)) != NULL) {
                if (strstr(iter_key, "loc") != NULL) {
                    gchar **locdata;
                    gint nlocdata;
                    WeatherLocation *weather_location;

                    gnome_config_make_vector(iter_val, &nlocdata, &locdata);
                    g_return_if_fail(nlocdata == 4);

                    pp[0] = locdata[0];
                    location = gtk_ctree_insert_node (ctree, state, location, pp, 0,
                                                      NULL, NULL, NULL, NULL,
                                                      FALSE, TRUE);
                    weather_location = weather_location_new(locdata[0], locdata[1], locdata[2], locdata[3]);
                    gtk_ctree_node_set_row_data_full (ctree, location,
                                                      weather_location,
                                                      (GtkDestroyNotify)weather_location_free);

                    if (gw_applet->gweather_pref.location && weather_location_equal(weather_location, gw_applet->gweather_pref.location)) {
                        gtk_ctree_expand (ctree, state);
                        gtk_ctree_expand (ctree, region);
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
    GtkWidget *pref_action_area;
    GtkWidget *scrolled_window;

    GtkWidget *pref_basic_vbox;
    GtkWidget *vbox;
    GtkWidget *frame;

    gw_applet->pref = gnome_dialog_new (_("GNOME Weather Properties"),
			     GNOME_STOCK_BUTTON_OK,
			     GNOME_STOCK_BUTTON_APPLY,
			     GNOME_STOCK_BUTTON_CLOSE,
			     GNOME_STOCK_BUTTON_HELP,
			     NULL);
    gtk_widget_set_usize (gw_applet->pref, -2, 280);
    gtk_window_set_policy (GTK_WINDOW (gw_applet->pref), TRUE, TRUE, FALSE);
    gnome_dialog_close_hides (GNOME_DIALOG (gw_applet->pref), TRUE);

    pref_vbox = GNOME_DIALOG (gw_applet->pref)->vbox;
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
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
				    GTK_POLICY_AUTOMATIC,
				    GTK_POLICY_AUTOMATIC);

    gw_applet->pref_loc_ctree = gtk_ctree_new (1, 0);
    gtk_container_add (GTK_CONTAINER (scrolled_window), gw_applet->pref_loc_ctree);
    gtk_widget_show (gw_applet->pref_loc_ctree);
    gtk_widget_show (scrolled_window);
    gtk_box_pack_start (GTK_BOX (pref_loc_hbox), scrolled_window, TRUE, TRUE, 0);
    gtk_clist_set_column_width (GTK_CLIST (gw_applet->pref_loc_ctree), 0, 80);
    gtk_clist_set_selection_mode (GTK_CLIST (gw_applet->pref_loc_ctree), GTK_SELECTION_BROWSE);
    gtk_clist_column_titles_hide (GTK_CLIST (gw_applet->pref_loc_ctree));
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

    gw_applet->pref_basic_metric_btn = gtk_check_button_new_with_label (_("Use metric system units"));
    gtk_widget_show (gw_applet->pref_basic_metric_btn);
    gtk_container_add (GTK_CONTAINER (pref_basic_metric_alignment), gw_applet->pref_basic_metric_btn);

    gw_applet->pref_basic_detailed_btn = gtk_check_button_new_with_label (_("Enable detailed forecast"));
    gtk_widget_show (gw_applet->pref_basic_detailed_btn);
    gtk_container_add (GTK_CONTAINER (pref_basic_detailed_alignment), gw_applet->pref_basic_detailed_btn);

#ifdef RADARMAP
    gw_applet->pref_basic_radar_btn = gtk_check_button_new_with_label (_("Enable radar map"));
    gtk_widget_show (gw_applet->pref_basic_radar_btn);
    gtk_container_add (GTK_CONTAINER (pref_basic_radar_alignment), gw_applet->pref_basic_radar_btn);
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

    pref_basic_update_spin_adj = gtk_adjustment_new (30, 1, 60, 1, 5, 1);
    gw_applet->pref_basic_update_spin = gtk_spin_button_new (GTK_ADJUSTMENT (pref_basic_update_spin_adj), 1, 0);
    gtk_widget_show (gw_applet->pref_basic_update_spin);
/*    gtk_box_pack_start (GTK_BOX (pref_basic_update_hbox), pref_basic_update_spin, TRUE, FALSE, 0); */
/*    gtk_widget_set_usize (pref_basic_update_spin, 80, -2); */
    gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (gw_applet->pref_basic_update_spin), TRUE);
    gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (gw_applet->pref_basic_update_spin), GTK_UPDATE_IF_VALID);

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

    pref_action_area = GNOME_DIALOG (gw_applet->pref)->action_area;
    gtk_widget_show (pref_action_area);
    gtk_button_box_set_layout (GTK_BUTTON_BOX (pref_action_area), GTK_BUTTONBOX_END);
    gtk_button_box_set_spacing (GTK_BUTTON_BOX (pref_action_area), 8);

    gtk_signal_connect (GTK_OBJECT (gw_applet->pref_loc_ctree), "tree_select_row",
			GTK_SIGNAL_FUNC (tree_select_row_cb), NULL);

    gtk_signal_connect (GTK_OBJECT (gw_applet->pref_basic_update_spin), "changed",
			GTK_SIGNAL_FUNC (change_cb), gw_applet);
    gtk_signal_connect (GTK_OBJECT (gw_applet->pref_basic_metric_btn), "toggled",
			GTK_SIGNAL_FUNC (change_cb), gw_applet);
    gtk_signal_connect (GTK_OBJECT (gw_applet->pref_basic_detailed_btn), "toggled",
			GTK_SIGNAL_FUNC (change_cb), gw_applet);
#ifdef RADARMAP
    gtk_signal_connect (GTK_OBJECT (gw_applet->pref_basic_radar_btn), "toggled",
			GTK_SIGNAL_FUNC (change_cb), gw_applet);
#endif /* RADARMAP */
    gtk_signal_connect (GTK_OBJECT (gw_applet->pref_basic_update_btn), "toggled",
			GTK_SIGNAL_FUNC (change_cb), gw_applet);
    gtk_signal_connect (GTK_OBJECT (gw_applet->pref_net_proxy_btn), "toggled",
			GTK_SIGNAL_FUNC (change_cb), gw_applet);
    gtk_signal_connect (GTK_OBJECT (gw_applet->pref_net_proxy_url_entry), "changed",
			GTK_SIGNAL_FUNC (change_cb), gw_applet);
    gtk_signal_connect (GTK_OBJECT (gw_applet->pref_net_proxy_user_entry), "changed",
			GTK_SIGNAL_FUNC (change_cb), gw_applet);
    gtk_signal_connect (GTK_OBJECT (gw_applet->pref_net_proxy_passwd_entry), "changed",
			GTK_SIGNAL_FUNC (change_cb), gw_applet);

    gtk_widget_show_all (gw_applet->pref);
}


void gweather_pref_load (const gchar *path, GWeatherApplet *gw_applet)
{
/*    gchar *prefix; */
    
/*    g_return_if_fail(path != NULL);

    prefix = g_strconcat (path, "Preferences/", NULL); */
/*    gnome_config_push_prefix(prefix); */
    /* fprintf(stderr, "gweather_pref_load: %s\n", prefix); */
/*    g_free(prefix);

    gw_applet->gweather_pref.update_interval = gnome_config_get_int("update_interval=1800");
    gw_applet->gweather_pref.update_enabled = gnome_config_get_bool("update_enabled=TRUE");
    gw_applet->gweather_pref.use_metric = gnome_config_get_bool("use_metric=FALSE");
    gw_applet->gweather_pref.detailed = gnome_config_get_bool("detailed=FALSE");
    gw_applet->gweather_pref.radar_enabled = gnome_config_get_bool("radar_enabled=TRUE");
    gw_applet->gweather_pref.location = weather_location_config_read("location");
    gw_applet->gweather_pref.proxy_url = gnome_config_get_string("proxy_url");
    gw_applet->gweather_pref.proxy_user = gnome_config_get_string("proxy_user");
    gw_applet->gweather_pref.proxy_passwd = gnome_config_private_get_string("proxy_passwd");
    gw_applet->gweather_pref.use_proxy = gnome_config_get_bool("use_proxy=FALSE");
*/
    gw_applet->gweather_pref.update_interval = 1800;
    gw_applet->gweather_pref.update_enabled =TRUE;
    gw_applet->gweather_pref.use_metric = FALSE;
    gw_applet->gweather_pref.detailed = TRUE;
    gw_applet->gweather_pref.radar_enabled = TRUE;
    gw_applet->gweather_pref.location = weather_location_config_read("location");
    gw_applet->gweather_pref.proxy_url = NULL;
    gw_applet->gweather_pref.proxy_user = NULL;
    gw_applet->gweather_pref.proxy_passwd = NULL;
    gw_applet->gweather_pref.use_proxy = FALSE;
/*    gnome_config_pop_prefix(); */

	return;
}

void gweather_pref_save (const gchar *path, GWeatherApplet *gw_applet)
{
    gchar *prefix;

    g_return_if_fail(path != NULL);

    prefix = g_strconcat (path, "Preferences/", NULL);
    gnome_config_push_prefix(prefix);
    /* fprintf(stderr, "gweather_pref_save: %s\n", prefix); */
    g_free(prefix);

    gnome_config_set_int("update_interval", gw_applet->gweather_pref.update_interval);
    gnome_config_set_bool("update_enabled", gw_applet->gweather_pref.update_enabled);
    gnome_config_set_bool("use_metric", gw_applet->gweather_pref.use_metric);
    gnome_config_set_bool("detailed", gw_applet->gweather_pref.detailed);
    gnome_config_set_bool("radar_enabled", gw_applet->gweather_pref.radar_enabled);
    weather_location_config_write("location", gw_applet->gweather_pref.location);
    gnome_config_set_string("proxy_url", gw_applet->gweather_pref.proxy_url);
    gnome_config_set_string("proxy_user", gw_applet->gweather_pref.proxy_user);
    gnome_config_private_set_string("proxy_passwd", gw_applet->gweather_pref.proxy_passwd);
    gnome_config_set_bool("use_proxy", gw_applet->gweather_pref.use_proxy);

    gnome_config_pop_prefix();

    gnome_config_sync();
    gnome_config_drop_all();
}


static void ok_cb (GtkButton *button, gpointer user_data)
{
	GWeatherApplet *gw_applet = (GWeatherApplet *)user_data;
	
    if (update_pref(gw_applet))
		close_cb (button, user_data);
    return;
}

static void apply_cb (GtkButton *button, gpointer user_data)
{
	GWeatherApplet *gw_applet = (GWeatherApplet *)user_data;
	
    gnome_dialog_set_sensitive(GNOME_DIALOG(gw_applet->pref), 0, FALSE);
    gnome_dialog_set_sensitive(GNOME_DIALOG(gw_applet->pref), 1, FALSE);
    update_pref(gw_applet);
    return;
}

static void close_cb (GtkButton *button, gpointer user_data)
{
    GWeatherApplet *gw_applet = (GWeatherApplet *)user_data;
    
    gnome_dialog_close (GNOME_DIALOG (gw_applet->pref));

    gtk_widget_destroy (gw_applet->pref);
    gw_applet->pref = NULL;
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
    if (gw_applet->pref)
	return;

    gweather_pref_create(gw_applet);
    update_dialog(gw_applet);

    gnome_dialog_button_connect (GNOME_DIALOG (gw_applet->pref), 0, G_CALLBACK(ok_cb), gw_applet);
    gnome_dialog_button_connect (GNOME_DIALOG (gw_applet->pref), 1, G_CALLBACK(apply_cb), gw_applet);
    gnome_dialog_button_connect (GNOME_DIALOG (gw_applet->pref), 2, G_CALLBACK(close_cb), gw_applet);
    gnome_dialog_button_connect (GNOME_DIALOG (gw_applet->pref), 3, G_CALLBACK(help_cb), gw_applet);
}

