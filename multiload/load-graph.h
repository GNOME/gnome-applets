#ifndef LOAD_GRAPH_H__
#define LOAD_GRAPH_H__

#include <global.h>

typedef struct _LoadGraph LoadGraph;

typedef void (*LoadGraphDataFunc) (int, int [], LoadGraph *);

#define NCPUSTATES 4

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
    GtkWidget *main_widget;
    GtkWidget *frame, *box, *disp;
    GdkPixmap *pixmap;
    GdkGC *gc;
    int timer_index;

    gint show_frame;

    long cpu_time [NCPUSTATES];
    long cpu_last [NCPUSTATES];
    int cpu_initialized;       
};

/* Create new load graph. */
LoadGraph *
load_graph_new (AppletWidget *applet, guint n, gchar *label,
		LoadGraphProperties *global_prop_data,
		LoadGraphProperties *prop_data, guint speed,
		guint size, LoadGraphDataFunc get_data,
		gchar *help_path);

/* Start load graph. */
void
load_graph_start (LoadGraph *g);

/* Stop load graph. */
void
load_graph_stop (LoadGraph *g);

#endif
