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
#include <stickynotes.h>
#include <stickynotes_callbacks.h>
#include <util.h>

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
StickyNote * stickynote_new(StickyNotesApplet *stickynotes)
{
	/* Sticky Note instance */
	StickyNote *note;
	
	/* Create and initialize a Sticky Note */
	note = g_new(StickyNote, 1);
	note->glade = glade_xml_new(GLADE_PATH, "stickynote_window", NULL);
	note->window = glade_xml_get_widget(note->glade, "stickynote_window");
	note->title = glade_xml_get_widget(note->glade, "title_label");
	note->body = glade_xml_get_widget(note->glade, "body_text");
	note->x = 0;
	note->y = 0;
	note->w = 0;
	note->h = 0;
	note->stickynotes = stickynotes;

	/* Customize the window */
	gtk_window_set_decorated(GTK_WINDOW(note->window), FALSE);
	if (gconf_client_get_bool(stickynotes->gconf_client, GCONF_PATH "/settings/sticky", NULL))
		gtk_window_stick(GTK_WINDOW(note->window));
	gtk_window_resize(GTK_WINDOW(note->window), gconf_client_get_int(stickynotes->gconf_client, GCONF_PATH "/defaults/width", NULL),
						    gconf_client_get_int(stickynotes->gconf_client, GCONF_PATH "/defaults/height", NULL));
	
	gtk_image_set_from_file(GTK_IMAGE(glade_xml_get_widget(note->glade, "resize_img")), STICKYNOTES_ICONDIR "/resize.png");
	gtk_image_set_from_file(GTK_IMAGE(glade_xml_get_widget(note->glade, "close_img")), STICKYNOTES_ICONDIR "/close.png");
	
	/* Customize the date in the title label */
	stickynote_set_title(note, NULL);
	
	/* Customize the colors */
	stickynote_set_color(note, gconf_client_get_string(stickynotes->gconf_client, GCONF_PATH "/defaults/color", NULL), FALSE);

	/* Connect signals to the sticky note window */
	g_signal_connect(G_OBJECT(note->window), "expose-event", G_CALLBACK(window_expose_cb), note);
	g_signal_connect(G_OBJECT(note->window), "configure-event", G_CALLBACK(window_configure_cb), note);
	g_signal_connect(G_OBJECT(note->window), "delete-event", G_CALLBACK(window_delete_cb), note);
	g_signal_connect(G_OBJECT(note->window), "enter-notify-event", G_CALLBACK(window_cross_cb), note);
	g_signal_connect(G_OBJECT(note->window), "leave-notify-event", G_CALLBACK(window_cross_cb), note);
	g_signal_connect(G_OBJECT(note->window), "focus-in-event", G_CALLBACK(window_focus_cb), note);
	g_signal_connect(G_OBJECT(note->window), "focus-out-event", G_CALLBACK(window_focus_cb), note);
	g_signal_connect(G_OBJECT(note->window), "button-press-event", G_CALLBACK(window_move_cb), note);
	
	g_signal_connect(G_OBJECT(glade_xml_get_widget(note->glade, "resize_button")), "button-press-event", G_CALLBACK(window_resize_cb), note);
	g_signal_connect(G_OBJECT(glade_xml_get_widget(note->glade, "close_button")), "clicked", G_CALLBACK(window_close_cb), note);

	/* Connect a popup menu to the buttons and title */
	gnome_popup_menu_attach(gnome_popup_menu_new(popup_menu), note->window, note);
	gnome_popup_menu_attach(gnome_popup_menu_new(popup_menu), glade_xml_get_widget(note->glade, "resize_button"), note);
	gnome_popup_menu_attach(gnome_popup_menu_new(popup_menu), glade_xml_get_widget(note->glade, "close_button"), note);

	return note;
}

/* Destroy a Sticky Note */
void stickynote_free(StickyNote *note)
{
	gtk_widget_destroy(note->window);
	g_object_unref(note->glade);
	g_free(note);
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
		stickynote_set_title(note, gtk_entry_get_text(GTK_ENTRY(entry)));
		stickynotes_save(note->stickynotes);
	}

	gtk_widget_destroy(dialog);
	g_object_unref(glade);
}

/* Check if a sticky note is empty */
gboolean stickynote_get_empty(const StickyNote *note)
{
	return gtk_text_buffer_get_char_count(gtk_text_view_get_buffer(GTK_TEXT_VIEW(note->body))) == 0;
}

/* (Un)highlight a sticky note */
void stickynote_set_color(StickyNote *note, const gchar *color_str, gboolean highlighted)
{
	GtkStyle *style = gtk_style_new();
	
	if (color_str) {
		gint i;
		for (i = 0; i < 4; i++) {
			gdk_color_parse(color_str, &note->color[i]);

			note->color[i].red = (note->color[i].red * (10 - i)) / 10;
			note->color[i].green = (note->color[i].green * (10 - i)) / 10;
			note->color[i].blue = (note->color[i].blue * (10 - i)) / 10;

			gdk_colormap_alloc_color(gtk_widget_get_colormap(note->window), &note->color[i], FALSE, TRUE);
		}
	}

	style->base[GTK_STATE_NORMAL] = note->color[highlighted ? 0 : 1];
	style->bg[GTK_STATE_NORMAL] = note->color[2];
	style->bg[GTK_STATE_ACTIVE] = note->color[3];
	style->bg[GTK_STATE_PRELIGHT] = note->color[1];

	gtk_widget_set_style(note->window, style);
	gtk_widget_set_style(note->body, style);
	gtk_widget_set_style(glade_xml_get_widget(note->glade, "resize_button"), style);
	gtk_widget_set_style(glade_xml_get_widget(note->glade, "close_button"), style);

	g_object_unref(G_OBJECT(style));
}

/* Set the sticky note title */
void stickynote_set_title(StickyNote *note, const gchar *title)
{
	if (!title) {
		gchar *date_format = gconf_client_get_string(note->stickynotes->gconf_client, GCONF_PATH "/settings/date_format", NULL);
		title = get_current_date(date_format);
		g_free(date_format);
	}
		
	gtk_window_set_title(GTK_WINDOW(note->window), title);

	{
		gchar *bold_title = g_strdup_printf("<b>%s</b>", title);
		gtk_label_set_markup(GTK_LABEL(note->title), bold_title);
		g_free(bold_title);
	}
}

/* Add a sticky notes */
void stickynotes_add(StickyNotesApplet *stickynotes)
{
	StickyNote *note = stickynote_new(stickynotes);

	gtk_widget_show(note->window);

	/* Add the note to the linked-list of all notes */
	stickynotes->notes = g_list_append(stickynotes->notes, note);
	
	/* Update tooltips */
	stickynotes_applet_update_tooltips(stickynotes);

	stickynotes_save(stickynotes);

	/* Unhide  all sticky notes */
	gconf_client_set_bool(note->stickynotes->gconf_client, GCONF_PATH "/settings/visible", TRUE, NULL);
	/* Unlock all sticky notes */
	gconf_client_set_bool(note->stickynotes->gconf_client, GCONF_PATH "/settings/locked", FALSE, NULL);
}

/* Remove a sticky note with confirmation, if needed */
void stickynotes_remove(StickyNotesApplet *stickynotes, StickyNote *note)
{
	GladeXML *glade = glade_xml_new(GLADE_PATH, "delete_dialog", NULL);
	GtkWidget *dialog = glade_xml_get_widget(glade, "delete_dialog");

	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(note->window));

	if (stickynote_get_empty(note) || gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
		stickynote_free(note);

		/* Remove the note from the linked-list of all notes */
		stickynotes->notes = g_list_remove(stickynotes->notes, note);
		
		/* Update tooltips */
		stickynotes_applet_update_tooltips(stickynotes);

		stickynotes_save(stickynotes);
	}

	gtk_widget_destroy(dialog);
	g_object_unref(glade);
}

/* Show/Hide all sticky notes */
void stickynotes_set_visible(StickyNotesApplet *stickynotes, gboolean visible)
{
	gint i;
	
	if (visible) 
		/* Show each sticky note, move to the corrected location on screen, and raise */
		for (i = 0; i < g_list_length(stickynotes->notes); i++) {
			StickyNote *note = g_list_nth_data(stickynotes->notes, i);
			gtk_widget_show(note->window);
			gtk_window_move(GTK_WINDOW(note->window), note->x, note->y);
			gdk_window_raise(note->window->window);
		}
	
	else
		/* Hide each sticky note */
		for (i = 0; i < g_list_length(stickynotes->notes); i++) {
			StickyNote *note = g_list_nth_data(stickynotes->notes, i);
			gtk_widget_hide(note->window);
		}
}

/* Lock/Unlock all sticky notes from editing */
void stickynotes_set_locked(StickyNotesApplet *stickynotes, gboolean locked)
{
	gint i;
	
	for (i = 0; i < g_list_length(stickynotes->notes); i++) {
		StickyNote *note = g_list_nth_data(stickynotes->notes, i);
		gtk_text_view_set_editable(GTK_TEXT_VIEW(note->body), !locked);
		gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(note->body), !locked);
	}
}

/* Save all sticky notes in an XML configuration file */
void stickynotes_save(StickyNotesApplet *stickynotes)
{
	gint i;

	/* Create a new XML document */
	xmlDocPtr doc = xmlNewDoc("1.0");
	xmlNodePtr root = xmlNewDocNode(doc, NULL, "stickynotes", NULL);

	xmlDocSetRootElement(doc, root);
	xmlNewProp(root, "version", VERSION);
	
	/* For all sticky notes */
	for (i = 0; i < g_list_length(stickynotes->notes); i++) {
		/* Access the current note in the list */
		StickyNote *note = g_list_nth_data(stickynotes->notes, i);

		/* Retrieve the window size of the note */
		gchar *w_str = g_strdup_printf("%d", note->w);
		gchar *h_str = g_strdup_printf("%d", note->h);
		
		/* Retrieve the window position of the note */
		gchar *x_str = g_strdup_printf("%d", note->x);
		gchar *y_str = g_strdup_printf("%d", note->y);
		
		/* Retreive the title of the note */
		const gchar *title = gtk_label_get_text(GTK_LABEL(note->title));

		/* Retrieve body contents of the note */
		GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(note->body));
		
		GtkTextIter start, end;
		gchar *body;
		
		gtk_text_buffer_get_bounds(buffer, &start, &end);
		body = gtk_text_iter_get_text(&start, &end);

		/* Save the note as a node in the XML document */
		{
			xmlNodePtr node = xmlNewTextChild(root, NULL, "note", body);		
			xmlNewProp(node, "title", title);
			xmlNewProp(node, "x", x_str);
			xmlNewProp(node, "y", y_str);
			xmlNewProp(node, "w", w_str);
			xmlNewProp(node, "h", h_str);
		}

		/* Now that it has been saved, reset the modified flag */
		gtk_text_buffer_set_modified(buffer, FALSE);
	
		g_free(x_str);
		g_free(y_str);
		g_free(w_str);
		g_free(h_str);
		g_free(body);
	}
	
	/* The XML file is $HOME/.gnome2/stickystickynotes_applet, most probably */
	{
		gchar *file = g_strdup_printf("%s/.gnome2/%s", g_get_home_dir(), XML_PATH);
		xmlSaveFormatFile(file, doc, 1);
		g_free(file);
	}
	
	xmlFreeDoc(doc);
}

/* Load all sticky notes from an XML configuration file */
void stickynotes_load(StickyNotesApplet *stickynotes)
{
	xmlDocPtr doc;
	xmlNodePtr root;
	xmlNodePtr node;
	
	/* The XML file is $HOME/.gnome2/stickystickynotes_applet, most probably */
	{
		gchar *file = g_strdup_printf("%s/.gnome2/%s", g_get_home_dir(), XML_PATH);
		doc = xmlParseFile(file);
		g_free(file);
	}

	/* If the XML file does not exist, create a blank one */
	if (!doc) {
		stickynotes_save(stickynotes);
		return;
	}
	
	/* If the XML file is corrupted/incorrect, create a blank one */
	root = xmlDocGetRootElement(doc);
	if (!root || xmlStrcmp(root->name, (const xmlChar *) "stickynotes")) {
		xmlFreeDoc(doc);
		stickynotes_save(stickynotes);
		return;
	}
	
	node = root->xmlChildrenNode;
	
	/* For all children of the root node (ie all sticky notes) */
	while (node) {
		if (!xmlStrcmp(node->name, (const xmlChar *) "note")) {
			/* Create a new note */
			StickyNote *note = stickynote_new(stickynotes);
			stickynotes->notes = g_list_append(stickynotes->notes, note);

			/* Retrieve and set the window size of the note */
			{
				gchar *w_str = xmlGetProp(node, "w");
				gchar *h_str = xmlGetProp(node, "h");
				if (w_str && h_str)
					gtk_window_resize(GTK_WINDOW(note->window), atoi(w_str), atoi(h_str));
				gtk_window_get_size(GTK_WINDOW(note->window), &note->w, &note->h);
				g_free(w_str);
				g_free(h_str);
			}
			
			/* Retrieve and set the window position of the note */
			{
				gchar *x_str = xmlGetProp(node, "x");
				gchar *y_str = xmlGetProp(node, "y");
				if (x_str && y_str)
					gtk_window_move(GTK_WINDOW(note->window), atoi(x_str), atoi(y_str));
				gtk_window_get_position(GTK_WINDOW(note->window), &note->x, &note->y);
				g_free(x_str);
				g_free(y_str);
			}

			/* Retrieve and set title of the note */
			{
				gchar *title = xmlGetProp(node, "title");
				stickynote_set_title(note, title);
				g_free(title);
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

	/* Update tooltips */
	stickynotes_applet_update_tooltips(stickynotes);
}
