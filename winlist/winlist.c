/*
 * GNOME FVWM window list module.
 * (C) 1998 Red Hat Software
 *
 * Author: Elliot Lee
 *
 */

#include <stdio.h>
#ifdef HAVE_LIBINTL
#    include <libintl.h>
#endif
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <gnome.h>
#include <gdk/gdkx.h>
#include "applet-lib.h"
#include "panel.h"
#include "mico-parse.h"

#define WINLIST_DATA "winlist_data"

GtkWidget *plug = NULL;

int applet_id = (-1); /*this is our id we use to comunicate with the panel */


static void
free_data(GtkWidget * widget, gpointer data)
{
	g_free(data);
}

extern void
show_winlist(GtkWidget *winlist);

static void
create_computer_winlist_widget(GtkWidget ** winlist)
{
	GtkWidget *frame;
	GtkWidget *btn;

	frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);
	gtk_widget_show(frame);

	btn = gtk_button_new_with_label("Winlist");
	gtk_container_add(GTK_CONTAINER(frame), btn);
	gtk_widget_show(btn);

	gtk_signal_connect(GTK_OBJECT(btn), "clicked",
			   GTK_SIGNAL_FUNC(show_winlist), NULL);
	*winlist = frame;
}

static GtkWidget *
create_winlist_widget(void)
{
	GtkWidget *winlist;
	time_t current_time;

	/*FIXME: different winlist types here */
	create_computer_winlist_widget(&winlist);

	return winlist;
}

/*these are commands sent over corba: */
void
change_orient(int id, int orient)
{
  PanelOrientType o = (PanelOrientType) orient;
}

void
session_save(int id, const char *cfgpath, const char *globcfgpath)
{
  /*save the session here */
}

static gint
quit_winlist(gpointer data)
{
  exit(0);
}

void
shutdown_applet(int id)
{
  /*kill our plug using destroy to avoid warnings we need to
    kill the plug but we also need to return from this call */
  if (plug)
    gtk_widget_destroy(plug);
  gtk_idle_add(quit_winlist, NULL);
}


/*
void
test_callback(int id, gpointer data)
{
	puts("TEST");
}
*/


int
main(int argc, char **argv)
{
	GtkWidget *winlist;
	char *result;
	char *cfgpath;
	char *globcfgpath;

	char *myinvoc;
	guint32 winid;

	myinvoc = get_full_path(argv[0]);
	if(!myinvoc)
		return 1;

	panel_corba_register_arguments();
	gnome_init("winlist_applet", NULL, argc, argv, 0, NULL);

	if (!gnome_panel_applet_init_corba())
		g_error("Could not comunicate with the panel\n");

	result = gnome_panel_applet_request_id(myinvoc, &applet_id,
					       &cfgpath, &globcfgpath,
					       &winid);

	g_free(myinvoc);
	if (result)
		g_error("Could not talk to the Panel: %s\n", result);
	/*use cfg path for loading up data! */

	g_free(globcfgpath);
	g_free(cfgpath);

	plug = gtk_plug_new(winid);

	winlist = create_winlist_widget();
	gtk_widget_show(winlist);
	gtk_container_add(GTK_CONTAINER(plug), winlist);
	gtk_widget_show(plug);


	result = gnome_panel_applet_register(plug, applet_id);
	if (result)
		g_error("Could not talk to the Panel: %s\n", result);

/*
	gnome_panel_applet_register_callback(applet_id,
					     "test",
					     "TEST CALLBACK",
					     test_callback,
					     NULL);
*/

	applet_corba_gtk_main("IDL:GNOME/Applet:1.0");

	return 0;
}
