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
#include "properties-cpu.h"
#include "cpuload.h"

cpuload_properties cpu_props;

static guchar udata  [128];
static guchar oudata [128];
static guchar sdata  [128];
static guchar osdata [128];

static int
draw (gpointer data_ptr)
{
	int usr=0, sys=0, nice=0, free=0, i;
	Cpuload *c = (Cpuload *) data_ptr;

	GetLoad (cpu_props.height, &usr, &nice, &sys, &free );

	for (i=0; i < cpu_props.width-1; i++) {
		udata [i+1] = oudata [i];
		sdata [i+1] = osdata [i];
	}
	udata [0] = usr;
	sdata [0] = sys;
	
	/* Erase Rectangle */
	gdk_draw_rectangle (c->pixmap,
			    c->disp->style->black_gc,
			    TRUE, 0, 0,
			    c->disp->allocation.width,
			    c->disp->allocation.height);
	
	gdk_gc_set_foreground (c->gc, &c->ucolor);
	for (i=0; i < cpu_props.width; i++)	{
		if (udata[i])
			gdk_draw_line (c->pixmap,
				       c->gc,
				       i, cpu_props.height,
				       i, (cpu_props.height - udata[i]));
	}
	
	gdk_gc_set_foreground (c->gc, &c->scolor);
	for (i=0; i < cpu_props.width; i++) {
		if (sdata[i])
			gdk_draw_line (c->pixmap,
				       c->gc,
				       i, (cpu_props.height - udata[i]),
				       i, ((cpu_props.height - udata[i]) -
					   sdata[i]));
	}

	gdk_draw_pixmap (c->disp->window,
			 c->disp->style->fg_gc [GTK_WIDGET_STATE(c->disp)],
			 c->pixmap,
			 0, 0,
			 0, 0,
			 c->disp->allocation.width,
			 c->disp->allocation.height);

	for (i=0; i < cpu_props.width; i++) {
		oudata[i] = udata[i];
		osdata[i] = sdata[i];
	}
	return TRUE;
}

static gint
cpuload_configure (GtkWidget *widget, GdkEventConfigure *event,
		   gpointer data_ptr)
{
	Cpuload *c = (Cpuload *) data_ptr;

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
cpuload_expose (GtkWidget *widget, GdkEventExpose *event,
		gpointer data_ptr)
{
	Cpuload *c = (Cpuload *) data_ptr;
	
	gdk_draw_pixmap (widget->window,
			 widget->style->fg_gc [GTK_WIDGET_STATE(widget)],
			 c->pixmap,
			 event->area.x, event->area.y,
			 event->area.x, event->area.y,
			 event->area.width, event->area.height);
        return FALSE;
}

static Cpuload *
cpuload_new (void)
{
	GtkWidget *box;
	Cpuload *c;

	c = g_malloc (sizeof (Cpuload));
	memset (c, 0, sizeof (Cpuload));

	c->timer_index = -1;
	
	box = gtk_vbox_new (FALSE, FALSE);
	gtk_widget_show (box);

	c->frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (c->frame),
				   cpu_props.look?GTK_SHADOW_OUT:GTK_SHADOW_IN);

	c->disp = gtk_drawing_area_new ();
	gtk_signal_connect (GTK_OBJECT (c->disp), "expose_event",
			    (GtkSignalFunc)cpuload_expose, c);
        gtk_signal_connect (GTK_OBJECT(c->disp), "configure_event",
			    (GtkSignalFunc)cpuload_configure, c);
        gtk_widget_set_events (c->disp, GDK_EXPOSURE_MASK);

	gtk_box_pack_start_defaults (GTK_BOX (box), c->disp);
	gtk_container_add (GTK_CONTAINER (c->frame), box);

	gtk_widget_set_usize (c->disp, cpu_props.width, cpu_props.height);

	cpuload_start_timer (c);
        
        gtk_widget_show_all (c->frame);
	return c;
}

void
cpuload_start_timer (Cpuload *c)
{
	if (c->timer_index != -1)
		gtk_timeout_remove (c->timer_index);

	c->timer_index =
		gtk_timeout_add	(cpu_props.speed, (GtkFunction)draw, c);
}

void
cpuload_setup_colors (Cpuload *c)
{
	GdkColormap *colormap;

	colormap = gtk_widget_get_colormap (c->disp);
                
        gdk_color_parse (cpu_props.ucolor, &c->ucolor);
        gdk_color_alloc (colormap, &c->ucolor);

        gdk_color_parse (cpu_props.scolor, &c->scolor);
        gdk_color_alloc (colormap, &c->scolor);
}
	        
static void
create_gc (Cpuload *c)
{
        c->gc = gdk_gc_new (c->disp->window);
        gdk_gc_copy (c->gc, c->disp->style->white_gc);
}

static gint
applet_save_session (GtkWidget *widget, char *privcfgpath,
		     char *globcfgpath, gpointer data)
{
	save_cpu_properties (privcfgpath, &cpu_props);
	return FALSE;
}

/* start a new instance of the cpuload applet */
void
make_cpuload_applet (const gchar *goad_id)
{
        GtkWidget *applet;
        GtkWidget *label;
	Cpuload *cpuload;

        /* create a new applet_widget */
        applet = applet_widget_new(goad_id);
        /* in the rare case that the communication with the panel
           failed, error out */
        if (!applet)
                g_error ("Can't create applet!\n");

	fprintf (stderr, "make_cpuload_applet (%s): %p\n",
		 goad_id, applet);

        load_cpu_properties (APPLET_WIDGET(applet)->privcfgpath, &cpu_props);

        cpuload = cpuload_new ();
        applet_widget_add (APPLET_WIDGET(applet), cpuload->frame);
        gtk_widget_show (applet);

	create_gc (cpuload);
	cpuload_setup_colors (cpuload);

        gtk_signal_connect (GTK_OBJECT(applet),"save_session",
			    GTK_SIGNAL_FUNC(applet_save_session),
			    NULL);

       	applet_widget_register_stock_callback (APPLET_WIDGET(applet),
					       "properties",
					       GNOME_STOCK_MENU_PROP,
					       _("Properties..."),
					       cpu_properties,
					       cpuload);
}
