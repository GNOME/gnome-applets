/*
 * GNOME Universal MultiMedia Applet
 * (C) 1999 The Free Software Foundation
 *
 * Authors: Jacob Berkman  <jberkman@andrew.cmu.edu>
 *
 * Based on gnome-applets/cdplayer/cdplayer.c
 */

#include <config.h>

#include <sys/types.h>
#include <dirent.h>

#include <gnome.h>
#include <gmodule.h>
#include <applet-widget.h>

#include "led.h"
#include "gumma.h"

#include "pixmaps/stop.xpm"
#include "pixmaps/play-pause.xpm"
#include "pixmaps/prev.xpm"
#include "pixmaps/next.xpm"
#include "pixmaps/eject.xpm"

typedef struct {
	struct {
		void (*about) ();
		GtkWidget *(*get_config_page) ();
		void (*init) ();
		void (*denit) ();
		GummaState (*get_state) ();
		void (*do_verb) (GummaVerb verb);
		void (*data_dropped) (GtkSelectionData *data);
		void (*get_time) (GummaTimeInfo *tinfo);
	} plugin;

	struct {
		GtkWidget *time;
		GtkWidget *track;

		/* control button */
		GtkWidget *play_pause;
		GtkWidget *stop;
		GtkWidget *prev;
		GtkWidget *next;
		GtkWidget *eject;
	} panel;

	GModule *module;
	gchar *module_path;
	int timeout;
} GummaPlayerData;

#define TIMEOUT_VALUE 500

static GtkWidget *applet = NULL;

static void
panel_update(GtkWidget *panel, GummaPlayerData *gpd)
{
	GummaTimeInfo tinfo;

	switch (gpd->plugin.get_state ()) {
	case GUMMA_STATE_PLAYING:
		gpd->plugin.get_time (&tinfo);
		led_time (gpd->panel.time,
			  tinfo.minutes,
			  tinfo.seconds,
			  gpd->panel.track,
			  tinfo.track);
		break;
	case GUMMA_STATE_PAUSED:
		gpd->plugin.get_time (&tinfo);
		led_paused (gpd->panel.time,
			   tinfo.minutes,
			   tinfo.seconds,
			   gpd->panel.track,
			   tinfo.track);
		break;
	case GUMMA_STATE_STOPPED:
		led_stop (gpd->panel.time,
			  gpd->panel.track);
		break;
	case GUMMA_STATE_ERROR:
		led_nodisc (gpd->panel.time,
			    gpd->panel.track);
		break;
	}
}

static int 
play_pause_cb (GtkWidget *w, gpointer data)
{
	GummaPlayerData *gpd = data;
	
	if (gpd->plugin.get_state () == GUMMA_STATE_PLAYING)
		gpd->plugin.do_verb (GUMMA_VERB_PAUSE);
	else
		gpd->plugin.do_verb (GUMMA_VERB_PLAY);
	return 0;
}

static void start_gtcd_cb()
{
	gnome_execute_shell(NULL, "gtcd");
}
                

static int 
generic_cb (GtkWidget *w, gpointer data)
{	
	GummaPlayerData *gpd;
	gpd = gtk_object_get_user_data (GTK_OBJECT (w));
	gpd->plugin.do_verb (GPOINTER_TO_INT (data));
	return 0;
}

static int
timeout_cb (gpointer data)
{
	GtkWidget *player = data;
	GummaPlayerData *gpd;

	gpd = gtk_object_get_user_data (GTK_OBJECT (player));

	panel_update (player, gpd);
	return 1;
}

static GtkWidget *
control_button_factory(GtkWidget *box_container,
		       gchar *pixmap_data[],
		       GummaPlayerData *gpd,
		       GtkSignalFunc func,
		       gpointer data)
{
	GtkWidget *w, *pixmap;

	w = gtk_button_new();
	gtk_object_set_user_data (GTK_OBJECT (w), gpd);
	GTK_WIDGET_UNSET_FLAGS(w, GTK_CAN_DEFAULT);
	GTK_WIDGET_UNSET_FLAGS(w, GTK_CAN_FOCUS);
	pixmap = gnome_pixmap_new_from_xpm_d (pixmap_data);
	gtk_box_pack_start(GTK_BOX(box_container), w, FALSE, TRUE, 0);
	gtk_widget_show(pixmap);
	gtk_container_add(GTK_CONTAINER(w), pixmap);
	gtk_signal_connect(GTK_OBJECT(w), "clicked", func, data);
	gtk_widget_show(w);
	return w;
}

static GtkWidget *
create_panel_widget(GtkWidget *window, GummaPlayerData *gpd)
{
	GtkWidget *frame;
	GtkWidget *vbox;
	GtkWidget *hbox;

	frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
	gtk_widget_show(frame);

	vbox = gtk_vbox_new(FALSE, FALSE);
	gtk_container_add(GTK_CONTAINER(frame), vbox);
	gtk_widget_show(vbox);

	/* */
	hbox = gtk_hbox_new(FALSE, FALSE);
	gtk_box_pack_start_defaults(GTK_BOX(vbox), hbox);
	gtk_widget_show(hbox);

	led_init(window);
	led_create_widget (window,&gpd->panel.time, &gpd->panel.track);

	gtk_box_pack_start (GTK_BOX(hbox), gpd->panel.time, TRUE, TRUE, 0);
	gtk_widget_show(gpd->panel.time);

	/* */
	hbox = gtk_hbox_new(FALSE, FALSE);
	gtk_box_pack_start_defaults(GTK_BOX(vbox), hbox);
	gtk_widget_show(hbox);

	gpd->panel.stop = control_button_factory(hbox, stop_xpm, gpd,
						 GTK_SIGNAL_FUNC (generic_cb),
						 GINT_TO_POINTER (GUMMA_VERB_STOP));
	gpd->panel.play_pause = control_button_factory(hbox, play_pause_xpm, gpd,
						       GTK_SIGNAL_FUNC (play_pause_cb), 
						       gpd);
	gpd->panel.eject = control_button_factory(hbox, eject_xpm, gpd,
						  GTK_SIGNAL_FUNC (generic_cb),
						  GINT_TO_POINTER (GUMMA_VERB_EJECT));

	/* */
	hbox = gtk_hbox_new(FALSE, FALSE);
	gtk_box_pack_start_defaults(GTK_BOX(vbox), hbox);
	gtk_widget_show(hbox);

	gpd->panel.prev = control_button_factory(hbox, prev_xpm, gpd,
						 GTK_SIGNAL_FUNC (generic_cb),
						 GINT_TO_POINTER (GUMMA_VERB_PREV));

	gtk_box_pack_start (GTK_BOX(hbox), gpd->panel.track, TRUE, TRUE, 0);
	gtk_widget_show(gpd->panel.track);						 
	
	gpd->panel.next = control_button_factory(hbox, next_xpm, gpd,
						 GTK_SIGNAL_FUNC (generic_cb),
						 GINT_TO_POINTER (GUMMA_VERB_NEXT));
	
	return frame;
}

static void 
destroy_player(GtkWidget * widget, void *data)
{
	GummaPlayerData *gpd = data;
	gtk_timeout_remove(gpd->timeout);
	gpd->plugin.denit ();
	g_free (gpd);
}

static void
panel_realized(GtkWidget *panel, GummaPlayerData *gpd)
{
	panel_update(panel, gpd);
}

static void
load_plugin (GummaPlayerData *gpd)
{
	g_assert (g_module_supported ());
	
	g_message (_("Loading plugin %s..."), gpd->module_path);
	gpd->module = g_module_open (gpd->module_path, 0);

	if (!gpd->module)
		return;

	g_module_symbol (gpd->module,
			 "gp_about",
			 (gpointer)&gpd->plugin.about);
	g_module_symbol (gpd->module,
			 "gp_get_config_page",
			 (gpointer)&gpd->plugin.get_config_page);
	g_module_symbol (gpd->module,
			 "gp_init",
			 (gpointer)&gpd->plugin.init);
	g_module_symbol (gpd->module,
			 "gp_denit",
			 (gpointer)&gpd->plugin.denit);
	g_module_symbol (gpd->module,
			 "gp_get_state",
			 (gpointer)&gpd->plugin.get_state);
	g_module_symbol (gpd->module,
			 "gp_do_verb",
			 (gpointer)&gpd->plugin.do_verb);
	g_module_symbol (gpd->module,
			 "gp_data_dropped",
			 (gpointer)&gpd->plugin.data_dropped);
	g_module_symbol (gpd->module,
			 "gp_get_time",
			 (gpointer)&gpd->plugin.get_time);
	
	g_assert (gpd->plugin.about);
	g_assert (gpd->plugin.get_config_page);
	g_assert (gpd->plugin.init);
	g_assert (gpd->plugin.denit);
	g_assert (gpd->plugin.get_state);
	g_assert (gpd->plugin.do_verb);
	g_assert (gpd->plugin.data_dropped);
	g_assert (gpd->plugin.get_time);
}

static GtkWidget *
create_player_widget(GtkWidget *window, const char *privcfgpath)
{
	GummaPlayerData *gpd;
	GtkWidget *panel;
	
	gpd = g_new0 (GummaPlayerData, 1);

	gnome_config_push_prefix(privcfgpath);
	gpd->module_path = gnome_config_get_string("GUMMA/plugin");
	gnome_config_pop_prefix();

	if (!gpd->module_path)
		gpd->module_path = 
			gnome_libdir_file ("gumma/libgumma-gqmpeg.so");
	load_plugin (gpd);
	if (!gpd->module)
		g_warning ("No plugin loaded");
	gpd->plugin.init ();
	panel = create_panel_widget (window, gpd);

	/* Install timeout handler */

	gpd->timeout = gtk_timeout_add (TIMEOUT_VALUE, 
					timeout_cb,
					panel);

	gtk_object_set_user_data (GTK_OBJECT (panel), gpd);
	gtk_signal_connect(GTK_OBJECT(panel), "destroy",
			   GTK_SIGNAL_FUNC (destroy_player),
			   gpd);
	gtk_signal_connect(GTK_OBJECT (panel), "realize",
			   GTK_SIGNAL_FUNC (panel_realized), 
			   gpd);

	return panel;
}

static void
about_cb (AppletWidget *applet, gpointer data)
{
	const char *authors[] = {
		"Jacob Berkman  <jberkman@andrew.cmu.edu>",
		NULL
	};
	static GtkWidget *about_box = NULL;
	
	if (about_box) {
		gdk_window_show (about_box->window);
		gdk_window_raise (about_box->window);
		return;
	}

	about_box = gnome_about_new("GNOME Useless MultiMedia Applet", VERSION,
				    "Copyright (C) 1999 The Free Software Foundation",
				    authors,
				    "Panel applet to control various media players.\n\n"
				    "Based partly on the GNOME cdplayer applet originally by Ching Hui",
				    NULL);

	gtk_signal_connect (GTK_OBJECT (about_box), "destroy",
			    GTK_SIGNAL_FUNC (gtk_widget_destroyed),
			    &about_box);

	gtk_widget_show(about_box);
}

static gboolean
clicked_cb (GtkWidget *dialog, gint button, gpointer data)
{
	GtkWidget *clist;
	gint row;
	gchar *file;
	GummaPlayerData *gpd;

	if (button == GNOME_CANCEL) {
		/*gtk_widget_destroy (dialog);*/
		return FALSE;
	}

	clist = data;
	row = GPOINTER_TO_INT (GTK_CLIST (data)->selection->data);
	file = gtk_clist_get_row_data (GTK_CLIST (data), row);
	gpd = gtk_object_get_user_data (GTK_OBJECT (clist));

	if (gpd->module) {
		gpd->plugin.denit ();
		g_module_close (gpd->module);
		gpd->module = NULL;
		g_free (gpd->module_path);
	}

	gpd->module_path = g_strdup (file);
	load_plugin (gpd);
	gpd->plugin.init ();

	/*gtk_widget_destroy (dialog);*/
	return FALSE;
}

static gint
cleanup (GtkWidget *w, gpointer data)
{
	char *s = data;
	g_free (s);
	return FALSE;
}

static void
plugin_cb (AppletWidget *applet, gpointer data)
{
	GtkWidget *dialog=NULL;
	GtkWidget *scroll;
	GtkWidget *clist;
	GtkWidget *vbox;

	gchar *titles[1];
	gint row;
	DIR *d;
	struct dirent *e;
	char *path, *s;
	
	if (dialog) {
		gdk_window_show (dialog->window);
		gdk_window_raise (dialog->window);
		return;
	}

	dialog = gnome_dialog_new (_("Select a plugin"),
				   GNOME_STOCK_BUTTON_OK,
				   GNOME_STOCK_BUTTON_CANCEL,
				   NULL);
	vbox = GTK_WIDGET (GNOME_DIALOG (dialog)->vbox);
				   
	scroll = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);

	gtk_box_pack_start (GTK_BOX (vbox), scroll,
			    TRUE, TRUE, GNOME_PAD);
	
	titles[0] = _("Plugins");
	clist = gtk_clist_new_with_titles (1, titles);
	gtk_container_add (GTK_CONTAINER (scroll), clist);

	path = gnome_libdir_file ("gumma");
	d = opendir (path);
	while ((e = readdir (d)) != NULL){
		if (strncmp (e->d_name + strlen (e->d_name) - 3, ".so", 3) == 0){
			titles[0] = e->d_name;
			row = gtk_clist_append (GTK_CLIST (clist), titles);
			s = g_concat_dir_and_file (path, e->d_name);
			gtk_clist_set_row_data (GTK_CLIST (clist), row, s);
			gtk_signal_connect (GTK_OBJECT (clist), "destroy",
					    GTK_SIGNAL_FUNC (cleanup), s);
					   
		}
	}
	closedir (d);
	g_free (path);

	gtk_object_set_user_data (GTK_OBJECT (clist), data);

	gnome_dialog_set_close (GNOME_DIALOG (dialog), TRUE);
	gtk_signal_connect (GTK_OBJECT (dialog), "clicked",
			    GTK_SIGNAL_FUNC (clicked_cb), 
			    clist);

	gtk_signal_connect (GTK_OBJECT (dialog), "destroy",
			    GTK_SIGNAL_FUNC (gtk_widget_destroyed),
			    &dialog);
	
	gtk_widget_show_all (dialog);
}

static gint
save_session_cb (GtkWidget *widget, gchar *privcfgpath,
		 gchar *globcfgpath, gpointer data)
{
	GummaPlayerData *gpd = data;
	gnome_config_push_prefix (privcfgpath);
	gnome_config_set_string ("GUMMA/plugin", gpd->module_path);
	gnome_config_pop_prefix ();
	gnome_config_sync ();
	return FALSE;
}

int
main (int argc, char **argv)
{
	GtkWidget *player;
	GummaPlayerData *gpd;

	bindtextdomain(PACKAGE, GNOMELOCALEDIR);
	textdomain(PACKAGE);

	applet_widget_init("gumma", VERSION, argc, argv,
			   NULL, 0, NULL);

	applet = applet_widget_new("gumma");
	if (!applet)
		g_error("Can't create applet!\n");

	gtk_widget_realize(applet);
	player = create_player_widget (applet,
				       APPLET_WIDGET(applet)->privcfgpath);

	if(player == NULL) {
		applet_widget_abort_load (APPLET_WIDGET (applet));
		return 1;
	}

	gpd = gtk_object_get_user_data (GTK_OBJECT (player));

        applet_widget_register_stock_callback(APPLET_WIDGET(applet), "about",
					      GNOME_STOCK_MENU_ABOUT, _("About..."),
					      about_cb, NULL);

	
	applet_widget_register_callback (APPLET_WIDGET (applet), "plugins",
					 _("Plugins..."),
					 plugin_cb, gpd);

	gtk_signal_connect (GTK_OBJECT (applet), "save_session",
			    GTK_SIGNAL_FUNC (save_session_cb),
			    gpd);

	gtk_widget_show(player);
	applet_widget_add (APPLET_WIDGET (applet), player);
	gtk_widget_show (applet);

	applet_widget_gtk_main ();

	if (gpd->plugin.denit)
		gpd->plugin.denit ();

	return 0;
}
