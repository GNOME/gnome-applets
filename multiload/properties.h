#ifndef __PROPERTIES_H__
#define __PROPERTIES_H__

#include <property-entries.h>

BEGIN_GNOME_DECLS

typedef struct	_MultiLoadProperties		MultiLoadProperties;

typedef struct	_LoadGraphProperties		LoadGraphProperties;

struct _LoadGraphProperties {
    guint n;
    const gchar *name;
    const gchar **texts;
    const gchar **color_defs;
    GdkColor *colors;
    glong adj_data [3];
};

struct _MultiLoadProperties {
    LoadGraphProperties cpuload, memload, swapload;
};

extern GList *multiload_property_object_list;

extern MultiLoadProperties multiload_properties;

void multiload_properties_apply (void);
void multiload_properties_close (void);
void multiload_properties_changed (void);
void multiload_show_properties (void);
void multiload_init_properties (void);

END_GNOME_DECLS

#endif
