#ifndef __LOCAL_PROPERTIES_H__
#define __LOCAL_PROPERTIES_H__

#include <properties.h>

G_BEGIN_DECLS

typedef struct	_LocalPropData		LocalPropData;

struct _LocalPropData {
    PanelApplet *applet;
    GList *local_property_object_list;
    GtkWidget *win;
    gchar *help_path;
    PropertyClass type;
};

void
multiload_local_properties_changed (LocalPropData *d);

G_END_DECLS

#endif
