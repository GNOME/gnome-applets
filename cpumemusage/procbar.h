#ifndef __PROCBAR_H__
#define __PROCBAR_H__

#include <gtk/gtk.h>

typedef struct _ProcBar ProcBar;

struct _ProcBar {
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *frame;
	GtkWidget *bar;

	gboolean   vertical : 1;

	GdkPixmap *bs;
	GdkColor *colors;

	gint colors_allocated;
	gint first_request;
	gint n;
	gint tag;

	unsigned *last;

	gint (*cb)();
};

ProcBar * procbar_new         (GtkWidget *label,
			       gint n, GdkColor *colors,
			       gint (*cb)());
void      procbar_set_values  (ProcBar *pb, unsigned val []);
void      procbar_set_orient  (ProcBar *pb, gboolean vertical);
void      procbar_start       (ProcBar *pb, gint time);
void      procbar_stop        (ProcBar *pb);

#endif
