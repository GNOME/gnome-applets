#include "global.h"
#include <libgnomeui/gnome-window-icon.h>

GList *multiload_property_object_list = NULL;

MultiLoadProperties multiload_properties;

static GtkWidget *win = NULL;

void
multiload_properties_cb (AppletWidget *widget, gpointer data)
{
    LoadGraph *g;

    g = data;

    if (g->global_prop_data == &multiload_properties.cpuload)
	multiload_show_properties (PROP_CPULOAD);
    else if (g->global_prop_data == &multiload_properties.memload)
	multiload_show_properties (PROP_MEMLOAD);
    else if (g->global_prop_data == &multiload_properties.swapload)
	multiload_show_properties (PROP_SWAPLOAD);
    else if (g->global_prop_data == &multiload_properties.netload)
	multiload_show_properties (PROP_NETLOAD);
    else if (g->global_prop_data == &multiload_properties.loadavg)
	multiload_show_properties (PROP_LOADAVG);
    else
	g_assert_not_reached();
    return;
    widget = NULL;
}

void
multiload_properties_close (void)
{
    gnome_property_object_list_walk (multiload_property_object_list,
				     GNOME_PROPERTY_ACTION_DISCARD_TEMP);
    
    win = NULL;
}

void
multiload_properties_apply (void)
{
    gnome_property_object_list_walk (multiload_property_object_list,
				     GNOME_PROPERTY_ACTION_APPLY);

    gnome_property_object_list_walk (multiload_property_object_list,
				     GNOME_PROPERTY_ACTION_SAVE_TEMP);

    gnome_property_object_list_walk (multiload_property_object_list,
				     GNOME_PROPERTY_ACTION_SAVE);

    gnome_property_object_list_walk (multiload_property_object_list,
				     GNOME_PROPERTY_ACTION_UPDATE);

    gnome_property_object_list_walk (multiload_property_object_list,
				     GNOME_PROPERTY_ACTION_LOAD_TEMP);
}

void
multiload_properties_changed (void)
{
    gnome_property_box_changed (GNOME_PROPERTY_BOX (win));
}

static void
phelp_cb (GtkWidget *w, gint tab, gpointer data)
{
	GnomeHelpMenuEntry help_entry = { NULL, NULL };
	char *das_names[] = {
		"cpuload_applet",
		"memload_applet",
		"swapload_applet",
		"netload_applet",
		"loadavg_applet"
	};
	char *das_pages[] = {
		"index.html#CPULOAD-PROPERTIES",
		"index.html#MEMLOAD-PROPERTIES",
		"index.html#SWAPLOAD-PROPERTIES",
		"index.html#NETLOAD-PROPERTIES",
		"index.html#LOADAVG-PROPERTIES"
	};
	help_entry.name = das_names[tab];
	help_entry.path = das_pages[tab];
	gnome_help_display(NULL, &help_entry);
}

void
multiload_show_properties (PropertyClass prop_class)
{
    GList *c;

    if (win) {
	gdk_window_raise (win->window);

	return;
    }
	
    win = gnome_property_box_new ();
    if (prop_class == PROP_MEMLOAD || prop_class == PROP_SWAPLOAD)
	    gnome_window_icon_set_from_file (GTK_WINDOW (win),
					     GNOME_ICONDIR"/gnome-mem.png");

    for (c = multiload_property_object_list; c; c = c->next) {
	GnomePropertyObject *object = c->data;

	gnome_property_object_register
	    (GNOME_PROPERTY_BOX (win), object);
    }

    gnome_property_object_list_walk (multiload_property_object_list,
				     GNOME_PROPERTY_ACTION_LOAD_TEMP);

    gtk_signal_connect (GTK_OBJECT (win), "apply",
			GTK_SIGNAL_FUNC (multiload_properties_apply), NULL);

    gtk_signal_connect (GTK_OBJECT (win), "destroy",
			GTK_SIGNAL_FUNC (multiload_properties_close), NULL);

    gtk_signal_connect (GTK_OBJECT (win), "help",
			GTK_SIGNAL_FUNC (phelp_cb), NULL);

    gtk_widget_show_all (win);
    gtk_notebook_set_page (GTK_NOTEBOOK (GNOME_PROPERTY_BOX (win)->notebook), prop_class);
}

void
multiload_init_properties (void)
{
}
