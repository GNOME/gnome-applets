#ifndef PROPERTIES_H__
#define PROPERTIES_H__

#include <applet-widget.h>

typedef struct 
{
	gchar *ucolor;
	gchar *scolor;
	guint speed, height, width;
	gboolean look;
} cpuload_properties;

void properties(AppletWidget *applet, gpointer data);
void load_properties( cpuload_properties *prop );
void save_properties( cpuload_properties *prop );

#endif