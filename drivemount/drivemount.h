/* GNOME drivemount applet
 * (C) 1999 John Ellis
 *
 * Author: John Ellis
 *
 */

#include <config.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <gnome.h>
#include <applet-widget.h>

typedef struct _DriveData DriveData;
struct _DriveData
{
	GtkWidget *applet;
	GtkWidget *button;
	GtkWidget *button_pixmap;
	GdkPixmap *pixmap_for_in;
	GdkPixmap *pixmap_for_out;
	gint device_pixmap;
	gint timeout_id;
	gint interval;
	gint mounted;
	gint autofs_friendly;
	gchar *mount_base;
	gchar *mount_point;
	PanelOrientType orient;
	PanelSizeType size;
	GtkTooltips *tooltip;
	/* the rest is for the properties window */
	GtkWidget *propwindow;
	GtkWidget *prop_spin;
	GtkWidget *mount_point_entry;
	gint prop_interval;
	gint prop_device_pixmap;
	gint prop_autofs_friendly;
};


void create_pixmaps(DriveData *dd);
void redraw_pixmap(DriveData *dd);
void start_callback_update(DriveData *dd);

void property_load(gchar *path, DriveData *dd);
void property_save(gchar *path, DriveData *dd);
void property_show(AppletWidget *applet, gpointer data);

