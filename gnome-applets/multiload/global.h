#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include "multiload-applet.h"

#include <glib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define NCPUSTATES 5
#define NGRAPHS 6

#define MULTILOAD_SCHEMA              "org.gnome.gnome-applets.multiload"
#define GNOME_DESKTOP_LOCKDOWN_SCHEMA "org.gnome.desktop.lockdown"

#define KEY_VIEW_CPULOAD  "view-cpuload"
#define KEY_VIEW_MEMLOAD  "view-memload"
#define KEY_VIEW_NETLOAD2 "view-netload2"
#define KEY_VIEW_SWAPLOAD "view-swapload"
#define KEY_VIEW_LOADAVG  "view-loadavg"
#define KEY_VIEW_DISKLOAD "view-diskload"

#define KEY_SPEED "speed"
#define KEY_SIZE  "size"

#define KEY_CPULOAD_COLOR0 "cpuload-color0"
#define KEY_CPULOAD_COLOR1 "cpuload-color1"
#define KEY_CPULOAD_COLOR2 "cpuload-color2"
#define KEY_CPULOAD_COLOR3 "cpuload-color3"
#define KEY_CPULOAD_COLOR4 "cpuload-color4"

#define KEY_MEMLOAD_COLOR0 "memload-color0"
#define KEY_MEMLOAD_COLOR1 "memload-color1"
#define KEY_MEMLOAD_COLOR2 "memload-color2"
#define KEY_MEMLOAD_COLOR3 "memload-color3"
#define KEY_MEMLOAD_COLOR4 "memload-color4"

#define KEY_NETLOAD2_COLOR0 "netload2-color0"
#define KEY_NETLOAD2_COLOR1 "netload2-color1"
#define KEY_NETLOAD2_COLOR2 "netload2-color2"
#define KEY_NETLOAD2_COLOR3 "netload2-color3"

#define KEY_SWAPLOAD_COLOR0 "swapload-color0"
#define KEY_SWAPLOAD_COLOR1 "swapload-color1"

#define KEY_LOADAVG_COLOR0 "loadavg-color0"
#define KEY_LOADAVG_COLOR1 "loadavg-color1"

#define KEY_DISKLOAD_COLOR0 "diskload-color0"
#define KEY_DISKLOAD_COLOR1 "diskload-color1"
#define KEY_DISKLOAD_COLOR2 "diskload-color2"

#define KEY_SYSTEM_MONITOR "system-monitor"

#define DISABLE_COMMAND_LINE "disable-command-line"

#define IS_STRING_EMPTY(x) ((x) == NULL || (x)[0] == '\0')

typedef struct _LoadGraph LoadGraph;
typedef void (*LoadGraphDataFunc) (int, int [], LoadGraph *);

#include "netspeed.h"

struct _LoadGraph {
    MultiloadApplet *multiload;

    guint n, id;
    guint speed, size;
    guint orient;
    guint draw_width, draw_height;
    LoadGraphDataFunc get_data;

    guint allocated;

    GdkRGBA *colors;
    gint **data;
    guint data_size;
    guint *pos;

    GtkWidget *main_widget;
    GtkWidget *frame, *box, *disp;
    cairo_surface_t *surface;
    int timer_index;

    gint show_frame;

    long cpu_time [NCPUSTATES];
    long cpu_last [NCPUSTATES];
    int cpu_initialized;

    double loadavg1;
    NetSpeed *netspeed_in;
    NetSpeed *netspeed_out;

    gboolean visible;
    gboolean tooltip_update;
    const gchar *name;
};

struct _MultiloadApplet
{
	GpApplet parent;
	
	LoadGraph *graphs[NGRAPHS];

	GtkOrientation orientation;
	GtkWidget *box;
	
	gboolean view_cpuload;
	gboolean view_memload;
	gboolean view_netload;
	gboolean view_swapload;
	gboolean view_loadavg;
	gboolean view_diskload;

	GtkWidget *check_boxes[NGRAPHS];
	GtkWidget *prop_dialog;
	GtkWidget *notebook;
	int last_clicked;

	GSettings *settings;
};

#include "load-graph.h"
#include "linux-proc.h"

/* show properties dialog */
G_GNUC_INTERNAL void
multiload_properties_cb (GSimpleAction *action,
                         GVariant      *parameter,
                         gpointer       user_data);

/* remove the old graphs and rebuild them */
G_GNUC_INTERNAL void
multiload_applet_refresh(MultiloadApplet *ma);

/* update the tooltip to the graph's current "used" percentage */
G_GNUC_INTERNAL void
multiload_applet_tooltip_update(LoadGraph *g);

G_END_DECLS

#endif
