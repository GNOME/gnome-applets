/* GNOME multiload panel applet
 * (C) 1997 The Free Software Foundation
 *
 * Authors: Tim P. Gerla
 *          Martin Baulig
 *
 * With code from wmload.c, v0.9.2, apparently by Ryan Land, rland@bc1.com.
 *
 */

#include <config.h>
#include <stdio.h>
#include <config.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <gnome.h>
#include <gdk/gdkx.h>
#include <applet-widget.h>
#include <libgnomeui/gnome-window-icon.h>

#include "global.h"

static const gchar *cpu_texts [4] = {
    N_("User"),  N_("System"),   N_("Nice"),  N_("Idle")
};

static const gchar *mem_texts [4] =  {
    N_("Other"), N_("Shared"), N_("Buffers"), N_("Free")
};

static const gchar *swap_texts [2] = {
    N_("Used"), N_("Free")
};

static const gchar *net_texts [4] = {
    N_("SLIP"), N_("PPP"), N_("ETH"), N_("Other"),
};

static const gchar *loadavg_texts [2] = {
    N_("Used"), N_("Free")
};

static const gchar *cpu_color_defs [4] = {
    "#ffffffff4fff", "#dfffdfffdfff",
    "#afffafffafff", "#000000000000"
};

static const gchar *mem_color_defs [4] = {
    "#bfffbfff4fff", "#efffefff4fff",
    "#afffafffafff", "#00008fff0000"
};

static const gchar *swap_color_defs [4] = {
    "#cfff5fff5fff", "#00008fff0000"
};

static const gchar *net_color_defs [4] = {
    "#64009500e0e0", "#d300d300d300", 
    "#00008fff0000", "#ffffffff4fff"
};

static const gchar *loadavg_color_defs [2] = {
    "#cfff5fff5fff", "#00008fff0000"
};

#define ADD_PROPERTIES(x,y) multiload_property_object_list = g_list_append (multiload_property_object_list, gnome_property_object_new (& ## x ## Property_Descriptor, &multiload_properties. ## y ##))
        
static GtkWidget *
make_new_applet (const gchar *goad_id)
{
    if (strstr (goad_id, "multiload_memload_applet"))
	return make_memload_applet (goad_id);
    else if (strstr (goad_id, "multiload_swapload_applet"))
	return make_swapload_applet (goad_id);
    else if (strstr (goad_id, "multiload_netload_applet"))
	return make_netload_applet (goad_id);
    else if (strstr (goad_id, "multiload_loadavg_applet"))
	return make_loadavg_applet (goad_id);
    else
	return make_cpuload_applet (goad_id);
}

/*when we get a command to start a new widget*/
static GtkWidget *
applet_start_new_applet (const gchar *goad_id, const char **params, int nparams)
{
    return make_new_applet (goad_id);
    params = NULL;
    nparams = 0;
}

/* run gtop (better location for this function?) */
void 
start_gtop_cb (AppletWidget *widget, gpointer data)
{
    gnome_execute_shell(NULL, "gtop");
    return;
    widget = NULL;
    data = NULL;
}
              
/* show help for the applet */
void
multiload_help_cb (AppletWidget *applet, gpointer data)
{
    GnomeHelpMenuEntry help_entry = { NULL, "index.html" };
    help_entry.name = data;
    gnome_help_display(NULL, &help_entry);
}


int
main (int argc, char **argv)
{
    const char *goad_id;
    GList *c;

    /* Initialize the i18n stuff */
    bindtextdomain (PACKAGE, GNOMELOCALEDIR);
    textdomain (PACKAGE);

    applet_widget_init ("multiload_applet", VERSION, argc, argv, NULL, 0, NULL);
    gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gnome-cpu.png");

    goad_id = goad_server_activation_id();
    if(!goad_id)
	return 0;

    /* Setup properties. */

    multiload_properties.cpuload.n = 4;
    multiload_properties.cpuload.name = "cpuload";
#ifdef ENABLE_NLS
    {
	int i;
	for (i=0;i<4;i++) cpu_texts[i]=_(cpu_texts[i]);
    }
#endif
    multiload_properties.cpuload.texts = cpu_texts;
    multiload_properties.cpuload.color_defs = cpu_color_defs;
    multiload_properties.cpuload.adj_data [0] = 500;
    multiload_properties.cpuload.adj_data [1] = 40;

    multiload_properties.memload.n = 4;
    multiload_properties.memload.name = "memload";
#ifdef ENABLE_NLS
    {
        int i;
        for (i=0;i<4;i++) mem_texts[i]=_(mem_texts[i]);
    }
#endif
    multiload_properties.memload.texts = mem_texts;
    multiload_properties.memload.color_defs = mem_color_defs;
    multiload_properties.memload.adj_data [0] = 500;
    multiload_properties.memload.adj_data [1] = 40;

    multiload_properties.swapload.n = 2;
    multiload_properties.swapload.name = "swapload";
#ifdef ENABLE_NLS
    {
        int i;
        for (i=0;i<2;i++) swap_texts[i]=_(swap_texts[i]);
    }
#endif
    multiload_properties.swapload.texts = swap_texts;
    multiload_properties.swapload.color_defs = swap_color_defs;
    multiload_properties.swapload.adj_data [0] = 500;
    multiload_properties.swapload.adj_data [1] = 40;

    multiload_properties.netload.n = 4;
    multiload_properties.netload.name = "netload";
#ifdef ENABLE_NLS
    {
        int i;
        for (i=0;i<3;i++) net_texts[i]=_(net_texts[i]);
    }
#endif
    multiload_properties.netload.texts = net_texts;
    multiload_properties.netload.color_defs = net_color_defs;
    multiload_properties.netload.adj_data [0] = 500;
    multiload_properties.netload.adj_data [1] = 40;

    multiload_properties.loadavg.n = 2;
    multiload_properties.loadavg.name = "loadavg";
#ifdef ENABLE_NLS
    {
        int i;
        for (i=0;i<2;i++) swap_texts[i]=_(swap_texts[i]);
    }
#endif
    multiload_properties.loadavg.texts = loadavg_texts;
    multiload_properties.loadavg.color_defs = loadavg_color_defs;
    multiload_properties.loadavg.adj_data [0] = 500;
    multiload_properties.loadavg.adj_data [1] = 40;
    multiload_properties.loadavg.adj_data [2] = 10;
#ifdef ENABLE_NLS
    {
        int i;
        for (i=0;i<2;i++) loadavg_texts[i]=_(loadavg_texts[i]);
    }
#endif


    /* Add property objects. */

    ADD_PROPERTIES (LoadGraph, cpuload);
    ADD_PROPERTIES (LoadGraph, memload);
    ADD_PROPERTIES (LoadGraph, swapload);
    ADD_PROPERTIES (LoadGraph, netload);
    ADD_PROPERTIES (LoadGraph, loadavg);

    /* This looks really ugly, but libgnomeui is already freezed so I can't
     * add new function there ... */

    c = g_list_nth (multiload_property_object_list, 0);
    ((GnomePropertyObject *) c->data)->label = gtk_label_new (_("CPU Load"));
    gtk_widget_ref (((GnomePropertyObject *) c->data)->label);

    c = g_list_nth (multiload_property_object_list, 1);
    ((GnomePropertyObject *) c->data)->label = gtk_label_new (_("Memory Load"));
    gtk_widget_ref (((GnomePropertyObject *) c->data)->label);

    c = g_list_nth (multiload_property_object_list, 2);
    ((GnomePropertyObject *) c->data)->label = gtk_label_new (_("Swap Load"));
    gtk_widget_ref (((GnomePropertyObject *) c->data)->label);

    c = g_list_nth (multiload_property_object_list, 3);
    ((GnomePropertyObject *) c->data)->label = gtk_label_new (_("Net Load"));
    gtk_widget_ref (((GnomePropertyObject *) c->data)->label);

    c = g_list_nth (multiload_property_object_list, 4);
    ((GnomePropertyObject *) c->data)->label = gtk_label_new (_("Load Average"));
    gtk_widget_ref (((GnomePropertyObject *) c->data)->label);

    /* Read properties. */
    multiload_init_properties ();

    applet_factory_new ("multiload_applet", NULL, applet_start_new_applet);

    /* Only do if factory wasn't requested. */
    if(goad_id && strcmp(goad_id, "multiload_applet"))
	make_new_applet(goad_id);

    applet_widget_gtk_main ();
	
    return 0;
}
