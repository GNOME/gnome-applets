/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>
#include <panel-applet.h>
#include <egg-screen-exec.h>

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
#include "cdburn_v_in.xpm"
#include "cdburn_v_out.xpm"
#include "cdburn_h_in.xpm"
#include "cdburn_h_out.xpm"
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


static gboolean applet_factory (PanelApplet *applet, const gchar *iid,
				gpointer data);
static gboolean applet_fill (PanelApplet *applet);
static DriveData *create_drive_widget (PanelApplet *applet);
static void dnd_init (DriveData *dd);
static void dnd_drag_begin_cb (GtkWidget *widget, GdkDragContext *context,
			       gpointer data);
static void dnd_set_data_cb (GtkWidget *widget, GdkDragContext *context,
			     GtkSelectionData *selection_data, guint info,
			     guint time, gpointer data);
static void applet_change_orient (GtkWidget *w, PanelAppletOrient o,
				  gpointer data);
static void applet_change_pixel_size (GtkWidget *w, int size, gpointer data);
static gboolean button_press_hack (GtkWidget      *widget,
				   GdkEventButton *event,
				   GtkWidget      *applet);
static void destroy_drive_widget (GtkWidget *widget, gpointer data);
static gint device_is_in_mountlist (DriveData *dd);
static gint get_device (const gchar *file);
static gint device_is_mounted (DriveData *dd);
static void update_pixmap (DriveData *dd, gint t);
static gint drive_update_cb (gpointer data);
static void mount_cb (GtkWidget *widget, DriveData *dd);
static void eject (DriveData *dd);
static gboolean key_press_cb (GtkWidget *widget, GdkEventKey *event, DriveData *dd);

static void browse_cb (BonoboUIComponent *uic,
		       DriveData         *drivemount,
		       const char        *verb);
static void eject_cb  (BonoboUIComponent *uic,
		       DriveData         *drivemount,
		       const char        *verb);
static void help_cb   (BonoboUIComponent *uic,
		       DriveData         *drivemount,
		       const char        *verb);
static void about_cb  (BonoboUIComponent *uic,
		       DriveData         *drivemount,
		       const char        *verb);

/*
 *-------------------------------------------------------------------------
 *icon struct
 *-------------------------------------------------------------------------
 */

typedef struct _IconData IconData;
struct _IconData {
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
	 floppy_v_out_xpm},
	{
	 cdrom_h_in_xpm,
	 cdrom_h_out_xpm,
	 cdrom_v_in_xpm,
	 cdrom_v_out_xpm},
	{
	 cdburn_h_in_xpm,
	 cdburn_h_out_xpm,
	 cdburn_v_in_xpm,
	 cdburn_v_out_xpm},
	{
	 zipdrive_h_in_xpm,
	 zipdrive_h_out_xpm,
	 zipdrive_v_in_xpm,
	 zipdrive_v_out_xpm},
	{
	 harddisk_h_in_xpm,
	 harddisk_h_out_xpm,
	 harddisk_v_in_xpm,
	 harddisk_v_out_xpm},
	{
	 jazdrive_h_in_xpm,
	 jazdrive_h_out_xpm,
	 jazdrive_v_in_xpm,
	 jazdrive_v_out_xpm},
	{
	 NULL,
	 NULL,
	 NULL,
	 NULL}
};

static gint icon_list_count = 6;

/* Bonobo Verbs for our popup menu */
static const BonoboUIVerb applet_menu_verbs [] = {
	BONOBO_UI_UNSAFE_VERB ("Browse", browse_cb),
	BONOBO_UI_UNSAFE_VERB ("Eject", eject_cb),
	BONOBO_UI_UNSAFE_VERB ("Properties", properties_show),
	BONOBO_UI_UNSAFE_VERB ("Help", help_cb),
	BONOBO_UI_UNSAFE_VERB ("About", about_cb),
	BONOBO_UI_VERB_END
};

enum {
	TARGET_URI_LIST,
	TARGET_TEXT_PLAIN
};

static GtkTargetEntry button_drag_types[] = {
	{"text/uri-list", 0, TARGET_URI_LIST},
	{"text/plain", 0, TARGET_TEXT_PLAIN}
};
static gint n_button_drag_types = 2;


PANEL_APPLET_BONOBO_FACTORY ("OAFIID:GNOME_DriveMountApplet_Factory", PANEL_TYPE_APPLET,
			     "Drive-Mount-Applet", "0", applet_factory, NULL)

static gboolean
applet_factory (PanelApplet *applet,
		const gchar *iid, gpointer data)
{
	gboolean retval = FALSE;

	if (!strcmp (iid, "OAFIID:GNOME_DriveMountApplet")) {
		retval = applet_fill (applet);
	}
	return retval;
}

static gboolean
applet_fill (PanelApplet *applet)
{
	DriveData *dd;
	BonoboUIComponent *component;
	gchar *tmp_path;
	
	gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/drivemount-applet.png");
	panel_applet_set_flags (applet, PANEL_APPLET_EXPAND_MINOR);
	
	panel_applet_add_preferences (applet,
				      "/schemas/apps/drivemount-applet/prefs",
				      NULL);
	dd = create_drive_widget (applet);

	gtk_container_add (GTK_CONTAINER (applet), dd->button);

	dd->applet = GTK_WIDGET (applet);
	dd->tooltips = gtk_tooltips_new ();
	dd->orient = panel_applet_get_orient (PANEL_APPLET (applet));
	dd->sizehint = panel_applet_get_size (PANEL_APPLET (applet));
	g_signal_connect (G_OBJECT (dd->button), "button_press_event",
			  G_CALLBACK (button_press_hack), applet);

	g_signal_connect (G_OBJECT (applet), "change_orient",
			  G_CALLBACK (applet_change_orient), dd);
	g_signal_connect (G_OBJECT (applet), "change_size",
			  G_CALLBACK (applet_change_pixel_size), dd);
	g_signal_connect (GTK_OBJECT (applet), "destroy",
			  G_CALLBACK (destroy_drive_widget), dd);
			  
	g_signal_connect (applet, "key_press_event",
				  G_CALLBACK (key_press_cb), dd);

	panel_applet_setup_menu_from_file (PANEL_APPLET (applet),
                                           NULL,
				           "GNOME_DriveMountApplet.xml",
                                           NULL,
                                           applet_menu_verbs,
                                           dd);

	component = panel_applet_get_popup_component (PANEL_APPLET (applet));

	tmp_path = gnome_is_program_in_path ("eject");
	if (!tmp_path)
		bonobo_ui_component_set_prop (component, "/commands/Eject",
					      "hidden", "1", NULL);
	else
		g_free (tmp_path);

	redraw_pixmap (dd);
	gtk_widget_show (GTK_WIDGET (applet));
	start_callback_update (dd);
	return TRUE;
}

static DriveData *
create_drive_widget (PanelApplet *applet)
{
	DriveData *dd;

	dd = g_new0 (DriveData, 1);
	dd->applet = GTK_WIDGET (applet);

	properties_load (dd);

	dd->button = gtk_button_new ();
	gtk_widget_show (dd->button);
	g_signal_connect (G_OBJECT (dd->button), "clicked",
			  G_CALLBACK (mount_cb), dd);
	dnd_init (dd);
	return dd;
}

/* This is a hack around the fact that gtk+ doesn't
 * propogate button presses on button2/3.
 */
static gboolean
button_press_hack (GtkWidget      *widget,
                   GdkEventButton *event,
                   GtkWidget      *applet)
{
    if (event->button == 3 || event->button == 2) {
	gtk_propagate_event (applet, (GdkEvent *) event);

	return TRUE;
    }

    return FALSE;
}

static void
dnd_init (DriveData *dd)
{
	gtk_drag_source_set (dd->button, GDK_BUTTON1_MASK,
			     button_drag_types, n_button_drag_types,
			     GDK_ACTION_COPY | GDK_ACTION_LINK |
			     GDK_ACTION_ASK);
	g_signal_connect (G_OBJECT (dd->button), "drag_data_get",
			  G_CALLBACK (dnd_set_data_cb), dd);
	g_signal_connect (G_OBJECT (dd->button), "drag_begin",
			  G_CALLBACK (dnd_drag_begin_cb), dd);
}

static void
dnd_drag_begin_cb (GtkWidget *widget, GdkDragContext *context, gpointer data)
{
	DriveData *dd = data;

	gtk_drag_set_icon_pixbuf (context,
				  gtk_image_get_pixbuf (GTK_IMAGE
							(dd->button_pixmap)),
				  -5, -5);
}

static void
dnd_set_data_cb (GtkWidget *widget, GdkDragContext *context,
		 GtkSelectionData *selection_data, guint info,
		 guint time, gpointer data)
{
	DriveData *dd = data;

	if (dd && dd->mount_point) {
		gchar *text = NULL;

		switch (info) {
		case TARGET_URI_LIST:
			text = g_strconcat ("file:", dd->mount_point, "\r\n",
					    NULL);
			break;
		case TARGET_TEXT_PLAIN:
			text = g_strdup (dd->mount_point);
			break;
		}
		gtk_selection_data_set (selection_data, selection_data->target,
					8, text, strlen (text));
		g_free (text);
	} else {
		gtk_selection_data_set (selection_data, selection_data->target,
					8, NULL, 0);
	}
}

void
start_callback_update (DriveData *dd)
{
	gint delay;

	delay = dd->interval *1000;
	if (dd->timeout_id)
		gtk_timeout_remove (dd->timeout_id);
	dd->timeout_id =
		gtk_timeout_add (delay, (GtkFunction) drive_update_cb, dd);
}

static void
applet_change_orient (GtkWidget *w, PanelAppletOrient o, gpointer data)
{
	/* resize the applet and set the proper pixmaps */
	DriveData *dd = data;

	if (dd->orient == o)
		return;

	dd->orient = o;

	redraw_pixmap (dd);
}

static void
applet_change_pixel_size (GtkWidget *w, int size, gpointer data)
{
	DriveData *dd = data;

	if (dd->sizehint != size) {
		dd->sizehint = size;
		redraw_pixmap (dd);
	}
}

static void
destroy_drive_widget (GtkWidget *widget, gpointer data)
{
	DriveData *dd = data;

	if (dd->error_dialog != NULL) {
		gtk_widget_destroy (dd->error_dialog);
		dd->error_dialog = NULL;
	}
	
	g_free (dd->custom_icon_in);
	g_free (dd->custom_icon_out);

	g_free (dd->mount_point);
	g_free (dd->mount_base);
	g_free (dd);
}

static void
browse_cb (BonoboUIComponent *uic,
	   DriveData         *drivemount,
	   const char        *verb)
{
	GError *error = NULL;
	char   *command;

	if (!drivemount->mounted)
		mount_cb (NULL, drivemount);

	if (!drivemount->mounted)
		return;

	command = g_strdup_printf ("nautilus %s", drivemount->mount_point);
	egg_screen_execute_command_line_async (
		gtk_widget_get_screen (drivemount->applet), command, &error);
	g_free (command);
	if (error) {
		GtkWidget *dialog;
		char *msg;

		msg = g_strdup_printf (_("There was an error executing %s: %s"),
				       command,
				       error->message);
		dialog = hig_dialog_new (NULL /* parent */,
					 0 /* flags */,
					 GTK_MESSAGE_ERROR,
					 GTK_BUTTONS_OK,
					 _("Cannot browse device"),
					 msg);
		g_free (msg);

		g_signal_connect (dialog, "response",
				  G_CALLBACK (gtk_widget_destroy),
				  NULL);

		gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
		gtk_window_set_screen (GTK_WINDOW (dialog),
				       gtk_widget_get_screen (drivemount->applet));

		gtk_widget_show (dialog);

		g_error_free (error);
	}
}

static void
eject_cb (BonoboUIComponent *uic,
	  DriveData         *drivemount,
	  const char        *verb)
{
	eject (drivemount);
}

static void
help_cb (BonoboUIComponent *uic,
	 DriveData         *drivemount,
	 const char        *verb)

{
	GError *error = NULL;
	static GnomeProgram *applet_program = NULL;
	
	if (!applet_program) {
		int argc = 1;
		char *argv[2] = { "drivemount" };
		applet_program = gnome_program_init ("drivemount", VERSION,
						      LIBGNOME_MODULE, argc, argv,
     						      GNOME_PROGRAM_STANDARD_PROPERTIES, NULL);
	}

	egg_help_display_desktop_on_screen (
			applet_program, "drivemount", "drivemount", "drivemount",
			gtk_widget_get_screen (GTK_WIDGET (uic)),
			&error);

	if (error) {
		GtkWidget *error_dialog;
		char *msg;

		msg = g_strdup_printf (_("There was an error displaying help: %s"),
				       error->message);
		error_dialog = hig_dialog_new (NULL /* parent */,
					       0 /* flags */,
					       GTK_MESSAGE_ERROR,
					       GTK_BUTTONS_OK,
					       _("Error displaying help"),
					       msg);
		g_free (msg);

		g_signal_connect (error_dialog, "response",
				  G_CALLBACK (gtk_widget_destroy),
				  NULL);

		gtk_window_set_resizable (GTK_WINDOW (error_dialog), FALSE);
		gtk_window_set_screen (GTK_WINDOW (error_dialog),
				       gtk_widget_get_screen (GTK_WIDGET (uic)));
		gtk_widget_show (error_dialog);
		g_error_free (error);
	}
}

static void
about_cb (BonoboUIComponent *uic,
	  DriveData         *drivemount,
	  const char        *verb)
{
	static GtkWidget *about = NULL;
   	GdkPixbuf        *pixbuf;
   	GError           *error = NULL;
   	gchar            *file;
	
	static const gchar *authors[] = {
		"John Ellis <johne@bellatlantic.net>",
		"Chris Phelps <chicane@renient.com>",
		NULL
	};

	const gchar *documenters[] = {
		NULL
	};

	const gchar *translator_credits = _("translator_credits");

	if (about) {
		gtk_window_set_screen (GTK_WINDOW (about),
				       gtk_widget_get_screen (drivemount->applet));
		gtk_window_present (GTK_WINDOW (about));
		return;
	}

	file = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_PIXMAP, "drivemount-applet.png", FALSE, NULL);
   	pixbuf = gdk_pixbuf_new_from_file (file, &error);
   	g_free (file);
   
   	if (error) {
   		g_warning (G_STRLOC ": cannot open %s: %s", file, error->message);
		g_error_free (error);
	}

	about = gnome_about_new (_("Disk Mounter"), VERSION,
				 _("(C) 1999-2001 The GNOME Hackers\n"),
				 _
				 ("Applet for mounting and unmounting block volumes."),
				 authors,
				 documenters,
				 strcmp (translator_credits, "translator_credits") != 0 ? translator_credits : NULL,
				 pixbuf);
	if (pixbuf) 
   		gdk_pixbuf_unref (pixbuf);
   
   	gtk_window_set_wmclass (GTK_WINDOW (about), "disk mounter", "Disk Mounter");
	gtk_window_set_screen (GTK_WINDOW (about),
			       gtk_widget_get_screen (drivemount->applet));
   	g_signal_connect (G_OBJECT (about), "destroy",
			  G_CALLBACK (gtk_widget_destroyed), &about);
	gtk_widget_show (about);
}

/*
 *-------------------------------------------------------------------------
 *mount status checks
 *-------------------------------------------------------------------------
 */

static gint
device_is_in_mountlist (DriveData *dd)
{
	FILE *fp;
	gchar *command_line = "mount";
	gchar buf[256];
	gint found = FALSE;

	if (!dd->mount_point)
		return FALSE;

	fp = popen (command_line, "r");
	if (!fp) {
		g_print ("unable to run command: %s\n", command_line);
		return FALSE;
	}
	while (fgets (buf, sizeof (buf), fp)) {
		gchar *ptr;
		ptr = strstr (buf, dd->mount_point);
		if (ptr != NULL) {
			gchar *p;
			p = ptr + strlen (dd->mount_point);
			if (*p == ' ' || *p == '\0' || *p == '\n') {
				found = TRUE;
				break;
			}
		}
	}
	pclose (fp);
	return found;
}

static gint
get_device (const gchar *file)
{
	struct stat file_info;
	gint t;

	if (stat (file, &file_info) == -1)
		t = -1;
	else
		t = (gint) file_info.st_dev;
	return t;
}

static gint
device_is_mounted (DriveData *dd)
{
	if (!dd->mount_point || !dd->mount_base)
		return FALSE;

	if (!dd->autofs_friendly) {
		gint b, p;

		b = get_device (dd->mount_base);
		p = get_device (dd->mount_point);
		if (b == p || p == -1 || b == -1)
			return FALSE;
		else
			return TRUE;
	} else {
		if (device_is_in_mountlist (dd))
			return TRUE;
		else
			return FALSE;
	}
}

/*
 *-------------------------------------------------------------------------
 *image / widget setup and size changes
 *-------------------------------------------------------------------------
 */

static void
update_pixmap (DriveData *dd, gint t)
{
	GdkPixbuf *pixbuf;
	GdkPixbuf *scaled;
	char **pmap_d_in;
	char **pmap_d_out;
	gint pixmap;

	gint width;
	gint height;

	gchar *tiptext;
	gchar *text;

	gint hint = dd->sizehint;

	pixmap = dd->device_pixmap;
	if (dd->device_pixmap > icon_list_count - 1)
		pixmap = 0;
	if (dd->device_pixmap < 0
	    && (!dd->custom_icon_in || !dd->custom_icon_out))
		pixmap = 0;

	if (!dd->button_pixmap) {
		dd->button_pixmap = gtk_image_new ();
		gtk_container_add (GTK_CONTAINER (dd->button),
				   dd->button_pixmap);
		gtk_widget_show (dd->button_pixmap);
	}

	if (pixmap < 0) {
		if (t) {
			pixbuf = gdk_pixbuf_new_from_file (dd->custom_icon_in,
							   NULL);
		} else {
			pixbuf = gdk_pixbuf_new_from_file (dd->custom_icon_out,
							   NULL);
		}

		if (pixbuf) {
			width = gdk_pixbuf_get_width (pixbuf);
			height = gdk_pixbuf_get_height (pixbuf);

			if (dd->orient == PANEL_APPLET_ORIENT_LEFT
			    || dd->orient == PANEL_APPLET_ORIENT_RIGHT) {
				if (width > hint || dd->scale_applet) {
					height = (float) height *hint / width;

					width = hint;
				}
			} else {
				if (height > hint || dd->scale_applet) {
					width = (float) width *hint / height;

					height = hint;
				}
			}
			scaled = gdk_pixbuf_scale_simple (pixbuf, width, height,
							  GDK_INTERP_BILINEAR);
			gtk_image_set_from_pixbuf (GTK_IMAGE
						   (dd->button_pixmap), scaled);
			g_object_unref (G_OBJECT (pixbuf));
			g_object_unref (scaled);
		}
	} else {
		if (dd->scale_applet) {
			width = hint;
			height = (float) hint / ICON_WIDTH *ICON_HEIGHT;
		} else {
			width = ICON_WIDTH;
			height = ICON_HEIGHT;
		}

		if (dd->orient == PANEL_APPLET_ORIENT_LEFT
		    || dd->orient == PANEL_APPLET_ORIENT_RIGHT || hint <= 36) {
			pmap_d_in = icon_list[pixmap].pmap_h_in;
			pmap_d_out = icon_list[pixmap].pmap_h_out;
		} else {
			gint tmp;

			tmp = width;
			width = height;
			height = tmp;

			pmap_d_in = icon_list[pixmap].pmap_v_in;
			pmap_d_out = icon_list[pixmap].pmap_v_out;
		}

		if (t)
			pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **)
							       pmap_d_in);
		else
			pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **)
							       pmap_d_out);
		scaled = gdk_pixbuf_scale_simple (pixbuf, width, height,
						  GDK_INTERP_BILINEAR);
		gtk_image_set_from_pixbuf (GTK_IMAGE (dd->button_pixmap),
					   scaled);
		g_object_unref (G_OBJECT (pixbuf));
		g_object_unref (scaled);
	}

	if (t) {
		text = _(" mounted");
	} else {
		text = _(" not mounted");
	}
	tiptext = g_strconcat (dd->mount_point, text, NULL);
	gtk_tooltips_set_tip (dd->tooltips, dd->button, tiptext, NULL);
	g_free (tiptext);
}

void
redraw_pixmap (DriveData *dd)
{
	if (!device_is_mounted (dd)) {
		update_pixmap (dd, FALSE);
		dd->mounted = FALSE;
	} else {
		update_pixmap (dd, TRUE);
		dd->mounted = TRUE;
	}
}

/*
 *-------------------------------------------------------------------------
 *main callback loop
 *-------------------------------------------------------------------------
 */

static gint
drive_update_cb (gpointer data)
{
	DriveData *dd = data;

	if (!device_is_mounted (dd)) {
		/* device not mounted */
		if (dd->mounted) {
			update_pixmap (dd, FALSE);
			dd->mounted = FALSE;
		}
	} else {
		/* device mounted */
		if (!dd->mounted) {
			update_pixmap (dd, TRUE);
			dd->mounted = TRUE;
		}
	}
	return TRUE;
}

/*
 *-------------------------------------------------------------------------
 *mount calls
 *-------------------------------------------------------------------------
 */

static void
mount_cb (GtkWidget *widget,
	  DriveData *dd)
{
	gchar *command_line;
	gchar buf[512];
	FILE *fp;
	GString *str;
	gint check = device_is_mounted (dd);

	/* Stop the user from displaying zillions of error messages */
	if (dd->error_dialog) {
		gtk_window_set_screen (GTK_WINDOW (dd->error_dialog),
				       gtk_widget_get_screen (dd->applet));
		gtk_window_present (GTK_WINDOW (dd->error_dialog));
		return;
	}

	if (!check) {
		command_line =
			g_strdup_printf ("mount %s 2>&1", dd->mount_point);
	} else {
		command_line =
			g_strdup_printf ("umount %s 2>&1", dd->mount_point);
	}

	fp = popen (command_line, "r");

	if (!fp) {
		printf ("unable to run command: %s\n", command_line);
		g_free (command_line);
		return;
	}

	str = g_string_new (NULL);

	while (fgets (buf, sizeof (buf), fp) != NULL) {
		gchar *b = buf;
		const gchar *charset;
		if (!g_get_charset (&charset)) {
			b = g_convert (buf, -1, "UTF-8", charset, NULL, NULL, NULL);
		} else {
			b =g_strdup (buf);
		}
		g_string_append (str, b);
		g_free (b);
	}
	pclose (fp);

	dd->mounted = device_is_mounted (dd);

	if (check != dd->mounted) {
		/* success! */
		update_pixmap (dd, dd->mounted);

		/* eject after unmounting, if enabled */
		if (check && dd->auto_eject) {
			eject (dd);
		}
	} else {
		g_string_prepend (str, _("\" reported:\n"));
		g_string_prepend (str, command_line);
		g_string_prepend (str, _("Drivemount command failed.\n\""));

		dd->error_dialog = hig_dialog_new (NULL /* parent */,
						   GTK_DIALOG_MODAL /* flags */,
						   GTK_MESSAGE_ERROR,
						   GTK_BUTTONS_OK,
						   _("Cannot mount device"),
						   str->str);
		gtk_window_set_screen (GTK_WINDOW (dd->error_dialog),
				       gtk_widget_get_screen (dd->applet));

		gtk_widget_show_all (dd->error_dialog);
		gtk_dialog_run (GTK_DIALOG (dd->error_dialog));
		gtk_widget_destroy (dd->error_dialog);
		dd->error_dialog = NULL;
	}
	g_string_free (str, TRUE);
	g_free (command_line);
	return;
}

static void
eject (DriveData *dd)
{
	gchar *command_line;
	gchar buffer[512];
	gchar dn[256];		/* Devicename */
	gchar mp[256];		/* Mountpoint */
	FILE *ml;		/* Mountlist */
	gint found = FALSE;

	if (!dd->mount_point)
		return;

	/*
	 * Search the output of mount for dd->mount_point
	 * and use the corresponting device name
	 * as argument for eject
	 * if the device is not mounted currently, use
	 * /etc/fstab for the check
	 */

	if (dd->mounted) {
		ml = popen ("mount", "r");
		while (ml && fgets (buffer, sizeof (buffer), ml)) {
			if (sscanf (buffer, "%255s %*s %255s", dn, mp) == 2 &&
			    (mp && strcmp (mp, dd->mount_point) == 0)) {
				found = TRUE;
				break;
			}
		}
		if (ml)
			pclose (ml);
	} else {
		ml = fopen ("/etc/fstab", "r");
		
		while (ml && fgets (buffer, sizeof (buffer), ml)) {
			if (sscanf (buffer, "%255s %255s", dn, mp) == 2 &&
			    strcmp (mp, dd->mount_point) == 0) {
				found = TRUE;
				break;
			}
		}
		if (ml)
			fclose (ml);
	}

	if (!found) {		/* mp != dd->mount_point */
		printf ("WARNING: drivemount.c ... dd->mount_point not found in list\
			 (output of mount, or /etc/fstab) \n");
		return;
	}

	if (dd->mounted) {
		command_line = g_strdup_printf ("eject -u '%s'", dn);
		/* perhaps it doesn't like the -u option */
		if (system (command_line) != 0) {
			g_free (command_line);
			command_line = g_strdup_printf ("eject '%s'", dn);
			gnome_execute_shell (NULL, command_line);
		}
		g_free (command_line);
	} else {
		command_line = g_strdup_printf ("eject '%s'", dn);
		gnome_execute_shell (NULL, command_line);
		g_free (command_line);
	}
}

static gboolean 
key_press_cb (GtkWidget *widget, GdkEventKey *event, DriveData *dd)
{
	if (event->state != GDK_CONTROL_MASK)
		return FALSE;
	else {
		switch (event->keyval) {
	
		case GDK_e:
			eject (dd);
			return TRUE;
		case GDK_b:
			browse_cb (NULL, dd, NULL);
			return TRUE;
		default:
			break;
		}
	}

	return FALSE;


}

/* stolen from gsearchtool */
GtkWidget*
hig_dialog_new (GtkWindow      *parent,
		GtkDialogFlags flags,
		GtkMessageType type,
		GtkButtonsType buttons,
		const gchar    *header,
		const gchar    *message)
{
	GtkWidget *dialog;
	GtkWidget *dialog_vbox;
	GtkWidget *dialog_action_area;
	GtkWidget *hbox;
	GtkWidget *vbox;
	GtkWidget *label;
	GtkWidget *button;
	GtkWidget *image;
	gchar     *title;

	dialog = gtk_dialog_new ();
	
	gtk_dialog_set_has_separator (GTK_DIALOG (dialog), FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (dialog), 5);
	gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
	gtk_window_set_title (GTK_WINDOW (dialog), "");
  
	dialog_vbox = GTK_DIALOG (dialog)->vbox;
	gtk_box_set_spacing (GTK_BOX (dialog_vbox), 14);

	hbox = gtk_hbox_new (FALSE, 12);
	gtk_box_pack_start (GTK_BOX (dialog_vbox), hbox, FALSE, FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
	gtk_widget_show (hbox);

	if (type == GTK_MESSAGE_ERROR) {
		image = gtk_image_new_from_stock ("gtk-dialog-error", GTK_ICON_SIZE_DIALOG);
	} else if (type == GTK_MESSAGE_QUESTION) {
		image = gtk_image_new_from_stock ("gtk-dialog-question", GTK_ICON_SIZE_DIALOG);
	} else {
		g_assert_not_reached ();
	}
	gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);
	gtk_widget_show (image);

	vbox = gtk_vbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);
	gtk_widget_show (vbox);
	
	title = g_strconcat ("<b>", header, "</b>", NULL);
	label = gtk_label_new (title);  
	gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
	gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_widget_show (label);
	g_free (title);
	
	label = gtk_label_new (message);
	gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_widget_show (label);
	
	dialog_action_area = GTK_DIALOG (dialog)->action_area;
	gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area), GTK_BUTTONBOX_END);

	switch (buttons) 
  	{		
		case GTK_BUTTONS_OK_CANCEL:
	
			button = gtk_button_new_from_stock ("gtk-cancel");
  			gtk_widget_show (button);
  			gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button, GTK_RESPONSE_CANCEL);
  			GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);

		  	button = gtk_button_new_from_stock ("gtk-ok");
  			gtk_widget_show (button);
  			gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button, GTK_RESPONSE_OK);
  			GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
			break;
		
		case GTK_BUTTONS_OK:
		
			button = gtk_button_new_from_stock ("gtk-ok");
			gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button, GTK_RESPONSE_OK);
			GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
			gtk_widget_show (button);
			break;
		
		default:
			g_warning ("Unhandled GtkButtonsType");
			break;
  	}

	if (parent != NULL) {
		gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (parent));
	}
	if (flags & GTK_DIALOG_MODAL) {
		gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
	}
	if (flags & GTK_DIALOG_DESTROY_WITH_PARENT) {
		gtk_window_set_destroy_with_parent (GTK_WINDOW (dialog), TRUE);
	}
	
  	return dialog;
}

