#include "global.h"

GList *multiload_property_object_list = NULL;

MultiLoadProperties multiload_properties;

static GtkWidget *win = NULL;

void
multiload_properties_cb (AppletWidget *widget, gpointer data)
{
    multiload_show_properties ();
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

void
multiload_show_properties (void)
{
    GList *c;

    if (win) {
	gdk_window_raise (win->window);

	return;
    }
	
    win = gnome_property_box_new ();

    for (c = multiload_property_object_list; c; c = c->next) {
	GnomePropertyObject *object = c->data;

	gnome_property_object_register
	    (GNOME_PROPERTY_BOX (win), object);
    }

    gnome_property_object_list_walk (multiload_property_object_list,
				     GNOME_PROPERTY_ACTION_LOAD_TEMP);

    gtk_signal_connect (GTK_OBJECT (win), "apply",
			GTK_SIGNAL_FUNC(multiload_properties_apply), NULL);

    gtk_signal_connect (GTK_OBJECT (win), "destroy",
			GTK_SIGNAL_FUNC(multiload_properties_close), NULL);

    gtk_widget_show_all (win);
}

void
multiload_init_properties (void)
{
}
