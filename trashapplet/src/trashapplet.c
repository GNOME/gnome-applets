/* -*- mode: c; c-basic-offset: 8 -*-
 * trashapplet.c
 *
 * Copyright (c) 2004  Michiel Sikkes <michiel@eyesopened.nl>,
 *               2004  Emmanuele Bassi <ebassi@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>

#include <gtk/gtk.h>
#include <gconf/gconf-client.h>
#include <libgnome/gnome-help.h>
#include <glade/glade.h>

#include "trashapplet.h"
#include "trash-monitor.h"
#include "xstuff.h"

static GConfClient *client = NULL;

G_DEFINE_TYPE(TrashApplet, trash_applet, PANEL_TYPE_APPLET)

static void     trash_applet_destroy            (GtkObject         *object);

static void     trash_applet_size_allocate      (GtkWidget         *widget,
						 GdkRectangle     *allocation);
static gboolean trash_applet_button_release     (GtkWidget         *widget,
						 GdkEventButton    *event);
static gboolean trash_applet_key_press          (GtkWidget         *widget,
						 GdkEventKey       *event);
static void     trash_applet_drag_leave         (GtkWidget         *widget,
						 GdkDragContext    *context,
						 guint              time_);
static gboolean trash_applet_drag_motion        (GtkWidget         *widget,
						 GdkDragContext    *context,
						 gint               x,
						 gint               y,
						 guint              time_);
static void     trash_applet_drag_data_received (GtkWidget         *widget,
						 GdkDragContext    *context,
						 gint               x,
						 gint               y,
						 GtkSelectionData  *selectiondata,
						 guint              info,
						 guint              time_);

static void     trash_applet_change_orient      (PanelApplet     *panel_applet,
						 PanelAppletOrient  orient);
static void     trash_applet_change_background  (PanelApplet     *panel_applet,
						 PanelAppletBackgroundType type,
						 GdkColor        *colour,
						 GdkPixmap       *pixmap);

static void trash_applet_do_empty    (BonoboUIComponent *component,
				      TrashApplet       *applet,
				      const gchar       *cname);
static void trash_applet_show_about  (BonoboUIComponent *component,
				      TrashApplet       *applet,
				      const gchar       *cname);
static void trash_applet_open_folder (BonoboUIComponent *component,
				      TrashApplet       *applet,
				      const gchar       *cname);
static void trash_applet_show_help   (BonoboUIComponent *component,
				      TrashApplet       *applet,
				      const gchar       *cname);

static const GtkTargetEntry drop_types[] = {
	{ "text/uri-list", 0, 0 }
};
static const gint n_drop_types = G_N_ELEMENTS (drop_types);
static const BonoboUIVerb trash_applet_menu_verbs [] = {
	BONOBO_UI_UNSAFE_VERB ("EmptyTrash", trash_applet_do_empty),
	BONOBO_UI_UNSAFE_VERB ("OpenTrash", trash_applet_open_folder),
	BONOBO_UI_UNSAFE_VERB ("AboutTrash", trash_applet_show_about),
	BONOBO_UI_UNSAFE_VERB ("HelpTrash", trash_applet_show_help),
	BONOBO_UI_VERB_END
};

static void trash_applet_queue_update (TrashApplet  *applet);
static void item_count_changed        (TrashMonitor *monitor,
				       TrashApplet  *applet);

static void
trash_applet_class_init (TrashAppletClass *class)
{
	GTK_OBJECT_CLASS (class)->destroy = trash_applet_destroy;
	GTK_WIDGET_CLASS (class)->size_allocate = trash_applet_size_allocate;
	GTK_WIDGET_CLASS (class)->button_release_event = trash_applet_button_release;
	GTK_WIDGET_CLASS (class)->key_press_event = trash_applet_key_press;
	GTK_WIDGET_CLASS (class)->drag_leave = trash_applet_drag_leave;
	GTK_WIDGET_CLASS (class)->drag_motion = trash_applet_drag_motion;
	GTK_WIDGET_CLASS (class)->drag_data_received = trash_applet_drag_data_received;
	PANEL_APPLET_CLASS (class)->change_orient = trash_applet_change_orient;
	PANEL_APPLET_CLASS (class)->change_background = trash_applet_change_background;
}

static void
trash_applet_init (TrashApplet *applet)
{
	GnomeVFSResult res;
	GnomeVFSURI *trash_uri;

	gtk_window_set_default_icon_name (TRASH_ICON_FULL);

	panel_applet_set_flags (PANEL_APPLET (applet),
				PANEL_APPLET_EXPAND_MINOR);
	/* get the default gconf client */
	if (!client)
		client = gconf_client_get_default ();

	applet->size = 0;
	applet->new_size = 0;

	switch (panel_applet_get_orient (PANEL_APPLET (applet))) {
	case PANEL_APPLET_ORIENT_LEFT:
	case PANEL_APPLET_ORIENT_RIGHT:
		applet->orient = GTK_ORIENTATION_VERTICAL;
		break;
	case PANEL_APPLET_ORIENT_UP:
	case PANEL_APPLET_ORIENT_DOWN:
		applet->orient = GTK_ORIENTATION_HORIZONTAL;
		break;
	}
	applet->tooltips = gtk_tooltips_new ();
	g_object_ref (applet->tooltips);
	gtk_object_sink (GTK_OBJECT (applet->tooltips));

	applet->image = gtk_image_new ();
	gtk_container_add (GTK_CONTAINER (applet), applet->image);
	gtk_widget_show (applet->image);
	applet->icon_state = TRASH_STATE_UNKNOWN;

	/* create local trash directory if needed */
	res = gnome_vfs_find_directory (NULL,
					GNOME_VFS_DIRECTORY_KIND_TRASH,
					&trash_uri,
					TRUE,
					TRUE,
					0700);
	if (trash_uri)
		gnome_vfs_uri_unref (trash_uri);
	if (res != GNOME_VFS_OK) {
		g_warning (_("Unable to find the Trash directory: %s"),
				gnome_vfs_result_to_string (res));
	}

	/* set up trash monitor */
	applet->monitor = trash_monitor_get ();
	applet->monitor_signal_id =
		g_signal_connect (applet->monitor, "item_count_changed",
				  G_CALLBACK (item_count_changed), applet);

	/* initial state */
	applet->item_count = -1;
	applet->is_empty = TRUE;
	applet->drag_hover = FALSE;

	/* set up DnD target */
        gtk_drag_dest_set (GTK_WIDGET (applet),
                           GTK_DEST_DEFAULT_ALL,
                           drop_types, n_drop_types,
                           GDK_ACTION_MOVE);
}

static void
trash_applet_destroy (GtkObject *object)
{
	TrashApplet *applet = TRASH_APPLET (object);
	
	applet->image = NULL;
	if (applet->tooltips)
		g_object_unref (applet->tooltips);
	applet->tooltips = NULL;

	if (applet->monitor_signal_id)
		g_signal_handler_disconnect (applet->monitor,
					     applet->monitor_signal_id);
	applet->monitor_signal_id = 0;

	if (applet->update_id)
		g_source_remove (applet->update_id);
	applet->update_id = 0;

	(* GTK_OBJECT_CLASS (trash_applet_parent_class)->destroy) (object);
}

static void
trash_applet_size_allocate (GtkWidget    *widget,
			    GdkRectangle *allocation)
{
	TrashApplet *applet = TRASH_APPLET (widget);
	gint new_size;

	if (applet->orient == GTK_ORIENTATION_HORIZONTAL)
		new_size = allocation->height;
	else
		new_size = allocation->width;

	if (new_size != applet->size) {
		applet->new_size = new_size;
		trash_applet_queue_update (applet);
	}

	(* GTK_WIDGET_CLASS (trash_applet_parent_class)->size_allocate) (widget, allocation);
}

static void
trash_applet_change_orient (PanelApplet       *panel_applet,
			    PanelAppletOrient  orient)
{
	TrashApplet *applet = TRASH_APPLET (panel_applet);
	gint new_size;

	switch (orient) {
	case PANEL_APPLET_ORIENT_LEFT:
	case PANEL_APPLET_ORIENT_RIGHT:
		applet->orient = GTK_ORIENTATION_VERTICAL;
		new_size = GTK_WIDGET (applet)->allocation.width;
		break;
	case PANEL_APPLET_ORIENT_UP:
	case PANEL_APPLET_ORIENT_DOWN:
		applet->orient = GTK_ORIENTATION_HORIZONTAL;
		new_size = GTK_WIDGET (applet)->allocation.height;
		break;
	}
	if (new_size != applet->size) {
		applet->new_size = new_size;
		trash_applet_queue_update (applet);
	}

	if (PANEL_APPLET_CLASS (trash_applet_parent_class)->change_orient)
		(* PANEL_APPLET_CLASS (trash_applet_parent_class)->change_orient) (panel_applet, orient);
}

static void
trash_applet_change_background (PanelApplet               *panel_applet,
				PanelAppletBackgroundType  type,
				GdkColor                  *colour,
				GdkPixmap                 *pixmap)
{
	TrashApplet *applet = TRASH_APPLET (panel_applet);
	GtkRcStyle *rc_style;
	GtkStyle *style;

	/* reset style */
	gtk_widget_set_style (GTK_WIDGET (applet), NULL);
	rc_style = gtk_rc_style_new ();
	gtk_widget_modify_style (GTK_WIDGET (applet), rc_style);
	gtk_rc_style_unref (rc_style);

	switch (type) {
	case PANEL_NO_BACKGROUND:
		break;
	case PANEL_COLOR_BACKGROUND:
		gtk_widget_modify_bg (GTK_WIDGET (applet),
				      GTK_STATE_NORMAL, colour);
		break;
	case PANEL_PIXMAP_BACKGROUND:
		style = gtk_style_copy (GTK_WIDGET (applet)->style);
		if (style->bg_pixmap[GTK_STATE_NORMAL])
			g_object_unref (style->bg_pixmap[GTK_STATE_NORMAL]);
		style->bg_pixmap[GTK_STATE_NORMAL] = g_object_ref (pixmap);
		gtk_widget_set_style (GTK_WIDGET (applet), style);
		g_object_unref (style);
		break;
	}
}

#define PANEL_ENABLE_ANIMATIONS "/apps/panel/global/enable_animations"
static gboolean
trash_applet_button_release (GtkWidget      *widget,
			     GdkEventButton *event)
{
	TrashApplet *applet = TRASH_APPLET (widget);

	if (event->button == 1) {
		if (gconf_client_get_bool (client, PANEL_ENABLE_ANIMATIONS, NULL))
			xstuff_zoom_animate (widget, NULL);
		trash_applet_open_folder (NULL, applet, NULL);
		return TRUE;
	}
	if (GTK_WIDGET_CLASS (trash_applet_parent_class)->button_release_event)
		return (* GTK_WIDGET_CLASS (trash_applet_parent_class)->button_release_event) (widget, event);
	else
		return FALSE;
}
static gboolean
trash_applet_key_press (GtkWidget   *widget,
			GdkEventKey *event)
{
	TrashApplet *applet = TRASH_APPLET (widget);

	switch (event->keyval) {
	case GDK_KP_Enter:
	case GDK_ISO_Enter:
	case GDK_3270_Enter:
	case GDK_Return:
	case GDK_space:
	case GDK_KP_Space:
		trash_applet_open_folder(NULL, applet, NULL);
		return TRUE;
	default:
		break;
	}
	if (GTK_WIDGET_CLASS (trash_applet_parent_class)->key_press_event)
		return (* GTK_WIDGET_CLASS (trash_applet_parent_class)->key_press_event) (widget, event);
	else
		return FALSE;
}

static void
trash_applet_drag_leave (GtkWidget      *widget,
			 GdkDragContext *context,
			 guint           time_)
{
	TrashApplet *applet = TRASH_APPLET (widget);

	if (applet->drag_hover) {
		applet->drag_hover = FALSE;
		trash_applet_queue_update (applet);
	}
}

static gboolean
trash_applet_drag_motion (GtkWidget      *widget,
			  GdkDragContext *context,
			  gint            x,
			  gint            y,
			  guint           time_)
{
	TrashApplet *applet = TRASH_APPLET (widget);

	if (!applet->drag_hover) {
		applet->drag_hover = TRUE;
		trash_applet_queue_update (applet);
	}
	gdk_drag_status (context, GDK_ACTION_MOVE, time_);

	return TRUE;
}

static void
item_count_changed (TrashMonitor *monitor,
		    TrashApplet  *applet)
{
	trash_applet_queue_update (applet);
}

static gboolean
trash_applet_update (gpointer user_data)
{
	TrashApplet *applet = TRASH_APPLET (user_data);
	gint new_item_count;
	BonoboUIComponent *popup_component;
	char *tip_text;
	TrashState new_state = TRASH_STATE_UNKNOWN;
	const char *new_icon;
	GdkScreen *screen;
	GtkIconTheme *icon_theme;
	GdkPixbuf *pixbuf, *scaled;

	applet->update_id = 0;

	new_item_count = trash_monitor_get_item_count (applet->monitor);
	if (new_item_count != applet->item_count) {
		applet->item_count = new_item_count;
		applet->is_empty = (applet->item_count == 0);

		/* set sensitivity on the "empty trash" context menu item */
		popup_component = panel_applet_get_popup_component (PANEL_APPLET (applet));
		bonobo_ui_component_set_prop (popup_component,
					      "/commands/EmptyTrash",
					      "sensitive",
					      applet->is_empty ? "0" : "1",
					      NULL);

		switch (applet->item_count) {
		case 0:
			tip_text = g_strdup (_("No Items in Trash"));
			break;
		default:
			tip_text = g_strdup_printf (ngettext (
						"%d Item in Trash",
						"%d Items in Trash",
						applet->item_count), 
						    applet->item_count);
		}
		gtk_tooltips_set_tip (applet->tooltips, GTK_WIDGET (applet),
				      tip_text, NULL);
		g_free (tip_text);
	}

	/* work out what icon to use */
	if (applet->drag_hover) {
		new_state = TRASH_STATE_ACCEPT;
		new_icon = TRASH_ICON_EMPTY_ACCEPT;
	} else if (applet->is_empty) {
		new_state = TRASH_STATE_EMPTY;
		new_icon = TRASH_ICON_EMPTY;
	} else {
		new_state = TRASH_STATE_FULL;
		new_icon = TRASH_ICON_FULL;
	}

	if (applet->image && applet->new_size > 10 &&
            (applet->icon_state != new_state ||
	     applet->new_size != applet->size)) {
                gint size;
		applet->size = applet->new_size;
		size = applet->size - 4;
		screen = gtk_widget_get_screen (GTK_WIDGET (applet));
		icon_theme = gtk_icon_theme_get_for_screen (screen);
		/* not all themes have the "accept" variant */
		if (new_state == TRASH_STATE_ACCEPT) {
			if (!gtk_icon_theme_has_icon (icon_theme, new_icon))
				new_icon = applet->is_empty
					? TRASH_ICON_EMPTY
					: TRASH_ICON_FULL;
		}
		pixbuf = gtk_icon_theme_load_icon (icon_theme, new_icon, 
						   size, 0, NULL);
		if (!pixbuf)
		    return FALSE;

                if (gdk_pixbuf_get_width (pixbuf) != size ||
                    gdk_pixbuf_get_height (pixbuf) != size) {
			scaled = gdk_pixbuf_scale_simple (pixbuf, size, size, GDK_INTERP_BILINEAR);
			g_object_unref (pixbuf);
		 	pixbuf = scaled;
		}

		gtk_image_set_from_pixbuf (GTK_IMAGE (applet->image), pixbuf);
		g_object_unref (pixbuf);
	}

	return FALSE;
}

static void
trash_applet_queue_update (TrashApplet *applet)
{
	if (applet->update_id == 0) {
		applet->update_id = g_idle_add (trash_applet_update, applet);
	}
}

/* TODO - Must HIGgify this dialog */
static void
error_dialog (TrashApplet *applet, const gchar *error, ...)
{
	va_list args;
	gchar *error_string;
	GtkWidget *dialog;

	g_return_if_fail (error != NULL);

	va_start (args, error);
	error_string = g_strdup_vprintf (error, args);
	va_end (args);

	dialog = gtk_message_dialog_new (NULL,
			GTK_DIALOG_MODAL,
			GTK_MESSAGE_ERROR,
			GTK_BUTTONS_OK,
			"%s",
			error_string);

	g_signal_connect (G_OBJECT (dialog), "response",
			G_CALLBACK (gtk_widget_destroy),
			NULL);

	gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
	gtk_window_set_screen (GTK_WINDOW(dialog),
			       gtk_widget_get_screen (GTK_WIDGET (applet)));
	gtk_widget_show (dialog);

	g_free (error_string);
}

static gint
update_transfer_callback (GnomeVFSAsyncHandle *handle,
                          GnomeVFSXferProgressInfo *progress_info,
                          gpointer user_data)
{
	TrashApplet *applet = TRASH_APPLET (user_data);
	GladeXML *xml = applet->xml;

	if (progress_info->files_total != 0) {
		GtkWidget *progressbar;
		gdouble fraction;
		gchar *progress_message;

		progressbar = glade_xml_get_widget (xml, "progressbar");

		fraction = (gulong) progress_info->file_index / (gulong) progress_info->files_total;
		gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (progressbar), fraction);

		progress_message = g_strdup_printf (_("Removing item %d of %d"),
						    progress_info->file_index,
						    progress_info->files_total);
		gtk_progress_bar_set_text (GTK_PROGRESS_BAR (progressbar),
					   progress_message);

		g_free (progress_message);
	}

	if (progress_info->source_name != NULL) {
		GtkWidget *location_label;
		GtkWidget *file_label;
		GnomeVFSURI *uri;
		gchar *short_name;
		gchar *from_location;
		gchar *file;

		location_label = glade_xml_get_widget (xml, "location_label");
		file_label = glade_xml_get_widget (xml, "file_label");

		uri = gnome_vfs_uri_new (progress_info->source_name);

		from_location = gnome_vfs_uri_extract_dirname (uri);

		short_name = gnome_vfs_uri_extract_short_name (uri);
		file = g_strdup_printf ("<i>%s %s</i>",
                                        _("Removing:"),
                                        short_name);
		g_free (short_name);

		gtk_label_set_markup (GTK_LABEL (location_label), from_location);
		gtk_label_set_markup (GTK_LABEL (file_label), file);

		g_free (from_location);
		g_free (file);
		gnome_vfs_uri_unref (uri);
       }

       return 1;
}

/* this function is based on the one with the same name in
   libnautilus-private/nautilus-file-operations.c */
#define NAUTILUS_PREFERENCES_CONFIRM_TRASH    "/apps/nautilus/preferences/confirm_trash"
static gboolean
confirm_empty_trash (GtkWidget *parent_view)
{
	GtkWidget *dialog;
	GtkWidget *button;
	GdkScreen *screen;
	int response;

	/* Just Say Yes if the preference says not to confirm. */
	/* Note: use directly gconf instead eel as in original nautilus function*/
	if (!gconf_client_get_bool (client,
				    NAUTILUS_PREFERENCES_CONFIRM_TRASH,
				    NULL)) {
		return TRUE;
	}
	
	screen = gtk_widget_get_screen (parent_view);

	dialog = gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL,
					 GTK_MESSAGE_WARNING,
					 GTK_BUTTONS_NONE,
					 _("Empty all of the items from "
					   "the trash?"));
	gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
						  _("If you choose to empty "
						    "the trash, all items in "
						    "it will be permanently "
						    "lost. Please note that "
						    "you can also delete them "
						    "separately."));

	gtk_window_set_screen (GTK_WINDOW (dialog), screen);
	atk_object_set_role (gtk_widget_get_accessible (dialog), ATK_ROLE_ALERT);
	gtk_window_set_wmclass (GTK_WINDOW (dialog), "empty_trash",
				"Nautilus");

	/* Make transient for the window group */
        gtk_widget_realize (dialog);
	gdk_window_set_transient_for (GTK_WIDGET (dialog)->window,
				      gdk_screen_get_root_window (screen));

	gtk_dialog_add_button (GTK_DIALOG (dialog), GTK_STOCK_CANCEL,
			       GTK_RESPONSE_CANCEL);

	button = gtk_button_new_with_mnemonic (_("_Empty Trash"));
	gtk_widget_show (button);
	GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);

	gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button,
				      GTK_RESPONSE_YES);

	gtk_dialog_set_default_response (GTK_DIALOG (dialog),
					 GTK_RESPONSE_YES);

	response = gtk_dialog_run (GTK_DIALOG (dialog));

	gtk_object_destroy (GTK_OBJECT (dialog));

	return response == GTK_RESPONSE_YES;
}

static void
on_empty_trash_cancel (GtkWidget *widget, GnomeVFSAsyncHandle **handle)
{
	if (handle != NULL) {
		gnome_vfs_async_cancel ((GnomeVFSAsyncHandle *) handle);
	}
		
	gtk_widget_hide (widget);
}

static void
trash_applet_do_empty (BonoboUIComponent *component,
		       TrashApplet       *applet,
		       const gchar       *cname)
{
	GtkWidget *dialog;

	GnomeVFSAsyncHandle *hnd;

	g_return_if_fail (TRASH_IS_APPLET (applet));

	if (applet->is_empty)
		return;

	if (!confirm_empty_trash (GTK_WIDGET (applet)))
		return;

	if (!applet->xml)
	  applet->xml = glade_xml_new (GNOME_GLADEDIR "/trashapplet.glade", NULL, NULL);

        dialog = glade_xml_get_widget(applet->xml, "empty_trash");

	g_signal_connect(dialog, "response", G_CALLBACK (on_empty_trash_cancel), &hnd);

	gtk_widget_show_all(dialog);

	trash_monitor_empty_trash (applet->monitor,
				   &hnd, update_transfer_callback, applet);

	gtk_widget_hide(dialog);

}


static void
trash_applet_open_folder (BonoboUIComponent *component,
			  TrashApplet       *applet,
			  const gchar       *cname)
{
	GdkScreen *screen;

	/* Open the "trash:" URI with gnome-open */
	gchar *argv[] = { "gnome-open", "trash:", NULL };
	GError *err = NULL;
	gboolean res;	

        g_return_if_fail (TRASH_IS_APPLET (applet));

	screen = gtk_widget_get_screen (GTK_WIDGET (applet));
	res = gdk_spawn_on_screen (screen, NULL,
			           argv, NULL,
			           G_SPAWN_SEARCH_PATH,
			           NULL, NULL,
			           NULL,
			           &err);
	
	if (! res) {
		error_dialog (applet, _("Error while spawning nautilus:\n%s"),
			      err->message);
		g_error_free (err);
	}
}

static void
trash_applet_show_help (BonoboUIComponent *component,
			TrashApplet       *applet,
			const gchar       *cname)
{
        GError *err = NULL;

        g_return_if_fail (TRASH_IS_APPLET (applet));

	/* FIXME - Actually, we need a user guide */
        gnome_help_display_desktop_on_screen
		(NULL, "trashapplet", "trashapplet", NULL,
		 gtk_widget_get_screen (GTK_WIDGET (applet)),
		 &err);

        if (err) {
        	error_dialog (applet, _("There was an error displaying help: %s"), err->message);
        	g_error_free (err);
        }
}


static void
trash_applet_show_about (BonoboUIComponent *component,
			 TrashApplet       *applet,
			 const gchar       *cname)
{
	static const char *authors[] = {
		"Michiel Sikkes <michiel@eyesopened.nl>",
		"Emmanuele Bassi <ebassi@gmail.com>",
		"Sebastian Bacher <seb128@canonical.com>",
		"James Henstridge <james.henstridge@canonical.com>",
		NULL
	};
	static const char *documenters[] = {
		"Michiel Sikkes <michiel@eyesopened.nl>",
		NULL
	};

	gtk_show_about_dialog
		(NULL,
		 "name", _("Trash Applet"),
		 "version", VERSION,
		 "copyright", "Copyright \xC2\xA9 2004 Michiel Sikkes",
		 "comments", _("A GNOME trash bin that lives in your panel. "
			       "You can use it to view the trash or drag "
			       "and drop items into the trash."),
		 "authors", authors,
		 "documenters", documenters,
		 "translator-credits", _("translator-credits"),
		 "logo_icon_name", "user-trash-full",
		 NULL);
}

static gboolean
confirm_delete_immediately (GtkWidget *parent_view,
			    gint num_files,
			    gboolean all)
{
	GdkScreen *screen;
	GtkWidget *dialog, *hbox, *vbox, *image, *label;
	gchar *str, *prompt, *detail;
	int response;

	screen = gtk_widget_get_screen (parent_view);
	
	dialog = gtk_dialog_new ();
	gtk_window_set_screen (GTK_WINDOW (dialog), screen);
	atk_object_set_role (gtk_widget_get_accessible (dialog), ATK_ROLE_ALERT);
	gtk_window_set_title (GTK_WINDOW (dialog), _("Delete Immediately?"));
	gtk_dialog_set_has_separator (GTK_DIALOG (dialog), FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (dialog), 5);
	gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);

	gtk_widget_realize (dialog);
	gdk_window_set_transient_for (GTK_WIDGET (dialog)->window,
				      gdk_screen_get_root_window (screen));
	gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);

	gtk_box_set_spacing (GTK_BOX (GTK_DIALOG (dialog)->vbox), 14);

	hbox = gtk_hbox_new (FALSE, 12);
	gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
	gtk_widget_show (hbox);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), hbox,
			    FALSE, FALSE, 0);

	image = gtk_image_new_from_stock (GTK_STOCK_DIALOG_QUESTION,
					  GTK_ICON_SIZE_DIALOG);
	gtk_misc_set_alignment (GTK_MISC (image), 0.5, 0.0);
	gtk_widget_show (image);
	gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);

        vbox = gtk_vbox_new (FALSE, 12);
        gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);
        gtk_widget_show (vbox);

	if (all) {
		prompt = _("Cannot move items to trash, do you want to delete them immediately?");
		detail = g_strdup_printf ("None of the %d selected items can be moved to the Trash", num_files);
	} else {
		prompt = _("Cannot move some items to trash, do you want to delete these immediately?");
		detail = g_strdup_printf ("%d of the selected items cannot be moved to the Trash", num_files);
	}

	str = g_strconcat ("<span weight=\"bold\" size=\"larger\">",
			   prompt, "</span>", NULL);
	label = gtk_label_new (str);
	gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
	gtk_widget_show (label);
	g_free (str);

	label = gtk_label_new (detail);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
	gtk_widget_show (label);
	g_free (detail);

	gtk_dialog_add_button (GTK_DIALOG (dialog), GTK_STOCK_CANCEL,
			       GTK_RESPONSE_CANCEL);
	gtk_dialog_add_button (GTK_DIALOG (dialog), GTK_STOCK_DELETE,
			       GTK_RESPONSE_YES);
	gtk_dialog_set_default_response (GTK_DIALOG (dialog),
					 GTK_RESPONSE_YES);

	response = gtk_dialog_run (GTK_DIALOG (dialog));

	gtk_object_destroy (GTK_OBJECT (dialog));

	return response == GTK_RESPONSE_YES;
}

static void
trash_applet_drag_data_received (GtkWidget        *widget,
				 GdkDragContext   *context,
				 gint              x,
				 gint              y,
				 GtkSelectionData *selectiondata,
				 guint             info,
				 guint             time_)
{
	TrashApplet *applet = TRASH_APPLET (widget);
  	GList *list, *scan;
	GList *source_uri_list, *target_uri_list, *unmovable_uri_list;
	GnomeVFSResult result;

	list = gnome_vfs_uri_list_parse ((gchar *)selectiondata->data);

	source_uri_list = NULL;
	target_uri_list = NULL;
	unmovable_uri_list = NULL;
	for (scan = g_list_first (list); scan; scan = g_list_next (scan)) {
		GnomeVFSURI *source_uri = scan->data;
		GnomeVFSURI *trash_uri, *target_uri;
		gchar *source_basename;

		/* find the trash directory for this file */
		result = gnome_vfs_find_directory (source_uri,
						   GNOME_VFS_DIRECTORY_KIND_TRASH,
						   &trash_uri, TRUE, FALSE, 0);
		if (result != GNOME_VFS_OK) {
			unmovable_uri_list = g_list_prepend (unmovable_uri_list,
							     gnome_vfs_uri_ref (source_uri));
			continue;
		}

		source_basename = gnome_vfs_uri_extract_short_name
			(source_uri);

		target_uri = gnome_vfs_uri_append_file_name(trash_uri,
							    source_basename);
		g_free (source_basename);
		gnome_vfs_uri_unref (trash_uri);

		source_uri_list = g_list_prepend (source_uri_list,
						  gnome_vfs_uri_ref (source_uri));
		target_uri_list = g_list_prepend (target_uri_list,
						  target_uri);
	}

	gnome_vfs_uri_list_free(list);

	/* we might have added a trash dir, so recheck */
	trash_monitor_recheck_trash_dirs (applet->monitor);

	if (source_uri_list) {
		result = gnome_vfs_xfer_uri_list (source_uri_list, target_uri_list,
						  GNOME_VFS_XFER_REMOVESOURCE |
						  GNOME_VFS_XFER_RECURSIVE,
						  GNOME_VFS_XFER_ERROR_MODE_ABORT,
						  GNOME_VFS_XFER_OVERWRITE_MODE_REPLACE,
						  NULL, NULL);
		gnome_vfs_uri_list_free (source_uri_list);
		gnome_vfs_uri_list_free (target_uri_list);
		if (result != GNOME_VFS_OK) {
			error_dialog (applet, _("Unable to move to trash:\n%s"),
				      gnome_vfs_result_to_string (result));
		}
	}
	if (unmovable_uri_list) {
		if (confirm_delete_immediately (widget,
						g_list_length (unmovable_uri_list),
						source_uri_list == NULL)) {
			result = gnome_vfs_xfer_delete_list (unmovable_uri_list,
							     GNOME_VFS_XFER_ERROR_MODE_ABORT,
							     GNOME_VFS_XFER_RECURSIVE,
							     NULL, NULL);
		} else {
			result = GNOME_VFS_OK;
		}
		gnome_vfs_uri_list_free (unmovable_uri_list);
		if (result != GNOME_VFS_OK) {
			error_dialog (applet, _("Unable to move to trash:\n%s"),
				      gnome_vfs_result_to_string (result));
		}
	}
	gtk_drag_finish (context, TRUE, FALSE, time_);
}

static gboolean
trash_applet_factory (PanelApplet *applet,
                      const gchar *iid,
		      gpointer     data)
{
	gboolean retval = FALSE;

  	if (!strcmp (iid, "OAFIID:GNOME_Panel_TrashApplet")) {
		/* Set up the menu */
		panel_applet_setup_menu_from_file (applet,
						   DATADIR,
						   "GNOME_Panel_TrashApplet.xml",
						   NULL,
						   trash_applet_menu_verbs,
						   applet);

		trash_applet_queue_update (TRASH_APPLET (applet));
		gtk_widget_show (GTK_WIDGET (applet));

		retval = TRUE;
	}

  	return retval;
}

int
main (int argc, char *argv [])
{
	/* gettext stuff */
        bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
        bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
        textdomain (GETTEXT_PACKAGE);

	gnome_authentication_manager_init();

        gnome_program_init ("trashapplet", VERSION,
                            LIBGNOMEUI_MODULE,
                            argc, argv,
                            GNOME_CLIENT_PARAM_SM_CONNECT, FALSE,
                            GNOME_PROGRAM_STANDARD_PROPERTIES,
                            NULL);
        return panel_applet_factory_main
		("OAFIID:GNOME_Panel_TrashApplet_Factory", TRASH_TYPE_APPLET,
		 trash_applet_factory, NULL);
}
