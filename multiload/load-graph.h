#ifndef LOAD_GRAPH_H__
#define LOAD_GRAPH_H__

#include <global.h>

typedef struct _LoadGraph LoadGraph;

typedef void (*LoadGraphDataFunc) (int, int []);

struct _LoadGraph {
    AppletWidget *applet;

    guint n;
    guint speed, size;
    guint orient, pixel_size;
    guint draw_width, draw_height;
    LoadGraphDataFunc get_data;

    LoadGraphProperties *prop_data;
    LoadGraphProperties *global_prop_data;
    LoadGraphProperties *prop_data_ptr;

    LocalPropData *local_prop_data;

    guint allocated;

    GdkColor *colors;
    guint **data, **odata;
    guint data_size;
    guint *pos;

    gint colors_allocated;
    GtkWidget *frame, *disp;
    GdkPixmap *pixmap;
    GdkGC *gc;
    int timer_index;
};

/* Create new load graph. */
LoadGraph *
load_graph_new (AppletWidget *applet, guint n, gchar *label,
		LoadGraphProperties *global_prop_data,
		LoadGraphProperties *prop_data, guint speed,
		guint size, LoadGraphDataFunc get_data);

/* Start load graph. */
void
load_graph_start (LoadGraph *g);

/* Stop load graph. */
void
load_graph_stop (LoadGraph *g);

#endif
