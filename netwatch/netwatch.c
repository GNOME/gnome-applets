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

typedef void (*NetwatchUpdateFunc) (GtkWidget *);

typedef struct {
   int                timeout;
   NetwatchUpdateFunc update_func;
   PanelOrientType    orient;
} NetwatchData;

GtkWidget  *plug = NULL;
GtkWidget  *netwatchw = NULL;
NetwatchData *nd = NULL;
int applet_id = (-1);

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
/*these are commands sent over corba:*/
void
change_orient(int id, int orient)
{
        PanelOrientType o = (PanelOrientType)orient;
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
        gtk_exit(0);
        return FALSE;
}

int
main(int argc, char **argv)
{
        char *result;
        char *cfgpath;
        char *globcfgpath;

        char *myinvoc;
        guint32 winid;

        myinvoc = get_full_path(argv[0]);
        if(!myinvoc)
                return 1;

        panel_corba_register_arguments();
        gnome_init("netwatch_applet", NULL, argc, argv, 0, NULL);

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

	netwatchw = create_netwatch_widget ();
	gtk_widget_show (netwatchw);
	gtk_container_add (GTK_CONTAINER (plug), netwatchw);
	gtk_widget_show (plug);
	gtk_signal_connect (GTK_OBJECT (plug), "destroy",
	                    GTK_SIGNAL_FUNC (destroy_plug),
	                    NULL);

        result = gnome_panel_applet_register(plug, applet_id);
        if (result)
                g_error("Could not talk to the Panel: %s\n", result);

        applet_corba_gtk_main("IDL:GNOME/Applet:1.0");

        return 0;
}

