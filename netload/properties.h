#include <applet-widget.h>

#ifndef PROPERTIES_H__
#define PROPERTIES_H__

typedef struct 
{
	gchar	*gcolor;	/* Graph colour. */
	gchar	*bcolor;	/* Line colour. */
	guint	speed, height, width;
	gboolean look;
	gchar	*device;
	guint	line_spacing;
} netload_properties;

void properties(AppletWidget *applet, gpointer data);
void load_properties(char *path, netload_properties *prop );
void save_properties(char *path, netload_properties *prop );

#endif
