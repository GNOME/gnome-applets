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

#include <gdk/gdkscreen.h>
#include <gdk/gdkx.h>
#include <glade/glade.h>

#include <libxklavier/xklavier_config.h>

#define GROUPS_SUBMENU_PATH "/popups/popup/groups"

/* we never build applet without XKB, so far: TODO */
#define HAVE_XKB  1

#ifdef HAVE_XKB
#include "X11/XKBlib.h"
/**
 * BAD STYLE: Taken from xklavier_private_xkb.h
 * Any ideas on architectural improvements are WELCOME
 */
extern Bool _XklXkbConfigPrepareNative( const XklConfigRecPtr data, XkbComponentNamesPtr componentNamesPtr );
extern void _XklXkbConfigCleanupNative( XkbComponentNamesPtr componentNamesPtr );
/* */
#endif

/* one instance for ALL applets */
static GSwitchItAppletGlobals globals;

#define GSwitchItAppletFirstInstance() \
	(!g_slist_length (globals.appletInstances))

#define GSwitchItAppletLastInstance() \
	(!g_slist_length (globals.appletInstances))

#define ForAllApplets() \
	{ \
		GSList* cur; \
		for (cur = globals.appletInstances; cur != NULL; cur = cur->next) { \
			GSwitchItApplet * sia = (GSwitchItApplet*)cur->data;
#define NextApplet \
		} \
	}

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

static void GSwitchItAppletFillNotebook (GSwitchItApplet * sia);

static void GSwitchItAppletCleanupNotebook (GSwitchItApplet * sia);

static gboolean GSwitchItAppletButtonPressed (GtkWidget * widget,
					      GdkEventButton * event,
					      GSwitchItAppletConfig *
					      config);

static gboolean GSwitchItAppletKeyPressed (GtkWidget * widget,
					   GdkEventKey * event,
					   GSwitchItAppletConfig * config);

static const BonoboUIVerb gswitchitAppletMenuVerbs[] = {
	BONOBO_UI_UNSAFE_VERB ("Capplet", GSwitchItAppletCmdCapplet),
	BONOBO_UI_UNSAFE_VERB ("Preview", GSwitchItAppletCmdPreview),
	BONOBO_UI_UNSAFE_VERB ("Plugins", GSwitchItAppletCmdPlugins),
	BONOBO_UI_UNSAFE_VERB ("About", GSwitchItAppletCmdAbout),
	BONOBO_UI_UNSAFE_VERB ("Help", GSwitchItAppletCmdHelp),
	BONOBO_UI_VERB_END
};

static void
GSwitchItAppletSetTooltip (GSwitchItApplet * sia, const char *str)
{
	GtkTooltips *tooltips;
	char buf[XKL_MAX_CI_DESC_LENGTH + 128];

	if (str == NULL)
		return;
	tooltips = gtk_tooltips_new ();
	g_object_ref (G_OBJECT (tooltips));
	gtk_object_sink (GTK_OBJECT (tooltips));
	g_object_set_data_full (G_OBJECT (sia->applet), "tooltips",
				tooltips, (GDestroyNotify) g_object_unref);
	g_snprintf (buf, sizeof (buf), _("Keyboard Indicator (%s)"), str);
	gtk_tooltips_set_tip (tooltips, sia->applet, buf, NULL);
}

void
GSwitchItAppletReinitUi (GSwitchItApplet * sia)
{
	const char *pname;
	XklState *currentState;

	GSwitchItAppletCleanupGroupsSubmenu (sia);
	GSwitchItAppletSetupGroupsSubmenu (sia);

	GSwitchItAppletCleanupNotebook (sia);
	GSwitchItAppletFillNotebook (sia);

	GSwitchItAppletRevalidate (sia);

	/* also, update tooltips */
	currentState = XklGetCurrentState ();
	if (currentState->group >= 0) {
		pname = g_slist_nth_data (globals.groupNames, currentState->group);
		GSwitchItAppletSetTooltip (sia, pname);
	}
}

/* Should be called once for all applets */
static void
GSwitchItConfigChanged (GConfClient * client,
			guint cnxn_id,
			GConfEntry * entry)
{
	XklDebug (100,
		  "General configuration changed in GConf - reiniting...\n");
	GSwitchItConfigLoadFromGConf (&globals.config);
	GSwitchItConfigActivate (&globals.config);
	ForAllApplets ()
		GSwitchItAppletReinitUi (sia);
	NextApplet
}

/* Should be called once for all applets */
static void
GSwitchItAppletConfigChanged (GConfClient * client,
			      guint cnxn_id,
			      GConfEntry * entry)
{
	GSList* enabledPlugins = NULL;

	XklDebug (100,
		  "Applet configuration changed in GConf - reiniting...\n");
	GSwitchItAppletConfigLoadFromGConf (&globals.appletConfig);
	GSwitchItAppletConfigUpdateImages (&globals.appletConfig, &globals.kbdConfig);
	GSwitchItAppletConfigActivate (&globals.appletConfig);
	
	GSwitchItPluginManagerTogglePlugins (&globals.pluginManager,
					     &globals.pluginContainer,
					     globals.appletConfig.enabledPlugins);

	ForAllApplets ()
		GSwitchItAppletReinitUi (sia);
	NextApplet
}

/* Should be called once for all applets */
static void
GSwitchItAppletKbdConfigCallback (void)
{
	XklDebug (100,
		  "XKB configuration changed on X Server - reiniting...\n");

	GSwitchItKbdConfigLoadFromXCurrent (&globals.kbdConfig);
	GSwitchItAppletConfigUpdateImages (&globals.appletConfig, &globals.kbdConfig);

	while (globals.groupNames != NULL) {
		GSList * nn = globals.groupNames;
		globals.groupNames = g_slist_remove_link (globals.groupNames, nn);
		g_free (nn->data);
		g_slist_free_1 (nn);
	}
	globals.groupNames = GSwitchItConfigLoadGroupDescriptionsUtf8 (&globals.config);

	ForAllApplets ()
		GSwitchItAppletReinitUi (sia);
	NextApplet
}

/* Should be called once for all applets */
static void
GSwitchItAppletStateCallback (XklStateChange changeType,
			      int group, Bool restore)
{
	XklDebug (150, "group is now %d, restore: %d\n", group, restore);

	if (changeType == GROUP_CHANGED) {
		GSwitchItPluginManagerGroupChanged (&globals.pluginManager,
						    group);
		ForAllApplets ()
			XklDebug (200, "do repaint\n");
			GSwitchItAppletRevalidateGroup (sia, group);
		NextApplet
	}
}

/* Should be called once for all applets */
GdkFilterReturn
GSwitchItAppletFilterXEvt (GdkXEvent * xev, GdkEvent * event)
{
	XEvent *xevent = (XEvent *) xev;

	XklFilterEvents (xevent);
	switch (xevent->type) {
	case ReparentNotify:
		{
			XReparentEvent *rne = (XReparentEvent *) xev;

			ForAllApplets ()
				GdkWindow * w = sia->appletAncestor;
				/* compare the applet's parent window with the even window */
				if (w != NULL && GDK_WINDOW_XID (w) == rne->window) {
					/* if so - make it transparent...*/
					XklSetTransparent (rne->window, TRUE);
					break; /* once is enough */
				}
			NextApplet
		}
		break;
	}

	return GDK_FILTER_CONTINUE;
}

void
GSwitchItAppletRevalidate (GSwitchItApplet * sia)
{
	XklState *currentState;
	currentState = XklGetCurrentState ();
	if (currentState->group >= 0)
		GSwitchItAppletRevalidateGroup (sia, currentState->group);
}

static void
GSwitchItAppletCleanupNotebook (GSwitchItApplet * sia)
{
	int i;
	GtkNotebook *notebook = GTK_NOTEBOOK (sia->notebook);

	/* Do not remove the first page! It is the default page */
	for (i = gtk_notebook_get_n_pages (notebook); --i > 0;) {
		gtk_notebook_remove_page (notebook, i);
	}
}

static GtkWidget *
GSwitchItAppletPrepareDrawing (GSwitchItApplet * sia, int group)
{
	gpointer pimage;
	GdkPixbuf *image;
	GdkPixbuf *scaled;
	PanelAppletOrient orient;
	int psize, xsize = 0, ysize = 0;
	double xyratio;
	pimage = g_slist_nth_data (globals.appletConfig.images, group);
	sia->ebox = gtk_event_box_new ();
	if (globals.appletConfig.showFlags) {
		GtkWidget *flagImg;
		if (pimage == NULL)
			return NULL;
		image = GDK_PIXBUF (pimage);
		orient =
		    panel_applet_get_orient (PANEL_APPLET (sia->applet));
		psize =
		    panel_applet_get_size (PANEL_APPLET (sia->applet)) - 4;
		XklDebug (150, "Resizing the applet for size %d\n", psize);
		xyratio =
		    1.0 * gdk_pixbuf_get_width (image) /
		    gdk_pixbuf_get_height (image);
		switch (orient) {
		case PANEL_APPLET_ORIENT_UP:
		case PANEL_APPLET_ORIENT_DOWN:
			ysize = psize;
			xsize = psize * xyratio;
			break;
		case PANEL_APPLET_ORIENT_LEFT:
		case PANEL_APPLET_ORIENT_RIGHT:
			ysize = psize / xyratio;
			break;
		}

		scaled =
		    gdk_pixbuf_scale_simple (image, xsize, ysize,
					     GDK_INTERP_HYPER);
		flagImg = gtk_image_new_from_pixbuf (scaled);
		gtk_container_add (GTK_CONTAINER (sia->ebox), flagImg);
		g_object_unref (G_OBJECT (scaled));
	} else {
		char *layoutName = NULL;
		XklConfigItem configItem;
		GtkWidget *align, *label;
		static GHashTable *shortDescrs = NULL;
		if (group == 0)
			shortDescrs = g_hash_table_new_full (g_str_hash, g_str_equal,
							     g_free, NULL);

		if (XklGetBackendFeatures() & XKLF_MULTIPLE_LAYOUTS_SUPPORTED) {
			char *fullLayoutName =
			    (char *) g_slist_nth_data (globals.kbdConfig.layouts, 
						       group);
			char *variantName;
			if (!GSwitchItKbdConfigSplitItems
			    (fullLayoutName, &layoutName, &variantName))
				/* just in case */
				layoutName = g_strdup (fullLayoutName);

			g_snprintf (configItem.name,
				    sizeof (configItem.name), "%s",
				    layoutName);

			if (XklConfigFindLayout (&configItem)) {
				char *sd = configItem.shortDescription;
				if (sd != NULL && *sd != '\0') {
					layoutName = 
					    g_locale_to_utf8 (sd, -1, NULL,
							      NULL, NULL);
				}
			}
		} else
			layoutName = g_strdup (g_slist_nth_data (globals.groupNames, group));

		gpointer pcounter = NULL; 
		char *prevLayoutName = NULL;
		char *labelTitle = NULL;
		int counter = 0;
		if (g_hash_table_lookup_extended (shortDescrs, layoutName, 
						  (gpointer*)&prevLayoutName, &pcounter))
		{
			/* "next" same description */
			char* appendix = NULL;
			counter = GPOINTER_TO_INT(pcounter);
			appendix = g_strnfill (counter, '*');
			//labelTitle = g_strconcat (layoutName, "(", appendix, ")", NULL);
			labelTitle = g_strconcat (layoutName, appendix, NULL);
			g_free (appendix);
			/* layoutName is g_freed inside g_hash_table_insert function */
		}
		else
		{
			/* "first" time this description */
			labelTitle = g_strdup (layoutName);
		}
		g_hash_table_insert (shortDescrs, layoutName, GINT_TO_POINTER (counter + 1));

		align = gtk_alignment_new (0.5, 0.5, 1.0, 1.0);
		label = gtk_label_new (labelTitle);
		g_free (labelTitle);

		if (group == XklGetNumGroups ())
			g_hash_table_destroy (shortDescrs);

		gtk_container_add (GTK_CONTAINER (align), label);
		gtk_container_add (GTK_CONTAINER (sia->ebox), align);

		gtk_container_set_border_width (GTK_CONTAINER (align), 2);
	}
	g_signal_connect (G_OBJECT (sia->ebox),
			  "button_press_event",
			  G_CALLBACK
			  (GSwitchItAppletButtonPressed), sia);

	g_signal_connect (G_OBJECT (sia->applet),
			  "key_press_event",
			  G_CALLBACK
			  (GSwitchItAppletKeyPressed), sia);

	return sia->ebox;
}

static void
GSwitchItAppletFillNotebook (GSwitchItApplet * sia)
{
	int grp;
	int totalGroups = XklGetNumGroups ();
	GtkNotebook *notebook = GTK_NOTEBOOK (sia->notebook);

	for (grp = 0; grp < totalGroups; grp++) {
		GtkWidget *page, *decoratedPage;
		XklDebug (150,
			  "The widget for the group %d is not ready, let's prepare it!\n",
			  grp);
		page = GSwitchItAppletPrepareDrawing (sia, grp);

		decoratedPage =
		    GSwitchItPluginManagerDecorateWidget (&globals.pluginManager,
							  page, grp,
							  g_slist_nth_data (
								globals.groupNames,
								grp),
							  &globals.kbdConfig);

		gtk_notebook_append_page (notebook,
					  decoratedPage ==
					  NULL ? page : decoratedPage,
					  gtk_label_new (""));

		gtk_widget_show_all (page);
	}
}

void
GSwitchItAppletRevalidateGroup (GSwitchItApplet * sia, int group)
{
	const char *pname;
	XklDebug (200, "Revalidating for group %d\n", group);

	gtk_notebook_set_current_page (GTK_NOTEBOOK (sia->notebook), group + 1);

	pname = g_slist_nth_data (globals.groupNames, group);
	GSwitchItAppletSetTooltip (sia, pname);
}

static void
GSwitchItAppletChangePixelSize (PanelApplet *
				widget, guint size, GSwitchItApplet * sia)
{
	GSwitchItAppletCleanupNotebook (sia);
	GSwitchItAppletFillNotebook (sia);

	GSwitchItAppletRevalidate (sia);
}

static void 
GSwitchItAppletSetBackground(PanelAppletBackgroundType type,
			     GtkRcStyle * rc_style,
			     GtkWidget * w, 
			     GdkColor * color,
			     GdkPixmap * pixmap)
{
	GtkStyle *style;

	gtk_widget_set_style (GTK_WIDGET (w), NULL);
	gtk_widget_modify_style (GTK_WIDGET (w), rc_style);

	switch (type)
	{
	case PANEL_NO_BACKGROUND:
	        break;
	case PANEL_COLOR_BACKGROUND:
	        gtk_widget_modify_bg (GTK_WIDGET (w),
	                              GTK_STATE_NORMAL, color);
		break;
	case PANEL_PIXMAP_BACKGROUND:
		style = gtk_style_copy (w->style);
		if (style->bg_pixmap[GTK_STATE_NORMAL])
			g_object_unref (style->bg_pixmap[GTK_STATE_NORMAL]);
		style->bg_pixmap[GTK_STATE_NORMAL] = g_object_ref (pixmap);
		gtk_widget_set_style (w, style);
		g_object_unref (style);
		break;
	}
	/* go down */
	if (GTK_IS_CONTAINER (w))
	{
		GList * child = gtk_container_get_children (GTK_CONTAINER (w));
		while (child != NULL)
		{
			GSwitchItAppletSetBackground (type, rc_style,
			     GTK_WIDGET (child->data), color, pixmap);
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
	GSwitchItAppletSetBackground(type, rc_style, sia->applet, color, pixmap);
	gtk_rc_style_unref (rc_style);
}

static gboolean
GSwitchItAppletKeyPressed (GtkWidget *
			   widget,
			   GdkEventKey *
			   event, GSwitchItAppletConfig * config)
{
	switch (event->keyval) {
	case GDK_KP_Enter:
	case GDK_ISO_Enter:
	case GDK_3270_Enter:
	case GDK_Return:
	case GDK_space:
	case GDK_KP_Space:
		GSwitchItConfigLockNextGroup ();
		return TRUE;
	default:
		break;
	}
	return FALSE;
}

static gboolean
GSwitchItAppletButtonPressed (GtkWidget *
			      widget,
			      GdkEventButton *
			      event, GSwitchItAppletConfig * config)
{
	if (event->button == 1 && event->type == GDK_BUTTON_PRESS) {
		XklDebug (150, "Mouse button pressed on applet\n");
		GSwitchItConfigLockNextGroup ();
		return TRUE;
	}
	return FALSE;
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
GSwitchItPreviewResponse (GtkWidget *dialog, gint resp)
{
	switch (resp) 
	{
		case GTK_RESPONSE_CLOSE:
			gtk_widget_destroy (dialog);
		default:;
	}
}

void
GSwitchItAppletCmdPreview (BonoboUIComponent *
			   uic, GSwitchItApplet * sia, const gchar * verb)
{
#ifdef HAVE_XKB
	/* TODO: set proper window title */
	/* TODO: set window-per-layout */
	GladeXML *gladeData = glade_xml_new (GNOME_GLADEDIR "/gswitchit.glade", "gswitchit_layout_view", NULL);
        GtkWidget *dialog =
        	glade_xml_get_widget (gladeData, "gswitchit_layout_view");
	static KeyboardDrawingGroupLevel groupsLevels[] = {{0,1},{0,3},{0,0},{0,2}};
	static KeyboardDrawingGroupLevel * pGroupsLevels[] = {
		groupsLevels, groupsLevels+1, groupsLevels+2, groupsLevels+3 };
	GtkWidget *kbdraw = keyboard_drawing_new ();
	XkbComponentNamesRec componentNames;

  	keyboard_drawing_set_groups_levels (KEYBOARD_DRAWING (kbdraw), pGroupsLevels);

	XklConfigRec xklData;
	XklConfigRecInit (&xklData);
      	if (XklConfigGetFromServer (&xklData))
	{
		/* TODO: hack xklData to put the current group into the group 1 */
		if (_XklXkbConfigPrepareNative (&xklData, &componentNames))
		{
			keyboard_drawing_set_keyboard (KEYBOARD_DRAWING (kbdraw), &componentNames);
			_XklXkbConfigCleanupNative (&componentNames);
		}
	}
	XklConfigRecDestroy (&xklData);

	gtk_object_set_data (GTK_OBJECT (dialog), "gladeData", gladeData);
	g_signal_connect_swapped (GTK_OBJECT (dialog),
				  "destroy", G_CALLBACK (g_object_unref),
				  gladeData);
	g_signal_connect (G_OBJECT (dialog), "response", G_CALLBACK(GSwitchItPreviewResponse), NULL);

	gtk_window_resize (GTK_WINDOW (dialog), 700, 400);
	gtk_window_set_resizable (GTK_WINDOW (dialog), TRUE);

	gtk_container_add (GTK_CONTAINER (glade_xml_get_widget (gladeData, "preview_vbox")), kbdraw);
	
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
	XklState st;
	Window cur;
	if (sscanf (verb, "Group_%d", &st.group) != 1) {
		XklDebug (50, "Strange verb: [%s]\n", verb);
		return;
	}

	XklAllowOneSwitchToSecondaryGroup ();
	cur = XklGetCurrentWindow ();
	if (cur != (Window) NULL) {
		XklDebug (150, "Enforcing the state %d for window %lx\n",
			  st.group, cur);
		XklSaveState (XklGetCurrentWindow (), &st);
/*    XSetInputFocus( GDK_DISPLAY(), cur, RevertToNone, CurrentTime );*/
	} else {
		XklDebug (150,
			  "??? Enforcing the state %d for unknown window\n",
			  st.group);
		/* strange situation - bad things can happen */
	}
	XklLockGroup (st.group);
}

void
GSwitchItAppletCmdHelp (BonoboUIComponent
			* uic, GSwitchItApplet * sia, const gchar * verb)
{
	GSwitchItHelp (GTK_WIDGET (sia->applet), NULL);
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
			       "name",         _("Keyboard Indicator"),
			       "version",      VERSION,
/* Translators: Please replace (C) with the proper copyright character. */
			       "copyright",    _("Copyright (c) Sergey V. Udaltsov 1999-2004"),
			       "comments",     _("Keyboard layout indicator applet for GNOME"),
			       "authors",      authors,
			       "documenters",  documenters,
			       "translator-credits",  strcmp (translator_credits, "translator-credits") != 0 ? translator_credits : NULL,
			       "logo-icon-name",       "gswitchit-applet",
			       NULL);
}

static void
GSwitchItAppletCleanupGroupsSubmenu (GSwitchItApplet * sia)
{
	int i;
	BonoboUIComponent *popup;
	popup =
	    panel_applet_get_popup_component (PANEL_APPLET (sia->applet));
	for (i = 0;;i++) {
		char path[80];
		g_snprintf (path, sizeof (path),
			    GROUPS_SUBMENU_PATH "/Group_%d", i);
		if (bonobo_ui_component_path_exists (popup, path, NULL)) {
			bonobo_ui_component_rm (popup, path, NULL);
			XklDebug (160,
				  "Unregistered group menu item \'%s\'\n",
				  path);
		} else
			break;
	}

	XklDebug (160, "Unregistered group submenu\n");
}

static void
GSwitchItAppletSetupGroupsSubmenu (GSwitchItApplet * sia)
{
	unsigned i, nGroups;
	GSList *nameNode = globals.groupNames;
	BonoboUIComponent *popup;
	popup =
	    panel_applet_get_popup_component (PANEL_APPLET (sia->applet));
	XklDebug (160, "Registering group submenu\n");
	nGroups = g_slist_length (globals.groupNames);
	for (i = 0; i < nGroups; i++) {
		char verb[40];
		BonoboUINode *node;
		g_snprintf (verb, sizeof (verb), "Group_%d", i);
		node = bonobo_ui_node_new ("menuitem");
		bonobo_ui_node_set_attr (node, "name", verb);
		bonobo_ui_node_set_attr (node, "verb", verb);
		bonobo_ui_node_set_attr (node, "label", nameNode->data);
		bonobo_ui_node_set_attr (node, "pixtype", "filename");
		if (globals.appletConfig.showFlags) {
			char *imageFile =
			    GSwitchItAppletConfigGetImagesFile (&globals.appletConfig,
								&globals.kbdConfig,
								i);
			if (imageFile != NULL) {
				bonobo_ui_node_set_attr (node, "pixname",
							 imageFile);
				g_free (imageFile);
			}
		}
		bonobo_ui_component_set_tree (popup, GROUPS_SUBMENU_PATH,
					      node, NULL);
		bonobo_ui_component_add_verb (popup, verb, (BonoboUIVerbFn)
					      GSwitchItAppletCmdSetGroup,
					      sia);
		XklDebug (160,
			  "Registered group menu item \'%s\' as \'%s\'\n",
			  verb, nameNode->data);
		nameNode = g_slist_next (nameNode);
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

/* Should be called once for all applets */
static void
GSwitchItAppletStartListen (GSwitchItApplet * sia)
{
	gdk_window_add_filter (NULL,
			       (GdkFilterFunc) GSwitchItAppletFilterXEvt,
			       NULL);
	gdk_window_add_filter (gdk_get_default_root_window (),
			       (GdkFilterFunc) GSwitchItAppletFilterXEvt,
			       NULL);

	XklStartListen (XKLL_TRACK_KEYBOARD_STATE);
}

/* Should be called once for all applets */
static void
GSwitchItAppletStopListen (GSwitchItApplet * sia)
{
	XklStopListen ();

	gdk_window_remove_filter (NULL, (GdkFilterFunc)
				  GSwitchItAppletFilterXEvt, NULL);
	gdk_window_remove_filter
	    (gdk_get_default_root_window (),
	     (GdkFilterFunc) GSwitchItAppletFilterXEvt, NULL);
}

static void GSwitchItAppletTerm (PanelApplet *
				 applet, GSwitchItApplet * sia);

static gboolean
GSwitchItAppletInit (GSwitchItApplet * sia, PanelApplet * applet)
{
	GtkWidget *defDrawingArea;
	GtkNotebook *notebook;
	GConfClient *confClient;

	XklDebug (100, "Starting the applet startup process for %p\n", sia);

	glade_gnome_init ();
	gtk_window_set_default_icon_name ("gswitchit-applet");

	sia->applet = GTK_WIDGET (applet);

	panel_applet_set_flags (applet, PANEL_APPLET_EXPAND_MINOR);

	notebook = GTK_NOTEBOOK (sia->notebook = gtk_notebook_new ());
	gtk_notebook_set_show_tabs (notebook, FALSE);
	gtk_notebook_set_show_border (notebook, FALSE);

	gtk_container_add (GTK_CONTAINER (sia->applet),
			   GTK_WIDGET (notebook));

	defDrawingArea =
	    gtk_image_new_from_stock (GTK_STOCK_STOP,
				      GTK_ICON_SIZE_BUTTON);

	gtk_notebook_append_page (notebook, defDrawingArea,
				  gtk_label_new (""));

	GSwitchItAppletSetTooltip (sia, _(PACKAGE));
	gtk_widget_show_all (sia->applet);
	gtk_widget_realize (sia->applet);

	sia->appletAncestor = gtk_widget_get_parent_window (sia->applet);

	if (GSwitchItAppletFirstInstance ()) {
		/* GSwitchItInstallGlibLogAppender(  ); */
		if (XklInit (GDK_DISPLAY ()) != 0) {
			GSwitchItAppletSetTooltip (sia,
						   _("XKB initialization error"));
			return TRUE;
		}
		XklDebug (100, "*** First instance *** \n");

		XklConfigInit ();
		if (!XklConfigLoadRegistry ()) {
			GSwitchItAppletSetTooltip (sia,
						   _
						   ("Error loading XKB configuration registry"));
			return TRUE;
		}

		confClient = gconf_client_get_default ();

		memset (&globals, 0, sizeof(globals));

		XklRegisterStateCallback ((XklStateCallback)
					  GSwitchItAppletStateCallback, NULL);
		XklRegisterConfigCallback ((XklConfigCallback)
					   GSwitchItAppletKbdConfigCallback, NULL);
		GSwitchItPluginContainerInit (&globals.pluginContainer, confClient);


		GSwitchItConfigInit (&globals.config, confClient);
		GSwitchItKbdConfigInit (&globals.kbdConfig, confClient);
		GSwitchItAppletConfigInit (&globals.appletConfig, confClient);

		g_object_unref (confClient);

		GSwitchItConfigLoadFromGConf (&globals.config);
		GSwitchItConfigActivate (&globals.config);
		GSwitchItKbdConfigLoadFromXCurrent (&globals.kbdConfig);
		GSwitchItAppletConfigLoadFromGConf (&globals.appletConfig);
		GSwitchItAppletConfigUpdateImages (&globals.appletConfig, &globals.kbdConfig);
		GSwitchItAppletConfigActivate (&globals.appletConfig);

		globals.groupNames = GSwitchItConfigLoadGroupDescriptionsUtf8 (&globals.config);

		GSwitchItPluginManagerInit (&globals.pluginManager);
		GSwitchItPluginManagerInitEnabledPlugins (&globals.pluginManager,
							  &globals.pluginContainer,
							  globals.appletConfig.enabledPlugins);
		GSwitchItConfigStartListen (&globals.config,
					    (GConfClientNotifyFunc)
					    GSwitchItConfigChanged,
					    NULL);
		GSwitchItAppletConfigStartListen (&globals.appletConfig,
						  (GConfClientNotifyFunc)
						  GSwitchItAppletConfigChanged,
						  NULL);
		GSwitchItAppletStartListen (sia);

		XklDebug (100, "*** First instance inited globals *** \n");
	}
	GSwitchItAppletFillNotebook (sia);
	GSwitchItAppletRevalidate (sia);

	g_signal_connect (G_OBJECT (sia->applet), "change_size",
			  G_CALLBACK (GSwitchItAppletChangePixelSize), sia);
	g_signal_connect (G_OBJECT (sia->applet), "change_background",
			  G_CALLBACK (GSwitchItAppletChangeBackground), sia);
	g_signal_connect (G_OBJECT (sia->applet), "button_press_event",
			  G_CALLBACK (GSwitchItAppletButtonPressed), sia);

	gtk_widget_add_events (sia->applet, GDK_BUTTON_PRESS_MASK);

	g_signal_connect (GTK_OBJECT (sia->applet), "destroy",
			  G_CALLBACK (GSwitchItAppletTerm), sia);
	gtk_object_set_data (GTK_OBJECT (sia->applet), "sia", sia);
	GSwitchItAppletSetupMenu (sia);
	return TRUE;
}

void
GSwitchItAppletTerm (PanelApplet * applet, GSwitchItApplet * sia)
{
	XklDebug (100, "Starting the applet shutdown process for %p\n", sia);
	/* remove BEFORE all termination work is finished */
	globals.appletInstances = g_slist_remove (globals.appletInstances, sia);

	if (GSwitchItAppletLastInstance ()) {
		XklDebug (100, "*** Last instance *** \n");
		GSwitchItAppletStopListen (sia);

		GSwitchItConfigStopListen (&globals.config);
		GSwitchItAppletConfigStopListen (&globals.appletConfig);

		XklRegisterStateCallback (NULL, NULL);
		XklRegisterConfigCallback (NULL, NULL);

		GSwitchItPluginManagerTermInitializedPlugins (&globals.pluginManager);
		GSwitchItPluginManagerTerm (&globals.pluginManager);

		GSwitchItAppletConfigTerm (&globals.appletConfig);
		GSwitchItKbdConfigTerm (&globals.kbdConfig);
		GSwitchItConfigTerm (&globals.config);

		GSwitchItPluginContainerTerm (&globals.pluginContainer);

		XklConfigFreeRegistry ();
		XklConfigTerm ();
		XklDebug (100, "*** Last instance terminated globals *** \n");
		XklTerm ();
	}

	GSwitchItAppletCleanupNotebook (sia);

	g_free (sia);
	XklDebug (100, "The applet successfully terminated\n");
}

gboolean
GSwitchItAppletNew (PanelApplet * applet)
{
	gboolean rv = TRUE;
	GSwitchItApplet * sia;
#if 0
	GLogLevelFlags fatal_mask;
	fatal_mask = G_LOG_LEVEL_WARNING | G_LOG_LEVEL_CRITICAL;
	g_log_set_always_fatal (fatal_mask);
#endif
	sia = g_new0 (GSwitchItApplet, 1);
	rv = GSwitchItAppletInit (sia, applet);
	/* append AFTER all initialization work is finished */
	globals.appletInstances = g_slist_append (globals.appletInstances, sia);
	XklDebug (100, "The applet successfully started: %d\n", rv);
	return rv;
}

/* Plugin support */
void
GSwitchItPluginContainerReinitUi (GSwitchItPluginContainer * pc)
{
	ForAllApplets ()
		GSwitchItAppletReinitUi (sia);
	NextApplet
}

GSList *
GSwitchItPluginLoadLocalizedGroupNames (GSwitchItPluginContainer * pc)
{
	return globals.groupNames;
}
