#ifndef PROPERTIES_H__
#define PROPERTIES_H__

#include <applet-widget.h>

typedef struct 
{
        gchar * dmap[2];
        gchar * dfile[2];
} gkb_properties;

void properties(AppletWidget *applet, gpointer data);
void load_properties( char *path, gkb_properties *prop );
void save_properties( char *path, gkb_properties *prop );

#endif
