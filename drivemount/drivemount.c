/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <stdio.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>
#include <panel-applet.h>

#include "drivemount.h"
#include "properties.h"

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


static BonoboObject *applet_factory (BonoboGenericFactory *this, const gchar *iid, gpointer data);
static BonoboObject *applet_new (void);
static DriveData *create_drive_widget(void);
static void dnd_init(DriveData *dd);
static void dnd_drag_begin_cb(GtkWidget *widget, GdkDragContext *context, gpointer data);
static void dnd_set_data_cb(GtkWidget *widget, GdkDragContext *context, GtkSelectionData *selection_data, guint info, guint time, gpointer data);
static gboolean applet_button_press_cb(GtkWidget *widget, GdkEventButton *event, gpointer data);
static void applet_change_orient(GtkWidget *w, PanelAppletOrient o, gpointer data);
static void applet_change_pixel_size(GtkWidget *w, int size, gpointer data);
static gint applet_save_session(PanelApplet *applet, gpointer data);
static void destroy_drive_widget(GtkWidget *widget, gpointer data);
static void browse_cb (PanelApplet *widget, gpointer data);
static void eject_cb(PanelApplet *applet, gpointer data);
static void help_cb (PanelApplet *widget, gpointer data);
static void about_cb (PanelApplet *widget, gpointer data);
static gint device_is_in_mountlist(DriveData *dd);
static gint get_device(const gchar *file);
static gint device_is_mounted(DriveData *dd);
static void update_pixmap(DriveData *dd, gint t);
static gint drive_update_cb(gpointer data);
static gint mount_cb(GtkWidget *widget, gpointer data);
static void eject(DriveData *dd);

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

/* Bonobo Verbs for our popup menu */
static const BonoboUIVerb applet_menu_verbs [] = {
	BONOBO_UI_UNSAFE_VERB ("Browse", browse_cb),
	BONOBO_UI_UNSAFE_VERB ("Eject", eject_cb),
	BONOBO_UI_UNSAFE_VERB ("Properties", property_show),
	BONOBO_UI_UNSAFE_VERB ("Help", help_cb),
	BONOBO_UI_UNSAFE_VERB ("About", about_cb),
    BONOBO_UI_VERB_END
};

/* and the XML definition for the popup menu */
static const char applet_menu_xml [] =
"<popup name=\"button3\">\n"
"   <menuitem name=\"Browse\" verb=\"Browse\" _label=\"Browse...\"\n"
"             pixtype=\"stock\" pixname=\"gtk-open\"/>\n"
"   <menuitem name=\"Eject\" verb=\"Eject\" _label=\"Eject\"\n"
"             pixtype=\"stock\" pixname=\"gtk-up-arrow\"/>\n"
"   <menuitem name=\"Properties\" verb=\"Properties\" _label=\"Properties...\"\n"
"             pixtype=\"stock\" pixname=\"gtk-properties\"/>\n"
"   <menuitem name=\"Help\" verb=\"Help\" _label=\"Help\"\n"
"             pixtype=\"stock\" pixname=\"gtk-help\"/>\n"
"   <menuitem name=\"About\" verb=\"About\" _label=\"About ...\"\n"
"             pixtype=\"stock\" pixname=\"gnome-stock-about\"/>\n"
"</popup>\n";

enum {
	TARGET_URI_LIST,
	TARGET_TEXT_PLAIN
};

static GtkTargetEntry button_drag_types[] = {
	{ "text/uri-list", 0, TARGET_URI_LIST },
	{ "text/plain", 0, TARGET_TEXT_PLAIN }
};
static gint n_button_drag_types = 2;


PANEL_APPLET_BONOBO_FACTORY ("OAFIID:GNOME_DriveMountApplet_Factory",
							 "Drive Mount Applet",
							 "0",
							  applet_factory,
							  NULL)

static BonoboObject *
applet_factory (BonoboGenericFactory *this, const gchar *iid, gpointer data)
{
	BonoboObject *applet = NULL;

	if (!strcmp (iid, "OAFIID:GNOME_DriveMountApplet"))
		applet = applet_new ();
	return(applet);
}

static BonoboObject *
applet_new ()
{
	GtkWidget *applet;
	DriveData *dd;
	BonoboUIComponent *component;
	gchar *global_key;
	gchar *private_key;
	gchar *current_key;
	gchar *tmp_path;

	dd = create_drive_widget();
	applet = panel_applet_new (dd->button);

	dd->applet = applet;
	dd->orient = panel_applet_get_orient(PANEL_APPLET(applet));
	dd->sizehint = panel_applet_get_size(PANEL_APPLET(applet));
	g_signal_connect(G_OBJECT(dd->button),"button_press_event", G_CALLBACK(applet_button_press_cb), applet);

	g_signal_connect(G_OBJECT(applet),"change_orient",
					 G_CALLBACK(applet_change_orient), dd);
	g_signal_connect(G_OBJECT(applet),"change_size",
					 G_CALLBACK(applet_change_pixel_size), dd);
	g_signal_connect(G_OBJECT(applet),"save_yourself",
					 G_CALLBACK(applet_save_session), dd);
	g_signal_connect(GTK_OBJECT(applet),"destroy",
					 G_CALLBACK(destroy_drive_widget), dd);

	panel_applet_setup_menu (PANEL_APPLET (applet),
							 applet_menu_xml,
							 applet_menu_verbs,
							 dd);

	component = panel_applet_get_popup_component (PANEL_APPLET (applet));

	tmp_path = gnome_is_program_in_path("eject");
	if(!tmp_path) bonobo_ui_component_set_prop(component, "/commands/Eject", "hidden", "1", NULL);
	else g_free (tmp_path);

	redraw_pixmap(dd);
	gtk_widget_show (applet);
	start_callback_update(dd);
	return BONOBO_OBJECT(panel_applet_get_control(PANEL_APPLET(applet)));
}

static DriveData *
create_drive_widget()
{
	DriveData *dd;

	dd = g_new0 (DriveData, 1);
	dd->scale_applet = FALSE;
	dd->device_pixmap = 0;
	dd->button_pixmap = NULL;
	dd->mount_point = g_strdup("/mnt/floppy");
	dd->mount_base = g_strdup("/mnt");
	dd->propwindow = NULL;
	dd->autofs_friendly = FALSE;
	dd->custom_icon_in = NULL;
	dd->custom_icon_out = NULL;
	dd->error_dialog = NULL;
	dd->interval = 10;

	/* FIXME: Load properties after we get a private key */

	dd->button = gtk_button_new();
	gtk_widget_show(dd->button);
	g_signal_connect(G_OBJECT(dd->button),"clicked", G_CALLBACK(mount_cb), dd);
	dnd_init(dd);
	return(dd);
}

static gboolean
applet_button_press_cb(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	PanelApplet *applet = PANEL_APPLET(data);

	if (event->button == 1)
	{
		return(FALSE);
	}
	else
	{
		GTK_WIDGET_CLASS(PANEL_APPLET_GET_CLASS(applet))->button_press_event(data, event);
	}
	return(TRUE);
}

static void
dnd_init(DriveData *dd)
{
	gtk_drag_source_set(dd->button, GDK_BUTTON1_MASK,
			button_drag_types, n_button_drag_types,
			GDK_ACTION_COPY | GDK_ACTION_LINK | GDK_ACTION_ASK);
	g_signal_connect(G_OBJECT(dd->button), "drag_data_get",
					 G_CALLBACK(dnd_set_data_cb), dd);
	g_signal_connect(G_OBJECT(dd->button), "drag_begin",
					 G_CALLBACK(dnd_drag_begin_cb), dd);
}

static void
dnd_drag_begin_cb(GtkWidget *widget, GdkDragContext *context, gpointer data)
{
	DriveData *dd = data;
    gtk_drag_set_icon_pixbuf(context, gtk_image_get_pixbuf(GTK_IMAGE(dd->button_pixmap)), -5, -5);
}

static void
dnd_set_data_cb(GtkWidget *widget, GdkDragContext *context,
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
		gtk_selection_data_set (selection_data, selection_data->target, 8, text, strlen(text));
		g_free(text);
	}
	else
	{
		gtk_selection_data_set (selection_data, selection_data->target, 8, NULL, 0);
	}
	return;
}

void
start_callback_update(DriveData *dd)
{
	gint delay;

	delay = dd->interval * 1000;
	if (dd->timeout_id) gtk_timeout_remove(dd->timeout_id);
	dd->timeout_id = gtk_timeout_add(delay, (GtkFunction)drive_update_cb, dd);
}

static void
applet_change_orient(GtkWidget *w, PanelAppletOrient o, gpointer data)
{
	/* resize the applet and set the proper pixmaps */
	DriveData *dd = data;

	if (dd->orient == o) return;

	dd->orient = o;

	redraw_pixmap(dd);
	return;
}

static void
applet_change_pixel_size(GtkWidget *w, int size, gpointer data)
{
	DriveData *dd = data;

	if (dd->sizehint == size) return;
	dd->sizehint = size;
	redraw_pixmap(dd);
}

static gint
applet_save_session(PanelApplet *applet, gpointer data)
{
	DriveData *dd = data;
	/* FIXME: Save our prefs ? */
	return(FALSE);
}

static void
destroy_drive_widget(GtkWidget *widget, gpointer data)
{
	DriveData *dd = data;

	g_free(dd->mount_point);
	dd->mount_point = NULL;
	g_free(dd->mount_base);
	dd->mount_base = NULL;
	g_free(dd);
}

static void
browse_cb (PanelApplet *widget, gpointer data)
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
	if (str != NULL)
	{
		argv[0] = "nautilus";
		argv[1] = dd->mount_point;
		argv[2] = NULL;
		gnome_execute_async (NULL, 2, argv);
	}
	else
	{
		argv[0] = "gmc-client";
		argv[1] = "--create-window";
		argv[2] = dd->mount_point;
		argv[3] = NULL;
		gnome_execute_async (NULL, 3, argv);
	}
}

static void
eject_cb(PanelApplet *applet, gpointer data)
{
	DriveData *dd = data;

	eject(dd);

	return;
}

static void
help_cb (PanelApplet *widget, gpointer data)
{
}

static void
about_cb (PanelApplet *widget, gpointer data)
{
    static GtkWidget   *about     = NULL;

    static const gchar *authors[] =
    {
		"John Ellis <johne@bellatlantic.net>",
        "Chris Phelps <chicane@renient.com>",
        NULL
    };

    if (about != NULL)
    {
        gdk_window_show(about->window);
        gdk_window_raise(about->window);
        return;
    }
    
    about = gnome_about_new (_("Drive Mount Applet"), VERSION,
                             _("(C) 1999-2001 The GNOME Hackers\n"),
                             _("Applet for mounting and unmounting block volumes."),
                             authors,  NULL, NULL, NULL);

    g_signal_connect (G_OBJECT(about), "destroy",
                      G_CALLBACK(gtk_widget_destroyed), &about);
    gtk_widget_show (about);
}

/*
 *-------------------------------------------------------------------------
 * mount status checks
 *-------------------------------------------------------------------------
 */

static gint
device_is_in_mountlist(DriveData *dd)
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

static gint
get_device(const gchar *file)
{
	struct stat file_info;
	gint t;

	if (stat (file, &file_info) == -1)
		t = -1;
	else
		t = (gint)file_info.st_dev;

	return t;
}

static gint
device_is_mounted(DriveData *dd)
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

static void
update_pixmap(DriveData *dd, gint t)
{
	GdkPixbuf *pixbuf;
	GdkPixbuf *scaled;
	char **pmap_d_in;
	char **pmap_d_out;

	gint width;
	gint height;

	gchar *tiptext;
	gchar *text;

	gint hint = dd->sizehint;

    g_print("Updating pixmap...\n");
	if (dd->device_pixmap > icon_list_count - 1) dd->device_pixmap = 0;
	if (dd->device_pixmap < 0 && (!dd->custom_icon_in || !dd->custom_icon_out) ) dd->device_pixmap = 0;

	if (!dd->button_pixmap)
	{
		dd->button_pixmap = gtk_image_new();
		gtk_container_add(GTK_CONTAINER(dd->button), dd->button_pixmap);
        gtk_widget_show(dd->button_pixmap);
	}

	if (dd->device_pixmap < 0)
	{
		if (t)
		{
			pixbuf = gdk_pixbuf_new_from_file(dd->custom_icon_in, NULL);
		}
		else
		{
			pixbuf = gdk_pixbuf_new_from_file(dd->custom_icon_out, NULL);
		}

		if (pixbuf)
		{
			width = gdk_pixbuf_get_width(pixbuf);
			height = gdk_pixbuf_get_width(pixbuf);

			if (dd->orient == PANEL_APPLET_ORIENT_LEFT || dd->orient == PANEL_APPLET_ORIENT_RIGHT)
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
			scaled = gdk_pixbuf_scale_simple(pixbuf, width, height, GDK_INTERP_BILINEAR);
			gtk_image_set_from_pixbuf(GTK_IMAGE(dd->button_pixmap), scaled);
			g_object_unref(G_OBJECT(pixbuf));
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

		if (dd->orient == PANEL_APPLET_ORIENT_LEFT || dd->orient == PANEL_APPLET_ORIENT_RIGHT || hint <= 36)
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
			pixbuf = gdk_pixbuf_new_from_xpm_data((const char **)pmap_d_in);
		else
			pixbuf = gdk_pixbuf_new_from_xpm_data((const char **)pmap_d_out);
		scaled = gdk_pixbuf_scale_simple(pixbuf, width, height, GDK_INTERP_BILINEAR);
		gtk_image_set_from_pixbuf(GTK_IMAGE(dd->button_pixmap), scaled);
		g_object_unref(G_OBJECT(pixbuf));
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
	/* applet_widget_set_tooltip(APPLET_WIDGET(dd->applet), tiptext); */
	g_free(tiptext);
}

void
redraw_pixmap(DriveData *dd)
{
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

static gint
drive_update_cb(gpointer data)
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

static gint
mount_cb(GtkWidget *widget, gpointer data)
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
		dd->error_dialog = gtk_dialog_new_with_buttons(_("Drive Mount Applet Warning"), NULL, GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);
		gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dd->error_dialog)->vbox), gtk_label_new(str->str), FALSE, FALSE, 10);
		gtk_widget_show_all(dd->error_dialog);
        gtk_dialog_run(GTK_DIALOG(dd->error_dialog));
		gtk_widget_destroy(dd->error_dialog);
		dd->error_dialog = NULL;
	}
	g_string_free(str, TRUE);
	g_free(command_line);
	return FALSE;
}

static void
eject(DriveData *dd)
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
