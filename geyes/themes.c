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
#include "geyes.h"

extern EyesApplet eyes_applet;

gchar *theme_directories[] = {
        GEYES_THEMES_DIR,
        "~/.gnome/geyes-themes/"
};
#define NUM_THEME_DIRECTORIES 2

static void
parse_theme_file (FILE *theme_file)
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
                        sscanf (token, "%d", &eyes_applet.wall_thickness); 
                } else if (strncmp (token, "num-eyes", strlen ("num-eyes")) == 0) {
                        token += strlen ("num-eyes");
                        while (!isdigit (*token)) {
                                token++;
                        }
                        sscanf (token, "%d", &eyes_applet.num_eyes);
                } else if (strncmp (token, "eye-pixmap", strlen ("eye-pixmap")) == 0) {
                        token = strtok (NULL, "\"");
                        token = strtok (NULL, "\"");          
                        if (eyes_applet.eye_filename != NULL) 
                                g_free (eyes_applet.eye_filename);
                        eyes_applet.eye_filename = g_strdup_printf ("%s%s",
                                                                    eyes_applet.theme_dir,
                                                                    token);
                } else if (strncmp (token, "pupil-pixmap", strlen ("pupil-pixmap")) == 0) {
                        token = strtok (NULL, "\"");
                        token = strtok (NULL, "\"");      
            if (eyes_applet.pupil_filename != NULL) 
                    g_free (eyes_applet.pupil_filename);
            eyes_applet.pupil_filename 
                    = g_strdup_printf ("%s%s",
                                       eyes_applet.theme_dir,
                                       token);   
                }
                fgets (line_buf, 512, theme_file);
        }       
}

void
load_theme (gchar *theme_dir)
{
        FILE* theme_file;
        gchar *file_name;
        
        eyes_applet.theme_dir = g_strdup_printf ("%s/", theme_dir);
        
        file_name = g_malloc (strlen (theme_dir) + strlen ("/config") + 1);
        strcpy (file_name, theme_dir);
        strcat (file_name, "/config");
        theme_file = fopen (file_name, "r");
        if (theme_file == NULL) {
                g_error ("Unable to open theme file.");
        }
        
        parse_theme_file (theme_file);
        fclose (theme_file);
        
        eyes_applet.theme_name = g_strdup (theme_dir);
        
        eyes_applet.eye_image = 
                gdk_imlib_load_image (eyes_applet.eye_filename);
        gdk_imlib_render (eyes_applet.eye_image,            
                          eyes_applet.eye_image->rgb_width,
                          eyes_applet.eye_image->rgb_height);
        eyes_applet.pupil_image = gdk_imlib_load_image (eyes_applet.pupil_filename);
        gdk_imlib_render (eyes_applet.pupil_image,            
                          eyes_applet.pupil_image->rgb_width,
                          eyes_applet.pupil_image->rgb_height);
        
        eyes_applet.eye_height = eyes_applet.eye_image->rgb_height;
        eyes_applet.eye_width = eyes_applet.eye_image->rgb_width;
        eyes_applet.pupil_height = eyes_applet.pupil_image->rgb_height;
        eyes_applet.pupil_width = eyes_applet.pupil_image->rgb_width;
        
        g_free (file_name);
        
}

static void
destroy_theme ()
{
        gdk_imlib_kill_image (eyes_applet.eye_image);
        gdk_imlib_kill_image (eyes_applet.pupil_image);
        
        g_free (eyes_applet.theme_dir);
        g_free (eyes_applet.theme_name);
}

static void
theme_selected_cb (GtkWidget *widget, 
                   gint row, 
                   gint col,
                   GdkEventButton *event,
                   gpointer data)
{
        eyes_applet.prop_box.selected_row = row;
        gnome_property_box_changed (GNOME_PROPERTY_BOX (eyes_applet.prop_box.pbox));
	return;
}

static void
apply_cb (GtkWidget *prob_box, gint page_num, gpointer data)
{
        gchar *clist_text;
        
        if (page_num == 0) {
                gtk_clist_get_text (GTK_CLIST (eyes_applet.prop_box.clist),
                                    eyes_applet.prop_box.selected_row,
                                    0,
                                    &clist_text);
                destroy_eyes ();
                destroy_theme ();
                load_theme (clist_text);
                create_eyes ();
	}
	return;
}

static void
phelp_cb (GtkWidget *w, gint tab, gpointer data)
{
	GnomeHelpMenuEntry help_entry = { "geyes_applet",
					  "index.html#GEYES-PREFS" };
	gnome_help_display(NULL, &help_entry);
}

void
properties_cb (AppletWidget *applet, gpointer data)
{
        GtkWidget *pbox;
        GtkWidget *clist;
        GtkWidget *label;
        gchar *title [] = { N_("Theme Name"), NULL };
        DIR *dfd;
        struct dirent *dp;
        int i;
        gchar filename [PATH_MAX];
        
	if (eyes_applet.prop_box.pbox) {
		gdk_window_show (eyes_applet.prop_box.pbox->window);
		gdk_window_raise (eyes_applet.prop_box.pbox->window);
		return;
	}

        pbox = gnome_property_box_new ();
        gtk_signal_connect (GTK_OBJECT (pbox),
                            "apply",
                            GTK_SIGNAL_FUNC (apply_cb),
                            NULL);
        gtk_signal_connect (GTK_OBJECT (pbox), "destroy",
			    GTK_SIGNAL_FUNC (gtk_widget_destroyed),
			    &eyes_applet.prop_box.pbox);
	gtk_signal_connect (GTK_OBJECT (pbox), "help",
			    GTK_SIGNAL_FUNC (phelp_cb), NULL);
        gtk_window_set_title (GTK_WINDOW (pbox),
                              _("gEyes Settings"));
        
        title[0] = _(title[0]); /* necessary? */
        clist = gtk_clist_new_with_titles (1, title);
        gtk_clist_set_column_width (GTK_CLIST (clist), 0, 200);
        gtk_signal_connect (GTK_OBJECT (clist), "select_row", 
                            GTK_SIGNAL_FUNC (theme_selected_cb), 
                            NULL);
        
        for (i = 0; i < NUM_THEME_DIRECTORIES; i++) {
                if ((dfd = opendir (theme_directories[i])) != NULL) {
                        while ((dp = readdir (dfd)) != NULL) {
                                if (dp->d_name[0] != '.') {
                                        gchar *elems[2] = {NULL, NULL };
					elems[0] = filename;
                                        strcpy (filename, 
                                                theme_directories[i]);
                                        strcat (filename, dp->d_name);
                                        gtk_clist_append (GTK_CLIST (clist), 
                                                          elems);
                                }
                        }
                        closedir (dfd);
                }
        }
        label = gtk_label_new (_("Themes"));
        gnome_property_box_append_page (GNOME_PROPERTY_BOX (pbox), 
                                        clist, label);
        
        gtk_widget_show_all (pbox);
        
        eyes_applet.prop_box.pbox = pbox;
	eyes_applet.prop_box.clist = clist;
	return;
}
