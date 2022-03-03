/*
 * Copyright (C) 2010 Andrej Belcijan
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *     Andrej Belcijan <{andrejx} at {gmail.com}>
 */

#include "theme.h"

void loadThemeComboBox(GtkComboBox *, gchar *);
void loadThemeButtons(GtkWidget ***, GdkPixbuf ***, gchar ***);
const gchar* getButtonImageState(int, const gchar*);
const gchar* getButtonImageState4(int);
const gchar* getButtonImageName(int);
GdkPixbuf ***getPixbufs(gchar ***);
gchar ***getImages(gchar *);

static gchar *
fixThemeName (gchar *theme_name)
{
  gchar prev;
  gint len = strlen(theme_name);
  gint i;

  if (len > 1)
    {
      prev = '-';

      for (i = 0; i < len; i++)
        {
          if (prev == '-')
            theme_name[i] = g_ascii_toupper (theme_name[i]);

          prev = theme_name[i];
        }

      return theme_name;
    }
  else if (len == 1)
    {
      return g_ascii_strup (theme_name, 1);
    }

  return NULL;
}

const gchar* getButtonImageName(int button_id) {
	switch (button_id) {
		case WB_IMAGE_MINIMIZE: return BTN_NAME_MINIMIZE;
		case WB_IMAGE_UNMAXIMIZE: return BTN_NAME_UNMAXIMIZE;
		case WB_IMAGE_MAXIMIZE: return BTN_NAME_MAXIMIZE;
		case WB_IMAGE_CLOSE: return BTN_NAME_CLOSE;
		default: return NULL;
	}
}

const gchar* getButtonImageState(int state_id, const gchar* separator) { // new 6-state mode
	switch (state_id) {
		case WB_IMAGE_FOCUSED_NORMAL:		return g_strconcat(BTN_STATE_FOCUSED,separator,BTN_STATE_NORMAL,NULL);
		case WB_IMAGE_FOCUSED_CLICKED:		return g_strconcat(BTN_STATE_FOCUSED,separator,BTN_STATE_CLICKED,NULL);
		case WB_IMAGE_FOCUSED_HOVERED:		return g_strconcat(BTN_STATE_FOCUSED,separator,BTN_STATE_HOVERED,NULL);
		case WB_IMAGE_UNFOCUSED_NORMAL:		return g_strconcat(BTN_STATE_UNFOCUSED,separator,BTN_STATE_NORMAL,NULL);
		case WB_IMAGE_UNFOCUSED_CLICKED:	return g_strconcat(BTN_STATE_UNFOCUSED,separator,BTN_STATE_CLICKED,NULL);
		case WB_IMAGE_UNFOCUSED_HOVERED:	return g_strconcat(BTN_STATE_UNFOCUSED,separator,BTN_STATE_HOVERED,NULL);
		default: return g_strconcat(BTN_STATE_UNFOCUSED,separator,BTN_STATE_NORMAL,NULL);
	}
}
const gchar* getButtonImageState4(int state_id) { // old 4-state mode for backwards compatibility
	switch (state_id) {
		case WB_IMAGE_FOCUSED_NORMAL:		return BTN_STATE_FOCUSED;
		case WB_IMAGE_FOCUSED_CLICKED:		return BTN_STATE_CLICKED;
		case WB_IMAGE_FOCUSED_HOVERED:		return BTN_STATE_HOVERED;
		case WB_IMAGE_UNFOCUSED_NORMAL:		return BTN_STATE_UNFOCUSED;
		case WB_IMAGE_UNFOCUSED_CLICKED:	return BTN_STATE_CLICKED;
		case WB_IMAGE_UNFOCUSED_HOVERED:	return BTN_STATE_HOVERED;
		default: return BTN_STATE_NORMAL;
	}
}

void
loadThemeComboBox (GtkComboBox *combo,
                   gchar       *active_theme)
{
	GError *error;
	GDir *dir_themes;
	gint active;
	gint N_THEMES;
	GtkListStore *store;
	const gchar *curtheme;
	GtkTreeIter iter;
	GtkCellRenderer	*cell;

	error = NULL;
	dir_themes = g_dir_open(PATH_THEMES, 0, &error);

	if (error) {
		g_printerr ("g_dir_open(%s) failed - %s\n", PATH_THEMES, error->message);
		g_error_free(error);
		return;
	}

	active = -1;
	N_THEMES = 0;

	// (0=real_name, 1=display_name, 2=id)
	store = gtk_list_store_new (3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT);

	while ((curtheme = g_dir_read_name(dir_themes))) { //TODO: do this in a separate function
		if ( g_strcmp0(
		    	g_ascii_strdown(curtheme,-1),
		    	g_ascii_strdown(active_theme,-1)
		    ) == 0 )
		{
			active = N_THEMES;
		}

		gtk_list_store_append( store, &iter );
		gtk_list_store_set( store, &iter,
		    	0, curtheme,
		        1, fixThemeName(g_strdup(curtheme)),
				2, 1+N_THEMES++,
				-1 );
	}

	g_dir_close (dir_themes);

	if (active<0) active = N_THEMES;

    gtk_list_store_append( store, &iter );
    gtk_list_store_set( store, &iter, 0,"custom", 1,"Custom", 2,0, -1 );

	gtk_combo_box_set_model( combo, GTK_TREE_MODEL(store) );
    g_object_unref( G_OBJECT( store ) );

	cell = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start( GTK_CELL_LAYOUT( combo ), cell, TRUE );
    gtk_cell_layout_set_attributes( GTK_CELL_LAYOUT( combo ), cell, "text",1, NULL );

	gtk_combo_box_set_active(combo, active);
}

/* Load theme pixbufs into buttons */
void loadThemeButtons(GtkWidget ***button, GdkPixbuf ***pixbufs, gchar ***images) {
	gint i,j;
	for (i=0; i<WB_IMAGE_STATES; i++) {
		for (j=0; j<WB_IMAGES; j++) {
			gtk_button_set_image(GTK_BUTTON(button[i][j]), gtk_image_new_from_pixbuf(pixbufs[i][j]));
			gtk_widget_set_tooltip_text(button[i][j], images[i][j]);
		}
	}
}

/* Returns absolute image paths and makes sure they exist */
//TODO: different extensions
gchar ***getImages(gchar *location) {
	gint i,j;
	gchar ***images = g_new(gchar**, WB_IMAGE_STATES);
	for (i=0; i<WB_IMAGE_STATES; i++) {
		images[i] = g_new(gchar*, WB_IMAGES);
		for (j=0; j<WB_IMAGES; j++) {
			images[i][j] = g_strconcat(location, getButtonImageName(j), "-", getButtonImageState(i,"-"), ".", THEME_EXTENSION, NULL);
			if (!g_file_test(images[i][j], G_FILE_TEST_EXISTS | ~G_FILE_TEST_IS_DIR)) {
				images[i][j] = g_strconcat(location, getButtonImageName(j), "-", getButtonImageState4(i), ".", THEME_EXTENSION, NULL);
			}
		}
	}
	return images;
}

/* Loads pixbufs from image paths */
GdkPixbuf ***getPixbufs(gchar ***images) {
	gint i,j;
	GdkPixbuf ***pixbufs = g_new(GdkPixbuf**, WB_IMAGE_STATES);
	for (i=0; i<WB_IMAGE_STATES; i++) {
		pixbufs[i] = g_new(GdkPixbuf*, WB_IMAGES);
		for (j=0; j<WB_IMAGES; j++) {
			GError *error = NULL;
			pixbufs[i][j] = gdk_pixbuf_new_from_file(images[i][j], &error);
			if (error) {
				printf("Error loading image \"%s\": %s\n", images[i][j], error->message);
				continue;
			}
		}
	}
	return pixbufs;
}
