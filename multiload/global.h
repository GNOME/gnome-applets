#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <config.h>
#include <gnome.h>
#include <applet-widget.h>

#include <linux-proc.h>
#include <properties.h>
#include <load-graph.h>

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

/* show properties dialog. */
void
multiload_properties_cb (AppletWidget *widget, gpointer data);

/* Load graph properties descriptor. */
extern GnomePropertyDescriptor LoadGraphProperty_Descriptor;

END_GNOME_DECLS

#endif
