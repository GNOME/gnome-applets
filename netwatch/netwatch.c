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
#include "applet-widget.h"

typedef void (*NetwatchUpdateFunc) (GtkWidget *);

typedef struct {
   int                timeout;
   NetwatchUpdateFunc update_func;
   PanelOrientType    orient;
} NetwatchData;

GtkWidget  *applet = NULL;
GtkWidget  *netwatchw = NULL;
NetwatchData *nd = NULL;

static void
free_data (GtkWidget *widget, gpointer data)
{
   g_free (data);
}

static int
netwatch_timeout_callback (gpointer data)
{
   return 1;
}

static void
destroy_netwatch (GtkWidget *widget, void *data)
{
   gtk_timeout_remove (nd->timeout);
   g_free (nd);
}

static GtkWidget *
create_netwatch_widget (void)
{
   GtkWidget *parent_widget;
   GtkWidget *label;

   nd = g_new (NetwatchData, 1);
   nd->orient = ORIENT_UP;
   parent_widget = gtk_hbox_new (FALSE, 0);
   label = gtk_label_new ("Netwatch!");
   gtk_widget_show (label);
   gtk_container_add (GTK_CONTAINER (parent_widget), label);
   gtk_signal_connect (GTK_OBJECT (parent_widget), "destroy",
                       (GtkSignalFunc) destroy_netwatch,
                       NULL);
   return parent_widget;
}

static gint
destroy_applet(GtkWidget *widget, gpointer data)
{
        gtk_exit(0);
        return FALSE;
}

int
main(int argc, char **argv)
{
        panel_corba_register_arguments();
        gnome_init("netwatch_applet", NULL, argc, argv, 0, NULL);

	applet = applet_widget_new(argv[0]);
	if (!applet)
		g_error("Can't create applet!\n");

	netwatchw = create_netwatch_widget ();
	gtk_widget_show (netwatchw);
	applet_widget_add (APPLET_WIDGET (applet), netwatchw);
	gtk_widget_show (applet);
	gtk_signal_connect (GTK_OBJECT (applet), "destroy",
	                    GTK_SIGNAL_FUNC (destroy_applet),
	                    NULL);

        applet_widget_gtk_main();

        return 0;
}

