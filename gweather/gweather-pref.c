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
#include <applet-widget.h>

#include "gweather-applet.h"
#include "gweather-pref.h"

GWeatherPrefs gweather_pref = {NULL, 1800, TRUE, FALSE, FALSE, TRUE,
                               NULL, NULL, NULL, FALSE};

static GtkWidget *pref = NULL;

static GtkWidget *pref_basic_metric_btn;
static GtkWidget *pref_basic_detailed_btn;
#ifdef RADARMAP
static GtkWidget *pref_basic_radar_btn;
#endif /* RADARMAP */
static GtkWidget *pref_basic_update_spin;
static GtkWidget *pref_basic_update_btn;
static GtkWidget *pref_net_proxy_btn;
static GtkWidget *pref_net_proxy_url_entry;
static GtkWidget *pref_net_proxy_user_entry;
static GtkWidget *pref_net_proxy_passwd_entry;
static GtkWidget *pref_loc_ctree;
static GtkCTreeNode *pref_loc_root;
static GtkCTreeNode *pref_loc_sel_node = NULL;
static GnomeHelpMenuEntry help_entry = { "gweather_applet", "index.html#GWEATHER-PREFS"};


static gint cmp_loc (const WeatherLocation *l1, const WeatherLocation *l2)
{
    return (l1 && l2 && weather_location_equal(l1, l2)) ? 0 : 1;
}

/* Update pref dialog from gweather_pref */
static gboolean update_dialog (void)
{
    GtkCTreeNode *node;

    g_return_val_if_fail(gweather_pref.location != NULL, FALSE);

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(pref_basic_update_spin), gweather_pref.update_interval);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pref_basic_update_btn), gweather_pref.update_enabled);
    gtk_widget_set_sensitive(pref_basic_update_spin, gweather_pref.update_enabled);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pref_basic_metric_btn), gweather_pref.use_metric);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pref_basic_detailed_btn), gweather_pref.detailed);
#ifdef RADARMAP
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pref_basic_radar_btn), gweather_pref.radar_enabled);
#endif /* RADARMAP */

    gtk_entry_set_text(GTK_ENTRY(pref_net_proxy_url_entry), gweather_pref.proxy_url ? gweather_pref.proxy_url : "");
    gtk_entry_set_text(GTK_ENTRY(pref_net_proxy_passwd_entry), gweather_pref.proxy_passwd ? gweather_pref.proxy_passwd : "");
    gtk_entry_set_text(GTK_ENTRY(pref_net_proxy_user_entry), gweather_pref.proxy_user ? gweather_pref.proxy_user : "");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pref_net_proxy_btn), gweather_pref.use_proxy);

    node = gtk_ctree_find_by_row_data_custom(GTK_CTREE(pref_loc_ctree),
                                     pref_loc_root, gweather_pref.location,
                                     (GCompareFunc)cmp_loc);
    g_return_val_if_fail(node != NULL, FALSE);
    gtk_ctree_select(GTK_CTREE(pref_loc_ctree), node);

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
static gboolean update_pref (void)
{
    WeatherLocation *loc = (WeatherLocation *)gtk_ctree_node_get_row_data(GTK_CTREE(pref_loc_ctree), GTK_CTREE_NODE(pref_loc_sel_node));
    gchar *proxy_url = gtk_entry_get_text(GTK_ENTRY(pref_net_proxy_url_entry));

    if (!loc) {
        gnome_error_dialog(_("Invalid location chosen!\nProperties remain unchanged."));
        return FALSE;
    }

    if (proxy_url && strlen(proxy_url) && !check_proxy_uri(proxy_url)) {
        gnome_error_dialog(_("Proxy URL is not of the form http://host:port/\nProperties remain unchanged."));
        return FALSE;
    }

    weather_location_free(gweather_pref.location);
    gweather_pref.location = weather_location_clone(loc);

    gweather_pref.update_interval = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(pref_basic_update_spin));
    gweather_pref.update_enabled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pref_basic_update_btn));
    gweather_pref.use_metric = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pref_basic_metric_btn));
    gweather_pref.detailed = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pref_basic_detailed_btn));
#ifdef RADARMAP
    gweather_pref.radar_enabled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pref_basic_radar_btn));
#endif /* RADARMAP */

    gweather_pref.use_proxy = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pref_net_proxy_btn));
    update_string(proxy_url, &gweather_pref.proxy_url);
    update_string(gtk_entry_get_text(GTK_ENTRY(pref_net_proxy_passwd_entry)), &gweather_pref.proxy_passwd);
    update_string(gtk_entry_get_text(GTK_ENTRY(pref_net_proxy_user_entry)), &gweather_pref.proxy_user);

    /* fprintf(stderr, "Set location to: %s\n", loc->name); */

    return TRUE;
}

static void ok_cb (GtkButton *button, gpointer user_data)
{
    update_pref();
    return;
}

static void apply_cb (GtkButton *button, gpointer user_data)
{
    gnome_dialog_set_sensitive(GNOME_DIALOG(pref), 0, FALSE);
    gnome_dialog_set_sensitive(GNOME_DIALOG(pref), 1, FALSE);
    update_pref();
    return;
}

static void change_cb (GtkButton *button, gpointer user_data)
{
    gnome_dialog_set_sensitive(GNOME_DIALOG(pref), 0, TRUE);
    gnome_dialog_set_sensitive(GNOME_DIALOG(pref), 1, TRUE);
    gtk_widget_set_sensitive(pref_basic_update_spin, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pref_basic_update_btn)));
    gtk_widget_set_sensitive(pref_net_proxy_url_entry, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pref_net_proxy_btn)));
    gtk_widget_set_sensitive(pref_net_proxy_user_entry, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pref_net_proxy_btn)));
    gtk_widget_set_sensitive(pref_net_proxy_passwd_entry, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pref_net_proxy_btn)));
    return;
}

static void tree_select_row_cb (GtkCTree     *ctree,
                                GtkCTreeNode *row,
                                gint          column)
{
    pref_loc_sel_node = row;
    if (gtk_ctree_node_get_row_data(GTK_CTREE(pref_loc_ctree), GTK_CTREE_NODE(pref_loc_sel_node)) != NULL)
        change_cb(NULL, NULL);
    return;
}

static void load_locations (void)
{
    GtkCTreeNode *region, *state, *location;
    GtkCTree *ctree = GTK_CTREE(pref_loc_ctree);

    gchar *pp[1], *path;
    gint nregions, iregions;
    gchar **regions;

    path = g_strdup_printf("=%s=/", gnome_datadir_file("gweather/Locations"));
    gnome_config_push_prefix(path);
    g_free(path);

    pp[0] = _("Regions");
    pref_loc_root = gtk_ctree_insert_node (ctree, NULL, NULL, pp, 0,
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
        region = gtk_ctree_insert_node (ctree, pref_loc_root, region, pp, 0,
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

                    if (gweather_pref.location && weather_location_equal(weather_location, gweather_pref.location)) {
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

static void gweather_pref_create (void)
{
  GtkWidget *pref_vbox;
  GtkWidget *pref_notebook;
  GtkWidget *pref_basic_table;
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
  GtkWidget *ok_button;
  GtkWidget *apply_button;
  GtkWidget *cancel_button;
  GtkWidget *help_button;
  GtkWidget *pref_loc_scroll;
  GtkObject *pref_loc_adj;

  g_return_if_fail(pref == NULL);

  pref = gnome_dialog_new (_("Gweather Properties"), NULL);
  gtk_widget_set_usize (pref, -2, 280);
  gtk_window_set_policy (GTK_WINDOW (pref), FALSE, FALSE, FALSE);
  gnome_dialog_close_hides(GNOME_DIALOG(pref), TRUE);

  pref_vbox = GNOME_DIALOG (pref)->vbox;
  gtk_widget_show (pref_vbox);

  pref_notebook = gtk_notebook_new ();
  gtk_widget_show (pref_notebook);
  gtk_box_pack_start (GTK_BOX (pref_vbox), pref_notebook, TRUE, TRUE, 0);

#ifdef RADARMAP
  pref_basic_table = gtk_table_new (5, 2, FALSE);
#else /* RADARMAP */
  pref_basic_table = gtk_table_new (4, 2, FALSE);
#endif /* RADARMAP */
  gtk_widget_show (pref_basic_table);
  gtk_container_add (GTK_CONTAINER (pref_notebook), pref_basic_table);
  gtk_container_set_border_width (GTK_CONTAINER (pref_basic_table), 8);
  gtk_table_set_row_spacings (GTK_TABLE (pref_basic_table), 4);
  gtk_table_set_col_spacings (GTK_TABLE (pref_basic_table), 4);

  pref_basic_update_alignment = gtk_alignment_new (0, 0.5, 0, 1);
  gtk_widget_show (pref_basic_update_alignment);
  gtk_table_attach (GTK_TABLE (pref_basic_table), pref_basic_update_alignment, 0, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  pref_basic_metric_alignment = gtk_alignment_new (0, 0.5, 0, 1);
  gtk_widget_show (pref_basic_metric_alignment);
  gtk_table_attach (GTK_TABLE (pref_basic_table), pref_basic_metric_alignment, 0, 2, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  pref_basic_detailed_alignment = gtk_alignment_new (0, 0.5, 0, 1);
  gtk_widget_show (pref_basic_detailed_alignment);
  gtk_table_attach (GTK_TABLE (pref_basic_table), pref_basic_detailed_alignment, 0, 2, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

#ifdef RADARMAP
  pref_basic_radar_alignment = gtk_alignment_new (0, 0.5, 0, 1);
  gtk_widget_show (pref_basic_radar_alignment);
  gtk_table_attach (GTK_TABLE (pref_basic_table), pref_basic_radar_alignment, 0, 2, 4, 5,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
#endif /* RADARMAP */

  pref_basic_update_btn = gtk_check_button_new_with_label (_("Update enabled"));
  gtk_widget_show (pref_basic_update_btn);
  gtk_container_add (GTK_CONTAINER (pref_basic_update_alignment), pref_basic_update_btn);

  pref_basic_metric_btn = gtk_check_button_new_with_label (_("Use metric"));
  gtk_widget_show (pref_basic_metric_btn);
  gtk_container_add (GTK_CONTAINER (pref_basic_metric_alignment), pref_basic_metric_btn);

  pref_basic_detailed_btn = gtk_check_button_new_with_label (_("Detailed forecast"));
  gtk_widget_show (pref_basic_detailed_btn);
  gtk_container_add (GTK_CONTAINER (pref_basic_detailed_alignment), pref_basic_detailed_btn);

#ifdef RADARMAP
  pref_basic_radar_btn = gtk_check_button_new_with_label (_("Enable radar maps"));
  gtk_widget_show (pref_basic_radar_btn);
  gtk_container_add (GTK_CONTAINER (pref_basic_radar_alignment), pref_basic_radar_btn);
#endif /* RADARMAP */

  pref_basic_update_lbl = gtk_label_new (_("Update:"));
  gtk_widget_show (pref_basic_update_lbl);
  gtk_table_attach (GTK_TABLE (pref_basic_table), pref_basic_update_lbl, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (pref_basic_update_lbl), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (pref_basic_update_lbl), 1, 0.5);

  pref_basic_update_hbox = gtk_hbox_new (FALSE, 2);
  gtk_widget_show (pref_basic_update_hbox);
  gtk_table_attach (GTK_TABLE (pref_basic_table), pref_basic_update_hbox, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  pref_basic_update_spin_adj = gtk_adjustment_new (300, 30, 7200, 10, 30, 30);
  pref_basic_update_spin = gtk_spin_button_new (GTK_ADJUSTMENT (pref_basic_update_spin_adj), 1, 0);
  gtk_widget_show (pref_basic_update_spin);
  gtk_box_pack_start (GTK_BOX (pref_basic_update_hbox), pref_basic_update_spin, TRUE, TRUE, 0);
  gtk_widget_set_usize (pref_basic_update_spin, 80, -2);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (pref_basic_update_spin), TRUE);
  gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (pref_basic_update_spin), GTK_UPDATE_IF_VALID);

  pref_basic_update_sec_lbl = gtk_label_new (_("(sec)"));
  gtk_widget_show (pref_basic_update_sec_lbl);
  gtk_box_pack_start (GTK_BOX (pref_basic_update_hbox), pref_basic_update_sec_lbl, FALSE, FALSE, 0);

  pref_basic_note_lbl = gtk_label_new (_("Basic"));
  gtk_widget_show (pref_basic_note_lbl);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (pref_notebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (pref_notebook), 0), pref_basic_note_lbl);

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

  pref_net_proxy_btn = gtk_check_button_new_with_label (_("Use proxy"));
  gtk_widget_show (pref_net_proxy_btn);
  gtk_container_add (GTK_CONTAINER (pref_net_proxy_alignment), pref_net_proxy_btn);

  pref_net_proxy_url_lbl = gtk_label_new (_("Proxy host:"));
  gtk_widget_show (pref_net_proxy_url_lbl);
  gtk_table_attach (GTK_TABLE (pref_net_table), pref_net_proxy_url_lbl, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (pref_net_proxy_url_lbl), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (pref_net_proxy_url_lbl), 1, 0.5);

  pref_net_proxy_url_entry = gtk_entry_new ();
  gtk_widget_show (pref_net_proxy_url_entry);
  gtk_table_attach (GTK_TABLE (pref_net_table), pref_net_proxy_url_entry, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  pref_net_proxy_user_lbl = gtk_label_new (_("Username:"));
  gtk_widget_show (pref_net_proxy_user_lbl);
  gtk_table_attach (GTK_TABLE (pref_net_table), pref_net_proxy_user_lbl, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (pref_net_proxy_user_lbl), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (pref_net_proxy_user_lbl), 1, 0.5);

  pref_net_proxy_user_entry = gtk_entry_new ();
  gtk_widget_show (pref_net_proxy_user_entry);
  gtk_table_attach (GTK_TABLE (pref_net_table), pref_net_proxy_user_entry, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  pref_net_proxy_passwd_lbl = gtk_label_new (_("Password:"));
  gtk_widget_show (pref_net_proxy_passwd_lbl);
  gtk_table_attach (GTK_TABLE (pref_net_table), pref_net_proxy_passwd_lbl, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (pref_net_proxy_passwd_lbl), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (pref_net_proxy_passwd_lbl), 1, 0.5);

  pref_net_proxy_passwd_entry = gtk_entry_new ();
  gtk_entry_set_visibility (GTK_ENTRY (pref_net_proxy_passwd_entry), FALSE);
  gtk_widget_show (pref_net_proxy_passwd_entry);
  gtk_table_attach (GTK_TABLE (pref_net_table), pref_net_proxy_passwd_entry, 1, 2, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  pref_net_caveat_lbl = gtk_label_new (_("Caveat warning: Even though your password will\nbe saved in the private configuration file, it will\nbe unencrypted!"));
  gtk_widget_show (pref_net_caveat_lbl);
  gtk_table_attach (GTK_TABLE (pref_net_table), pref_net_caveat_lbl, 0, 2, 4, 5,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (pref_net_caveat_lbl), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (pref_net_caveat_lbl), 1, 0.5);

  pref_net_note_lbl = gtk_label_new (_("Network"));
  gtk_widget_show (pref_net_note_lbl);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (pref_notebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (pref_notebook), 1), pref_net_note_lbl);

  pref_loc_hbox = gtk_hbox_new (FALSE, 2);
  gtk_widget_show (pref_loc_hbox);
  gtk_container_add (GTK_CONTAINER (pref_notebook), pref_loc_hbox);

  pref_loc_adj = gtk_adjustment_new(0, 0, 0, 0, 0, 0);

  pref_loc_ctree = gtk_ctree_new (1, 0);
  gtk_widget_show (pref_loc_ctree);
  gtk_box_pack_start (GTK_BOX (pref_loc_hbox), pref_loc_ctree, TRUE, TRUE, 0);
  gtk_clist_set_column_width (GTK_CLIST (pref_loc_ctree), 0, 80);
  gtk_clist_set_selection_mode (GTK_CLIST (pref_loc_ctree), GTK_SELECTION_BROWSE);
  gtk_clist_column_titles_hide (GTK_CLIST (pref_loc_ctree));
  gtk_clist_set_vadjustment (GTK_CLIST (pref_loc_ctree), GTK_ADJUSTMENT (pref_loc_adj));
  load_locations();

  pref_loc_scroll = gtk_vscrollbar_new (GTK_ADJUSTMENT (pref_loc_adj));
  gtk_widget_show (pref_loc_scroll);
  gtk_box_pack_start (GTK_BOX (pref_loc_hbox), pref_loc_scroll, FALSE, FALSE, 0);

  pref_loc_note_lbl = gtk_label_new (_("Location"));
  gtk_widget_show (pref_loc_note_lbl);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (pref_notebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (pref_notebook), 2), pref_loc_note_lbl);

  pref_action_area = GNOME_DIALOG (pref)->action_area;
  gtk_widget_show (pref_action_area);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (pref_action_area), GTK_BUTTONBOX_END);
  gtk_button_box_set_spacing (GTK_BUTTON_BOX (pref_action_area), 8);

  gnome_dialog_append_button (GNOME_DIALOG (pref), GNOME_STOCK_BUTTON_OK);
  ok_button = g_list_last (GNOME_DIALOG (pref)->buttons)->data;
  gtk_widget_show (ok_button);
  GTK_WIDGET_SET_FLAGS (ok_button, GTK_CAN_DEFAULT);

  gnome_dialog_append_button (GNOME_DIALOG (pref), GNOME_STOCK_BUTTON_APPLY);
  apply_button = g_list_last (GNOME_DIALOG (pref)->buttons)->data;
  gtk_widget_show (apply_button);
  GTK_WIDGET_SET_FLAGS (apply_button, GTK_CAN_DEFAULT);

  gnome_dialog_append_button (GNOME_DIALOG (pref), GNOME_STOCK_BUTTON_CLOSE);
  cancel_button = g_list_last (GNOME_DIALOG (pref)->buttons)->data;
  gtk_widget_show (cancel_button);
  GTK_WIDGET_SET_FLAGS (cancel_button, GTK_CAN_DEFAULT);

  gnome_dialog_append_button (GNOME_DIALOG (pref), GNOME_STOCK_BUTTON_HELP);
  help_button = g_list_last (GNOME_DIALOG (pref)->buttons)->data;
  gtk_widget_show (help_button);
  GTK_WIDGET_SET_FLAGS (help_button, GTK_CAN_DEFAULT);

  gtk_signal_connect (GTK_OBJECT (pref_loc_ctree), "tree_select_row",
                      GTK_SIGNAL_FUNC (tree_select_row_cb), NULL);
  gtk_signal_connect (GTK_OBJECT (ok_button), "clicked",
                      GTK_SIGNAL_FUNC (ok_cb), NULL);
  gtk_signal_connect (GTK_OBJECT (apply_button), "clicked",
                      GTK_SIGNAL_FUNC (apply_cb), NULL);
  gtk_signal_connect (GTK_OBJECT (pref_basic_update_spin), "changed",
                      GTK_SIGNAL_FUNC (change_cb), NULL);
  gtk_signal_connect (GTK_OBJECT (pref_basic_metric_btn), "toggled",
                      GTK_SIGNAL_FUNC (change_cb), NULL);
  gtk_signal_connect (GTK_OBJECT (pref_basic_detailed_btn), "toggled",
                      GTK_SIGNAL_FUNC (change_cb), NULL);
#ifdef RADARMAP
  gtk_signal_connect (GTK_OBJECT (pref_basic_radar_btn), "toggled",
                      GTK_SIGNAL_FUNC (change_cb), NULL);
#endif /* RADARMAP */
  gtk_signal_connect (GTK_OBJECT (pref_basic_update_btn), "toggled",
                      GTK_SIGNAL_FUNC (change_cb), NULL);
  gtk_signal_connect (GTK_OBJECT (pref_net_proxy_btn), "toggled",
                      GTK_SIGNAL_FUNC (change_cb), NULL);
  gtk_signal_connect (GTK_OBJECT (pref_net_proxy_url_entry), "changed",
                      GTK_SIGNAL_FUNC (change_cb), NULL);
  gtk_signal_connect (GTK_OBJECT (pref_net_proxy_user_entry), "changed",
                      GTK_SIGNAL_FUNC (change_cb), NULL);
  gtk_signal_connect (GTK_OBJECT (pref_net_proxy_passwd_entry), "changed",
                      GTK_SIGNAL_FUNC (change_cb), NULL);
}


void gweather_pref_load (const gchar *path)
{
    gchar *prefix;
    g_return_if_fail(path != NULL);

    prefix = g_strconcat (path, "Preferences/", NULL);
    gnome_config_push_prefix(prefix);
    /* fprintf(stderr, "gweather_pref_load: %s\n", prefix); */
    g_free(prefix);

    gweather_pref.update_interval = gnome_config_get_int("update_interval=1800");
    gweather_pref.update_enabled = gnome_config_get_bool("update_enabled=TRUE");
    gweather_pref.use_metric = gnome_config_get_bool("use_metric=FALSE");
    gweather_pref.detailed = gnome_config_get_bool("detailed=FALSE");
    gweather_pref.radar_enabled = gnome_config_get_bool("radar_enabled=FALSE");
    gweather_pref.location = weather_location_config_read("location");
    gweather_pref.proxy_url = gnome_config_get_string("proxy_url");
    gweather_pref.proxy_user = gnome_config_get_string("proxy_user");
    gweather_pref.proxy_passwd = gnome_config_private_get_string("proxy_passwd");
    gweather_pref.use_proxy = gnome_config_get_bool("use_proxy=FALSE");

    gnome_config_pop_prefix();
}

void gweather_pref_save (const gchar *path)
{
    gchar *prefix;

    g_return_if_fail(path != NULL);

    prefix = g_strconcat (path, "Preferences/", NULL);
    gnome_config_push_prefix(prefix);
    /* fprintf(stderr, "gweather_pref_save: %s\n", prefix); */
    g_free(prefix);

    gnome_config_set_int("update_interval", gweather_pref.update_interval);
    gnome_config_set_bool("update_enabled", gweather_pref.update_enabled);
    gnome_config_set_bool("use_metric", gweather_pref.use_metric);
    gnome_config_set_bool("detailed", gweather_pref.detailed);
    gnome_config_set_bool("radar_enabled", gweather_pref.radar_enabled);
    weather_location_config_write("location", gweather_pref.location);
    gnome_config_set_string("proxy_url", gweather_pref.proxy_url);
    gnome_config_set_string("proxy_user", gweather_pref.proxy_user);
    gnome_config_private_set_string("proxy_passwd", gweather_pref.proxy_passwd);
    gnome_config_set_bool("use_proxy", gweather_pref.use_proxy);

    gnome_config_pop_prefix();

    gnome_config_sync();
    gnome_config_drop_all();
}

void gweather_pref_run (void)
{
    gint btn;

    if (!pref)
        gweather_pref_create();

    update_dialog();

    do {
        gnome_dialog_set_sensitive(GNOME_DIALOG(pref), 0, FALSE);
        gnome_dialog_set_sensitive(GNOME_DIALOG(pref), 1, FALSE);
        btn = gnome_dialog_run(GNOME_DIALOG(pref));
        if (btn == 1) {  /* Apply */
            applet_widget_sync_config(APPLET_WIDGET(gweather_applet));
            gweather_update();
        } else if (btn == 3) { /* help */
	    gnome_help_display(NULL, &help_entry);
	}
    } while (btn == 1 || btn == 3);

    gtk_widget_hide(pref);

    if (btn == 0) {  /* OK */
        applet_widget_sync_config(APPLET_WIDGET(gweather_applet));
        gweather_update();
    }

    gtk_widget_destroy(pref);
    pref = NULL;
}

