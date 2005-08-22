#ifndef LOAD_GRAPH_H__
#define LOAD_GRAPH_H__

#include "global.h"

/* Create new load graph. */
LoadGraph *
load_graph_new (MultiloadApplet *multiload, guint n, const gchar *label,
		guint id, guint speed, guint size, gboolean visible, 
		const gchar *name, LoadGraphDataFunc get_data) G_GNUC_INTERNAL;

/* Start load graph. */
void
load_graph_start (LoadGraph *g) G_GNUC_INTERNAL;

/* Stop load graph. */
void
load_graph_stop (LoadGraph *g) G_GNUC_INTERNAL;

/* free load graph */
void
load_graph_unalloc (LoadGraph *g) G_GNUC_INTERNAL;
		      
#endif
