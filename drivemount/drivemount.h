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

#define ICON_HEIGHT 10
#define ICON_WIDTH  40

#define SIZEHINT_DEFAULT 46
#define SIZEHINT_MAX 86		/* Set this to 46 to disable scaling larger
				 * than the default size, but still allow
				 * scaling smaller.
				 */

typedef struct _DriveData DriveData;
struct _DriveData
{
	GtkWidget *applet;
	GtkWidget *button;
	GtkWidget *button_pixmap;
	gint device_pixmap;
	gint timeout_id;
	gint interval;
	gint mounted;
	gint autofs_friendly;
	gchar *mount_base;
	gchar *mount_point;
	PanelOrientType orient;
	gint sizehint;
	gint scale_applet;
	gint auto_eject;

	gchar *custom_icon_in;
	gchar *custom_icon_out;

	GtkWidget *error_dialog;

	/* the rest is for the properties window */
	GtkWidget *propwindow;
	GtkWidget *prop_spin;
	GtkWidget *mount_point_entry;
	GtkWidget *icon_entry_in;
	GtkWidget *icon_entry_out;
	gint prop_interval;
	gint prop_device_pixmap;
	gint prop_autofs_friendly;
	gint prop_scale_applet;
	gint prop_auto_eject;
};

void redraw_pixmap(DriveData *dd);
void start_callback_update(DriveData *dd);

void property_load(const gchar *path, DriveData *dd);
void property_save(const gchar *path, DriveData *dd);
void property_show(AppletWidget *applet, gpointer data);


