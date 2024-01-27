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

#include "config.h"

#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>
#include <string.h>
#include <time.h>

#include <glibtop.h>
#include <gio/gdesktopappinfo.h>
#include <gdk/gdkkeysyms.h>
#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "global.h"

G_DEFINE_TYPE (MultiloadApplet, multiload_applet, GP_TYPE_APPLET)

static void
about_cb (GSimpleAction *action,
          GVariant      *parameter,
          gpointer       user_data)
{
  gp_applet_show_about (GP_APPLET (user_data));
}

static void
help_cb (GSimpleAction *action,
         GVariant      *parameter,
         gpointer       user_data)
{
  gp_applet_show_help (GP_APPLET (user_data), NULL);
}

/* run the full-scale system process monitor */

static void
start_procman (MultiloadApplet *ma)
{
	GError *error = NULL;
	GDesktopAppInfo *appinfo;
	gchar *monitor;
	GdkAppLaunchContext *launch_context;
	GAppInfo *app_info;
	GdkScreen *screen;

	g_return_if_fail (ma != NULL);

	monitor = g_settings_get_string (ma->settings, KEY_SYSTEM_MONITOR);
	if (IS_STRING_EMPTY (monitor))
	        monitor = g_strdup ("gnome-system-monitor.desktop");

	screen = gtk_widget_get_screen (GTK_WIDGET (ma));
	appinfo = g_desktop_app_info_new (monitor);
	if (appinfo) {
		GdkAppLaunchContext *context;

		context = gdk_app_launch_context_new ();
		gdk_app_launch_context_set_screen (context, screen);
		gdk_app_launch_context_set_timestamp (context,
						      gtk_get_current_event_time ());

		g_app_info_launch_uris (G_APP_INFO (appinfo), NULL,
					(GAppLaunchContext *) context,
					&error);

		g_object_unref (context);
		g_object_unref (appinfo);

		if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
			g_error_free (error);
			error = NULL;
		}
	}
	else {	
		app_info = g_app_info_create_from_commandline ("gnome-system-monitor",
							      _("Start system-monitor"),
							      G_APP_INFO_CREATE_NONE,
							      &error);

		if (!error) {
			launch_context = gdk_app_launch_context_new ();
			gdk_app_launch_context_set_screen (launch_context, screen);
			g_app_info_launch (app_info, NULL, G_APP_LAUNCH_CONTEXT (launch_context), &error);

			g_object_unref (launch_context);
		}
	}
	g_free (monitor);

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
				       gtk_widget_get_screen (GTK_WIDGET (ma)));

		gtk_widget_show (dialog);

		g_error_free (error);
	}
}
              
static void
start_procman_cb (GSimpleAction *action,
                  GVariant      *parameter,
                  gpointer       user_data)
{
	MultiloadApplet *ma = (MultiloadApplet *) user_data;
	start_procman (ma);
}

static void
placement_changed_cb (GpApplet        *applet,
                      GtkOrientation   orientation,
                      GtkPositionType  position,
                      MultiloadApplet *self)
{
  self->orientation = orientation;
  multiload_applet_refresh (self);
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
	case GDK_KEY_KP_Enter:
	case GDK_KEY_ISO_Enter:
	case GDK_KEY_3270_Enter:
	case GDK_KEY_Return:
	case GDK_KEY_space:
	case GDK_KEY_KP_Space:
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
	else if (!strncmp(g->name, "netload2", strlen("netload2")))
		name = g_strdup(_("Network"));
	else if (!strncmp(g->name, "swapload", strlen("swapload")))
		name = g_strdup(_("Swap Space"));
	else if (!strncmp(g->name, "loadavg", strlen("loadavg")))
		name = g_strdup(_("Load Average"));
	else if (!strncmp (g->name, "diskload", strlen("diskload")))
		name = g_strdup(_("Disk"));
	else
		g_assert_not_reached();
	
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

	} else if (!strcmp(g->name, "netload2")) {
		char *tx_in, *tx_out;
		tx_in = netspeed_get(g->netspeed_in);
		tx_out = netspeed_get(g->netspeed_out);
		/* xgettext: same as in graphic tab of g-s-m */
		tooltip_text = g_strdup_printf(_("%s:\n"
						 "Receiving %s\n"
						 "Sending %s"),
					       name, tx_in, tx_out);
		g_free(tx_in);
		g_free(tx_out);
	} else {
		guint i, total_used, percent;

		for (i = 0, total_used = 0; i < (g->n - 1); i++)
			total_used += g->data[0][i];

		percent = 100.0f * total_used / g->draw_height;
		percent = MIN(percent, 100);

		tooltip_text = g_strdup_printf (dngettext (GETTEXT_PACKAGE,
		                                           "%s:\n%u%% in use",
		                                           "%s:\n%u%% in use",
		                                           percent),
		                                name,
		                                percent);
	}

	gtk_widget_set_tooltip_text(g->disp, tooltip_text);
		
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
			{ _("Net Load"),     "netload2",  4, GetNet },
			{ _("Swap Load"),    "swapload", 2, GetSwap },
			{ _("Load Average"), "loadavg",  2, GetLoadAvg },
			{ _("Disk Load"),    "diskload", 3, GetDiskLoad }
		};

	gint speed, size;
	unsigned int i;

	speed = g_settings_get_int (ma->settings, KEY_SPEED);
	size = g_settings_get_int (ma->settings, KEY_SIZE);
	speed = MAX (speed, 50);
	size = CLAMP (size, 10, 400);

	for (i = 0; i < G_N_ELEMENTS (graph_types); i++)
	{
		gboolean visible;
		char *key;

		key = g_strdup_printf ("view-%s", graph_types[i].name);
		visible = g_settings_get_boolean (ma->settings, key);
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

	if (ma->orientation == GTK_ORIENTATION_HORIZONTAL) {
			ma->box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	}
	else
		ma->box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

	gtk_container_add (GTK_CONTAINER (ma), ma->box);

	/* create the NGRAPHS graphs, passing in their user-configurable properties. */
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

static const GActionEntry multiload_menu_actions [] = {
	{ "run",         start_procman_cb,        NULL, NULL, NULL },
	{ "preferences", multiload_properties_cb, NULL, NULL, NULL },
	{ "help",        help_cb,                 NULL, NULL, NULL },
	{ "about",       about_cb,                NULL, NULL, NULL },
	{ NULL }
};

static void
multiload_applet_setup (MultiloadApplet *ma)
{
	GSettings *settings;
	const char *menu_resource;
	GAction *action;

	ma->prop_dialog = NULL;
	ma->last_clicked = 0;

	ma->settings = gp_applet_settings_new (GP_APPLET (ma), MULTILOAD_SCHEMA);
	gp_applet_set_flags (GP_APPLET (ma), GP_APPLET_FLAGS_EXPAND_MINOR);

	ma->orientation = gp_applet_get_orientation (GP_APPLET (ma));

	menu_resource = GRESOURCE_PREFIX "/ui/multiload-applet-menu.ui";
	gp_applet_setup_menu_from_resource (GP_APPLET (ma),
	                                    menu_resource,
	                                    multiload_menu_actions);

	action = gp_applet_menu_lookup_action (GP_APPLET (ma), "preferences");
	g_object_bind_property (ma, "locked-down", action, "enabled",
	                        G_BINDING_DEFAULT|G_BINDING_INVERT_BOOLEAN|G_BINDING_SYNC_CREATE);

	settings = g_settings_new (GNOME_DESKTOP_LOCKDOWN_SCHEMA);

	if (g_settings_get_boolean (settings, DISABLE_COMMAND_LINE) ||
	    gp_applet_get_locked_down (GP_APPLET (ma))) {
		/* When the panel is locked down or when the command line is inhibited,
		   it seems very likely that running the procman would be at least harmful */
		action = gp_applet_menu_lookup_action (GP_APPLET (ma), "run");
		g_simple_action_set_enabled (G_SIMPLE_ACTION (action), FALSE);
	}

	g_object_unref (settings);

	g_signal_connect (ma,
	                  "placement-changed",
	                  G_CALLBACK (placement_changed_cb),
	                  ma);

	g_signal_connect (ma,
	                  "button-press-event",
	                  G_CALLBACK (multiload_button_press_event_cb),
	                  ma);

	g_signal_connect (ma,
	                  "key-press-event",
	                  G_CALLBACK (multiload_key_press_event_cb),
	                  ma);

	multiload_applet_refresh (ma);

	gtk_widget_show (GTK_WIDGET (ma));
}

static void
multiload_applet_constructed (GObject *object)
{
  G_OBJECT_CLASS (multiload_applet_parent_class)->constructed (object);
  multiload_applet_setup (MULTILOAD_APPLET (object));
}

static void
multiload_applet_dispose (GObject *object)
{
  MultiloadApplet *self;
  int i;

  self = MULTILOAD_APPLET (object);

  for (i = 0; i < NGRAPHS; i++)
    {
      if (self->graphs[i] != NULL)
        {
          load_graph_stop (self->graphs[i]);

          g_clear_pointer (&self->graphs[i]->colors, g_free);
          g_clear_pointer (&self->graphs[i]->main_widget, gtk_widget_destroy);

          load_graph_unalloc (self->graphs[i]);
          g_free (self->graphs[i]);
          self->graphs[i] = NULL;
        }
    }

  g_clear_object (&self->settings);

  g_clear_pointer (&self->prop_dialog, gtk_widget_destroy);

  G_OBJECT_CLASS (multiload_applet_parent_class)->dispose (object);
}

static void
multiload_applet_class_init (MultiloadAppletClass *self_class)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (self_class);

  object_class->constructed = multiload_applet_constructed;
  object_class->dispose = multiload_applet_dispose;
}

static void
multiload_applet_init (MultiloadApplet *self)
{
}

void
multiload_applet_setup_about (GtkAboutDialog *dialog)
{
  const char *comments;
  const char **authors;
  const char **documenters;
  const char *copyright;

  comments = _("A system load monitor capable of displaying graphs "
               "for CPU, ram, and swap space use, plus network "
               "traffic.");

  authors = (const char *[])
    {
      "Martin Baulig <martin@home-of-linux.org>",
      "Todd Kulesza <fflewddur@dropline.net>",
      "Benoît Dejean <TazForEver@dlfp.org>",
      "Davyd Madeley <davyd@madeley.id.au>",
      NULL
    };

  documenters = (const char *[])
    {
      "Chee Bin HOH <cbhoh@gnome.org>",
      "Sun GNOME Documentation Team <gdocteam@sun.com>",
      NULL
    };

  copyright = "\xC2\xA9 1999-2005 Free Software Foundation and others";

  gtk_about_dialog_set_comments (dialog, comments);

  gtk_about_dialog_set_authors (dialog, authors);
  gtk_about_dialog_set_documenters (dialog, documenters);
  gtk_about_dialog_set_translator_credits (dialog, _("translator-credits"));
  gtk_about_dialog_set_copyright (dialog, copyright);
}
