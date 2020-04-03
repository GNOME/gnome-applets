/*
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * Authors : Carlos García Campos <carlosgc@gnome.org>
 */

#include "config.h"
#include "cpufreq-applet.h"

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <glib/gi18n-lib.h>
#include <stdlib.h>
#include <string.h>

#include "cpufreq-prefs.h"
#include "cpufreq-popup.h"
#include "cpufreq-monitor.h"
#include "cpufreq-utils.h"

#define SPACING 2

struct _CPUFreqApplet {
        GpApplet       parent;

        /* Visibility */
	CPUFreqShowMode   show_mode;
	CPUFreqShowTextMode show_text_mode;
        gboolean          show_freq;
        gboolean          show_perc;
        gboolean          show_unit;
        gboolean          show_icon;

        CPUFreqMonitor   *monitor;

        gint              size;

        GtkWidget        *box;
        GtkWidget        *icon;
        GtkWidget        *labels_box;
        GtkWidget        *label;
        GtkWidget        *unit_label;

        GdkPixbuf        *pixbufs[5];

	gint              max_label_width;
	gint              max_perc_width;
	gint              max_unit_width;

        guint             refresh_id;

        CPUFreqPrefs     *prefs;
        CPUFreqPopup     *popup;
};

static void     queue_refresh                    (CPUFreqApplet *applet);

static void     cpufreq_applet_setup             (CPUFreqApplet *applet);

static void     cpufreq_applet_preferences_cb    (GSimpleAction *action,
                                                  GVariant      *parameter,
                                                  gpointer       user_data);
static void     cpufreq_applet_help_cb           (GSimpleAction *action,
                                                  GVariant      *parameter,
                                                  gpointer       user_data);
static void     cpufreq_applet_about_cb          (GSimpleAction *action,
                                                  GVariant      *parameter,
                                                  gpointer       user_data);

static void     cpufreq_applet_update            (CPUFreqApplet      *applet,
                                                  CPUFreqMonitor     *monitor);
static void     cpufreq_applet_refresh           (CPUFreqApplet      *applet);

static void     cpufreq_applet_dispose           (GObject          *widget);
static gboolean cpufreq_applet_button_press      (GtkWidget          *widget,
                                                  GdkEventButton     *event);
static gboolean cpufreq_applet_key_press         (GtkWidget          *widget,
                                                  GdkEventKey        *event);
static void     cpufreq_applet_size_allocate     (GtkWidget          *widget,
                                                  GtkAllocation      *allocation);
static void   cpufreq_applet_get_preferred_width (GtkWidget          *widget,
                                                  gint               *minimum_width,
                                                  gint               *natural_width);

static const gchar *const cpufreq_icons[] = {
        GRESOURCE_PREFIX "/icons/cpufreq-25.png",
        GRESOURCE_PREFIX "/icons/cpufreq-50.png",
        GRESOURCE_PREFIX "/icons/cpufreq-75.png",
        GRESOURCE_PREFIX "/icons/cpufreq-100.png",
        GRESOURCE_PREFIX "/icons/cpufreq-na.png",
        NULL
};

static const GActionEntry cpufreq_applet_menu_actions [] = {
	{ "preferences", cpufreq_applet_preferences_cb, NULL, NULL, NULL },
	{ "help",        cpufreq_applet_help_cb,        NULL, NULL, NULL },
	{ "about",       cpufreq_applet_about_cb,       NULL, NULL, NULL },
	{ NULL }
};

G_DEFINE_TYPE (CPUFreqApplet, cpufreq_applet, GP_TYPE_APPLET)

/* Enum Types */
GType
cpufreq_applet_show_mode_get_type (void)
{
        static GType etype = 0;
        
        if (etype == 0) {
                static const GEnumValue values[] = {
                        { CPUFREQ_MODE_GRAPHIC, "CPUFREQ_MODE_GRAPHIC", "mode-graphic" },
                        { CPUFREQ_MODE_TEXT,    "CPUFREQ_MODE_TEXT",    "mode-text" },
                        { CPUFREQ_MODE_BOTH,    "CPUFREQ_MODE_BOTH",    "mode-both" },
                        { 0, NULL, NULL }
                };
                
                etype = g_enum_register_static ("CPUFreqShowMode", values);
        }
        
        return etype;
}

GType
cpufreq_applet_show_text_mode_get_type (void)
{
        static GType etype = 0;

        if (etype == 0) {
                static const GEnumValue values[] = {
                        { CPUFREQ_MODE_TEXT_FREQUENCY,      "CPUFREQ_MODE_TEXT_FREQUENCY",      "mode-text-frequency" },
                        { CPUFREQ_MODE_TEXT_FREQUENCY_UNIT, "CPUFREQ_MODE_TEXT_FREQUENCY_UNIT", "mode-text-frequency-unit" },
                        { CPUFREQ_MODE_TEXT_PERCENTAGE,     "CPUFREQ_MODE_TEXT_PERCENTAGE",     "mode-text-percentage" },
                        { 0, NULL, NULL }
                };

                etype = g_enum_register_static ("CPUFreqShowTextMode", values);
        }

        return etype;
}

static void
cpufreq_applet_style_updated (GtkWidget *widget)
{
        CPUFreqApplet *applet;

        applet = CPUFREQ_APPLET (widget);
        GTK_WIDGET_CLASS (cpufreq_applet_parent_class)->style_updated (widget);

        applet->max_label_width = 0;
        applet->max_perc_width = 0;
        applet->max_unit_width = 0;
}

static void
cpufreq_applet_init (CPUFreqApplet *applet)
{
        applet->prefs = NULL;
        applet->popup = NULL;
        applet->monitor = NULL;

	applet->show_mode = CPUFREQ_MODE_BOTH;
	applet->show_text_mode = CPUFREQ_MODE_TEXT_FREQUENCY_UNIT;

        gp_applet_set_flags (GP_APPLET (applet), GP_APPLET_FLAGS_EXPAND_MINOR);

        applet->box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, SPACING);
        gtk_container_add (GTK_CONTAINER (applet), applet->box);
        gtk_widget_set_valign (applet->box, GTK_ALIGN_CENTER);
        gtk_widget_show (applet->box);

        if (gp_applet_get_orientation (GP_APPLET (applet)) == GTK_ORIENTATION_VERTICAL) {
                gtk_widget_set_halign (applet->box, GTK_ALIGN_CENTER);
        } else {
                gtk_widget_set_halign (applet->box, GTK_ALIGN_START);
        }

        applet->icon = gtk_image_new ();
        gtk_box_pack_start (GTK_BOX (applet->box), applet->icon, FALSE, FALSE, 0);

        applet->labels_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, SPACING);
        gtk_box_pack_start (GTK_BOX (applet->box), applet->labels_box, FALSE, FALSE, 0);
        gtk_widget_show (applet->labels_box);

        applet->label = gtk_label_new (NULL);
        gtk_box_pack_start (GTK_BOX (applet->labels_box), applet->label,
                            FALSE, FALSE, 0);

        applet->unit_label = gtk_label_new (NULL);
        gtk_box_pack_start (GTK_BOX (applet->labels_box), applet->unit_label,
                            FALSE, FALSE, 0);
}

static void
cpufreq_applet_constructed (GObject *object)
{
  G_OBJECT_CLASS (cpufreq_applet_parent_class)->constructed (object);
  cpufreq_applet_setup (CPUFREQ_APPLET (object));
}

static void
cpufreq_applet_placement_changed (GpApplet        *applet,
                                  GtkOrientation   orientation,
                                  GtkPositionType  position)
{
  CPUFreqApplet *self;
  GtkAllocation allocation;
  gint size;

  self = CPUFREQ_APPLET (applet);

  gtk_widget_get_allocation (GTK_WIDGET (self), &allocation);

  if (orientation == GTK_ORIENTATION_VERTICAL)
    {
      size = allocation.width;
      gtk_widget_set_halign (self->box, GTK_ALIGN_CENTER);
    }
  else
    {
      size = allocation.height;
      gtk_widget_set_halign (self->box, GTK_ALIGN_START);
    }

  if (self->size != size)
    {
      self->size = size;
      queue_refresh (self);
    }
}

static void
cpufreq_applet_class_init (CPUFreqAppletClass *self_class)
{
  GObjectClass *object_class;
  GtkWidgetClass *widget_class;
  GpAppletClass *applet_class;

  object_class = G_OBJECT_CLASS (self_class);
  widget_class = GTK_WIDGET_CLASS (self_class);
  applet_class = GP_APPLET_CLASS (self_class);

  object_class->constructed = cpufreq_applet_constructed;
  object_class->dispose = cpufreq_applet_dispose;

  widget_class->size_allocate = cpufreq_applet_size_allocate;
  widget_class->style_updated = cpufreq_applet_style_updated;
  widget_class->get_preferred_width = cpufreq_applet_get_preferred_width;
  widget_class->button_press_event = cpufreq_applet_button_press;
  widget_class->key_press_event = cpufreq_applet_key_press;
           
  applet_class->placement_changed = cpufreq_applet_placement_changed;
}

static void
cpufreq_applet_dispose (GObject *widget)
{
        CPUFreqApplet *applet;
        gint           i;

        applet = CPUFREQ_APPLET (widget);

        if (applet->refresh_id != 0) {
                g_source_remove (applet->refresh_id);
                applet->refresh_id = 0;
        }

        if (applet->monitor) {
                g_object_unref (G_OBJECT (applet->monitor));
                applet->monitor = NULL;
        }

        for (i = 0; i <= 3; i++) {
                if (applet->pixbufs[i]) {
                        g_object_unref (G_OBJECT (applet->pixbufs[i]));
                        applet->pixbufs[i] = NULL;
                }
        }

        if (applet->prefs) {
                g_object_unref (applet->prefs);
                applet->prefs = NULL;
        }

        if (applet->popup) {
                g_object_unref (applet->popup);
                applet->popup = NULL;
        }

        G_OBJECT_CLASS (cpufreq_applet_parent_class)->dispose (widget);
}

static gboolean
refresh_cb (gpointer user_data)
{
        CPUFreqApplet *applet;

        applet = CPUFREQ_APPLET (user_data);

        cpufreq_applet_refresh (applet);
        applet->refresh_id = 0;

        return G_SOURCE_REMOVE;
}

static void
queue_refresh (CPUFreqApplet *applet)
{
        if (applet->refresh_id != 0)
                return;

        applet->refresh_id = g_idle_add (refresh_cb, applet);
        g_source_set_name_by_id (applet->refresh_id, "[cpufreq] refresh_cb");
}

static void
cpufreq_applet_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
        CPUFreqApplet *applet;
        gint           size = 0;

        applet = CPUFREQ_APPLET (widget);

	GTK_WIDGET_CLASS (cpufreq_applet_parent_class)->size_allocate (widget, allocation);

        switch (gp_applet_get_orientation (GP_APPLET (applet))) {
        case GTK_ORIENTATION_VERTICAL:
                size = allocation->width;
                break;
        case GTK_ORIENTATION_HORIZONTAL:
                size = allocation->height;
                break;
        default:
                g_assert_not_reached ();
                break;
        }

        if (size != applet->size) {
                applet->size = size;
                queue_refresh (applet);
        }
}

static gint
get_text_width (const gchar *text)
{
        gint width;
        GtkWidget *label;

        width = 0;
        label = gtk_label_new (text);
        g_object_ref_sink (label);

        gtk_widget_show (label);
        gtk_widget_get_preferred_width (label, &width, NULL);
        g_object_unref (label);

        return width;
}

static gint
cpufreq_applet_get_max_label_width (CPUFreqApplet *applet)
{
	gulong min;
	gulong max;
	gulong freq;
	gint width;
	
	if (applet->max_label_width > 0)
		return applet->max_label_width;

	if (!CPUFREQ_IS_MONITOR (applet->monitor))
		return 0;

	if (!cpufreq_monitor_get_hardware_limits (applet->monitor, &min, &max))
		return 0;

	width = 0;
	for (freq = min; freq <= max; freq += 10000) {
		gchar *freq_text;

		freq_text = cpufreq_utils_get_frequency_label (freq);

		width = MAX (width, get_text_width (freq_text));
		g_free (freq_text);
	}

	applet->max_label_width = width;

	return width;
}

static gint
cpufreq_applet_get_max_perc_width (CPUFreqApplet *applet)
{
	if (applet->max_perc_width > 0)
		return applet->max_perc_width;

	applet->max_perc_width = get_text_width ("100%");

	return applet->max_perc_width;
}

static gint
cpufreq_applet_get_max_unit_width (CPUFreqApplet *applet)
{
	gint w1;
	gint w2;

	if (applet->max_unit_width > 0)
		return applet->max_unit_width;

	w1 = get_text_width ("GHz");
	w2 = get_text_width ("MHz");

	applet->max_unit_width = MAX (w1, w2);

	return applet->max_unit_width;
}

static void
cpufreq_applet_get_preferred_width (GtkWidget *widget,
                                    gint      *minimum_width,
                                    gint      *natural_width)
{
        CPUFreqApplet *applet;
        gint icon_width;
        gint labels_width;
        gint total_width;

        applet = CPUFREQ_APPLET (widget);

        GTK_WIDGET_CLASS (cpufreq_applet_parent_class)->get_preferred_width (widget,
                                                                             minimum_width,
                                                                             natural_width);

        if (gp_applet_get_orientation (GP_APPLET (applet)) == GTK_ORIENTATION_VERTICAL)
                return;

        icon_width = 0;
        labels_width = 0;

        if (applet->show_icon)
                gtk_widget_get_preferred_width (applet->icon, &icon_width, NULL);

        if (applet->show_freq)
                labels_width += cpufreq_applet_get_max_label_width (applet);

        if (applet->show_perc)
                labels_width += cpufreq_applet_get_max_perc_width (applet);

        if (applet->show_unit)
                labels_width += cpufreq_applet_get_max_unit_width (applet);

        if ((applet->show_freq || applet->show_perc) && applet->show_unit)
                labels_width += SPACING;

        if (icon_width != 0) {
                GtkOrientation orientation;

                orientation = gtk_orientable_get_orientation (GTK_ORIENTABLE (applet->box));

                if (orientation == GTK_ORIENTATION_HORIZONTAL) {
                        total_width = icon_width + labels_width;

                        if (icon_width != 0 && labels_width != 0)
                                total_width += SPACING;
                } else {
                        total_width = MAX (icon_width, labels_width);
                }
        } else {
                total_width = labels_width;
        }

        if (*minimum_width < total_width)
                *minimum_width = *natural_width = total_width;
}

static void
cpufreq_applet_menu_popup (CPUFreqApplet *applet,
                           GdkEvent      *event)
{
        GtkWidget *menu;
        GtkPositionType position;
        GdkGravity widget_anchor;
        GdkGravity menu_anchor;

        if (!cpufreq_utils_selector_is_available ())
                return;

        if (!applet->popup) {
                applet->popup = cpufreq_popup_new ();
                cpufreq_popup_set_monitor (applet->popup, applet->monitor);
        }

        menu = cpufreq_popup_get_menu (applet->popup);

        if (!menu)
                return;

        position = gp_applet_get_position (GP_APPLET (applet));

        if (position == GTK_POS_TOP) {
                widget_anchor = GDK_GRAVITY_SOUTH_WEST;
                menu_anchor = GDK_GRAVITY_NORTH_WEST;
        } else if (position == GTK_POS_LEFT) {
                widget_anchor = GDK_GRAVITY_NORTH_EAST;
                menu_anchor = GDK_GRAVITY_NORTH_WEST;
        } else if (position == GTK_POS_RIGHT) {
                widget_anchor = GDK_GRAVITY_NORTH_WEST;
                menu_anchor = GDK_GRAVITY_NORTH_EAST;
        } else if (position == GTK_POS_BOTTOM) {
                widget_anchor = GDK_GRAVITY_NORTH_WEST;
                menu_anchor = GDK_GRAVITY_SOUTH_WEST;
        } else {
                g_assert_not_reached ();
        }

        gtk_menu_popup_at_widget (GTK_MENU (menu),
                                  GTK_WIDGET (applet),
                                  widget_anchor,
                                  menu_anchor,
                                  event);
}

static gboolean
cpufreq_applet_button_press (GtkWidget *widget, GdkEventButton *event)
{
        CPUFreqApplet *applet;

        applet = CPUFREQ_APPLET (widget);

        if (event->button == 2)
                return FALSE;

        if (event->button == 1 &&
            event->type != GDK_2BUTTON_PRESS &&
            event->type != GDK_3BUTTON_PRESS) {
                cpufreq_applet_menu_popup (applet, (GdkEvent *) event);
                
                return TRUE;
        }
        
        return GTK_WIDGET_CLASS (cpufreq_applet_parent_class)->button_press_event (widget, event);
}

static gboolean
cpufreq_applet_key_press (GtkWidget *widget, GdkEventKey *event)
{
        CPUFreqApplet *applet;

        applet = CPUFREQ_APPLET (widget);

        switch (event->keyval) {
        case GDK_KEY_KP_Enter:
        case GDK_KEY_ISO_Enter:
        case GDK_KEY_3270_Enter:
        case GDK_KEY_Return:
        case GDK_KEY_space:
        case GDK_KEY_KP_Space:
                cpufreq_applet_menu_popup (applet, (GdkEvent *) event);

                return TRUE;
        default:
                break;
        }

        return FALSE;
}

static void
cpufreq_applet_preferences_cb (GSimpleAction *action,
                               GVariant      *parameter,
                               gpointer       user_data)
{
	CPUFreqApplet *applet = (CPUFreqApplet *) user_data;
        cpufreq_preferences_dialog_run (applet->prefs,
                                        gtk_widget_get_screen (GTK_WIDGET (applet)));
}

static void
cpufreq_applet_help_cb (GSimpleAction *action,
                        GVariant      *parameter,
                        gpointer       user_data)
{
  gp_applet_show_help (GP_APPLET (user_data), NULL);
}

static void
cpufreq_applet_about_cb (GSimpleAction *action,
                         GVariant      *parameter,
                         gpointer       user_data)
{
  gp_applet_show_about (GP_APPLET (user_data));
}

static void
cpufreq_applet_pixmap_set_image (CPUFreqApplet *applet, gint perc)
{
        gint image;

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
        else if ((perc >= 90) && (perc <= 100))
                image = 3;
        else
                image = 4;

        if (applet->pixbufs[image] == NULL) {
                applet->pixbufs[image] = gdk_pixbuf_new_from_resource_at_scale (cpufreq_icons[image],
                                                                                24,
                                                                                24,
                                                                                TRUE,
                                                                                NULL);
        }

        gtk_image_set_from_pixbuf (GTK_IMAGE (applet->icon), applet->pixbufs[image]);
}

static void
cpufreq_applet_update_visibility (CPUFreqApplet *applet)
{
        CPUFreqShowMode     show_mode;
        CPUFreqShowTextMode show_text_mode;
        gboolean            show_freq = FALSE;
        gboolean            show_perc = FALSE;
        gboolean            show_unit = FALSE;
        gboolean            show_icon = FALSE;
        gboolean            changed = FALSE;
        gboolean            need_update = FALSE;

        show_mode = cpufreq_prefs_get_show_mode (applet->prefs);
        show_text_mode = cpufreq_prefs_get_show_text_mode (applet->prefs);
        
        if (show_mode != CPUFREQ_MODE_GRAPHIC) {
                show_icon = (show_mode == CPUFREQ_MODE_BOTH);

                switch (show_text_mode) {
                case CPUFREQ_MODE_TEXT_FREQUENCY:
                        show_freq = TRUE;
                        break;
                case CPUFREQ_MODE_TEXT_PERCENTAGE:
                        show_perc = TRUE;
                        break;
                case CPUFREQ_MODE_TEXT_FREQUENCY_UNIT:
                        show_freq = TRUE;
                        show_unit = TRUE;
                        break;
                default:
                        g_assert_not_reached ();
                        break;
                }
        } else {
                show_icon = TRUE;
        }

	if (applet->show_mode != show_mode) {
		applet->show_mode = show_mode;
		need_update = TRUE;
	}
	
	if (applet->show_text_mode != show_text_mode) {
		applet->show_text_mode = show_text_mode;
		need_update = TRUE;
	}
	    
        if (show_freq != applet->show_freq) {
                applet->show_freq = show_freq;
                changed = TRUE;
        }

        if (show_perc != applet->show_perc) {
                applet->show_perc = show_perc;
                changed = TRUE;
        }

        if (changed) {
                g_object_set (G_OBJECT (applet->label),
                              "visible",
                              applet->show_freq || applet->show_perc,
                              NULL);
        }

        if (show_unit != applet->show_unit) {
                applet->show_unit = show_unit;
                changed = TRUE;

                g_object_set (G_OBJECT (applet->unit_label),
                              "visible", applet->show_unit,
                              NULL);
        }

        if (show_icon != applet->show_icon) {
                applet->show_icon = show_icon;
                changed = TRUE;

                g_object_set (G_OBJECT (applet->icon),
                              "visible", applet->show_icon,
                              NULL);
        }

        if (changed)
                queue_refresh (applet);

        if (need_update)
                cpufreq_applet_update (applet, applet->monitor);
}

static void
cpufreq_applet_update (CPUFreqApplet *applet, CPUFreqMonitor *monitor)
{
        gchar       *text_mode = NULL;
        gchar       *freq_label, *unit_label;
        gint         freq;
        gint         perc;
        guint        cpu;
        const gchar *governor;

        cpu = cpufreq_monitor_get_cpu (monitor);
        freq = cpufreq_monitor_get_frequency (monitor);
        perc = cpufreq_monitor_get_percentage (monitor);
        governor = cpufreq_monitor_get_governor (monitor);

        freq_label = cpufreq_utils_get_frequency_label (freq);
        unit_label = cpufreq_utils_get_frequency_unit (freq);
        
        if (applet->show_freq) {
                gtk_label_set_text (GTK_LABEL (applet->label), freq_label);
        }

        if (applet->show_perc) {
                gchar *text_perc;

                text_perc = g_strdup_printf ("%d%%", perc);
                gtk_label_set_text (GTK_LABEL (applet->label), text_perc);
                g_free (text_perc);
        }

        if (applet->show_unit) {
                gtk_label_set_text (GTK_LABEL (applet->unit_label), unit_label);
        }

        if (applet->show_icon) {
                cpufreq_applet_pixmap_set_image (applet, perc);
        }

	if (governor) {
		gchar *gov_text;

		gov_text = g_strdup (governor);
		gov_text[0] = g_ascii_toupper (gov_text[0]);
		text_mode = g_strdup_printf ("%s\n%s %s (%d%%)",
					     gov_text, freq_label,
					     unit_label, perc);
		g_free (gov_text);
	}

        g_free (freq_label);
        g_free (unit_label);

	if (text_mode) {
		gchar *text_tip;
		
		text_tip = cpufreq_utils_get_n_cpus () == 1 ?
			g_strdup_printf ("%s", text_mode) :
			g_strdup_printf ("CPU %u - %s", cpu, text_mode);
		g_free (text_mode);
           
		gtk_widget_set_tooltip_text (GTK_WIDGET (applet), text_tip);
		g_free (text_tip);
	}

        queue_refresh (applet);
}

static gint
cpufreq_applet_get_widget_size (CPUFreqApplet *applet,
                                GtkWidget     *widget)
{
        GtkRequisition  req;
        gint            size;

        if (!gtk_widget_get_visible (widget))
                return 0;

        gtk_widget_get_preferred_size (widget, &req, NULL);

        switch (gp_applet_get_orientation (GP_APPLET (applet))) {
        case GTK_ORIENTATION_VERTICAL:
                size = req.width;
                break;
        case GTK_ORIENTATION_HORIZONTAL:
                size = req.height;
                break;
        default:
                g_assert_not_reached ();
        }

        return size;
} 

static void
cpufreq_applet_refresh (CPUFreqApplet *applet)
{
        gint      panel_size, label_size;
        gint      unit_label_size, pixmap_size;
        gint      size_step = 12;
	gboolean  horizontal;

        panel_size = applet->size - 1; /* 1 pixel margin */

	horizontal = (gp_applet_get_orientation (GP_APPLET (applet)) == GTK_ORIENTATION_HORIZONTAL);

        /* We want a fixed label size, the biggest */
	if (horizontal)
		label_size = cpufreq_applet_get_widget_size (applet, applet->label);
	else
		label_size = cpufreq_applet_get_max_label_width (applet);

	if (horizontal)
		unit_label_size = cpufreq_applet_get_widget_size (applet, applet->unit_label);
	else
		unit_label_size = cpufreq_applet_get_max_unit_width (applet);

        pixmap_size = cpufreq_applet_get_widget_size (applet, applet->icon);

        if (horizontal) {
                gint total_height;

                total_height = pixmap_size + label_size;

                if (applet->show_icon && (applet->show_freq || applet->show_perc))
                        total_height += SPACING;

                if (total_height <= panel_size) {
                        gtk_orientable_set_orientation (GTK_ORIENTABLE (applet->box),
                                                        GTK_ORIENTATION_VERTICAL);
                } else {
                        gtk_orientable_set_orientation (GTK_ORIENTABLE (applet->box),
                                                        GTK_ORIENTATION_HORIZONTAL);
                }

                gtk_orientable_set_orientation (GTK_ORIENTABLE (applet->labels_box),
                                                GTK_ORIENTATION_HORIZONTAL);
        } else {
                gint total_width;

                total_width = pixmap_size + label_size + unit_label_size;

                if (applet->show_icon && (applet->show_freq || applet->show_perc))
                        total_width += SPACING;

                if ((applet->show_freq || applet->show_perc) && applet->show_unit)
                        total_width += SPACING;

                if (total_width <= panel_size) {
                        gtk_orientable_set_orientation (GTK_ORIENTABLE (applet->box),
                                                        GTK_ORIENTATION_HORIZONTAL);
                        gtk_orientable_set_orientation (GTK_ORIENTABLE (applet->labels_box),
                                                        GTK_ORIENTATION_HORIZONTAL);
                } else if ((label_size + unit_label_size) <= (panel_size - size_step)) {
                        gtk_orientable_set_orientation (GTK_ORIENTABLE (applet->box),
                                                        GTK_ORIENTATION_VERTICAL);
                        gtk_orientable_set_orientation (GTK_ORIENTABLE (applet->labels_box),
                                                        GTK_ORIENTATION_HORIZONTAL);
                } else {
                        gtk_orientable_set_orientation (GTK_ORIENTABLE (applet->box),
                                                        GTK_ORIENTATION_VERTICAL);
                        gtk_orientable_set_orientation (GTK_ORIENTABLE (applet->labels_box),
                                                        GTK_ORIENTATION_VERTICAL);
                }
        }
}

/* Preferences callbacks */
static void
cpufreq_applet_prefs_cpu_changed (CPUFreqPrefs  *prefs,
				  GParamSpec    *arg1,
				  CPUFreqApplet *applet)
{
	cpufreq_monitor_set_cpu (applet->monitor,
				 cpufreq_prefs_get_cpu (applet->prefs));
}

static void
cpufreq_applet_prefs_show_mode_changed (CPUFreqPrefs  *prefs,
                                        GParamSpec    *arg1,
                                        CPUFreqApplet *applet)
{
        cpufreq_applet_update_visibility (applet);
}

static void
cpufreq_applet_setup (CPUFreqApplet *applet)
{
	const char *menu_resource;
	GAction *action;
	AtkObject      *atk_obj;
	GSettings *settings;

        settings = gp_applet_settings_new (GP_APPLET (applet), "org.gnome.gnome-applets.cpufreq");
        applet->prefs = cpufreq_prefs_new (applet, settings);

	g_signal_connect (G_OBJECT (applet->prefs),
			  "notify::cpu",
			  G_CALLBACK (cpufreq_applet_prefs_cpu_changed),
			  (gpointer) applet);
        g_signal_connect (G_OBJECT (applet->prefs),
                          "notify::show-mode",
                          G_CALLBACK (cpufreq_applet_prefs_show_mode_changed),
                          (gpointer) applet);
        g_signal_connect (G_OBJECT (applet->prefs),
                          "notify::show-text-mode",
                          G_CALLBACK (cpufreq_applet_prefs_show_mode_changed),
                          (gpointer) applet);

        /* Monitor */
        applet->monitor = cpufreq_monitor_new (cpufreq_prefs_get_cpu (applet->prefs));
        cpufreq_monitor_run (applet->monitor);
        g_signal_connect_swapped (G_OBJECT (applet->monitor), "changed",
                                  G_CALLBACK (cpufreq_applet_update),
                                  (gpointer) applet);
           
        /* Setup the menus */
        menu_resource = GRESOURCE_PREFIX "/ui/cpufreq-applet-menu.ui";
        gp_applet_setup_menu_from_resource (GP_APPLET (applet),
                                            menu_resource,
                                            cpufreq_applet_menu_actions);

        action = gp_applet_menu_lookup_action (GP_APPLET (applet), "preferences");
	g_object_bind_property (applet, "locked-down", action, "enabled",
                          G_BINDING_DEFAULT|G_BINDING_INVERT_BOOLEAN|G_BINDING_SYNC_CREATE);

        atk_obj = gtk_widget_get_accessible (GTK_WIDGET (applet));

        if (GTK_IS_ACCESSIBLE (atk_obj)) {
                atk_object_set_name (atk_obj, _("CPU Frequency Scaling Monitor"));
                atk_object_set_description (atk_obj, _("This utility shows the current CPU Frequency"));
        }

	cpufreq_applet_update_visibility (applet);

	gtk_widget_show (GTK_WIDGET (applet));
}

void
cpufreq_applet_setup_about (GtkAboutDialog *dialog)
{
  const char *comments;
  const char **authors;
  const char **documenters;
  const char **artists;
  const char *copyright;

  comments = _("This utility shows the current CPU Frequency Scaling.");

  authors = (const char *[])
    {
      "Carlos Garcia Campos <carlosgc@gnome.org>",
      NULL
    };

  documenters = (const char *[])
    {
      "Carlos Garcia Campos <carlosgc@gnome.org>",
      "Davyd Madeley <davyd@madeley.id.au>",
      NULL
    };

  artists = (const char *[])
    {
      "Pablo Arroyo Loma <zzioma@yahoo.es>",
      NULL
    };

  copyright = "\xC2\xA9 2004 Carlos Garcia Campos";

  gtk_about_dialog_set_comments (dialog, comments);

  gtk_about_dialog_set_authors (dialog, authors);
  gtk_about_dialog_set_documenters (dialog, documenters);
  gtk_about_dialog_set_artists (dialog, artists);
  gtk_about_dialog_set_translator_credits (dialog, _("translator-credits"));
  gtk_about_dialog_set_copyright (dialog, copyright);
}
