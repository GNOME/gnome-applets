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
#include <gconf/gconf-client.h>
#include <panel-applet-gconf.h>

#include <gtk/gtk.h>
#include <libgnomeui/libgnomeui.h>
#include <libgnome/libgnome.h>
#include <libgnomeui/gnome-window-icon.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <libart_lgpl/libart.h>
#include <egg-screen-exec.h>
#include <egg-screen-help.h>

#include "global.h"

static void
about_cb (BonoboUIComponent *uic,
	  MultiloadApplet   *ma,
	  const char        *name)
{
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
		"Chee Bin HOH <cbhoh@gnome.org>",
		"Sun GNOME Documentation Team <gdocteam@sun.com>",
		NULL
    };

    const gchar *translator_credits = _("translator_credits");

    if (ma->about_dialog) {
	gtk_window_set_screen (GTK_WINDOW (ma->about_dialog),
			       gtk_widget_get_screen (GTK_WIDGET (ma->applet)));

	gtk_window_present (GTK_WINDOW (ma->about_dialog));
	return;
    }
	
    file = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_PIXMAP, "gnome-monitor.png", FALSE, NULL);
    pixbuf = gdk_pixbuf_new_from_file (file, &error);
    g_free (file);
   
    if (error) {
	g_warning (G_STRLOC ": cannot open %s: %s", file, error->message);
	g_error_free (error);
    }

	
    ma->about_dialog = gnome_about_new (_("System Monitor"), VERSION,
					"(C) 1999 - 2002 The Free Software Foundation",
					_("Released under the GNU General Public License.\n\n"
					  "A system load monitor capable of displaying graphs for CPU, ram, and swap space use, plus network traffic."),
					authors,
					documenters,
					strcmp (translator_credits, "translator_credits") != 0 ? translator_credits : NULL,
					pixbuf);

    if (pixbuf) 
	g_object_unref (pixbuf);
  
    gtk_window_set_screen (GTK_WINDOW (ma->about_dialog),
			   gtk_widget_get_screen (GTK_WIDGET (ma->applet)));
    gtk_window_set_wmclass (GTK_WINDOW (ma->about_dialog), "system monitor",
			    "System Monitor");
    g_signal_connect (G_OBJECT (ma->about_dialog), "destroy",
		      G_CALLBACK (gtk_widget_destroyed), &ma->about_dialog);

    gtk_widget_show (ma->about_dialog);
}

static void
help_cb (BonoboUIComponent *uic,
	  MultiloadApplet   *ma,
	  const char        *name)
{

 	GError *error = NULL;
                                                                                
    	egg_help_display_on_screen (
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

	g_return_if_fail (ma != NULL);

	gdk_spawn_command_line_on_screen (
			gtk_widget_get_screen (GTK_WIDGET (ma->applet)),
			"gnome-system-monitor", &error);

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

void
multiload_change_size_cb(PanelApplet *applet, gint size, gpointer data)
{
	MultiloadApplet *ma = (MultiloadApplet *)data;
	
	multiload_applet_refresh(ma);
	
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
multiload_change_background_cb(PanelApplet *a, PanelAppletBackgroundType type,
				GdkColor *color, GdkPixmap *pixmap,
				gpointer *data)
{
	MultiloadApplet *ma = data;
	GtkRcStyle *rc_style = gtk_rc_style_new ();

	switch (type) {
		case PANEL_PIXMAP_BACKGROUND:
			gtk_widget_modify_style (GTK_WIDGET (ma->applet), rc_style);
			break;

		case PANEL_COLOR_BACKGROUND:
			gtk_widget_modify_bg (GTK_WIDGET (ma->applet), GTK_STATE_NORMAL, color);
			break;

		case PANEL_NO_BACKGROUND:
			gtk_widget_modify_style (GTK_WIDGET (ma->applet), rc_style);
			break;

		default:
			gtk_widget_modify_style (GTK_WIDGET (ma->applet), rc_style);
			break;
	}

	gtk_rc_style_unref (rc_style);
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
			g_free (ma->graphs[i]->colors);
			ma->graphs[i]->colors = NULL;
			gtk_widget_destroy(ma->graphs[i]->main_widget);
		}
		
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

static gboolean
multiload_button_press_event_cb (GtkWidget *widget, GdkEventButton *event, MultiloadApplet *ma)
{
	g_return_val_if_fail (event != NULL, FALSE);
	g_return_val_if_fail (ma != NULL, FALSE);

	if (event->button == 1 && event->type == GDK_2BUTTON_PRESS) {
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
	gint i, total_used, percent, mem_used, used_percent;
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
	
	percent = 100 * (gdouble)total_used / (gdouble)g->draw_height;
	percent = CLAMP (percent, 0, 100);

	if (!strncmp(g->name, "memload", strlen("memload"))) {
		mem_used = total_used - g->data[0][i - 1];
		used_percent = 100 * (gdouble)mem_used / (gdouble)g->draw_height;
		used_percent = CLAMP (used_percent, 0, 100);
		
		tooltip_text = g_strdup_printf(_("%s:\n%d%% in use of which\n%d%% is cache"), name, percent,
				percent - used_percent);
	} else
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
		if (!ma->graphs[i])
			continue;
			
		if (ma->graphs[i]->visible)
		{
			load_graph_stop(ma->graphs[i]);
			gtk_widget_destroy(ma->graphs[i]->main_widget);
		}
		
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
	
	ma->graphs[0] = cpuload_applet_new(ma->applet, NULL);
	ma->graphs[1] = memload_applet_new(ma->applet, NULL);
	ma->graphs[2] = netload_applet_new(ma->applet, NULL);
	ma->graphs[3] = swapload_applet_new(ma->applet, NULL);
	ma->graphs[4] = loadavg_applet_new(ma->applet, NULL);
	
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
gboolean
multiload_applet_new(PanelApplet *applet, const gchar *iid, gpointer data)
{
	MultiloadApplet *ma;
	GConfClient *client;
	BonoboUIComponent *popup_component;
	
	ma = g_new0(MultiloadApplet, 1);
	
	ma->applet = applet;
	
	ma->about_dialog = NULL;
	ma->prop_dialog = NULL;

	gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gnome-monitor.png");
	
	panel_applet_add_preferences (applet, "/schemas/apps/multiload/prefs", NULL);
	panel_applet_set_flags (applet, PANEL_APPLET_EXPAND_MINOR);

	panel_applet_setup_menu_from_file (applet,
					   NULL,
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
	g_signal_connect(G_OBJECT(applet), "change_background",
				G_CALLBACK(multiload_change_background_cb), ma);
	g_signal_connect(G_OBJECT(applet), "destroy",
				G_CALLBACK(multiload_destroy_cb), ma);
	g_signal_connect(G_OBJECT(applet), "enter_notify_event",
				G_CALLBACK(multiload_enter_cb), ma);
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
	
	retval = multiload_applet_new(applet, iid, data);
	
	return retval;
}

PANEL_APPLET_BONOBO_FACTORY ("OAFIID:GNOME_MultiLoadApplet_Factory",
				   PANEL_TYPE_APPLET,
				   "multiload",
				   "0",
				   multiload_factory,
				   NULL);
