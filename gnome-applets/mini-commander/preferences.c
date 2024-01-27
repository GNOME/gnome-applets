/*
 * Copyright (C) 1998, 1999 Oliver Maruhn <oliver@maruhn.com>
 *               2002 Sun Microsystems Inc.
 *
 * Authors: Oliver Maruhn <oliver@maruhn.com>
 *          Mark McLoughlin <mark@skynet.ie>
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
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include "preferences.h"

#include <string.h>

#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>

#include "command-line.h"
#include "history.h"
#include "gsettings.h"

enum {
	COLUMN_PATTERN,
	COLUMN_COMMAND
};

#define NEVER_SENSITIVE		"never_sensitive"

static GSList *mc_load_macros (MCData *mc);

/* set sensitive and setup NEVER_SENSITIVE appropriately */
static void
hard_set_sensitive (GtkWidget *w, gboolean sensitivity)
{
	gtk_widget_set_sensitive (w, sensitivity);
	g_object_set_data (G_OBJECT (w), NEVER_SENSITIVE,
			   GINT_TO_POINTER ( ! sensitivity));
}

/* set sensitive, but always insensitive if NEVER_SENSITIVE is set */
static void
soft_set_sensitive (GtkWidget *w, gboolean sensitivity)
{
	if (g_object_get_data (G_OBJECT (w), NEVER_SENSITIVE))
		gtk_widget_set_sensitive (w, FALSE);
	else
		gtk_widget_set_sensitive (w, sensitivity);
}

static void
show_default_theme_changed (GSettings   *settings,
                            const gchar *key,
                            MCData      *mc)
{
    mc->preferences.show_default_theme = g_settings_get_boolean (mc->settings, key);

    if (mc->prefs_dialog.dialog)
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (mc->prefs_dialog.use_default_theme_toggle),
                                      mc->preferences.show_default_theme);

    mc_applet_draw (mc); /* FIXME: we shouldn't have to redraw the whole applet */
}

static void
auto_complete_history_changed (GSettings   *settings,
                               const gchar *key,
                               MCData      *mc)
{
    mc->preferences.auto_complete_history = g_settings_get_boolean (mc->settings, key);

    if (mc->prefs_dialog.dialog)
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (mc->prefs_dialog.auto_complete_history_toggle),
                                      mc->preferences.auto_complete_history);
}

static void
normal_size_x_changed (GSettings   *settings,
                       const gchar *key,
                       MCData      *mc)
{
    mc->preferences.normal_size_x = g_settings_get_int (mc->settings, key);

    if (mc->prefs_dialog.dialog)
        gtk_spin_button_set_value (GTK_SPIN_BUTTON (mc->prefs_dialog.size_spinner),
                                   mc->preferences.normal_size_x);

    mc_command_update_entry_size (mc);
}

static void
cmd_line_color_fg_changed (GSettings   *settings,
                           const gchar *key,
                           MCData      *mc)
{
    GdkRGBA color;

    if (mc->preferences.cmd_line_color_fg)
       g_free (mc->preferences.cmd_line_color_fg);

    mc->preferences.cmd_line_color_fg = g_strdup (g_settings_get_string (mc->settings, key));

    if (mc->prefs_dialog.dialog) {
        gdk_rgba_parse (&color, mc->preferences.cmd_line_color_fg);
        gtk_color_button_set_rgba (GTK_COLOR_BUTTON (mc->prefs_dialog.fg_color_picker), &color);
    }

    mc_command_update_entry_color (mc);
}

static void
cmd_line_color_bg_changed (GSettings   *settings,
                           const gchar *key,
                           MCData      *mc)
{
    GdkRGBA color;

    if (mc->preferences.cmd_line_color_bg)
       g_free (mc->preferences.cmd_line_color_bg);

    mc->preferences.cmd_line_color_bg = g_strdup (g_settings_get_string (mc->settings, key));

    if (mc->prefs_dialog.dialog) {
        gdk_rgba_parse (&color, mc->preferences.cmd_line_color_bg);
        gtk_color_button_set_rgba (GTK_COLOR_BUTTON (mc->prefs_dialog.bg_color_picker), &color);
    }

    mc_command_update_entry_color (mc);
}

static gboolean
load_macros_in_idle (MCData *mc)
{
    mc->preferences.idle_macros_loader_id = 0;

    if (mc->preferences.macros)
	mc_macros_free (mc->preferences.macros);

    mc->preferences.macros = mc_load_macros (mc);

    return FALSE;
}

static void
macros_changed (GSettings   *settings,
                const gchar *key,
                MCData      *mc)
{
    if (mc->preferences.idle_macros_loader_id == 0)
	mc->preferences.idle_macros_loader_id =
		g_idle_add ((GSourceFunc) load_macros_in_idle, mc);
}

/* Properties dialog
 */
static void
save_macros_to_gsettings (MCData *mc)
{
    MCPrefsDialog *dialog;
    GtkTreeIter    iter;
    GArray        *patterns;
    GArray        *commands;

    dialog = &mc->prefs_dialog;

    if (!gtk_tree_model_get_iter_first  (GTK_TREE_MODEL (dialog->macros_store), &iter))
        return;

    patterns = g_array_new (TRUE, TRUE, sizeof (gchar *));
    commands = g_array_new (TRUE, TRUE, sizeof (gchar *));

    do {
        char *pattern = NULL;
        char *command = NULL;

        gtk_tree_model_get (GTK_TREE_MODEL (dialog->macros_store), &iter,
                            0, &pattern, 1, &command, -1);

        patterns = g_array_append_val (patterns, pattern);
        commands = g_array_append_val (commands, command);
    } while (gtk_tree_model_iter_next (GTK_TREE_MODEL (dialog->macros_store), &iter));

    g_settings_set_strv (mc->global_settings, KEY_MACRO_PATTERNS, (const char **) patterns->data);
    g_settings_set_strv (mc->global_settings, KEY_MACRO_COMMANDS, (const char **) commands->data);

    g_array_free (patterns, TRUE);
    g_array_free (commands, TRUE);
}

static gboolean
duplicate_pattern (MCData     *mc,
		   const char *new_pattern)
{
    MCPrefsDialog *dialog;
    GtkTreeIter    iter;

    dialog = &mc->prefs_dialog;

    if (!gtk_tree_model_get_iter_first  (GTK_TREE_MODEL (dialog->macros_store), &iter))
	return FALSE;

    do {
	char *pattern = NULL;

	gtk_tree_model_get (
		GTK_TREE_MODEL (dialog->macros_store), &iter,
		0, &pattern, -1);

	if (!strcmp (pattern, new_pattern))
		return TRUE;

    } while (gtk_tree_model_iter_next (GTK_TREE_MODEL (dialog->macros_store), &iter));


    return FALSE;
}

static void
add_response (GtkWidget *window,
	      int        id,
	      MCData    *mc)
{
    MCPrefsDialog *dialog;

    dialog = &mc->prefs_dialog;

    switch (id) {
    case GTK_RESPONSE_OK: {
	const char  *pattern;
	const char  *command;
	GtkTreeIter  iter;
	const char  *error_message = NULL;

	pattern = gtk_entry_get_text (GTK_ENTRY (dialog->pattern_entry));
	command = gtk_entry_get_text (GTK_ENTRY (dialog->command_entry));

	if ((pattern == NULL || *pattern == '\0') && (command == NULL || *command == '\0'))
		error_message = _("You must specify a pattern and a command");
	else if (pattern == NULL || *pattern == '\0')
		error_message = _("You must specify a pattern");
	else if (command == NULL || *command == '\0')
		error_message = _("You must specify a command");
	else if (duplicate_pattern (mc, pattern))
		error_message = _("You may not specify duplicate patterns");

	if (error_message) {
	    GtkWidget *error_dialog;

	    error_dialog = gtk_message_dialog_new (GTK_WINDOW (window),
					           GTK_DIALOG_DESTROY_WITH_PARENT,
					           GTK_MESSAGE_ERROR,
					           GTK_BUTTONS_OK,
					           "%s",
					           error_message);

	    g_signal_connect (error_dialog, "response", G_CALLBACK (gtk_widget_destroy), NULL);
	    gtk_window_set_resizable (GTK_WINDOW (error_dialog), FALSE);
	    gtk_widget_show_all (error_dialog);
	    return;
	}

	gtk_widget_hide (window);

	gtk_list_store_append (dialog->macros_store, &iter);
	gtk_list_store_set (dialog->macros_store, &iter,
			    COLUMN_PATTERN, pattern, 
			    COLUMN_COMMAND, command,
			    -1);

	gtk_tree_view_columns_autosize (GTK_TREE_VIEW (dialog->macros_tree));

	gtk_editable_delete_text (GTK_EDITABLE (dialog->pattern_entry), 0, -1);
	gtk_editable_delete_text (GTK_EDITABLE (dialog->command_entry), 0, -1);

	save_macros_to_gsettings (mc);
    }
	break;
    case GTK_RESPONSE_HELP:
        gp_applet_show_help (GP_APPLET (mc), "command-line-prefs-2");
	break;
    case GTK_RESPONSE_CLOSE:
    default:
	gtk_editable_delete_text (GTK_EDITABLE (dialog->pattern_entry), 0, -1);
	gtk_editable_delete_text (GTK_EDITABLE (dialog->command_entry), 0, -1);
	gtk_widget_hide (window);
	break;
    }
}

static void
setup_add_dialog (GtkBuilder *builder,
		  MCData     *mc)
{
    MCPrefsDialog *dialog;

    dialog = &mc->prefs_dialog;

    g_signal_connect (dialog->macro_add_dialog, "response",
		      G_CALLBACK (add_response), mc);

    dialog->pattern_entry = GTK_WIDGET (gtk_builder_get_object (builder, "pattern_entry"));
    dialog->command_entry = GTK_WIDGET (gtk_builder_get_object (builder, "command_entry"));

    gtk_dialog_set_default_response (GTK_DIALOG (dialog->macro_add_dialog), GTK_RESPONSE_OK);
}

static void
macro_add (GtkWidget *button,
           MCData    *mc)
{
    if (!mc->prefs_dialog.macro_add_dialog) {
	GtkBuilder *builder;

	builder = gtk_builder_new ();
	gtk_builder_set_translation_domain (builder, GETTEXT_PACKAGE);
	gtk_builder_add_from_resource (builder, GRESOURCE_PREFIX "/ui/mini-commander.ui", NULL);

	mc->prefs_dialog.macro_add_dialog = GTK_WIDGET (gtk_builder_get_object (builder, "mc_macro_add_dialog"));

	g_object_add_weak_pointer (G_OBJECT (mc->prefs_dialog.macro_add_dialog),
				   (gpointer *) &mc->prefs_dialog.macro_add_dialog);

	setup_add_dialog (builder, mc);

	g_object_unref (builder);
    }

    gtk_window_set_screen (GTK_WINDOW (mc->prefs_dialog.macro_add_dialog),
			   gtk_widget_get_screen (GTK_WIDGET (mc)));
    gtk_widget_grab_focus (mc->prefs_dialog.pattern_entry);
    gtk_window_present (GTK_WINDOW (mc->prefs_dialog.macro_add_dialog));
}

static void
macro_delete (GtkWidget *button,
	      MCData    *mc)
{
    MCPrefsDialog    *dialog;
    GtkTreeModel     *model = NULL;
    GtkTreeSelection *selection;
    GtkTreeIter       iter;
  
    dialog = &mc->prefs_dialog;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (dialog->macros_tree));

    if (!gtk_tree_selection_get_selected (selection, &model, &iter))
	return;

    gtk_list_store_remove (dialog->macros_store, &iter);

    save_macros_to_gsettings (mc);
}

static void
show_macros_list (MCData *mc)
{
    MCPrefsDialog *dialog;
    GtkTreeIter    iter;
    GSList        *l;

    dialog = &mc->prefs_dialog;

    gtk_list_store_clear (dialog->macros_store);

    for (l = mc->preferences.macros; l; l = l->next) {
	MCMacro *macro = l->data;

	gtk_list_store_append (dialog->macros_store, &iter);
	gtk_list_store_set (dialog->macros_store, &iter,
			    COLUMN_PATTERN, macro->pattern,
			    COLUMN_COMMAND, macro->command,
			    -1);
    }

    gtk_tree_view_columns_autosize (GTK_TREE_VIEW (dialog->macros_tree));
}

static void
macro_edited (GtkCellRendererText *renderer,
	      const char          *path,
	      const char          *new_text,
	      MCData              *mc)
{
    MCPrefsDialog *dialog;
    GtkTreeIter    iter;
    int            col;

    dialog = &mc->prefs_dialog;

    col = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (renderer), "column"));

    if (gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (dialog->macros_store), &iter, path))
	gtk_list_store_set (dialog->macros_store, &iter, col, new_text, -1);

    save_macros_to_gsettings (mc);
}

static void
foreground_color_set (GtkColorButton *color_button,
		      MCData    *mc)
{
    GdkRGBA color;
    gchar *str;

    gtk_color_button_get_rgba (color_button, &color);

    str = gdk_rgba_to_string (&color);
    g_settings_set_string (mc->settings, KEY_CMD_LINE_COLOR_FG, str);
    g_free (str);
}

static void
background_color_set (GtkColorButton *color_button,
		      MCData    *mc)
{
    GdkRGBA color;
    gchar *str;

    gtk_color_button_get_rgba (color_button, &color);

    str = gdk_rgba_to_string (&color);
    g_settings_set_string (mc->settings, KEY_CMD_LINE_COLOR_BG, str);
    g_free (str);
}

static void
auto_complete_history_toggled (GtkToggleButton *toggle,
			       MCData          *mc)
{
    gboolean auto_complete_history;
    
    auto_complete_history = gtk_toggle_button_get_active (toggle);
    if (auto_complete_history == mc->preferences.auto_complete_history) 
        return;

    g_settings_set_boolean (mc->settings,
                            KEY_AUTOCOMPLETE_HISTORY,
                            auto_complete_history);
}

static void
size_value_changed (GtkSpinButton *spinner,
		    MCData        *mc)
{
    int size;

    size = gtk_spin_button_get_value (spinner);
    if (size == mc->preferences.normal_size_x)
	return;

    g_settings_set_int (mc->settings, KEY_NORMAL_SIZE_X, size);
}

static void
use_default_theme_toggled (GtkToggleButton *toggle,
			   MCData          *mc)
{
    gboolean use_default_theme;
    
    use_default_theme = gtk_toggle_button_get_active (toggle);
    if (use_default_theme == mc->preferences.show_default_theme) 
        return;
        
    soft_set_sensitive (mc->prefs_dialog.fg_color_picker, !use_default_theme);
    soft_set_sensitive (mc->prefs_dialog.bg_color_picker, !use_default_theme);

    g_settings_set_boolean (mc->settings, KEY_SHOW_DEFAULT_THEME, use_default_theme);
}

static void
preferences_response (MCPrefsDialog *dialog,
		      int        id,
		      MCData    *mc)
{
    switch (id) {
    case GTK_RESPONSE_HELP:
        gp_applet_show_help (GP_APPLET (mc), "command-line-apperance");
	break;
    case GTK_RESPONSE_CLOSE:
    default: {
        GtkTreeViewColumn *col;
        GtkCellArea *area;
        GtkCellEditable *edit_widget;

	dialog = &mc->prefs_dialog;

	/* A hack to make sure 'edited' on the renderer if we
	 * close the dialog while editing.
	 */
	col = gtk_tree_view_get_column (GTK_TREE_VIEW (dialog->macros_tree), 0);
	area = gtk_cell_layout_get_area (GTK_CELL_LAYOUT (col));
	edit_widget = gtk_cell_area_get_edit_widget (area);
	if (edit_widget)
		gtk_cell_editable_editing_done (edit_widget);

	col = gtk_tree_view_get_column (GTK_TREE_VIEW (dialog->macros_tree), 1);
	area = gtk_cell_layout_get_area (GTK_CELL_LAYOUT (col));
	edit_widget = gtk_cell_area_get_edit_widget (area);
	if (edit_widget)
		gtk_cell_editable_editing_done (edit_widget);

	gtk_widget_hide (dialog->dialog);
    }
	break;
    }
}

static void
mc_preferences_setup_dialog (GtkBuilder *builder,
			     MCData     *mc)
{
    MCPrefsDialog   *dialog;
    GtkCellRenderer *renderer;
    GdkRGBA          color;

    dialog = &mc->prefs_dialog;

    g_signal_connect (dialog->dialog, "response",
		      G_CALLBACK (preferences_response), mc);

    gtk_dialog_set_default_response (GTK_DIALOG (dialog->dialog), GTK_RESPONSE_CLOSE);
    gtk_window_set_default_size (GTK_WINDOW (dialog->dialog), 400, -1);

    dialog->auto_complete_history_toggle = GTK_WIDGET (gtk_builder_get_object (builder, "auto_complete_history_toggle"));
    dialog->size_spinner                 = GTK_WIDGET (gtk_builder_get_object (builder, "size_spinner"));
    dialog->use_default_theme_toggle     = GTK_WIDGET (gtk_builder_get_object (builder, "default_theme_toggle"));
    dialog->fg_color_picker              = GTK_WIDGET (gtk_builder_get_object (builder, "fg_color_picker"));
    dialog->bg_color_picker              = GTK_WIDGET (gtk_builder_get_object (builder, "bg_color_picker"));
    dialog->macros_tree                  = GTK_WIDGET (gtk_builder_get_object (builder, "macros_tree"));
    dialog->delete_button                = GTK_WIDGET (gtk_builder_get_object (builder, "delete_button"));
    dialog->add_button                   = GTK_WIDGET (gtk_builder_get_object (builder, "add_button"));

    /* History based autocompletion */
    g_signal_connect (dialog->auto_complete_history_toggle, "toggled",
		      G_CALLBACK (auto_complete_history_toggled), mc);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->auto_complete_history_toggle),
				  mc->preferences.auto_complete_history);
    if (!g_settings_is_writable (mc->settings, KEY_AUTOCOMPLETE_HISTORY))
	    hard_set_sensitive (dialog->auto_complete_history_toggle, FALSE);

    /* Width */
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (dialog->size_spinner), mc->preferences.normal_size_x);
    g_signal_connect (dialog->size_spinner, "value_changed",
		      G_CALLBACK (size_value_changed), mc); 
    if (!g_settings_is_writable (mc->settings, KEY_NORMAL_SIZE_X)) {
	    hard_set_sensitive (dialog->size_spinner, FALSE);
	    hard_set_sensitive (GTK_WIDGET (gtk_builder_get_object (builder, "size_label")), FALSE);
	    hard_set_sensitive (GTK_WIDGET (gtk_builder_get_object (builder, "size_post_label")), FALSE);
    }

    /* Use default theme */
    g_signal_connect (dialog->use_default_theme_toggle, "toggled",
		      G_CALLBACK (use_default_theme_toggled), mc);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->use_default_theme_toggle),
				  mc->preferences.show_default_theme);
    if (!g_settings_is_writable (mc->settings, KEY_SHOW_DEFAULT_THEME))
	    hard_set_sensitive (dialog->use_default_theme_toggle, FALSE);

    /* Foreground color */
    g_signal_connect (dialog->fg_color_picker, "color_set",
		      G_CALLBACK (foreground_color_set), mc);
    gdk_rgba_parse (&color, mc->preferences.cmd_line_color_fg);
    gtk_color_button_set_rgba (GTK_COLOR_BUTTON (dialog->fg_color_picker), &color);
    soft_set_sensitive (dialog->fg_color_picker, !mc->preferences.show_default_theme);

    if (!g_settings_is_writable (mc->settings, KEY_CMD_LINE_COLOR_FG)) {
	    hard_set_sensitive (dialog->fg_color_picker, FALSE);
	    hard_set_sensitive (GTK_WIDGET (gtk_builder_get_object (builder, "fg_color_label")), FALSE);
    }

    /* Background color */
    g_signal_connect (dialog->bg_color_picker, "color_set",
		      G_CALLBACK (background_color_set), mc);
    gdk_rgba_parse (&color, mc->preferences.cmd_line_color_bg);
    gtk_color_button_set_rgba (GTK_COLOR_BUTTON (dialog->bg_color_picker), &color);
    soft_set_sensitive (dialog->bg_color_picker, !mc->preferences.show_default_theme);

    if (!g_settings_is_writable (mc->settings, KEY_CMD_LINE_COLOR_BG)) {
	    hard_set_sensitive (dialog->bg_color_picker, FALSE);
	    hard_set_sensitive (GTK_WIDGET (gtk_builder_get_object (builder, "bg_color_label")), FALSE);
    }


    /* Macros Delete and Add buttons */
    g_signal_connect (dialog->delete_button, "clicked", G_CALLBACK (macro_delete), mc);
    g_signal_connect (dialog->add_button, "clicked", G_CALLBACK (macro_add), mc);

    if (!g_settings_is_writable (mc->global_settings, KEY_MACRO_PATTERNS) ||
	    !g_settings_is_writable (mc->global_settings, KEY_MACRO_COMMANDS)) {
	    hard_set_sensitive (dialog->add_button, FALSE);
	    hard_set_sensitive (dialog->delete_button, FALSE);
	    hard_set_sensitive (dialog->macros_tree, FALSE);
    }

    /* Macros tree view */
    dialog->macros_store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING, NULL);
    gtk_tree_view_set_model (GTK_TREE_VIEW (dialog->macros_tree),
			     GTK_TREE_MODEL (dialog->macros_store));

    renderer = g_object_new (GTK_TYPE_CELL_RENDERER_TEXT, "editable", TRUE, NULL);
    g_object_set_data (G_OBJECT (renderer), "column", GINT_TO_POINTER (COLUMN_PATTERN));
    g_signal_connect (renderer, "edited", G_CALLBACK (macro_edited), mc);

    gtk_tree_view_insert_column_with_attributes (
			GTK_TREE_VIEW (dialog->macros_tree), -1,
			_("Pattern"), renderer,
			"text", COLUMN_PATTERN,
			NULL);

    renderer = g_object_new (GTK_TYPE_CELL_RENDERER_TEXT, "editable", TRUE, NULL);
    g_object_set_data (G_OBJECT (renderer), "column", GINT_TO_POINTER (COLUMN_COMMAND));
    g_signal_connect (renderer, "edited", G_CALLBACK (macro_edited), mc);

    gtk_tree_view_insert_column_with_attributes (
			GTK_TREE_VIEW (dialog->macros_tree), -1,
			_("Command"), renderer,
			"text", COLUMN_COMMAND,
			NULL);

    show_macros_list (mc);
}

void
mc_show_preferences (GSimpleAction *action,
                     GVariant      *parameter,
                     gpointer       user_data)
{
	MCData *mc = (MCData *) user_data;

    if (!mc->prefs_dialog.dialog) {
	GtkBuilder *builder;

	builder = gtk_builder_new ();
	gtk_builder_set_translation_domain (builder, GETTEXT_PACKAGE);
	gtk_builder_add_from_resource (builder, GRESOURCE_PREFIX "/ui/mini-commander.ui", NULL);

	mc->prefs_dialog.dialog = GTK_WIDGET (gtk_builder_get_object (builder,
			"mc_preferences_dialog"));

	g_object_add_weak_pointer (G_OBJECT (mc->prefs_dialog.dialog),
				   (gpointer *) &mc->prefs_dialog.dialog);

	mc_preferences_setup_dialog (builder, mc);

	g_object_unref (builder);
    }

    gtk_window_set_screen (GTK_WINDOW (mc->prefs_dialog.dialog),
			   gtk_widget_get_screen (GTK_WIDGET (mc)));
    gtk_window_present (GTK_WINDOW (mc->prefs_dialog.dialog));
}

static MCMacro *
mc_macro_new (const char *pattern,
	      const char *command)
{
    MCMacro *macro;

    g_return_val_if_fail (pattern != NULL, NULL);
    g_return_val_if_fail (command != NULL, NULL);

    macro = g_new0 (MCMacro, 1);
            
    macro->pattern = g_strdup (pattern);
    macro->command = g_strdup (command); 
	    
    if (macro->pattern [0] != '\0')
	regcomp (&macro->regex, macro->pattern, REG_EXTENDED);

    return macro;
}

void
mc_macros_free (GSList *macros)
{
    GSList *l;

    for (l = macros; l; l = l->next) {
	MCMacro *macro = l->data;

	regfree(&macro->regex);
	g_free (macro->pattern);
	g_free (macro->command);
	g_free (macro);
    }

    g_slist_free (macros);
}
	      
static GSList *
mc_load_macros (MCData *mc)
{
    gchar  **macro_patterns;
    gchar  **macro_commands;
    GSList  *macros_list = NULL;
    guint    i;

	macro_patterns = g_settings_get_strv (mc->global_settings, KEY_MACRO_PATTERNS);
	macro_commands = g_settings_get_strv (mc->global_settings, KEY_MACRO_COMMANDS);

    for (i = 0; macro_patterns[i] != NULL && macro_commands[i] != NULL; i++) {
        MCMacro *macro;

        if (!(macro = mc_macro_new (macro_patterns[i], macro_commands[i])))
            continue;

        macros_list = g_slist_prepend (macros_list, macro);
    }

    g_strfreev (macro_patterns);
    g_strfreev (macro_commands);

    macros_list = g_slist_reverse (macros_list);

    return macros_list;
}

void
mc_load_preferences (MCData *mc)
{
    gchar **history;
    guint i;

    g_return_if_fail (mc != NULL);

    mc->preferences.show_default_theme = g_settings_get_boolean (mc->settings, KEY_SHOW_DEFAULT_THEME);
    mc->preferences.auto_complete_history = g_settings_get_boolean (mc->settings, KEY_AUTOCOMPLETE_HISTORY);
    mc->preferences.normal_size_x = MAX (g_settings_get_int (mc->settings, KEY_NORMAL_SIZE_X), 50);
    mc->preferences.normal_size_y = 48;
    mc->preferences.cmd_line_color_fg = g_strdup (g_settings_get_string (mc->settings, KEY_CMD_LINE_COLOR_FG));
    mc->preferences.cmd_line_color_bg = g_strdup (g_settings_get_string (mc->settings, KEY_CMD_LINE_COLOR_BG));

    g_signal_connect (mc->settings, "changed::" KEY_SHOW_DEFAULT_THEME,
	                  G_CALLBACK (show_default_theme_changed), mc);
	g_signal_connect (mc->settings, "changed::" KEY_AUTOCOMPLETE_HISTORY,
	                  G_CALLBACK (auto_complete_history_changed), mc);
	g_signal_connect (mc->settings, "changed::" KEY_NORMAL_SIZE_X,
	                  G_CALLBACK (normal_size_x_changed), mc);
	g_signal_connect (mc->settings, "changed::" KEY_CMD_LINE_COLOR_FG,
	                  G_CALLBACK (cmd_line_color_fg_changed), mc);
	g_signal_connect (mc->settings, "changed::" KEY_CMD_LINE_COLOR_BG,
	                  G_CALLBACK (cmd_line_color_bg_changed), mc);

    mc->preferences.macros = mc_load_macros (mc);

    g_signal_connect (mc->global_settings, "changed::" KEY_MACRO_PATTERNS,
	                  G_CALLBACK (macros_changed), mc);
	g_signal_connect (mc->global_settings, "changed::" KEY_MACRO_COMMANDS,
	                  G_CALLBACK (macros_changed), mc);

    mc->preferences.idle_macros_loader_id = 0;

    history = g_settings_get_strv (mc->settings, KEY_HISTORY);
    for (i = 0; history[i] != NULL; i++) {
        append_history_entry (mc, history[i], TRUE);
    }
}
