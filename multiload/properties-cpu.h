#ifndef PROPERTIES_CPU_H__
#define PROPERTIES_CPU_H__

#include <applet-widget.h>

typedef struct 
{
	gchar *ucolor;
	gchar *scolor;
	guint speed, height, width;
	gboolean look;
} cpuload_properties;

void cpu_properties (AppletWidget *applet, gpointer data);
void load_cpu_properties (char *path, cpuload_properties *prop);
void save_cpu_properties (char *path, cpuload_properties *prop);

#endif
