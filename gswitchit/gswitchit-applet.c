/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "config.h"

#include "gswitchit-applet.h"
#include "libgswitchit/gswitchit_util.h"
#include "libgswitchit/gswitchit_plugin_manager.h"
#include "libkbdraw/keyboard-drawing.h"

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include <gdk/gdkscreen.h>
#include <gdk/gdkx.h>
#include <glade/glade.h>

#define GROUPS_SUBMENU_PATH "/popups/popup/groups"

/* we never build applet without XKB, so far: TODO */
#define HAVE_XKB  1

#ifdef HAVE_XKB
#include "X11/XKBlib.h"
/**
 * BAD STYLE: Taken from xklavier_private_xkb.h
 * Any ideas on architectural improvements are WELCOME
 */
extern gboolean xkl_xkb_config_native_prepare (XklEngine * engine,
					       const XklConfigRec * data,
					       XkbComponentNamesPtr
					       component_names);

extern void xkl_xkb_config_native_cleanup (XklEngine * engine,
					   XkbComponentNamesPtr
					   component_names);
/* */
#endif

typedef struct _GSwitchItAppletGlobals {
	GSList *applet_instances;
	GHashTable *previewDialogs;
} GSwitchItAppletGlobals;

/* one instance for ALL applets */
static GSwitchItAppletGlobals globals;

static void GSwitchItAppletCmdCapplet (BonoboUIComponent * uic,
				       GSwitchItApplet * sia,
				       const gchar * verb);

static void GSwitchItAppletCmdPreview (BonoboUIComponent * uic,
				       GSwitchItApplet * sia,
				       const gchar * verb);

static void GSwitchItAppletCmdPlugins (BonoboUIComponent * uic,
				       GSwitchItApplet * sia,
				       const gchar * verb);

static void GSwitchItAppletCmdAbout (BonoboUIComponent * uic,
				     GSwitchItApplet * sia,
				     const gchar * verb);

static void GSwitchItAppletCmdHelp (BonoboUIComponent * uic,
				    GSwitchItApplet * sia,
				    const gchar * verb);

static void GSwitchItAppletCmdSetGroup (BonoboUIComponent * uic,
					GSwitchItApplet * sia,
					const gchar * verb);

static void GSwitchItAppletCleanupGroupsSubmenu (GSwitchItApplet * sia);

static void GSwitchItAppletSetupGroupsSubmenu (GSwitchItApplet * sia);

static const BonoboUIVerb gswitchitAppletMenuVerbs[] = {
	BONOBO_UI_UNSAFE_VERB ("Capplet", GSwitchItAppletCmdCapplet),
	BONOBO_UI_UNSAFE_VERB ("Preview", GSwitchItAppletCmdPreview),
	BONOBO_UI_UNSAFE_VERB ("Plugins", GSwitchItAppletCmdPlugins),
	BONOBO_UI_UNSAFE_VERB ("About", GSwitchItAppletCmdAbout),
	BONOBO_UI_UNSAFE_VERB ("Help", GSwitchItAppletCmdHelp),
	BONOBO_UI_VERB_END
};

// gnome_kbd_indicator_set_tooltips_format(_("Keyboard Indicator (%s)"));

void
GSwitchItAppletReinitUi (GnomeKbdIndicator * gki, GSwitchItApplet * sia)
{
	GSwitchItAppletCleanupGroupsSubmenu (sia);
	GSwitchItAppletSetupGroupsSubmenu (sia);
}

static void
GSwitchItAppletChangePixelSize (PanelApplet *
				widget, guint size, GSwitchItApplet * sia)
{
	gnome_kbd_indicator_reinit_ui (GNOME_KBD_INDICATOR (sia->gki));
}

static void
GSwitchItAppletSetBackground (PanelAppletBackgroundType type,
			      GtkRcStyle * rc_style,
			      GtkWidget * w,
			      GdkColor * color, GdkPixmap * pixmap)
{
	GtkStyle *style;

	gtk_widget_set_style (GTK_WIDGET (w), NULL);
	gtk_widget_modify_style (GTK_WIDGET (w), rc_style);

	switch (type) {
	case PANEL_NO_BACKGROUND:
		break;
	case PANEL_COLOR_BACKGROUND:
		gtk_widget_modify_bg (GTK_WIDGET (w),
				      GTK_STATE_NORMAL, color);
		break;
	case PANEL_PIXMAP_BACKGROUND:
		style = gtk_style_copy (w->style);
		if (style->bg_pixmap[GTK_STATE_NORMAL])
			g_object_unref (style->
					bg_pixmap[GTK_STATE_NORMAL]);
		style->bg_pixmap[GTK_STATE_NORMAL] = g_object_ref (pixmap);
		gtk_widget_set_style (w, style);
		g_object_unref (style);
		break;
	}
	/* go down */
	if (GTK_IS_CONTAINER (w)) {
		GList *child =
		    gtk_container_get_children (GTK_CONTAINER (w));
		while (child != NULL) {
			GSwitchItAppletSetBackground (type, rc_style,
						      GTK_WIDGET (child->
								  data),
						      color, pixmap);
			child = child->next;
		}
	}
}

static void
GSwitchItAppletChangeBackground (PanelApplet * widget,
				 PanelAppletBackgroundType type,
				 GdkColor * color, GdkPixmap * pixmap,
				 GSwitchItApplet * sia)
{
	GtkRcStyle *rc_style = gtk_rc_style_new ();
	GSwitchItAppletSetBackground (type, rc_style, sia->applet, color,
				      pixmap);
	gtk_rc_style_unref (rc_style);
}

void
GSwitchItAppletCmdCapplet (BonoboUIComponent *
			   uic, GSwitchItApplet * sia, const gchar * verb)
{
	GError *error = NULL;

	gdk_spawn_command_line_on_screen (gtk_widget_get_screen
					  (GTK_WIDGET (sia->applet)),
					  "gnome-keyboard-properties",
					  &error);

	if (error != NULL) {
		/* FIXME: After string ui freeze are over, we want to show an error message here */
		g_error_free (error);
	}
}

static void
GSwitchItPreviewResponse (GtkWidget * dialog, gint resp)
{
	GdkRectangle rect;

	switch (resp) {
	case GTK_RESPONSE_HELP:
		gswitchit_help (GTK_WIDGET (dialog), "layout-view");
		return;
	case GTK_RESPONSE_CLOSE:
		gtk_window_get_position (GTK_WINDOW (dialog), &rect.x,
					 &rect.y);
		gtk_window_get_size (GTK_WINDOW (dialog), &rect.width,
				     &rect.height);
		gswitchit_preview_save (&rect);
		gtk_widget_destroy (dialog);
	}
}

static void
GSwitchItPreviewDestroy (GtkWidget * dialog, gint group)
{
	GladeXML *gladeData =
	    GLADE_XML (gtk_object_get_data
		       (GTK_OBJECT (dialog), "gladeData"));
	g_object_unref (G_OBJECT (gladeData));
	g_hash_table_remove (globals.previewDialogs,
			     GINT_TO_POINTER (group));
}

void
GSwitchItAppletCmdPreview (BonoboUIComponent *
			   uic, GSwitchItApplet * sia, const gchar * verb)
{
	static KeyboardDrawingGroupLevel groupsLevels[] =
	    { {0, 1}, {0, 3}, {0, 0}, {0, 2} };
	static KeyboardDrawingGroupLevel *pGroupsLevels[] = {
		groupsLevels, groupsLevels + 1, groupsLevels + 2,
		groupsLevels + 3
	};
#ifdef HAVE_XKB
	GladeXML *gladeData;
	GtkWidget *dialog, *kbdraw;
	XkbComponentNamesRec componentNames;
	XklConfigRec *xklData;
	GdkRectangle *rect;
#endif
	XklEngine *engine = gnome_kbd_indicator_get_xkl_engine ();
	XklState *xklState = xkl_engine_get_current_state (engine);
	gchar **groupNames = gnome_kbd_indicator_get_group_names ();
	gpointer p = g_hash_table_lookup (globals.previewDialogs,
					  GINT_TO_POINTER (xklState->
							   group));
	if (p != NULL) {
		/* existing window */
		gtk_window_present (GTK_WINDOW (p));
		return;
	}
#ifdef HAVE_XKB
	gladeData =
	    glade_xml_new (GNOME_GLADEDIR "/gswitchit.glade",
			   "gswitchit_layout_view", NULL);
	dialog = glade_xml_get_widget (gladeData, "gswitchit_layout_view");
	kbdraw = keyboard_drawing_new ();

	if (xklState->group >= 0 &&
	    xklState->group < g_strv_length (groupNames)) {
		char title[128];
		snprintf (title, sizeof (title),
			  _("Keyboard Layout \"%s\""),
			  groupNames[xklState->group]);
		gtk_window_set_title (GTK_WINDOW (dialog), title);
	}

	keyboard_drawing_set_groups_levels (KEYBOARD_DRAWING (kbdraw),
					    pGroupsLevels);

	xklData = xkl_config_rec_new ();
	if (xkl_config_rec_get_from_server (xklData, engine)) {
		int numLayouts = g_strv_length (xklData->layouts);
		int numVariants = g_strv_length (xklData->variants);
		if (xklState->group >= 0 &&
		    xklState->group < numLayouts &&
		    xklState->group < numVariants) {
			char *l =
			    g_strdup (xklData->layouts[xklState->group]);
			char *v =
			    g_strdup (xklData->variants[xklState->group]);
			char **p;
			int i;

			if ((p = xklData->layouts) != NULL)
				for (i = numLayouts; --i >= 0;)
					g_free (*p++);

			if ((p = xklData->variants) != NULL)
				for (i = numVariants; --i >= 0;)
					g_free (*p++);

			xklData->layouts =
			    g_realloc (xklData->layouts,
				       sizeof (char *) * 2);
			xklData->variants =
			    g_realloc (xklData->variants,
				       sizeof (char *) * 2);
			xklData->layouts[0] = l;
			xklData->variants[0] = v;
			xklData->layouts[1] = xklData->variants[1] = NULL;
		}

		if (xkl_xkb_config_native_prepare
		    (engine, xklData, &componentNames)) {
			keyboard_drawing_set_keyboard (KEYBOARD_DRAWING
						       (kbdraw),
						       &componentNames);
			xkl_xkb_config_native_cleanup (engine,
						       &componentNames);
		}
	}
	g_object_unref (G_OBJECT (xklData));

	gtk_object_set_data (GTK_OBJECT (dialog), "gladeData", gladeData);
	g_signal_connect (GTK_OBJECT (dialog),
			  "destroy", G_CALLBACK (GSwitchItPreviewDestroy),
			  GINT_TO_POINTER (xklState->group));
	g_signal_connect (G_OBJECT (dialog), "response",
			  G_CALLBACK (GSwitchItPreviewResponse), NULL);

	rect = gswitchit_preview_load ();
	if (rect != NULL) {
		gtk_window_move (GTK_WINDOW (dialog), rect->x, rect->y);
		gtk_window_resize (GTK_WINDOW (dialog), rect->width,
				   rect->height);
		g_free (rect);
	} else
		gtk_window_resize (GTK_WINDOW (dialog), 700, 400);

	gtk_window_set_resizable (GTK_WINDOW (dialog), TRUE);

	gtk_container_add (GTK_CONTAINER
			   (glade_xml_get_widget
			    (gladeData, "preview_vbox")), kbdraw);

	g_hash_table_insert (globals.previewDialogs,
			     GINT_TO_POINTER (xklState->group), dialog);

	gtk_widget_show_all (GTK_WIDGET (dialog));
#endif
}

void
GSwitchItAppletCmdPlugins (BonoboUIComponent *
			   uic, GSwitchItApplet * sia, const gchar * verb)
{
	GError *error = NULL;

	gdk_spawn_command_line_on_screen (gtk_widget_get_screen
					  (GTK_WIDGET (sia->applet)),
					  "gswitchit-plugins-capplet",
					  &error);

	if (error != NULL) {
		/* FIXME: after string ui freeze are over, we want to show an error message here */
		g_error_free (error);
	}
}

static void
GSwitchItAppletCmdSetGroup (BonoboUIComponent
			    * uic, GSwitchItApplet * sia,
			    const gchar * verb)
{
	XklEngine *engine = gnome_kbd_indicator_get_xkl_engine ();
	XklState st;
	Window cur;
	if (sscanf (verb, "Group_%d", &st.group) != 1) {
		xkl_debug (50, "Strange verb: [%s]\n", verb);
		return;
	}

	xkl_engine_allow_one_switch_to_secondary_group (engine);
	cur = xkl_engine_get_current_window (engine);
	if (cur != (Window) NULL) {
		xkl_debug (150, "Enforcing the state %d for window %lx\n",
			   st.group, cur);
		xkl_engine_save_state (engine,
				       xkl_engine_get_current_window
				       (engine), &st);
/*    XSetInputFocus( GDK_DISPLAY(), cur, RevertToNone, CurrentTime );*/
	} else {
		xkl_debug (150,
			   "??? Enforcing the state %d for unknown window\n",
			   st.group);
		/* strange situation - bad things can happen */
	}
	xkl_engine_lock_group (engine, st.group);
}

void
GSwitchItAppletCmdHelp (BonoboUIComponent
			* uic, GSwitchItApplet * sia, const gchar * verb)
{
	gswitchit_help (GTK_WIDGET (sia->applet), NULL);
}

void
GSwitchItAppletCmdAbout (BonoboUIComponent *
			 uic, GSwitchItApplet * sia, const gchar * verb)
{
	const gchar *authors[] = {
		"Sergey V. Udaltsov<svu@gnome.org>",
		NULL
	};
	const gchar *documenters[] = {
		"Sergey V. Udaltsov<svu@gnome.org>",
		NULL
	};

	const gchar *translator_credits = _("translator-credits");

	gtk_show_about_dialog (NULL,
			       "name", _("Keyboard Indicator"),
			       "version", VERSION,
/* Translators: Please replace (C) with the proper copyright character. */
			       "copyright",
			       _
			       ("Copyright (c) Sergey V. Udaltsov 1999-2004"),
			       "comments",
			       _
			       ("Keyboard layout indicator applet for GNOME"),
			       "authors", authors, "documenters",
			       documenters, "translator-credits",
			       strcmp (translator_credits,
				       "translator-credits") !=
			       0 ? translator_credits : NULL,
			       "logo-icon-name", "gswitchit-applet", NULL);
}

static void
GSwitchItAppletCleanupGroupsSubmenu (GSwitchItApplet * sia)
{
	int i;
	BonoboUIComponent *popup;
	popup =
	    panel_applet_get_popup_component (PANEL_APPLET (sia->applet));
	for (i = 0;; i++) {
		char path[80];
		g_snprintf (path, sizeof (path),
			    GROUPS_SUBMENU_PATH "/Group_%d", i);
		if (bonobo_ui_component_path_exists (popup, path, NULL)) {
			bonobo_ui_component_rm (popup, path, NULL);
			xkl_debug (160,
				   "Unregistered group menu item \'%s\'\n",
				   path);
		} else
			break;
	}

	xkl_debug (160, "Unregistered group submenu\n");
}

static void
GSwitchItAppletSetupGroupsSubmenu (GSwitchItApplet * sia)
{
	unsigned i;
	gchar **currentName = gnome_kbd_indicator_get_group_names ();
	BonoboUIComponent *popup;
	popup =
	    panel_applet_get_popup_component (PANEL_APPLET (sia->applet));
	xkl_debug (160, "Registering group submenu\n");
	for (i = 0; *currentName; i++, currentName++) {
		char verb[40];
		BonoboUINode *node;
		g_snprintf (verb, sizeof (verb), "Group_%d", i);
		node = bonobo_ui_node_new ("menuitem");
		bonobo_ui_node_set_attr (node, "name", verb);
		bonobo_ui_node_set_attr (node, "verb", verb);
		bonobo_ui_node_set_attr (node, "label", *currentName);
		bonobo_ui_node_set_attr (node, "pixtype", "filename");
		gchar *imageFile =
		    gnome_kbd_indicator_get_image_filename (i);
		if (imageFile != NULL) {
			bonobo_ui_node_set_attr (node, "pixname",
						 imageFile);
			g_free (imageFile);
		}
		bonobo_ui_component_set_tree (popup, GROUPS_SUBMENU_PATH,
					      node, NULL);
		bonobo_ui_component_add_verb (popup, verb, (BonoboUIVerbFn)
					      GSwitchItAppletCmdSetGroup,
					      sia);
		xkl_debug (160,
			   "Registered group menu item \'%s\' as \'%s\'\n",
			   verb, *currentName);
	}
}

static void
GSwitchItAppletSetupMenu (GSwitchItApplet * sia)
{
	panel_applet_setup_menu_from_file
	    (PANEL_APPLET (sia->applet), DATADIR,
	     "GNOME_GSwitchItApplet.xml", NULL, gswitchitAppletMenuVerbs,
	     sia);
	GSwitchItAppletSetupGroupsSubmenu (sia);
}

static void GSwitchItAppletTerm (PanelApplet *
				 applet, GSwitchItApplet * sia);

static gboolean
GSwitchItAppletInit (GSwitchItApplet * sia, PanelApplet * applet)
{
	gdouble max_ratio;

	xkl_debug (100, "Starting the applet startup process for %p\n",
		   sia);

	glade_gnome_init ();
	gtk_window_set_default_icon_name ("gswitchit-applet");

	sia->applet = GTK_WIDGET (applet);

	panel_applet_set_flags (applet, PANEL_APPLET_EXPAND_MINOR);

	sia->gki = gnome_kbd_indicator_new ();

	gtk_container_add (GTK_CONTAINER (sia->applet),
			   GTK_WIDGET (sia->gki));


	gtk_widget_show_all (sia->applet);
	gtk_widget_realize (sia->applet);

	max_ratio = gnome_kbd_indicator_get_max_width_height_ratio ();

	if (max_ratio > 0) {
		switch (panel_applet_get_orient (applet)) {
		case PANEL_APPLET_ORIENT_UP:
		case PANEL_APPLET_ORIENT_DOWN:
			gtk_widget_set_size_request (sia->applet,
						     panel_applet_get_size
						     (applet)
						     * max_ratio,
						     panel_applet_get_size
						     (applet));
			break;
		case PANEL_APPLET_ORIENT_LEFT:
		case PANEL_APPLET_ORIENT_RIGHT:
			gtk_widget_set_size_request (sia->applet,
						     panel_applet_get_size
						     (applet),
						     panel_applet_get_size
						     (applet) / max_ratio);
			break;
		}
	}


	if (!g_slist_length (globals.applet_instances)) {
		xkl_debug (100, "*** First instance *** \n");

		globals.previewDialogs =
		    g_hash_table_new (g_direct_hash, g_direct_equal);

		xkl_debug (100,
			   "*** First instance inited globals *** \n");
	}

	g_signal_connect (G_OBJECT (sia->applet), "change_size",
			  G_CALLBACK (GSwitchItAppletChangePixelSize),
			  sia);
	g_signal_connect (G_OBJECT (sia->applet), "change_background",
			  G_CALLBACK (GSwitchItAppletChangeBackground),
			  sia);

	//??gtk_widget_add_events (sia->applet, GDK_BUTTON_PRESS_MASK);

	g_signal_connect (GTK_OBJECT (sia->applet), "destroy",
			  G_CALLBACK (GSwitchItAppletTerm), sia);
	gtk_object_set_data (GTK_OBJECT (sia->applet), "sia", sia);
	GSwitchItAppletSetupMenu (sia);
	return TRUE;
}

void
GSwitchItAppletTerm (PanelApplet * applet, GSwitchItApplet * sia)
{
	xkl_debug (100, "Starting the applet shutdown process for %p\n",
		   sia);

	globals.applet_instances =
	    g_slist_remove (globals.applet_instances, sia);

	if (!g_slist_length (globals.applet_instances)) {
		xkl_debug (100, "*** Last instance *** \n");

		g_hash_table_destroy (globals.previewDialogs);

		xkl_debug (100,
			   "*** Last instance terminated globals *** \n");
	}

	g_free (sia);
	xkl_debug (100, "The applet successfully terminated\n");
}

gboolean
GSwitchItAppletNew (PanelApplet * applet)
{
	gboolean rv = TRUE;
	GSwitchItApplet *sia;
#if 0
	GLogLevelFlags fatal_mask;
	fatal_mask = G_LOG_LEVEL_WARNING | G_LOG_LEVEL_CRITICAL;
	g_log_set_always_fatal (fatal_mask);
#endif
	sia = g_new0 (GSwitchItApplet, 1);
	rv = GSwitchItAppletInit (sia, applet);
	globals.applet_instances =
	    g_slist_append (globals.applet_instances, sia);

	xkl_debug (100, "The applet successfully started: %d\n", rv);
	return rv;
}
