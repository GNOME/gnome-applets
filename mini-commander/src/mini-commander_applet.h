#ifndef _MC_APPLET_H_
#define _MC_APPLET_H_

#include <panel-applet.h>

extern GtkWidget *applet;
typedef struct _MCData MCData;
typedef struct struct_properties properties;

struct _MCData
{
    PanelApplet *applet;
    GtkWidget *entry;
    properties *prop;
    GtkWidget *applet_vbox;
    GtkWidget *applet_inner_vbox;
    GtkWidget *properties_box;
    gint label_timeout;
};

void redraw_applet(MCData *mcdata);
void set_atk_name_description(GtkWidget *widget, const gchar *name,
        const gchar *description);

#endif

