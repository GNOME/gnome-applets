/* 
 * Copyright (C) 2001, 2002 Free Software Foundation
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Authors : Eskil Heyn Olsen <eskil@eskil.dk>
 * 	Bastien Nocera <hadess@hadess.net> for the Gnome2 port
 */

#include <config.h>

#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <dirent.h>

#include <gnome.h>
#include <panel-applet.h>
#include <panel-applet-gconf.h>
#include <glade/glade.h>

#define CFG_DEVICE "eth0"
#define CFG_UPDATE_INTERVAL 2

typedef enum {
	BUSTED_LINK,
	NO_LINK,
	NONE,
} AnimationState;

typedef struct {
	PanelApplet base;
	gchar *device;
	gboolean show_percent, show_dialogs;

	GList *devices;

	/* a glist of char*, pointing to available images */
	GList *images;
	/* a glist of char*, pointing to the no-link-XX images if any) */
	GList *no_link_images;
	/* a glist of char*, pointing to the broken-XX images (if any) */
	GList *broken_images;
	/* contains pointers into the images GList.
	 * 0-100 are for link */
	GdkPixbuf*pixmaps[101];
	/* pointer to the current used file name */
	GdkPixbuf *current_pixbuf;
	/* set to true when the applet is display animated connection loss */
	gboolean flashing;

	GtkWidget *pct_label;
	GtkWidget *pixmap;
	AnimationState state;
	guint animate_timer;
	guint timeout_handler_id;
	FILE *file;
	GtkTooltips *tips;
	GtkWidget *prefs;
} WirelessApplet;

char *pixmap_extensions[] = 
{
	"png",
	"xpm",
	NULL
};

static GladeXML *xml = NULL;
static gchar* glade_file=NULL;

static void show_error_dialog (gchar*,...);
static void show_warning_dialog (gchar*,...);
static void show_message_dialog (gchar*,...);
static int wireless_applet_timeout_handler (WirelessApplet *applet);
static void wireless_applet_properties_dialog (BonoboUIComponent *uic,
		WirelessApplet *applet);
static void wireless_applet_about_cb (BonoboUIComponent *uic,
		WirelessApplet *applet);

static const BonoboUIVerb wireless_menu_verbs [] = {
	BONOBO_UI_UNSAFE_VERB ("WirelessProperties",
			wireless_applet_properties_dialog),
	BONOBO_UI_UNSAFE_VERB ("WirelessAbout",
			wireless_applet_about_cb),
	BONOBO_UI_VERB_END
};

static GType
wireless_applet_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static const GTypeInfo info = {
			sizeof (PanelAppletClass),
			NULL, NULL, NULL, NULL, NULL,
			sizeof (WirelessApplet),
			0, NULL, NULL
		};

		type = g_type_register_static (
				PANEL_TYPE_APPLET, "WirelessApplet", &info, 0);
	}

	return type;
}

static void 
wireless_applet_draw (WirelessApplet *applet, int percent)
{
	const char *label_text;
	char *tmp;

	if (!GTK_WIDGET_REALIZED (applet->pixmap)) {
		return;
	}

	/* Update the percentage */
	if (percent > 0) {
		tmp = g_strdup_printf ("%2.0d%%", percent);
	} else {
		tmp = g_strdup_printf ("N/A");
	}

	label_text = gtk_label_get_text (GTK_LABEL (applet->pct_label));
	if (g_strcasecmp (tmp, label_text) != 0)
	{
		gtk_tooltips_set_tip (applet->tips,
				GTK_WIDGET (applet),
				tmp, NULL);
		gtk_label_set_text (GTK_LABEL (applet->pct_label), tmp);
	}
	g_free (tmp);

	/* Update the image */
	percent = CLAMP (percent, 0, 100);

	if (applet->pixmaps[percent] != applet->current_pixbuf)
	{
		applet->current_pixbuf = (GdkPixbuf *)applet->pixmaps[percent];
		if ( !applet->flashing)
		{
			gtk_image_set_from_pixbuf
				(GTK_IMAGE (applet->pixmap),
				 applet->current_pixbuf);
		}
	}
}


static gboolean
wireless_applet_animate_timeout (WirelessApplet *applet) 
{	
	static int num = 0;
	GList *image;
	GList *animation_list = NULL;

	switch (applet->state) {
	case NO_LINK:
		animation_list = applet->no_link_images;
		break;
	case BUSTED_LINK:
		animation_list = applet->broken_images;
		break;
	case NONE:
		return;
	default:
		g_assert_not_reached ();
		break;
	};

	if (num >= g_list_length (animation_list)) {
		num = 0;
	}

	image = g_list_nth (animation_list, num);
	gtk_image_set_from_pixbuf (GTK_IMAGE (applet->pixmap),
			(GdkPixbuf*)image->data);
	num++;

	return TRUE;
}

static void
wireless_applet_start_animation (WirelessApplet *applet) 
{
	applet->animate_timer = gtk_timeout_add (500, 
			(GtkFunction)wireless_applet_animate_timeout,
			applet);
}

static void
wireless_applet_stop_animation (WirelessApplet *applet) 
{
	if (applet->animate_timer > 0)
		gtk_timeout_remove (applet->animate_timer);
	gtk_image_set_from_pixbuf (GTK_IMAGE (applet->pixmap),
			applet->current_pixbuf);	
}

static void
wireless_applet_animation_state (WirelessApplet *applet) 
{
	if (applet->flashing == FALSE) {
		wireless_applet_start_animation (applet);
		applet->flashing = TRUE;
	}
}

static void
wireless_applet_update_state (WirelessApplet *applet,
			     char *device,
			     double link,
			     long int level,
			     long int noise)
{
	int percent;

	/* Calculate the percentage based on the link quality */
	if (level < 0) {
		percent = -1;
	} else {
		if (link<1) {
			percent = 0;
		} else {
			percent = (int)rint ((log (link) / log (92)) * 100.0);
			percent = CLAMP (percent, 0, 100);
		}
	}

	if (percent < 0) {
		applet->state = BUSTED_LINK;
		wireless_applet_animation_state (applet);
	} else if (percent == 0) {
		applet->state = NO_LINK;
		wireless_applet_animation_state (applet);
	} else if (applet->flashing) {
		applet->state = NONE;
		wireless_applet_stop_animation (applet);
	}

	wireless_applet_draw (applet, percent);
}

static void
wireless_applet_load_theme_image (WirelessApplet *applet,
				 const char *themedir,
				 const char *filename) 
{
	int j;
	char *dot_pos = strrchr (filename, '.') + 1; /* only called if a previous strrchr worked */
	char *file = g_strdup_printf ("%s/%s", themedir, filename);

	/* Check the allowed extensions */
	for (j = 0; pixmap_extensions[j]; j++) {
		if (strcasecmp (dot_pos, pixmap_extensions[j])==0) { 
			int i;
			int pixmap_offset_begin = 0;
			int pixmap_offset_end = 0;
			char *dupe;
			gboolean check_range = FALSE;

			if (strncmp (filename, "signal-", 7) == 0) {
				sscanf (filename, "signal-%d-%d.",
					&pixmap_offset_begin, &pixmap_offset_end);
				check_range = TRUE;
			} else if (strncmp (filename, "no-link-", 8) == 0) {
				GdkPixbuf *pixbuf = NULL;
				pixbuf = gdk_pixbuf_new_from_file (file, NULL);
				if (pixbuf)
					applet->no_link_images = g_list_prepend (applet->no_link_images,
												      pixbuf);
			} else if (strncmp (filename, "broken-", 7) == 0) {
				GdkPixbuf *pixbuf = NULL;
				pixbuf = gdk_pixbuf_new_from_file (file, NULL);
				if (pixbuf)
					applet->broken_images = g_list_prepend (applet->broken_images,
												      pixbuf);
			}

			if (check_range) {
				GdkPixbuf *pixbuf = NULL;
				pixbuf = gdk_pixbuf_new_from_file (file, NULL);
				if (pixbuf)
					applet->images = g_list_prepend (applet->images,
												      pixbuf);
				for (i = pixmap_offset_begin; i <= pixmap_offset_end; i++) {
					if (applet->pixmaps[i] != NULL) {
						show_warning_dialog ("Probable image overlap in\n"
								"%s.", filename);

					} else {
						applet->pixmaps[i] = pixbuf;
					}
				}
			}
		}
	}
	
	g_free (file);
}

static void
wireless_applet_load_theme (WirelessApplet *applet) {
	DIR *dir;
	struct dirent *dirent;
	char *pixmapdir;

	pixmapdir = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_PIXMAP,
			"wireless-applet/", FALSE, NULL);
	dir = opendir (pixmapdir);

	/* blank out */
	if (applet->images) {
		int j;
		g_list_foreach (applet->no_link_images, (GFunc)g_object_unref, NULL);
		g_list_free (applet->no_link_images);
		applet->no_link_images = NULL;

		g_list_foreach (applet->broken_images, (GFunc)g_object_unref, NULL);
		g_list_free (applet->broken_images);
		applet->broken_images = NULL;

		g_list_foreach (applet->images, (GFunc)g_object_unref, NULL);
		g_list_free (applet->images);
		applet->images = NULL;
		for (j=0; j < 101; j++) {
			applet->pixmaps[j] = NULL;
		}
	}

	if (!dir) {
		show_error_dialog (_("The images necessary to run the wireless monitor are missing.\nPlease make sure that it is correctly installed."));
	} else 
		while ((dirent = readdir (dir)) != NULL) {
			if (*dirent->d_name != '.') {
				if (strrchr (dirent->d_name, '.')!=NULL) {
					wireless_applet_load_theme_image (applet, 
									 pixmapdir,
									 dirent->d_name);
				}
			}
		}
#if 0
	if (applet->no_link_images && g_list_length (applet->no_link_images) > 1) {
		applet->no_link_images = g_list_sort (applet->no_link_images,
							  (GCompareFunc)g_ascii_strncasecmp);
	}
	
	if (applet->broken_images && g_list_length (applet->broken_images) > 1) {
		applet->broken_images = g_list_sort (applet->broken_images,
							 (GCompareFunc)g_ascii_strncasecmp);
	}
#endif
	g_free (pixmapdir);
}

static void
wireless_applet_set_device (WirelessApplet *applet, gchar *device) {
	g_free (applet->device);
	applet->device = g_strdup (device);
}

static void
wireless_applet_set_show_percent (WirelessApplet *applet, gboolean show) {
	applet->show_percent = show;
	if (applet->show_percent) {
		/* reeducate label */
		gtk_widget_show_all (applet->pct_label);
	} else {
		gtk_widget_hide_all (applet->pct_label);
	}
}

static void
wireless_applet_set_show_dialogs (WirelessApplet *applet, gboolean show) {
	applet->show_dialogs = show;
}

/* check stats, modify the state attribute */
static void
wireless_applet_read_device_state (WirelessApplet *applet)
{
	long int level, noise;
	double link;
	char device[256];
	char line[256];

	/* resest list of available wireless devices */
	g_list_foreach (applet->devices, (GFunc)g_free, NULL);
	g_list_free (applet->devices);
	applet->devices = NULL;

	/* Here we begin to suck... */
	do {
		char *ptr;

		fgets (line, 256, applet->file);

		if (feof (applet->file)) {
			break;
		}

		if (line[6] == ':') {
			char *tptr = line;
			while (isspace (*tptr)) tptr++;
			strncpy (device, tptr, 6);
			(*strchr(device, ':')) = 0;
			ptr = line + 12;

			/* Add the devices encountered to the list of possible devices */
			applet->devices = g_list_prepend (applet->devices, g_strdup (device));

			/* is it the one we're supposed to monitor ? */
			if (g_strcasecmp (applet->device, device)==0) {
				link = strtod (ptr, &ptr);
				ptr++;

				level = strtol (ptr, &ptr, 10);
				ptr++;

				noise = strtol (ptr, &ptr, 10);
				ptr++;

				wireless_applet_update_state (applet, device, link, level, noise);
			}
		}
	} while (1);

	if (g_list_length (applet->devices)==1) {
		wireless_applet_set_device (applet,
				(char*)applet->devices->data);
	} else {
		wireless_applet_update_state (applet,
				applet->device, -1, -1, -1);
	}

	/* rewind the /proc/net/wireless file */
	rewind (applet->file);
}

static int
wireless_applet_timeout_handler (WirelessApplet *applet)
{
	if (applet->file == NULL) {
		wireless_applet_update_state (applet,
				applet->device, -1, -1, -1);
		return FALSE;
	}

	wireless_applet_read_device_state (applet);

	return TRUE;
}


static void 
show_error_dialog (gchar *mesg,...) 
{
	GtkWidget *dialog;
	char *tmp;
	va_list ap;

	va_start (ap,mesg);
	tmp = g_strdup_vprintf (mesg,ap);
	dialog = gtk_message_dialog_new (NULL,
			0, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
			mesg, NULL);
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
	g_free (tmp);
	va_end (ap);
}

static void 
show_warning_dialog (gchar *mesg,...) 
{
	GtkWidget *dialog;
	char *tmp;
	va_list ap;

	va_start (ap,mesg);
	tmp = g_strdup_vprintf (mesg,ap);
	dialog = gtk_message_dialog_new (NULL,
			0, GTK_MESSAGE_WARNING, GTK_BUTTONS_CLOSE,
			mesg, NULL);
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
	g_free (tmp);
	va_end (ap);
}


static void
show_message_dialog (char *mesg,...)
{
	GtkWidget *dialog;
	char *tmp;
	va_list ap;

	va_start (ap,mesg);
	tmp = g_strdup_vprintf (mesg,ap);
	dialog = gtk_message_dialog_new (NULL,
			0, GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE,
			mesg, NULL);
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
	g_free (tmp);
	va_end (ap);
}

static void
start_file_read (WirelessApplet *applet)
{
	applet->file = fopen ("/proc/net/wireless", "rt");
	if (applet->file == NULL) {
		gtk_tooltips_set_tip (applet->tips,
				GTK_WIDGET (applet),
				_("No Wireless Devices"),
				NULL);
		show_error_dialog (_("There doesn't seem to be any wireless devices configured on your system.\nPlease verify your configuration if you think this is incorrect."));
	}
}

static void
wireless_applet_load_properties (WirelessApplet *applet)
{
	applet->device = panel_applet_gconf_get_string (PANEL_APPLET (applet),
			"device", NULL);

	/* Oooh, new applet, let's put in the defaults */
	if (applet->device == NULL)
	{
		applet->device = g_strdup (CFG_DEVICE);
		applet->show_percent = TRUE;
		applet->show_dialogs = TRUE;
		return;
	}

	applet->show_percent = panel_applet_gconf_get_bool
		(PANEL_APPLET (applet), "percent", NULL);
	applet->show_dialogs = panel_applet_gconf_get_bool
		(PANEL_APPLET (applet), "dialog", NULL);
}

static void
wireless_applet_save_properties (WirelessApplet *applet)
{
	if (applet->device)
		panel_applet_gconf_set_string (PANEL_APPLET (applet),
			"device", applet->device, NULL);
	panel_applet_gconf_set_bool (PANEL_APPLET (applet),
			"percent", applet->show_percent, NULL);
	panel_applet_gconf_set_bool (PANEL_APPLET (applet),
			"dialog", applet->show_dialogs, NULL);
}

static void
wireless_applet_option_change (GtkWidget *widget, gpointer user_data)
{
	GtkWidget *entry;
	GtkWidget *menu = NULL;
	char *str;
	WirelessApplet *applet = (WirelessApplet *)user_data;

	/* Get all the properties and update the applet */
	entry = g_object_get_data (G_OBJECT (applet->prefs),
			"show-percent-button");
	wireless_applet_set_show_percent (applet,
			gtk_toggle_button_get_active
			(GTK_TOGGLE_BUTTON (entry)));

	entry = g_object_get_data (G_OBJECT (applet->prefs),
			"show-dialog-button");
	wireless_applet_set_show_dialogs (applet, 
			gtk_toggle_button_get_active
			(GTK_TOGGLE_BUTTON (entry)));

	entry = g_object_get_data (G_OBJECT (applet->prefs), "device-menu");

	menu = gtk_menu_get_active
		(GTK_MENU (gtk_option_menu_get_menu (GTK_OPTION_MENU (entry))));
	if (menu) {
		str = g_object_get_data (G_OBJECT (menu), "device-selected");
		wireless_applet_set_device (applet, str);
	}

	/* Save the properties */
	wireless_applet_save_properties (applet);
}

static void
wireless_applet_properties_dialog (BonoboUIComponent *uic,
				  WirelessApplet *applet)
{
	static GtkWidget *global_property_box = NULL,
		*glade_property_box = NULL;
	GtkWidget *pct, *dialog, *device;

	g_return_if_fail (PANEL_IS_APPLET (PANEL_APPLET (applet)));

	if (applet->prefs != NULL)
	{
		gtk_widget_show (applet->prefs);
		gtk_window_present (GTK_WINDOW (applet->prefs));
		return;
	}

	if (global_property_box == NULL) {
		xml = glade_xml_new (glade_file, NULL, NULL);
		glade_property_box = glade_xml_get_widget (xml,"dialog1");
	}

	applet->prefs = glade_property_box;

	pct = glade_xml_get_widget (xml, "pct_check_button");
	dialog = glade_xml_get_widget (xml, "dialog_check_button");
	device = glade_xml_get_widget (xml, "device_menu");

	/* Set the show-percent thingy */
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pct),
			applet->show_percent);
	g_signal_connect (GTK_OBJECT (pct),
			"toggled",
			GTK_SIGNAL_FUNC (wireless_applet_option_change),
			applet);
	gtk_object_set_data (GTK_OBJECT (applet->prefs),
			"show-percent-button", pct);

	/* Set the show-dialog thingy */
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog),
			applet->show_dialogs);
	g_signal_connect (GTK_OBJECT (dialog),
			"toggled",
			GTK_SIGNAL_FUNC (wireless_applet_option_change),
			applet);
	gtk_object_set_data (GTK_OBJECT (applet->prefs),
			"show-dialog-button", dialog);

        /* Set the device menu */
	gtk_option_menu_remove_menu (GTK_OPTION_MENU (device));
	{
		GtkWidget *menu;
		GtkWidget *item;
		GList *d;
		int idx = 0, choice = 0;

		menu = gtk_menu_new ();

		for (d = applet->devices; d != NULL; d = g_list_next (d)) {
			item = gtk_menu_item_new_with_label ((char*)d->data);
			gtk_menu_shell_append  (GTK_MENU_SHELL (menu),item);
			gtk_object_set_data_full (GTK_OBJECT (item), 
					"device-selected",
					g_strdup (d->data),
					g_free);
			g_signal_connect (GTK_OBJECT (item),
					"activate",
					GTK_SIGNAL_FUNC (wireless_applet_option_change),
					applet);

			if ((applet->device != NULL)
					&& (d->data != NULL)
					&& strcmp (applet->device, d->data)==0)
			{
				choice = idx;
			}
			idx++;
		}
		if (applet->devices == NULL) {
			char *markup;
			GtkWidget *label;
			
			label = gtk_label_new (NULL);
			markup = g_strdup_printf ("<i>%s</i>",
					_("No Wireless Devices"));
			gtk_label_set_markup (GTK_LABEL (label), markup);
			g_free (markup);

			item = gtk_menu_item_new ();
			gtk_container_add (GTK_CONTAINER (item), label);
			gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
		}
		gtk_option_menu_set_menu (GTK_OPTION_MENU (device), menu);
		gtk_option_menu_set_history (GTK_OPTION_MENU (device), choice);
	}
	gtk_object_set_data (GTK_OBJECT (applet->prefs), "device-menu", device);

	g_signal_connect (GTK_OBJECT (applet->prefs),
			"response", 
			GTK_SIGNAL_FUNC (gtk_widget_destroy),
			NULL);
	g_signal_connect (GTK_OBJECT (applet->prefs),
			"destroy",
			GTK_SIGNAL_FUNC (gtk_widget_destroy),
			NULL);

	g_object_add_weak_pointer (G_OBJECT (applet->prefs),
			(void**)&(applet->prefs));

	gtk_widget_show_all (applet->prefs);
}

static void
wireless_applet_about_cb (BonoboUIComponent *uic, WirelessApplet *applet)
{
	static GtkWidget *about = NULL;
	GdkPixbuf *pixbuf;
	char *file;
	const gchar *authors[] = {
		"Eskil Heyn Olsen <eskil@eskil.org>",
		"Bastien Nocera <hadess@hadess.net> (Gnome2 port)",
		NULL
	};
	const gchar *documenters[] = { NULL };
	const gchar *translator_credits = _("translator_credits");

	if (about != NULL)
	{
		gtk_widget_show (about);
		gtk_window_present (GTK_WINDOW (about));
		return;
	}

	file = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_PIXMAP,
			"wireless-applet/wireless-applet.png", FALSE, NULL);
	pixbuf = gdk_pixbuf_new_from_file (file, NULL);
	g_free (file);

	about = gnome_about_new (
			_("Wireless Link Monitor"),
			VERSION,
			_("(C) 2001, 2002 Free Software Foundation "),
			_("This utility shows the status of a wireless link."),
			authors,
			documenters,
			strcmp (translator_credits, "translator_credits") != 0 ? translator_credits : NULL,
			pixbuf);

	g_object_unref (pixbuf);
	
	gtk_widget_show (about);

	g_object_add_weak_pointer (G_OBJECT (about),
			(void**)&about);

	return;
}

static void
wireless_applet_destroy (WirelessApplet *applet, gpointer horse)
{
	g_free (applet->device);

	g_list_foreach (applet->no_link_images, (GFunc)g_object_unref, NULL);
	g_list_free (applet->no_link_images);

	g_list_foreach (applet->broken_images, (GFunc)g_object_unref, NULL);
	g_list_free (applet->broken_images);

	g_list_foreach (applet->images, (GFunc)g_object_unref, NULL);
	g_list_free (applet->images);

	g_list_foreach (applet->devices, (GFunc)g_free, NULL);
	g_list_free (applet->devices);

	if (applet->animate_timer > 0) {
		gtk_timeout_remove (applet->animate_timer);
		applet->animate_timer = 0;
	}

	if (applet->timeout_handler_id > 0) {
		gtk_timeout_remove (applet->timeout_handler_id);
		applet->timeout_handler_id = 0;
	}

	if (applet->prefs) {
		gtk_widget_destroy (applet->prefs);
		applet->prefs = NULL;
	}

	if (applet->file)
		fclose (applet->file);
}

static GtkWidget *
wireless_applet_new (WirelessApplet *applet)
{
	GtkWidget *box;

	/* this ensures that properties are loaded */
	wireless_applet_load_properties (applet);
	wireless_applet_load_theme (applet);

	/* construct pixmap widget */
	applet->pixmap = gtk_image_new_from_pixbuf (applet->pixmaps[0]);
	gtk_widget_show (applet->pixmap);

	/* construct pct widget */
	applet->pct_label = gtk_label_new (NULL);

	if (applet->show_percent == TRUE) {
		gtk_widget_show (applet->pct_label);
	}

	/* pack */
	box = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (box), applet->pixmap, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (box), applet->pct_label, FALSE, FALSE, 0);
	/* note, I don't use show_all, because this way the percent label is
	 * only realised if it's enabled */
	gtk_widget_show (box);
	gtk_container_add (GTK_CONTAINER (applet), box);

	applet->tips = gtk_tooltips_new ();
	applet->prefs = NULL;

	g_signal_connect (GTK_OBJECT (applet),"destroy",
			   GTK_SIGNAL_FUNC (wireless_applet_destroy),NULL);

	/* Setup the menus */
	panel_applet_setup_menu_from_file (PANEL_APPLET (applet),
			NULL,
			"GNOME_WirelessApplet.xml",
			NULL,
			wireless_menu_verbs,
			applet);

	start_file_read (applet);
	wireless_applet_timeout_handler (applet);

	applet->timeout_handler_id = gtk_timeout_add
		(CFG_UPDATE_INTERVAL * 1000,
		 (GtkFunction)wireless_applet_timeout_handler,
		 applet);
  
	return GTK_WIDGET (applet);
}

static gboolean
wireless_applet_fill (WirelessApplet *applet)
{
	gnome_window_icon_set_default_from_file
		(ICONDIR"/wireless-applet/wireless-applet.png");

	glade_gnome_init ();
	glade_file = gnome_program_locate_file
		(NULL, GNOME_FILE_DOMAIN_DATADIR,
		 "wireless-applet/wireless-applet.glade", FALSE, NULL);

	wireless_applet_new (applet);
	gtk_widget_show (GTK_WIDGET (applet));

	return TRUE;
}

static gboolean
wireless_applet_factory (WirelessApplet *applet,
		const gchar          *iid,
		gpointer              data)
{
	gboolean retval = FALSE;

	if (!strcmp (iid, "OAFIID:GNOME_Panel_WirelessApplet"))
		retval = wireless_applet_fill (applet);

	return retval;
}

PANEL_APPLET_BONOBO_FACTORY ("OAFIID:GNOME_Panel_WirelessApplet_Factory",
		wireless_applet_get_type (),
		"Wireless applet",
		"0",
		(PanelAppletFactoryCallback) wireless_applet_factory,
		NULL)

