#ifndef __PROPERTIES_H__
#define __PROPERTIES_H__

#include <property-entries.h>

BEGIN_GNOME_DECLS

typedef enum {
    PROP_CPULOAD,
    PROP_MEMLOAD,
    PROP_SWAPLOAD,
    PROP_NETLOAD,
    PROP_LOADAVG
} PropertyClass;

typedef enum {
    LOADAVG_1 = 0,
    LOADAVG_5,
    LOADAVG_15
} LoadAvgType;

typedef struct	_MultiLoadProperties		MultiLoadProperties;

typedef struct	_LoadGraphProperties		LoadGraphProperties;

struct _LoadGraphProperties {
    guint type, n;
    const gchar *name;
    const gchar **texts;
    const gchar **color_defs;
    GdkColor *colors;
    gulong adj_data [3];
    gint loadavg_type;
    gint use_default;
};

struct _MultiLoadProperties {
    LoadGraphProperties cpuload, memload, swapload, netload, loadavg;
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
