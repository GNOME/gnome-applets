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

void
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
	(_("System Monitor Applet"), VERSION,
	 "(C) 1999 - 2002 The Free Software Foundation",
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

const gchar multiload_menu_xml [] =
        "<popup name=\"button3\">\n"
        "	<menuitem name=\"Properties Item\" verb=\"MultiLoadProperties\" _label=\"Properties ...\"\n"
        "		pixtype=\"stock\" pixname=\"gtk-properties\"/>\n"
        "	<menuitem name=\"Procman Item\" verb=\"MultiLoadRunProcman\" _label=\"Run Procman ...\"\n"
        "		pixtype=\"stock\" pixname=\"gtk-execute\"/>\n"
        "	<menuitem name=\"About Item\" verb=\"MultiLoadAbout\" _label=\"About ...\"\n"
        "		pixtype=\"stock\" pixname=\"gnome-stock-about\"/>\n"
        "</popup>\n";

/* run the full-scale system process monitor */

void
start_procman_cb (BonoboUIComponent *uic, gpointer data, const gchar *name)
{
	gnome_execute_shell(NULL, "procman");
	return;
}
              
void
multiload_change_size_cb(PanelApplet *applet, gint arg1, gpointer data)
{
	gint i;
	MultiloadApplet *ma = (MultiloadApplet *)data;
	
	for (i = 0; i < 5; i++)
		if (ma->graphs[i]->visible)
		{
			ma->graphs[i]->pixel_size = panel_applet_get_size (applet);
    
  	  	if (ma->graphs[i]->orient)
				gtk_widget_set_size_request (ma->graphs[i]->main_widget, ma->graphs[i]->pixel_size, ma->graphs[i]->size);
			else
				gtk_widget_set_size_request (ma->graphs[i]->main_widget, ma->graphs[i]->size, ma->graphs[i]->pixel_size);
		}
		
	return;
}

void
multiload_change_orient_cb(PanelApplet *applet, gint arg1, gpointer data)
{
	multiload_applet_refresh((MultiloadApplet *)data);
		
	return;
}

/* FIXME: i'm not sure if this is setup wrong or libpanel is broken.  this doesn't work at all. */
void
multiload_destroy_cb(GtkWidget *widget, gpointer data)
{
	gint i;
	MultiloadApplet *ma = data;
	
	for (i = 0; i < 5; i++)
	{
		if (ma->graphs[i]->visible)
		{
			load_graph_stop(ma->graphs[i]);
			gtk_widget_destroy(ma->graphs[i]->main_widget);
		}
		
		load_graph_unalloc(ma->graphs[i]);
		g_free(ma->graphs[i]);
	}
	
	gtk_widget_destroy(GTK_WIDGET(ma->applet));
			
	return;
}

gboolean
multiload_enter_cb(GtkWidget *widget, GdkEventCrossing *event, gpointer data)
{
	MultiloadApplet *ma;
	gint i;
	
	ma = (MultiloadApplet *)data;
	
	for (i = 0; i < 5; i++)
		if (ma->graphs[i]->visible)
		{
			ma->graphs[i]->tooltip_update = TRUE;
			multiload_applet_tooltip_update(ma->graphs[i]);
		}
		
	return TRUE;
}

gboolean
multiload_leave_cb(GtkWidget *widget, GdkEventCrossing *event, gpointer data)
{
	MultiloadApplet *ma;
	gint i;
	
	ma = (MultiloadApplet *)data;
	
	for (i = 0; i < 5; i++)
		if (ma->graphs[i]->visible)
			ma->graphs[i]->tooltip_update = FALSE;
			
	return TRUE;
}

/* update the tooltip to the graph's current "used" percentage */
void
multiload_applet_tooltip_update(LoadGraph *g)
{
	gint i, total_used, percent;
	gchar *tooltip_text, *name;
	
	total_used = 0;

	/* label the tooltip intuitively */
	if (!strncmp(g->name, "cpuload", strlen("cpuload")))
		name = g_strdup(_("Processor"));
	else if (!strncmp(g->name, "memload", strlen("memload")))
		name = g_strdup(_("Memory"));
	else if (!strncmp(g->name, "netload", strlen("netload")))
		name = g_strdup(_("Network"));
	else if (!strncmp(g->name, "swapload", strlen("swapload")))
		name = g_strdup(_("Swap File"));
	else
		name = g_strdup(_("Resource"));
	
	/* fill data[0] with the current load */
	g->get_data (g->draw_height, g->data[0], g);
	
	for (i = 0; i < (g->n - 1); i++)
		total_used += g->data[0][i];
	/* netload has a fifth data structute */
	if (!strncmp(g->name, "netload", strlen("netload")))
		total_used += g->data[0][g->n - 1];	
	percent = 100 * (gdouble)total_used / (gdouble)g->draw_height;

	tooltip_text = g_strdup_printf(_("%s:\n%d%% in use"), name, percent);
	gtk_tooltips_set_tip(g->tooltips, g->disp, tooltip_text, tooltip_text);
		
	g_free(tooltip_text);
	g_free(name);
	
	return;
}

/* remove the old graphs and rebuild them */
void
multiload_applet_refresh(MultiloadApplet *ma)
{
	gint i;
	PanelAppletOrient orientation;
		
	/* stop and free the old graphs */
	for (i = 0; i < 5; i++)
	{
		if (ma->graphs[i]->visible)
		{
			load_graph_stop(ma->graphs[i]);
			gtk_widget_destroy(ma->graphs[i]->main_widget);
		}
		
		load_graph_unalloc(ma->graphs[i]);
		g_free(ma->graphs[i]);
	}

	gtk_widget_destroy(ma->box);
	
	orientation = panel_applet_get_orient(ma->applet);
	
	if ( (orientation == PANEL_APPLET_ORIENT_UP) || (orientation == PANEL_APPLET_ORIENT_DOWN) )
		ma->box = gtk_hbox_new(FALSE, 0);
	else
		ma->box = gtk_vbox_new(FALSE, 0);
	
	gtk_container_add(GTK_CONTAINER(ma->applet), ma->box);
			
	/* create the 5 graphs, passing in their user-configurable properties with gconf. */
	
	ma->graphs[0] = cpuload_applet_new(ma->applet, NULL);
	ma->graphs[1] = memload_applet_new(ma->applet, NULL);
	ma->graphs[2] = netload_applet_new(ma->applet, NULL);
	ma->graphs[3] = swapload_applet_new(ma->applet, NULL);
	ma->graphs[4] = loadavg_applet_new(ma->applet, NULL);
	
	/* only start and display the graphs the user has turned on */

	for (i = 0; i < 5; i++)
		if (ma->graphs[i]->visible)
		{
			gtk_box_pack_start(GTK_BOX(ma->box), ma->graphs[i]->main_widget, FALSE, FALSE, 1);
			load_graph_start(ma->graphs[i]);
		}
	
	gtk_widget_show_all(GTK_WIDGET(ma->applet));
		
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

	/* only start and display the graphs the user has turned on */
	for (i = 0; i < 5; i++)
		if (ma->graphs[i]->visible)
		{
			gtk_box_pack_start(GTK_BOX(box), ma->graphs[i]->main_widget, FALSE, FALSE, 1);
			load_graph_start(ma->graphs[i]);
		}

	{
		/* we need to pass 'ma' into the properties_cb or else every instance of multiload will use the same properties dialog*/
		const BonoboUIVerb multiload_menu_verbs [] = {
			BONOBO_UI_VERB_DATA ("MultiLoadProperties", multiload_properties_cb, ma),
			BONOBO_UI_VERB ("MultiLoadRunProcman", start_procman_cb),
	        BONOBO_UI_VERB ("MultiLoadAbout", about_cb),

  	      BONOBO_UI_VERB_END
		};		
		panel_applet_setup_menu (applet, multiload_menu_xml, multiload_menu_verbs, NULL);	
	}

	ma->box = box;
	
	g_signal_connect(G_OBJECT(applet), "change_size",
				G_CALLBACK(multiload_change_size_cb), ma);
	g_signal_connect(G_OBJECT(applet), "change_orient",
				G_CALLBACK(multiload_change_orient_cb), ma);
	g_signal_connect(G_OBJECT(applet), "destroy",
				G_CALLBACK(multiload_destroy_cb), ma);
	g_signal_connect(G_OBJECT(applet), "enter_notify_event",
				G_CALLBACK(multiload_enter_cb), ma);
					
	gtk_widget_show_all(GTK_WIDGET(applet));
			
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