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
	(_("Memeory Load Applet"), VERSION,
	 "(C) 1999, 2001",
	 _("Released under the GNU general public license.\n\n"
	   "Memory Load Indicator."),
	 authors,
	 NULL,
	 NULL,
	 NULL);

    gtk_signal_connect (GTK_OBJECT (about), "destroy",
			GTK_SIGNAL_FUNC (gtk_widget_destroyed), &about);

    gtk_widget_show (about);
}

static const gchar *mem_texts [4] =  {
    N_("Other"), N_("Shared"), N_("Buffers"), N_("Free")
};

static const gchar *mem_color_defs [4] = {
    "#bfffbfff4fff", "#efffefff4fff",
    "#afffafffafff", "#00008fff0000"
};

static const gchar memload_menu_xml [] =
        "<popup name=\"button3\">\n"
        "	<menuitem name=\"Properties Item\" verb=\"MemLoadProperties\" _label=\"Properties ...\"\n"
        "		pixtype=\"stock\" pixname=\"gtk-properties\"/>\n"
        "	<menuitem name=\"Procman Item\" verb=\"MemLoadRunProcman\" _label=\"Run Procman ...\"\n"
        "		pixtype=\"stock\" pixname=\"gnome-stock-run\"/>\n"
        "	<menuitem name=\"Help Item\" verb=\"MemLoadHelp\" _label=\"Help\"\n"
        "		pixtype=\"stock\" pixname=\"gtk-help\"/>\n"
        "	<menuitem name=\"About Item\" verb=\"MemLoadAbout\" _label=\"About ...\"\n"
        "		pixtype=\"stock\" pixname=\"gnome-stock-about\"/>\n"
        "</popup>\n";

static const BonoboUIVerb memload_menu_verbs [] = {
		BONOBO_UI_VERB ("MemLoadProperties", NULL),
		BONOBO_UI_VERB ("MemLoadRunProcman", start_procman_cb),
        BONOBO_UI_VERB_DATA ("MemLoadHelp", multiload_help_cb, "index.html#MEMLOAD-PROPERTIES"),
        BONOBO_UI_VERB ("MemLoadAbout", about_cb),

        BONOBO_UI_VERB_END
};

/* start a new instance of the memload applet */
gboolean
memload_applet_new(PanelApplet *applet)
{
    LoadGraphProperties *prop_data;
    LoadGraph *g;

	/* Setup Properties */
	
	multiload_properties.memload.n = 4;
    multiload_properties.memload.name = "memload";
#ifdef ENABLE_NLS
    {
        int i;
        for (i=0;i<4;i++) mem_texts[i]=_(mem_texts[i]);
    }
#endif
    multiload_properties.memload.texts = mem_texts;
    multiload_properties.memload.color_defs = mem_color_defs;
    multiload_properties.memload.adj_data [0] = 500;
    multiload_properties.memload.adj_data [1] = 40;

    prop_data = g_memdup (&multiload_properties.memload,
			  sizeof (LoadGraphProperties));

    g = load_graph_new (applet, 4, N_("Memory Load"),
			&multiload_properties.memload, prop_data,
			multiload_properties.memload.adj_data[0],
			multiload_properties.memload.adj_data[1], GetMemory);

    panel_applet_setup_menu (PANEL_APPLET (g->applet), memload_menu_xml, memload_menu_verbs, NULL);	

	load_graph_start(g);
	
	return TRUE;
}
