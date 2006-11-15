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
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>
#include <string.h>
#include <time.h>

#include <glibtop.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gconf/gconf-client.h>
#include <libgnome/gnome-desktop-item.h> 
#include <libgnomeui/gnome-help.h>
#include <panel-applet.h>
#include <panel-applet-gconf.h>

#include "global.h"

static void
about_cb (BonoboUIComponent *uic,
	  MultiloadApplet   *ma,
	  const char        *name)
{
    const gchar * const authors[] =
    {
		"Martin Baulig <martin@home-of-linux.org>",
		"Todd Kulesza <fflewddur@dropline.net>",
		"Benoît Dejean <TazForEver@dlfp.org>",
		"Davyd Madeley <davyd@madeley.id.au>",
		NULL
    };

    const gchar * const documenters[] =
    {
		"Chee Bin HOH <cbhoh@gnome.org>",
		"Sun GNOME Documentation Team <gdocteam@sun.com>",
		NULL
    };

    gtk_show_about_dialog (NULL,
	"name",		_("System Monitor"),
	"version",	VERSION,
	"copyright",	"\xC2\xA9 1999-2005 Free Software Foundation "
			"and others",
	"comments",	_("A system load monitor capable of displaying graphs "
			"for CPU, ram, and swap space use, plus network "
			"traffic."),
	"authors",	authors,
	"documenters",	documenters,
	"translator-credits",	_("translator-credits"),
	"logo-icon-name",	"utilities-system-monitor",
	NULL);
}

static void
help_cb (BonoboUIComponent *uic,
	  MultiloadApplet   *ma,
	  const char        *name)
{

 	GError *error = NULL;
                                                                                
    	gnome_help_display_on_screen (
                "multiload", NULL,
                gtk_widget_get_screen (GTK_WIDGET (ma->applet)),
                &error);
                                                                                
    	if (error) { /* FIXME: the user needs to see this */
        	g_warning ("help error: %s\n", error->message);
        	g_error_free (error);
        	error = NULL;
    	}


}

/* run the full-scale system process monitor */

static void
start_procman (MultiloadApplet *ma)
{
	GError *error = NULL;
	GnomeDesktopItem *ditem;

	g_return_if_fail (ma != NULL);

	if ((ditem = gnome_desktop_item_new_from_basename ("gnome-system-monitor.desktop", 0, NULL))) {
		gnome_desktop_item_set_launch_time (ditem, gtk_get_current_event_time ());
		gnome_desktop_item_launch_on_screen (ditem, NULL,
		                                     GNOME_DESKTOP_ITEM_LAUNCH_ONLY_ONE,
		                                     gtk_widget_get_screen (GTK_WIDGET (ma->applet)),
		                                     -1, &error);
		gnome_desktop_item_unref (ditem);
	}
	else {	
	     	gdk_spawn_command_line_on_screen (
				gtk_widget_get_screen (GTK_WIDGET (ma->applet)),
				"gnome-system-monitor", &error);
	}

	if (error) {
		GtkWidget *dialog;

		dialog = gtk_message_dialog_new (NULL,
						 GTK_DIALOG_DESTROY_WITH_PARENT,
						 GTK_MESSAGE_ERROR,
						 GTK_BUTTONS_OK,
						 _("There was an error executing '%s': %s"),
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
              
static void
start_procman_cb (BonoboUIComponent *uic,
		  MultiloadApplet   *ma,
		  const char        *name)
{
	start_procman (ma);
}

static void
multiload_change_size_cb(PanelApplet *applet, gint size, gpointer data)
{
	MultiloadApplet *ma = (MultiloadApplet *)data;
	
	multiload_applet_refresh(ma);
	
	return;
}

static void
multiload_change_orient_cb(PanelApplet *applet, gint arg1, gpointer data)
{
	MultiloadApplet *ma = data;
	multiload_applet_refresh((MultiloadApplet *)data);
	gtk_widget_show (GTK_WIDGET (ma->applet));		
	return;
}

static void
multiload_destroy_cb(GtkWidget *widget, gpointer data)
{
	gint i;
	MultiloadApplet *ma = data;

	for (i = 0; i < NGRAPHS; i++)
	{
		load_graph_stop(ma->graphs[i]);
		if (ma->graphs[i]->colors)
		{
			g_free (ma->graphs[i]->colors);
			ma->graphs[i]->colors = NULL;
		}
		gtk_widget_destroy(ma->graphs[i]->main_widget);
	
		load_graph_unalloc(ma->graphs[i]);
		g_free(ma->graphs[i]);
	}
	
	if (ma->about_dialog)
		gtk_widget_destroy (ma->about_dialog);
	
	if (ma->prop_dialog)
		gtk_widget_destroy (ma->prop_dialog);

	gtk_widget_destroy(GTK_WIDGET(ma->applet));

	g_free (ma);

	return;
}

static gboolean
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


static gboolean
multiload_leave_cb(GtkWidget *widget, GdkEventCrossing *event, gpointer data)
{
	MultiloadApplet *ma;
	gint i;

	ma = (MultiloadApplet *)data;

	for (i = 0; i < NGRAPHS; i++)
			ma->graphs[i]->tooltip_update = FALSE;
		
	return TRUE;
}


static gboolean
multiload_button_press_event_cb (GtkWidget *widget, GdkEventButton *event, MultiloadApplet *ma)
{
	g_return_val_if_fail (event != NULL, FALSE);
	g_return_val_if_fail (ma != NULL, FALSE);

	if (event->button == 1 && event->type == GDK_BUTTON_PRESS) {
		start_procman (ma);
		return TRUE;
	}
	return FALSE;
}

static gboolean
multiload_key_press_event_cb (GtkWidget *widget, GdkEventKey *event, MultiloadApplet *ma)
{
	g_return_val_if_fail (event != NULL, FALSE);
	g_return_val_if_fail (ma != NULL, FALSE);

	switch (event->keyval) {
	/* this list of keyvals taken from mixer applet, which seemed to have
		a good list of keys to use */
	case GDK_KP_Enter:
	case GDK_ISO_Enter:
	case GDK_3270_Enter:
	case GDK_Return:
	case GDK_space:
	case GDK_KP_Space:
		/* activate */
		start_procman (ma);
		return TRUE;

	default:
		break;
	}

	return FALSE;
}

/* update the tooltip to the graph's current "used" percentage */
void
multiload_applet_tooltip_update(LoadGraph *g)
{
	gchar *tooltip_text, *name;

	g_assert(g);
	g_assert(g->name);

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
	else if (!strncmp (g->name, "diskload", strlen("diskload")))
		name = g_strdup(_("Disk"));
	else
		g_assert_not_reached();
	
	/* fill data[0] with the current load */
	g->get_data (g->draw_height, g->data[0], g);
	
	if (!strncmp(g->name, "memload", strlen("memload"))) {
		guint mem_user, mem_cache, user_percent, cache_percent;
		mem_user  = g->data[0][0];
		mem_cache = g->data[0][1] + g->data[0][2] + g->data[0][3];
		user_percent = 100.0f * mem_user / g->draw_height;
		cache_percent = 100.0f * mem_cache / g->draw_height;
		user_percent = MIN(user_percent, 100);
		cache_percent = MIN(cache_percent, 100);

		/* xgettext: use and cache are > 1 most of the time,
		   please assume that they always are.
		 */
		tooltip_text = g_strdup_printf(_("%s:\n"
						 "%u%% in use by programs\n"
						 "%u%% in use as cache"),
					       name,
					       user_percent,
					       cache_percent);
	} else if (!strcmp(g->name, "loadavg")) {

		tooltip_text = g_strdup_printf(_("The system load average is %0.02f"),
					       g->loadavg1);

	} else {
		const char *msg;
		guint i, total_used, percent;

		for (i = 0, total_used = 0; i < (g->n - 1); i++)
			total_used += g->data[0][i];

		percent = 100.0f * total_used / g->draw_height;
		percent = MIN(percent, 100);

		msg = ngettext("%s:\n"
			       "%u%% in use",
			       "%s:\n"
			       "%u%% in use",
			       percent);

		tooltip_text = g_strdup_printf(msg,
					       name,
					       percent);
	}

	gtk_tooltips_set_tip(g->tooltips, g->disp, tooltip_text, tooltip_text);
		
	g_free(tooltip_text);
	g_free(name);
}

static void
multiload_create_graphs(MultiloadApplet *ma)
{
	struct { const char *label;
		 const char *name;
		 int num_colours;
		 LoadGraphDataFunc callback;
	       } graph_types[] = {
			{ _("CPU Load"),     "cpuload",  5, GetLoad },
			{ _("Memory Load"),  "memload",  5, GetMemory },
			{ _("Net Load"),     "netload",  5, GetNet },
			{ _("Swap Load"),    "swapload", 2, GetSwap },
			{ _("Load Average"), "loadavg",  2, GetLoadAvg },
			{ _("Disk Load"),    "diskload", 3, GetDiskLoad }
		};

	gint speed, size;
	gint i;

	speed = panel_applet_gconf_get_int (ma->applet, "speed", NULL);
	size = panel_applet_gconf_get_int (ma->applet, "size", NULL);
	speed = MAX (speed, 50);
	size = CLAMP (size, 10, 400);

	for (i = 0; i < G_N_ELEMENTS (graph_types); i++)
	{
		gboolean visible;
		char *key;

		key = g_strdup_printf ("view_%s", graph_types[i].name);
		visible = panel_applet_gconf_get_bool (ma->applet, key, NULL);
		g_free (key);

		ma->graphs[i] = load_graph_new (ma,
				                graph_types[i].num_colours,
						graph_types[i].label,
                                                i,
						speed,
						size,
						visible,
						graph_types[i].name,
						graph_types[i].callback);
	}
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
		if (!ma->graphs[i])
			continue;
			
		load_graph_stop(ma->graphs[i]);
		gtk_widget_destroy(ma->graphs[i]->main_widget);
		
		load_graph_unalloc(ma->graphs[i]);
		g_free(ma->graphs[i]);
	}

	if (ma->box)
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
	multiload_create_graphs (ma);

	/* only start and display the graphs the user has turned on */

	for (i = 0; i < NGRAPHS; i++) {
	    gtk_box_pack_start(GTK_BOX(ma->box), 
			       ma->graphs[i]->main_widget, 
			       TRUE, TRUE, 1);
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
	BONOBO_UI_UNSAFE_VERB ("MultiLoadHelp", help_cb),
	BONOBO_UI_UNSAFE_VERB ("MultiLoadAbout", about_cb),

	BONOBO_UI_VERB_END
};		

/* create a box and stuff the load graphs inside of it */
static gboolean
multiload_applet_new(PanelApplet *applet, const gchar *iid, gpointer data)
{
	MultiloadApplet *ma;
	GConfClient *client;
	BonoboUIComponent *popup_component;
	
	ma = g_new0(MultiloadApplet, 1);
	
	ma->applet = applet;
	
	ma->about_dialog = NULL;
	ma->prop_dialog = NULL;
        ma->last_clicked = 0;

	gtk_window_set_default_icon_name ("utilities-system-monitor");
	panel_applet_set_background_widget (applet, GTK_WIDGET(applet));
	
	panel_applet_add_preferences (applet, "/schemas/apps/multiload/prefs", NULL);
	panel_applet_set_flags (applet, PANEL_APPLET_EXPAND_MINOR);

	panel_applet_setup_menu_from_file (applet,
					   DATADIR,
					   "GNOME_MultiloadApplet.xml",
					   NULL,
					   multiload_menu_verbs,
					  ma);	

	popup_component = panel_applet_get_popup_component (applet);

	if (panel_applet_get_locked_down (applet)) {
		bonobo_ui_component_set_prop (popup_component,
					      "/commands/MultiLoadProperties",
					      "hidden", "1",
					      NULL);
	}

	client = gconf_client_get_default ();
	if (gconf_client_get_bool (client, "/desktop/gnome/lockdown/inhibit_command_line", NULL) ||
	    panel_applet_get_locked_down (applet)) {
		/* When the panel is locked down or when the command line is inhibited,
		   it seems very likely that running the procman would be at least harmful */
		bonobo_ui_component_set_prop (popup_component,
					      "/commands/MultiLoadRunProcman",
					      "hidden", "1",
					      NULL);
	}

	g_signal_connect(G_OBJECT(applet), "change_size",
				G_CALLBACK(multiload_change_size_cb), ma);
	g_signal_connect(G_OBJECT(applet), "change_orient",
				G_CALLBACK(multiload_change_orient_cb), ma);
	g_signal_connect(G_OBJECT(applet), "destroy",
				G_CALLBACK(multiload_destroy_cb), ma);
	g_signal_connect(G_OBJECT(applet), "enter_notify_event",
				G_CALLBACK(multiload_enter_cb), ma);
	g_signal_connect(G_OBJECT(applet), "leave_notify_event",
				G_CALLBACK(multiload_leave_cb), ma);
	g_signal_connect(G_OBJECT(applet), "button_press_event",
				G_CALLBACK(multiload_button_press_event_cb), ma);
	g_signal_connect(G_OBJECT(applet), "key_press_event",
				G_CALLBACK(multiload_key_press_event_cb), ma);
	
	multiload_applet_refresh (ma);
		
	gtk_widget_show(GTK_WIDGET(applet));
			
	return TRUE;
}

static gboolean
multiload_factory (PanelApplet *applet,
				const gchar *iid,
				gpointer data)
{
	gboolean retval = FALSE;
	
	glibtop_init();

	retval = multiload_applet_new(applet, iid, data);
	
	return retval;
}

PANEL_APPLET_BONOBO_FACTORY ("OAFIID:GNOME_MultiLoadApplet_Factory",
				   PANEL_TYPE_APPLET,
				   "multiload",
				   "0",
				   multiload_factory,
				   NULL)
