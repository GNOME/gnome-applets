/* GNOME multiload panel applet
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
#include <panel-applet-gconf.h>

#include <gtk/gtk.h>
#include <libgnomeui/libgnomeui.h>
#include <libgnome/libgnome.h>
#include <libgnomeui/gnome-window-icon.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <libart_lgpl/libart.h>

#include "global.h"

MultiloadApplet *multiload_applet;

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
	(_("System Load Applet"), VERSION,
	 "(C) 1999 - 2002",
	 _("Released under the GNU general public license.\n\n"
	   "A system load monitor capable of displaying graphs for cpu, ram, and swap file use, plus network traffic."),
	 authors,
	 NULL,
	 NULL,
	 NULL);

    g_signal_connect (G_OBJECT (about), "destroy",
			G_CALLBACK (gtk_widget_destroyed), &about);

    gtk_widget_show (about);
}

static const gchar multiload_menu_xml [] =
        "<popup name=\"button3\">\n"
        "	<menuitem name=\"Properties Item\" verb=\"MultiLoadProperties\" _label=\"Properties ...\"\n"
        "		pixtype=\"stock\" pixname=\"gtk-properties\"/>\n"
        "	<menuitem name=\"Procman Item\" verb=\"MultiLoadRunProcman\" _label=\"Run Procman ...\"\n"
        "		pixtype=\"stock\" pixname=\"gtk-execute\"/>\n"
        "	<menuitem name=\"Help Item\" verb=\"MultiLoadHelp\" _label=\"Help\"\n"
        "		pixtype=\"stock\" pixname=\"gtk-help\"/>\n"
        "	<menuitem name=\"About Item\" verb=\"MultiLoadAbout\" _label=\"About ...\"\n"
        "		pixtype=\"stock\" pixname=\"gnome-stock-about\"/>\n"
        "</popup>\n";

static const BonoboUIVerb multiload_menu_verbs [] = {
		BONOBO_UI_VERB ("MultiLoadProperties", multiload_properties_cb),
		BONOBO_UI_VERB ("MultiLoadRunProcman", start_procman_cb),
        BONOBO_UI_VERB_DATA ("MultiLoadHelp", multiload_help_cb, "index.html"),
        BONOBO_UI_VERB ("MultiLoadAbout", about_cb),

        BONOBO_UI_VERB_END
};

/* run the full-scale system process monitor */

void
start_procman_cb (BonoboUIComponent *uic, gpointer data, const gchar *name)
{
	gnome_execute_shell(NULL, "procman");
	return;
}
              
/* show help for the applet */

void
multiload_help_cb (BonoboUIComponent *uic, gpointer data, const gchar *name)
{
    gnome_help_display(data, NULL, NULL);
    return;
}

/* FIXME: i'm not sure if this is setup wrong or libpanel is broken.  this doesn't work at all. */

void
multiload_applet_destroy(GtkWidget *widget, gpointer data)
{
	gint i;
	MultiloadApplet *ma = data;
	
	for (i = 0; i < 5; i++)
		if (ma->graphs[i]->visible)
			load_graph_stop(ma->graphs[i]);
			
	return;
}

/* create a box and stuff the load graphs inside of it */

gboolean
multiload_applet_new(PanelApplet *applet, const gchar *iid, gpointer data)
{
	gint i;
	GtkWidget *box;
	PanelAppletOrient orientation;
	MultiloadApplet *ma;
	
	ma = g_new0(MultiloadApplet, 1);
	
	ma->applet = applet;
	
	panel_applet_add_preferences (applet, "/schemas/apps/multiload/prefs", NULL);
	
	orientation = panel_applet_get_orient(applet);
	
	if ( (orientation == PANEL_APPLET_ORIENT_UP) || (orientation == PANEL_APPLET_ORIENT_DOWN) )
		box = gtk_hbox_new(FALSE, 0);
	else
		box = gtk_vbox_new(FALSE, 0);
	
	gtk_container_add(GTK_CONTAINER(applet), box);
	
	/* create the 5 graphs, passing in their user-configurable properties with gconf. */
	ma->graphs[0] = cpuload_applet_new(applet, NULL);
	ma->graphs[1] = memload_applet_new(applet, NULL);
	ma->graphs[2] = netload_applet_new(applet, NULL);
	ma->graphs[3] = swapload_applet_new(applet, NULL);
	ma->graphs[4] = loadavg_applet_new(applet, NULL);
	
	g_signal_connect(G_OBJECT(box), "destroy",
			G_CALLBACK(multiload_applet_destroy), ma);

	/* only start and display the graphs the user has turned on */
	for (i = 0; i < 5; i++)
		if (ma->graphs[i]->visible)
		{
			gtk_box_pack_start(GTK_BOX(box), ma->graphs[i]->main_widget, FALSE, FALSE, 2);
			load_graph_start(ma->graphs[i]);
		}
	
	panel_applet_setup_menu (applet, multiload_menu_xml, multiload_menu_verbs, NULL);	
	
	gtk_widget_show_all(GTK_WIDGET(applet));

	multiload_applet = ma;
			
	return TRUE;
}

static gboolean
multiload_factory (PanelApplet *applet,
				const gchar *iid,
				gpointer data)
{
	gboolean retval = FALSE;
	
	retval = multiload_applet_new(applet, iid, data);
	
	return retval;
}

PANEL_APPLET_BONOBO_FACTORY ("OAFIID:GNOME_MultiLoadApplet_Factory",
				   "MultiLoad Applet factory",
				   "0",
				   multiload_factory,
				   NULL);