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

/* Create a new (empty) Sticky Note */
StickyNote * stickynote_new()
{
	/* Create Sticky Note instance */
	StickyNote *note = g_new(StickyNote, 1);
	
	/* Create and initialize a Sticky Note */
	note->window = glade_xml_new(GLADE_PATH, "stickynote_window", NULL);
	note->menu = glade_xml_new(GLADE_PATH, "stickynote_menu", NULL);
	note->properties = glade_xml_new(GLADE_PATH, "stickynote_properties", NULL);
	note->w_window = glade_xml_get_widget(note->window, "stickynote_window");
	note->w_menu = glade_xml_get_widget(note->menu, "stickynote_menu");
	note->w_properties = glade_xml_get_widget(note->properties, "stickynote_properties");
	note->w_entry = glade_xml_get_widget(note->properties, "title_entry");
	note->w_color = glade_xml_get_widget(note->properties, "note_color");
	note->w_default = GTK_WIDGET(&GTK_CHECK_BUTTON(glade_xml_get_widget(note->properties, "use_default_check"))->toggle_button);
	note->w_title = glade_xml_get_widget(note->window, "title_label");
	note->w_body = glade_xml_get_widget(note->window, "body_text");
	note->w_lock = glade_xml_get_widget(note->window, "lock_button");
	note->w_close = glade_xml_get_widget(note->window, "close_button");
	note->w_resize_se = glade_xml_get_widget(note->window, "resize_se_box");
	note->w_resize_sw = glade_xml_get_widget(note->window, "resize_sw_box");
	note->color = NULL;
	note->locked = FALSE;
	note->visible = FALSE;
	note->x = 0;
	note->y = 0;
	note->w = 0;
	note->h = 0;

	/* Customize the window */
	gtk_window_set_decorated(GTK_WINDOW(note->w_window), FALSE);
	if (gconf_client_get_bool(stickynotes->gconf, GCONF_PATH "/settings/sticky", NULL))
		gtk_window_stick(GTK_WINDOW(note->w_window));
	gtk_window_resize(GTK_WINDOW(note->w_window), gconf_client_get_int(stickynotes->gconf, GCONF_PATH "/defaults/width", NULL),
						      gconf_client_get_int(stickynotes->gconf, GCONF_PATH "/defaults/height", NULL));

	/* Set the button images */
	gtk_image_set_from_file(GTK_IMAGE(glade_xml_get_widget(note->window, "close_img")), STICKYNOTES_ICONDIR "/close.png");
	gtk_image_set_from_file(GTK_IMAGE(glade_xml_get_widget(note->window, "resize_se_img")), STICKYNOTES_ICONDIR "/resize_se.png");
	gtk_image_set_from_file(GTK_IMAGE(glade_xml_get_widget(note->window, "resize_sw_img")), STICKYNOTES_ICONDIR "/resize_sw.png");
	
	/* Customize the title and colors, hide and unlock */
	stickynote_set_title(note, NULL);
	stickynote_set_color(note, NULL, TRUE);
	stickynote_set_locked(note, FALSE);
	stickynote_set_visible(note, FALSE);

	/* Connect a popup menu to all buttons and title */
	gnome_popup_menu_attach(note->w_menu, note->w_window, note);
	gnome_popup_menu_attach(note->w_menu, note->w_lock, note);
	gnome_popup_menu_attach(note->w_menu, note->w_close, note);
	gnome_popup_menu_attach(note->w_menu, note->w_resize_se, note);
	gnome_popup_menu_attach(note->w_menu, note->w_resize_sw, note);

	/* Connect a properties dialog to the note */
	gtk_window_set_transient_for(GTK_WINDOW(note->w_properties), GTK_WINDOW(note->w_window));
	
	/* Connect signals to the sticky note */

	g_signal_connect(G_OBJECT(note->w_lock), "clicked", G_CALLBACK(stickynote_toggle_lock_cb), note);
	g_signal_connect(G_OBJECT(note->w_close), "clicked", G_CALLBACK(stickynote_close_cb), note);
	g_signal_connect(G_OBJECT(note->w_resize_se), "button-press-event", G_CALLBACK(stickynote_resize_cb), note);
	g_signal_connect(G_OBJECT(note->w_resize_sw), "button-press-event", G_CALLBACK(stickynote_resize_cb), note);

	g_signal_connect(G_OBJECT(note->w_window), "button-press-event", G_CALLBACK(stickynote_move_cb), note);
	g_signal_connect(G_OBJECT(note->w_window), "expose-event", G_CALLBACK(stickynote_expose_cb), note);
	g_signal_connect(G_OBJECT(note->w_window), "configure-event", G_CALLBACK(stickynote_configure_cb), note);
	g_signal_connect(G_OBJECT(note->w_window), "delete-event", G_CALLBACK(stickynote_delete_cb), note);
	g_signal_connect(G_OBJECT(note->w_window), "enter-notify-event", G_CALLBACK(stickynote_cross_cb), note);
	g_signal_connect(G_OBJECT(note->w_window), "leave-notify-event", G_CALLBACK(stickynote_cross_cb), note);
	g_signal_connect(G_OBJECT(note->w_window), "focus-in-event", G_CALLBACK(stickynote_focus_cb), note);
	g_signal_connect(G_OBJECT(note->w_window), "focus-out-event", G_CALLBACK(stickynote_focus_cb), note);
	
	g_signal_connect(G_OBJECT(glade_xml_get_widget(note->menu, "popup_create")), "activate", G_CALLBACK(popup_create_cb), note);
	g_signal_connect(G_OBJECT(glade_xml_get_widget(note->menu, "popup_destroy")), "activate", G_CALLBACK(popup_destroy_cb), note);
	g_signal_connect(G_OBJECT(glade_xml_get_widget(note->menu, "popup_toggle_lock")), "toggled", G_CALLBACK(popup_toggle_lock_cb), note);
	g_signal_connect(G_OBJECT(glade_xml_get_widget(note->menu, "popup_properties")), "activate", G_CALLBACK(popup_properties_cb), note);

	g_signal_connect_swapped(G_OBJECT(note->w_entry), "changed", G_CALLBACK(properties_apply_title_cb), note);
	g_signal_connect(G_OBJECT(note->w_color), "color_set", G_CALLBACK(properties_color_cb), note);
	g_signal_connect_swapped(G_OBJECT(note->w_default), "toggled", G_CALLBACK(properties_apply_color_cb), note);
	g_signal_connect(G_OBJECT(note->w_entry), "activate", G_CALLBACK(properties_activate_cb), note);
	g_signal_connect(G_OBJECT(note->w_properties), "response", G_CALLBACK(properties_response_cb), note);
	
	return note;
}

/* Destroy a Sticky Note */
void stickynote_free(StickyNote *note)
{
	gtk_widget_destroy(note->w_properties);
	gtk_widget_destroy(note->w_menu);
	gtk_widget_destroy(note->w_window);
	
	g_object_unref(G_OBJECT(note->properties));
	g_object_unref(G_OBJECT(note->menu));
	g_object_unref(G_OBJECT(note->window));
	
	g_free(note->color);
	
	g_free(note);
}

/* Change the sticky note title and color */
void stickynote_change_properties(StickyNote *note)
{
	gtk_dialog_run(GTK_DIALOG(note->w_properties));

	stickynotes_save();
}

/* Check if a sticky note is empty */
gboolean stickynote_get_empty(const StickyNote *note)
{
	return gtk_text_buffer_get_char_count(gtk_text_view_get_buffer(GTK_TEXT_VIEW(note->w_body))) == 0;
}

/* Set the sticky note title */
void stickynote_set_title(StickyNote *note, const gchar *title)
{
	/* If title is NULL, use the current date as the title. */
	if (!title) {
		gchar *date_format = gconf_client_get_string(stickynotes->gconf, GCONF_PATH "/settings/date_format", NULL);
		title = get_current_date(date_format);
		g_free(date_format);
	}
		
	gtk_window_set_title(GTK_WINDOW(note->w_window), title);
	gtk_entry_set_text(GTK_ENTRY(note->w_entry), gtk_label_get_text(GTK_LABEL(note->w_title)));

	{
		gchar *bold_title = g_strdup_printf("<b>%s</b>", title);
		gtk_label_set_markup(GTK_LABEL(note->w_title), bold_title);
		g_free(bold_title);
	}
}

/* Set the sticky note color */
void stickynote_set_color(StickyNote *note, const gchar *color_str, gboolean save)
{
	GtkRcStyle *rc_style = gtk_rc_style_new();
	gchar *color_str_actual;

	if (save) {
		g_free(note->color);
		note->color = color_str ? g_strdup(color_str) : NULL;

		if (note->color) {
			GdkColor color;
			gdk_color_parse(note->color, &color);
			gnome_color_picker_set_i16(GNOME_COLOR_PICKER(note->w_color), color.red, color.green, color.blue, 65535);
		}

		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(note->w_default), note->color == NULL);
		gtk_widget_set_sensitive(glade_xml_get_widget(note->properties, "color_label"), note->color != NULL);
		gtk_widget_set_sensitive(note->w_color, note->color != NULL);
	}
	
	/* If "force_default_color" is enabled or color_str is NULL, then we use the default color instead of color_str. */
	if (!color_str || gconf_client_get_bool(stickynotes->gconf, GCONF_PATH "/settings/force_default_color", NULL)) {
		if (gconf_client_get_bool(stickynotes->gconf, GCONF_PATH "/settings/use_system_color", NULL))
			color_str_actual = NULL;
		else
			color_str_actual = gconf_client_get_string(stickynotes->gconf, GCONF_PATH "/defaults/color", NULL);
	}
	else
		color_str_actual = g_strdup(color_str);

	/* Do not use custom colors if "use_system_color" is enabled */
	if (color_str_actual != NULL) {
		/* Custom colors */
		GdkColor color[4];

		/* Make 4 shades of the color, getting darker from the original. */
		gint i;
		for (i = 0; i < 4; i++) {
			gdk_color_parse(color_str_actual, &color[i]);

			color[i].red = (color[i].red * (10 - i)) / 10;
			color[i].green = (color[i].green * (10 - i)) / 10;
			color[i].blue = (color[i].blue * (10 - i)) / 10;

			gdk_colormap_alloc_color(gtk_widget_get_colormap(note->w_window), &color[i], FALSE, TRUE);
		}

		/* Apply colors to style */
		rc_style->base[GTK_STATE_NORMAL] = color[0];
		rc_style->bg[GTK_STATE_PRELIGHT] = color[1];
		rc_style->bg[GTK_STATE_NORMAL] = color[2];
		rc_style->bg[GTK_STATE_ACTIVE] = color[3];

		rc_style->color_flags[GTK_STATE_PRELIGHT] = GTK_RC_BG;
		rc_style->color_flags[GTK_STATE_NORMAL] = GTK_RC_BG | GTK_RC_BASE;
		rc_style->color_flags[GTK_STATE_ACTIVE] = GTK_RC_BG;
	}

	/* Apply the style to the widgets */
	gtk_widget_modify_style(note->w_window, rc_style);
	gtk_widget_modify_style(note->w_title, rc_style);
	gtk_widget_modify_style(note->w_body, rc_style);
	gtk_widget_modify_style(note->w_lock, rc_style);
	gtk_widget_modify_style(note->w_close, rc_style);
	gtk_widget_modify_style(note->w_resize_se, rc_style);
	gtk_widget_modify_style(note->w_resize_sw, rc_style);

	g_free(color_str_actual);
	g_object_unref(G_OBJECT(rc_style));
}

/* Lock/Unlock a sticky note from editing */
void stickynote_set_locked(StickyNote *note, gboolean locked)
{
	note->locked = locked;

	/* Set cursor visibility and editability */
	gtk_text_view_set_editable(GTK_TEXT_VIEW(note->w_body), !locked);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(note->w_body), !locked);

	/* Show appropriate icon and tooltip */
	if (locked) {
		gtk_image_set_from_file(GTK_IMAGE(glade_xml_get_widget(note->window, "lock_img")), STICKYNOTES_ICONDIR "/lock.png");
		gtk_tooltips_set_tip(stickynotes->tooltips, note->w_lock, _("Locked note"), NULL);
	}
	else {
		gtk_image_set_from_file(GTK_IMAGE(glade_xml_get_widget(note->window, "lock_img")), STICKYNOTES_ICONDIR "/unlock.png");
		gtk_tooltips_set_tip(stickynotes->tooltips, note->w_lock, _("Unlocked note"), NULL);
	}

	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(note->menu, "popup_toggle_lock")), locked);
}

/* Show/Hide a sticky note */
void stickynote_set_visible(StickyNote *note, gboolean visible)
{
	note->visible = visible;

	if (visible) {
		/* Show & raise sticky note, then move to the corrected location on screen. */
		gtk_window_present(GTK_WINDOW(note->w_window));
		gtk_window_move(GTK_WINDOW(note->w_window), note->x, note->y);
	}
	else {
		/* Hide sticky note */
		gtk_widget_hide(note->w_window);
	}
}

/* Add a sticky note */
void stickynotes_add()
{
	StickyNote *note = stickynote_new();

	/* Add the note to the linked-list of all notes */
	stickynotes->notes = g_list_append(stickynotes->notes, note);
	
	/* Update tooltips */
	stickynotes_applet_update_tooltips();

	/* Save notes */
	stickynotes_save();

	/* Show all sticky notes */
	stickynote_set_visible(note, TRUE);
	gconf_client_set_bool(stickynotes->gconf, GCONF_PATH "/settings/visible", TRUE, NULL);
}

/* Remove a sticky note with confirmation, if needed */
void stickynotes_remove(StickyNote *note)
{
	GladeXML *glade = glade_xml_new(GLADE_PATH, "delete_dialog", NULL);
	GtkWidget *dialog = glade_xml_get_widget(glade, "delete_dialog");

	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(note->w_window));

	if (stickynote_get_empty(note) ||
	    !gconf_client_get_bool(stickynotes->gconf, GCONF_PATH "/settings/confirm_deletion", NULL) ||
	    gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
		stickynote_free(note);

		/* Remove the note from the linked-list of all notes */
		stickynotes->notes = g_list_remove(stickynotes->notes, note);
		
		/* Update tooltips */
		stickynotes_applet_update_tooltips();

		/* Save notes */
		stickynotes_save();
	}

	gtk_widget_destroy(dialog);
	g_object_unref(glade);
}

/* Save all sticky notes in an XML configuration file */
void stickynotes_save()
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
		const gchar *title = gtk_label_get_text(GTK_LABEL(note->w_title));

		/* Retrieve body contents of the note */
		GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(note->w_body));
		
		GtkTextIter start, end;
		gchar *body;
		
		gtk_text_buffer_get_bounds(buffer, &start, &end);
		body = gtk_text_iter_get_text(&start, &end);

		/* Save the note as a node in the XML document */
		{
			xmlNodePtr node = xmlNewTextChild(root, NULL, "note", body);		
			xmlNewProp(node, "title", title);
			if (note->color)
				xmlNewProp(node, "color", note->color);
			if (note->visible)
				xmlNewProp(node, "visible", "true");
			if (note->locked)
				xmlNewProp(node, "locked", "true");
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
	
	/* The XML file is $HOME/.gnome2/stickynotes_applet, most probably */
	{
		gchar *file = g_strdup_printf("%s/.gnome2/%s", g_get_home_dir(), XML_PATH);
		xmlSaveFormatFile(file, doc, 1);
		g_free(file);
	}
	
	xmlFreeDoc(doc);
}

/* Load all sticky notes from an XML configuration file */
void stickynotes_load()
{
	xmlDocPtr doc;
	xmlNodePtr root;
	xmlNodePtr node;
	
	/* The XML file is $HOME/.gnome2/stickynotes_applet, most probably */
	{
		gchar *file = g_strdup_printf("%s/.gnome2/%s", g_get_home_dir(), XML_PATH);
		doc = xmlParseFile(file);
		g_free(file);
	}

	/* If the XML file does not exist, create a blank one */
	if (!doc) {
		stickynotes_save();
		return;
	}
	
	/* If the XML file is corrupted/incorrect, create a blank one */
	root = xmlDocGetRootElement(doc);
	if (!root || xmlStrcmp(root->name, (const xmlChar *) "stickynotes")) {
		xmlFreeDoc(doc);
		stickynotes_save();
		return;
	}
	
	node = root->xmlChildrenNode;
	
	/* For all children of the root node (ie all sticky notes) */
	while (node) {
		if (!xmlStrcmp(node->name, (const xmlChar *) "note")) {
			/* Create a new note */
			StickyNote *note = stickynote_new();
			stickynotes->notes = g_list_append(stickynotes->notes, note);

			/* Retrieve and set title of the note */
			{
				gchar *title = xmlGetProp(node, "title");
				if (title)
					stickynote_set_title(note, title);
				g_free(title);
			}

			/* Retrieve and set the color of the note */
			{
				gchar *color_str = xmlGetProp(node, "color");
				if (color_str)
					stickynote_set_color(note, color_str, TRUE);
				g_free(color_str);
			}

			/* Retrieve and set the window size of the note */
			{
				gchar *w_str = xmlGetProp(node, "w");
				gchar *h_str = xmlGetProp(node, "h");
				if (w_str && h_str)
					gtk_window_resize(GTK_WINDOW(note->w_window), atoi(w_str), atoi(h_str));
				gtk_window_get_size(GTK_WINDOW(note->w_window), &note->w, &note->h);
				g_free(w_str);
				g_free(h_str);
			}
			
			/* Retrieve and set the window position of the note */
			{
				gchar *x_str = xmlGetProp(node, "x");
				gchar *y_str = xmlGetProp(node, "y");
				if (x_str && y_str)
					gtk_window_move(GTK_WINDOW(note->w_window), atoi(x_str), atoi(y_str));
				gtk_window_get_position(GTK_WINDOW(note->w_window), &note->x, &note->y);
				g_free(x_str);
				g_free(y_str);
			}

			/* Retrieve and set (if any) the body contents of the note */
			{
				gchar *body = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
				if (body) {
					GtkTextBuffer *buffer;
					GtkTextIter start, end;
					
					buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(note->w_body));
					gtk_text_buffer_get_bounds(buffer, &start, &end);
					gtk_text_buffer_insert(buffer, &start, body, -1);
				}
				g_free(body);
			}

			/* Retrieve and set the locked state of the note, by default unlocked */
			{
				gchar *locked = xmlGetProp(node, "locked");
				if (locked)
					stickynote_set_locked(note, strcmp(locked, "true") == 0);
				g_free(locked);
			}

			/* Retrieve and set the visibility of the note, be default invisible */
			{
				gchar *visible = xmlGetProp(node, "visible");
				if (visible)
					stickynote_set_visible(note, strcmp(visible, "true") == 0);
				g_free(visible);
			}
		}
		
		node = node->next;
	}

	xmlFreeDoc(doc);
}
