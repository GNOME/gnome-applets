/* trashapplet.c
 *
 * Copyright (c) 2004  Michiel Sikkes <michiel@eyesopened.nl>,
 *               2004  Emmanuele Bassi <ebassi@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License VERSION 2 as
 * published by the Free Software Foundation.  You are not allowed to
 * use any other version of the license; unless you got the explicit
 * permission from the author to do so.
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

#include <gtk/gtktooltips.h>
#include <gtk/gtkimage.h>
#include <gtk/gtkstock.h>
#include <gtk/gtkmessagedialog.h>
#include <gconf/gconf-client.h>
#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>

#include "eel-extension.h"
#include "trashapplet.h"
#include "trash-monitor.h"

static void	trash_applet_do_empty 	(BonoboUIComponent *component,
					 TrashApplet *ta,
					 const gchar *cname);
static void	trash_applet_show_about	(BonoboUIComponent *component,
					 TrashApplet *ta,
					 const gchar *cname);
static void	trash_applet_open_folder (BonoboUIComponent *component,
					  TrashApplet *ta,
					  const gchar *cname);
static void	trash_applet_show_help (BonoboUIComponent *component,
					TrashApplet *ta,
					const gchar *cname);

static gboolean		trash_applet_is_empty	(TrashApplet *trash_applet);
static void		trash_applet_update_icon (TrashApplet *trash_applet);
static void 		trash_applet_update_tip (TrashApplet *trash_applet);

static const BonoboUIVerb trash_applet_menu_verbs [] = {
	BONOBO_UI_UNSAFE_VERB ("EmptyTrash", trash_applet_do_empty),
	BONOBO_UI_UNSAFE_VERB ("OpenTrash", trash_applet_open_folder),
	BONOBO_UI_UNSAFE_VERB ("AboutTrash", trash_applet_show_about),
	BONOBO_UI_UNSAFE_VERB ("HelpTrash", trash_applet_show_help),
	BONOBO_UI_VERB_END
};

static GtkTargetEntry drop_types[] =
{
	{ "text/uri-list", 0, 0 }
};

static gint n_drop_types = sizeof (drop_types) / sizeof (drop_types[0]);

static GConfClient *client = NULL;

/* TODO - Must HIGgify this dialog */
static void
error_dialog (TrashApplet *ta, const gchar *error, ...)
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
			gtk_widget_get_screen (GTK_WIDGET (ta->applet)));
	gtk_widget_show (dialog);

	g_free (error_string);
}

static GnomeVFSURI *
trash_applet_get_main_trash_directory_uri (void)
{
	GnomeVFSResult trash_check;
	GnomeVFSURI *retval;

	trash_check = gnome_vfs_find_directory (NULL,
			GNOME_VFS_DIRECTORY_KIND_TRASH,
			&retval,
			FALSE,
			FALSE,
			0);

	if (trash_check != GNOME_VFS_OK) {
		g_warning (_("Unable to find the Trash directory: %s"),
				gnome_vfs_result_to_string (trash_check));
		return NULL;
	}

	return retval;
}

static void
trash_applet_update_item_count (TrashApplet *trash_applet)
{
	BonoboUIComponent *popup_component;

	g_return_if_fail (trash_applet != NULL);
	
	trash_applet->item_count = trash_monitor_get_item_count (trash_applet->monitor);

	trash_applet->is_empty = (trash_applet->item_count == 0);

	/* set sensitivity on the "empty trash" context menu item */
	popup_component = panel_applet_get_popup_component (trash_applet->applet);
	bonobo_ui_component_set_prop (popup_component,
				      "/commands/EmptyTrash",
				      "sensitive",
				      trash_applet->is_empty ? "0" : "1",
				      NULL);
}

static guint
trash_applet_items_count (TrashApplet *trash_applet)
{
	g_return_val_if_fail (trash_applet != NULL, 0);

	return trash_applet->item_count;
}


static gboolean
trash_applet_is_empty (TrashApplet *trash_applet)
{
	g_return_val_if_fail (trash_applet != NULL, FALSE);

	return trash_applet->is_empty;
}

static void
trash_applet_update_icon (TrashApplet *trash_applet)
{
	TrashState new_state = TRASH_STATE_UNKNOWN;
	gchar *icon_name = NULL;
	GdkPixbuf *new_icon;
	
	g_return_if_fail (trash_applet != NULL);

	if (trash_applet->icon == NULL)
		return;

	if (trash_applet->drag_hover) {
		new_state = TRASH_STATE_ACCEPT;
		icon_name = TRASH_ICON_EMPTY_ACCEPT;
	} else if (trash_applet->is_empty) {
		new_state = TRASH_STATE_EMPTY;
		icon_name = TRASH_ICON_EMPTY;
	} else {
		new_state = TRASH_STATE_FULL;
		icon_name = TRASH_ICON_FULL;
	}

	/* if the icon is up to date, return */
	if (trash_applet->icon && trash_applet->icon_state == new_state)
		return;

	new_icon = gtk_icon_theme_load_icon (trash_applet->icon_theme,
					     icon_name,
					     trash_applet->size,
					     0,
					     NULL);

	/* Handle themes that do not have a -accept icon variant */
	if (!new_icon && new_state == TRASH_STATE_ACCEPT) {
		icon_name = trash_applet->is_empty
			? TRASH_ICON_EMPTY : TRASH_ICON_FULL;
		new_icon = gtk_icon_theme_load_icon (trash_applet->icon_theme,
						     icon_name,
						     trash_applet->size,
						     0,
						     NULL);
	}

	if (trash_applet->icon)
		g_object_unref (G_OBJECT (trash_applet->icon));

	trash_applet->icon = new_icon;
	trash_applet->icon_state = new_state;
	
	gtk_image_set_from_pixbuf (GTK_IMAGE (trash_applet->image), trash_applet->icon);
	
	return;
}

static void
trash_applet_update_tip (TrashApplet *trash_applet)
{
	gchar *tip_text;
	guint items;

	items = trash_applet_items_count (trash_applet);
	switch (items) {
		case 0:
			tip_text = g_strdup (_("No Items in Trash"));
			break;
		case 1:
			tip_text = g_strdup (_("1 Item in Trash"));
			break;
		default:
			tip_text = g_strdup_printf (_("%d Items in Trash"), 
							items);
	}

	gtk_tooltips_set_tip (GTK_TOOLTIPS (trash_applet->tooltips),
			     GTK_WIDGET (trash_applet->applet),
			     tip_text,
			     NULL);

	g_free (tip_text);
}

static gint
update_transfer_callback (GnomeVFSAsyncHandle *handle,
                         GnomeVFSXferProgressInfo *progress_info,
                         gpointer user_data)
{
       /* UI updates here */
       TrashApplet *ta = (TrashApplet *) user_data;

       return 1;
}


/* this function is based on the one with the same name in
   libnautilus-private/nautilus-file-operations.c */
#define NAUTILUS_PREFERENCES_CONFIRM_TRASH    "/apps/nautilus/preferences/confirm_trash"
static gboolean
confirm_empty_trash (GtkWidget *parent_view)
{
	GtkWidget *dialog;
	int response;
	GtkWidget *hbox, *vbox, *image, *label, *button;
	gchar     *str;
	GdkScreen *screen;

	/* Just Say Yes if the preference says not to confirm. */
	if (!gconf_client_get_bool (client,
				    NAUTILUS_PREFERENCES_CONFIRM_TRASH,
				    NULL)) {
		return TRUE;
	}
	
	screen = gtk_widget_get_screen (parent_view);

	dialog = gtk_dialog_new ();
	gtk_window_set_screen (GTK_WINDOW (dialog), screen);
	atk_object_set_role (gtk_widget_get_accessible (dialog), ATK_ROLE_ALERT);
	gtk_window_set_title (GTK_WINDOW (dialog), "");
	gtk_dialog_set_has_separator (GTK_DIALOG (dialog), FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (dialog), 5);
	gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
	gtk_window_set_wmclass (GTK_WINDOW (dialog), "empty_trash",
				"Nautilus");

	/* Make transient for the window group */
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

	str = g_strconcat ("<span weight=\"bold\" size=\"larger\">", 
		_("Are you sure you want to empty "
		"all of the items from the trash?"), 
		"</span>", 
		NULL);
		
	label = gtk_label_new (str);  
	gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
	gtk_widget_show (label);
	g_free (str);

	label = gtk_label_new (_("If you empty the trash, items "
		"will be permanently deleted."));
	
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
	gtk_widget_show (label);

	gtk_dialog_add_button (GTK_DIALOG (dialog), GTK_STOCK_CANCEL,
			       GTK_RESPONSE_CANCEL);

	button = eel_gtk_button_new_with_stock_icon (_("_Empty"),
						     GTK_STOCK_DELETE);
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
trash_applet_do_empty (BonoboUIComponent *component,
		    TrashApplet *ta,
		    const gchar *cname)
{
       GnomeVFSAsyncHandle *hnd;

       g_return_if_fail (PANEL_IS_APPLET (ta->applet));

       if (trash_applet_is_empty (ta))
               return;

       if (!confirm_empty_trash (GTK_WIDGET (ta->applet)))
	       return;

       trash_monitor_empty_trash (ta->monitor,
				  &hnd, update_transfer_callback, ta);
}

static void
trash_applet_open_folder (BonoboUIComponent *component,
			  TrashApplet *ta,
			  const gchar *cname)
{
	/* Open the "trash:" URI with gnome-open */
	gchar *argv[] = { "gnome-open", EEL_TRASH_URI, NULL };
	GError *err = NULL;
	gboolean res;	

        g_return_if_fail (PANEL_IS_APPLET (ta->applet));

	res = g_spawn_async (NULL,
			     argv, NULL,
			     G_SPAWN_SEARCH_PATH,
			     NULL, NULL,
			     NULL,
			     &err);
	
	if (! res) {
		error_dialog (ta, _("Error while spawing nautilus:\n%s"), err->message);
		g_error_free (err);
	}
}

static void
trash_applet_show_help (BonoboUIComponent *component,
                  TrashApplet *ta,
                  const gchar *cname)
{
        GError *error = NULL;

        g_return_if_fail (PANEL_IS_APPLET (ta->applet));

       /* FIXME - Actually, we need a user guide */
        gnome_help_display_desktop_on_screen (
                NULL, "trashapplet", "trashapplet", NULL,
                gtk_widget_get_screen (GTK_WIDGET (ta->applet)),
               &error);

        if (error) {
        	error_dialog (ta, _("There was an error displaying help: %s"), error->message);
        	g_error_free (error);
        }
}


static void
trash_applet_show_about (BonoboUIComponent *component,
		    TrashApplet *ta,
		    const gchar *cname)
{
	GtkWidget *about;
	gchar *translator_credits = _("translator_credits");
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

	about = gnome_about_new ("Trash Applet", VERSION,
			_("Copyright (c) 2004 Michiel Sikkes"),
			_("The GNOME Trash Applet"),
			authors,
			documenters,
			strcmp (translator_credits, "translator_credits") != 0 ? translator_credits : NULL,
			NULL);

	gtk_widget_show(about);

}

static void 
update_icon_cb (GtkWidget *applet, gpointer data)
{
	TrashApplet *trash_applet = (TrashApplet *) data;

	trash_applet->icon_state = TRASH_STATE_UNKNOWN;
	trash_applet_update_icon(trash_applet);
}

static gboolean 
button_press_cb (GtkWidget *applet, GdkEventButton *event, TrashApplet *ta)
{
  if (event->button == 1)
    trash_applet_open_folder(NULL, ta, NULL);

  return FALSE;
}

static void
destroy_cb (GtkWidget *applet, gpointer data)
{
	TrashApplet *trash_applet = (TrashApplet *) data;
	
	if (trash_applet) {
	       g_signal_handler_disconnect (trash_applet->monitor,
					    trash_applet->monitor_signal_id);

               g_object_unref (trash_applet->icon);
               g_object_unref (trash_applet->tooltips);

               g_free (trash_applet);
	}
}

static gboolean
confirm_delete_immediately (GtkWidget *parent_view,
			    gint num_files,
			    gboolean all)
{
	GdkScreen *screen;
	GtkWidget *dialog, *hbox, *vbox, *image, *label, *button;
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
drag_data_received_cb (GtkWidget *applet, GdkDragContext *context,
		       gint x, gint y,
		       GtkSelectionData *selection_data,
		       guint info, guint time, gpointer data)
{
  	GList *list, *scan;
	GList *source_uri_list, *target_uri_list, *unmovable_uri_list;
	GnomeVFSResult result;
	TrashApplet *trash_applet = (TrashApplet *) data;

	list = gnome_vfs_uri_list_parse (selection_data->data);

	source_uri_list = NULL;
	target_uri_list = NULL;
	unmovable_uri_list = NULL;
	for (scan = g_list_first (list); scan; scan = g_list_next (scan)) {
		GnomeVFSURI *source_uri = scan->data;
		GnomeVFSURI *trash_uri, *target_uri;
		gboolean is_local;
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
	trash_monitor_recheck_trash_dirs (trash_applet->monitor);

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
			error_dialog (trash_applet, _("Unable to move to trash:\n%s"),
				      gnome_vfs_result_to_string (result));
		}
	}
	if (unmovable_uri_list) {
		if (confirm_delete_immediately (trash_applet->applet,
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
			error_dialog (trash_applet, _("Unable to move to trash:\n%s"),
				      gnome_vfs_result_to_string (result));
		}
	}
}

static gboolean
drag_motion_cb (GtkWidget      *applet,
		GdkDragContext *context,
		gint            x,
		gint            y,
		guint           time,
		TrashApplet    *trash_applet)
{
	if (!trash_applet->drag_hover) {
		trash_applet->drag_hover = TRUE;
		trash_applet_update_icon (trash_applet);
	}
	gdk_drag_status (context, context->suggested_action, time);
	return TRUE;
}

static void
drag_leave_cb (GtkWidget      *applet,
	       GdkDragContext *context,
	       guint           time,
	       TrashApplet    *trash_applet)
{
	if (trash_applet->drag_hover) {
		trash_applet->drag_hover = FALSE;
		trash_applet_update_icon (trash_applet);
	}
}

static void
changed_orient_cb (GtkWidget         *applet,
                  PanelAppletOrient orient,
                  TrashApplet       *trash_applet)
{
       GtkOrientation new_orient;
       switch (orient) {
               case PANEL_APPLET_ORIENT_LEFT:
               case PANEL_APPLET_ORIENT_RIGHT:
                       new_orient = GTK_ORIENTATION_VERTICAL;
                       break;
               case PANEL_APPLET_ORIENT_UP:
               case PANEL_APPLET_ORIENT_DOWN:
               default:
                       new_orient = GTK_ORIENTATION_HORIZONTAL;
                       break;
       }
       if (new_orient == trash_applet->orient)
               return;
       trash_applet->orient = new_orient;
       trash_applet->icon_state = TRASH_STATE_UNKNOWN;
       trash_applet_update_icon (trash_applet);
}

static void
changed_background_cb (PanelApplet               *applet,
                      PanelAppletBackgroundType  type,
                      GdkColor                  *color,
                      const gchar               *pixmap,
                      TrashApplet               *trash_applet)
{
       if (type == PANEL_NO_BACKGROUND ||
	   type == PANEL_PIXMAP_BACKGROUND) {
               GtkRcStyle *rc_style;

               rc_style = gtk_rc_style_new ();

               gtk_widget_modify_style (GTK_WIDGET (trash_applet->applet), rc_style);
	       gtk_widget_modify_style (GTK_WIDGET (trash_applet->image), rc_style);
               g_object_unref (rc_style);
       } else if (type == PANEL_COLOR_BACKGROUND) {
               gtk_widget_modify_bg (GTK_WIDGET (trash_applet->applet),
                                     GTK_STATE_NORMAL,
                                     color);
               gtk_widget_modify_bg (GTK_WIDGET (trash_applet->image),
                                     GTK_STATE_NORMAL,
                                     color);
       }
}

static void
size_allocate_cb (PanelApplet *applet,
		  GtkAllocation *allocation,
		  gpointer    data)
{
       TrashApplet *trash_applet = (TrashApplet *) data;
       PanelAppletOrient orient;

       orient = panel_applet_get_orient (applet);

       if (orient == PANEL_APPLET_ORIENT_UP || orient == PANEL_APPLET_ORIENT_DOWN) {
	 if (trash_applet->size == allocation->height)
	   return;
	 trash_applet->size = allocation->height;
       } else {
	 if (trash_applet->size == allocation->width)
	   return;
	 trash_applet->size = allocation->width;
       }

       trash_applet->icon_state = TRASH_STATE_UNKNOWN;

       trash_applet_update_icon (trash_applet);
       trash_applet_update_tip (trash_applet);
}

static void
item_count_changed_cb (TrashMonitor *monitor,
		       TrashApplet *trash_applet)
{
       trash_applet_update_item_count (trash_applet);

       trash_applet_update_icon (trash_applet);
       trash_applet_update_tip (trash_applet);
}
    

static gboolean
trash_applet_fill (PanelApplet *applet)
{
	GtkWidget *box;
	guint size;
	TrashApplet *ta;
        gchar *trash_dir;
        GnomeVFSResult res;
        GnomeVFSURI *trash_dir_uri;
        GnomeVFSMonitorHandle *hnd;

	/* get default gconf client */
	if (!client)
		client = gconf_client_get_default ();

	panel_applet_set_flags (applet, PANEL_APPLET_EXPAND_MINOR);
	
	ta = g_new0 (TrashApplet, 1);
	
	/* init applet data */
	ta->applet = applet;

        ta->size = panel_applet_get_size (ta->applet);
        switch (panel_applet_get_orient (ta->applet)) {
                case PANEL_APPLET_ORIENT_LEFT:
                case PANEL_APPLET_ORIENT_RIGHT:
                        ta->orient = GTK_ORIENTATION_VERTICAL;
                        break;
                case PANEL_APPLET_ORIENT_UP:
                case PANEL_APPLET_ORIENT_DOWN:
                default:
                        ta->orient = GTK_ORIENTATION_HORIZONTAL;
                        break;
        }

	ta->item_count = 0;
	ta->is_empty = TRUE;
	ta->drag_hover = FALSE;

	trash_dir = g_build_filename (g_get_home_dir (), ".Trash", NULL);
	if (!g_file_test (trash_dir, G_FILE_TEST_EXISTS))
	  mkdir (trash_dir, 0700);

	g_free(trash_dir);
	trash_dir = NULL;	  
	  
	trash_dir_uri = trash_applet_get_main_trash_directory_uri ();
	if (trash_dir_uri != NULL)
	  trash_dir = gnome_vfs_uri_to_string (trash_dir_uri, GNOME_VFS_URI_HIDE_NONE);

	ta->monitor = trash_monitor_get ();
	ta->monitor_signal_id =
		g_signal_connect (ta->monitor, "item_count_changed",
				  G_CALLBACK (item_count_changed_cb), ta);

        /* fetch the icon theme */
        ta->icon_theme = gtk_icon_theme_get_default ();
	ta->icon_state = TRASH_STATE_UNKNOWN;
			
 
        /* use the trash icon from the icon theme */
        if (ta->icon == NULL) {
                ta->icon = gtk_icon_theme_load_icon (ta->icon_theme,
                                (trash_applet_is_empty (ta) ?
                                 TRASH_ICON_EMPTY : TRASH_ICON_FULL),
                                ta->size,
                                0,
                                NULL);
        }

        if (ta->icon) {
                ta->image = gtk_image_new_from_pixbuf (ta->icon);
        } else {
                ta->image = gtk_image_new_from_stock (GTK_STOCK_MISSING_IMAGE,
                                                      GTK_ICON_SIZE_SMALL_TOOLBAR);
        }
 
        ta->tooltips = gtk_tooltips_new ();
 
        box = gtk_hbox_new(TRUE, 0);

        gtk_box_pack_start (GTK_BOX(box), ta->image, TRUE, TRUE, 0);

        gtk_drag_dest_set (GTK_WIDGET (ta->applet),
                           GTK_DEST_DEFAULT_ALL,
                           drop_types, n_drop_types,
                           GDK_ACTION_MOVE | GDK_ACTION_COPY);

        g_signal_connect (G_OBJECT (ta->applet),
                          "destroy",
                          G_CALLBACK (destroy_cb),
                          ta);
        g_signal_connect (G_OBJECT (ta->applet),
                          "drag_data_received",
                          G_CALLBACK (drag_data_received_cb),
                          ta);
	g_signal_connect (G_OBJECT (ta->applet),
			  "drag_motion",
			  G_CALLBACK (drag_motion_cb),
			  ta);
	g_signal_connect (G_OBJECT (ta->applet),
			  "drag_leave",
			  G_CALLBACK (drag_leave_cb),
			  ta);
        g_signal_connect (G_OBJECT (ta->applet),
                          "change_orient",
                          G_CALLBACK (changed_orient_cb),
                          ta);
        g_signal_connect (G_OBJECT (ta->applet),
                          "size_allocate",
                          G_CALLBACK (size_allocate_cb),
			  ta);

        g_signal_connect (G_OBJECT (ta->applet),
                          "change_background",
                          G_CALLBACK (changed_background_cb),
                          ta);
        g_signal_connect (G_OBJECT (ta->applet),
                          "button-press-event",
                          G_CALLBACK (button_press_cb),
                          ta);
	g_signal_connect (G_OBJECT (ta->icon_theme),
			"changed",
			G_CALLBACK(update_icon_cb),
			ta);
 
        gtk_container_add (GTK_CONTAINER(ta->applet), box);
	
	/* Set up the menu */
	panel_applet_setup_menu_from_file (ta->applet,
			NULL,
			"GNOME_Panel_TrashApplet.xml",
			NULL,
			trash_applet_menu_verbs,
			ta);

        /* update display */
	trash_applet_update_item_count (ta);
        trash_applet_update_icon (ta);
        trash_applet_update_tip (ta);

	gtk_widget_show_all (GTK_WIDGET(ta->applet));

	return TRUE;
}	

static gboolean
trash_applet_factory (PanelApplet *applet,
                      const gchar *iid,
		      gpointer     data)
{
	gboolean retval = FALSE;

  	if (!strcmp (iid, "OAFIID:GNOME_Panel_TrashApplet"))
    		retval = trash_applet_fill (applet);

  	return retval;
}

PANEL_APPLET_BONOBO_FACTORY ("OAFIID:GNOME_Panel_TrashApplet_Factory",
		PANEL_TYPE_APPLET,
		"trashapplet",
		"0",
		trash_applet_factory,
		NULL);
