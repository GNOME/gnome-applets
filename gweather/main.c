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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>
#include <applet-widget.h>

#include "http.h"

#include "gweather-pref.h"
#include "gweather-dialog.h"
#include "gweather-applet.h"


int main (int argc, char *argv[])
{

    bindtextdomain (PACKAGE, GNOMELOCALEDIR);
    textdomain (PACKAGE);

#if 0
    g_thread_init(NULL);
    http_init();
#endif

    gweather_applet_create(argc, argv);

    gweather_pref_load(APPLET_WIDGET(gweather_applet)->privcfgpath);
    gweather_info_load(APPLET_WIDGET(gweather_applet)->privcfgpath);

    gweather_update();

    applet_widget_gtk_main();

    http_done();

    return 0;
}

