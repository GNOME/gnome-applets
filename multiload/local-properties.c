#include "global.h"

static void multiload_show_local_properties (LocalPropData *);
static void multiload_local_properties_apply (GtkWidget *, gint, LocalPropData *);
static void multiload_local_properties_close (GtkWidget *, LocalPropData *);

void
multiload_local_properties_cb (AppletWidget *widget, gpointer data)
{
    LoadGraph *g = (LoadGraph *) data;

    multiload_show_local_properties (g->local_prop_data);
}

static void
multiload_local_properties_close (GtkWidget *widget, LocalPropData *d)
{
    gnome_property_object_list_walk (d->local_property_object_list,
				     GNOME_PROPERTY_ACTION_DISCARD_TEMP);
    
    d->win = NULL;
}

static void
multiload_local_properties_apply (GtkWidget *widget, gint page,
				  LocalPropData *d)
{
    gnome_property_object_list_walk (d->local_property_object_list,
				     GNOME_PROPERTY_ACTION_APPLY);

    gnome_property_object_list_walk (d->local_property_object_list,
				     GNOME_PROPERTY_ACTION_SAVE_TEMP);

    gnome_property_object_list_walk (d->local_property_object_list,
				     GNOME_PROPERTY_ACTION_SAVE);

    gnome_property_object_list_walk (d->local_property_object_list,
				     GNOME_PROPERTY_ACTION_UPDATE);

    gnome_property_object_list_walk (d->local_property_object_list,
				     GNOME_PROPERTY_ACTION_LOAD_TEMP);
}

void
multiload_local_properties_changed (LocalPropData *d)
{
    gnome_property_box_changed (GNOME_PROPERTY_BOX (d->win));
}

static void
multiload_show_local_properties (LocalPropData *d)
{
    static GnomeHelpMenuEntry help_entry = { NULL, "properties" };
    GList *c;

    help_entry.name = gnome_app_id;

    if (d->win) {
	gdk_window_raise (d->win->window);

	return;
    }
	
    d->win = gnome_property_box_new ();

    for (c = d->local_property_object_list; c; c = c->next) {
	GnomePropertyObject *object = c->data;

	gnome_property_object_register
	    (GNOME_PROPERTY_BOX (d->win), object);
    }

    gnome_property_object_list_walk (d->local_property_object_list,
				     GNOME_PROPERTY_ACTION_LOAD_TEMP);

    gtk_signal_connect (GTK_OBJECT (d->win), "destroy",
			GTK_SIGNAL_FUNC (multiload_local_properties_close),
			(gpointer) d);

    gtk_signal_connect (GTK_OBJECT (d->win), "apply",
			GTK_SIGNAL_FUNC (multiload_local_properties_apply),
			(gpointer) d);

    gtk_signal_connect (GTK_OBJECT (d->win), "destroy",
			GTK_SIGNAL_FUNC (multiload_local_properties_close),
			(gpointer) d);

    gtk_signal_connect (GTK_OBJECT (d->win), "help",
			GTK_SIGNAL_FUNC (gnome_help_pbox_display),
			&help_entry);

    gtk_widget_show_all (d->win);
}
