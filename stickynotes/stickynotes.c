/* Sticky Notes
 * Copyright (C) 2002-2003 Loban A Rahman
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include <config.h>
#include <libxml/parser.h>
#include <glade/glade.h>
#include <util.h>
#include <stickynotes.h>
#include <stickynotes_callbacks.h>
#include <stickynotes_applet.h>

/* Settings for the popup menu on all sticky notes */
static GnomeUIInfo popup_menu[] =
{
	GNOMEUIINFO_ITEM_STOCK	(N_("_New Note"), NULL, popup_create_cb, GTK_STOCK_NEW),
	GNOMEUIINFO_ITEM_STOCK	(N_("_Delete Note"), NULL, popup_destroy_cb, GTK_STOCK_DELETE),
	GNOMEUIINFO_SEPARATOR,
	GNOMEUIINFO_ITEM	(N_("_Edit Title"), NULL, popup_edit_cb, NULL),
	GNOMEUIINFO_END
};

/* Create a new (empty) Sticky Note */
StickyNote * stickynote_new()
{
	/* Create and initialize a Sticky Note */
	StickyNote *note = g_new(StickyNote, 1);
	note->glade = glade_xml_new(GLADE_PATH, "stickynote_window", NULL);
	note->window = glade_xml_get_widget(note->glade, "stickynote_window");
	note->title = glade_xml_get_widget(note->glade, "title_label");
	note->body = glade_xml_get_widget(note->glade, "body_text");

	/* FIXME : Hack because libglade does not properly set these */
	gtk_image_set_from_file(GTK_IMAGE(glade_xml_get_widget(note->glade, "resize_img")), STICKYNOTES_ICONDIR "/resize.png");
	gtk_image_set_from_file(GTK_IMAGE(glade_xml_get_widget(note->glade, "close_img")), STICKYNOTES_ICONDIR "/close.png");
	
	/* Add the note to the linked-list of all notes */
	stickynotes->notes = g_list_append(stickynotes->notes, note);
	
	/* Customize the window */
	gtk_window_set_decorated(GTK_WINDOW(note->window), FALSE);
	gtk_window_set_skip_taskbar_hint(GTK_WINDOW(note->window), TRUE);
	gtk_window_set_skip_pager_hint(GTK_WINDOW(note->window), TRUE);
	if (gconf_client_get_bool(stickynotes->gconf_client, GCONF_PATH "/settings/sticky", NULL))
		gtk_window_stick(GTK_WINDOW(note->window));
	gtk_window_resize(GTK_WINDOW(note->window), gconf_client_get_int(stickynotes->gconf_client, GCONF_PATH "/defaults/width", NULL),
						    gconf_client_get_int(stickynotes->gconf_client, GCONF_PATH "/defaults/height", NULL));

	/* Customize the date in the title label */
	{
		gchar *date0 = gconf_client_get_string(stickynotes->gconf_client, GCONF_PATH "/settings/date_format", NULL);
		gchar *date1 = get_current_date(date0);
		gchar *date2 = g_strdup_printf("<b>%s</b>", date1);
		gtk_label_set_markup(GTK_LABEL(note->title), date2);
		g_free(date0);
		g_free(date1);
		g_free(date2);
	}
	
	/* Customize the colors */
	stickynote_set_highlighted(note, FALSE);

	{
		GtkWidget *title_box = glade_xml_get_widget(note->glade, "title_box");
		GtkWidget *resize_box = glade_xml_get_widget(note->glade, "resize_box");
		GtkWidget *close_box = glade_xml_get_widget(note->glade, "close_box");
		
		/* Connect a popup menu to the title label */
		gnome_popup_menu_attach(gnome_popup_menu_new(popup_menu), title_box, note);

		/* Connect signals for window management (deleting, moving, resizing) on the sticky note */
		g_signal_connect(G_OBJECT(note->window), "delete-event", G_CALLBACK(window_delete_cb), note);
		g_signal_connect(G_OBJECT(note->window), "enter-notify-event", G_CALLBACK(window_cross_cb), note);
		g_signal_connect(G_OBJECT(note->window), "leave-notify-event", G_CALLBACK(window_cross_cb), note);
		g_signal_connect(G_OBJECT(note->window), "focus-in-event", G_CALLBACK(window_focus_cb), note);
		g_signal_connect(G_OBJECT(note->window), "focus-out-event", G_CALLBACK(window_focus_cb), note);
		g_signal_connect(G_OBJECT(title_box), "button-press-event", G_CALLBACK(window_move_edit_cb), note);
		g_signal_connect(G_OBJECT(resize_box), "button-press-event", G_CALLBACK(window_resize_cb), note);
		g_signal_connect(G_OBJECT(close_box), "button-release-event", G_CALLBACK(window_close_cb), note);
	}

	/* Update tooltips */
	{
		gchar *tooltip = g_strdup_printf(_("Sticky Notes\n%d note(s)"), g_list_length(stickynotes->notes));
		gtk_tooltips_set_tip(stickynotes->tooltips, GTK_WIDGET(stickynotes->applet), tooltip, NULL);
		g_free(tooltip);
	}
	
	/* Finally show it all. */
	gtk_widget_show_all(note->window);
	
	return note;
}

/* Destroy a Sticky Note */
void stickynote_free(StickyNote *note)
{
	/* Remove the note from the linked-list of all notes */
	stickynotes->notes = g_list_remove(stickynotes->notes, note);
	
	g_object_unref(note->glade);
	gtk_widget_destroy(note->window);
	g_free(note);

	/* Update tooltips */
	{
		gchar *tooltip = g_strdup_printf(_("Sticky Notes\n%d note(s)"), g_list_length(stickynotes->notes));
		gtk_tooltips_set_tip(stickynotes->tooltips, GTK_WIDGET(stickynotes->applet), tooltip, NULL);
		g_free(tooltip);
	}
}

/* Check if a sticky note is empty */
gboolean stickynote_get_empty(const StickyNote *note)
{
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(note->body));
	
	return gtk_text_buffer_get_char_count(buffer) == 0;
}

/* (Un)highlight a sticky note */
void stickynote_set_highlighted(StickyNote *note, gboolean highlighted)
{
	GtkWidget *title_box = glade_xml_get_widget(note->glade, "title_box");
	GtkWidget *resize_box = glade_xml_get_widget(note->glade, "resize_box");
	GtkWidget *close_box = glade_xml_get_widget(note->glade, "close_box");

	GdkColor color;

	if (highlighted) {
		gchar *color_str = gconf_client_get_string(stickynotes->gconf_client, GCONF_PATH "/settings/title_color_prelight", NULL);
		gdk_color_parse(color_str, &color);
		g_free(color_str);

		gtk_widget_modify_bg(GTK_WIDGET(title_box), GTK_STATE_NORMAL, &color);
		gtk_widget_modify_bg(GTK_WIDGET(resize_box), GTK_STATE_NORMAL, &color);
		gtk_widget_modify_bg(GTK_WIDGET(close_box), GTK_STATE_NORMAL, &color);
		
		color_str = gconf_client_get_string(stickynotes->gconf_client, GCONF_PATH "/settings/body_color_prelight", NULL);
		gdk_color_parse(color_str, &color);
		g_free(color_str);
		
		gtk_widget_modify_base(GTK_WIDGET(note->body), GTK_STATE_NORMAL, &color);
	}
	
	else {
		gchar *color_str = gconf_client_get_string(stickynotes->gconf_client, GCONF_PATH "/settings/title_color", NULL);
		gdk_color_parse(color_str, &color);
		g_free(color_str);
		
		gtk_widget_modify_bg(GTK_WIDGET(title_box), GTK_STATE_NORMAL, &color);
		gtk_widget_modify_bg(GTK_WIDGET(resize_box), GTK_STATE_NORMAL, &color);
		gtk_widget_modify_bg(GTK_WIDGET(close_box), GTK_STATE_NORMAL, &color);
		
		color_str = gconf_client_get_string(stickynotes->gconf_client, GCONF_PATH "/settings/body_color", NULL);
		gdk_color_parse(color_str, &color);
		g_free(color_str);
		
		gtk_widget_modify_base(GTK_WIDGET(note->body), GTK_STATE_NORMAL, &color);
	}
}

/* Edit the sticky note title */
void stickynote_edit_title(StickyNote *note)
{
	GladeXML *glade = glade_xml_new(GLADE_PATH, "edit_title_dialog", NULL);
	GtkWidget *dialog = glade_xml_get_widget(glade, "edit_title_dialog");
	GtkWidget *entry = glade_xml_get_widget(glade, "title_entry");
	
	g_signal_connect(G_OBJECT(entry), "activate", G_CALLBACK(dialog_apply_cb), dialog);

	gtk_entry_set_text(GTK_ENTRY(entry), gtk_label_get_text(GTK_LABEL(note->title)));
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(note->window));

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
		gchar *title = g_strdup_printf("<b>%s</b>", gtk_entry_get_text(GTK_ENTRY(entry)));
		gtk_label_set_markup(GTK_LABEL(note->title), title);
		g_free(title);
	
		stickynotes_save_all();
	}

	gtk_widget_destroy(dialog);
	g_object_unref(glade);
}

/* Remove a sticky note with confirmation, if needed */
void stickynote_remove(StickyNote *note)
{
	GladeXML *glade = glade_xml_new(GLADE_PATH, "delete_dialog", NULL);
	GtkWidget *dialog = glade_xml_get_widget(glade, "delete_dialog");

	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(note->window));

	if (stickynote_get_empty(note) || gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
		stickynote_free(note);
		stickynotes_save_all();
	}

	gtk_widget_destroy(dialog);
	g_object_unref(glade);
}

/* Show all sticky notes */
void stickynotes_show_all()
{
	gint i;
	
	for (i = 0; i < g_list_length(stickynotes->notes); i++) {
		StickyNote *note = g_list_nth_data(stickynotes->notes, i);
		gtk_widget_show(note->window);
		gdk_window_raise(note->window->window);
	}

	stickynotes->hidden = FALSE;
}

/* Hide all sticky notes */
void stickynotes_hide_all()
{
	gint i;
	
	for (i = 0; i < g_list_length(stickynotes->notes); i++) {
		StickyNote *note = g_list_nth_data(stickynotes->notes, i);
		gtk_widget_hide(note->window);
	}

	stickynotes->hidden = TRUE;
}

/* Save all sticky notes in an XML configuration file */
void stickynotes_save_all()
{
	/* Create a new XML document */
	xmlDocPtr doc = xmlNewDoc("1.0");
	xmlNodePtr root = xmlNewDocNode(doc, NULL, "stickynotes", NULL);

	gint i;

	xmlDocSetRootElement(doc, root);
	xmlNewProp(root, "version", VERSION);
	
	/* For all sticky notes */
	for (i = 0; i < g_list_length(stickynotes->notes); i++) {
		/* Access the current note in the list */
		StickyNote *note = g_list_nth_data(stickynotes->notes, i);

		gchar *body;
		gchar *x_str, *y_str, *w_str, *h_str;

		/* Retrieve body contents of the note */
		{
			GtkTextBuffer *buffer;
			GtkTextIter start, end;
			
			buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(note->body));
			gtk_text_buffer_get_bounds(buffer, &start, &end);
			body = gtk_text_iter_get_text(&start, &end);
		}

		/* Retrieve the window size of the note */
		{
			gint w, h;

			gtk_window_get_size(GTK_WINDOW(note->window), &w, &h);
			w_str = g_strdup_printf("%d", w);
			h_str = g_strdup_printf("%d", h);
		}
		
		/* Retrieve the window position of the note */
		{
			gint x, y;
			
			gtk_window_get_position(GTK_WINDOW(note->window), &x, &y);
			x_str = g_strdup_printf("%d", x);
			y_str = g_strdup_printf("%d", y);
		}
		
		/* Save the note as a node in the XML document */
		{
			xmlNodePtr node = xmlNewTextChild(root, NULL, "note", body);		
			xmlNewProp(node, "title", gtk_label_get_text(GTK_LABEL(note->title)));
			xmlNewProp(node, "x", x_str);
			xmlNewProp(node, "y", y_str);
			xmlNewProp(node, "w", w_str);
			xmlNewProp(node, "h", h_str);
		}
	
		/* Now that it has been saved, reset the modified flag */
		{
			GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(note->body));
			gtk_text_buffer_set_modified(buffer, FALSE);
		}

		g_free(body);
		g_free(x_str);
		g_free(y_str);
		g_free(w_str);
		g_free(h_str);
	}
	
	/* The XML file is $HOME/.gnome2/stickystickynotes_applet, most probably */
	{
		gchar *file = g_strdup_printf("%s/.gnome2/%s", g_get_home_dir(), PACKAGE);
		xmlSaveFormatFile(file, doc, 1);
		g_free(file);
	}
	
	xmlFreeDoc(doc);
}

/* Load all sticky notes from an XML configuration file */
void stickynotes_load_all()
{
	xmlDocPtr doc;
	xmlNodePtr root;
	xmlNodePtr node;
	
	/* The XML file is $HOME/.gnome2/stickystickynotes_applet, most probably */
	{
		gchar *file = g_strdup_printf("%s/.gnome2/%s", g_get_home_dir(), PACKAGE);
		doc = xmlParseFile(file);
		g_free(file);
	}

	/* If the XML file does not exist, create a blank one */
	if (!doc) {
		stickynotes_save_all();
		return;
	}
	
	/* If the XML file is corrupted/incorrect, create a blank one */
	root = xmlDocGetRootElement(doc);
	if (!root || xmlStrcmp(root->name, (const xmlChar *) "stickynotes")) {
		xmlFreeDoc(doc);
		stickynotes_save_all();
		return;
	}
	
	node = root->xmlChildrenNode;
	
	/* For all children of the root node (ie all sticky notes) */
	while (node) {
		if (!xmlStrcmp(node->name, (const xmlChar *) "note")) {
			/* Create a new note */
			StickyNote *note = stickynote_new();

			/* Retrieve and set title of the note */
			{
				gchar *title0 = gconf_client_get_string(stickynotes->gconf_client, GCONF_PATH "/settings/date_format", NULL);
				gchar *title1 = xmlGetProp(node, "title");
				gchar *title2;
				if (!title1)
					title1 = get_current_date(title0);
				title2 = g_strdup_printf("<b>%s</b>", title1);
				gtk_label_set_markup(GTK_LABEL(note->title), title2);
				g_free(title0);
				g_free(title1);
				g_free(title2);
			}

			/* Retrieve and set the window size of the note */
			{
				gchar *w_str = xmlGetProp(node, "w");
				gchar *h_str = xmlGetProp(node, "h");
				if (w_str && h_str)
					gtk_window_resize(GTK_WINDOW(note->window), atoi(w_str), atoi(h_str));
				g_free(w_str);
				g_free(h_str);
			}
			
			/* Retrieve and set the window position of the note */
			{
				gchar *x_str = xmlGetProp(node, "x");
				gchar *y_str = xmlGetProp(node, "y");
				if (x_str && y_str)
					gtk_window_move(GTK_WINDOW(note->window), atoi(x_str), atoi(y_str));
				g_free(x_str);
				g_free(y_str);
			}

			/* Retrieve and set (if any) the body contents of the note */
			{
				gchar *body = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
				if (body) {
					GtkTextBuffer *buffer;
					GtkTextIter start, end;
					
					buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(note->body));
					gtk_text_buffer_get_bounds(buffer, &start, &end);
					gtk_text_buffer_insert(buffer, &start, body, -1);
				}
				g_free(body);
			}
		}
		
		node = node->next;
	}

	xmlFreeDoc(doc);
}
