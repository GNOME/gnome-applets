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

#include <egg-screen-help.h>

#define CFG_DEVICE "eth0"
#define CFG_UPDATE_INTERVAL 2

typedef enum {
	PIX_BROKEN,
	PIX_NO_LINK,
	PIX_SIGNAL_1,
	PIX_SIGNAL_2,
	PIX_SIGNAL_3,
	PIX_SIGNAL_4,
	PIX_NUMBER
} PixmapState;

static char * pixmap_names[] = {
	"broken-0.png",
	"no-link-0.png",
	"signal-1-40.png",
	"signal-41-60.png",
	"signal-61-80.png",
	"signal-81-100.png",
};

typedef struct {
	PanelApplet base;
	gchar *device;
	gboolean show_percent;

	GList *devices;

	/* contains pointers into the images GList.
	 * 0-100 are for link */
	GdkPixbuf*pixmaps[PIX_NUMBER];
	/* pointer to the current used file name */
	GdkPixbuf *current_pixbuf;

	GtkWidget *pct_label;
	GtkWidget *pixmap;
	GtkWidget *box;
	GtkWidget *about_dialog;
	guint timeout_handler_id;
	FILE *file;
	GtkTooltips *tips;
	GtkWidget *prefs;
} WirelessApplet;

static GladeXML *xml = NULL;
static gchar* glade_file=NULL;

static void show_error_dialog (gchar*,...);
static void show_warning_dialog (gchar*,...);
static int wireless_applet_timeout_handler (WirelessApplet *applet);
static void wireless_applet_properties_dialog (BonoboUIComponent *uic,
		WirelessApplet *applet);
static void wireless_applet_help_cb (BonoboUIComponent *uic,
		WirelessApplet *applet);
static void wireless_applet_about_cb (BonoboUIComponent *uic,
		WirelessApplet *applet);
static void prefs_response_cb (GtkDialog *dialog, gint response, gpointer data);

static const BonoboUIVerb wireless_menu_verbs [] = {
	BONOBO_UI_UNSAFE_VERB ("WirelessProperties",
			wireless_applet_properties_dialog),
	BONOBO_UI_UNSAFE_VERB ("WirelessHelp",
			wireless_applet_help_cb),
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

/* FIXME: The icon used by this applet when there's no signal is impossible
 * to localise as it says N/A in the icon itself. Need to swap the icon
 * with something more l10n friendly. Also, localising the label makes it
 * necessary to ditch g_strcasecmp() in favor of something else.
 */

static void 
wireless_applet_draw (WirelessApplet *applet, int percent)
{
	const char *label_text;
	char *tmp;
	PixmapState state;

	/* Update the percentage */
	if (percent > 0) {
		tmp = g_strdup_printf ( _("Link Strength: %2.0d%%"), percent);
	} else {
		tmp = g_strdup_printf (_("N/A"));
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
	percent = CLAMP (percent, -1, 100);

	if (percent < 0)
		state = PIX_BROKEN;
	else if (percent == 0)
		state = PIX_NO_LINK;
	else if (percent <= 40)
		state = PIX_SIGNAL_1;
	else if (percent <= 60)
		state = PIX_SIGNAL_2;
	else if (percent <= 80)
		state = PIX_SIGNAL_3;
	else
		state = PIX_SIGNAL_4;

	if (applet->pixmaps[state] != applet->current_pixbuf)
	{
		applet->current_pixbuf = (GdkPixbuf *)applet->pixmaps[state];
		gtk_image_set_from_pixbuf (GTK_IMAGE (applet->pixmap),
				applet->current_pixbuf);
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
		if (link < 1) {
			percent = 0;
		} else {
			percent = (int)rint ((log (link) / log (92)) * 100.0);
			percent = CLAMP (percent, 0, 100);
		}
	}

	wireless_applet_draw (applet, percent);
}

static void
wireless_applet_load_theme (WirelessApplet *applet) {
	char *pixmapdir;
	char *pixmapname;
	int i;

	pixmapdir = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_PIXMAP,
			"wireless-applet/", FALSE, NULL);


	for (i = 0; i < PIX_NUMBER; i++)
	{
		pixmapname = g_build_filename (G_DIR_SEPARATOR_S,
				pixmapdir, pixmap_names[i], NULL);
		applet->pixmaps[i] = gdk_pixbuf_new_from_file (pixmapname, NULL);
		g_free (pixmapname);
	}

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

/* check stats, modify the state attribute */
static void
wireless_applet_read_device_state (WirelessApplet *applet)
{
	long int level, noise;
	double link;
	char device[256];
	char line[256];
	gboolean found = FALSE;

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
			if (g_ascii_strcasecmp (applet->device, device)==0) {
				link = strtod (ptr, &ptr);
				ptr++;

				level = strtol (ptr, &ptr, 10);
				ptr++;

				noise = strtol (ptr, &ptr, 10);
				ptr++;

				wireless_applet_update_state (applet, device, link, level, noise);
				found = TRUE;
			}
		}
	} while (1);

	if (g_list_length (applet->devices)==1) {
		wireless_applet_set_device (applet,
				(char*)applet->devices->data);
	} else if (found == FALSE) {
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
			0, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
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
			0, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK,
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
		return;
	}

	applet->show_percent = panel_applet_gconf_get_bool
		(PANEL_APPLET (applet), "percent", NULL);
}

static void
wireless_applet_save_properties (WirelessApplet *applet)
{
	if (applet->device)
		panel_applet_gconf_set_string (PANEL_APPLET (applet),
			"device", applet->device, NULL);
	panel_applet_gconf_set_bool (PANEL_APPLET (applet),
			"percent", applet->show_percent, NULL);
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
	gtk_window_set_resizable (GTK_WINDOW (applet->prefs), FALSE);

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
			G_CALLBACK (prefs_response_cb),
			NULL);
	g_signal_connect (GTK_OBJECT (applet->prefs),
			"destroy",
			GTK_SIGNAL_FUNC (gtk_widget_destroy),
			NULL);

	g_object_add_weak_pointer (G_OBJECT (applet->prefs),
			(void**)&(applet->prefs));
	gtk_window_set_screen (GTK_WINDOW (applet->prefs),
			       gtk_widget_get_screen (GTK_WIDGET (applet)));
	gtk_widget_show_all (applet->prefs);
}

static void
wireless_applet_help_cb (BonoboUIComponent *uic, WirelessApplet *applet)
{
	GError *error = NULL;
	egg_help_display_on_screen ("wireless", NULL,
				   gtk_widget_get_screen (GTK_WIDGET (
						applet)), &error);
	if (error) {
		GtkWidget *dialog =
		gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL,
					GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
					_("There was an error displaying help: %s"),
					error->message);
		g_signal_connect (G_OBJECT (dialog), "response",
				  G_CALLBACK (gtk_widget_destroy), NULL);
		gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
		gtk_window_set_screen (GTK_WINDOW (dialog),
				       gtk_widget_get_screen (GTK_WIDGET (
							      applet)));
		gtk_widget_show (dialog);
		g_error_free (error);
	}
}

static void
wireless_applet_about_cb (BonoboUIComponent *uic, WirelessApplet *applet)
{
	GdkPixbuf *pixbuf;
	char *file;

	const gchar *authors[] = {
		"Eskil Heyn Olsen <eskil@eskil.org>",
		"Bastien Nocera <hadess@hadess.net> (Gnome2 port)",
		NULL
	};

	const gchar *documenters[] = { 
                 "Sun GNOME Documentation Team <gdocteam@sun.com>",
 	        NULL 
         };	

	const gchar *translator_credits = _("translator_credits");

	if (applet->about_dialog != NULL) {
		gtk_window_set_screen (GTK_WINDOW (applet->about_dialog),
				       gtk_widget_get_screen (GTK_WIDGET (&applet->base)));
		
		gtk_window_present (GTK_WINDOW (applet->about_dialog));
		return;
	}

	file = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_PIXMAP,
			"wireless-applet/wireless-applet.png", FALSE, NULL);
	pixbuf = gdk_pixbuf_new_from_file (file, NULL);
	g_free (file);

	applet->about_dialog = gnome_about_new (
			_("Wireless Link Monitor"),
			VERSION,
			_("(C) 2001, 2002 Free Software Foundation "),
			_("This utility shows the status of a wireless link."),
			authors,
			documenters,
			strcmp (translator_credits, "translator_credits") != 0 ? translator_credits : NULL,
			pixbuf);

	g_object_unref (pixbuf);

	gtk_window_set_screen (GTK_WINDOW (applet->about_dialog),
			       gtk_widget_get_screen (GTK_WIDGET (applet)));

	g_signal_connect (applet->about_dialog, "destroy",
			  G_CALLBACK (gtk_widget_destroyed),
			  &applet->about_dialog);
 
	gtk_widget_show (applet->about_dialog);

	return;
}

static void
prefs_response_cb (GtkDialog *dialog, gint response, gpointer data)
{
	GError *error = NULL;
	if (response == GTK_RESPONSE_HELP) {
		egg_help_display_on_screen ("wireless", "wireless-prefs",
					    gtk_widget_get_screen (GTK_WIDGET (
					    dialog)), &error);
		if (error) {
			GtkWidget *dlg =
			gtk_message_dialog_new (GTK_WINDOW (dialog),
						GTK_DIALOG_DESTROY_WITH_PARENT
						|| GTK_DIALOG_MODAL,
						GTK_MESSAGE_ERROR,
						GTK_BUTTONS_CLOSE,
						_("There was an error displaying help: %s"),
						error->message);
			g_signal_connect (G_OBJECT (dlg), "response",
					  G_CALLBACK (gtk_widget_destroy),
					  NULL);
			gtk_window_set_resizable (GTK_WINDOW (dlg), FALSE);
			gtk_window_set_screen (GTK_WINDOW (dlg),
					       gtk_widget_get_screen (
							GTK_WIDGET (dialog)));
			gtk_widget_show (dlg);
			g_error_free (error);
		}
	}
	else
		gtk_widget_hide (GTK_WIDGET (dialog));
}

static void
wireless_applet_destroy (WirelessApplet *applet, gpointer horse)
{
	int i;

	g_free (applet->device);

	g_list_foreach (applet->devices, (GFunc)g_free, NULL);
	g_list_free (applet->devices);

	if (applet->timeout_handler_id > 0) {
		gtk_timeout_remove (applet->timeout_handler_id);
		applet->timeout_handler_id = 0;
	}

	for (i = 0; i < PIX_NUMBER; i++)
		g_object_unref (applet->pixmaps[i]);

	if (applet->about_dialog) {
		gtk_widget_destroy (applet->about_dialog);
		applet->about_dialog = NULL;
	}

	if (applet->prefs) {
		gtk_widget_destroy (applet->prefs);
		applet->prefs = NULL;
	}

	if (applet->file)
		fclose (applet->file);
	if (applet->tips)
		g_object_unref (applet->tips);
}

static void
setup_widgets (WirelessApplet *applet)
{
	GtkRequisition req;
	gint total_size = 0;
	gboolean horizontal = FALSE;
	gint panel_size;
	
	panel_size = panel_applet_get_size (PANEL_APPLET (applet));
	
	switch (panel_applet_get_orient(PANEL_APPLET (applet))) {
	case PANEL_APPLET_ORIENT_LEFT:
	case PANEL_APPLET_ORIENT_RIGHT:
		horizontal = FALSE;
		break;
	case PANEL_APPLET_ORIENT_UP:
	case PANEL_APPLET_ORIENT_DOWN:
		horizontal = TRUE;
		break;
	}

	/* construct pixmap widget */
	applet->pixmap = gtk_image_new ();
	gtk_image_set_from_pixbuf (GTK_IMAGE (applet->pixmap), applet->pixmaps[PIX_BROKEN]);
	gtk_widget_size_request (applet->pixmap, &req);
	gtk_widget_show (applet->pixmap);

	if (horizontal)
		total_size += req.height;
	else
		total_size += req.width;

	/* construct pct widget */
	applet->pct_label = gtk_label_new ("N/A");

	if (applet->show_percent == TRUE) {
		gtk_widget_show (applet->pct_label);
		gtk_widget_size_request (applet->pct_label, &req);
		if (horizontal)
			total_size += req.height;
		else
			total_size += req.width;
	}

	/* pack */
	if (applet->box)
		gtk_widget_destroy (applet->box);

	if (horizontal && (total_size <= panel_size))
		applet->box = gtk_vbox_new (FALSE, 0);
	else if (horizontal && (total_size > panel_size))
		applet->box = gtk_hbox_new (FALSE, 0);
	else if (!horizontal && (total_size <= panel_size))
		applet->box = gtk_hbox_new (FALSE, 0);
	else 
		applet->box = gtk_vbox_new (FALSE, 0);
	
	gtk_box_pack_start (GTK_BOX (applet->box), applet->pixmap, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (applet->box), applet->pct_label, TRUE, TRUE, 0);
	/* note, I don't use show_all, because this way the percent label is
	 * only realised if it's enabled */
	gtk_widget_show (applet->box);
	gtk_container_add (GTK_CONTAINER (applet), applet->box);

	applet->current_pixbuf = NULL;
	applet->about_dialog = NULL;
}

static void change_size_cb(PanelApplet *pa, gint s, WirelessApplet *applet)
{
	setup_widgets (applet);
	wireless_applet_timeout_handler (applet);
}

static void change_orient_cb(PanelApplet *pa, gint s, WirelessApplet *applet)
{
	setup_widgets (applet);
	wireless_applet_timeout_handler (applet);
}

static void change_background_cb(PanelApplet *a, PanelAppletBackgroundType type,
				 GdkColor *color, GdkPixmap *pixmap,
				 WirelessApplet *applet)
{
	GtkRcStyle *rc_style = gtk_rc_style_new ();

	switch (type) {
		case PANEL_PIXMAP_BACKGROUND:
			gtk_widget_modify_style (GTK_WIDGET (applet), rc_style);
			break;

		case PANEL_COLOR_BACKGROUND:
			gtk_widget_modify_bg (GTK_WIDGET (applet), GTK_STATE_NORMAL, color);
			break;

		case PANEL_NO_BACKGROUND:
			gtk_widget_modify_style (GTK_WIDGET (applet), rc_style);
			break;

		default:
			gtk_widget_modify_style (GTK_WIDGET (applet), rc_style);
			break;
	}

	gtk_rc_style_unref (rc_style);
}

static GtkWidget *
wireless_applet_new (WirelessApplet *applet)
{
	AtkObject *atk_obj;

	panel_applet_set_flags (PANEL_APPLET (applet), PANEL_APPLET_EXPAND_MINOR);

	/* this ensures that properties are loaded */
	wireless_applet_load_properties (applet);
	wireless_applet_load_theme (applet);

	setup_widgets (applet);

	applet->tips = gtk_tooltips_new ();
	g_object_ref (applet->tips);
	gtk_object_sink (GTK_OBJECT (applet->tips));
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

	if (panel_applet_get_locked_down (PANEL_APPLET (applet))) {
		BonoboUIComponent *popup_component;

		popup_component = panel_applet_get_popup_component (PANEL_APPLET (applet));

		bonobo_ui_component_set_prop (popup_component,
					      "/commands/WirelessProperties",
					      "hidden", "1",
					      NULL);
	}

	start_file_read (applet);
	wireless_applet_timeout_handler (applet);

	gtk_tooltips_set_tip (applet->tips,
			      GTK_WIDGET (applet),
			      _("No Wireless Devices"),
			      NULL);

	applet->timeout_handler_id = gtk_timeout_add
		(CFG_UPDATE_INTERVAL * 1000,
		 (GtkFunction)wireless_applet_timeout_handler,
		 applet);

	atk_obj = gtk_widget_get_accessible (GTK_WIDGET (applet));

	if (GTK_IS_ACCESSIBLE (atk_obj)) {
		atk_object_set_name (atk_obj, _("Wireless Link Monitor"));
		atk_object_set_description (atk_obj, _("This utility shows the status of a wireless link"));
	}
		 
	g_signal_connect (G_OBJECT (applet), "change_size",
			  G_CALLBACK (change_size_cb), applet);
	g_signal_connect (G_OBJECT (applet), "change_orient",
			  G_CALLBACK (change_orient_cb), applet);
 	g_signal_connect (G_OBJECT (applet), "change_background",
			  G_CALLBACK (change_background_cb), applet);

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
		"wireless",
		"0",
		(PanelAppletFactoryCallback) wireless_applet_factory,
		NULL)

