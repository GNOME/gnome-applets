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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * Authors : Carlos García Campos <carlosgc@gnome.org>
 */

#include "config.h"
#include "cpufreq-prefs.h"

#include <gtk/gtk.h>
#include <glib/gi18n-lib.h>

#include "cpufreq-utils.h"

enum {
	PROP_0,
	PROP_CPU,
	PROP_SHOW_MODE,
	PROP_SHOW_TEXT_MODE,
};

struct _CPUFreqPrefs
{
	GObject parent;

	guint               cpu;
	CPUFreqShowMode     show_mode;
	CPUFreqShowTextMode show_text_mode;

	CPUFreqApplet *applet;
	GSettings *settings;

	/* Preferences dialog */
	GtkWidget *dialog;
	GtkWidget *show_freq;
	GtkWidget *show_unit;
	GtkWidget *show_perc;
	GtkWidget *cpu_combo;
	GtkWidget *monitor_settings_box;
	GtkWidget *show_mode_combo;
};

static void cpufreq_prefs_finalize                  (GObject           *object);

static void cpufreq_prefs_set_property              (GObject           *object,
						     guint              prop_id,
						     const GValue      *value,
						     GParamSpec        *pspec);
static void cpufreq_prefs_get_property              (GObject           *object,
						     guint              prop_id,
						     GValue            *value,
						     GParamSpec        *pspec);

static void cpufreq_prefs_dialog_update_sensitivity (CPUFreqPrefs      *prefs);

G_DEFINE_TYPE (CPUFreqPrefs, cpufreq_prefs, G_TYPE_OBJECT)

static void
cpufreq_prefs_init (CPUFreqPrefs *prefs)
{
}

static void
cpufreq_prefs_class_init (CPUFreqPrefsClass *klass)
{
	GObjectClass *g_object_class = G_OBJECT_CLASS (klass);

	g_object_class->set_property = cpufreq_prefs_set_property;
	g_object_class->get_property = cpufreq_prefs_get_property;

	/* Properties */
	g_object_class_install_property (g_object_class,
					 PROP_CPU,
					 g_param_spec_uint ("cpu",
							    "CPU",
							    "The monitored cpu",
							    0,
							    G_MAXUINT,
							    0,
							    G_PARAM_READWRITE));
	g_object_class_install_property (g_object_class,
					 PROP_SHOW_MODE,
					 g_param_spec_enum ("show-mode",
							    "ShowMode",
							    "The applet show mode",
							    CPUFREQ_TYPE_SHOW_MODE,
							    CPUFREQ_MODE_BOTH,
							    G_PARAM_READWRITE));
	g_object_class_install_property (g_object_class,
					 PROP_SHOW_TEXT_MODE,
					 g_param_spec_enum ("show-text-mode",
							    "ShowTextMode",
							    "The applet show text mode",
							    CPUFREQ_TYPE_SHOW_TEXT_MODE,
							    CPUFREQ_MODE_TEXT_FREQUENCY_UNIT,
							    G_PARAM_READWRITE));

	g_object_class->finalize = cpufreq_prefs_finalize;
}

static void
cpufreq_prefs_finalize (GObject *object)
{
	CPUFreqPrefs *prefs = CPUFREQ_PREFS (object);

	g_clear_object (&prefs->settings);
	g_clear_pointer (&prefs->dialog, gtk_widget_destroy);

	G_OBJECT_CLASS (cpufreq_prefs_parent_class)->finalize (object);
}

static void
cpufreq_prefs_set_property (GObject      *object,
			    guint         prop_id,
			    const GValue *value,
			    GParamSpec   *pspec)
{
	CPUFreqPrefs *prefs = CPUFREQ_PREFS (object);
	gboolean      update_sensitivity = FALSE;

	switch (prop_id) {
	case PROP_CPU: {
		guint cpu;

		cpu = g_value_get_uint (value);

		if (prefs->cpu != cpu) {
			prefs->cpu = cpu;
			g_settings_set_int (prefs->settings, "cpu", cpu);
		}
	}
		break;
	case PROP_SHOW_MODE: {
		CPUFreqShowMode mode;

		mode = g_value_get_enum (value);
		if (prefs->show_mode != mode) {
			update_sensitivity = TRUE;
			prefs->show_mode = mode;
			g_settings_set_enum (prefs->settings, "show-mode", mode);
		}
	}
		break;
	case PROP_SHOW_TEXT_MODE: {
		CPUFreqShowTextMode mode;

		mode = g_value_get_enum (value);
		if (prefs->show_text_mode != mode) {
			update_sensitivity = TRUE;
			prefs->show_text_mode = mode;
			g_settings_set_enum (prefs->settings, "show-text-mode", mode);
		}
	}
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}

	if (prefs->dialog && update_sensitivity)
		cpufreq_prefs_dialog_update_sensitivity (prefs);
}

static void
cpufreq_prefs_get_property (GObject    *object,
			    guint       prop_id,
			    GValue     *value,
			    GParamSpec *pspec)
{
	CPUFreqPrefs *prefs = CPUFREQ_PREFS (object);

	switch (prop_id) {
	case PROP_CPU:
		g_value_set_uint (value, prefs->cpu);
		break;
	case PROP_SHOW_MODE:
		g_value_set_enum (value, prefs->show_mode);
		break;
	case PROP_SHOW_TEXT_MODE:
		g_value_set_enum (value, prefs->show_text_mode);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}
}

static void
cpufreq_prefs_setup (CPUFreqPrefs *prefs)
{
	g_assert (G_IS_SETTINGS (prefs->settings));

	prefs->cpu = g_settings_get_int (prefs->settings, "cpu");
	prefs->show_mode = g_settings_get_enum (prefs->settings, "show-mode");
	prefs->show_text_mode = g_settings_get_enum (prefs->settings, "show-text-mode");
}

CPUFreqPrefs *
cpufreq_prefs_new (CPUFreqApplet *applet,
                   GSettings     *settings)
{
	CPUFreqPrefs *prefs;

	g_return_val_if_fail (settings != NULL, NULL);

	prefs = CPUFREQ_PREFS (g_object_new (CPUFREQ_TYPE_PREFS, NULL));
	prefs->applet = applet;
	prefs->settings = g_object_ref (settings);

	cpufreq_prefs_setup (prefs);

	return prefs;
}

/* Public Methods */
guint
cpufreq_prefs_get_cpu (CPUFreqPrefs *prefs)
{
	g_return_val_if_fail (CPUFREQ_IS_PREFS (prefs), 0);
	
	return MIN (prefs->cpu, cpufreq_utils_get_n_cpus () - 1);
}

CPUFreqShowMode
cpufreq_prefs_get_show_mode (CPUFreqPrefs *prefs)
{
	g_return_val_if_fail (CPUFREQ_IS_PREFS (prefs),
			      CPUFREQ_MODE_BOTH);

	return prefs->show_mode;
}

CPUFreqShowTextMode
cpufreq_prefs_get_show_text_mode (CPUFreqPrefs *prefs)
{
	g_return_val_if_fail (CPUFREQ_IS_PREFS (prefs),
			      CPUFREQ_MODE_TEXT_FREQUENCY_UNIT);

	return prefs->show_text_mode;
}

/* Preferences Dialog */
static void
cpufreq_prefs_dialog_show_freq_toggled (GtkWidget *show_freq, CPUFreqPrefs *prefs)
{
	CPUFreqShowTextMode show_text_mode;

        if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (show_freq))) {
		GtkWidget *show_unit = prefs->show_unit;

                if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (show_unit)))
                        show_text_mode = CPUFREQ_MODE_TEXT_FREQUENCY_UNIT;
                else
                        show_text_mode = CPUFREQ_MODE_TEXT_FREQUENCY;

		g_object_set (G_OBJECT (prefs),
			      "show-text-mode", show_text_mode,
			      NULL);
	}
}	

static void
cpufreq_prefs_dialog_show_unit_toggled (GtkWidget *show_unit, CPUFreqPrefs *prefs)
{
	CPUFreqShowTextMode show_text_mode;
           
        if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (show_unit))) {
                show_text_mode = CPUFREQ_MODE_TEXT_FREQUENCY_UNIT;
        } else {
                show_text_mode = CPUFREQ_MODE_TEXT_FREQUENCY;
        }

	g_object_set (G_OBJECT (prefs),
		      "show-text-mode", show_text_mode,
		      NULL);
}

static void
cpufreq_prefs_dialog_show_perc_toggled (GtkWidget *show_perc, CPUFreqPrefs *prefs)
{

	CPUFreqShowTextMode show_text_mode;
	
        if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (show_perc))) {
                /* Show cpu usage in percentage */
                show_text_mode = CPUFREQ_MODE_TEXT_PERCENTAGE;

		g_object_set (G_OBJECT (prefs),
			      "show-text-mode", show_text_mode,
			      NULL);
        }
}

static void
cpufreq_prefs_dialog_cpu_number_changed (GtkWidget *cpu_combo, CPUFreqPrefs *prefs)
{
        gint cpu;

        cpu = gtk_combo_box_get_active (GTK_COMBO_BOX (prefs->cpu_combo));

        if (cpu >= 0) {
		g_object_set (G_OBJECT (prefs),
			      "cpu", cpu,
			      NULL);
        }
}

static void
cpufreq_prefs_dialog_show_mode_changed (GtkWidget *show_mode_combo, CPUFreqPrefs *prefs)
{
	CPUFreqShowMode show_mode;
           
        show_mode = gtk_combo_box_get_active (GTK_COMBO_BOX (show_mode_combo));
	g_object_set (G_OBJECT (prefs),
		      "show-mode", show_mode,
		      NULL);
}

static void
cpufreq_prefs_dialog_response_cb (CPUFreqPrefs *prefs,
				  gint          response,
				  GtkDialog    *dialog)
{
        if (response == GTK_RESPONSE_HELP) {
                gp_applet_show_help (GP_APPLET (prefs->applet),
                                     "cpufreq-applet-prefs");
        } else {
                gtk_widget_destroy (prefs->dialog);
                prefs->dialog = NULL;
        }
}

static void
cpufreq_prefs_dialog_update_visibility (CPUFreqPrefs *prefs)
{
	if (cpufreq_utils_get_n_cpus () > 1)
		gtk_widget_show (prefs->monitor_settings_box);
	else
		gtk_widget_hide (prefs->monitor_settings_box);
}

static void
cpufreq_prefs_dialog_update_sensitivity (CPUFreqPrefs *prefs)
{
	gtk_widget_set_sensitive (prefs->show_mode_combo,
				  g_settings_is_writable (prefs->settings, "show-mode"));
	
	if (prefs->show_mode != CPUFREQ_MODE_GRAPHIC) {
		gboolean key_writable;
		
		key_writable = g_settings_is_writable (prefs->settings, "show-text-mode");
		
		gtk_widget_set_sensitive (prefs->show_freq,
					  (TRUE && key_writable));
		gtk_widget_set_sensitive (prefs->show_perc,
					  (TRUE && key_writable));
		
		if (prefs->show_text_mode == CPUFREQ_MODE_TEXT_PERCENTAGE)
			gtk_widget_set_sensitive (prefs->show_unit,
						  FALSE);
		else
			gtk_widget_set_sensitive (prefs->show_unit,
						  (TRUE && key_writable));
	} else {
		gtk_widget_set_sensitive (prefs->show_freq, FALSE);
		gtk_widget_set_sensitive (prefs->show_unit, FALSE);
		gtk_widget_set_sensitive (prefs->show_perc, FALSE);
	}
}

static void
cpufreq_prefs_dialog_update (CPUFreqPrefs *prefs)
{
	if (cpufreq_utils_get_n_cpus () > 1) {
		gtk_combo_box_set_active (GTK_COMBO_BOX (prefs->cpu_combo),
					  MIN (prefs->cpu, cpufreq_utils_get_n_cpus () - 1));
	}

	gtk_combo_box_set_active (GTK_COMBO_BOX (prefs->show_mode_combo),
				  prefs->show_mode);
	
	switch (prefs->show_text_mode) {
	case CPUFREQ_MODE_TEXT_FREQUENCY:
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (prefs->show_freq),
					      TRUE);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (prefs->show_unit),
					      FALSE);

		break;
	case CPUFREQ_MODE_TEXT_FREQUENCY_UNIT:
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (prefs->show_freq),
					      TRUE);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (prefs->show_unit),
					      TRUE);

		break;
	case CPUFREQ_MODE_TEXT_PERCENTAGE:
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (prefs->show_perc),
					      TRUE);

		break;
	default:
		g_assert_not_reached ();
		break;
	}
}

static void
cpufreq_prefs_dialog_cpu_combo_setup (CPUFreqPrefs *prefs)
{
	GtkListStore    *model;
	GtkTreeIter      iter;
	GtkCellRenderer *renderer;
	guint            i;
	guint            n_cpus;

	model = gtk_list_store_new (1, G_TYPE_STRING);
	gtk_combo_box_set_model (GTK_COMBO_BOX (prefs->cpu_combo),
				 GTK_TREE_MODEL (model));

	n_cpus = cpufreq_utils_get_n_cpus ();
	
	for (i = 0; i < n_cpus; i++) {
		gchar *text_label;
		
		text_label = g_strdup_printf ("CPU %u", i);

		gtk_list_store_append (model, &iter);
		gtk_list_store_set (model, &iter,
				    0, text_label,
				    -1);

		g_free (text_label);
	}

	g_object_unref (model);

	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_clear (GTK_CELL_LAYOUT (prefs->cpu_combo));
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (prefs->cpu_combo),
				    renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (prefs->cpu_combo),
					renderer,
					"text", 0,
					NULL);
}

static void
cpufreq_prefs_dialog_show_mode_combo_setup (CPUFreqPrefs *prefs)
{
	GtkListStore    *model;
	GtkTreeIter      iter;
	GtkCellRenderer *renderer;

	model = gtk_list_store_new (1, G_TYPE_STRING);
	gtk_combo_box_set_model (GTK_COMBO_BOX (prefs->show_mode_combo),
				 GTK_TREE_MODEL (model));

	gtk_list_store_append (model, &iter);
	gtk_list_store_set (model, &iter,
			    0, _("Graphic"),
			    -1);

	gtk_list_store_append (model, &iter);
	gtk_list_store_set (model, &iter,
			    0, _("Text"),
			    -1);

	gtk_list_store_append (model, &iter);
	gtk_list_store_set (model, &iter,
			    0, _("Graphic and Text"),
			    -1);

	g_object_unref (model);

	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_clear (GTK_CELL_LAYOUT (prefs->show_mode_combo));
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (prefs->show_mode_combo),
				    renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (prefs->show_mode_combo),
					renderer,
					"text", 0,
					NULL);
}

static void
cpufreq_prefs_dialog_create (CPUFreqPrefs *prefs)
{
	GtkBuilder *builder;

	builder = gtk_builder_new ();
	gtk_builder_set_translation_domain (builder, GETTEXT_PACKAGE);
	gtk_builder_add_from_resource (builder, GRESOURCE_PREFIX "/ui/cpufreq-preferences.ui", NULL);

	prefs->dialog = GTK_WIDGET (gtk_builder_get_object (builder, "prefs_dialog"));

	prefs->cpu_combo = GTK_WIDGET (gtk_builder_get_object (builder, "prefs_cpu_number"));
	
	prefs->show_mode_combo = GTK_WIDGET (gtk_builder_get_object (builder, "prefs_show_mode"));
	
	prefs->show_freq = GTK_WIDGET (gtk_builder_get_object (builder, "prefs_show_freq"));
	prefs->show_unit = GTK_WIDGET (gtk_builder_get_object (builder, "prefs_show_unit"));
	prefs->show_perc = GTK_WIDGET (gtk_builder_get_object (builder, "prefs_show_perc"));

	prefs->monitor_settings_box = GTK_WIDGET (gtk_builder_get_object (builder, "monitor_settings_box"));

	g_object_unref (builder);

	cpufreq_prefs_dialog_show_mode_combo_setup (prefs);
	
	if (cpufreq_utils_get_n_cpus () > 1)
		cpufreq_prefs_dialog_cpu_combo_setup (prefs);
		
	g_signal_connect_swapped (prefs->dialog, "response",
				  G_CALLBACK (cpufreq_prefs_dialog_response_cb),
				  (gpointer) prefs);
	
	g_signal_connect (prefs->show_freq, "toggled",
			  G_CALLBACK (cpufreq_prefs_dialog_show_freq_toggled),
			  (gpointer) prefs);
	g_signal_connect (prefs->show_unit, "toggled",
			  G_CALLBACK (cpufreq_prefs_dialog_show_unit_toggled),
			  (gpointer) prefs);
	g_signal_connect (prefs->show_perc, "toggled",
			  G_CALLBACK (cpufreq_prefs_dialog_show_perc_toggled),
			  (gpointer) prefs);
	g_signal_connect (prefs->cpu_combo, "changed",
			  G_CALLBACK (cpufreq_prefs_dialog_cpu_number_changed),
			  (gpointer) prefs);
	g_signal_connect (prefs->show_mode_combo, "changed",
			  G_CALLBACK (cpufreq_prefs_dialog_show_mode_changed),
			  (gpointer) prefs);
}

void 
cpufreq_preferences_dialog_run (CPUFreqPrefs *prefs, GdkScreen *screen)
{
        g_return_if_fail (CPUFREQ_IS_PREFS (prefs));

        if (prefs->dialog) {
                /* Dialog already exist, only show it */
                gtk_window_present (GTK_WINDOW (prefs->dialog));
                return;
        }

	cpufreq_prefs_dialog_create (prefs);
        gtk_window_set_screen (GTK_WINDOW (prefs->dialog), screen);

	cpufreq_prefs_dialog_update_sensitivity (prefs);
	cpufreq_prefs_dialog_update_visibility (prefs);
	cpufreq_prefs_dialog_update (prefs);

	gtk_widget_show (prefs->dialog);
}
