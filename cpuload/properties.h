#ifndef PROPERTIES_H__
#define PROPERTIES_H__

typedef struct 
{
	gchar *ucolor;
	gchar *scolor;
	guint speed, height, width;
	gboolean look;
} cpuload_properties;

void properties(int id, gpointer data);
void load_properties( cpuload_properties *prop );
void save_properties( cpuload_properties *prop );

#endif