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
	(_("CPU Load Applet"), VERSION,
	 "(C) 1999, 2001",
	 _("Released under the GNU general public license.\n\n"
	   "CPU Load Indicator."),
	 authors,
	 NULL,
	 NULL,
	 NULL);

    gtk_signal_connect (GTK_OBJECT (about), "destroy",
			GTK_SIGNAL_FUNC (gtk_widget_destroyed), &about);

    gtk_widget_show (about);
}

static const gchar *cpu_texts [4] = {
    N_("User"),  N_("System"),   N_("Nice"),  N_("Idle")
};

static const gchar *cpu_color_defs [4] = {
    "#ffffffff4fff", "#dfffdfffdfff",
    "#afffafffafff", "#000000000000"
};

static const gchar cpuload_menu_xml [] =
        "<popup name=\"button3\">\n"
        "	<menuitem name=\"Properties Item\" verb=\"CPULoadProperties\" _label=\"Properties ...\"\n"
        "		pixtype=\"stock\" pixname=\"gtk-properties\"/>\n"
        "	<menuitem name=\"Procman Item\" verb=\"CPULoadRunProcman\" _label=\"Run Procman ...\"\n"
        "		pixtype=\"stock\" pixname=\"gnome-stock-run\"/>\n"
        "	<menuitem name=\"Help Item\" verb=\"CPULoadHelp\" _label=\"Help\"\n"
        "		pixtype=\"stock\" pixname=\"gtk-help\"/>\n"
        "	<menuitem name=\"About Item\" verb=\"CPULoadAbout\" _label=\"About ...\"\n"
        "		pixtype=\"stock\" pixname=\"gnome-stock-about\"/>\n"
        "</popup>\n";

static const BonoboUIVerb cpuload_menu_verbs [] = {
		BONOBO_UI_VERB ("CPULoadProperties", NULL),
		BONOBO_UI_VERB ("CPULoadRunProcman", start_procman_cb),
        BONOBO_UI_VERB_DATA ("CPULoadHelp", multiload_help_cb, "index.html#CPULOAD-PROPERTIES"),
        BONOBO_UI_VERB ("CPULoadAbout", about_cb),

        BONOBO_UI_VERB_END
};

gboolean
cpuload_applet_new(PanelApplet *applet)
{
	LoadGraph *g;
	LoadGraphProperties *prop_data;
	
	g_print("cpuload_applet_new\n");
	
	/* Setup properties. */

    multiload_properties.cpuload.n = 4;
    multiload_properties.cpuload.name = "cpuload";
#ifdef ENABLE_NLS
    {
	int i;
	for (i=0;i<4;i++) cpu_texts[i]=_(cpu_texts[i]);
    }
#endif
    multiload_properties.cpuload.texts = cpu_texts;
    multiload_properties.cpuload.color_defs = cpu_color_defs;
    multiload_properties.cpuload.adj_data [0] = 500;
    multiload_properties.cpuload.adj_data [1] = 40;

	prop_data = g_memdup (&multiload_properties.cpuload, sizeof (LoadGraphProperties));
	
	g = load_graph_new (applet, 4, N_("CPU Load"), &multiload_properties.cpuload, prop_data, 500, 40, GetLoad);
	
	panel_applet_setup_menu (PANEL_APPLET (g->applet), cpuload_menu_xml, cpuload_menu_verbs, NULL);	

	load_graph_start(g);
	
	return TRUE;
}