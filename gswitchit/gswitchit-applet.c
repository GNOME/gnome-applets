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
#include <libgnomekbd/gkbd-desktop-config.h>
#include <libgnomekbd/gkbd-util.h>
#include <libgnomekbd/gkbd-keyboard-drawing.h>

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include <gdk/gdk.h>
#include <gdk/gdkx.h>

#define GROUPS_SUBMENU_PATH "/popups/popup/groups"
#define PLUGINS_CAPPLET_EXECUTABLE "gkbd-indicator-plugins-capplet"

#define GTK_RESPONSE_PRINT 2

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

static void GSwitchItAppletUpdateBackground (GSwitchItApplet * sia);

static const BonoboUIVerb gswitchitAppletMenuVerbs[] = {
	BONOBO_UI_UNSAFE_VERB ("Capplet", GSwitchItAppletCmdCapplet),
	BONOBO_UI_UNSAFE_VERB ("Plugins", GSwitchItAppletCmdPlugins),
	BONOBO_UI_UNSAFE_VERB ("Preview", GSwitchItAppletCmdPreview),
	BONOBO_UI_UNSAFE_VERB ("About", GSwitchItAppletCmdAbout),
	BONOBO_UI_UNSAFE_VERB ("Help", GSwitchItAppletCmdHelp),
	BONOBO_UI_VERB_END
};

void
GSwitchItAppletReinitUi (GkbdIndicator * gki, GSwitchItApplet * sia)
{
	GSwitchItAppletCleanupGroupsSubmenu (sia);
	GSwitchItAppletSetupGroupsSubmenu (sia);
	GSwitchItAppletUpdateBackground (sia);
}

static void
GSwitchItAppletChangePixelSize (PanelApplet *
				widget, guint size, GSwitchItApplet * sia)
{
	gkbd_indicator_reinit_ui (GKBD_INDICATOR (sia->gki));
}

static void
GSwitchitAppletUpdateAngle (GSwitchItApplet * sia,
			    PanelAppletOrient orient)
{
	gdouble angle = 0;
	switch (orient) {
	case PANEL_APPLET_ORIENT_UP:
	case PANEL_APPLET_ORIENT_DOWN:
		break;
	case PANEL_APPLET_ORIENT_LEFT:
	case PANEL_APPLET_ORIENT_RIGHT:
		angle = orient == PANEL_APPLET_ORIENT_LEFT ? 270 : 90;
	}
	gkbd_indicator_set_angle (GKBD_INDICATOR (sia->gki), angle);
	gkbd_indicator_reinit_ui (GKBD_INDICATOR (sia->gki));
}

static void
GSwitchItAppletChangeOrient (PanelApplet * widget,
			     PanelAppletOrient orient,
			     GSwitchItApplet * sia)
{
	GSwitchitAppletUpdateAngle (sia, orient);
}

static void
GSwitchItAppletSetBackground (PanelApplet * applet, GtkWidget * w)
{
	panel_applet_set_background_widget (applet, w);

	/* go down */
	if (GTK_IS_CONTAINER (w)) {
                GList *child;
		GList *children =
		    gtk_container_get_children (GTK_CONTAINER (w));
		child = children;
		while (child != NULL) {
			GSwitchItAppletSetBackground (applet,
						      GTK_WIDGET (child->
								  data));
			child = child->next;
		}
		g_list_free (children);
	}
}

static void
GSwitchItAppletChangeBackground (GtkWidget * widget,
				 PanelAppletBackgroundType type,
				 GdkColor * color, GdkPixmap * pixmap,
				 GSwitchItApplet * sia)
{
		
	GSwitchItAppletSetBackground (PANEL_APPLET (sia->applet), widget);
}

void
GSwitchItAppletUpdateBackground (GSwitchItApplet * sia)
{
	GdkColor color;
	GdkPixmap *pixmap;
	PanelAppletBackgroundType bt =
	    panel_applet_get_background (PANEL_APPLET (sia->applet),
					 &color, &pixmap);
	GSwitchItAppletChangeBackground (sia->applet, bt, &color, pixmap,
					 sia);
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
		g_warning
		    ("Could not execute keyboard properties capplet: [%s]\n",
		     error->message);
		g_error_free (error);
	}
}

static void
GSwitchItPreviewResponse (GtkWidget * dialog, gint resp)
{
	GdkRectangle rect;
	GtkWidget *kbdraw;
	const gchar *groupName;

	switch (resp) {
	case GTK_RESPONSE_HELP:
		gtk_show_uri (gtk_widget_get_screen (GTK_WIDGET (dialog)),
			"ghelp:gswitchit?layout-view",
			gtk_get_current_event_time (),
			NULL);
		return;
	case GTK_RESPONSE_CLOSE:
		gtk_window_get_position (GTK_WINDOW (dialog), &rect.x,
					 &rect.y);
		gtk_window_get_size (GTK_WINDOW (dialog), &rect.width,
				     &rect.height);
		gkbd_preview_save_position (&rect);
		gtk_widget_destroy (dialog);
		break;
	case GTK_RESPONSE_PRINT:
		kbdraw =
		    GTK_WIDGET (g_object_get_data
				(G_OBJECT (dialog), "kbdraw"));
		groupName =
		    (const gchar *) g_object_get_data (G_OBJECT (dialog),
						       "groupName");
		gkbd_keyboard_drawing_print (GKBD_KEYBOARD_DRAWING
					     (kbdraw), GTK_WINDOW (dialog),
					     groupName ? groupName :
					     _("Unknown"));
	}
}

static void
GSwitchItPreviewDestroy (GtkWidget * dialog, gint group)
{
	GtkBuilder *builder =
	    GTK_BUILDER (g_object_get_data
			 (GTK_OBJECT (dialog), "builderData"));
	g_object_unref (G_OBJECT (builder));
	g_hash_table_remove (globals.previewDialogs,
			     GINT_TO_POINTER (group));
}

void
GSwitchItAppletCmdPreview (BonoboUIComponent *
			   uic, GSwitchItApplet * sia, const gchar * verb)
{
	static GkbdKeyboardDrawingGroupLevel groupsLevels[] = { {
								 0, 1}, {
									 0,
									 3},
	{
	 0, 0}, {
		 0, 2}
	};
	static GkbdKeyboardDrawingGroupLevel *pGroupsLevels[] = {
		groupsLevels, groupsLevels + 1, groupsLevels + 2,
		groupsLevels + 3
	};
#ifdef HAVE_XKB
	GtkBuilder *builder;
	GtkWidget *dialog, *kbdraw;
	XkbComponentNamesRec componentNames;
	XklConfigRec *xklData;
	GdkRectangle *rect;
	GError *error = NULL;
#endif
	XklEngine *engine = gkbd_indicator_get_xkl_engine ();
	XklState *xklState = xkl_engine_get_current_state (engine);
	gchar **groupNames = gkbd_indicator_get_group_names ();
	gpointer p = g_hash_table_lookup (globals.previewDialogs,
					  GINT_TO_POINTER (xklState->
							   group));
	if (p != NULL) {
		/* existing window */
		gtk_window_present (GTK_WINDOW (p));
		return;
	}
#ifdef HAVE_XKB
	builder = gtk_builder_new ();
	gtk_builder_add_from_file (builder, GTK_BUILDERDIR "/gswitchit.ui",
				   &error);

	if (error) {
		g_error ("building ui from %s failed: %s",
			 GTK_BUILDERDIR "/gswitchit.ui", error->message);
		g_clear_error (&error);
	}


	dialog =
	    GTK_WIDGET (gtk_builder_get_object
			(builder, "gswitchit_layout_view"));
	kbdraw = gkbd_keyboard_drawing_new ();

	if (xklState->group >= 0 &&
	    xklState->group < g_strv_length (groupNames)) {
		char title[128] = "";
		snprintf (title, sizeof (title),
			  _("Keyboard Layout \"%s\""),
			  groupNames[xklState->group]);
		gtk_window_set_title (GTK_WINDOW (dialog), title);
		g_object_set_data_full (G_OBJECT (dialog), "groupName",
					g_strdup (groupNames
						  [xklState->group]),
					g_free);
	}

	gkbd_keyboard_drawing_set_groups_levels (GKBD_KEYBOARD_DRAWING
						 (kbdraw), pGroupsLevels);

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
			gkbd_keyboard_drawing_set_keyboard
			    (GKBD_KEYBOARD_DRAWING (kbdraw),
			     &componentNames);
			xkl_xkb_config_native_cleanup (engine,
						       &componentNames);
		}
	}
	g_object_unref (G_OBJECT (xklData));

	g_object_set_data (GTK_OBJECT (dialog), "builderData", builder);
	g_signal_connect (GTK_OBJECT (dialog),
			  "destroy", G_CALLBACK (GSwitchItPreviewDestroy),
			  GINT_TO_POINTER (xklState->group));
	g_signal_connect (G_OBJECT (dialog), "response",
			  G_CALLBACK (GSwitchItPreviewResponse), NULL);

	rect = gkbd_preview_load_position ();
	if (rect != NULL) {
		gtk_window_move (GTK_WINDOW (dialog), rect->x, rect->y);
		gtk_window_resize (GTK_WINDOW (dialog), rect->width,
				   rect->height);
		g_free (rect);
	} else
		gtk_window_resize (GTK_WINDOW (dialog), 700, 400);

	gtk_window_set_resizable (GTK_WINDOW (dialog), TRUE);

	gtk_container_add (GTK_CONTAINER
			   (gtk_builder_get_object
			    (builder, "preview_vbox")), kbdraw);

	g_object_set_data (GTK_OBJECT (dialog), "kbdraw", kbdraw);

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
					  PLUGINS_CAPPLET_EXECUTABLE,
					  &error);

	if (error != NULL) {
		g_warning ("Could not execute plugins capplet: [%s]\n",
			   error->message);
		g_error_free (error);
	}
}
static void
GSwitchItAppletCmdSetGroup (BonoboUIComponent
			    * uic, GSwitchItApplet * sia,
			    const gchar * verb)
{
	XklEngine *engine = gkbd_indicator_get_xkl_engine ();
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
	gtk_show_uri (gtk_widget_get_screen (GTK_WIDGET (sia->applet)),
		"ghelp:gswitchit-view",
		gtk_get_current_event_time (),
		NULL);
} void
GSwitchItAppletCmdAbout (BonoboUIComponent *
			 uic, GSwitchItApplet * sia, const gchar * verb)
{
	const gchar *authors[] = {
		"Sergey V. Udaltsov <svu@gnome.org>", NULL
	};
	const gchar *documenters[] = {
		"Sergey V. Udaltsov <svu@gnome.org>", NULL
	};

	const gchar *translator_credits = _("translator-credits");

	gtk_show_about_dialog (NULL, "version", VERSION,
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
			       "logo-icon-name", "input-keyboard", NULL);
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
	gchar **currentName = gkbd_indicator_get_group_names ();
	BonoboUIComponent *popup;
	popup =
	    panel_applet_get_popup_component (PANEL_APPLET (sia->applet));
	xkl_debug (160, "Registering group submenu\n");
	for (i = 0; *currentName; i++, currentName++) {
		char verb[40];
		BonoboUINode *node;
		gchar *imageFile;
		g_snprintf (verb, sizeof (verb), "Group_%d", i);
		node = bonobo_ui_node_new ("menuitem");
		bonobo_ui_node_set_attr (node, "name", verb);
		bonobo_ui_node_set_attr (node, "verb", verb);
		bonobo_ui_node_set_attr (node, "label", *currentName);
		bonobo_ui_node_set_attr (node, "pixtype", "filename");
		imageFile = gkbd_indicator_get_image_filename (i);
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
                bonobo_ui_node_unref (node);
		xkl_debug (160,
			   "Registered group menu item \'%s\' as \'%s\'\n",
			   verb, *currentName);
	}
}

static void
GSwitchItAppletSetupMenu (GSwitchItApplet * sia)
{
	BonoboUIComponent *popup;
	gchar *pluginsCappletFullPath =
	    g_find_program_in_path (PLUGINS_CAPPLET_EXECUTABLE);
	XklEngine *engine = gkbd_indicator_get_xkl_engine ();
	const char *engine_backend_name =
	    xkl_engine_get_backend_name (engine);

	panel_applet_setup_menu_from_file
	    (PANEL_APPLET (sia->applet), DATADIR,
	     "GNOME_GSwitchItApplet.xml", NULL, gswitchitAppletMenuVerbs,
	     sia);

	popup =
	    panel_applet_get_popup_component (PANEL_APPLET (sia->applet));

	/* Layout preview is for XKB only */
	bonobo_ui_component_set_prop (popup, "/commands/Preview",
				      "sensitive",
				      g_ascii_strcasecmp ("XKB",
							  engine_backend_name)
				      ? "0" : "1", NULL);

	/* Delete the plugins item if the binary does not exist */
	if (pluginsCappletFullPath == NULL)
		bonobo_ui_component_rm (popup,
					"/popups/popup/Plugins", NULL);
	else
		g_free (pluginsCappletFullPath);

	GSwitchItAppletSetupGroupsSubmenu (sia);
}

static void GSwitchItAppletTerm (PanelApplet *
				 applet, GSwitchItApplet * sia);

static gboolean
GSwitchItAppletInit (GSwitchItApplet * sia, PanelApplet * applet)
{
	gdouble max_ratio;
	PanelAppletOrient orient;

	xkl_debug (100, "Starting the applet startup process for %p\n",
		   sia);

	g_set_application_name (_("Keyboard Indicator"));

	gtk_window_set_default_icon_name ("input-keyboard");

	sia->applet = GTK_WIDGET (applet);

	panel_applet_set_flags (applet, PANEL_APPLET_EXPAND_MINOR);

	/* enable transparency hack */
	panel_applet_set_background_widget (PANEL_APPLET (applet),
					    GTK_WIDGET (applet));

	sia->gki = gkbd_indicator_new ();

	orient = panel_applet_get_orient (applet);
	GSwitchitAppletUpdateAngle (sia, orient);

	gtk_container_add (GTK_CONTAINER (sia->applet),
			   GTK_WIDGET (sia->gki));

	gkbd_indicator_set_tooltips_format (_("Keyboard Indicator (%s)"));
	gkbd_indicator_set_parent_tooltips (GKBD_INDICATOR
					    (sia->gki), TRUE);


	gtk_widget_show_all (sia->applet);
	gtk_widget_realize (sia->applet);

	max_ratio = gkbd_indicator_get_max_width_height_ratio ();

	if (max_ratio > 0) {
		switch (orient) {
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
	g_signal_connect (G_OBJECT (sia->applet), "change_orient",
			  G_CALLBACK (GSwitchItAppletChangeOrient), sia);

	g_signal_connect (GTK_OBJECT (sia->applet), "destroy",
			  G_CALLBACK (GSwitchItAppletTerm), sia);

	g_signal_connect (GTK_OBJECT (sia->gki), "reinit-ui",
			  G_CALLBACK (GSwitchItAppletReinitUi), sia);

	g_object_set_data (GTK_OBJECT (sia->applet), "sia", sia);
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
