#ifndef __PROCBAR_H__
#define __PROCBAR_H__

#include <gtk/gtk.h>

typedef struct _ProcBar ProcBar;

struct _ProcBar {
	GtkWidget *hbox;
	GtkWidget *vbox;
	GtkWidget *box;
	GtkWidget *label;
	GtkWidget *frame;
	GtkWidget *bar;

	GdkPixmap *bs;
	GdkColor *colors;

	gint colors_allocated;
	gint first_request;
	gint n;
	gint tag;

	gint orientation;

	unsigned *last;

	gint (*cb)();
};

enum {
	PROCBAR_HORIZONTAL,
	PROCBAR_VERTICAL,
};

ProcBar * procbar_new         (GtkWidget *label, gint orient,
			       gint n, GdkColor *colors,
			       gint (*cb)());
void      procbar_set_values  (ProcBar *pb, unsigned val []);
void      procbar_start       (ProcBar *pb, gint time);
void      procbar_stop        (ProcBar *pb);

#endif
