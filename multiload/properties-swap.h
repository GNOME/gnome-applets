#ifndef PROPERTIES_SWAP_H__
#define PROPERTIES_SWAP_H__

#include <applet-widget.h>

typedef struct 
{
	gchar *ucolor;
	gchar *fcolor;
	guint speed, height, width;
	gboolean look;
} swapload_properties;

void swap_properties (AppletWidget *applet, gpointer data);
void load_swap_properties (char *path, swapload_properties *prop);
void save_swap_properties (char *path, swapload_properties *prop);

#endif
