/* GNOME cpuload/memload panel applet
 * (C) 1997 The Free Software Foundation
 *
 * Authors: Tim P. Gerla
 *          Martin Baulig
 *		  Todd Kulesza
 *
 * With code from wmload.c, v0.9.2, apparently by Ryan Land, rland@bc1.com.
 *
 */

#include <config.h>
#include <stdio.h>
#include <config.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <gnome.h>
#include <panel-applet.h>

#include <gtk/gtk.h>
#include <libgnomeui/libgnomeui.h>
#include <libgnome/libgnome.h>
#include <libgnomeui/gnome-window-icon.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <libart_lgpl/libart.h>
#include <bonobo/bonobo-shlib-factory.h>

#include "global.h"

static void
about_cb (BonoboUIComponent *uic, gpointer data, const gchar *name)
{
    static GtkWidget *about = NULL;
    static const gchar *authors[] =
	{
		"Martin Baulig <martin@home-of-linux.org>",
		"Todd Kulesza <fflewddur@dropline.net>\n",
		NULL
	};

    if (about != NULL)
	{
	    gdk_window_show(about->window);
	    gdk_window_raise(about->window);
	    return;
	}

    about = gnome_about_new
	(_("Average Load Applet"), VERSION,
	 "(C) 1999, 2001",
	 _("Released under the GNU general public license.\n\n"
	   "Average Load Indicator."),
	 authors,
	 NULL,
	 NULL,
	 NULL);

    gtk_signal_connect (GTK_OBJECT (about), "destroy",
			GTK_SIGNAL_FUNC (gtk_widget_destroyed), &about);

    gtk_widget_show (about);
}

static const char *loadavg_texts [3] = {
    N_("Load average over 1 minute"),
    N_("Load average over 5 minutes"),
    N_("Load average over 15 minutes")
};

static const gchar *loadavg_texts_main [2] = {
    N_("Used"), N_("Free")
};

static const gchar *loadavg_color_defs [2] = {
    "#cfff5fff5fff", "#00008fff0000"
};

static const gchar avgload_menu_xml [] =
        "<popup name=\"button3\">\n"
        "	<menuitem name=\"Properties Item\" verb=\"AvgLoadProperties\" _label=\"Properties ...\"\n"
        "		pixtype=\"stock\" pixnam=\"gtk-properties\"/>\n"
        "	<menuitem name=\"Procman Item\" verb=\"AvgLoadRunProcman\" _label=\"Run Procman ...\"\n"
        "		pixtype=\"stock\" pixname=\"gnome-stock-run\"/>\n"
        "	<menuitem name=\"Help Item\" verb=\"AvgLoadHelp\" _label=\"Help\"\n"
        "		pixtype=\"stock\" pixname=\"gtk-help\"/>\n"
        "	<menuitem name=\"About Item\" verb=\"AvgLoadAbout\" _label=\"About ...\"\n"
        "		pixtype=\"stock\" pixname=\"gnome-stock-about\"/>\n"
        "</popup>\n";

static const BonoboUIVerb avgload_menu_verbs [] = {
		BONOBO_UI_VERB ("AvgLoadProperties", NULL),
		BONOBO_UI_VERB ("AvgLoadRunProcman", start_procman_cb),
        BONOBO_UI_VERB_DATA ("AvgLoadHelp", multiload_help_cb, "index.html#LOADAVG-PROPERTIES"),
        BONOBO_UI_VERB ("AvgLoadAbout", about_cb),

        BONOBO_UI_VERB_END
};
/*
static void
loadavg_1_cb (AppletWidget *widget, gpointer data)
{
    LoadGraph *g = (LoadGraph *) data;

    g->prop_data->loadavg_type = LOADAVG_1;

    gnome_property_object_list_walk
	(g->local_prop_data->local_property_object_list,
	 GNOME_PROPERTY_ACTION_UPDATE);

    gnome_property_object_list_walk
	(g->local_prop_data->local_property_object_list,
	 GNOME_PROPERTY_ACTION_SAVE);

    applet_widget_set_tooltip (g->applet, _(loadavg_texts [0]));
}

static void
loadavg_5_cb (AppletWidget *widget, gpointer data)
{
    LoadGraph *g = (LoadGraph *) data;

    g->prop_data->loadavg_type = LOADAVG_5;

    gnome_property_object_list_walk
	(g->local_prop_data->local_property_object_list,
	 GNOME_PROPERTY_ACTION_UPDATE);

    gnome_property_object_list_walk
	(g->local_prop_data->local_property_object_list,
	 GNOME_PROPERTY_ACTION_SAVE);

    applet_widget_set_tooltip (g->applet, _(loadavg_texts [1]));
}

static void
loadavg_15_cb (AppletWidget *widget, gpointer data)
{
    LoadGraph *g = (LoadGraph *) data;

    g->prop_data->loadavg_type = LOADAVG_15;

    gnome_property_object_list_walk
	(g->local_prop_data->local_property_object_list,
	 GNOME_PROPERTY_ACTION_UPDATE);

    gnome_property_object_list_walk
	(g->local_prop_data->local_property_object_list,
	 GNOME_PROPERTY_ACTION_SAVE);

    applet_widget_set_tooltip (g->applet, _(loadavg_texts [2]));
}
*/

/* start a new instance of the loadavg applet */
gboolean
loadavg_applet_new(PanelApplet *applet)
{
    LoadGraphProperties *prop_data;
    LoadGraph *g;
    
    multiload_properties.loadavg.n = 2;
    multiload_properties.loadavg.name = "loadavg";
#ifdef ENABLE_NLS
    {
        int i;
        for (i=0;i<2;i++) loadavg_texts_main[i]=_(loadavg_texts_main[i]);
    }
#endif
    multiload_properties.loadavg.texts = loadavg_texts_main;
    multiload_properties.loadavg.color_defs = loadavg_color_defs;
    multiload_properties.loadavg.adj_data [0] = 500;
    multiload_properties.loadavg.adj_data [1] = 40;
    multiload_properties.loadavg.adj_data [2] = 10;

    prop_data = g_memdup (&multiload_properties.loadavg,
			  sizeof (LoadGraphProperties));

    g = load_graph_new (applet, 2, N_("Load Average"),
			&multiload_properties.loadavg, prop_data,
			multiload_properties.loadavg.adj_data[0],
			multiload_properties.loadavg.adj_data[1], GetLoadAvg);

	panel_applet_setup_menu (PANEL_APPLET (g->applet), avgload_menu_xml, avgload_menu_verbs, NULL);	

	load_graph_start(g);
	
	return TRUE;
}
