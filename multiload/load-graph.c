#include <stdio.h>
#include <config.h>
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

/* Redraws the backing pixmap for the load graph and updates the window */
static void
load_graph_draw (LoadGraph *g)
{
    gint i, j;

    /* we might get called before the configure event so that
     * g->disp->allocation may not have the correct size
     * (after the user resized the applet in the prop dialog). */

    if (!g->pixmap)
	g->pixmap = gdk_pixmap_new (g->disp->window,
				    g->width, g->height,
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
	    g->colors [i] = g->prop_data->colors [i];
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

    for (i = 0; i < g->width; i++)
	g->pos [i] = g->height;

    for (j = 0; j < g->n; j++) {
	gdk_gc_set_foreground (g->gc, &(g->colors [j]));

	for (i = 0; i < g->width; i++) {
	    gdk_draw_line (g->pixmap, g->gc,
			   g->width - i, g->pos[i],
			   g->width - i, g->pos[i] - g->data[i][j]);

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

    for (i = 0; i < g->width; i++)
	memcpy (g->odata [i], g->data [i], g->data_size);
}

/* Updates the load graph when the timeout expires */
static int
load_graph_update (LoadGraph *g)
{
    gint i, j;

    g->get_data (g->height, g->data [0]);

    for (i=0; i < g->width-1; i++)
	for (j=0; j < g->n; j++)
	    g->data [i+1][j] = g->odata [i][j];

    load_graph_draw (g);
    return TRUE;
}

static gint
load_graph_configure (GtkWidget *widget, GdkEventConfigure *event,
		      gpointer data_ptr)
{
    LoadGraph *c = (LoadGraph *) data_ptr;

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
}

static GtkWidget *
load_graph_properties_init (GnomePropertyObject *object)
{
    GtkWidget *vb, *frame, *label, *entry, *button, *table, *spin;
    LoadGraphProperties *prop_data = object->prop_data;
    guint i;

    static const gchar *adj_data_texts [3] = {
	N_("Speed:"), N_("Width:"), N_("Height:")
    };

    static glong adj_data_descr [3*8] = {
	1, 0, 0, 1, INT_MAX, 1, 256, 256,
	1, 0, 0, 1, INT_MAX, 1, 256, 256,
	1, 0, 0, 0, INT_MAX, 1, 256, 256
    };

    vb = gtk_vbox_new (FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (vb), GNOME_PAD_SMALL);

    frame = gnome_property_entry_colors
	(object, _("Colors"), prop_data->n,
	 prop_data->n, NULL, prop_data->colors,
	 prop_data->texts);

    gtk_container_add (GTK_CONTAINER (vb), frame);

    frame = gnome_property_entry_adjustments
	(object, NULL, 3, 3, 2, NULL, adj_data_texts,
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
	
	sprintf (name, "multiload/%s/color%d=%s",
		 prop_data->name, i, prop_data->color_defs [i]);

	temp = gnome_config_get_string (name);
	gdk_color_parse (temp, color);
	g_free (temp);
    }

    sprintf (name, "multiload/%s/speed=%ld",
	     prop_data->name, prop_data->adj_data [0]);
    prop_data->adj_data [0] = gnome_config_get_int (name);

    sprintf (name, "multiload/%s/width=%ld",
	     prop_data->name, prop_data->adj_data [1]);
    prop_data->adj_data [1] = gnome_config_get_int (name);

    sprintf (name, "multiload/%s/height=%ld",
	     prop_data->name, prop_data->adj_data [2]);
    prop_data->adj_data [2] = gnome_config_get_int (name);

}

static void
load_graph_properties_save (GnomePropertyObject *object)
{
    LoadGraphProperties *prop_data = object->prop_data;
    gchar name [BUFSIZ], temp [BUFSIZ];
    guint i;

    for (i=0; i < prop_data->n; i++) {
	GdkColor *color = &(prop_data->colors [i]);

	sprintf (temp, "#%04x%04x%04x",
		 color->red, color->green, color->blue);
	
	sprintf (name, "multiload/%s/color%d", prop_data->name, i);
	gnome_config_set_string (name, temp);
    }

    sprintf (name, "multiload/%s/speed", prop_data->name);
    gnome_config_set_int (name, prop_data->adj_data [0]);

    sprintf (name, "multiload/%s/width", prop_data->name);
    gnome_config_set_int (name, prop_data->adj_data [1]);

    sprintf (name, "multiload/%s/height", prop_data->name);
    gnome_config_set_int (name, prop_data->adj_data [2]);
}

static void
load_graph_properties_changed (GnomePropertyObject *object)
{
    multiload_properties_changed ();
}

static void
load_graph_properties_update (GnomePropertyObject *object)
{
    GList *c;
    gint i;

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

	if (g->width != g->prop_data->adj_data [1]) {
	    /* User changed width. */

	    for (i = 0; i < g->width; i++) {
		g_free (g->data [i]);
		g_free (g->odata [i]);
	    }

	    g_free (g->data);
	    g_free (g->odata);
	    g_free (g->pos);

	    g->width = g->prop_data->adj_data [1];

	    g->data = g_new0 (guint *, g->width);
	    g->odata = g_new0 (guint *, g->width);
	    g->pos = g_new0 (guint, g->width);

	    g->data_size = sizeof (guint) * g->n;

	    for (i = 0; i < g->width; i++) {
		g->data [i] = g_malloc0 (g->data_size);
		g->odata [i] = g_malloc0 (g->data_size);
	    }

	    if (g->pixmap) {
		gdk_pixmap_unref (g->pixmap);
		g->pixmap = NULL;
	    }

	    gtk_widget_set_usize (g->disp, g->width, g->height);
	}

	if (g->height != g->prop_data->adj_data [2]) {
	    if (g->pixmap) {
		gdk_pixmap_unref (g->pixmap);
		g->pixmap = NULL;
	    }

	    g->height = g->prop_data->adj_data [2];

	    gtk_widget_set_usize (g->disp, g->width, g->height);
	}

	load_graph_draw (g);
    }
}

LoadGraph *
load_graph_new (guint n, gchar *label, LoadGraphProperties *prop_data,
		guint speed, guint width, guint height,
		LoadGraphDataFunc get_data)
{
    GtkWidget *box;
    LoadGraph *g;
    guint i;

    g = g_new0 (LoadGraph, 1);

    g->n = n;
    g->prop_data = prop_data;

    g->speed  = speed;
    g->width  = width;
    g->height = height;

    g->get_data = get_data;

    g->data = g_new0 (guint *, g->width);
    g->odata = g_new0 (guint *, g->width);
    g->pos = g_new0 (guint, g->width);

    g->data_size = sizeof (guint) * g->n;

    for (i = 0; i < g->width; i++) {
	g->data [i] = g_malloc0 (g->data_size);
	g->odata [i] = g_malloc0 (g->data_size);
    }

    g->colors = g_new0 (GdkColor, g->n);

    g->timer_index = -1;
	
    box = gtk_vbox_new (FALSE, FALSE);
    gtk_widget_show (box);

    g->frame = gtk_frame_new (NULL);

    g->disp = gtk_drawing_area_new ();
    gtk_signal_connect (GTK_OBJECT (g->disp), "expose_event",
			(GtkSignalFunc)load_graph_expose, g);
    gtk_signal_connect (GTK_OBJECT(g->disp), "configure_event",
			(GtkSignalFunc)load_graph_configure, g);
    gtk_signal_connect (GTK_OBJECT(g->disp), "destroy",
			(GtkSignalFunc)load_graph_destroy, g);
    gtk_widget_set_events (g->disp, GDK_EXPOSURE_MASK);

    gtk_box_pack_start_defaults (GTK_BOX (box), g->disp);
    gtk_container_add (GTK_CONTAINER (g->frame), box);

    gtk_widget_set_usize (g->disp, g->width, g->height);

    object_list = g_list_append (object_list, g);

    gtk_widget_show_all (g->frame);
    return g;
}

void
load_graph_start (LoadGraph *g)
{
    if (g->timer_index != -1)
	gtk_timeout_remove (g->timer_index);

    g->timer_index = gtk_timeout_add (g->prop_data->adj_data [0],
				      (GtkFunction) load_graph_update, g);
}

void
load_graph_stop (LoadGraph *g)
{
    if (g->timer_index != -1)
	gtk_timeout_remove (g->timer_index);
    
    g->timer_index = -1;
}
