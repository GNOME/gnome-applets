/*#####################################################*/
/*##           drivemount applet 0.1.0 alpha         ##*/
/*#####################################################*/

#include "drivemount.h"

#include "floppy_v_in.xpm"
#include "floppy_v_out.xpm"
#include "floppy_h_in.xpm"
#include "floppy_h_out.xpm"
#include "cdrom_v_in.xpm"
#include "cdrom_v_out.xpm"
#include "cdrom_h_in.xpm"
#include "cdrom_h_out.xpm"
#include "zipdrive_v_in.xpm"
#include "zipdrive_v_out.xpm"
#include "zipdrive_h_in.xpm"
#include "zipdrive_h_out.xpm"
#include "harddisk_v_in.xpm"
#include "harddisk_v_out.xpm"
#include "harddisk_h_in.xpm"
#include "harddisk_h_out.xpm"

static void about_cb (AppletWidget *widget, gpointer data)
{
	GtkWidget *about;
	gchar *authors[2];
	gchar version[32];

	sprintf(version,"%d.%d.%d",DRIVEMOUNT_APPLET_VERSION_MAJ,
		DRIVEMOUNT_APPLET_VERSION_MIN, DRIVEMOUNT_APPLET_VERSION_REV);

	authors[0] = "John Ellis (gqview@geocities.com)";
	authors[1] = NULL;

        about = gnome_about_new ( _("Drive Mount Applet"), version,
			"(C) 1998",
			authors,
			_("Released under the GNU general public license.\n"
			"Mounts and Unmounts drives."
			"."),
			NULL);
	gtk_widget_show (about);
}

static dev_t get_device(char *file)
{
	struct stat file_info;
	dev_t t;

	if (stat (file, &file_info) == -1)
		t = 0;
	else
		t = file_info.st_dev;

	return t;
}

static void update_pixmap(DriveData *dd, int t)
{
	GdkPixmap *pixmap;
	char * text;
	gchar * tiptext;
	if (t)
		{
		pixmap = dd->pixmap_for_in;
		text = _(" mounted");
		}
	else
		{
		pixmap = dd->pixmap_for_out;
		text = _(" not mounted");
		}
	gtk_pixmap_set(GTK_PIXMAP(dd->button_pixmap), pixmap, NULL);
	tiptext = g_copy_strings(dd->mount_point,text,NULL);
	gtk_tooltips_set_tip (dd->tooltip, dd->applet, tiptext, NULL);
	g_free(tiptext);
}

static gint drive_update_cb(gpointer data)
{
	DriveData *dd = data;

	if (get_device(dd->mount_base) == get_device(dd->mount_point))
		{
		/* device not mounted */
		if (dd->mounted)
			{
			update_pixmap(dd, FALSE);
			dd->mounted = FALSE;
			}
		}
	else
		{
		/* device mounted */
		if (!dd->mounted)
			{
			update_pixmap(dd, TRUE);
			dd->mounted = TRUE;
			}
		}

	return TRUE;
}

static int mount_cb(GtkWidget *widget, gpointer data)
{
	DriveData *dd = data;
	char command_line[300];

	if (get_device(dd->mount_base) == get_device(dd->mount_point))
		{
		sprintf(command_line, "mount %s", dd->mount_point);
		}
	else
		{
		sprintf(command_line, "umount %s", dd->mount_point);
		}
	system (command_line);
	drive_update_cb(dd);
	return FALSE;
}

/* start or change the update callback timeout interval */
void start_callback_update(DriveData *dd)
{
	gint delay;
	delay = dd->interval * 1000;
	if (dd->timeout_id) gtk_timeout_remove(dd->timeout_id);
	dd->timeout_id = gtk_timeout_add(delay, (GtkFunction)drive_update_cb, dd);

}

void create_pixmaps(DriveData *dd)
{
	GdkBitmap *mask;
	GtkStyle *style;
	char **pmap_d_in;
	char **pmap_d_out;

	if (dd->orient == ORIENT_LEFT || dd->orient == ORIENT_RIGHT)
		{
		switch (dd->device_pixmap)
			{
			case 0:
				pmap_d_in = floppy_h_in_xpm;
				pmap_d_out = floppy_h_out_xpm;
				break;
			case 1:
				pmap_d_in = cdrom_h_in_xpm;
				pmap_d_out = cdrom_h_out_xpm;
				break;
			case 2:
				pmap_d_in = zipdrive_h_in_xpm;
				pmap_d_out = zipdrive_h_out_xpm;
				break;
			case 3:
				pmap_d_in = harddisk_h_in_xpm;
				pmap_d_out = harddisk_h_out_xpm;
				break;
			default:
				pmap_d_in = floppy_h_in_xpm;
				pmap_d_out = floppy_h_out_xpm;
				break;
			}
		}
	else
		{
		switch (dd->device_pixmap)
			{
			case 0:
				pmap_d_in = floppy_v_in_xpm;
				pmap_d_out = floppy_v_out_xpm;
				break;
			case 1:
				pmap_d_in = cdrom_v_in_xpm;
				pmap_d_out = cdrom_v_out_xpm;
				break;
			case 2:
				pmap_d_in = zipdrive_v_in_xpm;
				pmap_d_out = zipdrive_v_out_xpm;
				break;
			case 3:
				pmap_d_in = harddisk_v_in_xpm;
				pmap_d_out = harddisk_v_out_xpm;
				break;
			default:
				pmap_d_in = floppy_v_in_xpm;
				pmap_d_out = floppy_v_out_xpm;
				break;
			}
		}
	
	style = gtk_widget_get_style(dd->applet);

	dd->pixmap_for_in = gdk_pixmap_create_from_xpm_d(dd->applet->window, &mask,
		&style->bg[GTK_STATE_NORMAL], (gchar **)pmap_d_in);
	dd->pixmap_for_out = gdk_pixmap_create_from_xpm_d(dd->applet->window, &mask,
		&style->bg[GTK_STATE_NORMAL], (gchar **)pmap_d_out);
}

void redraw_pixmap(DriveData *dd)
{
	if (get_device(dd->mount_base) == get_device(dd->mount_point))
		{
		update_pixmap(dd, FALSE);
		dd->mounted = FALSE;
		}
	else
		{
		update_pixmap(dd, TRUE);
		dd->mounted = TRUE;
		}
}

static void applet_change_orient(GtkWidget *w, PanelOrientType o, gpointer data)
{
	/* resize the applet and set the proper pixmaps */
	DriveData *dd = data;
	dd->orient = o;

	create_pixmaps(dd);

	if (dd->orient == ORIENT_LEFT || dd->orient == ORIENT_RIGHT)
		{
		gtk_widget_set_usize(dd->button,46,16);
		}
	else
		{
		gtk_widget_set_usize(dd->button,16,46);
		}
	redraw_pixmap(dd);
}

static gint applet_save_session(GtkWidget *widget, char *privcfgpath, char *globcfgpath, gpointer data)
{
	DriveData *dd = data;
	property_save(privcfgpath, dd);
        return FALSE;
}

static DriveData * create_drive_widget(GtkWidget *applet)
{
	DriveData *dd;

	dd = g_new(DriveData, 1);

	dd->applet = applet;
	dd->orient = ORIENT_UP;
	dd->device_pixmap = 0;
	dd->mount_point = NULL;
	dd->propwindow = NULL;
	dd->mount_base = strdup("/mnt");

	property_load(APPLET_WIDGET(applet)->privcfgpath, dd);

	dd->button=gtk_button_new();
	gtk_signal_connect(GTK_OBJECT(dd->button),"clicked",
				GTK_SIGNAL_FUNC(mount_cb),
				dd);
	gtk_widget_show(dd->button);

	dd->tooltip=gtk_tooltips_new();

	gtk_widget_realize(dd->applet);
	create_pixmaps(dd);

	dd->button_pixmap = gtk_pixmap_new(dd->pixmap_for_out, NULL);
        gtk_container_add(GTK_CONTAINER(dd->button), dd->button_pixmap);
        gtk_widget_show(dd->button_pixmap);

	redraw_pixmap(dd);

/* attach applet signals here */
	gtk_signal_connect(GTK_OBJECT(applet),"change_orient",
				GTK_SIGNAL_FUNC(applet_change_orient),
				dd);
	gtk_signal_connect(GTK_OBJECT(applet),"save_session",
				GTK_SIGNAL_FUNC(applet_save_session),
				dd);
	applet_widget_register_stock_callback(APPLET_WIDGET(applet),
					      "about",
					      GNOME_STOCK_MENU_ABOUT,
					      _("About..."),
					      about_cb, NULL);
	applet_widget_register_stock_callback(APPLET_WIDGET(applet),
					      "properties",
					      GNOME_STOCK_MENU_PROP,
					      _("Properties..."),
					      property_show,
					      dd);

	start_callback_update(dd);
	return dd;
}

static void applet_start_new_applet(const gchar *param, gpointer data)
{
	DriveData *dd;
	GtkWidget *applet;

	applet = applet_widget_new_with_param(param);
		if (!applet)
			g_error("Can't create applet!\n");

	dd = create_drive_widget(applet);

	applet_widget_add(APPLET_WIDGET(applet), dd->button);
	gtk_widget_show(applet);
}

int main (int argc, char *argv[])
{
	DriveData *dd;
	GtkWidget *applet;

	/* Initialize the i18n stuff */
	bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain (PACKAGE);

	applet_widget_init("drivemount_applet", NULL, argc, argv, 0, NULL,
				argv[0], TRUE, TRUE, applet_start_new_applet, NULL);

	applet = applet_widget_new();
	if (!applet)
		g_error("Can't create applet!\n");

	dd = create_drive_widget(applet);

	applet_widget_add(APPLET_WIDGET(applet), dd->button);
	gtk_widget_show(applet);

	applet_widget_gtk_main();
	return 0;
}
