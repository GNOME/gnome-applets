/*
 * Copyright (C) 1999 Dave Camp <dave@davec.dhs.org>
 *  
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *  
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *  
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *  
 */

#include <config.h>
#include <gnome.h>
#include <string.h>
#include <dirent.h>
#include <limits.h>
#include <ctype.h>

#include <panel-applet-gconf.h>
#include <egg-screen-help.h>
#include "geyes.h"

gchar *theme_directories[] = {
        GEYES_THEMES_DIR,
        "~/.gnome/geyes-themes/"
};
#define NUM_THEME_DIRECTORIES 2

static void
parse_theme_file (EyesApplet *eyes_applet, FILE *theme_file)
{
        gchar line_buf [512]; /* prolly overkill */
        gchar *token;
        fgets (line_buf, 512, theme_file);
        while (!feof (theme_file)) {
                token = strtok (line_buf, "=");
                if (strncmp (token, "wall-thickness", 
                             strlen ("wall-thickness")) == 0) {
                        token += strlen ("wall-thickness");
                        while (!isdigit (*token)) {
                                token++;
                        }
                        sscanf (token, "%d", &eyes_applet->wall_thickness); 
                } else if (strncmp (token, "num-eyes", strlen ("num-eyes")) == 0) {
                        token += strlen ("num-eyes");
                        while (!isdigit (*token)) {
                                token++;
                        }
                        sscanf (token, "%d", &eyes_applet->num_eyes);
                } else if (strncmp (token, "eye-pixmap", strlen ("eye-pixmap")) == 0) {
                        token = strtok (NULL, "\"");
                        token = strtok (NULL, "\"");          
                        if (eyes_applet->eye_filename != NULL) 
                                g_free (eyes_applet->eye_filename);
                        eyes_applet->eye_filename = g_strdup_printf ("%s%s",
                                                                    eyes_applet->theme_dir,
                                                                    token);
                } else if (strncmp (token, "pupil-pixmap", strlen ("pupil-pixmap")) == 0) {
                        token = strtok (NULL, "\"");
                        token = strtok (NULL, "\"");      
            if (eyes_applet->pupil_filename != NULL) 
                    g_free (eyes_applet->pupil_filename);
            eyes_applet->pupil_filename 
                    = g_strdup_printf ("%s%s",
                                       eyes_applet->theme_dir,
                                       token);   
                }
                fgets (line_buf, 512, theme_file);
        }       
}

void
load_theme (EyesApplet *eyes_applet, const gchar *theme_dir)
{
	FILE* theme_file;
        gchar *file_name;

        eyes_applet->theme_dir = g_strdup_printf ("%s/", theme_dir);

        file_name = g_strdup_printf("%s%s",theme_dir,"/config");
        theme_file = fopen (file_name, "r");
        if (theme_file == NULL) {
                g_error ("Unable to open theme file.");
        }
        
        parse_theme_file (eyes_applet, theme_file);
        fclose (theme_file);

        eyes_applet->theme_name = g_strdup (theme_dir);
       
        if (eyes_applet->eye_image)
        	g_object_unref (eyes_applet->eye_image);
        eyes_applet->eye_image = gdk_pixbuf_new_from_file (eyes_applet->eye_filename, NULL);
        if (eyes_applet->pupil_image)
        	g_object_unref (eyes_applet->pupil_image);
        eyes_applet->pupil_image = gdk_pixbuf_new_from_file (eyes_applet->pupil_filename, NULL);

	eyes_applet->eye_height = gdk_pixbuf_get_height (eyes_applet->eye_image);
        eyes_applet->eye_width = gdk_pixbuf_get_width (eyes_applet->eye_image);
        eyes_applet->pupil_height = gdk_pixbuf_get_height (eyes_applet->pupil_image);
        eyes_applet->pupil_width = gdk_pixbuf_get_width (eyes_applet->pupil_image);
        
        g_free (file_name);
        
}

static void
destroy_theme (EyesApplet *eyes_applet)
{
	/* Dunno about this - to unref or not to unref? */
	if (eyes_applet->eye_image != NULL) {
        	gdk_pixbuf_unref (eyes_applet->eye_image); 
        	eyes_applet->eye_image = NULL;
        }
        if (eyes_applet->pupil_image != NULL) {
        	gdk_pixbuf_unref (eyes_applet->pupil_image); 
        	eyes_applet->pupil_image = NULL;
	}
	
        g_free (eyes_applet->theme_dir);
        g_free (eyes_applet->theme_name);
}

static void
theme_selected_cb (GtkTreeSelection *selection, gpointer data)
{
	EyesApplet *eyes_applet = data;
	GtkTreeModel *model;
	GtkTreeIter iter;
	gchar *theme;
	gchar *theme_dir;
	
	if (!gtk_tree_selection_get_selected (selection, &model, &iter))
		return;
	
	gtk_tree_model_get (model, &iter, 0, &theme, -1);
	
	g_return_if_fail (theme);
	
	theme_dir = g_strdup_printf ("%s/", theme);
	if (!g_strcasecmp (theme_dir, eyes_applet->theme_dir)) {
		g_free (theme_dir);
		return;
	}
	g_free (theme_dir);
		
	destroy_eyes (eyes_applet);
        destroy_theme (eyes_applet);
        load_theme (eyes_applet, theme);
        setup_eyes (eyes_applet);
	
	panel_applet_gconf_set_string (
		eyes_applet->applet, "theme_path", theme, NULL);
	
	g_free (theme);
}

static void
phelp_cb (GtkDialog *dialog)
{
	GError *error = NULL;

	egg_screen_help_display (
		gtk_window_get_screen (GTK_WINDOW (dialog)),
		"geyes", "geyes-settings", &error);

	if (error) { /* FIXME: the user needs to see this */
		g_warning ("help error: %s\n", error->message);
		g_error_free (error);
		error = NULL;
	}
}

static void
presponse_cb (GtkDialog *dialog, gint id, gpointer data)
{
	EyesApplet *eyes_applet = data;
	if(id == GTK_RESPONSE_HELP){
		phelp_cb (dialog);
		return;
	}


	gtk_widget_destroy (GTK_WIDGET (dialog));

	eyes_applet->prop_box.pbox = NULL;
}

void
properties_cb (BonoboUIComponent *uic,
	       EyesApplet        *eyes_applet,
	       const gchar       *verbname)
{
	GtkWidget *pbox, *hbox;
        GtkWidget *tree;
        GtkWidget *label;
        GtkListStore *model;
        GtkTreeViewColumn *column;
        GtkCellRenderer *cell;
        GtkTreeSelection *selection;
        GtkTreeIter iter;
        DIR *dfd;
        struct dirent *dp;
        int i;
        gchar filename [PATH_MAX];
     
	if (eyes_applet->prop_box.pbox) {
		gtk_window_set_screen (
			GTK_WINDOW (eyes_applet->prop_box.pbox),
			gtk_widget_get_screen (GTK_WIDGET (eyes_applet->applet)));
		gtk_window_present (GTK_WINDOW (eyes_applet->prop_box.pbox));
		return;
	}

        pbox = gtk_dialog_new_with_buttons (_("Geyes Preferences"), NULL,
        				     GTK_DIALOG_DESTROY_WITH_PARENT,
					     GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
					     GTK_STOCK_HELP, GTK_RESPONSE_HELP,
					     NULL);
	gtk_window_set_screen (GTK_WINDOW (pbox),
			       gtk_widget_get_screen (GTK_WIDGET (eyes_applet->applet)));
        gtk_dialog_set_default_response(GTK_DIALOG (pbox), GTK_RESPONSE_CLOSE);

        g_signal_connect (pbox, "response",
			  G_CALLBACK (presponse_cb),
			  eyes_applet);
	
	hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (pbox)->vbox), hbox, FALSE, FALSE, 2);
	
	label = gtk_label_new_with_mnemonic (_("_Theme Name:"));
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	model = gtk_list_store_new (1, G_TYPE_STRING);
	tree = gtk_tree_view_new_with_model (GTK_TREE_MODEL (model));
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (tree), FALSE);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), tree);
	g_object_unref (model);
	
	cell = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("not used", cell,
                                                           "text", 0, NULL);
        gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);
                                                           
        selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree));
        g_signal_connect (selection, "changed",
        		  G_CALLBACK (theme_selected_cb),
			  eyes_applet);
        
        for (i = 0; i < NUM_THEME_DIRECTORIES; i++) {
                if ((dfd = opendir (theme_directories[i])) != NULL) {
                        while ((dp = readdir (dfd)) != NULL) {
                                if (dp->d_name[0] != '.') {
                                        gchar *elems[2] = {NULL, NULL };
                                        gchar *theme_dir;
					elems[0] = filename;
                                        strcpy (filename, 
                                                theme_directories[i]);
                                        strcat (filename, dp->d_name);
                                        gtk_list_store_insert (model, &iter, 0);
                                        gtk_list_store_set (model, &iter, 0, &filename, -1);
                                        theme_dir = g_strdup_printf ("%s/", filename);
                                        if (!g_strcasecmp (eyes_applet->theme_dir, theme_dir)) {
                                        	GtkTreePath *path;
                                        	path = gtk_tree_model_get_path (GTK_TREE_MODEL (model), 
                                                        			&iter);
                                                gtk_tree_view_set_cursor (GTK_TREE_VIEW (tree), 
                                                			  path, 
                                                			  NULL, 
                                                			  FALSE);
                                                gtk_tree_path_free (path);
                                        }
                                        g_free (theme_dir);
                                }
                        }
                        closedir (dfd);
                }
        }
        
        gtk_box_pack_start (GTK_BOX (GTK_DIALOG (pbox)->vbox), tree, TRUE, TRUE, 2);
        
        gtk_widget_show_all (pbox);
        
        eyes_applet->prop_box.pbox = pbox;
	
	return;
}


