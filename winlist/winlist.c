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

GtkWidget *plug = NULL;

/* We use this to create our window menu. The data is put here by the
   wm-dependant module */
GHashTable *winlist = NULL;

int applet_id = (-1); /*this is our id we use to comunicate with the panel */

extern void
init_winlist(int argc, char **argv, int firstarg);
extern void
switch_to_window(guint win_id);
static error_t
parseAnArg (int key, char *arg, struct argp_state *state);


static struct argp_option options[] = {
	{ NULL, 0, NULL, 0, NULL, 0 }
};

static struct argp parser = {
	options, parseAnArg, N_("readfd writefd"),  NULL,  NULL, NULL, NULL
};


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

/*
 gtk_signal_connect(GTK_OBJECT(winlist), "destroy",
                    (GtkSignalFunc) destroy_winlist,
		    NULL);
*/
  return winlist;
}

/*these are commands sent over corba: */
void
change_orient(int id, int orient)
{
}

int
session_save(int id, const char *cfgpath, const char *globcfgpath)
{
	/*save the session here */
	return TRUE;
}

static gint
destroy_plug(GtkWidget *widget, gpointer data)
{
g_message("In quit_winlist");

	gtk_exit(0);
	return FALSE;
}

#if 0
void
shutdown_applet(int id)
{
  /*kill our plug using destroy to avoid warnings we need to
    kill the plug but we also need to return from this call */
g_message("In shutdown_applet");

  if (plug)
    gtk_widget_destroy(plug);
  gtk_idle_add(quit_winlist, NULL);
}
#endif


static error_t
parseAnArg (int key, char *arg, struct argp_state *state)
{
	gint val;
	
	/* We dont recognize the key they specified */
	if (key != ARGP_KEY_ARG)
		return ARGP_ERR_UNKNOWN;

#if 0	
	/* if they already specfied URL, we dont take more than one */
	if (fvwm)
		argp_usage (state);
	
	/* must be user specified URL */
	helpURL = g_strdup(arg);
	return 0;
#endif
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

  int i=0;

/*  while (!i);  */

  myinvoc = get_full_path(argv[0]);

  if(!myinvoc)
    return 1;

g_message("myinvoc -> %s",myinvoc);

  panel_corba_register_arguments();

g_message("argc is %d\n",argc);
for (lastarg = 0; lastarg < argc; lastarg++)
  g_message("argv[%d] -> %s",lastarg, argv[lastarg]);

  gnome_init("winlist_applet", &parser, argc, argv, 0, &lastarg);

  winlist = g_hash_table_new(g_direct_hash, NULL);

  g_print("winlist = %p\n", winlist);

g_message("lastarg was %d\n",lastarg);

/*  init_winlist(argc, argv, lastarg); */
  init_winlist(argc, argv, 1);
 
g_message("Back from init_winlist");

  if (!gnome_panel_applet_init_corba())
    g_error("Could not comunicate with the panel\n");

g_message("back from gnome_panel_applet_init_corba");

  result = gnome_panel_applet_request_id(myinvoc, &applet_id,
					 &cfgpath, &globcfgpath,
					 &winid);
g_message("gnome_panel_applet_request_id gave us:"
	  "\tapplet_id:\t%d"
	  "\tcfpath:\t%s"
	  "\tglobcfgpath:\t%s"
	  "\twinid:\t%d", applet_id, cfgpath, globcfgpath, winid);

  g_free(myinvoc);
  if (result)
    g_error("Could not talk to the Panel: %s\n", result);
  /*use cfg path for loading up data! */

g_message("Applet id is %d",applet_id);

  g_free(globcfgpath);
  g_free(cfgpath);

  plug = gtk_plug_new(winid);
#if 1
  w_winlist = create_winlist_widget();
  gtk_widget_show(w_winlist);
  gtk_container_add(GTK_CONTAINER(plug), w_winlist); 
#endif
  gtk_widget_show(plug);
  gtk_signal_connect(GTK_OBJECT(plug),"destroy",
		     GTK_SIGNAL_FUNC(destroy_plug),
		     NULL);

g_message("Got window created and signals connected");

  result = gnome_panel_applet_register(plug, applet_id);
  if (result)
    g_error("Could not talk to the Panel: %s\n", result);
g_message("gnome_panel_applet_register done");
  applet_corba_gtk_main("IDL:GNOME/Applet:1.0");
g_message("back from applet_corba_gtk_main");
  return 0;
}
