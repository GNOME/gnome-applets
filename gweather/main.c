/* $Id$ */

/*
 *  Papadimitriou Spiros <spapadim+@cs.cmu.edu>
 *
 *  This code released under the GNU GPL.
 *  Read the file COPYING for more information.
 *
 *  Main applet widget
 *
 */

#include <config.h>

#include <gnome.h>

#include "gweather-pref.h"
#include "gweather-dialog.h"
#include "gweather-applet.h"

#if 0
static gint save_yourself_cb (GnomeClient       *client,
                              gint               phase,
                              GnomeRestartStyle  save_style,
                              gint               shutdown,
                              GnomeInteractStyle interact_style,
                              gint               fast,
                              gpointer           client_data)
{
    gchar *argv[] = {NULL, NULL};
    gint argc = 1;

    argv[0] = (gchar *)client_data;

    gnome_client_set_restart_command(client, argc, argv);
    gnome_client_set_clone_command(client, 0, NULL);

    return TRUE;
}
#endif

int main (int argc, char *argv[])
{
#if 0
    GnomeClient *client;
#endif

    bindtextdomain (PACKAGE, GNOMELOCALEDIR);
    textdomain (PACKAGE);

    gweather_applet_create(argc, argv);
    gweather_dialog_create();

    gweather_pref_load();
    gweather_info_load();

#if 0
    if ((client = gnome_master_client()) != NULL)
        gtk_signal_connect (GTK_OBJECT(client), "save_yourself",
                            GTK_SIGNAL_FUNC(save_yourself_cb), (gpointer)argv[0]);
#endif

    gweather_update();

    applet_widget_gtk_main();

    gweather_pref_save();

    return 0;
}

