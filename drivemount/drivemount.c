/* GNOME drivemount applet
 * (C) 2000 John Ellis
 *
 * Author: John Ellis
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Cambridge, MA 
 * 02139, USA.
 *
 */

#include "drivemount.h"
#include <libgnomeui/gnome-window-icon.h>

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

#include "jazdrive_h_in.xpm"
#include "jazdrive_h_out.xpm"
#include "jazdrive_v_in.xpm"
#include "jazdrive_v_out.xpm"


/*
 *-------------------------------------------------------------------------
 * icon struct
 *-------------------------------------------------------------------------
 */

typedef struct _IconData IconData;
struct _IconData
{
	char **pmap_h_in;
	char **pmap_h_out;
	char **pmap_v_in;
	char **pmap_v_out;
};

static IconData icon_list[] = {
	{
		floppy_h_in_xpm,
		floppy_h_out_xpm,
		floppy_v_in_xpm,
		floppy_v_out_xpm
	},
	{
		cdrom_h_in_xpm,
		cdrom_h_out_xpm,
		cdrom_v_in_xpm,
		cdrom_v_out_xpm
	},
	{
		zipdrive_h_in_xpm,
		zipdrive_h_out_xpm,
		zipdrive_v_in_xpm,
		zipdrive_v_out_xpm
	},
	{
		harddisk_h_in_xpm,
		harddisk_h_out_xpm,
		harddisk_v_in_xpm,
		harddisk_v_out_xpm
	},
	{
		jazdrive_h_in_xpm,
		jazdrive_h_out_xpm,
		jazdrive_v_in_xpm,
		jazdrive_v_out_xpm
	},
	{
		NULL,
		NULL,
		NULL,
		NULL
	}
};

static gint icon_list_count = 5;

static gint mount_cb(GtkWidget *widget, gpointer data);
static void eject(DriveData *dd);
static void dnd_init(DriveData *dd);


/*
 *-------------------------------------------------------------------------
 * about dlg
 *-------------------------------------------------------------------------
 */

static void about_cb (AppletWidget *widget, gpointer data)
{
	static GtkWidget *about = NULL;
	const gchar *authors[2];

	authors[0] = "John Ellis <johne@bellatlantic.net>";
	authors[1] = NULL;

	if (about != NULL)
	{
		gdk_window_show(about->window);
		gdk_window_raise(about->window);
		return;
	}

        about = gnome_about_new ( _("Drive Mount Applet"), VERSION,
			"(C) 2000",
			authors,
			_("Released under the GNU general public license.\n"
			  "Mounts and Unmounts drives."),
			NULL);
	gtk_signal_connect(GTK_OBJECT(about), "destroy",
	                   GTK_SIGNAL_FUNC(gtk_widget_destroyed), &about);
	gtk_widget_show (about);

	return;
}

static void browse_cb (AppletWidget *widget, gpointer data)
{
	DriveData *dd = data;
	char *str;
	char *argv[4];

	/* attempt to mount first, otherwise, what is the point? */
	if(!dd->mounted)
		{
		mount_cb(NULL, dd);
		if (!dd->mounted) return; /* failed to mount, so abort */
		}

	/* Do we have nautilus */
	str = gnome_is_program_in_path ("nautilus");
	if (str != NULL) {
		argv[0] = "nautilus";
		argv[1] = dd->mount_point;
		argv[2] = NULL;
		gnome_execute_async (NULL, 2, argv);
	} else {
		argv[0] = "gmc-client";
		argv[1] = "--create-window";
		argv[2] = dd->mount_point;
		argv[3] = NULL;
		gnome_execute_async (NULL, 3, argv);
	}

	return;
}

/*
 *-------------------------------------------------------------------------
 * mount status checks
 *-------------------------------------------------------------------------
 */

static gint device_is_in_mountlist(DriveData *dd)
{
	FILE *fp;
	gchar *command_line = "mount";
	gchar buf[256];
	gint found = FALSE;

	if (!dd->mount_point) return FALSE;

	fp = popen(command_line, "r");

	if (!fp)
		{
		printf("unable to run command: %s\n", command_line);
		return FALSE;
		}

	while (fgets(buf, sizeof(buf), fp) != NULL)
		{
		gchar *ptr;
		ptr = strstr(buf, dd->mount_point);
		if (ptr != NULL)
			{
			gchar *p;
			p = ptr + strlen(dd->mount_point);
			if (*p == ' ' || *p == '\0' || *p == '\n') found = TRUE;
			}
		}

	pclose (fp);

	return found;
}

static gint get_device(const gchar *file)
{
	struct stat file_info;
	gint t;

	if (stat (file, &file_info) == -1)
		t = -1;
	else
		t = (gint)file_info.st_dev;

	return t;
}

static gint device_is_mounted(DriveData *dd)
{
	if (!dd->mount_point || !dd->mount_base) return FALSE;

	if (!dd->autofs_friendly)
		{
		gint b, p;
		b = get_device(dd->mount_base);
		p = get_device(dd->mount_point);
		if (b == p || p == -1 || b == -1)
			return FALSE;
		else
			return TRUE;
		}
	else
		{
		if (device_is_in_mountlist(dd))
			return TRUE;
		else
			return FALSE;
		}
}

/*
 *-------------------------------------------------------------------------
 * image / widget setup and size changes
 *-------------------------------------------------------------------------
 */

static void update_pixmap(DriveData *dd, gint t)
{
	char **pmap_d_in;
	char **pmap_d_out;

	gint width;
	gint height;

	gchar *tiptext;
	gchar *text;

	gint hint = dd->sizehint;

	if (hint > SIZEHINT_MAX) hint = SIZEHINT_MAX; /* maximum size we can scale to */
	hint -= 6; /* buttons have border of 3 >>> FIXME!, broken for themes ? */

/*
	printf("dm applet check: orient=%d size=%d scale=%d\n", dd->orient, dd->sizehint, dd->scale_applet);
*/
	if (dd->device_pixmap > icon_list_count - 1) dd->device_pixmap = 0;

	if (dd->device_pixmap < 0 && (!dd->custom_icon_in || !dd->custom_icon_out) ) dd->device_pixmap = 0;

	if (!dd->button_pixmap)
		{
		/* if only a blank Gnome Pixmap could be created */
		dd->button_pixmap = gnome_pixmap_new_from_xpm_d_at_size(floppy_h_out_xpm, 1, 1);
		gtk_container_add(GTK_CONTAINER(dd->button), dd->button_pixmap);
	        gtk_widget_show(dd->button_pixmap);
		}

	if (dd->device_pixmap < 0)
		{
		GdkImlibImage *im;

		if (t)
			{
			im = gdk_imlib_load_image(dd->custom_icon_in);
			}
		else
			{
			im = gdk_imlib_load_image(dd->custom_icon_out);
			}

		if (im)
			{
			width = im->rgb_width;
			height = im->rgb_height;

			if (dd->orient == ORIENT_LEFT || dd->orient == ORIENT_RIGHT)
				{
				if (width > hint || dd->scale_applet)
					{
					height = (float)height * hint / width;
					width = hint;
					}
				}
			else
				{
				if (height > hint || dd->scale_applet)
					{
					width = (float)width * hint / height;
					height = hint;
					}
				}

			gnome_pixmap_load_imlib_at_size(GNOME_PIXMAP(dd->button_pixmap), im, width, height);
			gdk_imlib_destroy_image(im);
			}
		}
	else
		{
		if (dd->scale_applet)
			{
			width = hint;
			height = (float)hint / ICON_WIDTH * ICON_HEIGHT;
			}
		else
			{
			width = ICON_WIDTH;
			height = ICON_HEIGHT;
			}

#ifdef HAVE_PANEL_PIXEL_SIZE
		if ( (dd->scale_applet && (dd->orient == ORIENT_LEFT || dd->orient == ORIENT_RIGHT) ) ||
		     (!dd->scale_applet && dd->sizehint >= SIZEHINT_DEFAULT && (dd->orient == ORIENT_LEFT || dd->orient == ORIENT_RIGHT) ) ||
		     (!dd->scale_applet && dd->sizehint < SIZEHINT_DEFAULT && (dd->orient == ORIENT_UP || dd->orient == ORIENT_DOWN)) )
#else
		if (dd->orient == ORIENT_LEFT || dd->orient == ORIENT_RIGHT)
#endif
			{
			pmap_d_in = icon_list[dd->device_pixmap].pmap_h_in;
			pmap_d_out = icon_list[dd->device_pixmap].pmap_h_out;
			}
		else
			{
			gint tmp;

			tmp = width;
			width = height;
			height = tmp;

			pmap_d_in = icon_list[dd->device_pixmap].pmap_v_in;
			pmap_d_out = icon_list[dd->device_pixmap].pmap_v_out;
			}

		if (t)
			{
			gnome_pixmap_load_xpm_d_at_size(GNOME_PIXMAP(dd->button_pixmap), pmap_d_in, width, height);
			}
		else
			{
			gnome_pixmap_load_xpm_d_at_size(GNOME_PIXMAP(dd->button_pixmap), pmap_d_out, width, height);
			}
		}

	if (t)
		{
		text = _(" mounted");
		}
	else
		{
		text = _(" not mounted");
		}

	tiptext = g_strconcat(dd->mount_point, text, NULL);
	applet_widget_set_tooltip(APPLET_WIDGET(dd->applet), tiptext);
	g_free(tiptext);
}

void redraw_pixmap(DriveData *dd)
{
	if (dd->sizehint < 1) return;

	if (!device_is_mounted(dd))
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

/*
 *-------------------------------------------------------------------------
 * main callback loop
 *-------------------------------------------------------------------------
 */

static gint drive_update_cb(gpointer data)
{
	DriveData *dd = data;

	if (!device_is_mounted(dd))
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

/*
 *-------------------------------------------------------------------------
 * mount calls
 *-------------------------------------------------------------------------
 */

static gint mount_cb(GtkWidget *widget, gpointer data)
{
	DriveData *dd = data;
	gchar *command_line;
	gchar buf[512];
	FILE *fp;
	GString *str;
	gint check = device_is_mounted(dd);

	/* Stop the user from displaying zillions of error messages */
	if (dd->error_dialog)
		{
		gdk_window_show(dd->error_dialog->window);
		gdk_window_raise(dd->error_dialog->window);
		return FALSE;
		}

	if (!check)
		{
		command_line = g_strdup_printf("mount %s 2>&1", dd->mount_point);
		}
	else
		{
		command_line = g_strdup_printf("umount %s 2>&1", dd->mount_point);
		}

	fp = popen(command_line, "r");

	if (!fp)
		{
		printf("unable to run command: %s\n", command_line);
		g_free(command_line);
		return FALSE;
		}

	str = g_string_new(NULL);

	while (fgets(buf, sizeof(buf), fp) != NULL)
		{
		gchar *b = buf;
		g_string_append(str, b);
		}

	pclose (fp);

	dd->mounted = device_is_mounted(dd);

	if (check != dd->mounted)
		{
		/* success! */
		update_pixmap(dd, dd->mounted);

		/* eject after unmounting, if enabled */
		if (check && dd->auto_eject)
			{
			eject(dd);
			}
		}
	else
		{
		/* the mount status is the same, print
		   the returned output from (u)mount, we are assuming an error */

		g_string_prepend(str, _("\" reported:\n"));
		g_string_prepend(str, command_line);
		g_string_prepend(str, _("Drivemount command failed.\n\""));
		dd->error_dialog = gnome_warning_dialog(str->str);
		gtk_signal_connect(GTK_OBJECT(dd->error_dialog), "destroy",
				   GTK_SIGNAL_FUNC(gtk_widget_destroyed), &dd->error_dialog);
		}

	g_string_free(str, TRUE);
	g_free(command_line);

	return FALSE;
}

static void eject(DriveData *dd)
{
	gchar *command_line;
	gchar buffer[512];
	gchar dn[256];	/* Devicename */
	gchar mp[256];	/* Mountpoint */
	FILE *ml;	/* Mountlist */
	gint found = FALSE;

	if (!dd->mount_point) return;

	/*
	 * Search the output of mount for dd->mount_point
	 * and use the corresponting device name
	 * as argument for eject
	 * if the device is not mounted currently, use
	 * /etc/fstab for the check
	 */
	
	if (dd->mounted) {
		ml = popen("mount", "r");
		while (fgets(buffer, sizeof(buffer), ml)) {
			if (sscanf(buffer, "%255s %*s %255s", dn, mp) == 2 &&
			    (mp && strcmp(mp, dd->mount_point) == 0)) {
				found = TRUE;
				break;
			}
		}
		pclose (ml);
	} else {
		ml = fopen("/etc/fstab", "r");
		while (fgets(buffer, sizeof(buffer), ml)) {
			if (sscanf(buffer, "%255s %255s", dn, mp) == 2 &&
			    strcmp(mp, dd->mount_point) == 0) {
				found = TRUE;
				break;
			}
		}
		fclose (ml);
	}
	
	if (!found) {	/* mp != dd->mount_point */
		printf("WARNING: drivemount.c ... dd->mount_point not found in list\
			 (output of mount, or /etc/fstab) \n");
		return;
	}

	if (dd->mounted) {
		command_line = g_strdup_printf("eject -u '%s'", dn);
		/* perhaps it doesn't like the -u option */
		if(system(command_line) != 0) {
			g_free(command_line);
			command_line = g_strdup_printf("eject '%s'", dn);
			gnome_execute_shell (NULL, command_line);
		}
		g_free(command_line);
	} else {
		command_line = g_strdup_printf("eject '%s'", dn);
		gnome_execute_shell (NULL, command_line);
		g_free(command_line);
	}
}

static void eject_cb(AppletWidget *applet, gpointer data)
{
	DriveData *dd = data;

	eject(dd);

	return;
}

/*
 *-------------------------------------------------------------------------
 * startup and (re)initialization
 *-------------------------------------------------------------------------
 */

/* start or change the update callback timeout interval */
void start_callback_update(DriveData *dd)
{
	gint delay;
	delay = dd->interval * 1000;
	if (dd->timeout_id) gtk_timeout_remove(dd->timeout_id);
	dd->timeout_id = gtk_timeout_add(delay, (GtkFunction)drive_update_cb, dd);

}

static void applet_change_orient(GtkWidget *w, PanelOrientType o, gpointer data)
{
	/* resize the applet and set the proper pixmaps */
	DriveData *dd = data;

	if (dd->orient == o) return;

	dd->orient = o;

	redraw_pixmap(dd);
	return;
}

#ifdef HAVE_PANEL_PIXEL_SIZE
static void applet_change_pixel_size(GtkWidget *w, int size, gpointer data)
{
	/* resize the applet and set the proper pixmaps */
	DriveData *dd = data;

	if (dd->sizehint == size) return;

	dd->sizehint = size;

	redraw_pixmap(dd);
	return;
}
#endif

static gint applet_save_session(GtkWidget *widget, gchar *privcfgpath, gchar *globcfgpath, gpointer data)
{
	DriveData *dd = data;
	property_save(privcfgpath, dd);
	return FALSE;
}

static void destroy_drive_widget(GtkWidget *widget, gpointer data)
{
	DriveData *dd = data;

	g_free(dd->mount_point);
	dd->mount_point = NULL;
	g_free(dd->mount_base);
	dd->mount_base = NULL;

	g_free(dd);

	return;
}

static void
help_cb (AppletWidget *applet, gpointer data)
{
	GnomeHelpMenuEntry help_entry = { "drivemount_applet",
					  "index.html" };
	gnome_help_display(NULL, &help_entry);
}

static DriveData * create_drive_widget(GtkWidget *applet)
{
	DriveData *dd;
	gchar *tmp_path;

	dd = g_new0 (DriveData, 1);

	dd->applet = applet;
	dd->scale_applet = FALSE;
	dd->device_pixmap = 0;
	dd->button_pixmap = NULL;
	dd->mount_point = NULL;
	dd->propwindow = NULL;
	dd->mount_base = g_strdup("/mnt");
	dd->autofs_friendly = FALSE;
	dd->custom_icon_in = NULL;
	dd->custom_icon_out = NULL;
	dd->error_dialog = NULL; 

	dd->orient = applet_widget_get_panel_orient(APPLET_WIDGET(applet));

#ifdef HAVE_PANEL_PIXEL_SIZE
	dd->sizehint = applet_widget_get_panel_pixel_size(APPLET_WIDGET(applet));
#else
	dd->sizehint = SIZEHINT_DEFAULT;
#endif

	property_load(APPLET_WIDGET(applet)->privcfgpath, dd);

	/* attach applet signals here */

	gtk_signal_connect(GTK_OBJECT(applet),"change_orient",
				GTK_SIGNAL_FUNC(applet_change_orient),
				dd);
#ifdef HAVE_PANEL_PIXEL_SIZE
	gtk_signal_connect(GTK_OBJECT(applet),"change_pixel_size",
				GTK_SIGNAL_FUNC(applet_change_pixel_size),
				dd);
#endif

	gtk_signal_connect(GTK_OBJECT(applet),"save_session",
				GTK_SIGNAL_FUNC(applet_save_session),
				dd);



	dd->button=gtk_button_new();
	applet_widget_add(APPLET_WIDGET(applet), dd->button);
	gtk_widget_show(dd->button);

	gtk_signal_connect(GTK_OBJECT(applet),"destroy",
				GTK_SIGNAL_FUNC(destroy_drive_widget),
				dd);
	gtk_signal_connect(GTK_OBJECT(dd->button),"clicked",
				GTK_SIGNAL_FUNC(mount_cb),
				dd);
	dnd_init(dd);

	applet_widget_register_stock_callback(APPLET_WIDGET(applet),
					      "browse",
					      GNOME_STOCK_MENU_OPEN,
					      _("Browse..."),
					      browse_cb,
					      dd);

	/* add "eject" entry if eject program is found in PATH */
	tmp_path = gnome_is_program_in_path("eject");
	if (tmp_path)
		{
		applet_widget_register_callback(APPLET_WIDGET(applet),
					      "eject",
					      _("Eject"),
					      eject_cb,
					      dd);
		g_free (tmp_path);
		}

	applet_widget_register_stock_callback(APPLET_WIDGET(applet),
					      "properties",
					      GNOME_STOCK_MENU_PROP,
					      _("Properties..."),
					      property_show,
					      dd);
	applet_widget_register_stock_callback(APPLET_WIDGET(applet),
					      "help",
					      GNOME_STOCK_PIXMAP_HELP,
					      _("Help"),
					      help_cb, NULL);
	applet_widget_register_stock_callback(APPLET_WIDGET(applet),
					      "about",
					      GNOME_STOCK_MENU_ABOUT,
					      _("About..."),
					      about_cb, NULL);

	redraw_pixmap(dd);
	gtk_widget_show(applet);

	start_callback_update(dd);
	return dd;
}

static GtkWidget * applet_start_new_applet(const gchar *goad_id, const gchar **params, gint nparams)
{
	DriveData *dd;
	GtkWidget *applet;

	if(strcmp(goad_id, "drivemount_applet")) return NULL;

	applet = applet_widget_new(goad_id);
	if (!applet)
		g_error("Can't create applet!\n");

	dd = create_drive_widget(applet);

	return applet;
}

int main (int argc, char *argv[])
{
	DriveData *dd;
	GtkWidget *applet;
	char *goad_id;

	/* Initialize the i18n stuff */
	bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain (PACKAGE);

	applet_widget_init("drivemount_applet", VERSION, argc, argv,
			   NULL, 0, NULL);
	gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/mc/i-floppy.png");

	applet_factory_new("drivemount_applet_factory", NULL, applet_start_new_applet);

	goad_id = (char *)goad_server_activation_id();

	if(goad_id && !strcmp(goad_id, "drivemount_applet")) {
		applet = applet_widget_new("drivemount_applet");
		if (!applet)
			g_error("Can't create applet!\n");

		dd = create_drive_widget(applet);
	}

	applet_widget_gtk_main();
	return 0;
}

/*
 *---------------------------------------------------------------------------
 * drag and drop functions (should eventually be broken into separate file)
 *---------------------------------------------------------------------------
 */

enum {
	TARGET_URI_LIST,
	TARGET_TEXT_PLAIN
};

static GtkTargetEntry button_drag_types[] = {
	{ "text/uri-list", 0, TARGET_URI_LIST },
	{ "text/plain", 0, TARGET_TEXT_PLAIN }
};
static gint n_button_drag_types = 2;

static void dnd_drag_begin_cb(GtkWidget *widget, GdkDragContext *context, gpointer data)
{
	DriveData *dd = data;

	gtk_drag_set_icon_pixmap(context, gtk_widget_get_colormap (dd->button),
				 GNOME_PIXMAP(dd->button_pixmap)->pixmap, NULL,
				 -5, -5);
	return;
}

static void dnd_set_data_cb(GtkWidget *widget, GdkDragContext *context,
			    GtkSelectionData *selection_data, guint info,
			    guint time, gpointer data)
{
        DriveData *dd = data;

	if (dd && dd->mount_point)
		{
		gchar *text = NULL;
		switch (info)
			{
			case TARGET_URI_LIST:
				text = g_strconcat("file:", dd->mount_point, "\r\n", NULL);
				break;
			case TARGET_TEXT_PLAIN:
				text = g_strdup(dd->mount_point);
				break;
			}
		gtk_selection_data_set (selection_data, selection_data->target,
					8, text, strlen(text));
		g_free(text);
		}
	else
		{
		gtk_selection_data_set (selection_data, selection_data->target,
					8, NULL, 0);
		}
	return;
}

static void dnd_init(DriveData *dd)
{
	gtk_drag_source_set(dd->button, GDK_BUTTON1_MASK,
			button_drag_types, n_button_drag_types,
			GDK_ACTION_COPY | GDK_ACTION_LINK | GDK_ACTION_ASK);
	gtk_signal_connect(GTK_OBJECT(dd->button), "drag_data_get",
			dnd_set_data_cb, dd);
	gtk_signal_connect(GTK_OBJECT(dd->button), "drag_begin",
			dnd_drag_begin_cb, dd);
}


