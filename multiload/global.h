#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <glib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtkwidget.h>
#include <gtk/gtktooltips.h>
#include <panel-applet.h>

G_BEGIN_DECLS

#define NCPUSTATES 5
#define NGRAPHS 6

typedef struct _MultiloadApplet MultiloadApplet;
typedef struct _LoadGraph LoadGraph;
typedef void (*LoadGraphDataFunc) (int, int [], LoadGraph *);

struct _LoadGraph {
    PanelApplet *applet;

    guint n;
    guint speed, size;
    guint orient, pixel_size;
    guint draw_width, draw_height;
    LoadGraphDataFunc get_data;

    guint allocated;

    GdkColor *colors;
    gint **odata, **data;
    guint data_size;
    guint *pos;

    gint colors_allocated;
    GtkWidget *main_widget;
    GtkWidget *frame, *box, *disp;
    GdkPixmap *pixmap;
    GdkGC *gc;
    int timer_index;
    GtkTooltips *tooltips;

    gint show_frame;

    long cpu_time [NCPUSTATES];
    long cpu_last [NCPUSTATES];
    int cpu_initialized;

    gboolean visible;
    gboolean tooltip_update;
    gchar *name;
};

struct _MultiloadApplet
{
	PanelApplet *applet;
	
	LoadGraph *graphs[NGRAPHS];
	
	GtkWidget *box;
	
	gboolean view_cpuload;
	gboolean view_memload;
	gboolean view_netload;
	gboolean view_swapload;
	gboolean view_loadavg;
	gboolean view_diskload;
	
	GtkWidget *about_dialog;
	GtkWidget *check_boxes[NGRAPHS];
	GtkWidget *prop_dialog;
};

#include "load-graph.h"
#include "linux-proc.h"

/* start a new instance of the cpuload applet */
LoadGraph *
cpuload_applet_new(PanelApplet *applet, gpointer data) G_GNUC_INTERNAL;

/* start a new instance of the memload applet */
LoadGraph *
memload_applet_new(PanelApplet *applet, gpointer data) G_GNUC_INTERNAL;

/* start a new instance of the swapload applet */
LoadGraph *
swapload_applet_new(PanelApplet *applet, gpointer data) G_GNUC_INTERNAL;

/* start a new instance of the netload applet */
LoadGraph *
netload_applet_new(PanelApplet *applet, gpointer data) G_GNUC_INTERNAL;

/* start a new instance of the loadavg applet */
LoadGraph *
loadavg_applet_new(PanelApplet *applet, gpointer data) G_GNUC_INTERNAL;

/* start a new instance of the loadavg applet */
LoadGraph *
diskload_applet_new(PanelApplet *applet, gpointer data) G_GNUC_INTERNAL;

/* show properties dialog */
void
multiload_properties_cb (BonoboUIComponent *uic,
			 MultiloadApplet   *ma,	
			 const char        *name) G_GNUC_INTERNAL;

/* remove the old graphs and rebuild them */
void
multiload_applet_refresh(MultiloadApplet *ma) G_GNUC_INTERNAL;

/* update the tooltip to the graph's current "used" percentage */
void
multiload_applet_tooltip_update(LoadGraph *g) G_GNUC_INTERNAL;

G_END_DECLS

#endif
