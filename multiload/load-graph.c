#include <config.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <gnome.h>
#include <gdk/gdkx.h>
#include <panel-applet.h>

#include "global.h"

static GList *object_list = NULL;

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
    if (!g->gc)
    {
		g->gc = gdk_gc_new (g->disp->window);
		gdk_gc_copy (g->gc, g->disp->style->black_gc);
    }

    /* Allocate colors. */
    if (!g->colors_allocated)
    {
		GdkColormap *colormap;

		colormap = gdk_window_get_colormap (g->disp->window);
		
		for (i = 0; i < g->n; i++)
	 	   gdk_color_alloc (colormap, &(g->colors [i]));

		g->colors_allocated = 1;
    }
	
    /* Erase Rectangle */
    gdk_draw_rectangle (g->pixmap,
			g->disp->style->black_gc,
			TRUE, 0, 0,
			g->draw_width,
			g->draw_height);

    for (i = 0; i < g->draw_width; i++)
		g->pos [i] = g->draw_height;

    for (j = 0; j < g->n; j++)
    {
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
		     g->draw_width,
		     g->draw_height);
	
    for (i = 0; i < g->draw_width; i++)
		memcpy (g->odata [i], g->data [i], g->data_size);
	
	return;
}

/* Updates the load graph when the timeout expires */
static int
load_graph_update (LoadGraph *g)
{
    guint i, j;

	if (g->tooltip_update)
		multiload_applet_tooltip_update(g);
	else		
	    g->get_data (g->draw_height, g->data [0], g);

    for (i=0; i < g->draw_width-1; i++)
		for (j=0; j < g->n; j++)
		    g->data [i+1][j] = g->odata [i][j];

    load_graph_draw (g);
    return TRUE;
}

void
load_graph_unalloc (LoadGraph *g)
{
    int i;
	gchar name[32];
	
    if (!g->allocated)
		return;

    for (i = 0; i < g->draw_width; i++)
    {
		g_free (g->data [i]);
		g_free (g->odata [i]);
    }

    g_free (g->data);
    g_free (g->odata);
    g_free (g->pos);

    g->pos = NULL;
    g->data = g->odata = NULL;
    
    g_snprintf(name, sizeof(name), "%s_size", g->name);
    g->size = panel_applet_gconf_get_int(g->applet, "size", NULL);
    g->size = MAX (g->size, 10);

    if (g->pixmap) {
		gdk_pixmap_unref (g->pixmap);
		g->pixmap = NULL;
    }

    g->allocated = FALSE;
}

static void
load_graph_alloc (LoadGraph *g)
{
    int pixel_size, i;

    if (g->allocated)
		return;

    g->show_frame = 1;

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

gint
load_graph_configure (GtkWidget *widget, GdkEventConfigure *event,
		      gpointer data_ptr)
{
    LoadGraph *c = (LoadGraph *) data_ptr;
	
    load_graph_unalloc (c);

    if (c->orient) {
    	c->draw_width = c->pixel_size - 2;
    	c->draw_height = c->size - 2;
    }
    else {
    	c->draw_width = c->size - 2;
    	c->draw_height = c->pixel_size - 2;
    }

    load_graph_alloc (c);

    if (!c->pixmap)
	c->pixmap = gdk_pixmap_new (c->disp->window,
				    c->draw_width,
				    c->draw_height,
				    gtk_widget_get_visual (c->disp)->depth);

    gdk_draw_rectangle (c->pixmap,
			widget->style->black_gc,
			TRUE, 0,0,
			c->draw_width,
			c->draw_height);
    gdk_draw_pixmap (widget->window,
		     c->disp->style->fg_gc [GTK_WIDGET_STATE(widget)],
		     c->pixmap,
		     0, 0,
		     0, 0,
		     c->draw_width,
		     c->draw_height);
	
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

    gtk_widget_destroy(widget);
    object_list = g_list_remove (object_list, g);
    return;
    widget = NULL;
}

void
load_graph_load_config (LoadGraph *g)
{
	
    gchar name [BUFSIZ], *temp;
    guint i;

	if (!g->colors)
		g->colors = g_new0(GdkColor, g->n);
		
	for (i = 0; i < g->n; i++)
	{
		g_snprintf(name, sizeof(name), "%s_color%d", g->name, i);
		temp = g_strdup_printf("%s", panel_applet_gconf_get_string(g->applet, name, NULL));
		gdk_color_parse(temp, &(g->colors[i]));
		g_free(temp);
	}

	return;	
}

LoadGraph *
load_graph_new (PanelApplet *applet, guint n, gchar *label,
		guint speed,
		guint size, 
		gboolean visible, 
		gchar *name,
		LoadGraphDataFunc get_data)
{
    LoadGraph *g;
    PanelAppletOrient orient;
    
    g = g_new0 (LoadGraph, 1);

    g->visible = visible;
    g->name = name;
    g->n = n;
    g->speed  = MAX (speed, 50);
    g->size   = MAX (size, 10);
    g->pixel_size = panel_applet_get_size (applet);
    g->tooltip_update = FALSE;
    
    g->applet = applet;
		
    g->main_widget = gtk_vbox_new (FALSE, FALSE);

    g->box = gtk_vbox_new (FALSE, FALSE);
    
    orient = panel_applet_get_orient (g->applet);
    switch (orient)
    {
    case PANEL_APPLET_ORIENT_UP:
    case PANEL_APPLET_ORIENT_DOWN:
    {
	g->orient = FALSE;
	break;
    }
    case PANEL_APPLET_ORIENT_LEFT:
    case PANEL_APPLET_ORIENT_RIGHT:
    {
	g->orient = TRUE;
	break;
    }
    default:
	g_assert_not_reached ();
    }
    
    if (g->orient) {
    	g->draw_width = g->pixel_size - 2;
    	g->draw_height = g->size - 2;
    }
    else {
    	g->draw_width = g->size - 2;
    	g->draw_height = g->pixel_size - 2;
    }

    load_graph_alloc (g);	
		
    if (g->show_frame)
    {
	g->frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (g->frame), GTK_SHADOW_ETCHED_IN);
	gtk_container_add (GTK_CONTAINER (g->frame), g->box);
	gtk_container_add (GTK_CONTAINER (g->main_widget), g->frame);
    }
    else
    {
	g->frame = NULL;
	gtk_container_add (GTK_CONTAINER (g->main_widget), g->box);
    }

    load_graph_load_config (g);

    g->get_data = get_data;

    g->timer_index = -1;


    gtk_widget_set_size_request (g->main_widget, g->pixel_size, g->size);
    gtk_widget_set_size_request (g->main_widget, g->size, g->pixel_size);

    g->tooltips = gtk_tooltips_new();
   	
    g->disp = gtk_drawing_area_new ();
    gtk_widget_set_events (g->disp, GDK_EXPOSURE_MASK | GDK_ENTER_NOTIFY_MASK 
    				    | GDK_LEAVE_NOTIFY_MASK);
	
    g_signal_connect (G_OBJECT (g->disp), "expose_event",
			G_CALLBACK (load_graph_expose), g);
    g_signal_connect (G_OBJECT(g->disp), "configure_event",
			G_CALLBACK (load_graph_configure), g);
    g_signal_connect (G_OBJECT(g->disp), "destroy",
			G_CALLBACK (load_graph_destroy), g);
	
    gtk_box_pack_start_defaults (GTK_BOX (g->box), g->disp);    
    gtk_widget_show_all(g->box);
    object_list = g_list_append (object_list, g);
    
    return g;
    label = NULL;
}

void
load_graph_start (LoadGraph *g)
{    
    if (g->timer_index != -1)
		gtk_timeout_remove (g->timer_index);

    g->timer_index = gtk_timeout_add (g->speed,
				      (GtkFunction) load_graph_update, g);
}

void
load_graph_stop (LoadGraph *g)
{
    if (g->timer_index != -1)
		gtk_timeout_remove (g->timer_index);
    
    g->timer_index = -1;
}
