#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <config.h>
#include <gnome.h>
#include <applet-widget.h>

#include <local-properties.h>
#include <load-graph.h>

#include <linux-proc.h>

BEGIN_GNOME_DECLS

/* start a new instance of the cpuload applet */
GtkWidget *
make_cpuload_applet (const gchar *goad_id);

/* start a new instance of the memload applet */
GtkWidget *
make_memload_applet (const gchar *goad_id);

/* start a new instance of the swapload applet */
GtkWidget *
make_swapload_applet (const gchar *goad_id);

/* start a new instance of the netload applet */
GtkWidget *
make_netload_applet (const gchar *goad_id);

/* start a new instance of the loadavg applet */
GtkWidget *
make_loadavg_applet (const gchar *goad_id);

/* show default properties dialog. */
void
multiload_properties_cb (AppletWidget *widget, gpointer data);

/* show local properties dialog. */
void
multiload_local_properties_cb (AppletWidget *widget, gpointer data);

/* run gtop */
void 
start_gtop_cb (AppletWidget *widget, gpointer data);

/* show help */
void
multiload_help_cb (AppletWidget *widget, gpointer data);

/* Load graph properties descriptor. */
extern GnomePropertyDescriptor LoadGraphProperty_Descriptor;

/* Load graph local properties descriptor. */
extern GnomePropertyDescriptor LoadGraphLocalProperty_Descriptor;

END_GNOME_DECLS

#endif
