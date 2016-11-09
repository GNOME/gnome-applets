/*
 * main.c
 * Copyright (C) Andrej Belcijan 2009 <andrej.{@}.gmail.dot.com>
 * 
 * main.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * main.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "windowtitle.h"

/* Prototypes */
//static void applet_change_background (PanelApplet *, PanelAppletBackgroundType, GdkColor *, GdkPixmap *);
static void applet_change_orient (PanelApplet *, PanelAppletOrient, WTApplet *);
static void active_workspace_changed (WnckScreen *, WnckWorkspace *, WTApplet *);
static void active_window_changed (WnckScreen *, WnckWindow *, WTApplet *);
static void active_window_state_changed (WnckWindow *, WnckWindowState, WnckWindowState, WTApplet *);
static void active_window_nameicon_changed (WnckWindow *, WTApplet *);
static void umaxed_window_state_changed (WnckWindow *, WnckWindowState, WnckWindowState, WTApplet *);
static void umaxed_window_nameicon_changed (WnckWindow *, WTApplet *);
static void viewports_changed (WnckScreen *, WTApplet *);
static void window_closed (WnckScreen *, WnckWindow *, WTApplet *);
static void window_opened (WnckScreen *, WnckWindow *, WTApplet *);
//static void about_cb (BonoboUIComponent *, WTApplet *);
static void about_cb (GtkAction *, WTApplet *);
static WnckWindow *getRootWindow (WnckScreen *);
static WnckWindow *getUpperMaximized (WTApplet *);
const gchar *getCheckBoxGConfKey (gushort);
//void properties_cb (BonoboUIComponent *, WTApplet *, const char *);
void properties_cb (GtkAction *, WTApplet *);
void setAlignment(WTApplet *, gfloat);
void placeWidgets (WTApplet *);
void reloadWidgets (WTApplet *);
void toggleHidden(WTApplet *);
void savePreferences(WTPreferences *, WTApplet *);
WTPreferences *loadPreferences(WTApplet *);
gchar *getButtonLayoutGConf(WTApplet *, gboolean);

G_DEFINE_TYPE (WTApplet, wt_applet, PANEL_TYPE_APPLET);

static GtkActionEntry windowtitle_menu_actions [] = {
        { "WTPreferences", GTK_STOCK_PROPERTIES, N_("_Properties"), NULL, NULL, G_CALLBACK (properties_cb) },
        { "WTAbout", GTK_STOCK_ABOUT, N_("_About"), NULL, NULL, G_CALLBACK (about_cb) }
};

static const gchar windowtitle_menu_items [] =
	"<menuitem name=\"Preferences\"	action=\"WTPreferences\" />"
	"<menuitem name=\"About\"		action=\"WTAbout\" />";

WTApplet* wt_applet_new (void) {
        return g_object_new (WT_TYPE_APPLET, NULL);
}

static void wt_applet_class_init (WTAppletClass *klass) {
	// Not required
}

static void wt_applet_init(WTApplet *wtapplet) {
	// Not required
}

/* The About dialog */
//static void about_cb (BonoboUIComponent *uic, WTApplet *wtapplet) {
static void about_cb (GtkAction *action, WTApplet *wtapplet) {
        static const gchar *authors [] = {
		"Andrej Belcijan <{andrejx}at{gmail.com}>",
		" ",
		"Also contributed:",
		"Niko Bellic <{yurik81}at{gmail.com}>",
		NULL
	};

	const gchar *artists[] = {
		"Nasser Alshammari <{designernasser}at{gmail.com}>",
		NULL
	};
	
	const gchar *documenters[] = {
        "Andrej Belcijan <{andrejx}at{gmail.com}>",
		NULL
	};

	GdkPixbuf *logo = gdk_pixbuf_new_from_file (PATH_LOGO, NULL);

	gtk_show_about_dialog (NULL,
		"version",	VERSION,
		"comments",	N_("Window title for your GNOME Panel."),
		"copyright",	"\xC2\xA9 2011 Andrej Belcijan",
		"authors",	authors,
	    "artists",	artists,	    
		"documenters",	documenters,
		"translator-credits",	("translator-credits"),
		"logo",		logo,
   	    "website",	"http://www.gnome-look.org/content/show.php?content=103732",
		"website-label", N_("Window Applets on Gnome-Look"),
		NULL);
}

/* Safely returns the bottom-most (root) window */
static WnckWindow *getRootWindow (WnckScreen *screen) {
	GList *winstack = wnck_screen_get_windows_stacked(screen);
	if (winstack)
		return winstack->data;
	else
		return NULL;
}

/* Returns the highest maximized window */
static WnckWindow *getUpperMaximized (WTApplet *wtapplet) {	
	if (!wtapplet->prefs->only_maximized)
		return wtapplet->activewindow;
	
	GList *windows = wnck_screen_get_windows_stacked(wtapplet->activescreen);
	WnckWindow *returnwindow = NULL;

	while (windows && windows->data) {
		if (wnck_window_is_maximized(windows->data)) {
			// if(wnck_window_is_on_workspace(windows->data, wtapplet->activeworkspace))
			if (!wnck_window_is_minimized(windows->data))
				if (wnck_window_is_in_viewport(windows->data, wtapplet->activeworkspace))
					returnwindow = windows->data;
		}
		windows = windows->next;
	}
	 
	// start tracking the new umaxed window
	if (wtapplet->umaxedwindow) {
		if (g_signal_handler_is_connected(G_OBJECT(wtapplet->umaxedwindow), wtapplet->umaxed_handler_state))
			g_signal_handler_disconnect(G_OBJECT(wtapplet->umaxedwindow), wtapplet->umaxed_handler_state);
		if (g_signal_handler_is_connected(G_OBJECT(wtapplet->umaxedwindow), wtapplet->umaxed_handler_name))
			g_signal_handler_disconnect(G_OBJECT(wtapplet->umaxedwindow), wtapplet->umaxed_handler_name);
		if (g_signal_handler_is_connected(G_OBJECT(wtapplet->umaxedwindow), wtapplet->umaxed_handler_icon))
			g_signal_handler_disconnect(G_OBJECT(wtapplet->umaxedwindow), wtapplet->umaxed_handler_icon); 
	}
	if (returnwindow) {
		// maxed window was found
		wtapplet->umaxed_handler_state = g_signal_connect(G_OBJECT(returnwindow),
		                                             "state-changed",
		                                             G_CALLBACK (umaxed_window_state_changed),
		                                                  wtapplet);
		wtapplet->umaxed_handler_name = g_signal_connect(G_OBJECT(returnwindow),
		                                             "name-changed",
		                                             G_CALLBACK (umaxed_window_nameicon_changed),
		                                             wtapplet);		
		wtapplet->umaxed_handler_icon = g_signal_connect(G_OBJECT(returnwindow),
		                                             "icon-changed",
		                                             G_CALLBACK (umaxed_window_nameicon_changed),
		                                             wtapplet);	
		//return returnwindow;
	} else {
		// maxed window was not found
		returnwindow = wtapplet->rootwindow; //return wtapplet->rootwindow;
	}
	return returnwindow;
	// WARNING: if this function returns NULL, applet won't detect clicks!
}

// Updates the images according to preferences and the window situation
// Warning! This function is called very often, so it should only do the most necessary things!
void updateTitle(WTApplet *wtapplet) {
	WnckWindow *controlledwindow;
	gchar *title_text, *title_color, *title_font;
	GdkPixbuf *icon_pixbuf;
	
	if (wtapplet->prefs->only_maximized) {
		controlledwindow = wtapplet->umaxedwindow;
	} else {
		controlledwindow = wtapplet->activewindow;
	}

	if (controlledwindow == NULL)
		return;
	
	if (controlledwindow == wtapplet->rootwindow) {
		// we're on desktop
		if (wtapplet->prefs->hide_on_unmaximized) {
			// hide everything
			icon_pixbuf = NULL;
			title_text = "";
		} else {
			// display "custom" icon/title (TODO: customization via preferences?)
			icon_pixbuf = gtk_widget_render_icon(GTK_WIDGET(wtapplet),GTK_STOCK_HOME,GTK_ICON_SIZE_MENU,NULL); // This has to be unrefed!
			title_text = ("Desktop");
		}
	} else {
		icon_pixbuf = wnck_window_get_icon(controlledwindow); // This only returns a pointer - it SHOULDN'T be unrefed!
		title_text = (gchar*)wnck_window_get_name(controlledwindow);
	}
	
	// TODO: we need the default font to somehow be the same in both modes
	if (wtapplet->prefs->custom_style) {
		// custom style
		if (controlledwindow == wtapplet->activewindow) {
			// window focused
			title_color = wtapplet->prefs->title_active_color;
			title_font = wtapplet->prefs->title_active_font;
		} else {
			// window unfocused
			title_color = wtapplet->prefs->title_inactive_color;
			title_font = wtapplet->prefs->title_inactive_font;	
		}
	} else {
		// automatic (non-custom) style
		if (controlledwindow == wtapplet->activewindow) {
			// window focused				
			title_color = wtapplet->panel_color_fg;
			title_font = "";
		} else {
			// window unfocused
			title_color = "#808080"; // inactive title color. best fits for any panel regardless of color
			title_font = "";
		}		
	}

	// Set tooltips
	if (wtapplet->prefs->show_tooltips) {
		gtk_widget_set_tooltip_text (GTK_WIDGET(wtapplet->icon), title_text);
		gtk_widget_set_tooltip_text (GTK_WIDGET(wtapplet->title), title_text);
	}

	title_text = g_markup_printf_escaped("<span font=\"%s\" color=\"%s\">%s</span>", title_font, title_color, title_text);
	// Apply markup to label widget
	gtk_label_set_markup(GTK_LABEL(wtapplet->title), title_text);
	g_free(title_text);

	if (icon_pixbuf == NULL) {
		gtk_image_clear(wtapplet->icon);
	} else {
		// We're updating window info (Careful! We've had pixbuf memory leaks here)
		GdkPixbuf *ipb1 = gdk_pixbuf_scale_simple(icon_pixbuf, ICON_WIDTH, ICON_HEIGHT, GDK_INTERP_BILINEAR);
		if (controlledwindow == wtapplet->rootwindow) g_object_unref(icon_pixbuf); //this is stupid beyond belief, thanks to the retarded GTK framework
		GdkPixbuf *ipb2 = gdk_pixbuf_rotate_simple(ipb1, wtapplet->angle);
		g_object_unref(ipb1);	// Unref ipb1 to get it cleared from memory (we still need ipb2)

		// Saturate icon when window is not focused
		if (controlledwindow != wtapplet->activewindow) 
			gdk_pixbuf_saturate_and_pixelate(ipb2, ipb2, 0, FALSE);
		
		// Apply pixbuf to icon widget
		gtk_image_set_from_pixbuf(wtapplet->icon, ipb2);
		g_object_unref(ipb2);   // Unref ipb2 to get it cleared from memory			
	}
}

/* Expand/unexpand applet according to preferences */
void toggleExpand(WTApplet *wtapplet) {
	if (wtapplet->prefs->expand_applet) {
		panel_applet_set_flags (wtapplet->applet, PANEL_APPLET_EXPAND_MINOR | PANEL_APPLET_EXPAND_MAJOR);
	} else {
		// We must have a handle due to bug https://bugzilla.gnome.org/show_bug.cgi?id=556355
		// panel_applet_set_flags (wtapplet->applet, PANEL_APPLET_EXPAND_MINOR | PANEL_APPLET_EXPAND_MAJOR | PANEL_APPLET_HAS_HANDLE);
		panel_applet_set_flags (wtapplet->applet, PANEL_APPLET_EXPAND_MINOR);
	}
	reloadWidgets(wtapplet);
	setAlignment(wtapplet, (gfloat)wtapplet->prefs->alignment);
}

/* Hide/unhide stuff according to preferences */
void toggleHidden (WTApplet *wtapplet) {
	if (wtapplet->prefs->hide_icon) {
		gtk_widget_hide (GTK_WIDGET(wtapplet->icon));
	} else {
		gtk_widget_show (GTK_WIDGET(wtapplet->icon));
	}
	
	if (wtapplet->prefs->hide_title) {
		gtk_widget_hide (GTK_WIDGET(wtapplet->title));
	} else {
		gtk_widget_show (GTK_WIDGET(wtapplet->title));
	}

	if (!gtk_widget_get_visible(GTK_WIDGET(wtapplet->eb_icon)))
		gtk_widget_show_all(GTK_WIDGET(wtapplet->eb_icon));
	if (!gtk_widget_get_visible(GTK_WIDGET(wtapplet->eb_title)))
		gtk_widget_show_all(GTK_WIDGET(wtapplet->eb_title));
	if (!gtk_widget_get_visible(GTK_WIDGET(wtapplet->box)))
		gtk_widget_show_all(GTK_WIDGET(wtapplet->box));
	if (!gtk_widget_get_visible(GTK_WIDGET(wtapplet->applet)))
		gtk_widget_show_all(GTK_WIDGET(wtapplet->applet));
}

/* Called when panel background is changed */
static void applet_change_background (PanelApplet		*applet,
				     				 cairo_pattern_t	*pattern,
									 WTApplet			*wtapplet)
{
	if (wtapplet->panel_color_fg)
		g_free(wtapplet->panel_color_fg);	

	// Look up the default text color in the theme, use a default if it's not defined
	// This way is deprecated
	GdkColor color;
	GtkStyle *style = gtk_widget_get_style(GTK_WIDGET(applet));
	if (!gtk_style_lookup_color(style, "dark_fg_color", &color))	// check if dark_fg_color is set otherwise...
		if (!gtk_style_lookup_color(style, "fg_color", &color))		// ... check if fg_color is set otherwise...
			gdk_color_parse("#808080", &color);						// ... set universally acceptable color #808080
	wtapplet->panel_color_fg = gdk_color_to_string(&color);

	/*
	GdkRGBA rgba;
	GtkStyleContext *stylecontext = gtk_widget_get_style_context(GTK_WIDGET(applet));
	if (!gtk_style_context_lookup_color(stylecontext, "dark_fg_color", &rgba))
		if (!gtk_style_context_lookup_color(stylecontext, "fg_color", &rgba))
			gdk_rgba_parse(&rgba, "#808080");
	wtapplet->panel_color_fg = gdk_rgba_to_string(&rgba); // This does not produce the right syntax (gtk_label_set_markup fails)
	*/

	updateTitle(wtapplet); // We need to redraw the title using the new colors
}

/* Triggered when a different panel orientation is detected */
static void applet_change_orient (PanelApplet *panelapplet,
                                  PanelAppletOrient orient,
                                  WTApplet *wtapplet)
{
	if (orient != wtapplet->orient) {
		wtapplet->orient = orient;

		reloadWidgets(wtapplet);
		updateTitle(wtapplet);
	}
}

/* (Supposedly) tiggered when panel size changes */
static void applet_change_pixel_size (PanelApplet	*applet,
									  gint			size,
									  WTApplet		*wtapplet)
{
	if (wtapplet->size == size)
		return;

	wtapplet->size = size;

	updateTitle(wtapplet);
}

/*
static void applet_title_size_request (GtkWidget *widget,
                                       GtkRequisition *requisition,
                                       gpointer user_data)
{
	WTApplet *wtapplet = WT_APPLET(user_data);

	if (wtapplet->prefs->expand_applet)
		return;

	gint size_min = MIN(requisition->width, wtapplet->asize);
	gint size_max = MAX(requisition->width, wtapplet->asize);

	//g_printf("New size: %d\n", new_size);
	
	wtapplet->size_hints[0] = size_min;
	wtapplet->size_hints[1] = wtapplet->asize - 1;
	panel_applet_set_size_hints (PANEL_APPLET(wtapplet), wtapplet->size_hints, 2, 0);	
	// This is the only way to go, but it cannot work because of gnome-panel bugs:
	// * https://bugzilla.gnome.org/show_bug.cgi?id=556355
	// * https://bugzilla.gnome.org/show_bug.cgi?id=557232

	//	GtkAllocation child_allocation;
	//	child_allocation.x = 0;
	//	child_allocation.y = 0;
	//	child_allocation.width = new_size  - (16+11);
	//	child_allocation.height = requisition->height;
	//	gtk_widget_size_allocate (GTK_WIDGET(wtapplet->title), &child_allocation);
	//	gtk_widget_set_child_visible (GTK_WIDGET(wtapplet->title), TRUE);
}
*/

/* Triggered when applet allocates new size */
static void applet_size_allocate (GtkWidget     *widget,
                                  GtkAllocation *allocation,
                                  WTApplet		*wtapplet)
{
	if (wtapplet->prefs->expand_applet) return;
	
	if (wtapplet->asize != allocation->width) {
		wtapplet->asize = allocation->width;
	}
}

/* Triggers when a new window has been opened */
// in case a new maximized non-active window appears
static void window_opened (WnckScreen *screen,
                           WnckWindow *window,
                           WTApplet *wtapplet) {

	wtapplet->umaxedwindow = getUpperMaximized(wtapplet);

	updateTitle(wtapplet);
}

/* Triggers when a window has been closed */
// in case the last maximized window was closed
static void window_closed (WnckScreen *screen,
                           WnckWindow *window,
                           WTApplet *wtapplet) {

	wtapplet->umaxedwindow = getUpperMaximized(wtapplet);

	updateTitle(wtapplet); // required when closing window in the background
}

/* Triggers when a new active window is selected */
static void active_window_changed (WnckScreen *screen,
                                   WnckWindow *previous,
                                   WTApplet *wtapplet)
{
	// Start tracking the new active window:
	if (wtapplet->activewindow) {
		if (g_signal_handler_is_connected(G_OBJECT(wtapplet->activewindow), wtapplet->active_handler_state))
			g_signal_handler_disconnect(G_OBJECT(wtapplet->activewindow), wtapplet->active_handler_state);
		if (g_signal_handler_is_connected(G_OBJECT(wtapplet->activewindow), wtapplet->active_handler_name))
			g_signal_handler_disconnect(G_OBJECT(wtapplet->activewindow), wtapplet->active_handler_name);
		if (g_signal_handler_is_connected(G_OBJECT(wtapplet->activewindow), wtapplet->active_handler_icon))
			g_signal_handler_disconnect(G_OBJECT(wtapplet->activewindow), wtapplet->active_handler_icon);
	}
	
	wtapplet->activewindow = wnck_screen_get_active_window(screen);
	wtapplet->umaxedwindow = getUpperMaximized(wtapplet); // returns wbapplet->activewindow if not only_maximized
	wtapplet->rootwindow = getRootWindow(wtapplet->activescreen);
	
	if (wtapplet->activewindow) {
		wtapplet->active_handler_state = g_signal_connect(G_OBJECT (wtapplet->activewindow), "state-changed", G_CALLBACK (active_window_state_changed), wtapplet);
		wtapplet->active_handler_name = g_signal_connect(G_OBJECT (wtapplet->activewindow), "name-changed", G_CALLBACK (active_window_nameicon_changed), wtapplet);
		wtapplet->active_handler_icon = g_signal_connect(G_OBJECT (wtapplet->activewindow), "icon-changed", G_CALLBACK (active_window_nameicon_changed), wtapplet);
										   
		wtapplet->focused = TRUE;

		updateTitle(wtapplet);
	}
}

/* Trigger when activewindow's state changes */
static void active_window_state_changed (WnckWindow *window,
                                         WnckWindowState changed_mask,
                                         WnckWindowState new_state,
                                         WTApplet *wtapplet) {

	wtapplet->umaxedwindow = getUpperMaximized(wtapplet);
  	wtapplet->rootwindow = getRootWindow(wtapplet->activescreen);

	if (new_state & (WNCK_WINDOW_STATE_MAXIMIZED_HORIZONTALLY | WNCK_WINDOW_STATE_MAXIMIZED_VERTICALLY)) {
		wtapplet->focused = TRUE;
	} else {
		if (wtapplet->prefs->only_maximized) {
				wtapplet->focused = FALSE;
		}
	}
	
	updateTitle(wtapplet);
}

/* Triggers when activewindow's name changes */
static void active_window_nameicon_changed (WnckWindow *window, WTApplet *wtapplet) {
	updateTitle(wtapplet);
}

/* Triggers when umaxedwindow's state changes */
static void umaxed_window_state_changed (WnckWindow *window,
                                          WnckWindowState changed_mask,
                                          WnckWindowState new_state,
                                          WTApplet *wtapplet)
{
	wtapplet->umaxedwindow = getUpperMaximized(wtapplet);
	wtapplet->rootwindow = getRootWindow(wtapplet->activescreen);
	
	updateTitle(wtapplet);
}

/* Triggers when umaxedwindow's name OR ICON changes */
static void umaxed_window_nameicon_changed(WnckWindow *window, WTApplet *wtapplet) {
	updateTitle(wtapplet);
}

/* Triggers when user changes viewports (Compiz) */
static void viewports_changed (WnckScreen *screen,
                               WTApplet *wtapplet)
{
	wtapplet->activeworkspace = wnck_screen_get_active_workspace(screen);
	wtapplet->activewindow = wnck_screen_get_active_window(screen);
	wtapplet->rootwindow = getRootWindow(wtapplet->activescreen); //?
	wtapplet->umaxedwindow = getUpperMaximized(wtapplet);

	// active_window_changed will do it too, but this one will be sooner
	updateTitle(wtapplet);
}

/* Triggers when.... ? not sure... (Metacity?) */
static void active_workspace_changed (WnckScreen *screen,
                                      WnckWorkspace *previous,
                                      WTApplet *wtapplet)
{
	wtapplet->activeworkspace = wnck_screen_get_active_workspace(screen);
	/*
	wtapplet->activewindow = wnck_screen_get_active_window(screen);
	// wtapplet->rootwindow = getRootWindow(wtapplet->activescreen); //?
	wtapplet->umaxedwindow = getUpperMaximized(wtapplet);

	updateTitle(wtapplet);
	*/
}

static gboolean icon_clicked (GtkWidget *icon,
                              GdkEventButton *event,
                              WTApplet *wtapplet)
{
	if (event->button != 1) return FALSE;
	
	WnckWindow *controlledwindow;

	if (wtapplet->prefs->only_maximized) {
		controlledwindow = wtapplet->umaxedwindow;
	} else {
		controlledwindow = wtapplet->activewindow;
	}

	// single click:
	if (controlledwindow) {
		wnck_window_activate(controlledwindow, gtk_get_current_event_time());
	}

	// double click:
	if (event->type == GDK_2BUTTON_PRESS) {
		wnck_window_close(controlledwindow, gtk_get_current_event_time());
	}		

	return TRUE;
}

static gboolean title_clicked (GtkWidget *title,
                               GdkEventButton *event,
                               WTApplet *wtapplet)
{
	// only allow left and right mouse button
	//if (event->button != 1 && event->button != 3) return FALSE;
	
	WnckWindow *controlledwindow;

	if (wtapplet->prefs->only_maximized) {
		controlledwindow = wtapplet->umaxedwindow;
	} else {
		controlledwindow = wtapplet->activewindow;
	}

	if (!controlledwindow) 
		return FALSE;
	
	// single click (left/right)
	if (event->button == 1) {
		// left-click
		wnck_window_activate(controlledwindow, gtk_get_current_event_time());
		if (event->type==GDK_2BUTTON_PRESS || event->type==GDK_3BUTTON_PRESS) {
			// double/tripple click
			//if (event->type==GDK_2BUTTON_PRESS) {
			if (wnck_window_is_maximized(controlledwindow)) {
				wnck_window_unmaximize(controlledwindow);
			} else {
				wnck_window_maximize(controlledwindow);
			}
		}
	} else if (event->button == 3) {
		// right-click
		if (wtapplet->prefs->show_window_menu) {
			wnck_window_activate(controlledwindow, gtk_get_current_event_time());
			GtkMenu *window_menu = GTK_MENU(wnck_action_menu_new(controlledwindow));
			gtk_menu_popup(window_menu, NULL, NULL, NULL, NULL, event->button, gtk_get_current_event_time());
			//TODO: somehow alter the panel action menu to also display the wnck_action_menu !
		} else {
			return FALSE;
		}
	} else {
		return FALSE;
	}
	return TRUE;
}

/* Places widgets in box accordingly with angle and order */
void placeWidgets (WTApplet *wtapplet) {

	// TODO: merge all this... or not?
	if (wtapplet->orient == PANEL_APPLET_ORIENT_RIGHT) {
		wtapplet->angle = GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE;
	} else if (wtapplet->orient == PANEL_APPLET_ORIENT_LEFT) {
		wtapplet->angle = GDK_PIXBUF_ROTATE_CLOCKWISE;
	} else {
		wtapplet->angle = GDK_PIXBUF_ROTATE_NONE;
	}

	if (wtapplet->angle == GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE) {
		wtapplet->packtype = GTK_PACK_END;
	} else {
		wtapplet->packtype = GTK_PACK_START;
	}
	
	// set box orientation
	if (wtapplet->angle == GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE || wtapplet->angle == GDK_PIXBUF_ROTATE_CLOCKWISE) {
		gtk_orientable_set_orientation(GTK_ORIENTABLE(wtapplet->box), GTK_ORIENTATION_VERTICAL);
	} else {
		gtk_orientable_set_orientation(GTK_ORIENTABLE(wtapplet->box), GTK_ORIENTATION_HORIZONTAL);
	}

	// set packing order
	if (wtapplet->prefs->swap_order == wtapplet->packtype) {
		gtk_box_pack_start(wtapplet->box, GTK_WIDGET(wtapplet->eb_icon), FALSE, TRUE, 0);
		gtk_box_pack_start(wtapplet->box, GTK_WIDGET(wtapplet->eb_title), TRUE, TRUE, 0);
	} else {
		gtk_box_pack_start(wtapplet->box, GTK_WIDGET(wtapplet->eb_title), TRUE, TRUE, 0);
		gtk_box_pack_start(wtapplet->box, GTK_WIDGET(wtapplet->eb_icon), FALSE, TRUE, 0);
	}

	// Set alignment/orientation
	gtk_label_set_angle( wtapplet->title, wtapplet->angle );
	setAlignment(wtapplet, (gfloat)wtapplet->prefs->alignment);
}

/* Reloads all widgets */
void reloadWidgets (WTApplet *wtapplet) {
	g_object_ref(wtapplet->eb_icon);
	g_object_ref(wtapplet->eb_title);
	gtk_container_remove(GTK_CONTAINER(wtapplet->box), GTK_WIDGET(wtapplet->eb_icon));
	gtk_container_remove(GTK_CONTAINER(wtapplet->box), GTK_WIDGET(wtapplet->eb_title));
	placeWidgets(wtapplet);
	g_object_unref(wtapplet->eb_icon);
	g_object_unref(wtapplet->eb_title);
}

/* Sets alignment, min size, padding to title according to panel orientation */
void setAlignment (WTApplet *wtapplet, gfloat alignment) {
	if (!wtapplet->prefs->expand_applet)
		alignment = 0.0;
	
	if (wtapplet->angle == GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE || wtapplet->angle == GDK_PIXBUF_ROTATE_CLOCKWISE) {
		// Alignment is vertical
		if (wtapplet->angle == GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE) {
			gtk_misc_set_alignment(GTK_MISC(wtapplet->title), 0.5, 1-alignment);
		} else {
			gtk_misc_set_alignment(GTK_MISC(wtapplet->title), 0.5, alignment);
		}
		gtk_widget_set_size_request(GTK_WIDGET(wtapplet->title), -1, wtapplet->prefs->title_size);
		gtk_misc_set_padding(GTK_MISC(wtapplet->icon), 0, ICON_PADDING);
	} else {
		// Alignment is horizontal
		gtk_misc_set_alignment(GTK_MISC(wtapplet->title), alignment, 0.5);
		gtk_widget_set_size_request(GTK_WIDGET(wtapplet->title), wtapplet->prefs->title_size, -1);
		gtk_misc_set_padding(GTK_MISC(wtapplet->icon), ICON_PADDING, 0);
	}
}

/* Do the actual applet initialization */
static void init_wtapplet (PanelApplet *applet) {
	WTApplet *wtapplet = g_new0 (WTApplet, 1);

	wtapplet->applet = applet;
	wtapplet->prefs = loadPreferences(wtapplet);
	wtapplet->activescreen = wnck_screen_get_default();
	wnck_screen_force_update(wtapplet->activescreen);
	wtapplet->activeworkspace = wnck_screen_get_active_workspace(wtapplet->activescreen);
	wtapplet->activewindow = wnck_screen_get_active_window(wtapplet->activescreen);
	wtapplet->umaxedwindow = getUpperMaximized(wtapplet);
	wtapplet->rootwindow = getRootWindow(wtapplet->activescreen);
	wtapplet->prefbuilder = gtk_builder_new();
	wtapplet->box = GTK_BOX(gtk_hbox_new(FALSE, 0));
	wtapplet->icon = GTK_IMAGE(gtk_image_new());
	wtapplet->title = GTK_LABEL(gtk_label_new(NULL));
	wtapplet->eb_icon = GTK_EVENT_BOX(gtk_event_box_new());
	wtapplet->eb_title = GTK_EVENT_BOX(gtk_event_box_new());
	wtapplet->orient = panel_applet_get_orient(wtapplet->applet);
	wtapplet->size_hints = g_new(gint,2);

	// Widgets to eventboxes, eventboxes to box
	gtk_widget_set_can_focus(GTK_WIDGET(wtapplet->icon), TRUE);
	gtk_widget_set_can_focus(GTK_WIDGET(wtapplet->title), TRUE);
	gtk_container_add (GTK_CONTAINER (wtapplet->eb_icon), GTK_WIDGET(wtapplet->icon));
	gtk_container_add (GTK_CONTAINER (wtapplet->eb_title), GTK_WIDGET(wtapplet->title));
	gtk_event_box_set_visible_window (wtapplet->eb_icon, FALSE);
	gtk_event_box_set_visible_window (wtapplet->eb_title, FALSE);

	// Rotate & place elements
	setAlignment(wtapplet, (gfloat)wtapplet->prefs->alignment);
	placeWidgets(wtapplet);

	// Add box to applet
	gtk_container_add (GTK_CONTAINER(wtapplet->applet), GTK_WIDGET(wtapplet->box));

	// Set event handling (icon & title clicks)
	g_signal_connect(G_OBJECT (wtapplet->eb_icon), "button-press-event", G_CALLBACK (icon_clicked), wtapplet);
	g_signal_connect(G_OBJECT (wtapplet->eb_title), "button-press-event", G_CALLBACK (title_clicked), wtapplet);
	
	// Global window tracking
	g_signal_connect(wtapplet->activescreen, "active-window-changed", G_CALLBACK (active_window_changed), wtapplet); // <-- this thing is crashing with compiz !!!
	g_signal_connect(wtapplet->activescreen, "viewports-changed", G_CALLBACK (viewports_changed), wtapplet);
	g_signal_connect(wtapplet->activescreen, "active-workspace-changed", G_CALLBACK (active_workspace_changed), wtapplet);
	g_signal_connect(wtapplet->activescreen, "window-closed", G_CALLBACK (window_closed), wtapplet);
	g_signal_connect(wtapplet->activescreen, "window-opened", G_CALLBACK (window_opened), wtapplet);

	// g_signal_connect(G_OBJECT (wtapplet->title), "size-request", G_CALLBACK (applet_title_size_request), wtapplet);
	g_signal_connect(G_OBJECT (wtapplet->applet), "size-allocate", G_CALLBACK (applet_size_allocate), wtapplet);

	g_signal_connect(G_OBJECT (wtapplet->applet), "change-background", G_CALLBACK (applet_change_background), wtapplet);
	g_signal_connect(G_OBJECT (wtapplet->applet), "change-orient", G_CALLBACK (applet_change_orient), wtapplet);
	g_signal_connect(G_OBJECT (wtapplet->applet), "change-size", G_CALLBACK (applet_change_pixel_size), wtapplet);
	
	// Track active window changes
	wtapplet->active_handler_state = 
		g_signal_connect(G_OBJECT (wtapplet->activewindow), "state-changed", G_CALLBACK (active_window_state_changed), wtapplet);
	wtapplet->active_handler_name = 
		g_signal_connect(G_OBJECT (wtapplet->activewindow), "name-changed", G_CALLBACK (active_window_nameicon_changed), wtapplet);
	wtapplet->active_handler_icon = 
		g_signal_connect(G_OBJECT (wtapplet->activewindow), "icon-changed", G_CALLBACK (active_window_nameicon_changed), wtapplet);

	
	// Setup applet right-click menu
	GtkActionGroup *action_group = gtk_action_group_new ("WindowButtons Applet Actions");
	gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);
	gtk_action_group_add_actions (action_group, windowtitle_menu_actions, G_N_ELEMENTS (windowtitle_menu_actions), wtapplet);
	panel_applet_setup_menu (applet, windowtitle_menu_items, action_group);
	panel_applet_set_background_widget (wtapplet->applet, GTK_WIDGET (wtapplet->applet)); // Automatic background update
	
	toggleExpand  (wtapplet);
	toggleHidden  (wtapplet);	// Properly hide or show stuff
	updateTitle   (wtapplet);
}

// Initial function that draws the applet
static gboolean windowtitle_applet_factory (PanelApplet *applet, const gchar *iid, gpointer data) {
	if (strcmp (iid, APPLET_OAFIID) != 0) return FALSE;
	
	g_set_application_name (APPLET_NAME); //GLib-WARNING **: g_set_application_name() called multiple times
	panel_applet_add_preferences (applet, GCONF_PREFS, NULL);
	wnck_set_client_type (WNCK_CLIENT_TYPE_PAGER);

	init_wtapplet (applet);

	return TRUE;
}

PANEL_APPLET_OUT_PROCESS_FACTORY (APPLET_OAFIID_FACTORY,
                                  PANEL_TYPE_APPLET,
                                  (PanelAppletFactoryCallback) windowtitle_applet_factory,
                                  NULL)
