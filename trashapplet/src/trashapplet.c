/* -*- mode: c; c-basic-offset: 8 -*-
 * trashapplet.c
 *
 * Copyright (c) 2004  Michiel Sikkes <michiel@eyesopened.nl>,
 *               2004  Emmanuele Bassi <ebassi@gmail.com>
 *               2008  Ryan Lortie <desrt@desrt.ca>
 *                     Matthias Clasen <mclasen@redhat.com>
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
#include <gio/gio.h>

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
static void trash_applet_theme_change (GtkIconTheme *icon_theme, gpointer data);

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
static void empty_changed             (GObject      *monitor,
                                       GParamSpec   *pspace,
                                       gpointer      user_data);

static gboolean
trash_applet_query_tooltip (GtkWidget  *widget,
                            gint        x,
                            gint        y,
                            gboolean    keyboard_tip,
                            GtkTooltip *tooltip)
{
  TrashApplet *applet = TRASH_APPLET (widget);
  gint items;

  g_object_get (applet->monitor, "items", &items, NULL);

  if (items)
    {
      char *text;

      text = g_strdup_printf (ngettext ("%d Item in Trash",
                                        "%d Items in Trash",
                                        items), items);
      gtk_tooltip_set_text (tooltip, text);
      g_free (text);
    }
  else
    gtk_tooltip_set_text (tooltip, _("No Items in Trash"));

  return TRUE;
}

static void
trash_applet_class_init (TrashAppletClass *class)
{
  PanelAppletClass *applet_class = PANEL_APPLET_CLASS (class);
  GtkObjectClass *gtkobject_class = GTK_OBJECT_CLASS (class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (class);

  gtkobject_class->destroy = trash_applet_destroy;
  widget_class->size_allocate = trash_applet_size_allocate;
  widget_class->button_release_event = trash_applet_button_release;
  widget_class->key_press_event = trash_applet_key_press;
  widget_class->drag_leave = trash_applet_drag_leave;
  widget_class->drag_motion = trash_applet_drag_motion;
  widget_class->drag_data_received = trash_applet_drag_data_received;
  widget_class->query_tooltip = trash_applet_query_tooltip;
  applet_class->change_orient = trash_applet_change_orient;
  applet_class->change_background = trash_applet_change_background;
}

static void
trash_applet_init (TrashApplet *applet)
{
	gtk_window_set_default_icon_name (TRASH_ICON_FULL);

	panel_applet_set_flags (PANEL_APPLET (applet),
				PANEL_APPLET_EXPAND_MINOR);
        gtk_widget_set_has_tooltip (GTK_WIDGET (applet), TRUE);

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

	applet->image = gtk_image_new ();
	gtk_container_add (GTK_CONTAINER (applet), applet->image);
	gtk_widget_show (applet->image);
	applet->icon_state = TRASH_STATE_UNKNOWN;

	/* set up trash monitor */
	applet->monitor = trash_monitor_new ();
	applet->monitor_signal_id =
		g_signal_connect (applet->monitor, "notify::empty",
				  G_CALLBACK (empty_changed), applet);

	/* initial state */
	applet->is_empty = TRUE;
	applet->drag_hover = FALSE;

	/* set up DnD target */
        gtk_drag_dest_set (GTK_WIDGET (applet),
                           GTK_DEST_DEFAULT_ALL,
                           drop_types, n_drop_types,
                           GDK_ACTION_MOVE);

  /* handle icon theme changes */
  g_signal_connect (gtk_icon_theme_get_default (),
		    "changed", G_CALLBACK (trash_applet_theme_change),
		    applet);        
}

static void
trash_applet_destroy (GtkObject *object)
{
	TrashApplet *applet = TRASH_APPLET (object);
	
	applet->image = NULL;

	if (applet->monitor_signal_id)
		g_signal_handler_disconnect (applet->monitor,
					     applet->monitor_signal_id);
	applet->monitor_signal_id = 0;

        g_object_unref (applet->monitor);

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
	default:
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
empty_changed (GObject    *monitor,
               GParamSpec *pspac,
               gpointer    user_data)
{
  trash_applet_queue_update (TRASH_APPLET (user_data));
}

static gboolean
trash_applet_update (gpointer user_data)
{
	TrashApplet *applet = TRASH_APPLET (user_data);
	BonoboUIComponent *popup_component;
	char *tip_text;
	TrashState new_state = TRASH_STATE_UNKNOWN;
	const char *new_icon;
	GdkScreen *screen;
	GtkIconTheme *icon_theme;
	GdkPixbuf *pixbuf, *scaled;
        gboolean is_empty;

	applet->update_id = 0;

        g_object_get (applet->monitor, "empty", &is_empty, NULL);
	if (is_empty != applet->is_empty)
        {
		applet->is_empty = is_empty;

		/* set sensitivity on the "empty trash" context menu item */
		popup_component = panel_applet_get_popup_component (PANEL_APPLET (applet));
		bonobo_ui_component_set_prop (popup_component,
					      "/commands/EmptyTrash",
					      "sensitive",
					      applet->is_empty ? "0" : "1",
					      NULL);
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

static void
trash_applet_theme_change (GtkIconTheme *icon_theme, gpointer data)
{
  TrashApplet *applet = TRASH_APPLET (data);
  trash_applet_update (applet);
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

static void
trash_applet_do_empty (BonoboUIComponent *component,
		       TrashApplet       *applet,
		       const gchar       *cname)
{
  trash_empty (GTK_WIDGET (applet));
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
                "Ryan Lortie <desrt@desrt.ca>",
		NULL
	};
	static const char *documenters[] = {
		"Michiel Sikkes <michiel@eyesopened.nl>",
		NULL
	};

	gtk_show_about_dialog
		(NULL,
		 "version", VERSION,
		 "copyright", "Copyright \xC2\xA9 2004 Michiel Sikkes,"
                              "\xC2\xA9 2008 Ryan Lortie",
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
	gchar **list;
	gint i;
	GList *trashed = NULL;
	GList *untrashable = NULL;
	GList *l;
	GError *error = NULL;

	list = g_uri_list_extract_uris ((gchar *)selectiondata->data);

	for (i = 0; list[i]; i++) {
		GFile *file;

		file = g_file_new_for_uri (list[i]);
		if (!g_file_trash (file, NULL, NULL)) {
			untrashable = g_list_prepend (untrashable, file);
		}
		else {
			trashed = g_list_prepend (trashed, file);
		}
	}
	
	if (untrashable) {
		if (confirm_delete_immediately (widget, 
						g_list_length (untrashable),
						trashed == NULL)) {
			for (l = untrashable; l; l = l->next) {
				if (!g_file_delete (l->data, NULL, &error)) {
/*
 * FIXME: uncomment me after branched (we're string frozen)
					error_dialog (applet,
						      _("Unable to delete '%s': %s"),
							g_file_get_uri (l->data),
							error->message);
*/
					g_clear_error (&error);
				}	
			}
		}
	}

	g_list_foreach (untrashable, (GFunc)g_object_unref, NULL);
	g_list_free (untrashable);
	g_list_foreach (trashed, (GFunc)g_object_unref, NULL);
	g_list_free (trashed);

	g_strfreev (list);	

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
	g_thread_init (NULL);

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
	g_set_application_name (_("Trash Applet"));

        return panel_applet_factory_main
		("OAFIID:GNOME_Panel_TrashApplet_Factory", TRASH_TYPE_APPLET,
		 trash_applet_factory, NULL);
}
