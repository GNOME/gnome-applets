#ifndef LOAD_GRAPH_H__
#define LOAD_GRAPH_H__

#include <global.h>

/* Create new load graph. */
LoadGraph *
load_graph_new (PanelApplet *applet, guint n, gchar *label,
		guint speed,
		guint size, 
		gboolean visible, 
		gchar *name, 
		LoadGraphDataFunc get_data);

/* Start load graph. */
void
load_graph_start (LoadGraph *g);

/* Stop load graph. */
void
load_graph_stop (LoadGraph *g);

#endif
