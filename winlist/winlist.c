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

GtkWidget *plug = NULL;

/* We use this to create our window menu. The data is put here by the
   wm-dependant module */
GHashTable *winlist = NULL;

int applet_id = (-1); /*this is our id we use to comunicate with the panel */

extern void
init_winlist(int argc, char **argv, int firstarg);
extern void
switch_to_window(guint win_id);

static void activate_winlist_menu_item(GtkWidget *menu_item,
				       gpointer window_id)
{
  switch_to_window((guint)window_id);
}

static void
make_winlist_menu_item(gpointer key, gpointer value, gpointer user_data)
{
  GtkWidget *nmi;

  g_print("Adding menu item %s\n", (gchar *)value);
  nmi = gtk_menu_item_new_with_label(value);
  gtk_signal_connect(GTK_OBJECT(nmi), "activate",
		     GTK_SIGNAL_FUNC(activate_winlist_menu_item), key);
  gtk_menu_append(GTK_MENU(user_data), nmi);
  gtk_widget_show(nmi);
}

static GtkWidget*
make_winlist_menu(void)
{
  GtkWidget *retval = gtk_menu_new();
  g_print("Making the menu\n");
  g_hash_table_foreach(winlist, make_winlist_menu_item,
		       retval);
  return retval;
}

static void
hide_winlist(GtkWidget *themenu)
{
  gtk_widget_unref(themenu);
}

static void
show_winlist(void)
{
  GtkWidget *themenu = make_winlist_menu();

  gtk_signal_connect(GTK_OBJECT(themenu), "hide",
		     GTK_SIGNAL_FUNC(hide_winlist), NULL);
  gtk_menu_popup(GTK_MENU(themenu), NULL, NULL, NULL,
		 NULL, 1, GDK_CURRENT_TIME);
}

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

  /*FIXME: different winlist types here */
  create_computer_winlist_widget(&winlist);

  return winlist;
}

/*these are commands sent over corba: */
void
change_orient(int id, int orient)
{
}

void
session_save(int id, const char *cfgpath, const char *globcfgpath)
{
  /*save the session here */
}

static gint
destroy_plug(GtkWidget *widget, gpointer data)
{
	gtk_exit(0);
	return FALSE;
}


int
main(int argc, char **argv)
{
  guint32 lastarg;
  GtkWidget *w_winlist;
  char *result;
  char *cfgpath;
  char *globcfgpath;

  char *myinvoc;
  guint32 winid;

  myinvoc = get_full_path(argv[0]);
  if(!myinvoc)
    return 1;

  panel_corba_register_arguments();
  gnome_init("winlist_applet", NULL, argc, argv, 0, &lastarg);

  winlist = g_hash_table_new(g_direct_hash, g_direct_compare);
  g_print("winlist = %p\n", winlist);
  init_winlist(argc, argv, lastarg);

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

  w_winlist = create_winlist_widget();
  gtk_widget_show(w_winlist);
  gtk_container_add(GTK_CONTAINER(plug), w_winlist);
  gtk_widget_show(plug);
  gtk_signal_connect(GTK_OBJECT(plug),"destroy",
		     GTK_SIGNAL_FUNC(destroy_plug),
		     NULL);

  result = gnome_panel_applet_register(plug, applet_id);
  if (result)
    g_error("Could not talk to the Panel: %s\n", result);

  applet_corba_gtk_main("IDL:GNOME/Applet:1.0");

  return 0;
}
