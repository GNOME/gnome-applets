#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <config.h>

#include <gnome.h>
#include <panel-applet.h>

#include <gtk/gtk.h>
#include <libgnomeui/libgnomeui.h>
#include <libgnome/libgnome.h>
#include <libgnomeui/gnome-window-icon.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <libart_lgpl/libart.h>

#include "local-properties.h"
#include "load-graph.h"
#include "linux-proc.h"

G_BEGIN_DECLS

/* start a new instance of the cpuload applet */
gboolean
cpuload_applet_new(PanelApplet *applet);

/* start a new instance of the memload applet */
gboolean
memload_applet_new(PanelApplet *applet);

/* start a new instance of the swapload applet */
gboolean
swapload_applet_new(PanelApplet *applet);

/* start a new instance of the netload applet */
gboolean
netload_applet_new(PanelApplet *applet);

/* start a new instance of the loadavg applet */
gboolean
loadavg_applet_new(PanelApplet *applet);

/* run gtop */
/* if gtop is being phased out in favor of procman, is it safe to remove this function? */
void 
start_gtop_cb (BonoboUIComponent *uic, gpointer data, const gchar *name);

/* run procman, gtop's replacement */
void
start_procman_cb (BonoboUIComponent *uic, gpointer data, const gchar *name);

/* show help */
void
multiload_help_cb (BonoboUIComponent *uic, gpointer data, const gchar *name);

G_END_DECLS

#endif
