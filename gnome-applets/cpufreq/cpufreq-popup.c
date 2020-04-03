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
#include "cpufreq-popup.h"

#include <glib/gi18n-lib.h>

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <stdlib.h>
#include <string.h>

#include "cpufreq-selector-gen.h"
#include "cpufreq-utils.h"

struct _CPUFreqPopup
{
	GObject              parent;

	CPUFreqSelectorGen  *selector;

	GtkUIManager        *ui_manager;
	GSList              *radio_group;
	
	GtkActionGroup      *freqs_group;
	GSList              *freqs_actions;
	
	GtkActionGroup      *govs_group;
	GSList              *govs_actions;

	guint                merge_id;
	gboolean             need_build;
	gboolean             show_freqs;

	CPUFreqMonitor      *monitor;
};

static void cpufreq_popup_finalize   (GObject           *object);

G_DEFINE_TYPE (CPUFreqPopup, cpufreq_popup, G_TYPE_OBJECT)

static const gchar *ui_popup =
"<ui>"
"    <popup name=\"CPUFreqSelectorPopup\" action=\"PopupAction\">"
"        <placeholder name=\"FreqsItemsGroup\">"
"        </placeholder>"
"        <separator />"
"        <placeholder name=\"GovsItemsGroup\">"
"        </placeholder>"
"    </popup>"
"</ui>";

#define FREQS_PLACEHOLDER_PATH "/CPUFreqSelectorPopup/FreqsItemsGroup"
#define GOVS_PLACEHOLDER_PATH "/CPUFreqSelectorPopup/GovsItemsGroup"

static void
set_frequency_cb (GObject      *source,
                  GAsyncResult *result,
                  gpointer      user_data)
{
	GError *error;

	error = NULL;
	cpufreq_selector_gen_call_set_frequency_finish (CPUFREQ_SELECTOR_GEN (source),
	                                                result, &error);

	if (error != NULL) {
		g_warning ("%s", error->message);
		g_error_free (error);
	}
}

static void
set_governor_cb (GObject      *source,
                 GAsyncResult *result,
                 gpointer      user_data)
{
	GError *error;

	error = NULL;
	cpufreq_selector_gen_call_set_governor_finish (CPUFREQ_SELECTOR_GEN (source),
	                                               result,
	                                               &error);

	if (error != NULL) {
		g_warning ("%s", error->message);
		g_error_free (error);
	}
}

static void
cpufreq_popup_init (CPUFreqPopup *popup)
{
	GError *error;

	error = NULL;
	popup->selector = cpufreq_selector_gen_proxy_new_for_bus_sync (G_BUS_TYPE_SYSTEM,
	                                                               G_DBUS_PROXY_FLAGS_NONE,
	                                                               "org.gnome.CPUFreqSelector",
	                                                               "/org/gnome/cpufreq_selector/selector",
	                                                               NULL,
	                                                               &error);

	if (error != NULL) {
		g_warning ("%s", error->message);
		g_error_free (error);
	}

	popup->ui_manager = gtk_ui_manager_new ();
	popup->need_build = TRUE;

	gtk_ui_manager_add_ui_from_string (popup->ui_manager,
					   ui_popup, -1, NULL);
}

static void
cpufreq_popup_class_init (CPUFreqPopupClass *klass)
{
	GObjectClass *g_object_class = G_OBJECT_CLASS (klass);

	g_object_class->finalize = cpufreq_popup_finalize;
}

static void
cpufreq_popup_finalize (GObject *object)
{
	CPUFreqPopup *popup = CPUFREQ_POPUP (object);

	g_clear_object (&popup->selector);
	g_clear_object (&popup->ui_manager);
	g_clear_object (&popup->freqs_group);
	g_clear_object (&popup->govs_group);
	g_clear_object (&popup->monitor);

	g_clear_pointer (&popup->freqs_actions, g_slist_free);
	g_clear_pointer (&popup->govs_actions, g_slist_free);

	G_OBJECT_CLASS (cpufreq_popup_parent_class)->finalize (object);
}

CPUFreqPopup *
cpufreq_popup_new (void)
{
	CPUFreqPopup *popup;

	popup = CPUFREQ_POPUP (g_object_new (CPUFREQ_TYPE_POPUP,
					     NULL));

	return popup;
}

/* Public methods */
void
cpufreq_popup_set_monitor (CPUFreqPopup   *popup,
			   CPUFreqMonitor *monitor)
{
	g_return_if_fail (CPUFREQ_IS_POPUP (popup));
	g_return_if_fail (CPUFREQ_IS_MONITOR (monitor));

	if (popup->monitor == monitor)
		return;
	
	if (popup->monitor)
		g_object_unref (popup->monitor);
	popup->monitor = g_object_ref (monitor);
}

static void
cpufreq_popup_frequencies_menu_activate (GtkAction    *action,
					 CPUFreqPopup *popup)
{
	const gchar     *name;
	guint            cpu;
	guint            freq;

	if (!gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action)))
		return;

	if (popup->selector == NULL)
		return;

	cpu = cpufreq_monitor_get_cpu (popup->monitor);
	name = gtk_action_get_name (action);
	freq = (guint) atoi (name + strlen ("Frequency"));

	cpufreq_selector_gen_call_set_frequency (popup->selector,
	                                         cpu,
	                                         freq,
	                                         NULL,
	                                         set_frequency_cb,
	                                         popup);
}

static void
cpufreq_popup_governors_menu_activate (GtkAction    *action,
				       CPUFreqPopup *popup)
{
	const gchar     *name;
	guint            cpu;
	const gchar     *governor;

	if (!gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action)))
		return;

	if (popup->selector == NULL)
		return;
	
	cpu = cpufreq_monitor_get_cpu (popup->monitor);
	name = gtk_action_get_name (action);
	governor = name + strlen ("Governor");

	cpufreq_selector_gen_call_set_governor (popup->selector,
	                                        cpu,
	                                        governor,
	                                        NULL,
	                                        set_governor_cb,
	                                        popup);
}

static void
cpufreq_popup_menu_add_action (CPUFreqPopup   *popup,
			       const gchar    *menu,
			       GtkActionGroup *action_group,
			       const gchar    *action_name,
			       const gchar    *label,
			       gboolean        sensitive)
{
	GtkToggleAction *action;
	gchar           *name;

	name = g_strdup_printf ("%s%s", menu, action_name);
	
	action = g_object_new (GTK_TYPE_RADIO_ACTION,
			       "name", name,
			       "label", label,
			       NULL);

	gtk_action_set_sensitive (GTK_ACTION (action), sensitive);
	
	gtk_radio_action_set_group (GTK_RADIO_ACTION (action), popup->radio_group);
	popup->radio_group = gtk_radio_action_get_group (GTK_RADIO_ACTION (action));
	
	if (g_ascii_strcasecmp (menu, "Frequency") == 0) {
		popup->freqs_actions = g_slist_prepend (popup->freqs_actions,
		                                        (gpointer) action);

		g_signal_connect (action, "activate",
				  G_CALLBACK (cpufreq_popup_frequencies_menu_activate),
				  (gpointer) popup);
	} else if (g_ascii_strcasecmp (menu, "Governor") == 0) {
		popup->govs_actions = g_slist_prepend (popup->govs_actions,
		                                       (gpointer) action);

		g_signal_connect (action, "activate",
				  G_CALLBACK (cpufreq_popup_governors_menu_activate),
				  (gpointer) popup);
	}

	gtk_action_group_add_action (action_group, GTK_ACTION (action));
	g_object_unref (action);
	
	g_free (name);
}

static void
frequencies_menu_create_actions (CPUFreqPopup *popup)
{
	GList *available_freqs;

	available_freqs = cpufreq_monitor_get_available_frequencies (popup->monitor);

	while (available_freqs) {
		const gchar *text;
		gchar       *freq_text;
		gchar       *label;
		gchar       *unit;
		gint         freq;
		
		text = (const gchar *) available_freqs->data;
		freq = atoi (text);

		freq_text = cpufreq_utils_get_frequency_label (freq);
		unit = cpufreq_utils_get_frequency_unit (freq);

		label = g_strdup_printf ("%s %s", freq_text, unit);
		g_free (freq_text);
		g_free (unit);

		cpufreq_popup_menu_add_action (popup,
					       "Frequency", 
					       popup->freqs_group,
					       text, label, TRUE);
		g_free (label);

		available_freqs = g_list_next (available_freqs);
	}
}

static void
governors_menu_create_actions (CPUFreqPopup *popup)
{
	GList *available_govs;

	available_govs = cpufreq_monitor_get_available_governors (popup->monitor);
	available_govs = g_list_sort (available_govs, (GCompareFunc)g_ascii_strcasecmp);

	while (available_govs) {
		const gchar *governor;
		gchar       *label;

		governor = (const gchar *) available_govs->data;
		if (g_ascii_strcasecmp (governor, "userspace") == 0) {
			popup->show_freqs = TRUE;
			available_govs = g_list_next (available_govs);
			continue;
		}
		
		label = g_strdup (governor);
		label[0] = g_ascii_toupper (label[0]);
		
		cpufreq_popup_menu_add_action (popup,
					       "Governor",
					       popup->govs_group,
					       governor, label, TRUE);
		g_free (label);

		available_govs = g_list_next (available_govs);
	}
}

static void
cpufreq_popup_build_ui (CPUFreqPopup *popup,
			GSList       *actions,
			const gchar  *menu_path)
{
	GSList *l = NULL;
	
	for (l = actions; l && l->data; l = g_slist_next (l)) { 		
		GtkAction *action;
		gchar     *name = NULL;
		gchar     *label = NULL;

		action = (GtkAction *) l->data;

		g_object_get (G_OBJECT (action),
			      "name", &name,
			      "label", &label,
			      NULL);

		gtk_ui_manager_add_ui (popup->ui_manager,
				       popup->merge_id,
				       menu_path,
				       label, name,
				       GTK_UI_MANAGER_MENUITEM,
				       FALSE);
		
		g_free (name);
		g_free (label);
	}
}

static void
cpufreq_popup_build_frequencies_menu (CPUFreqPopup *popup,
				      const gchar  *path)
{
	if (!popup->freqs_group) {
		GtkActionGroup *action_group;

		action_group = gtk_action_group_new ("FreqsActions");
		popup->freqs_group = action_group;
		gtk_action_group_set_translation_domain (action_group, NULL);

		frequencies_menu_create_actions (popup);
		popup->freqs_actions = g_slist_reverse (popup->freqs_actions);
		gtk_ui_manager_insert_action_group (popup->ui_manager,
						    action_group, 0);
	}

	cpufreq_popup_build_ui (popup, popup->freqs_actions, path);
}

static void
cpufreq_popup_build_governors_menu (CPUFreqPopup *popup,
				    const gchar  *path)
{
	if (!popup->govs_group) {
		GtkActionGroup *action_group;

		action_group = gtk_action_group_new ("GovsActions");
		popup->govs_group = action_group;
		gtk_action_group_set_translation_domain (action_group, NULL);

		governors_menu_create_actions (popup);
		popup->govs_actions = g_slist_reverse (popup->govs_actions);
		gtk_ui_manager_insert_action_group (popup->ui_manager, action_group, 1);
	}

	cpufreq_popup_build_ui (popup, popup->govs_actions, path);
}

static void
cpufreq_popup_build_menu (CPUFreqPopup *popup)
{
	if (popup->merge_id > 0) {
		gtk_ui_manager_remove_ui (popup->ui_manager, popup->merge_id);
		gtk_ui_manager_ensure_update (popup->ui_manager);
	}
	
	popup->merge_id = gtk_ui_manager_new_merge_id (popup->ui_manager);
		
	cpufreq_popup_build_frequencies_menu (popup, FREQS_PLACEHOLDER_PATH);
	cpufreq_popup_build_governors_menu (popup, GOVS_PLACEHOLDER_PATH);

	gtk_action_group_set_visible (popup->freqs_group, popup->show_freqs);
}

static void
cpufreq_popup_menu_set_active_action (CPUFreqPopup   *popup,
				      GtkActionGroup *action_group,
				      const gchar    *prefix,
				      const gchar    *item)
{
	gchar      name[128];
	GtkAction *action;

	g_snprintf (name, sizeof (name), "%s%s", prefix, item);
	action = gtk_action_group_get_action (action_group, name);
	
	g_signal_handlers_block_by_func (action,
					 cpufreq_popup_frequencies_menu_activate,
					 popup);
	g_signal_handlers_block_by_func (action,
					 cpufreq_popup_governors_menu_activate,
					 popup);
	
	gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action), TRUE);

	g_signal_handlers_unblock_by_func (action,
					   cpufreq_popup_frequencies_menu_activate,
					   popup);
	g_signal_handlers_unblock_by_func (action,
					   cpufreq_popup_governors_menu_activate,
					   popup);
}

static void
cpufreq_popup_menu_set_active (CPUFreqPopup *popup)
{
	const gchar *governor;

	governor = cpufreq_monitor_get_governor (popup->monitor);
	
	if (g_ascii_strcasecmp (governor, "userspace") == 0) {
		gchar *active;
		guint  freq;

		freq = cpufreq_monitor_get_frequency (popup->monitor);
		active = g_strdup_printf ("%d", freq);
		cpufreq_popup_menu_set_active_action (popup,
						      popup->freqs_group,
						      "Frequency", active);
		g_free (active);
	} else {
		cpufreq_popup_menu_set_active_action (popup,
						      popup->govs_group,
						      "Governor", governor);
	}
}

GtkWidget *
cpufreq_popup_get_menu (CPUFreqPopup *popup)
{
	GtkWidget *menu;
	
	g_return_val_if_fail (CPUFREQ_IS_POPUP (popup), NULL);
	g_return_val_if_fail (CPUFREQ_IS_MONITOR (popup->monitor), NULL);
	
	if (!cpufreq_utils_selector_is_available ())
		return NULL;

	if (popup->need_build) {
		cpufreq_popup_build_menu (popup);
		popup->need_build = FALSE;
	}

	cpufreq_popup_menu_set_active (popup);

	menu = gtk_ui_manager_get_widget (popup->ui_manager,
					  "/CPUFreqSelectorPopup");

	return menu;
}
