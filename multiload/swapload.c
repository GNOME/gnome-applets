/* GNOME cpuload/memload panel applet
 * (C) 1997 The Free Software Foundation
 *
 * Authors: Tim P. Gerla
 *          Martin Baulig
 *          Todd Kulesza
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
	(_("Swap Load Applet"), VERSION,
	 "(C) 1999, 2001",
	 _("Released under the GNU general public license.\n\n"
	   "Swap Load Indicator."),
	 authors,
	 NULL,
	 NULL,
	 NULL);

    gtk_signal_connect (GTK_OBJECT (about), "destroy",
			GTK_SIGNAL_FUNC (gtk_widget_destroyed), &about);

    gtk_widget_show (about);
}

static const gchar *swap_texts [2] = {
    N_("Used"), N_("Free")
};

static const gchar *swap_color_defs [4] = {
    "#cfff5fff5fff", "#00008fff0000"
};

static const gchar swapload_menu_xml [] =
        "<popup name=\"button3\">\n"
        "	<menuitem name=\"Properties Item\" verb=\"SwapLoadProperties\" _label=\"Properties ...\"\n"
        "		pixtype=\"stock\" pixname=\"gtk-properties\"/>\n"
        "	<menuitem name=\"Procman Item\" verb=\"SwapLoadRunProcman\" _label=\"Run Procman ...\"\n"
        "		pixtype=\"stock\" pixname=\"gnome-stock-run\"/>\n"
        "	<menuitem name=\"Help Item\" verb=\"SwapLoadHelp\" _label=\"Help\"\n"
        "		pixtype=\"stock\" pixname=\"gtk-help\"/>\n"
        "	<menuitem name=\"About Item\" verb=\"SwapLoadAbout\" _label=\"About ...\"\n"
        "		pixtype=\"stock\" pixname=\"gnome-stock-about\"/>\n"
        "</popup>\n";

static const BonoboUIVerb swapload_menu_verbs [] = {
		BONOBO_UI_VERB ("SwapLoadProperties", NULL),
		BONOBO_UI_VERB ("SwapLoadRunProcman", start_procman_cb),
        BONOBO_UI_VERB_DATA ("SwapLoadHelp", multiload_help_cb, "index.html#SWAPLOAD-PROPERTIES"),
        BONOBO_UI_VERB ("SwapLoadAbout", about_cb),

        BONOBO_UI_VERB_END
};

/* start a new instance of the swapload applet */
gboolean
swapload_applet_new(PanelApplet *applet)
{
    LoadGraphProperties *prop_data;
    LoadGraph *g;

	multiload_properties.swapload.n = 2;
    multiload_properties.swapload.name = "swapload";
#ifdef ENABLE_NLS
    {
        int i;
        for (i=0;i<2;i++) swap_texts[i]=_(swap_texts[i]);
    }
#endif
    multiload_properties.swapload.texts = swap_texts;
    multiload_properties.swapload.color_defs = swap_color_defs;
    multiload_properties.swapload.adj_data [0] = 500;
    multiload_properties.swapload.adj_data [1] = 40;
    
    prop_data = g_memdup (&multiload_properties.swapload,
			  sizeof (LoadGraphProperties));

    g = load_graph_new (applet, 2, N_("Swap Load"),
			&multiload_properties.swapload, prop_data,
			multiload_properties.swapload.adj_data[0],
			multiload_properties.swapload.adj_data[1], GetSwap);
	
	panel_applet_setup_menu (PANEL_APPLET (g->applet), swapload_menu_xml, swapload_menu_verbs, NULL);	

	load_graph_start(g);
	
	return TRUE;
}
