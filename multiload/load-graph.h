#ifndef LOAD_GRAPH_H__
#define LOAD_GRAPH_H__

typedef struct _LoadGraph LoadGraph;

typedef void (*LoadGraphDataFunc) (int, int []);

struct _LoadGraph {
    guint n;
    guint speed, width, height;
    LoadGraphDataFunc get_data;

    LoadGraphProperties *prop_data;

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
load_graph_new (guint n, gchar *label, LoadGraphProperties *prop_data,
		guint speed, guint width, guint height,
		LoadGraphDataFunc get_data);

/* Start load graph. */
void
load_graph_start (LoadGraph *g);

/* Stop load graph. */
void
load_graph_stop (LoadGraph *g);

#endif
