#include <config.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <config.h>
#include <gnome.h>
#include <gdk/gdkx.h>
#include <applet-widget.h>

#include "global.h"

static GtkWidget *load_graph_properties_init (GnomePropertyObject *object);
static void load_graph_properties_changed (GnomePropertyObject *object);
static void load_graph_properties_update (GnomePropertyObject *object);
static void load_graph_properties_load (GnomePropertyObject *object);
static void load_graph_properties_save (GnomePropertyObject *object);

static GList *object_list = NULL;

typedef struct	_RadioButtonCbData		RadioButtonCbData;

struct _RadioButtonCbData
{
    GnomePropertyObject *object;
    GtkWidget *button, *color_frame, *data_frame;
    gint index;
};

GnomePropertyDescriptor LoadGraphProperty_Descriptor = {
    sizeof (LoadGraphProperties),
    N_("Load Graph"),
    load_graph_properties_init,
    NULL,
    load_graph_properties_update,
    load_graph_properties_load,
    load_graph_properties_save,
    NULL, NULL, NULL,
    load_graph_properties_changed,
    NULL
};

static GtkWidget *load_graph_local_properties_init (GnomePropertyObject *object);
static void load_graph_local_properties_changed (GnomePropertyObject *object);
static void load_graph_local_properties_update (GnomePropertyObject *object);
static void load_graph_local_properties_save (GnomePropertyObject *object);

GnomePropertyDescriptor LoadGraphLocalProperty_Descriptor = {
    sizeof (LoadGraphProperties),
    N_("Load Graph"),
    load_graph_local_properties_init,
    NULL,
    load_graph_local_properties_update,
    NULL,
    load_graph_local_properties_save,
    NULL, NULL, NULL,
    load_graph_local_properties_changed,
    NULL
};

/* Redraws the backing pixmap for the load graph and updates the window */
static void
load_graph_draw (LoadGraph *g)
{
    guint i, j;

    /* we might get called before the configure event so that
     * g->disp->allocation may not have the correct size
     * (after the user resized the applet in the prop dialog). */

    if (!g->pixmap)
	g->pixmap = gdk_pixmap_new (g->disp->window,
				    g->draw_width, g->draw_height,
				    gtk_widget_get_visual (g->disp)->depth);

    /* Create GC if necessary. */
    if (!g->gc) {
	g->gc = gdk_gc_new (g->disp->window);
	gdk_gc_copy (g->gc, g->disp->style->white_gc);
    }

    /* Allocate colors. */
    if (!g->colors_allocated) {
	GdkColormap *colormap;

	colormap = gdk_window_get_colormap (g->disp->window);
	for (i = 0; i < g->n; i++) {
	    g->colors [i] = g->prop_data_ptr->colors [i];
	    gdk_color_alloc (colormap, &(g->colors [i]));
	}

	g->colors_allocated = 1;
    }
	
    /* Erase Rectangle */
    gdk_draw_rectangle (g->pixmap,
			g->disp->style->black_gc,
			TRUE, 0, 0,
			g->disp->allocation.width,
			g->disp->allocation.height);

    for (i = 0; i < g->draw_width; i++)
	g->pos [i] = g->draw_height;

    for (j = 0; j < g->n; j++) {
	gdk_gc_set_foreground (g->gc, &(g->colors [j]));

	for (i = 0; i < g->draw_width; i++) {
	    gdk_draw_line (g->pixmap, g->gc,
			   g->draw_width - i, g->pos[i],
			   g->draw_width - i, g->pos[i] - g->data[i][j]);

	    g->pos [i] -= g->data [i][j];
	}
    }
	
    gdk_draw_pixmap (g->disp->window,
		     g->disp->style->fg_gc [GTK_WIDGET_STATE(g->disp)],
		     g->pixmap,
		     0, 0,
		     0, 0,
		     g->disp->allocation.width,
		     g->disp->allocation.height);

    for (i = 0; i < g->draw_width; i++)
	memcpy (g->odata [i], g->data [i], g->data_size);
}

/* Updates the load graph when the timeout expires */
static int
load_graph_update (LoadGraph *g)
{
    guint i, j;

    g->get_data (g->draw_height, g->data [0], g);

    for (i=0; i < g->draw_width-1; i++)
	for (j=0; j < g->n; j++)
	    g->data [i+1][j] = g->odata [i][j];

    load_graph_draw (g);
    return TRUE;
}

static void
load_graph_unalloc (LoadGraph *g)
{
    int i;

    if (!g->allocated)
	return;

    for (i = 0; i < g->draw_width; i++) {
	g_free (g->data [i]);
	g_free (g->odata [i]);
    }

    g_free (g->data);
    g_free (g->odata);
    g_free (g->pos);

    g->pos = NULL;
    g->data = g->odata = NULL;
    g->size = g->prop_data_ptr->adj_data [1];

    if (g->pixmap) {
	gdk_pixmap_unref (g->pixmap);
	g->pixmap = NULL;
    }

    g->allocated = FALSE;
}

static void
load_graph_alloc (LoadGraph *g)
{
    PanelOrientType orient;
    int pixel_size, i;

    if (g->allocated)
	return;

    orient = applet_widget_get_panel_orient (g->applet);
    switch (orient) {
    case ORIENT_UP:
    case ORIENT_DOWN:
	g->orient = FALSE;
	break;
    case ORIENT_LEFT:
    case ORIENT_RIGHT:
	g->orient = TRUE;
	break;
    default:
	g_assert_not_reached ();
    }

    pixel_size = applet_widget_get_panel_pixel_size (g->applet);
    g_assert (pixel_size > 0);

    g->pixel_size = pixel_size;

    g->show_frame = pixel_size > PIXEL_SIZE_SMALL;

    if (g->orient) {
	g->draw_width = g->pixel_size;
	g->draw_height = g->size;
    } else {
	g->draw_width = g->size;
	g->draw_height = g->pixel_size;
    }

    g->data = g_new0 (guint *, g->draw_width);
    g->odata = g_new0 (guint *, g->draw_width);
    g->pos = g_new0 (guint, g->draw_width);

    g->data_size = sizeof (guint) * g->n;

    for (i = 0; i < g->draw_width; i++) {
	g->data [i] = g_malloc0 (g->data_size);
	g->odata [i] = g_malloc0 (g->data_size);
    }

    g->allocated = TRUE;
}

static gint
load_graph_configure (GtkWidget *widget, GdkEventConfigure *event,
		      gpointer data_ptr)
{
    LoadGraph *c = (LoadGraph *) data_ptr;

    load_graph_unalloc (c);
    load_graph_alloc (c);

    if (!c->pixmap)
	c->pixmap = gdk_pixmap_new (widget->window,
				    widget->allocation.width,
				    widget->allocation.height,
				    gtk_widget_get_visual (c->disp)->depth);

    gdk_draw_rectangle (c->pixmap,
			widget->style->black_gc,
			TRUE, 0,0,
			widget->allocation.width,
			widget->allocation.height);
    gdk_draw_pixmap (widget->window,
		     c->disp->style->fg_gc [GTK_WIDGET_STATE(widget)],
		     c->pixmap,
		     0, 0,
		     0, 0,
		     c->disp->allocation.width,
		     c->disp->allocation.height);
    return TRUE;
    event = NULL;
}

static gint
load_graph_expose (GtkWidget *widget, GdkEventExpose *event,
		   gpointer data_ptr)
{
    LoadGraph *g = (LoadGraph *) data_ptr;
	
    gdk_draw_pixmap (widget->window,
		     widget->style->fg_gc [GTK_WIDGET_STATE(widget)],
		     g->pixmap,
		     event->area.x, event->area.y,
		     event->area.x, event->area.y,
		     event->area.width, event->area.height);
    return FALSE;
}

static void
load_graph_destroy (GtkWidget *widget, gpointer data_ptr)
{
    LoadGraph *g = (LoadGraph *) data_ptr;

    load_graph_stop (g);

    object_list = g_list_remove (object_list, g);
    return;
    widget = NULL;
}

static void
applet_pixel_size_changed_cb (GtkWidget *applet, int size, LoadGraph *g)
{
    load_graph_unalloc (g);

    g->pixel_size = size;

    gtk_widget_ref (g->box);

    gtk_container_remove (GTK_CONTAINER (g->box->parent), g->box);

    if (g->frame) {
	gtk_widget_destroy (g->frame);
	g->frame = NULL;
    }

    load_graph_alloc (g);

    if (g->show_frame) {
	g->frame = gtk_frame_new (NULL);
	gtk_container_add (GTK_CONTAINER (g->frame), g->box);
	gtk_container_add (GTK_CONTAINER (g->main_widget), g->frame);
	gtk_widget_show (g->frame);
    } else {
	gtk_container_add (GTK_CONTAINER (g->main_widget), g->box);
    }

    gtk_widget_unref (g->box);

    if (g->orient)
	gtk_widget_set_usize (g->main_widget, g->pixel_size, g->size);
    else
	gtk_widget_set_usize (g->main_widget, g->size, g->pixel_size);
}

static gint
applet_orient_changed_cb (GtkWidget *applet, gpointer data, LoadGraph *g)
{
    load_graph_unalloc (g);
    load_graph_alloc (g);

    if (g->orient)
	gtk_widget_set_usize (g->main_widget, g->pixel_size, g->size);
    else
	gtk_widget_set_usize (g->main_widget, g->size, g->pixel_size);

    return FALSE;
}

static gint
applet_save_session_cb (GtkWidget *w, const char *privcfgpath,
			const char *globcfgpath, LoadGraph *g)
{
    gchar name [BUFSIZ], temp [BUFSIZ];
    guint i;

    gnome_config_push_prefix (privcfgpath);

    for (i=0; i < g->prop_data->n; i++) {
	GdkColor *color = &(g->prop_data->colors [i]);

	g_snprintf (temp, sizeof(temp), "#%04x%04x%04x",
		 color->red, color->green, color->blue);
	
	g_snprintf (name, sizeof(name), "%s/color%d", g->prop_data->name, i);
	gnome_config_set_string (name, temp);
    }

    g_snprintf (name, sizeof(name), "%s/speed", g->prop_data->name);
    gnome_config_set_int (name, g->prop_data->adj_data [0]);

    g_snprintf (name, sizeof(name), "%s/size", g->prop_data->name);
    gnome_config_set_int (name, g->prop_data->adj_data [1]);

    g_snprintf (name, sizeof(name), "%s/maximum", g->prop_data->name);
    gnome_config_set_int (name, g->prop_data->adj_data [2]);

    g_snprintf (name, sizeof(name), "%s/use_default", g->prop_data->name);
    gnome_config_set_int (name, g->prop_data->use_default);

    g_snprintf (name, sizeof(name), "%s/loadavg_type", g->prop_data->name);
    gnome_config_set_int (name, g->prop_data->loadavg_type);

    gnome_config_pop_prefix();

    gnome_config_sync();
    /* you need to use the drop_all here since we're all writing to
       one file, without it, things might not work too well */
    gnome_config_drop_all ();

    /* make sure you return FALSE, otherwise your applet might not
       work compeltely, there are very few circumstances where you
       want to return TRUE. This behaves similiar to GTK events, in
       that if you return FALSE it means that you haven't done
       everything yourself, meaning you want the panel to save your
       other state such as the panel you are on, position,
       parameter, etc ... */

    return FALSE;
}

static void
applet_load_config (LoadGraph *g)
{
    gchar name [BUFSIZ], *temp;
    guint i;

    gnome_config_push_prefix (g->applet->privcfgpath);

    if (!g->prop_data->colors)
	g->prop_data->colors = g_new0 (GdkColor, g->prop_data->n);

    for (i=0; i < g->prop_data->n; i++) {
	GdkColor *color = &(g->prop_data->colors [i]);
	
	g_snprintf (name, sizeof(name), "%s/color%d=%s",
		 g->prop_data->name, i, g->prop_data->color_defs [i]);

	temp = gnome_config_get_string (name);
	gdk_color_parse (temp, color);
	g_free (temp);
    }

    g_snprintf (name, sizeof(name), "%s/speed=%ld",
	     g->prop_data->name, g->prop_data->adj_data [0]);
    g->prop_data->adj_data [0] = gnome_config_get_int (name);

    g_snprintf (name, sizeof(name), "%s/size=%ld",
	     g->prop_data->name, g->prop_data->adj_data [1]);
    g->prop_data->adj_data [1] = gnome_config_get_int (name);

    g_snprintf (name, sizeof(name), "%s/maximum=%ld",
	     g->prop_data->name, g->prop_data->adj_data [2]);
    g->prop_data->adj_data [2] = gnome_config_get_int (name);

    g_snprintf (name, sizeof(name), "%s/use_default=1", g->prop_data->name);
    g->prop_data->use_default = gnome_config_get_int (name);

    g_snprintf (name, sizeof(name), "%s/loadavg_type=%d",
	     g->prop_data->name, g->prop_data->loadavg_type);
    g->prop_data->loadavg_type = gnome_config_get_int (name);

    if (g->prop_data->use_default)
	g->prop_data_ptr = g->global_prop_data;
    else
	g->prop_data_ptr = g->prop_data;

    gnome_config_pop_prefix ();
}

LoadGraph *
load_graph_new (AppletWidget *applet, guint n, gchar *label,
		LoadGraphProperties *global_prop_data,
		LoadGraphProperties *prop_data, guint speed,
		guint size, LoadGraphDataFunc get_data,
		gchar *help_path)
{
    LoadGraph *g;

    g = g_new0 (LoadGraph, 1);

    g->applet = applet;

    g->n = n;
    g->prop_data = prop_data;
    g->global_prop_data = global_prop_data;
    applet_load_config (g);

    g->local_prop_data = g_new0 (LocalPropData, 1);
    g->local_prop_data->help_path = help_path; /* no need to dup */

    g->local_prop_data->applet = g->applet;

    g->local_prop_data->property_object = gnome_property_object_new
	(&LoadGraphLocalProperty_Descriptor, g->prop_data);

    g->local_prop_data->property_object->user_data = g->local_prop_data;

    g->local_prop_data->local_property_object_list = g_list_append
	(NULL, g->local_prop_data->property_object);

    if (g->global_prop_data == &multiload_properties.cpuload)
	g->prop_data->type = PROP_CPULOAD;
    else if (g->global_prop_data == &multiload_properties.memload)
	g->prop_data->type = PROP_MEMLOAD;
    else if (g->global_prop_data == &multiload_properties.swapload)
	g->prop_data->type = PROP_SWAPLOAD;
    else if (g->global_prop_data == &multiload_properties.netload)
	g->prop_data->type = PROP_NETLOAD;
    else if (g->global_prop_data == &multiload_properties.loadavg)
	g->prop_data->type = PROP_LOADAVG;
    else
	g_assert_not_reached();

    g->local_prop_data->type = g->prop_data->type;

    g->speed  = speed;
    g->size   = size;

    g->get_data = get_data;

    g->colors = g_new0 (GdkColor, g->n);

    g->timer_index = -1;
	
    load_graph_alloc (g);

    g->main_widget = gtk_vbox_new (FALSE, FALSE);
    gtk_widget_show (g->main_widget);

    g->box = gtk_vbox_new (FALSE, FALSE);
    gtk_widget_show (g->box);

    if (g->show_frame) {
	g->frame = gtk_frame_new (NULL);
	gtk_container_add (GTK_CONTAINER (g->frame), g->box);
	gtk_container_add (GTK_CONTAINER (g->main_widget), g->frame);
    } else {
	g->frame = NULL;
	gtk_container_add (GTK_CONTAINER (g->main_widget), g->box);
    }

    g->disp = gtk_drawing_area_new ();
    gtk_signal_connect (GTK_OBJECT (g->disp), "expose_event",
			(GtkSignalFunc)load_graph_expose, g);
    gtk_signal_connect (GTK_OBJECT(g->disp), "configure_event",
			(GtkSignalFunc)load_graph_configure, g);
    gtk_signal_connect (GTK_OBJECT(g->disp), "destroy",
			(GtkSignalFunc)load_graph_destroy, g);
    gtk_widget_set_events (g->disp, GDK_EXPOSURE_MASK);

    gtk_box_pack_start_defaults (GTK_BOX (g->box), g->disp);

    gtk_signal_connect (GTK_OBJECT (applet), "change_orient",
			GTK_SIGNAL_FUNC (applet_orient_changed_cb),
			(gpointer) g);

    gtk_signal_connect (GTK_OBJECT (applet),
			"change_pixel_size",
			GTK_SIGNAL_FUNC (applet_pixel_size_changed_cb),
			(gpointer) g);

    gtk_signal_connect (GTK_OBJECT (applet), "save_session",
			GTK_SIGNAL_FUNC (applet_save_session_cb),
			(gpointer) g);

    if (g->orient)
	gtk_widget_set_usize (g->main_widget, g->pixel_size, g->size);
    else
	gtk_widget_set_usize (g->main_widget, g->size, g->pixel_size);

    object_list = g_list_append (object_list, g);

    gtk_widget_show_all (g->main_widget);

    return g;
    label = NULL;
}

void
load_graph_start (LoadGraph *g)
{
    if (g->timer_index != -1)
	gtk_timeout_remove (g->timer_index);

    g->timer_index = gtk_timeout_add (g->prop_data_ptr->adj_data [0],
				      (GtkFunction) load_graph_update, g);
}

void
load_graph_stop (LoadGraph *g)
{
    if (g->timer_index != -1)
	gtk_timeout_remove (g->timer_index);
    
    g->timer_index = -1;
}

static GtkWidget *
load_graph_properties_init (GnomePropertyObject *object)
{
    GtkWidget *vb, *frame;
    /* GtkWidget *label, *entry, *button, *table, *spin; */
    LoadGraphProperties *prop_data = object->prop_data;
    /* guint i; */

    static const gchar *adj_data_texts [3] = {
	N_("Interval:"), N_("Size:"), N_("Maximum:")
    };

    static glong adj_data_descr [3*8] = {
	1, 0, 0, 1, G_MAXINT, 1, 256, 256,
	1, 0, 0, 1, G_MAXINT, 1, 256, 256,
	1, 0, 0, 1, G_MAXINT, 1, 256, 256
    };

    vb = gtk_vbox_new (FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (vb), GNOME_PAD_SMALL);

    frame = gnome_property_entry_colors
	(object, _("Colors"), prop_data->n,
	 prop_data->n, NULL, prop_data->colors,
	 prop_data->texts);

    gtk_container_add (GTK_CONTAINER (vb), frame);

    if (object->prop_data == &multiload_properties.loadavg)
	frame = gnome_property_entry_adjustments
	    (object, NULL, 3, 3, 2, NULL, adj_data_texts,
	     adj_data_descr, prop_data->adj_data);
    else
	frame = gnome_property_entry_adjustments
	    (object, NULL, 2, 2, 2, NULL, adj_data_texts,
	     adj_data_descr, prop_data->adj_data);

    gtk_container_add (GTK_CONTAINER (vb), frame);

    return vb;
}

static void
load_graph_properties_load (GnomePropertyObject *object)
{
    LoadGraphProperties *prop_data = object->prop_data;
    gchar name [BUFSIZ], *temp;
    guint i;

    if (!prop_data->colors)
	prop_data->colors = g_new0 (GdkColor, prop_data->n);

    for (i=0; i < prop_data->n; i++) {
	GdkColor *color = &(prop_data->colors [i]);
	
	g_snprintf (name, sizeof(name), "multiload/%s/color%d=%s",
		 prop_data->name, i, prop_data->color_defs [i]);

	temp = gnome_config_get_string (name);
	gdk_color_parse (temp, color);
	g_free (temp);
    }

    g_snprintf (name, sizeof(name), "multiload/%s/speed=%ld",
	     prop_data->name, prop_data->adj_data [0]);
    prop_data->adj_data [0] = gnome_config_get_int (name);

    g_snprintf (name, sizeof(name), "multiload/%s/size=%ld",
	     prop_data->name, prop_data->adj_data [1]);
    prop_data->adj_data [1] = gnome_config_get_int (name);

    g_snprintf (name, sizeof(name), "multiload/%s/maximum=%ld",
	     prop_data->name, prop_data->adj_data [2]);
    prop_data->adj_data [2] = gnome_config_get_int (name);

    g_snprintf (name, sizeof(name), "multiload/%s/loadavg_type=%d",
	     prop_data->name, prop_data->loadavg_type);
    prop_data->loadavg_type = gnome_config_get_int (name);
}

static void
load_graph_properties_save (GnomePropertyObject *object)
{
    LoadGraphProperties *prop_data = object->prop_data;
    gchar name [BUFSIZ], temp [BUFSIZ];
    guint i;

    for (i=0; i < prop_data->n; i++) {
	GdkColor *color = &(prop_data->colors [i]);

	g_snprintf (temp, sizeof(temp), "#%04x%04x%04x",
		 color->red, color->green, color->blue);
	
	g_snprintf (name, sizeof(name), "multiload/%s/color%d", prop_data->name, i);
	gnome_config_set_string (name, temp);
    }

    g_snprintf (name, sizeof(name), "multiload/%s/speed", prop_data->name);
    gnome_config_set_int (name, prop_data->adj_data [0]);

    g_snprintf (name, sizeof(name), "multiload/%s/size", prop_data->name);
    gnome_config_set_int (name, prop_data->adj_data [1]);

    g_snprintf (name, sizeof(name), "multiload/%s/maximum", prop_data->name);
    gnome_config_set_int (name, prop_data->adj_data [2]);

    g_snprintf (name, sizeof(name), "multiload/%s/loadavg_type", prop_data->name);
    gnome_config_set_int (name, prop_data->loadavg_type);
}

static void
load_graph_properties_changed (GnomePropertyObject *object)
{
    multiload_properties_changed ();
    return;
    object = NULL;
}

static void
load_graph_properties_update (GnomePropertyObject *object)
{
    GList *c;

    for (c = object_list; c; c = c->next) {
	LoadGraph *g = (LoadGraph *) c->data;

	/* Remember not to use `object->prop_data' here,
	 * `g->prop_data' is what you want !
	 *
	 * To make this clear:
	 *
	 * This function is called for each GnomePropertyObject
	 * (currently cpuload, memload, swapload), but this
	 * block loops over each real applet.
	 *
	 * So if you have for instance only a cpuload applet running
	 * this function is called three times (for cpuload, memload
	 * and swapload properties), but `g' will always point to
	 * the running cpuload applet and thus `g->prop_data' will
	 * always point to the cpuload properties while `prop_data'
	 * will point to the cpuload, memload and swapload properties
	 * in each invocation of this function.
	 *
	 * Feb 22, Martin Baulig
	 */

	if (g->prop_data->use_default)
	    g->prop_data_ptr = g->global_prop_data;
	else
	    g->prop_data_ptr = g->prop_data;

	if (g->colors_allocated) {
		GdkColormap *colormap;

		colormap = gdk_window_get_colormap (g->disp->window);
		gdk_colormap_free_colors (colormap, g->colors, g->n);

		g->colors_allocated = 0;
	}

	if (g->timer_index != -1) {
	    load_graph_stop (g);
	    load_graph_start (g);
	}

	load_graph_unalloc (g);
	load_graph_alloc (g);

	if (g->orient)
	    gtk_widget_set_usize (g->main_widget, g->pixel_size, g->size);
	else
	    gtk_widget_set_usize (g->main_widget, g->size, g->pixel_size);

	load_graph_draw (g);
    }
    return;
    object = NULL;
}

static void
use_defaults_cb (GtkWidget *widget, RadioButtonCbData *cb_data)
{
    LoadGraphProperties *prop_ptr = cb_data->object->temp_data;

    prop_ptr->use_default = GTK_TOGGLE_BUTTON (cb_data->button)->active;

    gtk_widget_set_sensitive (cb_data->color_frame, !prop_ptr->use_default);
    gtk_widget_set_sensitive (cb_data->data_frame, !prop_ptr->use_default);

    multiload_local_properties_changed (cb_data->object->user_data);
}

static GtkWidget *
load_graph_local_properties_init (GnomePropertyObject *object)
{
    GtkWidget *color_frame, *data_frame;
    GtkWidget *vb, *frame, *button, *table;
    LoadGraphProperties *prop_data = object->prop_data;
    RadioButtonCbData *cb_data;

    static const gchar *adj_data_texts [3] = {
	N_("Speed:"), N_("Size:"), N_("Maximum:")
    };

    static glong adj_data_descr [3*8] = {
	1, 0, 0, 1, G_MAXINT, 1, 256, 256,
	1, 0, 0, 1, G_MAXINT, 1, 256, 256,
	1, 0, 0, 1, G_MAXINT, 1, 256, 256
    };

    vb = gtk_vbox_new (FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (vb), GNOME_PAD_SMALL);

    color_frame = gnome_property_entry_colors
	(object, _("Colors"), prop_data->n,
	 prop_data->n, NULL, prop_data->colors,
	 prop_data->texts);

    gtk_widget_set_sensitive (color_frame, !prop_data->use_default);

    gtk_container_add (GTK_CONTAINER (vb), color_frame);

    if (prop_data->type == PROP_LOADAVG)
	data_frame = gnome_property_entry_adjustments
	    (object, NULL, 3, 3, 2, NULL, adj_data_texts,
	     adj_data_descr, prop_data->adj_data);
    else
	data_frame = gnome_property_entry_adjustments
	    (object, NULL, 2, 2, 2, NULL, adj_data_texts,
	     adj_data_descr, prop_data->adj_data);

    gtk_widget_set_sensitive (data_frame, !prop_data->use_default);

    gtk_container_add (GTK_CONTAINER (vb), data_frame);

    frame = gtk_frame_new (NULL);

    table = gtk_table_new (1, 1, FALSE);
    gtk_table_set_col_spacings (GTK_TABLE (table), GNOME_PAD);

    button = gtk_check_button_new_with_label (_("Use default properties"));
    if (prop_data->use_default)
	gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (button), TRUE);

    cb_data = g_new0 (RadioButtonCbData, 1);
    cb_data->button = button;
    cb_data->object = object;
    cb_data->color_frame = color_frame;
    cb_data->data_frame = data_frame;

    gtk_signal_connect
	(GTK_OBJECT (button), "toggled", use_defaults_cb, cb_data);

    gtk_table_attach_defaults (GTK_TABLE (table), button, 0, 1, 0, 1);

    gtk_container_add (GTK_CONTAINER (frame), table);
    gtk_box_pack_start (GTK_BOX (vb), frame, FALSE, TRUE, 0);

    return vb;
}

static void
load_graph_local_properties_save (GnomePropertyObject *object)
{
    LocalPropData *d = object->user_data;

    applet_widget_sync_config (d->applet);
}

static void
load_graph_local_properties_changed (GnomePropertyObject *object)
{
    multiload_local_properties_changed (object->user_data);
}

static void
load_graph_local_properties_update (GnomePropertyObject *object)
{
    GList *c;

    for (c = object_list; c; c = c->next) {
	LoadGraph *g = (LoadGraph *) c->data;

	/* Remember not to use `object->prop_data' here,
	 * `g->prop_data' is what you want !
	 *
	 * To make this clear:
	 *
	 * This function is called for each GnomePropertyObject
	 * (currently cpuload, memload, swapload), but this
	 * block loops over each real applet.
	 *
	 * So if you have for instance only a cpuload applet running
	 * this function is called three times (for cpuload, memload
	 * and swapload properties), but `g' will always point to
	 * the running cpuload applet and thus `g->prop_data' will
	 * always point to the cpuload properties while `prop_data'
	 * will point to the cpuload, memload and swapload properties
	 * in each invocation of this function.
	 *
	 * Feb 22, Martin Baulig
	 */

	if (g->prop_data->use_default)
	    g->prop_data_ptr = g->global_prop_data;
	else
	    g->prop_data_ptr = g->prop_data;

	if (g->colors_allocated) {
		GdkColormap *colormap;

		colormap = gdk_window_get_colormap (g->disp->window);
		gdk_colormap_free_colors (colormap, g->colors, g->n);

		g->colors_allocated = 0;
	}

	if (g->timer_index != -1) {
	    load_graph_stop (g);
	    load_graph_start (g);
	}

	load_graph_unalloc (g);
	load_graph_alloc (g);

	if (g->orient)
	    gtk_widget_set_usize (g->main_widget, g->pixel_size, g->size);
	else
	    gtk_widget_set_usize (g->main_widget, g->size, g->pixel_size);

	load_graph_draw (g);
    }
    return;
    object = NULL;
}
