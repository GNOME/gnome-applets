#ifndef __PROPERTIES_H__
#define __PROPERTIES_H__

#include <applet-widget.h>

typedef struct 
{
	gint startfs;
	gchar *ucolor;	/* color for used space */
	gchar *fcolor;	/* color for free space */
	gchar *tcolor;  /* color for text  */
	gchar *bcolor;  /* background color */
	gchar *font;
	gboolean best_size;
	guint speed, size;
	gboolean look;
} diskusage_properties;

extern gboolean font_changed;

void properties (AppletWidget *applet, gpointer data);
void load_properties (const char *path, diskusage_properties *prop);
void save_properties (const char *path, diskusage_properties *prop);

#endif /* __PROPERTIES_H__ */
