/*
 * GNOME CPUFreq Applet
 * Copyright (C) 2004 Carlos Garcia Campos <carlosgc@gnome.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Authors : Carlos García Campos <carlosgc@gnome.org>
 */

#include <config.h>

#include <gnome.h>

#include "cpufreq-popup.h"
#include "cpufreq-monitor.h"

static void       cpufreq_popup_position_menu         (GtkMenu *menu, int *x, int *y,
                                                       gboolean *push_in, gpointer  gdata);
static void       cpufreq_popup_set_frequency         (GtkWidget *widget, gpointer gdata);
static void       cpufreq_popup_set_governor          (GtkWidget *widget, gpointer gdata);
static void       cpufreq_popup_menu_item_set_image   (CPUFreqApplet *applet, GtkWidget *menu_item,
						       gint freq, gint max_freq);
static GtkWidget *cpufreq_popup_frequencies_menu_new  (CPUFreqApplet *applet, GList *available_freqs);
static GtkWidget *cpufreq_popup_governors_menu_new    (GList *available_govs);
static GtkWidget *cpufreq_popup_new                   (CPUFreqApplet *applet, GList *available_freqs,
						       GList *available_govs);

typedef struct _cpufreq_t {
        gint freq;
        gint cpu;
} cpufreq_t;

static void
cpufreq_popup_position_menu (GtkMenu *menu, int *x, int *y,
                             gboolean *push_in, gpointer  gdata)
{
        GtkWidget      *widget;
        GtkRequisition  requisition;
        gint            menu_xpos;
        gint            menu_ypos;

        widget = GTK_WIDGET (gdata);

        gtk_widget_size_request (GTK_WIDGET (menu), &requisition);

        gdk_window_get_origin (widget->window, &menu_xpos, &menu_ypos);

        menu_xpos += widget->allocation.x;
        menu_ypos += widget->allocation.y;

        switch (panel_applet_get_orient (PANEL_APPLET (widget))) {
        case PANEL_APPLET_ORIENT_DOWN:
        case PANEL_APPLET_ORIENT_UP:
                if (menu_ypos > gdk_screen_get_height (gtk_widget_get_screen (widget)) / 2)
                        menu_ypos -= requisition.height;
                else
                        menu_ypos += widget->allocation.height;
                break;
        case PANEL_APPLET_ORIENT_RIGHT:
        case PANEL_APPLET_ORIENT_LEFT:
                if (menu_xpos > gdk_screen_get_width (gtk_widget_get_screen (widget)) / 2)
                        menu_xpos -= requisition.width;
                else
                        menu_xpos += widget->allocation.width;
                break;
        default:
                g_assert_not_reached ();
        }
           
        *x = menu_xpos;
        *y = menu_ypos;
        *push_in = TRUE;
}

gboolean 
cpufreq_popup_show (CPUFreqApplet *applet, guint32 time)
{
        GList *available_freqs = NULL;
	GList *available_govs = NULL;

        if (!cpufreq_applet_selector_is_available ())
                return FALSE;
           
        if (applet->popup) {
                gtk_widget_destroy (applet->popup);
                applet->popup = NULL;
        }

        available_freqs = cpufreq_monitor_get_available_frequencies (applet->monitor);
	available_govs = cpufreq_monitor_get_available_governors (applet->monitor);
        if (!available_freqs)
                return FALSE;

        applet->popup = cpufreq_popup_new (applet, available_freqs, available_govs);
                         
        gtk_widget_grab_focus (GTK_WIDGET (applet));
                         
        gtk_menu_popup (GTK_MENU (applet->popup), NULL, NULL,
                        cpufreq_popup_position_menu, (gpointer) applet,
                        1, time);
        return TRUE;
}

static void
cpufreq_popup_set_frequency (GtkWidget *widget, gpointer gdata)
{
        gint   freq, cpu;
        gchar *path = NULL;
        gchar *command;
        cpufreq_t *cf;
        
        cf = (cpufreq_t *)gdata;
        freq = cf->freq;
        cpu = cf->cpu;

        path = g_find_program_in_path ("cpufreq-selector");

        if (!path)
                return;
           
        command = g_strdup_printf ("%s -f %d -c %d", path, freq, cpu);

        g_spawn_command_line_async (command, NULL); /* TODO: error */

        g_free (command);
        g_free (path);
}

static void
cpufreq_popup_set_governor (GtkWidget *widget, gpointer gdata)
{
	gchar *governor;
	gchar *path = NULL;
	gchar *command;

	governor = (gchar *) gdata;

	path = g_find_program_in_path ("cpufreq-selector");

	if (!path)
		return;

	command = g_strdup_printf ("%s -g %s", path, governor);

	g_spawn_command_line_async (command, NULL); /* TODO: error */

	g_free (command);
	g_free (path);
}

static void
cpufreq_popup_menu_item_set_image (CPUFreqApplet *applet, GtkWidget *menu_item,
                                   gint freq, gint max_freq)
{
        gint   perc, image;
        gchar *pixmaps[] = {
                GNOME_PIXMAPSDIR"/cpufreq-applet/cpufreq-25.png",
                GNOME_PIXMAPSDIR"/cpufreq-applet/cpufreq-50.png",
                GNOME_PIXMAPSDIR"/cpufreq-applet/cpufreq-75.png",
                GNOME_PIXMAPSDIR"/cpufreq-applet/cpufreq-100.png",
                NULL };

        perc = (freq * 100) / max_freq;

        if (perc < 30)
                image = 0;
        else if ((perc >= 30) && (perc < 70))
                image = 1;
        else if ((perc >= 70) && (perc < 90))
                image = 2;
        else
                image = 3;

        if (applet->pixbufs[image] == NULL) {
                applet->pixbufs[image] = gdk_pixbuf_new_from_file_at_size (pixmaps[image],
                                                                           24, 24, NULL);
        }
           
        gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item),
                                       gtk_image_new_from_pixbuf (applet->pixbufs[image]));
}

static GtkWidget *
cpufreq_popup_governors_menu_new (GList *available_govs)
{
	GtkWidget *menu_item, *submenu;
	gchar     *governor;

	submenu = gtk_menu_new ();

	while (available_govs) {
		governor = (gchar *) available_govs->data;
		menu_item = gtk_menu_item_new_with_label (governor);
		gtk_menu_shell_append (GTK_MENU_SHELL (submenu), menu_item);
		gtk_widget_show (menu_item);

		g_signal_connect (G_OBJECT (menu_item), "activate",
				  G_CALLBACK (cpufreq_popup_set_governor),
				  (gpointer) governor);

		available_govs = g_list_next (available_govs);
	}

	return submenu;
}

static GtkWidget *
cpufreq_popup_frequencies_menu_new (CPUFreqApplet *applet, GList *available_freqs)
{
	GtkWidget *menu_item, *submenu;
	gchar     *label;
	gchar     *text_freq, *text_unit, *text_perc;
	gint       freq, max_freq, divisor, cpu;
	cpufreq_t *cf;

	max_freq = atoi ((gchar *) available_freqs->data); /* First item is the max freq */
	cpu = cpufreq_monitor_get_cpu (applet->monitor);

	submenu = gtk_menu_new ();
	while (available_freqs) {
		freq = atoi ((gchar *) available_freqs->data);

		if (applet->show_mode != MODE_GRAPHIC &&
		    applet->show_text_mode == MODE_TEXT_PERCENTAGE) {
			if (freq > 0) {
				text_perc = g_strdup_printf ("%d%%", (freq * 100) / max_freq);
				label = g_strdup_printf ("%s", text_perc);
				g_free (text_perc);
			} else {
				label = g_strdup (_("Unknown"));
			}
		} else {
			if (freq > 999999) {
				divisor = (1000 * 1000);
				text_unit = g_strdup ("GHz");
			} else {
				divisor = 1000;
				text_unit = g_strdup ("MHz");
			}

			if (((freq % divisor) == 0) || divisor == 1000)
				text_freq = g_strdup_printf ("%d", freq / divisor);
			else
				text_freq = g_strdup_printf ("%3.2f", ((gfloat)freq / divisor));

			label = g_strdup_printf ("%s %s", text_freq, text_unit);
			g_free (text_freq);
			g_free (text_unit);
		}

		menu_item = gtk_image_menu_item_new_with_label (label);
		if (applet->show_mode != MODE_TEXT) {
			cpufreq_popup_menu_item_set_image (applet, menu_item, freq, max_freq);
		}
		gtk_menu_shell_append (GTK_MENU_SHELL (submenu), menu_item);
		gtk_widget_show (menu_item);
		
		cf = g_new (cpufreq_t, 1);
		cf->freq = freq;
		cf->cpu = cpu;
		
		g_signal_connect_data (G_OBJECT (menu_item), "activate",
				       G_CALLBACK (cpufreq_popup_set_frequency),
				       cf, (GClosureNotify)g_free, G_CONNECT_AFTER);

		g_free (label);

		available_freqs = g_list_next (available_freqs);
	}

	return submenu;
}

static GtkWidget *
cpufreq_popup_new (CPUFreqApplet *applet, GList *available_freqs, GList *available_govs)
{
        GtkWidget *popup = NULL;
	GtkWidget *menu_item, *submenu;

	if ((available_freqs == NULL) &&
	    (available_govs == NULL)) {
		return NULL;
	}
	
	switch (applet->selector_mode) {
	case SELECTOR_MODE_FREQUENCIES:
		popup = cpufreq_popup_frequencies_menu_new (applet, available_freqs);

		break;
	case SELECTOR_MODE_GOVERNORS:
		popup = cpufreq_popup_governors_menu_new (available_govs);

		break;
	case SELECTOR_MODE_BOTH:
		popup = gtk_menu_new ();

		submenu = cpufreq_popup_governors_menu_new (available_govs);
	
		menu_item = gtk_menu_item_new_with_label (_("Governors"));
		gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), submenu);
		gtk_menu_shell_append (GTK_MENU_SHELL (popup), menu_item);
		gtk_widget_show (menu_item);

		submenu = cpufreq_popup_frequencies_menu_new (applet, available_freqs);

		menu_item = gtk_menu_item_new_with_label (_("Frequencies"));
		gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), submenu);
		gtk_menu_shell_append (GTK_MENU_SHELL (popup), menu_item);
		gtk_widget_show (menu_item);

		break;
	}

        return popup;
}
