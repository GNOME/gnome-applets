/*#####################################################*/
/*##           drivemount applet 0.2.1               ##*/
/*#####################################################*/

#include <config.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <gnome.h>
#include <applet-widget.h>

#define DRIVEMOUNT_APPLET_VERSION_MAJ 0
#define DRIVEMOUNT_APPLET_VERSION_MIN 2
#define DRIVEMOUNT_APPLET_VERSION_REV 1

typedef struct _DriveData DriveData;
struct _DriveData
{
	GtkWidget *applet;
	GtkWidget *button;
	GtkWidget *button_pixmap;
	GdkPixmap *pixmap_for_in;
	GdkPixmap *pixmap_for_out;
	int device_pixmap;
	int timeout_id;
	int interval;
	int mounted;
	int autofs_friendly;
	char *mount_base;
	char *mount_point;
	PanelOrientType orient;
	GtkTooltips *tooltip;
	/* the rest is for the properties window */
	GtkWidget *propwindow;
	GtkWidget *prop_spin;
	GtkWidget *mount_point_entry;
	int prop_interval;
	int prop_device_pixmap;
	int prop_autofs_friendly;
};


void create_pixmaps(DriveData *dd);
void redraw_pixmap(DriveData *dd);
void start_callback_update(DriveData *dd);

void property_load(char *path, DriveData *dd);
void property_save(char *path, DriveData *dd);
void property_show(AppletWidget *applet, gpointer data);

