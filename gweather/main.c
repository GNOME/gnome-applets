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

#include "gweather-pref.h"
#include "gweather-dialog.h"
#include "gweather-applet.h"


int main (int argc, char *argv[])
{

    bindtextdomain (PACKAGE, GNOMELOCALEDIR);
    textdomain (PACKAGE);

    gweather_applet_create(argc, argv);

    gweather_pref_load();
    gweather_info_load();

    gweather_update();

    applet_widget_gtk_main();

    gweather_pref_save();

    return 0;
}

