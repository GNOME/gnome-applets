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
#include <egg-screen-exec.h>

#include "global.h"

static void
about_cb (BonoboUIComponent *uic,
	  MultiloadApplet   *ma,
	  const char        *name)
{
    static GtkWidget *about = NULL;
    GdkPixbuf        *pixbuf;
    GError           *error = NULL;
    gchar            *file;
   
    static const gchar *authors[] =
	{
		"Martin Baulig <martin@home-of-linux.org>",
		"Todd Kulesza <fflewddur@dropline.net>\n",
		NULL
	};

    const gchar *documenters[] =
	{
		NULL
	};

    const gchar *translator_credits = _("translator_credits");

    if (about) {
	gtk_window_set_screen (GTK_WINDOW (about),
			       gtk_widget_get_screen (GTK_WIDGET (ma->applet)));
	gtk_window_present (GTK_WINDOW (about));
	return;
    }
	
    file = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_PIXMAP, "gnome-monitor.png", FALSE, NULL);
    pixbuf = gdk_pixbuf_new_from_file (file, &error);
    g_free (file);
   
    if (error) 
    	{
   	    g_warning (G_STRLOC ": cannot open %s: %s", file, error->message);
	    g_error_free (error);
   	}

	
    about = gnome_about_new
	(_("System Monitor"), VERSION,
	 "(C) 1999 - 2002 The Free Software Foundation",
	 _("Released under the GNU General Public License.\n\n"
	   "A system load monitor capable of displaying graphs for CPU, ram, and swap space use, plus network traffic."),
	 authors,
	 documenters,
	 strcmp (translator_credits, "translator_credits") != 0 ? translator_credits : NULL,
	 pixbuf);

    if (pixbuf) 
   	gdk_pixbuf_unref (pixbuf);
  
    gtk_window_set_screen (GTK_WINDOW (about),
   			   gtk_widget_get_screen (GTK_WIDGET (ma->applet)));
    gtk_window_set_wmclass (GTK_WINDOW (about), "system monitor", "System Monitor");
    g_signal_connect (G_OBJECT (about), "destroy",
			G_CALLBACK (gtk_widget_destroyed), &about);

    gtk_widget_show (about);
}

/* run the full-scale system process monitor */

static void
start_procman_cb (BonoboUIComponent *uic,
		  MultiloadApplet   *ma,
		  const char        *name)
{
	GError *error = NULL;

	egg_screen_execute_command_line_async (
			gtk_widget_get_screen (GTK_WIDGET (ma->applet)),
			"gnome-system-monitor", &error);
	if (error) {
		GtkWidget *dialog;

		dialog = gtk_message_dialog_new (NULL,
						 GTK_DIALOG_DESTROY_WITH_PARENT,
						 GTK_MESSAGE_ERROR,
						 GTK_BUTTONS_CLOSE,
						 _("There was an error executing '%s' : %s"),
						 "gnome-system-monitor",
						 error->message);

		g_signal_connect (dialog, "response",
				  G_CALLBACK (gtk_widget_destroy),
				  NULL);

		gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
		gtk_window_set_screen (GTK_WINDOW (dialog),
				       gtk_widget_get_screen (GTK_WIDGET (ma->applet)));

		gtk_widget_show (dialog);

		g_error_free (error);
	}
}
              
void
multiload_change_size_cb(PanelApplet *applet, gint size, gpointer data)
{
	gint i;
	MultiloadApplet *ma = (MultiloadApplet *)data;

	for (i = 0; i < NGRAPHS; i++) {
		ma->graphs[i]->pixel_size = size;
  	  	if (ma->graphs[i]->orient) 
  	  		gtk_widget_set_size_request (ma->graphs[i]->main_widget, 
					             ma->graphs[i]->pixel_size, 
						     ma->graphs[i]->size);
		else
			gtk_widget_set_size_request (ma->graphs[i]->main_widget, 
							     ma->graphs[i]->size, 
							     ma->graphs[i]->pixel_size);
	}
	
	return;
}

void
multiload_change_orient_cb(PanelApplet *applet, gint arg1, gpointer data)
{
	MultiloadApplet *ma = data;
	multiload_applet_refresh((MultiloadApplet *)data);
	gtk_widget_show (GTK_WIDGET (ma->applet));		
	return;
}

void
multiload_destroy_cb(GtkWidget *widget, gpointer data)
{
	gint i;
	MultiloadApplet *ma = data;
	
	for (i = 0; i < NGRAPHS; i++)
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
	
	for (i = 0; i < NGRAPHS; i++)
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

	for (i = 0; i < NGRAPHS; i++)
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

	g_return_if_fail(g);
	g_return_if_fail(g->name);
		
	total_used = 0;

	/* label the tooltip intuitively */
	if (!strncmp(g->name, "cpuload", strlen("cpuload")))
		name = g_strdup(_("Processor"));
	else if (!strncmp(g->name, "memload", strlen("memload")))
		name = g_strdup(_("Memory"));
	else if (!strncmp(g->name, "netload", strlen("netload")))
		name = g_strdup(_("Network"));
	else if (!strncmp(g->name, "swapload", strlen("swapload")))
		name = g_strdup(_("Swap Space"));
	else if (!strncmp(g->name, "loadavg", strlen("loadavg")))
		name = g_strdup(_("Load Average"));
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
	for (i = 0; i < NGRAPHS; i++)
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
	
	if ( (orientation == PANEL_APPLET_ORIENT_UP) || 
	     (orientation == PANEL_APPLET_ORIENT_DOWN) ) {
	     	ma->box = gtk_hbox_new(FALSE, 0);
	}
	else
		ma->box = gtk_vbox_new(FALSE, 0);
	
	gtk_container_add(GTK_CONTAINER(ma->applet), ma->box);
			
	/* create the NGRAPHS graphs, passing in their user-configurable properties with gconf. */
	
	ma->graphs[0] = cpuload_applet_new(ma->applet, NULL);
	ma->graphs[1] = memload_applet_new(ma->applet, NULL);
	ma->graphs[2] = netload_applet_new(ma->applet, NULL);
	ma->graphs[3] = swapload_applet_new(ma->applet, NULL);
	ma->graphs[4] = loadavg_applet_new(ma->applet, NULL);
	
	/* only start and display the graphs the user has turned on */

	for (i = 0; i < NGRAPHS; i++) {
	    gtk_box_pack_start(GTK_BOX(ma->box), 
			       ma->graphs[i]->main_widget, 
			       FALSE, FALSE, 1);
	    if (ma->graphs[i]->visible) {
	    	gtk_widget_show_all (ma->graphs[i]->main_widget);
		load_graph_start(ma->graphs[i]);
	    }
	}
	gtk_widget_show (ma->box);
			
	return;
}

static const BonoboUIVerb multiload_menu_verbs [] = {
	BONOBO_UI_UNSAFE_VERB ("MultiLoadProperties", multiload_properties_cb),
	BONOBO_UI_UNSAFE_VERB ("MultiLoadRunProcman", start_procman_cb),
	BONOBO_UI_UNSAFE_VERB ("MultiLoadAbout", about_cb),

	BONOBO_UI_VERB_END
};		

/* create a box and stuff the load graphs inside of it */
gboolean
multiload_applet_new(PanelApplet *applet, const gchar *iid, gpointer data)
{
	gint i;
	GtkWidget *box;
	PanelAppletOrient orientation;
	MultiloadApplet *ma;
	gboolean visible = FALSE;
	
	ma = g_new0(MultiloadApplet, 1);
	
	ma->applet = applet;
	
	gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gnome-monitor.png");
	
	panel_applet_add_preferences (applet, "/schemas/apps/multiload/prefs", NULL);
	
	orientation = panel_applet_get_orient(applet);
	
	if ( (orientation == PANEL_APPLET_ORIENT_UP) || (orientation == PANEL_APPLET_ORIENT_DOWN) )
		box = gtk_hbox_new(FALSE, 0);
	else
		box = gtk_vbox_new(FALSE, 0);
	
	gtk_container_add(GTK_CONTAINER(applet), box);
	
	/* create the NGRAPHS graphs, passing in their user-configurable properties with gconf. */
	ma->graphs[0] = cpuload_applet_new(applet, NULL);
	ma->graphs[1] = memload_applet_new(applet, NULL);
	ma->graphs[2] = netload_applet_new(applet, NULL);
	ma->graphs[3] = swapload_applet_new(applet, NULL);
	ma->graphs[4] = loadavg_applet_new(applet, NULL);

	/* only start and display the graphs the user has turned on */
	for (i = 0; i < NGRAPHS; i++) {
	    gtk_box_pack_start(GTK_BOX(box), 
			       ma->graphs[i]->main_widget, 
			       FALSE, FALSE, 1);
	    if (ma->graphs[i]->visible) {
	    	gtk_widget_show_all (ma->graphs[i]->main_widget);
		load_graph_start(ma->graphs[i]);
		visible = TRUE;
	    }
	}
	
	if (!visible) {
		/* No graphs shown - need to show at least one so show cpu*/
		ma->graphs[0]->visible = TRUE;
		gtk_widget_show_all (ma->graphs[0]->main_widget);
		load_graph_start(ma->graphs[0]);
		panel_applet_gconf_set_bool(ma->applet, "view_cpuload", 
			                    TRUE, NULL);
	}

	panel_applet_setup_menu_from_file (applet,
					   NULL,
					   "GNOME_MultiloadApplet.xml",
					   NULL,
					   multiload_menu_verbs,
					  ma);	

	ma->box = box;
	
	g_signal_connect(G_OBJECT(applet), "change_size",
				G_CALLBACK(multiload_change_size_cb), ma);
	g_signal_connect(G_OBJECT(applet), "change_orient",
				G_CALLBACK(multiload_change_orient_cb), ma);
	g_signal_connect(G_OBJECT(applet), "destroy",
				G_CALLBACK(multiload_destroy_cb), ma);
	g_signal_connect(G_OBJECT(applet), "enter_notify_event",
				G_CALLBACK(multiload_enter_cb), ma);
	multiload_change_size_cb (ma->applet,
				  panel_applet_get_size (ma->applet),
				  ma);	
	gtk_widget_show (box);			
	gtk_widget_show(GTK_WIDGET(applet));
			
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
				   PANEL_TYPE_APPLET,
				   "MultiLoad Applet factory",
				   "0",
				   multiload_factory,
				   NULL);
