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
#include <glib/gi18n.h>
#include <string.h>

#include "cpufreq-applet.h"
#include "cpufreq-prefs.h"
#include "cpufreq-popup.h"
#include "cpufreq-monitor.h"
#include "cpufreq-monitor-factory.h"

#define PARENT_TYPE PANEL_TYPE_APPLET

static void     cpufreq_applet_init              (CPUFreqApplet *applet);
static void     cpufreq_applet_class_init        (CPUFreqAppletClass *klass);

static void     cpufreq_applet_preferences_cb    (BonoboUIComponent *uic, CPUFreqApplet *applet,
										const gchar *cname);
static void     cpufreq_applet_help_cb           (BonoboUIComponent *uic, CPUFreqApplet *applet,
										const gchar *cname);
static void     cpufreq_applet_about_cb          (BonoboUIComponent *uic, CPUFreqApplet *applet,
										const gchar *cname);

static gint     cpufreq_applet_get_max_cpu       (void);
static void     cpufreq_applet_pixmap_set_image  (CPUFreqApplet *applet, const gchar *percentage);

static void     cpufreq_applet_update            (CPUFreqMonitor *monitor, gpointer gdata);
static void     cpufreq_applet_refresh           (CPUFreqApplet *applet);

static void     cpufreq_applet_destroy           (GtkObject *widget);
static gboolean cpufreq_applet_button_press      (GtkWidget *widget, GdkEventButton *event);
static gboolean cpufreq_applet_key_press         (GtkWidget *widget, GdkEventKey *event);
static void     cpufreq_applet_size_allocate     (GtkWidget *widget, GtkAllocation *allocation);
static void     cpufreq_applet_change_orient     (PanelApplet *pa, PanelAppletOrient orient);
static void     cpufreq_applet_change_background (PanelApplet *pa, PanelAppletBackgroundType type,
										GdkColor *color, GdkPixmap *pixmap);

static gboolean cpufreq_applet_fill              (CPUFreqApplet *applet);
static gboolean cpufreq_applet_factory           (CPUFreqApplet *applet, const gchar *iid,
										gpointer gdata);

static PanelAppletClass *parent_class = NULL;

static const BonoboUIVerb cpufreq_applet_menu_verbs[] = {
	   BONOBO_UI_UNSAFE_VERB ("CPUFreqAppletPreferences",
						 cpufreq_applet_preferences_cb),
	   BONOBO_UI_UNSAFE_VERB ("CPUFreqAppletHelp",
						 cpufreq_applet_help_cb),
	   BONOBO_UI_UNSAFE_VERB ("CPUFreqAppletAbout",
						 cpufreq_applet_about_cb),
	   BONOBO_UI_VERB_END
};

GType
cpufreq_applet_get_type (void)
{
	   static GType type = 0;

	   if (!type) {
			 static const GTypeInfo info = {
				    sizeof (CPUFreqAppletClass),
				    (GBaseInitFunc) NULL,
				    (GBaseFinalizeFunc) NULL,
				    (GClassInitFunc) cpufreq_applet_class_init,
				    NULL,
				    NULL,
				    sizeof (CPUFreqApplet),
				    0,
				    (GInstanceInitFunc) cpufreq_applet_init
			 };

			 type = g_type_register_static (PARENT_TYPE, "CPUFreqApplet",
									  &info, 0);
	   }

	   return type;
}

static void
cpufreq_applet_init (CPUFreqApplet *applet)
{
	   AtkObject *atk_obj;
	   gint       i;
	   GError    *error;
	   guint      cpu;

	   applet->mcpu = cpufreq_applet_get_max_cpu ();
	   applet->prefs = NULL;
	   applet->popup = NULL;

	   applet->label = NULL;
	   applet->unit_label = NULL;
	   applet->pixmap = NULL;
	   applet->box = NULL;
	   
	   for (i = 0; i <= 4; i++)
			 applet->pixbufs[i] = NULL;

	   applet->tips = gtk_tooltips_new ();
	   g_object_ref (G_OBJECT (applet->tips));

	   applet->container = gtk_alignment_new (0.5, 0.5, 0, 0);
	   gtk_container_add (GTK_CONTAINER (applet), applet->container);

	   panel_applet_add_preferences (PANEL_APPLET (applet),
							   "/schemas/apps/cpufreq-applet/prefs", NULL);

	   panel_applet_set_flags (PANEL_APPLET (applet), PANEL_APPLET_EXPAND_MINOR);

	   applet->size = panel_applet_get_size (PANEL_APPLET (applet));
	   applet->orient = panel_applet_get_orient (PANEL_APPLET (applet));
	   
	   error = NULL;
	   cpu = panel_applet_gconf_get_int (PANEL_APPLET (applet),
								  "cpu", &error);

	   /* In case anything went wrong with gconf, get back to the default */
	   if (error || cpu < 0) {
			 cpu = 0;
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
	   
	   atk_obj = gtk_widget_get_accessible (GTK_WIDGET (applet));

	   if (GTK_IS_ACCESSIBLE (atk_obj)) {
			 atk_object_set_name (atk_obj, _("CPU Frequency Scaling Monitor"));
			 atk_object_set_description (atk_obj, _("This utility shows the current CPU Frequency"));
	   }

	   applet->monitor = cpufreq_monitor_factory_create_monitor (cpu);
	   g_signal_connect (G_OBJECT (applet->monitor), "changed",
					 G_CALLBACK (cpufreq_applet_update),
					 (gpointer) applet);
}

static void
cpufreq_applet_class_init (CPUFreqAppletClass *klass)
{
	   PanelAppletClass *applet_class = PANEL_APPLET_CLASS (klass);
	   GtkObjectClass   *gtkobject_class = GTK_OBJECT_CLASS (klass);
	   GtkWidgetClass   *widget_class = GTK_WIDGET_CLASS (klass);

	   parent_class = g_type_class_peek_parent (klass);

	   gtkobject_class->destroy = cpufreq_applet_destroy;
	   
	   widget_class->size_allocate = cpufreq_applet_size_allocate;
	   widget_class->key_press_event = cpufreq_applet_key_press;
	   widget_class->button_press_event = cpufreq_applet_button_press;
	   
	   applet_class->change_orient = cpufreq_applet_change_orient;
	   applet_class->change_background = cpufreq_applet_change_background;
}

void
cpufreq_applet_display_error (const gchar *message, 
                              const gchar *secondary)
{
	   GtkWidget *dialog;
	   gchar     *bold_str;
	   
	   bold_str = g_strconcat ("<span weight=\"bold\" size=\"larger\">", message, "</span>", NULL);		

	   dialog = gtk_message_dialog_new_with_markup (NULL,
								 GTK_DIALOG_DESTROY_WITH_PARENT,
								 GTK_MESSAGE_ERROR,
								 GTK_BUTTONS_OK,
								 bold_str);
	   gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
	                                             secondary);
	   gtk_dialog_run (GTK_DIALOG (dialog));
	   gtk_widget_destroy (dialog);
	   g_free (bold_str);
}

static void
cpufreq_applet_preferences_cb (BonoboUIComponent *uic, CPUFreqApplet *applet, const gchar *cname)
{
	   g_return_if_fail (PANEL_IS_APPLET (PANEL_APPLET (applet)));
	   
	   cpufreq_preferences_dialog_run (applet);
}

static void
cpufreq_applet_help_cb (BonoboUIComponent *uic, CPUFreqApplet *applet, const gchar *cname)
{
	   GError *error;
	   
	   g_return_if_fail (PANEL_IS_APPLET (PANEL_APPLET (applet)));

	   error = NULL;
	   gnome_help_display_on_screen ("cpufreq-applet", NULL,
							   gtk_widget_get_screen (GTK_WIDGET (applet)),
							   &error);

	   if (error) {
			 cpufreq_applet_display_error (_("Could not open help document"),
									 error->message);
			 g_error_free (error);
	   }
}

static void
cpufreq_applet_about_cb (BonoboUIComponent *uic, CPUFreqApplet *applet, const gchar *cname)
{
	   static GtkWidget   *about = NULL;
	   static const gchar *authors[] = {
			 "Carlos Garcia Campos <carlosgc@gnome.org>",
			 NULL
	   };
	   static const gchar *documenters[] = {
			 "Carlos Garcia Campos <carlosgc@gnome.org>",
			 NULL
	   };
	   static const gchar *artists[] = {
			 "Pablo Arroyo Loma <zzioma@yahoo.es>",
			 NULL
	   };
	   GdkPixbuf          *pixbuf = NULL;

	   g_return_if_fail (PANEL_IS_APPLET (PANEL_APPLET (applet)));
	   
	   pixbuf = gdk_pixbuf_new_from_file_at_size (ICONDIR"/cpufreq-applet/cpufreq-applet.png",
										 48, 48, NULL);

	   if (about != NULL) {
			 gtk_window_set_transient_for (GTK_WINDOW (about),
									 GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (applet))));
			 gtk_window_set_screen (GTK_WINDOW (about),
							    gtk_widget_get_screen (GTK_WIDGET (applet)));
			 gtk_window_present (GTK_WINDOW (about));

			 return;
	   }

	   about = g_object_new (GTK_TYPE_ABOUT_DIALOG,
						"name", _("CPU Frequency Scaling Monitor"),
						"version", VERSION,
						"copyright", _("Copyright (C) 2004 Free Software Foundation. Inc."),
						"comments", _("This utility shows the current CPU Frequency Scaling."),
						"authors", authors,
						"documenters", documenters,
						"artists", artists, 
						"translator-credits", _("translator_credits"),
						"logo", pixbuf,
						NULL);

	   gtk_window_set_destroy_with_parent (GTK_WINDOW (about), TRUE);

	   g_signal_connect (about, "response", G_CALLBACK (gtk_widget_destroy), NULL);
	   g_signal_connect (about, "destroy", G_CALLBACK (gtk_widget_destroyed), &about);

	   gtk_window_set_transient_for (GTK_WINDOW (about),
							   GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (applet))));
	   gtk_window_set_screen (GTK_WINDOW (about),
						 gtk_widget_get_screen (GTK_WIDGET (applet)));
	   gtk_window_present (GTK_WINDOW (about));

	   if (pixbuf)
			 g_object_unref (pixbuf);
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
cpufreq_applet_pixmap_set_image (CPUFreqApplet *applet, const gchar *percentage)
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

	   if (!percentage) {
			 image = 4;
	   } else {
			 text_perc = g_strndup (percentage, strlen (percentage) - 1);
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

static void
cpufreq_applet_update (CPUFreqMonitor *monitor, gpointer gdata)
{
	   gchar         *text_tip, *text_mode;
	   gchar         *freq, *perc, *unit;
	   guint          cpu;
	   gchar         *governor;
	   CPUFreqApplet *applet;

	   applet = CPUFREQ_APPLET (gdata);

	   cpu = cpufreq_monitor_get_cpu (monitor);
	   freq = cpufreq_monitor_get_frequency (monitor);
	   perc = cpufreq_monitor_get_percentage (monitor);
	   unit = cpufreq_monitor_get_unit (monitor);
	   governor = cpufreq_monitor_get_governor (monitor);

	   if (applet->show_mode != MODE_GRAPHIC) {
			 if (applet->show_mode == MODE_TEXT) {
				    gtk_widget_hide (applet->pixmap);
			 } else {
				    cpufreq_applet_pixmap_set_image (applet, perc);
				    gtk_widget_show (applet->pixmap);
			 }

			 switch (applet->show_text_mode) {
			 case MODE_TEXT_FREQUENCY:
				    gtk_label_set_text (GTK_LABEL (applet->label), freq);
				    gtk_widget_hide (applet->unit_label);

				    break;
			 case MODE_TEXT_FREQUENCY_UNIT:
				    gtk_label_set_text (GTK_LABEL (applet->label), freq);
				    gtk_label_set_text (GTK_LABEL (applet->unit_label), unit);
				    gtk_widget_show (applet->unit_label);

				    break;
			 case MODE_TEXT_PERCENTAGE:
				    gtk_label_set_text (GTK_LABEL (applet->label), perc);
				    
				    break;
			 }
			 
			 gtk_widget_show (applet->label);
	   } else {
			 cpufreq_applet_pixmap_set_image (applet, perc);
			 
			 gtk_widget_show (applet->pixmap);
			 gtk_widget_hide (applet->label);
			 gtk_widget_hide (applet->unit_label);
	   }
	   
	   governor[0] = g_ascii_toupper (governor[0]);
	   text_mode = g_strdup_printf ("%s\n%s %s (%s)", governor, freq, unit, perc);

	   g_free (freq);
	   g_free (unit);
	   g_free (perc);
	   g_free (governor);

	   if (applet->mcpu == 0)
			 text_tip = g_strdup_printf ("%s", text_mode);
	   else
			 text_tip = g_strdup_printf ("CPU %d - %s", cpu, text_mode);

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
cpufreq_applet_destroy (GtkObject *widget)
{
	   CPUFreqApplet *applet;
	   gint           i;

	   applet = CPUFREQ_APPLET (widget);

	   if (applet->monitor) {
			 g_object_unref (G_OBJECT (applet->monitor));
			 applet->monitor = NULL;
	   }

	   if (applet->tips) {
			 g_object_unref (G_OBJECT (applet->tips));
			 applet->tips = NULL;
	   }
	   
	   for (i = 0; i <= 3; i++) {
			 if (applet->pixbufs[i]) {
				    g_object_unref (G_OBJECT (applet->pixbufs[i]));
				    applet->pixbufs[i] = NULL;
			 }
	   }

	   if (applet->prefs) {
			 gtk_widget_destroy (applet->prefs);
			 applet->prefs = NULL;
	   }

	   if (applet->popup) {
			 gtk_widget_destroy (applet->popup);
			 applet->popup = NULL;
	   }

	   (* GTK_OBJECT_CLASS (parent_class)->destroy) (widget);
}	

static gboolean
cpufreq_applet_button_press (GtkWidget *widget, GdkEventButton *event)
{
	   CPUFreqApplet *applet;

	   applet = CPUFREQ_APPLET (widget);

	   if (event->button == 1)
			 return cpufreq_popup_show (applet, event->time);

	   if (GTK_WIDGET_CLASS (parent_class)->button_press_event)
			 return (* GTK_WIDGET_CLASS (parent_class)->button_press_event) (widget, event);
	   else
			 return FALSE;
}

static gboolean
cpufreq_applet_key_press (GtkWidget *widget, GdkEventKey *event)
{
	   CPUFreqApplet *applet;

	   applet = CPUFREQ_APPLET (widget);

	   switch (event->keyval) {
	   case GDK_KP_Enter:
	   case GDK_ISO_Enter:
	   case GDK_3270_Enter:
	   case GDK_Return:
	   case GDK_space:
	   case GDK_KP_Space:
			 return cpufreq_popup_show (applet, event->time);
	   default:
			 break;
	   }

	   if (GTK_WIDGET_CLASS (parent_class)->key_press_event)
			 return (* GTK_WIDGET_CLASS (parent_class)->key_press_event) (widget, event);
	   else
			 return FALSE;
}
	   
static void
cpufreq_applet_refresh (CPUFreqApplet *applet)
{
	   GtkWidget      *labels_box;
	   GtkRequisition  req;
	   gint            total_size = 0;
	   gboolean        horizontal = FALSE;
	   gint            panel_size, label_size, unit_label_size, pixmap_size;
	   gint            size_step = 12;
	   gchar          *freq, *perc, *unit;

	   g_return_if_fail (PANEL_IS_APPLET (PANEL_APPLET (applet)));

	   freq = cpufreq_monitor_get_frequency (applet->monitor);
	   perc = cpufreq_monitor_get_percentage (applet->monitor);
	   unit = cpufreq_monitor_get_unit (applet->monitor);
	   
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
	   if (freq) {
			 gtk_label_set_text (GTK_LABEL (applet->label), freq);
			 g_free (freq);
	   }
	   
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
	   if (unit) {
			 gtk_label_set_text (GTK_LABEL (applet->unit_label), unit);
			 g_free (unit);
	   }
	   
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

	   cpufreq_applet_pixmap_set_image (applet, perc);
	   if (perc) g_free (perc);

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
cpufreq_applet_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
	   CPUFreqApplet *applet;
	   gint           size;

	   applet = CPUFREQ_APPLET (widget);

	   if ((applet->orient == PANEL_APPLET_ORIENT_LEFT) ||
		  (applet->orient == PANEL_APPLET_ORIENT_RIGHT)) {
			 size = allocation->width;
	   } else {
			 size = allocation->height;
	   }

	   if (size != applet->size) {
			 applet->size = size;
			 cpufreq_applet_refresh (applet);
	   }
	   
	   (* GTK_WIDGET_CLASS (parent_class)->size_allocate) (widget, allocation);
}

static void
cpufreq_applet_change_orient (PanelApplet *pa, PanelAppletOrient orient)
{
	   CPUFreqApplet *applet;
	   gint           size;

	   applet = CPUFREQ_APPLET (pa);

	   applet->orient = orient;
	   
	   if ((orient == PANEL_APPLET_ORIENT_LEFT) ||
		  (orient == PANEL_APPLET_ORIENT_RIGHT)) {
			 size = GTK_WIDGET (applet)->allocation.width;
	   } else {
			 size = GTK_WIDGET (applet)->allocation.height;
	   }
			 
	   if (size != applet->size) {
			 applet->size = size;
			 cpufreq_applet_refresh (applet);
	   }

	   if (PANEL_APPLET_CLASS (parent_class)->change_orient)
			 (* PANEL_APPLET_CLASS (parent_class)->change_orient) (pa, orient);
}

static void
cpufreq_applet_change_background (PanelApplet *pa,
						    PanelAppletBackgroundType type,
						    GdkColor *color, GdkPixmap *pixmap)
{
	   CPUFreqApplet *applet;
	   /* Taken from TrashApplet */
	   GtkRcStyle    *rc_style;
	   GtkStyle      *style;

	   applet = CPUFREQ_APPLET (pa);
	   
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

static gboolean
cpufreq_applet_fill (CPUFreqApplet *applet)
{
	   BonoboUIComponent *popup_component;
	   
	   g_return_val_if_fail (PANEL_IS_APPLET (PANEL_APPLET (applet)), FALSE);
	   
	   gnome_window_icon_set_default_from_file
			 (ICONDIR"/cpufreq-applet/cpufreq-applet.png");

	   glade_gnome_init ();

	   /* Setup the menus */
	   panel_applet_setup_menu_from_file (PANEL_APPLET (applet),
								   DATADIR,
								   "GNOME_CPUFreqApplet.xml",
								   NULL,
								   cpufreq_applet_menu_verbs,
								   applet);

	   if (panel_applet_get_locked_down (PANEL_APPLET (applet))) {
			 popup_component = panel_applet_get_popup_component (PANEL_APPLET (applet));
			 
			 bonobo_ui_component_set_prop (popup_component,
									 "/commands/CPUFreqPreferences",
									 "hidden", "1",
									 NULL);
	   }

	   cpufreq_applet_refresh (applet);

	   cpufreq_monitor_run (applet->monitor);
	   
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
					    TYPE_CPUFREQ_APPLET,
					    "cpufreq-applet",
					    "0",
					    (PanelAppletFactoryCallback) cpufreq_applet_factory,
					    NULL)
