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

#include <gdk/gdkscreen.h>
#include <gdk/gdkx.h>
#include <gnome.h>
#include <glade/glade.h>

#include <libxklavier/xklavier_config.h>

#include "gswitchit-applet.h"

#include "libgswitchit/gswitchit_util.h"
#include "libgswitchit/gswitchit_plugin_manager.h"

#if 1
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#define GROUPS_SUBMENU_PATH "/popups/popup/groups"

static gboolean terminatedOnce = FALSE;
static GSwitchItApplet *theAppletInstance = NULL;

static void GSwitchItAppletCmdProps (BonoboUIComponent * uic,
				     GSwitchItApplet * sia,
				     const gchar * verb);

static void GSwitchItAppletCmdCapplet (BonoboUIComponent * uic,
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

static GdkFilterReturn GSwitchItAppletWmProtocolsFilter (GdkXEvent * xev,
							 GdkEvent * event,
							 gpointer data);

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
	BONOBO_UI_UNSAFE_VERB ("Props", GSwitchItAppletCmdProps),
	BONOBO_UI_UNSAFE_VERB ("Capplet", GSwitchItAppletCmdCapplet),
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
		pname = sia->groupNames[currentState->group];
		GSwitchItAppletSetTooltip (sia, pname);
	}
}

static void
GSwitchItAppletConfigChanged (GConfClient * client,
			      guint cnxn_id,
			      GConfEntry * entry, GSwitchItApplet * sia)
{
	XklDebug (100,
		  "Applet configuration changed in GConf - reiniting...\n");

	GSwitchItAppletConfigLoad (&sia->appletConfig);
	GSwitchItAppletConfigUpdateImages (&sia->appletConfig,
					   &sia->xkbConfig);
	GSwitchItAppletConfigActivate (&sia->appletConfig);
	GSwitchItPluginManagerTogglePlugins (&sia->pluginManager,
					     &sia->pluginContainer,
					     sia->appletConfig.
					     enabledPlugins);
	GSwitchItAppletReinitUi (sia);
}

static int
GSwitchItAppletWindowCallback (Window win, Window parent,
			       GSwitchItApplet * sia)
{
	return GSwitchItPluginManagerWindowCreated (&sia->pluginManager,
						    win, parent);
}

static void
GSwitchItAppletXkbConfigCallback (GSwitchItApplet * sia)
{
	XklDebug (100,
		  "XKB configuration changed on X Server - reiniting...\n");
	GSwitchItXkbConfigLoadCurrent (&sia->xkbConfig);
	GSwitchItAppletConfigUpdateImages (&sia->appletConfig,
					   &sia->xkbConfig);
	GSwitchItAppletConfigLoadGroupDescriptionsUtf8 (&sia->appletConfig,
							sia->groupNames);

	GSwitchItAppletReinitUi (sia);
}

static void
GSwitchItAppletStateCallback (XklStateChange
			      changeType,
			      int group, Bool restore,
			      GSwitchItApplet * sia)
{
	XklDebug (150, "group is now %d, restore: %d\n", group, restore);

	if (changeType == GROUP_CHANGED) {
		GSwitchItPluginManagerGroupChanged (&sia->pluginManager,
						    group);
		XklDebug (200, "do repaint\n");
		GSwitchItAppletRevalidateGroup (sia, group);
	}
}

GdkFilterReturn
GSwitchItAppletFilterXEvt (GdkXEvent * xev,
			   GdkEvent * event, GSwitchItApplet * sia)
{
	XEvent *xevent = (XEvent *) xev;
	Display *display = xevent->xany.display;
	XklFilterEvents (xevent);
	switch (xevent->type) {
	case ReparentNotify:
		{
			XReparentEvent *rne = (XReparentEvent *) xev;
			GtkWidget *w1;
			GdkWindow *w;
			w1 = gtk_widget_get_ancestor (sia->applet,
						      GTK_TYPE_WINDOW);
			if (w1 == NULL)
				break;
			w = w1->window;
			if (w == NULL || GDK_WINDOW_XID (w) != rne->window)
				break;
			XklSetTransparent (GDK_WINDOW_XID (w), TRUE);
		}
		break;
	}

	return GDK_FILTER_CONTINUE;
}

GdkFilterReturn
GSwitchItAppletWmProtocolsFilter (GdkXEvent *
				  xev, GdkEvent * event, gpointer data)
{
	XEvent *xevent = (XEvent *) xev;
	Display *display = xevent->xany.display;
	if ((Atom) xevent->xclient.data.l[0] ==
	    XInternAtom (display, "_NET_WM_PING", FALSE)) {
		static long lastTimestamp = -1;
		long thisTimestamp = xevent->xclient.data.l[1];
		/* allow only events we have not processed yet */
		if (lastTimestamp == thisTimestamp) {
			return GDK_FILTER_REMOVE;
		}
		lastTimestamp = thisTimestamp;
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
	GtkNotebook *notebook =
	    GTK_NOTEBOOK (GTK_BIN (sia->applet)->child);

	int i;
	/* Do not remove the first page! It is the default page */
	for (i = gtk_notebook_get_n_pages (notebook); --i > 0;) {
		gtk_notebook_remove_page (notebook, i);
	}
}

static GtkWidget *
GSwitchItAppletPrepareDrawing (GSwitchItApplet * sia, int group)
{
	GdkPixbuf *image = sia->appletConfig.images[group];
	GdkPixbuf *scaled;
	PanelAppletOrient orient;
	int psize, xsize = 0, ysize = 0;
	double xyratio;
	sia->ebox = gtk_event_box_new ();
	if (sia->appletConfig.showFlags) {
		GtkWidget *flagImg;
		if (image == NULL)
			return NULL;
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
		//sia->ebox = NULL;	// not used in this case!
	} else {
		char *layoutName;
		char *allocLayoutName = NULL;
		XklConfigItem configItem;
		GtkWidget *align, *label;

		if (XklMultipleLayoutsSupported ()) {
			char *fullLayoutName =
			    (char *) g_slist_nth_data (sia->xkbConfig.
						       layouts, group);
			char *variantName;
			if (!GSwitchItConfigSplitItems
			    (fullLayoutName, &layoutName, &variantName))
				/* just in case */
				layoutName = fullLayoutName;

			g_snprintf (configItem.name,
				    sizeof (configItem.name), "%s",
				    layoutName);

			if (XklConfigFindLayout (&configItem)) {
				char *sd = configItem.shortDescription;
				if (sd != NULL && *sd != '\0') {
					layoutName = allocLayoutName =
					    g_locale_to_utf8 (sd, -1, NULL,
							      NULL, NULL);
				}
			}
		} else
			layoutName = sia->groupNames[group];
		align = gtk_alignment_new (0.5, 0.5, 1.0, 1.0);
		label = gtk_label_new (layoutName);
		if (allocLayoutName != NULL)
			g_free (allocLayoutName);
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
	GtkNotebook *notebook;
	int grp;
	int totalGroups = XklGetNumGroups ();
	notebook = GTK_NOTEBOOK (GTK_BIN (sia->applet)->child);

	for (grp = 0; grp < totalGroups; grp++) {
		GtkWidget *page, *decoratedPage;
		XklDebug (150,
			  "The widget for the group %d is not ready, let's prepare it!\n",
			  grp);
		page = GSwitchItAppletPrepareDrawing (sia, grp);

		decoratedPage =
		    GSwitchItPluginManagerDecorateWidget (&sia->
							  pluginManager,
							  page, grp,
							  sia->
							  groupNames[grp],
							  &sia->xkbConfig);

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
	GtkNotebook *notebook;
	XklDebug (200, "Revalidating for group %d\n", group);
	notebook = GTK_NOTEBOOK (GTK_BIN (sia->applet)->child);

	gtk_notebook_set_current_page (notebook, group + 1);

	pname = sia->groupNames[group];
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
GSwitchItAppletChangeBackground (PanelApplet *
				 widget, PanelAppletBackgroundType type,
				 GdkColor * color, GdkPixmap * pixmap,
				 GSwitchItApplet * sia)
{
	GtkRcStyle *rc_style = gtk_rc_style_new ();

	switch (type) {
	case PANEL_PIXMAP_BACKGROUND:
		if (sia->ebox != NULL)
			gtk_widget_modify_style (GTK_WIDGET (sia->ebox),
						 rc_style);
		gtk_widget_modify_style (GTK_WIDGET (sia->applet),
					 rc_style);
		break;

	case PANEL_COLOR_BACKGROUND:
		if (sia->ebox != NULL)
			gtk_widget_modify_bg (GTK_WIDGET (sia->ebox),
					      GTK_STATE_NORMAL, color);
		gtk_widget_modify_bg (GTK_WIDGET (sia->applet),
				      GTK_STATE_NORMAL, color);
		break;

	case PANEL_NO_BACKGROUND:
		if (sia->ebox != NULL)
			gtk_widget_modify_style (GTK_WIDGET (sia->ebox),
						 rc_style);
		gtk_widget_modify_style (GTK_WIDGET (sia->applet),
					 rc_style);
		break;

	default:
		if (sia->ebox != NULL)
			gtk_widget_modify_style (GTK_WIDGET (sia->ebox),
						 rc_style);
		gtk_widget_modify_style (GTK_WIDGET (sia->applet),
					 rc_style);
		break;
	}

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
		GSwitchItAppletConfigLockNextGroup ();
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
		GSwitchItAppletConfigLockNextGroup ();
		return TRUE;
	}
	return FALSE;
}

void
GSwitchItAppletCmdProps (BonoboUIComponent *
			 uic, GSwitchItApplet * sia, const gchar * verb)
{
	/* Only one preferences window at a time */
	if (sia->propsDialog) {
		gtk_window_set_screen (GTK_WINDOW (sia->propsDialog),
				       gtk_widget_get_screen (GTK_WIDGET
							      (sia->
							       applet)));

		gtk_window_present (GTK_WINDOW (sia->propsDialog));
		return;
	}

	GSwitchItAppletPropsCreate (sia);
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
	GSwitchItHelp (GTK_WIDGET (sia->applet), "gswitchit-applet-intro");
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
	gchar *translatorCredits;
	GdkPixbuf *pixbuf = NULL;
	gchar *file;
	if (sia->aboutDialog) {
		gtk_window_set_screen (GTK_WINDOW (sia->aboutDialog),
				       gtk_widget_get_screen (GTK_WIDGET
							      (sia->
							       applet)));

		gtk_window_present (GTK_WINDOW (sia->aboutDialog));
		return;
	}

	file =
	    gnome_program_locate_file (NULL,
				       GNOME_FILE_DOMAIN_PIXMAP,
				       "gswitchit-applet.png", TRUE, NULL);
	pixbuf = gdk_pixbuf_new_from_file (file, NULL);
	translatorCredits = _("translator_credits");
	translatorCredits =
	    strcmp (translatorCredits,
		    "translator_credits") != 0 ? translatorCredits : NULL;
	sia->aboutDialog =
	    gnome_about_new (_("Keyboard Indicator"), VERSION,
/* Translators: Please replace (C) with the proper copyright character. */
			     _
			     ("Copyright (c) Sergey V. Udaltsov 1999-2004"),
			     _
			     ("Keyboard layout indicator applet for GNOME"),
			     authors, documenters, translatorCredits,
			     pixbuf);
	g_object_add_weak_pointer (G_OBJECT (sia->aboutDialog),
				   (gpointer) & (sia->aboutDialog));
	gnome_window_icon_set_from_file (GTK_WINDOW (sia->aboutDialog),
					 file);
	g_free (file);

	gtk_window_set_screen (GTK_WINDOW (sia->aboutDialog),
			       gtk_widget_get_screen (GTK_WIDGET
						      (sia->applet)));

	gtk_window_present (GTK_WINDOW (sia->aboutDialog));
}

static void
GSwitchItAppletCleanupGroupsSubmenu (GSwitchItApplet * sia)
{
	int i;
	BonoboUIComponent *popup;
	popup =
	    panel_applet_get_popup_component (PANEL_APPLET (sia->applet));
	for (i = XkbNumKbdGroups; --i >= 0;) {
		char path[80];
		g_snprintf (path, sizeof (path),
			    GROUPS_SUBMENU_PATH "/Group_%d", i);
		if (bonobo_ui_component_path_exists (popup, path, NULL)) {
			bonobo_ui_component_rm (popup, path, NULL);
			XklDebug (160,
				  "Unregistered group menu item \'%s\'\n",
				  path);
		}
	}

	XklDebug (160, "Unregistered group submenu\n");
}

static void
GSwitchItAppletSetupGroupsSubmenu (GSwitchItApplet * sia)
{
	unsigned i, nGroups;
	char *pname;
	BonoboUIComponent *popup;
	GSList *layout;
	popup =
	    panel_applet_get_popup_component (PANEL_APPLET (sia->applet));
	XklDebug (160, "Registered group submenu\n");
	nGroups = XklGetNumGroups ();
	pname = (char *) sia->groupNames;
	layout = sia->xkbConfig.layouts;
	for (i = 0; i < nGroups; i++) {
		char verb[40];
		BonoboUINode *node;
		g_snprintf (verb, sizeof (verb), "Group_%d", i);
		node = bonobo_ui_node_new ("menuitem");
		bonobo_ui_node_set_attr (node, "name", verb);
		bonobo_ui_node_set_attr (node, "verb", verb);
		bonobo_ui_node_set_attr (node, "label", pname);
		bonobo_ui_node_set_attr (node, "pixtype", "filename");
		if (sia->appletConfig.showFlags) {
			const char *imageFile =
			    GSwitchItAppletConfigGetImagesFile (&sia->
								appletConfig,
								&sia->
								xkbConfig,
								i);
			if (imageFile != NULL)
				bonobo_ui_node_set_attr (node, "pixname",
							 imageFile);
		}
		bonobo_ui_component_set_tree (popup, GROUPS_SUBMENU_PATH,
					      node, NULL);
		bonobo_ui_component_add_verb (popup, verb, (BonoboUIVerbFn)
					      GSwitchItAppletCmdSetGroup,
					      sia);
		XklDebug (160,
			  "Registered group menu item \'%s\' as \'%s\'\n",
			  verb, pname);
		pname += sizeof (sia->groupNames[0]);
		layout = g_slist_next (layout);
	}
}

static void
GSwitchItAppletSetupMenu (GSwitchItApplet * sia)
{
	panel_applet_setup_menu_from_file
	    (PANEL_APPLET (sia->applet), NULL,
	     "GNOME_GSwitchItApplet.xml", NULL, gswitchitAppletMenuVerbs,
	     sia);
	GSwitchItAppletSetupGroupsSubmenu (sia);
}

static void
GSwitchItAppletStartListen (GSwitchItApplet * sia)
{
	gdk_window_add_filter (NULL,
			       (GdkFilterFunc) GSwitchItAppletFilterXEvt,
			       sia);
	gdk_window_add_filter (gdk_get_default_root_window (),
			       (GdkFilterFunc) GSwitchItAppletFilterXEvt,
			       sia);
	XklStartListen ();
}

static void
GSwitchItAppletStopListen (GSwitchItApplet * sia)
{
	XklStopListen ();
/* !! no client message filter removal in gnome 2.2 */
	gdk_window_remove_filter (NULL, (GdkFilterFunc)
				  GSwitchItAppletFilterXEvt, sia);
	gdk_window_remove_filter
	    (gdk_get_default_root_window (),
	     (GdkFilterFunc) GSwitchItAppletFilterXEvt, sia);
}

static void GSwitchItAppletTerm (PanelApplet *
				 applet, GSwitchItApplet * sia);

static gboolean
GSwitchItAppletInit (GSwitchItApplet * sia, PanelApplet * applet)
{
	GtkWidget *defDrawingArea;
	GtkNotebook *notebook;
	GConfClient *confClient;

	glade_gnome_init ();

	sia->applet = GTK_WIDGET (applet);

	panel_applet_set_flags (applet, PANEL_APPLET_EXPAND_MINOR);

	notebook = GTK_NOTEBOOK (gtk_notebook_new ());
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
	/* GSwitchItInstallGlibLogAppender(  ); */
	if (XklInit (GDK_DISPLAY ()) != 0) {
		GSwitchItAppletSetTooltip (sia,
					   _("XKB initialization error"));
		return TRUE;
	}

	XklConfigInit ();
	if (!XklConfigLoadRegistry ()) {
		GSwitchItAppletSetTooltip (sia,
					   _
					   ("Error loading XKB configuration registry"));
		return TRUE;
	}

	XklRegisterWindowCallback ((XklWinCallback)
				   GSwitchItAppletWindowCallback,
				   (void *) sia);
	XklRegisterStateCallback ((XklStateCallback)
				  GSwitchItAppletStateCallback,
				  (void *) sia);
	XklRegisterConfigCallback ((XklConfigCallback)
				   GSwitchItAppletXkbConfigCallback,
				   (void *) sia);

	confClient = gconf_client_get_default ();
	GSwitchItPluginContainerInit (&sia->pluginContainer, confClient);
	g_object_unref (confClient);

	GSwitchItXkbConfigInit (&sia->xkbConfig, confClient);
	GSwitchItAppletConfigInit (&sia->appletConfig, confClient);
	GSwitchItPluginManagerInit (&sia->pluginManager);
	GSwitchItXkbConfigLoadCurrent (&sia->xkbConfig);
	GSwitchItAppletConfigLoad (&sia->appletConfig);
	GSwitchItAppletConfigUpdateImages (&sia->appletConfig,
					   &sia->xkbConfig);
	GSwitchItAppletConfigActivate (&sia->appletConfig);
	GSwitchItAppletConfigLoadGroupDescriptionsUtf8 (&sia->appletConfig,
							sia->groupNames);
	GSwitchItPluginManagerInitEnabledPlugins (&sia->pluginManager,
						  &sia->pluginContainer,
						  sia->appletConfig.
						  enabledPlugins);
	GSwitchItAppletConfigStartListen (&sia->appletConfig,
					  (GConfClientNotifyFunc)
					  GSwitchItAppletConfigChanged,
					  sia);
	GSwitchItAppletFillNotebook (sia);
	GSwitchItAppletRevalidate (sia);
	g_signal_connect (G_OBJECT (sia->applet), "change_size",
			  G_CALLBACK (GSwitchItAppletChangePixelSize),
			  sia);
	g_signal_connect (G_OBJECT (sia->applet), "change_background",
			  G_CALLBACK (GSwitchItAppletChangeBackground),
			  sia);

	GSwitchItAppletStartListen (sia);
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
	if (terminatedOnce) {
		XklDebug (0,
			  "Please do not call the termination method twice!!!\n");
		return;
	}
	terminatedOnce = TRUE;
	XklDebug (100, "Starting the applet shutdown process\n");
	GSwitchItAppletStopListen (sia);
	GSwitchItAppletConfigStopListen (&sia->appletConfig);
	XklRegisterStateCallback (NULL, NULL);
	XklRegisterConfigCallback (NULL, NULL);
	XklRegisterWindowCallback (NULL, NULL);
	GSwitchItPluginManagerTermInitializedPlugins (&sia->pluginManager);
	GSwitchItPluginManagerTerm (&sia->pluginManager);
	GSwitchItAppletConfigTerm (&sia->appletConfig);
	GSwitchItXkbConfigTerm (&sia->xkbConfig);
	GSwitchItPluginContainerTerm (&sia->pluginContainer);
	GSwitchItAppletCleanupNotebook (sia);

	XklConfigFreeRegistry ();
	XklConfigTerm ();
	XklTerm ();

	terminatedOnce = TRUE;
	g_free (theAppletInstance);
	theAppletInstance = NULL;
	XklDebug (100, "The applet successfully terminated\n");
}

gboolean
GSwitchItAppletNew (PanelApplet * applet)
{
	gboolean rv = TRUE;
#if 0
	GLogLevelFlags fatal_mask;
	fatal_mask = G_LOG_LEVEL_WARNING | G_LOG_LEVEL_CRITICAL;
	g_log_set_always_fatal (fatal_mask);
#endif
	if (theAppletInstance == NULL) {
		terminatedOnce = FALSE;
		theAppletInstance = g_new0 (GSwitchItApplet, 1);
		rv = GSwitchItAppletInit (theAppletInstance, applet);
		XklDebug (100, "The applet successfully started: %d\n",
			  rv);
	} else
		XklDebug (0,
			  "Please do not call the initialization method twice!!!\n");
	return rv;
}

/* Plugin support */
void
GSwitchItPluginContainerReinitUi (GSwitchItPluginContainer * pc)
{
	GSwitchItAppletReinitUi ((GSwitchItApplet *) pc);
}

void
GSwitchItPluginLoadLocalizedGroupNames (GSwitchItPluginContainer
					* pc,
					const GroupDescriptionsBuffer
					** outDescr)
{
	*outDescr =
	    (const GroupDescriptionsBuffer
	     *) (&(((GSwitchItApplet *) pc)->groupNames));
}
