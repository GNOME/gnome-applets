#ifndef PROPERTIES_MEM_H__
#define PROPERTIES_MEM_H__

#include <applet-widget.h>

typedef struct 
{
	gchar *ucolor;
	gchar *scolor;
	gchar *bcolor;
	gchar *ccolor;
	guint speed, height, width;
	gboolean look;
} memload_properties;

void mem_properties (AppletWidget *applet, gpointer data);
void load_mem_properties (char *path, memload_properties *prop);
void save_mem_properties (char *path, memload_properties *prop);

#endif
