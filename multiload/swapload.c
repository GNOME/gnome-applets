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
#include "properties-swap.h"
#include "swapload.h"

swapload_properties swap_props;

static guchar udata  [128];
static guchar oudata [128];
static guchar fdata  [128];
static guchar ofdata [128];

static int
draw (gpointer data_ptr)
{
	int used=0, free=0, i;
	Swapload *c = (Swapload *) data_ptr;

	GetSwap (swap_props.height, &used, &free );

	for (i=0; i < swap_props.width-1; i++) {
		udata [i+1] = oudata [i];
		fdata [i+1] = ofdata [i];
	}
	udata [0] = used;
	fdata [0] = free;
	
	/* Erase Rectangle */
	gdk_draw_rectangle (c->pixmap,
			    c->disp->style->black_gc,
			    TRUE, 0, 0,
			    c->disp->allocation.width,
			    c->disp->allocation.height);
	
	gdk_gc_set_foreground (c->gc, &c->ucolor);
	for (i=0; i < swap_props.width; i++)	{
		if (udata[i])
			gdk_draw_line (c->pixmap,
				       c->gc,
				       i, swap_props.height,
				       i, (swap_props.height - udata[i]));
	}
	
	gdk_gc_set_foreground (c->gc, &c->fcolor);
	for (i=0; i < swap_props.width; i++) {
		if (fdata[i])
			gdk_draw_line (c->pixmap,
				       c->gc,
				       i, (swap_props.height - udata[i]),
				       i, ((swap_props.height - udata[i]) -
					   fdata[i]));
	}

	gdk_draw_pixmap (c->disp->window,
			 c->disp->style->fg_gc [GTK_WIDGET_STATE(c->disp)],
			 c->pixmap,
			 0, 0,
			 0, 0,
			 c->disp->allocation.width,
			 c->disp->allocation.height);

	for (i=0; i < swap_props.width; i++) {
		oudata[i] = udata[i];
		ofdata[i] = fdata[i];
	}
	return TRUE;
}

static gint
swapload_configure (GtkWidget *widget, GdkEventConfigure *event,
		   gpointer data_ptr)
{
	Swapload *c = (Swapload *) data_ptr;

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
swapload_expose (GtkWidget *widget, GdkEventExpose *event,
		gpointer data_ptr)
{
	Swapload *c = (Swapload *) data_ptr;
	
	gdk_draw_pixmap (widget->window,
			 widget->style->fg_gc [GTK_WIDGET_STATE(widget)],
			 c->pixmap,
			 event->area.x, event->area.y,
			 event->area.x, event->area.y,
			 event->area.width, event->area.height);
        return FALSE;
}

static Swapload *
swapload_new (void)
{
	GtkWidget *box;
	Swapload *c;

	c = g_malloc (sizeof (Swapload));
	memset (c, 0, sizeof (Swapload));

	c->timer_index = -1;
	
	box = gtk_vbox_new (FALSE, FALSE);
	gtk_widget_show (box);

	c->frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (c->frame),
				   swap_props.look?GTK_SHADOW_OUT:GTK_SHADOW_IN);

	c->disp = gtk_drawing_area_new ();
	gtk_signal_connect (GTK_OBJECT (c->disp), "expose_event",
			    (GtkSignalFunc)swapload_expose, c);
        gtk_signal_connect (GTK_OBJECT(c->disp), "configure_event",
			    (GtkSignalFunc)swapload_configure, c);
        gtk_widget_set_events (c->disp, GDK_EXPOSURE_MASK);

	gtk_box_pack_start_defaults (GTK_BOX (box), c->disp);
	gtk_container_add (GTK_CONTAINER (c->frame), box);

	gtk_widget_set_usize (c->disp, swap_props.width, swap_props.height);

	swapload_start_timer (c);
        
        gtk_widget_show_all (c->frame);
	return c;
}

void
swapload_start_timer (Swapload *c)
{
	if (c->timer_index != -1)
		gtk_timeout_remove (c->timer_index);

	c->timer_index =
		gtk_timeout_add	(swap_props.speed, (GtkFunction)draw, c);
}

void
swapload_setup_colors (Swapload *c)
{
	GdkColormap *colormap;

	colormap = gtk_widget_get_colormap (c->disp);
                
        gdk_color_parse (swap_props.ucolor, &c->ucolor);
        gdk_color_alloc (colormap, &c->ucolor);

        gdk_color_parse (swap_props.fcolor, &c->fcolor);
        gdk_color_alloc (colormap, &c->fcolor);
}
	        
static void
create_gc (Swapload *c)
{
        c->gc = gdk_gc_new (c->disp->window);
        gdk_gc_copy (c->gc, c->disp->style->white_gc);
}

static gint
applet_save_session (GtkWidget *widget, char *privcfgpath,
		     char *globcfgpath, gpointer data)
{
	save_swap_properties (privcfgpath, &swap_props);
	return FALSE;
}

/* start a new instance of the swapload applet */
void
make_swapload_applet (const gchar *param)
{
        GtkWidget *applet;
        GtkWidget *label;
	Swapload *swapload;

        /* create a new applet_widget */
        applet = applet_widget_new_with_param (param);
        /* in the rare case that the communication with the panel
           failed, error out */
        if (!applet)
                g_error ("Can't create applet!\n");

	fprintf (stderr, "make_swapload_applet (%s): %p\n",
		 param, applet);

        load_swap_properties (APPLET_WIDGET(applet)->privcfgpath, &swap_props);

        swapload = swapload_new ();
        applet_widget_add (APPLET_WIDGET(applet), swapload->frame);
        gtk_widget_show (applet);

	create_gc (swapload);
	swapload_setup_colors (swapload);

        gtk_signal_connect (GTK_OBJECT(applet),"save_session",
			    GTK_SIGNAL_FUNC(applet_save_session),
			    NULL);

       	applet_widget_register_stock_callback (APPLET_WIDGET(applet),
					       "properties",
					       GNOME_STOCK_MENU_PROP,
					       _("Properties..."),
					       swap_properties,
					       swapload);
}
