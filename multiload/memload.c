/* GNOME cpuload/memload panel applet
 * (C) 1997 The Free Software Foundation
 *
 * Authors: Tim P. Gerla
 *          Martin Baulig
 *
 * With code from wmload.c, v0.9.2, apparently by Ryan Land, rland@bc1.com.
 *
 */

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

#include "linux-proc.h"
#include "properties-mem.h"
#include "memload.h"

memload_properties mem_props;

static guchar udata  [128];
static guchar oudata [128];
static guchar sdata  [128];
static guchar osdata [128];
static guchar bdata  [128];
static guchar obdata [128];
static guchar cdata  [128];
static guchar ocdata [128];

static int
draw (gpointer data_ptr)
{
	int used=0, shared=0, buffer=0, cached=0, i;
	Memload *c = (Memload *) data_ptr;

	GetMemory (mem_props.height, &used, &shared, &buffer, &cached);

	for (i=0; i < mem_props.width-1; i++) {
		udata [i+1] = oudata [i];
		sdata [i+1] = osdata [i];
		bdata [i+1] = obdata [i];
		cdata [i+1] = ocdata [i];
	}
	udata [0] = used;
	sdata [0] = shared;
	bdata [0] = buffer;
	cdata [0] = cached;
	
	/* Erase Rectangle */
	gdk_draw_rectangle (c->pixmap,
			    c->disp->style->black_gc,
			    TRUE, 0, 0,
			    c->disp->allocation.width,
			    c->disp->allocation.height);
	
	gdk_gc_set_foreground (c->gc, &c->ucolor);
	for (i=0; i < mem_props.width; i++)	{
		if (udata[i])
			gdk_draw_line (c->pixmap,
				       c->gc,
				       i, mem_props.height,
				       i, mem_props.height - udata[i]);
	}
	
	gdk_gc_set_foreground (c->gc, &c->scolor);
	for (i=0; i < mem_props.width; i++) {
		if (sdata[i])
			gdk_draw_line (c->pixmap,
				       c->gc,
				       i, (mem_props.height - udata[i]),
				       i, ((mem_props.height -
					    (udata[i] + sdata[i]))));
	}

	gdk_gc_set_foreground (c->gc, &c->bcolor);
	for (i=0; i < mem_props.width; i++) {
		if (bdata[i])
			gdk_draw_line (c->pixmap,
				       c->gc,
				       i, (mem_props.height -
					   (udata[i] + sdata[i])),
				       i, (mem_props.height -
					   (udata[i] + sdata[i] + bdata[i])));
	}
	
	gdk_gc_set_foreground (c->gc, &c->ccolor);
	for (i=0; i < mem_props.width; i++) {
		if (cdata[i])
			gdk_draw_line (c->pixmap,
				       c->gc,
				       i, (mem_props.height -
					   (udata[i] + sdata[i] + bdata[i])),
				       i, (mem_props.height -
					   (udata [i] + sdata[i] +
					    bdata [i] + cdata[i])));
	}

	gdk_draw_pixmap (c->disp->window,
			 c->disp->style->fg_gc [GTK_WIDGET_STATE(c->disp)],
			 c->pixmap,
			 0, 0,
			 0, 0,
			 c->disp->allocation.width,
			 c->disp->allocation.height);

	for (i=0; i < mem_props.width; i++) {
		oudata[i] = udata[i];
		osdata[i] = sdata[i];
		obdata[i] = bdata[i];
		ocdata[i] = cdata[i];
	}
	return TRUE;
}

static gint
memload_configure (GtkWidget *widget, GdkEventConfigure *event,
		   gpointer data_ptr)
{
	Memload *c = (Memload *) data_ptr;

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
memload_expose (GtkWidget *widget, GdkEventExpose *event,
		gpointer data_ptr)
{
	Memload *c = (Memload *) data_ptr;
	
	gdk_draw_pixmap (widget->window,
			 widget->style->fg_gc [GTK_WIDGET_STATE(widget)],
			 c->pixmap,
			 event->area.x, event->area.y,
			 event->area.x, event->area.y,
			 event->area.width, event->area.height);
        return FALSE;
}

static Memload *
memload_new (void)
{
	GtkWidget *box;
	Memload *c;

	c = g_malloc (sizeof (Memload));
	memset (c, 0, sizeof (Memload));

	c->timer_index = -1;
	
	box = gtk_vbox_new (FALSE, FALSE);
	gtk_widget_show (box);

	c->frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (c->frame),
				   mem_props.look?GTK_SHADOW_OUT:GTK_SHADOW_IN);

	c->disp = gtk_drawing_area_new ();
	gtk_signal_connect (GTK_OBJECT (c->disp), "expose_event",
			    (GtkSignalFunc)memload_expose, c);
        gtk_signal_connect (GTK_OBJECT(c->disp), "configure_event",
			    (GtkSignalFunc)memload_configure, c);
        gtk_widget_set_events (c->disp, GDK_EXPOSURE_MASK);

	gtk_box_pack_start_defaults (GTK_BOX (box), c->disp);
	gtk_container_add (GTK_CONTAINER (c->frame), box);

	gtk_widget_set_usize (c->disp, mem_props.width, mem_props.height);

	memload_start_timer (c);
        
        gtk_widget_show_all (c->frame);
	return c;
}

void
memload_start_timer (Memload *c)
{
	if (c->timer_index != -1)
		gtk_timeout_remove (c->timer_index);

	c->timer_index =
		gtk_timeout_add	(mem_props.speed, (GtkFunction)draw, c);
}

void
memload_setup_colors (Memload *c)
{
	GdkColormap *colormap;

	colormap = gtk_widget_get_colormap (c->disp);
                
        gdk_color_parse (mem_props.ucolor, &c->ucolor);
        gdk_color_alloc (colormap, &c->ucolor);

        gdk_color_parse (mem_props.scolor, &c->scolor);
        gdk_color_alloc (colormap, &c->scolor);

        gdk_color_parse (mem_props.bcolor, &c->bcolor);
        gdk_color_alloc (colormap, &c->bcolor);

        gdk_color_parse (mem_props.ccolor, &c->ccolor);
        gdk_color_alloc (colormap, &c->ccolor);
}
	        
static void
create_gc (Memload *c)
{
        c->gc = gdk_gc_new (c->disp->window);
        gdk_gc_copy (c->gc, c->disp->style->white_gc);
}

static gint
applet_save_session (GtkWidget *widget, char *privcfgpath,
		     char *globcfgpath, gpointer data)
{
	save_mem_properties (privcfgpath, &mem_props);
	return FALSE;
}

/* start a new instance of the memload applet */
void
make_memload_applet (const gchar *goad_id)
{
        GtkWidget *applet;
        GtkWidget *label;
	Memload *memload;

        /* create a new applet_widget */
        applet = applet_widget_new(goad_id);
        /* in the rare case that the communication with the panel
           failed, error out */
        if (!applet)
                g_error ("Can't create applet!\n");

        load_mem_properties (APPLET_WIDGET(applet)->privcfgpath, &mem_props);

        memload = memload_new ();
        applet_widget_add (APPLET_WIDGET(applet), memload->frame);
        gtk_widget_show (applet);

	create_gc (memload);
	memload_setup_colors (memload);

        gtk_signal_connect (GTK_OBJECT(applet),"save_session",
			    GTK_SIGNAL_FUNC(applet_save_session),
			    NULL);

       	applet_widget_register_stock_callback (APPLET_WIDGET(applet),
					       "properties",
					       GNOME_STOCK_MENU_PROP,
					       _("Properties..."),
					       mem_properties,
					       memload);
}
