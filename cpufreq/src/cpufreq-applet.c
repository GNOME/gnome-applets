/*
 * Copyright (C) 2001, 2002 Free Software Foundation
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
#include <panel-applet.h>
#include <panel-applet-gconf.h>
#include <glade/glade.h>
#include <string.h>

#include "cpufreq-applet.h"
#include "cpufreq-prefs.h"
#include "cpufreq-popup.h"
#include "cpufreq.h"

static void       cpufreq_applet_preferences_dialog (BonoboUIComponent *uic, CPUFreqApplet *applet);
static void       cpufreq_applet_help_cb            (BonoboUIComponent *uic, CPUFreqApplet *applet);
static void       cpufreq_applet_about_cb           (BonoboUIComponent *uic, CPUFreqApplet *applet);
static gint       cpufreq_applet_get_max_cpu        (void);
static void       cpufreq_applet_pixmap_set_image   (CPUFreqApplet *applet);
static void       cpufreq_applet_destroy            (CPUFreqApplet *applet);
static void       cpufreq_setup_widgets             (CPUFreqApplet *applet);
static void       cpufreq_size_allocate_cb          (PanelApplet *pa, GtkAllocation *allocation,
										   gpointer gdata);
static void       cpufreq_change_orient_cb          (PanelApplet *pa, PanelAppletOrient orient,
										   gpointer gdata);
static void       cpufreq_background_changed        (PanelApplet *pa, PanelAppletBackgroundType type,
										   GdkColor *color, GdkPixmap *pixmap,
										   CPUFreqApplet *applet);
static GtkWidget *cpufreq_applet_new                (CPUFreqApplet *applet);
static gboolean   cpufreq_applet_fill               (CPUFreqApplet *applet);
static gboolean   cpufreq_applet_factory            (CPUFreqApplet *applet, const gchar *iid,
										   gpointer gdata);

static const BonoboUIVerb cpufreq_menu_verbs [] = {
	   BONOBO_UI_UNSAFE_VERB ("CPUFreqAppletPreferences",
						 cpufreq_applet_preferences_dialog),
	   BONOBO_UI_UNSAFE_VERB ("CPUFreqAppletHelp",
						 cpufreq_applet_help_cb),
	   BONOBO_UI_UNSAFE_VERB ("CPUFreqAppletAbout",
						 cpufreq_applet_about_cb),
	   BONOBO_UI_VERB_END
};

static GType
cpufreq_applet_get_type (void)
{
	   static GType type = 0;

	   if (!type) {
			 static const GTypeInfo info = {
				    sizeof (PanelAppletClass),
				    NULL, NULL, NULL, NULL, NULL,
				    sizeof (CPUFreqApplet),
				    0, NULL, NULL
			 };

			 type = g_type_register_static (
				    PANEL_TYPE_APPLET, "CPUFreqApplet", &info, 0);
	   }

	   return type;
}

void
cpufreq_applet_display_error (const gchar *error_message)
{
	   GtkWidget *dialog;

	   dialog = gtk_message_dialog_new (NULL,
								 GTK_DIALOG_DESTROY_WITH_PARENT,
								 GTK_MESSAGE_ERROR,
								 GTK_BUTTONS_CLOSE,
								 error_message);
	   gtk_dialog_run (GTK_DIALOG (dialog));
	   gtk_widget_destroy (dialog);
}

static void
cpufreq_applet_preferences_dialog (BonoboUIComponent *uic, CPUFreqApplet *applet)
{
	   g_return_if_fail (PANEL_IS_APPLET (PANEL_APPLET (applet)));
	   
	   cpufreq_preferences_dialog_run (applet);
}

static void
cpufreq_applet_help_cb (BonoboUIComponent *uic, CPUFreqApplet *applet)
{
	   GError *error;
	   
	   g_return_if_fail (PANEL_IS_APPLET (PANEL_APPLET (applet)));

	   error = NULL;
	   gnome_help_display_on_screen ("cpufreq-applet", NULL,
							   gtk_widget_get_screen (GTK_WIDGET (applet)),
							   &error);

	   if (error) {
			 if (error->code == GNOME_HELP_ERROR_INTERNAL) {
				    cpufreq_applet_display_error (_("Sorry, an internal error has occurred "
											 "with the CPU Frequency Scaling Monitor help"));
			 } else {
				    g_print ("help error: %s\n", error->message);
				    cpufreq_applet_display_error (_("Sorry, the document can not be found"));
			 }
			 
			 g_error_free (error);
	   }
}

static void
cpufreq_applet_about_cb (BonoboUIComponent *uic, CPUFreqApplet *applet)
{
	   GdkPixbuf   *pixbuf;
	   const gchar *authors[] = {
			 "Carlos Garcia Campos <carlosgc@gnome.org>",
			 " ",
			 _("Graphic Arts:"),
			 "Pablo Arroyo Loma <zzioma@yahoo.es>",
			 NULL
	   };
	   const gchar *documenters[] = {
			 "Carlos Garcia Campos <carlosgc@gnome.org>",
			 NULL
	   };
	   const gchar *translator_credits = _("translator_credits");

	   g_return_if_fail (PANEL_IS_APPLET (PANEL_APPLET (applet)));
	   
	   if (applet->about_dialog != NULL) {
			 gtk_window_set_screen (GTK_WINDOW (applet->about_dialog),
							    gtk_widget_get_screen (GTK_WIDGET (applet)));

			 gtk_window_present (GTK_WINDOW (applet->about_dialog));
			 return;
	   }

	   pixbuf = gdk_pixbuf_new_from_file_at_size (ICONDIR"/cpufreq-applet/cpufreq-applet.png",
										 48, 48, NULL);

	   applet->about_dialog = gnome_about_new (
			 _("CPU Frequency Scaling Monitor"),
			 VERSION,
			 _("Copyright (C) 2004 Free Software Foundation. Inc."),
			 _("This utility shows the current CPU Frequency Scaling."),
			 authors,
			 documenters,
			 g_ascii_strcasecmp (
				    translator_credits, "translator_credits") != 0 ? translator_credits : NULL,
			 pixbuf);

	   g_object_unref (pixbuf);

	   gtk_window_set_screen (GTK_WINDOW (applet->about_dialog),
						 gtk_widget_get_screen (GTK_WIDGET (applet)));

	   g_signal_connect (applet->about_dialog, "destroy",
					 G_CALLBACK (gtk_widget_destroyed),
					 &applet->about_dialog);
	   
	   gtk_widget_show (applet->about_dialog);

	   return;
}

static gint
cpufreq_applet_get_max_cpu ()
{
	   gint   mcpu = -1;
	   gchar *file = NULL;

	   do {
			 if (file) g_free (file);
			 mcpu ++;
			 file = g_strdup_printf ("/sys/devices/system/cpu/cpu%d", mcpu);
	   } while (g_file_test (file, G_FILE_TEST_EXISTS));
	   g_free (file);
	   mcpu --;

	   if (mcpu >= 0)
			 return mcpu;

	   mcpu = -1;
	   file = NULL;
	   do {
			 if (file) g_free (file);
			 mcpu ++;
			 file = g_strdup_printf ("/proc/sys/cpu/%d", mcpu);
	   } while (g_file_test (file, G_FILE_TEST_EXISTS));
	   g_free (file);
	   mcpu --;

	   if (mcpu >= 0)
			 return mcpu;
	   else
			 return 0;
}

static void
cpufreq_applet_pixmap_set_image (CPUFreqApplet *applet)
{
	   gint   perc, image;
	   gchar *text_perc;
	   gchar *pixmaps[] = {
			 ICONDIR"/cpufreq-applet/cpufreq-25.png",
			 ICONDIR"/cpufreq-applet/cpufreq-50.png",
			 ICONDIR"/cpufreq-applet/cpufreq-75.png",
			 ICONDIR"/cpufreq-applet/cpufreq-100.png",
			 ICONDIR"/cpufreq-applet/cpufreq-na.png",
			 NULL };

	   if (!applet->perc) {
			 image = 4;
	   } else {
			 text_perc = g_strndup (applet->perc, strlen (applet->perc) - 1);
			 perc = atoi (text_perc);
			 g_free (text_perc);
	   
			 /* 0-29   -> 25%	
			  * 30-69  -> 50%	
			  * 70-89  -> 75%	
			  * 90-100 -> 100%	
			  */
			 if (perc < 30)
				    image = 0;
			 else if ((perc >= 30) && (perc < 70))
				    image = 1;
			 else if ((perc >= 70) && (perc < 90))
				    image = 2;
			 else
				    image = 3;
	   }

	   if (applet->pixbufs[image] == NULL) {
			 applet->pixbufs[image] = gdk_pixbuf_new_from_file_at_size (pixmaps[image],
															24, 24, NULL);
	   }

	   gtk_image_set_from_pixbuf (GTK_IMAGE (applet->pixmap), applet->pixbufs[image]);
}

void
cpufreq_applet_update (CPUFreqApplet *applet)
{
	   gchar *text_tip, *text_mode;
	   gchar *governor;

	   g_return_if_fail (PANEL_IS_APPLET (PANEL_APPLET (applet)));

	   if (applet->show_mode != MODE_GRAPHIC) {
			 if (applet->show_mode == MODE_TEXT) {
				    gtk_widget_hide (applet->pixmap);
			 } else {
				    cpufreq_applet_pixmap_set_image (applet);
				    gtk_widget_show (applet->pixmap);
			 }

			 switch (applet->show_text_mode) {
			 case MODE_TEXT_FREQUENCY:
				    gtk_label_set_text (GTK_LABEL (applet->label), applet->freq);
				    gtk_widget_hide (applet->unit_label);

				    break;
			 case MODE_TEXT_FREQUENCY_UNIT:
				    gtk_label_set_text (GTK_LABEL (applet->label), applet->freq);
				    gtk_label_set_text (GTK_LABEL (applet->unit_label), applet->unit);
				    gtk_widget_show (applet->unit_label);

				    break;
			 case MODE_TEXT_PERCENTAGE:
				    gtk_label_set_text (GTK_LABEL (applet->label), applet->perc);
				    
				    break;
			 }
			 
			 gtk_widget_show (applet->label);
	   } else {
			 cpufreq_applet_pixmap_set_image (applet);
			 
			 gtk_widget_show (applet->pixmap);
			 gtk_widget_hide (applet->label);
			 gtk_widget_hide (applet->unit_label);
	   }
	   
	   governor = g_strdup (applet->governor);
	   governor[0] = g_ascii_toupper (governor[0]);
	   text_mode = g_strdup_printf ("%s\n%s %s (%s)", governor, 
							  applet->freq, applet->unit, applet->perc);
	   g_free (governor);

	   if (applet->mcpu == 0)
			 text_tip = g_strdup_printf ("%s", text_mode);
	   else
			 text_tip = g_strdup_printf ("CPU %d - %s", applet->cpu, text_mode);

	   g_free (text_mode);
	   
	   gtk_tooltips_set_tip (applet->tips, GTK_WIDGET (applet), text_tip, NULL);
	   g_free (text_tip);
}

static void
free_string (gpointer str, gpointer gdata)
{
	   if (str) g_free (str);
}

static void
cpufreq_applet_destroy (CPUFreqApplet *applet)
{
	   gint i;
	   
	   g_return_if_fail (PANEL_IS_APPLET (PANEL_APPLET (applet)));

	   if (applet->timeout_handler > 0)
			 g_source_remove (applet->timeout_handler);

	   if (applet->tips)
			 g_object_unref (G_OBJECT (applet->tips));

	   for (i=0; i<=3; i++) {
			 if (applet->pixbufs[i])
				    g_object_unref (G_OBJECT (applet->pixbufs[i]));
	   }

	   if (applet->freq) {
			 g_free (applet->freq);
			 applet->freq = NULL;
	   }
	   
	   if (applet->perc) {
			 g_free (applet->perc);
			 applet->perc = NULL;
	   }
	   
	   if (applet->unit) {
			 g_free (applet->unit);
			 applet->unit = NULL;
	   }

	   if (applet->available_freqs) {
			 g_list_foreach (applet->available_freqs,
						  free_string, NULL);
			 g_list_free (applet->available_freqs);
			 applet->available_freqs = NULL;
	   }

	   if (applet->about_dialog) {
			 gtk_widget_destroy (applet->about_dialog);
			 applet->about_dialog = NULL;
	   }

	   if (applet->prefs) {
			 gtk_widget_destroy (applet->prefs);
			 applet->prefs = NULL;
	   }

	   if (applet->popup) {
			 gtk_widget_destroy (applet->popup);
			 applet->popup = NULL;
	   }
}	

static void
cpufreq_setup_widgets (CPUFreqApplet *applet)
{
	   GtkWidget      *labels_box;
	   GtkRequisition  req;
	   gint            total_size = 0;
	   gboolean        horizontal = FALSE;
	   gint            panel_size, label_size, unit_label_size, pixmap_size;
	   gint            size_step = 12;

	   g_return_if_fail (PANEL_IS_APPLET (PANEL_APPLET (applet)));
	   
	   panel_size = applet->size - 1; /* 1 pixel margin */

	   switch (applet->orient) {
	   case PANEL_APPLET_ORIENT_LEFT:
	   case PANEL_APPLET_ORIENT_RIGHT:
			 horizontal = FALSE;
			 break;
	   case PANEL_APPLET_ORIENT_UP:
	   case PANEL_APPLET_ORIENT_DOWN:
			 horizontal = TRUE;
			 break;
	   }

	   if (applet->label)
			 gtk_widget_destroy (applet->label);
	   
	   applet->label = gtk_label_new (" --- ");
	   if (applet->freq)
			 gtk_label_set_text (GTK_LABEL (applet->label), applet->freq);
	   
	   gtk_widget_size_request (applet->label, &req);
	   if (applet->show_mode != MODE_GRAPHIC)
			 gtk_widget_show (applet->label);

	   if (horizontal) {
			 label_size = req.height;
			 total_size += req.height;
	   } else {
			 label_size = req.width;
			 total_size += req.width;
	   }
	   
	   if (applet->unit_label)
			 gtk_widget_destroy (applet->unit_label);
				    
	   applet->unit_label = gtk_label_new ("  ?  ");
	   if (applet->unit)
			 gtk_label_set_text (GTK_LABEL (applet->unit_label), applet->unit);
	   
	   gtk_widget_size_request (applet->unit_label, &req);
	   if (applet->show_mode != MODE_GRAPHIC &&
		  applet->show_text_mode == MODE_TEXT_FREQUENCY_UNIT)
			 gtk_widget_show (applet->unit_label);

	   if (horizontal) {
			 unit_label_size = req.height;
			 total_size += req.height;
	   } else {
			 unit_label_size = req.width;
			 total_size += req.width;
	   }

	   if (applet->pixmap)
			 gtk_widget_destroy (applet->pixmap);

	   applet->pixmap = gtk_image_new ();

	   cpufreq_applet_pixmap_set_image (applet);

	   gtk_widget_size_request (applet->pixmap, &req);
	   if (applet->show_mode != MODE_TEXT)
			 gtk_widget_show (applet->pixmap);

	   if (horizontal) {
			 pixmap_size = req.height;
			 total_size += req.height;
	   } else {
			 pixmap_size = req.width;
			 total_size += req.width;
	   }

	   if (applet->box)
			 gtk_widget_destroy (applet->box);

	   if (horizontal) {
			 labels_box  = gtk_hbox_new (FALSE, 2);
			 if ((label_size + pixmap_size) <= panel_size)
				    applet->box = gtk_vbox_new (FALSE, 2);
			 else
				    applet->box = gtk_hbox_new (FALSE, 2);
	   } else {
			 
			 if (total_size <= panel_size) {
				    applet->box = gtk_hbox_new (FALSE, 2);
				    labels_box  = gtk_hbox_new (FALSE, 2);
			 } else if ((label_size + unit_label_size) <= (panel_size - size_step)) {
				    applet->box = gtk_vbox_new (FALSE, 2);
				    labels_box  = gtk_hbox_new (FALSE, 2);
			 } else {
				    applet->box = gtk_vbox_new (FALSE, 2);
				    labels_box  = gtk_vbox_new (FALSE, 2);
			 }
	   }
			 
	   gtk_box_pack_start (GTK_BOX (labels_box), applet->label, FALSE, TRUE, 0);
	   gtk_box_pack_start (GTK_BOX (labels_box), applet->unit_label, FALSE, TRUE, 0);

	   gtk_widget_show (labels_box);
	   
	   gtk_box_pack_start (GTK_BOX (applet->box), applet->pixmap, FALSE, TRUE, 0);
	   gtk_box_pack_start (GTK_BOX (applet->box), labels_box, FALSE, TRUE, 0);
	   
	   gtk_widget_show (applet->box);

	   gtk_container_add (GTK_CONTAINER (applet->container), applet->box);

	   gtk_widget_show (applet->container);
}

static void
cpufreq_size_allocate_cb (PanelApplet *pa, GtkAllocation *allocation,
					 gpointer gdata)
{
	   CPUFreqApplet *applet;

	   applet = (CPUFreqApplet *) gdata;

	   if ((applet->orient == PANEL_APPLET_ORIENT_LEFT) ||
		  (applet->orient == PANEL_APPLET_ORIENT_RIGHT)) {
			 if (applet->size == allocation->width)
				    return;
			 applet->size = allocation->width;
	   } else {
			 if (applet->size == allocation->height)
				    return;
			 applet->size = allocation->height;
	   }

	   cpufreq_setup_widgets (applet);
}

static void
cpufreq_change_orient_cb (PanelApplet *pa, PanelAppletOrient orient, gpointer gdata)
{
	   CPUFreqApplet *applet;

	   applet = (CPUFreqApplet *) gdata;

	   applet->orient = orient;

	   cpufreq_setup_widgets (applet);
}

static void
cpufreq_background_changed (PanelApplet *pa,
			    PanelAppletBackgroundType type,
			    GdkColor *color,
			    GdkPixmap *pixmap,
			    CPUFreqApplet *applet)
{
	   /* Taken from TrashApplet */
	   GtkRcStyle *rc_style;
	   GtkStyle *style;

	   /* reset style */
	   gtk_widget_set_style (GTK_WIDGET (applet), NULL);
	   rc_style = gtk_rc_style_new ();
	   gtk_widget_modify_style (GTK_WIDGET (applet), rc_style);
	   g_object_unref (rc_style);
	   
	   switch (type) {
	   case PANEL_PIXMAP_BACKGROUND:
		   	 style = gtk_style_copy (GTK_WIDGET (applet)->style);
			 if (style->bg_pixmap[GTK_STATE_NORMAL])
				 g_object_unref (
					style->bg_pixmap[GTK_STATE_NORMAL]);
			 style->bg_pixmap[GTK_STATE_NORMAL] = g_object_ref (
					 pixmap);
			 gtk_widget_set_style (GTK_WIDGET (applet), style);
			 break;
	   case PANEL_COLOR_BACKGROUND:
			 gtk_widget_modify_bg (GTK_WIDGET (applet),
					 GTK_STATE_NORMAL, color);
			 break;
	   case PANEL_NO_BACKGROUND:
	   default:
			 break;
	   }
}

void
cpufreq_applet_run (CPUFreqApplet *applet)
{
	   gchar *text_tip;
	   
	   if (applet->timeout_handler > 0)
			 g_source_remove (applet->timeout_handler);
	   
	   switch (applet->iface) {
	   case IFACE_SYSFS:
			 applet->timeout_handler = g_timeout_add (1000, cpufreq_get_from_sysfs, (gpointer) applet);
			 
			 break;
	   case IFACE_PROCFS:
			 applet->timeout_handler = g_timeout_add (1000, cpufreq_get_from_procfs, (gpointer) applet);
			 
			 break;
	   case IFACE_CPUINFO:
			 /* If there is no cpufreq support it shows only the cpu frequency,
			  * I thi	nk is better than do nothing. I have to notify it to the user, because
			  * he could think	 that cpufreq is supported but it doesn't work succesfully
			  */
			 cpufreq_applet_display_error (_("There is no cpufreq support in your system, needed "
									   "to use CPU Frequency Scaling Monitor.\n"
									   "If your system already supports cpufreq, check whether "
									   "/sys or /proc file systems are mounted.\n"
									   "Now the applet will only show the current "
									   "cpu frequency."));


			 if (cpufreq_get_from_procfs_cpuinfo (applet)) {
				    text_tip = g_strdup_printf (_("CPU %d - cpufreq Not Supported\n%s (%s)"),
										  applet->cpu, applet->freq, applet->perc);
				    gtk_tooltips_set_tip (applet->tips, GTK_WIDGET (applet), text_tip, NULL);
				    g_free (text_tip);
			 } else {
				    gtk_tooltips_set_tip (applet->tips, GTK_WIDGET (applet),
									 _("cpufreq Not Supported"),
									 NULL);
			 }

			 gtk_widget_show (applet->pixmap);

			 break;
	   }
}

static GtkWidget *
cpufreq_applet_new (CPUFreqApplet *applet)
{
	   AtkObject *atk_obj;
	   gint       i;
	   GError    *error;

	   panel_applet_add_preferences (PANEL_APPLET (applet),
							   "/schemas/apps/cpufreq-applet/prefs", NULL);
	   
	   panel_applet_set_flags (PANEL_APPLET (applet), PANEL_APPLET_EXPAND_MINOR);

	   /* New applet, default values */
	   applet->timeout_handler = 0;
	   applet->mcpu = cpufreq_applet_get_max_cpu ();
	   applet->prefs = NULL;
	   applet->about_dialog = NULL;
	   applet->available_freqs = NULL;
	   applet->popup = NULL;

	   error = NULL;
	   applet->cpu = panel_applet_gconf_get_int (PANEL_APPLET (applet),
										"cpu", &error);

	   /* In case anything went wrong with gconf, get back to the default */
	   if (error || applet->cpu < 0) {
			 applet->cpu = 0;
			 if (error)
				    g_error_free (error);
	   }
	   
	   error = NULL;
	   applet->show_mode = panel_applet_gconf_get_int (PANEL_APPLET (applet),
											 "show_mode", &error);
	   
        /* In case anything went wrong with gconf, get back to the default */
	   if (error || applet->show_mode < MODE_GRAPHIC || applet->show_mode > MODE_BOTH) {
			 applet->show_mode = MODE_BOTH;
			 if (error)
				    g_error_free (error);
	   }

	   error = NULL;
	   applet->show_text_mode = panel_applet_gconf_get_int (PANEL_APPLET (applet),
												  "show_text_mode", &error);

	   /* In case anything went wrong with gconf, get back to the default */
	   if (error || applet->show_text_mode < MODE_TEXT_FREQUENCY ||
		  applet->show_text_mode > MODE_TEXT_PERCENTAGE) {
			 applet->show_text_mode = MODE_TEXT_FREQUENCY_UNIT;
			 g_error_free (error);
	   }
	   
	   applet->freq     = NULL;
	   applet->perc     = NULL;
	   applet->unit     = NULL;
	   applet->governor = NULL;

	   for (i=0; i<=4; i++)
			 applet->pixbufs[i] = NULL;

	   applet->tips = gtk_tooltips_new ();
	   g_object_ref (G_OBJECT (applet->tips));

	   g_signal_connect (G_OBJECT (applet), "destroy",
					 G_CALLBACK (cpufreq_applet_destroy),
					 NULL);
	   g_signal_connect (G_OBJECT (applet), "button_press_event",
					 G_CALLBACK (cpufreq_popup_show),
					 NULL);
	   g_signal_connect (G_OBJECT (applet), "size_allocate",
					 G_CALLBACK (cpufreq_size_allocate_cb),
					 (gpointer) applet);
	   g_signal_connect (G_OBJECT (applet), "change_orient",
					 G_CALLBACK (cpufreq_change_orient_cb),
					 (gpointer) applet);
	   g_signal_connect (G_OBJECT (applet), "change_background",
					 G_CALLBACK (cpufreq_background_changed),
					 (gpointer) applet);

	   applet->container = gtk_alignment_new (0.5, 0.5, 0, 0);
	   gtk_container_add (GTK_CONTAINER (applet), applet->container);
	   
	   applet->size = panel_applet_get_size (PANEL_APPLET (applet));
	   applet->orient = panel_applet_get_orient (PANEL_APPLET (applet));
	   
	   cpufreq_setup_widgets (applet);

	   /* Setup the menus */
	   panel_applet_setup_menu_from_file (PANEL_APPLET (applet),
								   DATADIR,
								   "GNOME_CPUFreqApplet.xml",
								   NULL,
								   cpufreq_menu_verbs,
								   applet);

	   if (panel_applet_get_locked_down (PANEL_APPLET (applet))) {
			 BonoboUIComponent *popup_component;
			 
			 popup_component = panel_applet_get_popup_component (PANEL_APPLET (applet));
			 
			 bonobo_ui_component_set_prop (popup_component,
									 "/commands/CPUFreqPreferences",
									 "hidden", "1",
									 NULL);
	   }

	   if (g_file_test ("/sys/devices/system/cpu/cpu0/cpufreq", G_FILE_TEST_EXISTS)) { /* 2.6 kernel */
			 applet->iface = IFACE_SYSFS;
	   } else if (g_file_test ("/proc/cpufreq", G_FILE_TEST_EXISTS)) { /* 2.4 kernel */
			 applet->iface = IFACE_PROCFS;
	   } else if (g_file_test ("/proc/cpuinfo", G_FILE_TEST_EXISTS)) {
			 applet->iface = IFACE_CPUINFO;
	   }

	   cpufreq_applet_run (applet);

	   atk_obj = gtk_widget_get_accessible (GTK_WIDGET (applet));

	   if (GTK_IS_ACCESSIBLE (atk_obj)) {
			 atk_object_set_name (atk_obj, _("CPU Frequency Scaling Monitor"));
			 atk_object_set_description (atk_obj, _("This utility shows the current CPU Frequency"));
	   }

	   return GTK_WIDGET (applet);
}

static gboolean
cpufreq_applet_fill (CPUFreqApplet *applet)
{
	   g_return_val_if_fail (PANEL_IS_APPLET (PANEL_APPLET (applet)), FALSE);
	   
	   gnome_window_icon_set_default_from_file
			 (ICONDIR"/cpufreq-applet/cpufreq-applet.png");

	   glade_gnome_init ();

	   cpufreq_applet_new (applet);

	   gtk_widget_show (GTK_WIDGET (applet));
	   
	   return TRUE;
}

static gboolean
cpufreq_applet_factory (CPUFreqApplet *applet, const gchar *iid, gpointer gdata)
{
	   gboolean retval = FALSE;

	   if (!strcmp (iid, "OAFIID:GNOME_CPUFreqApplet"))
			 retval = cpufreq_applet_fill (applet);

	   return retval;
}

PANEL_APPLET_BONOBO_FACTORY ("OAFIID:GNOME_CPUFreqApplet_Factory",
					    cpufreq_applet_get_type (),
					    "cpufreq-applet",
					    "0",
					    (PanelAppletFactoryCallback) cpufreq_applet_factory,
					    NULL)
