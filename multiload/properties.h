#ifndef __PROPERTIES_H__
#define __PROPERTIES_H__

#include <property-entries.h>

BEGIN_GNOME_DECLS

typedef enum {
    PROP_CPULOAD,
    PROP_MEMLOAD,
    PROP_SWAPLOAD,
    PROP_NETLOAD,
} PropertyClass;

typedef struct	_MultiLoadProperties		MultiLoadProperties;

typedef struct	_LoadGraphProperties		LoadGraphProperties;

struct _LoadGraphProperties {
    guint n;
    const gchar *name;
    const gchar **texts;
    const gchar **color_defs;
    GdkColor *colors;
    gulong adj_data [2];
};

struct _MultiLoadProperties {
    LoadGraphProperties cpuload, memload, swapload, netload;
};

extern GList *multiload_property_object_list;

extern MultiLoadProperties multiload_properties;

void multiload_properties_apply (void);
void multiload_properties_close (void);
void multiload_properties_changed (void);
void multiload_show_properties (PropertyClass prop_class);
void multiload_init_properties (void);

END_GNOME_DECLS

#endif
