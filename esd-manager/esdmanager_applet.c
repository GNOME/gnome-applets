/*
 * ESD Manager Applet
 * (C) 1998 Red Hat Software
 *
 * Author: The Rasterman (Carsten Haitzler)
 */

#include <esd.h>
#include <gnome.h>
#include <gdk/gdkx.h>
#include <gdk/gdkprivate.h>
#include <applet-widget.h>

void                cb_about(AppletWidget * widget, gpointer data);
void                cb_properties_dialog(AppletWidget * widget, gpointer data);
void                cb_applet_change_orient(GtkWidget *w, PanelOrientType o, gpointer data);

GtkWidget          *applet;
PanelOrientType     orient;

void
cb_about(AppletWidget * widget, gpointer data)
{
  GtkWidget          *about;
  const gchar        *authors[] =
    {"The Rasterman", NULL};
  
  about = gnome_about_new
    (_("Esound MAnager Applet"), "0.1", _("Copyright (C)1998 Red Hat Software"),
     authors,
     _("This does nothing useful"),
     NULL);
  gtk_widget_show(about);
  data = NULL;
  widget = NULL;
}

void 
cb_applet_change_orient(GtkWidget *w, PanelOrientType o, gpointer data)
{
  orient = o;
  switch (o) {
   case ORIENT_UP:
   case ORIENT_DOWN:
   case ORIENT_LEFT:
   case ORIENT_RIGHT:
  }
  w = NULL;
  data = NULL;
}

void
cb_properties_dialog(AppletWidget * widget, gpointer data)
{
  data = NULL;
  widget = NULL;
}

int 
main(int argc, char *argv[])
{
  panel_corba_register_arguments();
  
  applet_widget_init_defaults("esdmanager_applet", NULL, argc, argv,
  			      NULL, 0, NULL);

  applet = applet_widget_new("esdmanager_applet");
  if (!applet)
    g_error("Can't create applet!\n");

  gtk_widget_realize(applet);
  gtk_widget_show(applet);

  applet_widget_register_stock_callback(APPLET_WIDGET(applet),
					"about",
					GNOME_STOCK_MENU_ABOUT,
					_("About..."),
					cb_about,
					NULL);

  applet_widget_register_stock_callback(APPLET_WIDGET(applet),
					"properties",
					GNOME_STOCK_MENU_PROP,
					_("Properties..."),
					cb_properties_dialog,
					NULL);

  gtk_signal_connect(GTK_OBJECT(applet), "change_orient",
		     GTK_SIGNAL_FUNC(cb_applet_change_orient),
		     NULL);
  
  applet_widget_gtk_main();

  return 0;
}
