#ifndef _MC_APPLET_H_
#define _MC_APPLET_H_

#define LENGTH_HISTORY_LIST	  50
#include <panel-applet.h>

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
    GtkWidget *entry_command;
    GtkWidget *label_message;
    int message_locked;

    char *history_command[LENGTH_HISTORY_LIST];
    int history_position;
};

void redraw_applet(MCData *mcdata);
void set_atk_name_description(GtkWidget *widget, const gchar *name,
        const gchar *description);

#endif

