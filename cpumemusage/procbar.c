#include <gnome.h>
#include "procbar.h"

#define A (w->allocation)

static void
procbar_expose (GtkWidget *w, GdkEventExpose *e, ProcBar *pb)
{
	/* printf ("procbar expose %d %d %d %d\n",
	e->area.x, e->area.y,
	e->area.width, e->area.height);

	printf ("%d\n", pb->bs);
	printf ("procbar expose %d %d %d\n",
	w->window,
	w->style->black_gc,
	pb->bs); */


	gdk_window_copy_area (w->window,
			      w->style->black_gc,
			      e->area.x, e->area.y,
			      pb->bs,
			      e->area.x, e->area.y,
			      e->area.width, e->area.height);
}

static void
procbar_size_request (GtkWidget *w, GtkRequisition *r, ProcBar *pb)
{
	if (!pb->first_request) {
		r->width = w->allocation.width;
		r->height = w->allocation.height;
	}
	pb->first_request = 0;
}

static gint
procbar_configure (GtkWidget *w, GdkEventConfigure *e, ProcBar *pb)
{
	gint i;
	
	if(w->allocation.width == 0 || w->allocation.height == 0)
		return TRUE;

	/* printf ("procbar allocate %d %d\n",
	w->allocation.width, w->allocation.height); */

	if (!pb->colors_allocated) {
		GdkColormap *cmap;

		cmap = gdk_window_get_colormap (pb->bar->window);
		for (i=0; i<pb->n; i++)
			gdk_color_alloc (cmap, &pb->colors [i]);

		pb->colors_allocated = 1;
	}


	if (pb->bs) {
		gdk_pixmap_unref (pb->bs);
		pb->bs = NULL;
	}

	pb->bs = gdk_pixmap_new (w->window,
				 w->allocation.width,
				 w->allocation.height,
				 -1);

	/* printf ("%d\n", pb->bs); */

	return TRUE;
}

#undef A

ProcBar *
procbar_new (GtkWidget *label, gint n, GdkColor *colors, gint (*cb)())
{
	ProcBar *pb;

	pb = g_new (ProcBar, 1);

	pb->cb = cb;
	pb->tag = -1;
	pb->n = n;
	pb->first_request = 1;
	pb->colors = colors;
	pb->colors_allocated = 0;
	pb->vertical = FALSE;

	pb->last = g_new (unsigned, n+1);
	pb->last [0] = 0;

	pb->label = label;
	pb->hbox = gtk_hbox_new (FALSE, GNOME_PAD_SMALL);
	pb->bar = gtk_drawing_area_new ();
	pb->frame = gtk_frame_new (NULL);

	pb->bs = NULL;

	gtk_frame_set_shadow_type (GTK_FRAME (pb->frame), GTK_SHADOW_IN);

	gtk_widget_set_events (pb->bar, GDK_EXPOSURE_MASK);
	gtk_signal_connect (GTK_OBJECT (pb->bar), "expose_event",
			    (GtkSignalFunc) procbar_expose, (gpointer) pb);
	gtk_signal_connect (GTK_OBJECT (pb->bar), "configure_event",
			    (GtkSignalFunc) procbar_configure, (gpointer) pb);
	gtk_signal_connect (GTK_OBJECT (pb->bar), "size_request",
			    (GtkSignalFunc) procbar_size_request, (gpointer) pb);

	gtk_container_add (GTK_CONTAINER (pb->frame), pb->bar);

	if (label)
		gtk_box_pack_start (GTK_BOX (pb->hbox), label, FALSE, TRUE, 0);
	gtk_box_pack_start_defaults (GTK_BOX (pb->hbox), pb->frame);

	if (label)
		gtk_widget_show (pb->label);

	gtk_widget_show (pb->bar);
	gtk_widget_show (pb->frame);
	gtk_widget_show (pb->hbox);

	return pb;
}

void      
procbar_set_orient  (ProcBar *pb, gboolean vertical)
{
	pb->vertical = vertical;
}

#define W (pb->bar)
#define A ((pb->bar)->allocation)

void
procbar_set_values (ProcBar *pb, unsigned val [])
{
	unsigned tot = val [0];
	gint i;
	gint change = 0;
	gint offset;
	gint lengthr, length;
	GdkGC *gc;

	if (!GTK_WIDGET_REALIZED (pb->bar) ||
	    A.width == 0 || A.height == 0)
		return;

	/* check if values changed */

	for (i=0; i<pb->n+1; i++) {
		if (val[i] != pb->last [i]) {
			change = 1;
			break;
		}
		pb->last [i] = val [i];
	}

	if (!change || !tot)
		return;

	length = pb->vertical ? A.height : A.width;
	offset = 0;

	gc = gdk_gc_new (pb->bar->window);

	/* printf ("procbar_set_values %d\n", pb->bar->window); */

	for (i=0; i<pb->n; i++) {
		if (i<pb->n-1)
			lengthr = (unsigned) length * ((float)val [i+1]/tot);
		else
			lengthr = (pb->vertical ? A.height : A.width) - offset;

		/* printf ("%d %d %d %d\n", offset, 0, lengthr, A.height);
		printf ("%u ", val[i+1]);
		printf ("%d ", lengthr); */

		gdk_gc_set_foreground (gc,
				       &pb->colors [i]);

		if (pb->vertical)
			gdk_draw_rectangle (pb->bs,
					    gc,
					    TRUE,
					    0, A.height - offset - lengthr,
					    A.width, lengthr);
		else
			gdk_draw_rectangle (pb->bs,
					    gc,
					    TRUE,
					    offset, 0,
					    lengthr, A.height);

		offset += lengthr;
	}
	/* printf ("\n"); */
		
	gdk_window_copy_area (pb->bar->window,
			      gc,
			      0, 0,
			      pb->bs,
			      0, 0,
			      A.width, A.height);

	gdk_gc_destroy (gc);
}

#undef W
#undef A

void
procbar_start (ProcBar *pb, gint time)

{
	if (pb->cb)
		pb->tag = gtk_timeout_add (time, pb->cb, NULL);
}

void
procbar_stop (ProcBar *pb)

{
	if (pb->cb && pb->tag != -1) {
		gtk_timeout_remove (pb->tag);
	}

	pb->tag = -1;
}

