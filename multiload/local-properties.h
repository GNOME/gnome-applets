#ifndef __LOCAL_PROPERTIES_H__
#define __LOCAL_PROPERTIES_H__

#include <properties.h>

BEGIN_GNOME_DECLS

typedef struct	_LocalPropData		LocalPropData;

struct _LocalPropData {
    AppletWidget *applet;
    GnomePropertyObject *property_object;
    GList *local_property_object_list;
    GtkWidget *win;
    gchar *help_path;
    PropertyClass type;
};

void
multiload_local_properties_changed (LocalPropData *d);

END_GNOME_DECLS

#endif
