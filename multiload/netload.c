/* GNOME cpuload/memload panel applet
 * (C) 1997 The Free Software Foundation
 *
 * Authors: Eric S. Raymond
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
	(_("Net Load Applet"), VERSION,
	 "(C) 1999, 2001",
	 _("Released under the GNU general public license.\n\n"
	   "Net Load Indicator."),
	 authors,
	 NULL,
	 NULL,
	 NULL);

    gtk_signal_connect (GTK_OBJECT (about), "destroy",
			GTK_SIGNAL_FUNC (gtk_widget_destroyed), &about);

    gtk_widget_show (about);
}

static const gchar *net_texts [4] = {
    N_("SLIP"), N_("PPP"), N_("ETH"), N_("Other"),
};

static const gchar *net_color_defs [4] = {
    "#64009500e0e0", "#d300d300d300", 
    "#00008fff0000", "#ffffffff4fff"
};

static const gchar netload_menu_xml [] =
        "<popup name=\"button3\">\n"
        "	<menuitem name=\"Properties Item\" verb=\"NetLoadProperties\" _label=\"Properties ...\"\n"
        "		pixtype=\"stock\" pixname=\"gtk-properties\"/>\n"
        "	<menuitem name=\"Procman Item\" verb=\"NetLoadRunProcman\" _label=\"Run Procman ...\"\n"
        "		pixtype=\"stock\" pixname=\"gnome-stock-run\"/>\n"
        "	<menuitem name=\"Help Item\" verb=\"NetLoadHelp\" _label=\"Help\"\n"
        "		pixtype=\"stock\" pixname=\"gtk-help\"/>\n"
        "	<menuitem name=\"About Item\" verb=\"NetLoadAbout\" _label=\"About ...\"\n"
        "		pixtype=\"stock\" pixname=\"gnome-stock-about\"/>\n"
        "</popup>\n";

static const BonoboUIVerb netload_menu_verbs [] = {
		BONOBO_UI_VERB ("NetLoadProperties", NULL),
		BONOBO_UI_VERB ("NetLoadRunProcman", start_procman_cb),
        BONOBO_UI_VERB_DATA ("NetLoadHelp", multiload_help_cb, "index.html#NETLOAD-PROPERTIES"),
        BONOBO_UI_VERB ("NetLoadAbout", about_cb),

        BONOBO_UI_VERB_END
};

/* start a new instance of the netload applet */
gboolean
netload_applet_new(PanelApplet *applet)
{
    LoadGraphProperties *prop_data;
    LoadGraph *g;

    multiload_properties.netload.n = 4;
    multiload_properties.netload.name = "netload";
#ifdef ENABLE_NLS
    {
        int i;
        for (i=0;i<3;i++) net_texts[i]=_(net_texts[i]);
    }
#endif
    multiload_properties.netload.texts = net_texts;
    multiload_properties.netload.color_defs = net_color_defs;
    multiload_properties.netload.adj_data [0] = 500;
    multiload_properties.netload.adj_data [1] = 40;

    prop_data = g_memdup (&multiload_properties.netload,
			  sizeof (LoadGraphProperties));

    g = load_graph_new (applet, 4, N_("Net Load"),
			&multiload_properties.netload, prop_data,
			multiload_properties.netload.adj_data[0],
			multiload_properties.netload.adj_data[1], GetNet);
	
	panel_applet_setup_menu (PANEL_APPLET (g->applet), netload_menu_xml, netload_menu_verbs, NULL);	

	load_graph_start(g);
	
	return TRUE;
}

