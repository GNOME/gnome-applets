#ifndef PROPERTIES_H__
#define PROPERTIES_H__

#include <applet-widget.h>

typedef struct 
{
	gint startfs;
	gchar *ucolor;	/* color for used space */
	gchar *fcolor;	/* color for free space */
	gchar *tcolor;  /* color for text  */
	gchar *bcolor;  /* background color */
	guint speed, size;
	gboolean look;
} diskusage_properties;

void properties(AppletWidget *applet, gpointer data);
void load_properties( const char *path, diskusage_properties *prop );
void save_properties( const char *path, diskusage_properties *prop );

#endif
