/*
 * GNOME FVWM window list module.
 * (C) 1998 Red Hat Software
 *
 * Author: Elliot Lee
 *
 */

#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <config.h>
#include <gnome.h>
#include <gdk/gdkx.h>
#include <applet-widget.h>

GtkWidget *applet = NULL;

/* We use this to create our window menu. The data is put here by the
   wm-dependant module */
GHashTable *winlist = NULL;

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

/*
 gtk_signal_connect(GTK_OBJECT(winlist), "destroy",
                    (GtkSignalFunc) destroy_winlist,
		    NULL);
*/
  return winlist;
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

int
main(int argc, char **argv)
{
  guint32 lastarg;
  GtkWidget *w_winlist;
  int i=0;
  poptContext ctx;

  applet_widget_init("winlist_applet", VERSION, argc, argv, NULL, 0,
			      NULL);

  winlist = g_hash_table_new(g_direct_hash, NULL);

  g_print("winlist = %p\n", winlist);

g_message("lastarg was %d\n",lastarg);

/*  init_winlist(argc, argv, lastarg); */
  init_winlist(argc, argv, 1);
 
g_message("Back from init_winlist");

  applet = applet_widget_new(argv[0]);
  if (!applet)
    g_error("Can't create applet!\n");

g_message("back from applet_widget_new");

#if 1
  w_winlist = create_winlist_widget();
  gtk_widget_show(w_winlist);
  applet_widget_add(APPLET_WIDGET(applet), w_winlist); 
#endif
  gtk_widget_show(applet);

g_message("Got window created and signals connected");

  applet_widget_gtk_main();
g_message("back from applet_widget_gtk_main");
  return 0;
}
