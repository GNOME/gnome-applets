/* panel-menu-add.c
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <libbonobo.h>
#include <libbonoboui.h>
#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>
#include <panel-applet.h>

#include "panel-menu.h"
#include "panel-menu-common.h"
#include "panel-menu-path.h"
#include "panel-menu-links.h"
#include "panel-menu-directory.h"
#include "panel-menu-documents.h"
#include "panel-menu-add.h"

enum {
	TARGET_URI_LIST,
	TARGET_PANEL_MENU_ENTRY
};

static GtkTargetEntry drag_types[] = {
	{"application/panel-menu-entry", GTK_TARGET_SAME_APP,
	 TARGET_PANEL_MENU_ENTRY}
};
static gint n_drag_types = sizeof (drag_types) / sizeof (GtkTargetEntry);

static void handle_response (GtkWidget *widget, gint response, gpointer data);
static void panel_menu_add_entry_cb (GtkWidget *widget,
				     PanelMenu *panel_menu);
static void panel_menu_add_entry_dnd_init (PanelMenu *panel_menu,
					   GtkWidget *widget);
static void add_entry_dnd_drag_begin_cb (GtkWidget *widget,
					 GdkDragContext *context,
					 gpointer data);
static void add_entry_dnd_set_data_cb (GtkWidget *widget,
				       GdkDragContext *context,
				       GtkSelectionData *selection_data,
				       guint info, guint time,
				       PanelMenu *panel_menu);

void
applet_add_cb (BonoboUIComponent *uic, PanelMenu *panel_menu,
	       const gchar *verbname)
{
	GtkWidget *dialog;
	GtkWidget *box;
	GtkWidget *frame;
	GtkSizeGroup *group;
	GtkWidget *hbox;
	GtkWidget *vbox;
	GtkWidget *label;
	GtkWidget *button;
	gint response;

	dialog = gtk_dialog_new ();
	gtk_window_set_title (GTK_WINDOW (dialog), _("Add items to the Menu Bar"));
	gtk_dialog_set_has_separator (GTK_DIALOG (dialog), FALSE);

	box = GTK_DIALOG (dialog)->vbox;
	frame = gtk_frame_new (NULL);
	gtk_container_set_border_width (GTK_CONTAINER (frame), GNOME_PAD_SMALL);
	gtk_box_pack_start (GTK_BOX (box), frame, TRUE, TRUE, 0);
	gtk_widget_show (frame);

	vbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), GNOME_PAD_SMALL);
	gtk_container_add (GTK_CONTAINER (frame), vbox);
	gtk_widget_show (vbox);

	group = gtk_size_group_new (GTK_SIZE_GROUP_BOTH);

	/* Row numero 1 */
	label = gtk_label_new ("To add an item to your menu panel,\n"
			       "either drag a default item with a middle-click,\n"
			       "or left-click on an item to construct and append\n"
			       "it to your panel menu.");
	gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
	gtk_widget_show (label);

	hbox = gtk_hbox_new (FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, FALSE, 0);
	gtk_widget_show (hbox);

	button = gtk_button_new_with_label ("Menu Path");
	gtk_size_group_add_widget (group, button);
	gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, FALSE, 0);
	g_object_set_data (G_OBJECT (button), "panel-menu-type-id",
			   GINT_TO_POINTER (PANEL_MENU_TYPE_PATH));
	g_signal_connect (G_OBJECT (button), "clicked",
			  G_CALLBACK (panel_menu_add_entry_cb), panel_menu);
	panel_menu_add_entry_dnd_init (panel_menu, button);
	gtk_widget_show (button);

	button = gtk_button_new_with_label ("Links");
	gtk_size_group_add_widget (group, button);
	gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, FALSE, 0);
	g_object_set_data (G_OBJECT (button), "panel-menu-type-id",
			   GINT_TO_POINTER (PANEL_MENU_TYPE_LINKS));
	g_signal_connect (G_OBJECT (button), "clicked",
			  G_CALLBACK (panel_menu_add_entry_cb), panel_menu);
	panel_menu_add_entry_dnd_init (panel_menu, button);
	gtk_widget_show (button);

	button = gtk_button_new_with_label ("Directory");
	gtk_size_group_add_widget (group, button);
	gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, FALSE, 0);
	g_object_set_data (G_OBJECT (button), "panel-menu-type-id",
			   GINT_TO_POINTER (PANEL_MENU_TYPE_DIRECTORY));
	g_signal_connect (G_OBJECT (button), "clicked",
			  G_CALLBACK (panel_menu_add_entry_cb), panel_menu);
	panel_menu_add_entry_dnd_init (panel_menu, button);
	gtk_widget_show (button);

	button = gtk_button_new_with_label ("Documents");
	gtk_size_group_add_widget (group, button);
	gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, FALSE, 0);
	g_object_set_data (G_OBJECT (button), "panel-menu-type-id",
			   GINT_TO_POINTER (PANEL_MENU_TYPE_DOCUMENTS));
	g_signal_connect (G_OBJECT (button), "clicked",
			  G_CALLBACK (panel_menu_add_entry_cb), panel_menu);
	panel_menu_add_entry_dnd_init (panel_menu, button);
	gtk_widget_show (button);

	/* Row numero 2 */
	hbox = gtk_hbox_new (FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, FALSE, 0);
	gtk_widget_show (hbox);

	g_signal_connect (G_OBJECT (dialog), "response",
			  G_CALLBACK (handle_response), NULL);
	gtk_widget_show_all (dialog);
}

static void
handle_response (GtkWidget *widget, gint response, gpointer data)
{
	gtk_widget_destroy (widget);
}

static void
panel_menu_add_entry_cb (GtkWidget *widget, PanelMenu *panel_menu)
{
	PanelMenuEntryType type;
	PanelMenuEntry *entry;

	type = (PanelMenuEntryType) g_object_get_data (G_OBJECT (widget),
						       "panel-menu-type-id");
	switch (type) {
	case PANEL_MENU_TYPE_PATH:
		panel_menu_path_new_with_dialog (panel_menu);
		break;
	case PANEL_MENU_TYPE_LINKS:
		panel_menu_links_new_with_dialog (panel_menu);
		break;
	case PANEL_MENU_TYPE_DIRECTORY:
		panel_menu_directory_new_with_dialog (panel_menu);
		break;
	case PANEL_MENU_TYPE_DOCUMENTS:
		panel_menu_documents_new_with_dialog (panel_menu);
		break;
	default:
		break;
	}
}

static void
panel_menu_add_entry_dnd_init (PanelMenu *panel_menu, GtkWidget *widget)
{
	gtk_drag_source_set (widget, GDK_BUTTON2_MASK, drag_types, n_drag_types,
			     GDK_ACTION_MOVE);
	g_signal_connect (G_OBJECT (widget), "drag_begin",
			  G_CALLBACK (add_entry_dnd_drag_begin_cb), NULL);
	g_signal_connect (G_OBJECT (widget), "drag_data_get",
			  G_CALLBACK (add_entry_dnd_set_data_cb), panel_menu);
}

static void
add_entry_dnd_drag_begin_cb (GtkWidget *widget, GdkDragContext *context,
			     gpointer data)
{
	GtkWidget *window;
	GtkWidget *button;

	window = gtk_window_new (GTK_WINDOW_POPUP);
	button = gtk_button_new_with_label (gtk_label_get_text
					    (GTK_LABEL
					    (GTK_BIN (widget)->child)));
	gtk_container_add (GTK_CONTAINER (window), button);
	gtk_widget_show_all (button);
	gtk_drag_set_icon_widget (context, window, 0, 0);
}

static void
add_entry_dnd_set_data_cb (GtkWidget *widget, GdkDragContext *context,
			   GtkSelectionData *selection_data, guint info,
			   guint time, PanelMenu *panel_menu)
{
	PanelMenuEntry *entry = NULL;
	PanelMenuEntryType type = 0;
	GtkWidget *menuitem;
	GtkWidget *checkitem;
	gchar *str;

	if (info == TARGET_PANEL_MENU_ENTRY) {
		type = (PanelMenuEntryType)
			g_object_get_data (G_OBJECT (widget),
					   "panel-menu-type-id");
		switch (type) {
		case PANEL_MENU_TYPE_PATH:
			entry = panel_menu_path_new (panel_menu,
						     "applications:");
			break;
		case PANEL_MENU_TYPE_LINKS:
			entry = panel_menu_path_new (panel_menu, _("Shortcuts"));
			break;
		case PANEL_MENU_TYPE_DIRECTORY:
			str = g_strdup_printf ("%s/Documents",
					       g_get_home_dir ());
			entry = panel_menu_directory_new (panel_menu,
							  _("My Documents"), str,
							  1);
			g_free (str);
			break;
		case PANEL_MENU_TYPE_DOCUMENTS:
			entry = panel_menu_documents_new (panel_menu,
							  _("Documents"));
			break;
		default:
			break;
		}
		if (entry) {
			entry->parent = NULL;
			menuitem = panel_menu_common_get_entry_menuitem (entry);
			g_object_ref (G_OBJECT (menuitem));
			gtk_selection_data_set (selection_data,
						selection_data->target, 8,
						(gpointer) & entry,
						sizeof (gpointer));
		} else
			gtk_selection_data_set (selection_data,
						selection_data->target, 8, NULL,
						0);
	} else {
		gtk_selection_data_set (selection_data, selection_data->target,
					8, NULL, 0);
	}
}
