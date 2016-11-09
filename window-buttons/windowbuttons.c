/***************************************************************************
 *            windowbuttons.c
 *
 *  Mon May  4 01:21:56 2009
 *  Copyright  2009  Andrej Belcijan
 *  <{andrejx} at {gmail.com}>
 ****************************************************************************/

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */
 
#include "windowbuttons.h"

/* Prototypes */
//static void applet_change_background (PanelApplet *, PanelAppletBackgroundType, GdkColor *, GdkPixmap *);
static void active_workspace_changed (WnckScreen *, WnckWorkspace *, WBApplet *);
static void active_window_changed (WnckScreen *, WnckWindow *, WBApplet *);
static void active_window_state_changed (WnckWindow *, WnckWindowState, WnckWindowState, WBApplet *);
static void umaxed_window_state_changed (WnckWindow *, WnckWindowState, WnckWindowState, WBApplet *);
static void viewports_changed (WnckScreen *, WBApplet *);
static void window_closed (WnckScreen *, WnckWindow *, WBApplet *);
static void window_opened (WnckScreen *, WnckWindow *, WBApplet *);
//static void about_cb (BonoboUIComponent *, WBApplet *);
static void about_cb (GtkAction *, WBApplet *);
//void properties_cb (BonoboUIComponent *, WBApplet *, const char *);
void properties_cb (GtkAction *, WBApplet *);
static gboolean hover_enter (GtkWidget *, GdkEventCrossing *, WBApplet *);
static gboolean hover_leave (GtkWidget *, GdkEventCrossing *, WBApplet *);
static gboolean button_press (GtkWidget *, GdkEventButton *, WBApplet *);
static gboolean button_release (GtkWidget *, GdkEventButton *, WBApplet *);
static WnckWindow *getRootWindow (WnckScreen *screen);
static WnckWindow *getUpperMaximized (WBApplet *);
void setCustomLayout (WBPreferences *, gchar *);
void rotatePixbufs(WBApplet *);
void placeButtons(WBApplet *);
void reloadButtons(WBApplet *);
void toggleHidden(WBApplet *);
void savePreferences(WBPreferences *, WBApplet *);
void loadThemes(GtkComboBox *, gchar *);
WBPreferences *loadPreferences(WBApplet *);
gchar *getButtonLayoutGConf(WBApplet *, gboolean);
gchar *getMetacityLayout(void);
const gchar *getCheckBoxCfgKey (gushort);
GdkPixbuf ***getPixbufs(gchar ***);

//G_DEFINE_TYPE(TN, t_n, T_P) G_DEFINE_TYPE_EXTENDED (TN, t_n, T_P, 0, {})
//This line is very important! It defines how the requested functions will be called
G_DEFINE_TYPE (WBApplet, wb_applet, PANEL_TYPE_APPLET);

static const gchar windowbuttons_menu_items [] =
	"<menuitem name=\"Preferences\"	action=\"WBPreferences\" />"
	"<menuitem name=\"About\"		action=\"WBAbout\" />";

static const GtkActionEntry windowbuttons_menu_actions [] = {
        { "WBPreferences", GTK_STOCK_PROPERTIES, N_("_Preferences"), NULL, NULL, G_CALLBACK (properties_cb) },
        { "WBAbout", GTK_STOCK_ABOUT, N_("_About"), NULL, NULL, G_CALLBACK (about_cb) }
};

WBApplet* wb_applet_new (void) {
        return g_object_new (WB_TYPE_APPLET, NULL);
}

static void wb_applet_class_init (WBAppletClass *klass) {
	// Called before wb_applet_init()
}

static void wb_applet_init(WBApplet *wbapplet) {
	// Called before windowbuttons_applet_fill(), after wb_applet_class_init()
}

/* The About dialog */
//static void about_cb (BonoboUIComponent *uic, WBApplet *applet) {
static void about_cb (GtkAction *action, WBApplet *applet) {
    static const gchar *authors [] = {
		"Andrej Belcijan <{andrejx}at{gmail.com}>",
		" ",
		"Also contributed:",
		"quiescens",
		NULL
	};

	const gchar *artists[] = {
		"Andrej Belcijan <{andrejx}at{gmail.com}> for theme \"Default\" and contribution moderating",
		"Maurizio De Santis <{desantis.maurizio}at{gmail.com}> for theme \"Blubuntu\"",
		"Nasser Alshammari <{designernasser}at{gmail.com}> for logo design",
		"Jeff M. Hubbard <{jeffmhubbard}at{gmail.com}> for theme \"Elementary\"",
		"Gaurang Arora for theme \"Dust-Invert\"",
		"Giacomo Porrà for themes \"Ambiance\", \"Ambiance-Maverick\", \"Radiance\", \"Radiance-Maverick\"",
		"Vitalii Dmytruk for theme \"Ambiance-X-Studio\"",
		"amoob for themes \"Plano\", \"New-Hope\" and \"WoW\"",
		"McBurri for theme \"Equinox-Glass\"",
		"milky2313 for theme \"Sorbet\"",
		NULL
	};
	
	const gchar *documenters[] = {
        "Andrej Belcijan <{andrejx}at{gmail.com}>",
		NULL
	};

	GdkPixbuf *logo = gdk_pixbuf_new_from_file (PATH_LOGO, NULL);

	gtk_show_about_dialog (NULL,
		"version",	VERSION,
		"comments",	N_("Window buttons for your GNOME Panel."),
		"copyright",	"\xC2\xA9 2011 Andrej Belcijan",
		"authors",	authors,
	    "artists",	artists,
		"documenters",	documenters,
		"translator-credits", ("translator-credits"),
		"logo",		logo,
	    "website",	"http://www.gnome-look.org/content/show.php?content=103732",
   		"website-label", N_("Window Applets on Gnome-Look"),
		NULL);
}

static WnckWindow *getRootWindow (WnckScreen *screen) {
	GList *winstack = wnck_screen_get_windows_stacked(screen);
	if (winstack) // we can't access data directly because sometimes we will get NULL
		return winstack->data;
	else
		return NULL;
}

/* Returns the highest maximized window */
static WnckWindow *getUpperMaximized (WBApplet *wbapplet) {
	if (!wbapplet->prefs->only_maximized)
		return wbapplet->activewindow;
	
	GList *windows = wnck_screen_get_windows_stacked(wbapplet->activescreen);
	WnckWindow *returnwindow = NULL;

	while (windows && windows->data) {
		if (wnck_window_is_maximized(windows->data)) {
			//if (wnck_window_is_on_workspace(windows->data, wbapplet->activeworkspace))
			if (!wnck_window_is_minimized(windows->data))
				if (wnck_window_is_in_viewport(windows->data, wbapplet->activeworkspace))
					returnwindow = windows->data;
		}
		windows = windows->next;
	}

	// start tracking the new umaxed window
	if (wbapplet->umaxedwindow) {
		if (g_signal_handler_is_connected(G_OBJECT(wbapplet->umaxedwindow), wbapplet->umaxed_handler))
			g_signal_handler_disconnect(G_OBJECT(wbapplet->umaxedwindow), wbapplet->umaxed_handler);
	}
	if (returnwindow) {
		wbapplet->umaxed_handler = g_signal_connect(G_OBJECT(returnwindow),
		                                             "state-changed",
		                                             G_CALLBACK (umaxed_window_state_changed),
		                                             wbapplet);
	} else {
		returnwindow = wbapplet->rootwindow;
	}

	return returnwindow;
}

/* Return image ID according to button state */
gushort getImageState (WBButtonState button_state) {
	if (button_state & WB_BUTTON_STATE_FOCUSED) {
		if (button_state & WB_BUTTON_STATE_CLICKED) {
			return WB_IMAGE_FOCUSED_CLICKED;
		} else if (button_state & WB_BUTTON_STATE_HOVERED) {
			return WB_IMAGE_FOCUSED_HOVERED;
		} else {
			return WB_IMAGE_FOCUSED_NORMAL;
		}
	} else {
		if (button_state & WB_BUTTON_STATE_CLICKED) {
			return WB_IMAGE_UNFOCUSED_CLICKED;
		} else if (button_state & WB_BUTTON_STATE_HOVERED) {
			return WB_IMAGE_UNFOCUSED_HOVERED;
		} else {
			return WB_IMAGE_UNFOCUSED_NORMAL;
		}
	}
}

/* Updates the images according to preferences and the umaxed window situation */
void updateImages (WBApplet *wbapplet) {
	WnckWindow *controlledwindow;
	gint i;

	if (wbapplet->prefs->only_maximized) {
		controlledwindow = wbapplet->umaxedwindow;
	} else {
		controlledwindow = wbapplet->activewindow;
	}

	if (controlledwindow == wbapplet->rootwindow) {
		// There are no maximized windows (or any windows at all) left
		for (i=0; i<WB_BUTTONS; i++)
			wbapplet->button[i]->state &= ~WB_BUTTON_STATE_FOCUSED;

		// Hide/unhide all buttons according to hide_on_unmaximized and button_hidden
		for (i=0; i<WB_BUTTONS; i++) {
			if (wbapplet->prefs->hide_on_unmaximized || wbapplet->prefs->button_hidden[i]) {
				// hide if we want them hidden or it is hidden anyway
				wbapplet->button[i]->state |= WB_BUTTON_STATE_HIDDEN;
			} else {
				if (!wbapplet->prefs->button_hidden[i]) {
					// unhide only if it is not explicitly hidden
					wbapplet->button[i]->state &= ~WB_BUTTON_STATE_HIDDEN;
				}
			}
		}
	} else {
		for (i=0; i<WB_BUTTONS; i++) { // update states
			if (wbapplet->prefs->button_hidden[i]) {
				wbapplet->button[i]->state |= WB_BUTTON_STATE_HIDDEN;
			} else {
				wbapplet->button[i]->state &= ~WB_BUTTON_STATE_HIDDEN;
			}
		}
	}

	toggleHidden(wbapplet);
	
	//update minimize button:
	gtk_image_set_from_pixbuf (wbapplet->button[WB_BUTTON_MINIMIZE]->image, wbapplet->pixbufs[getImageState(wbapplet->button[WB_BUTTON_MINIMIZE]->state)][WB_IMAGE_MINIMIZE]);
	// update maximize/unmaximize button:
	if (controlledwindow && wnck_window_is_maximized(controlledwindow)) {
		gtk_image_set_from_pixbuf (wbapplet->button[WB_BUTTON_UMAXIMIZE]->image, wbapplet->pixbufs[getImageState(wbapplet->button[WB_BUTTON_UMAXIMIZE]->state)][WB_IMAGE_UNMAXIMIZE]);
		if (wbapplet->prefs->show_tooltips)
			gtk_widget_set_tooltip_text (GTK_WIDGET(wbapplet->button[WB_BUTTON_UMAXIMIZE]->image), N_("Unmaximize")); // Set correct tooltip
	} else {
		gtk_image_set_from_pixbuf (wbapplet->button[WB_BUTTON_UMAXIMIZE]->image, wbapplet->pixbufs[getImageState(wbapplet->button[WB_BUTTON_UMAXIMIZE]->state)][WB_IMAGE_MAXIMIZE]);
		if (wbapplet->prefs->show_tooltips)
			gtk_widget_set_tooltip_text (GTK_WIDGET(wbapplet->button[WB_BUTTON_UMAXIMIZE]->image), N_("Maximize")); // Set correct tooltip
	}
	//update close button
	gtk_image_set_from_pixbuf (wbapplet->button[WB_BUTTON_CLOSE]->image, wbapplet->pixbufs[getImageState(wbapplet->button[WB_BUTTON_CLOSE]->state)][WB_IMAGE_CLOSE]);

	//set remaining tooltips
	if (wbapplet->prefs->show_tooltips) {
		gtk_widget_set_tooltip_text (GTK_WIDGET(wbapplet->button[WB_BUTTON_MINIMIZE]->image), N_("Minimize"));
		gtk_widget_set_tooltip_text (GTK_WIDGET(wbapplet->button[WB_BUTTON_CLOSE]->image), N_("Close"));
	}
}

/* Triggers when a new window has been opened */
// in case a new maximized non-active window appears
static void window_opened (WnckScreen *screen,
                           WnckWindow *window,
                           WBApplet *wbapplet) {

	wbapplet->umaxedwindow = getUpperMaximized(wbapplet);

	//updateImages(wbapplet); // not required(?)
}

/* Triggers when a window has been closed */
// in case the last maximized window was closed
static void window_closed (WnckScreen *screen,
                           WnckWindow *window,
                           WBApplet *wbapplet) {

	wbapplet->umaxedwindow = getUpperMaximized(wbapplet);

	updateImages(wbapplet); //required to hide buttons when last window is closed
}

/* Triggers when a new active window is selected */
static void active_window_changed (WnckScreen *screen,
                                   WnckWindow *previous,
                                   WBApplet *wbapplet) {
	gint i;
	
	// Start tracking the new active window:
	if (wbapplet->activewindow)
		if (g_signal_handler_is_connected(G_OBJECT(wbapplet->activewindow), wbapplet->active_handler))
			g_signal_handler_disconnect(G_OBJECT(wbapplet->activewindow), wbapplet->active_handler);
	wbapplet->activewindow = wnck_screen_get_active_window(screen);
	wbapplet->umaxedwindow = getUpperMaximized (wbapplet); // returns wbapplet->activewindow if not only_maximized
	wbapplet->rootwindow = getRootWindow(wbapplet->activescreen);

	// if we got NULL it would start flickering (but we shouldn't get NULL anymore)
	if (wbapplet->activewindow) {
		wbapplet->active_handler = g_signal_connect(G_OBJECT (wbapplet->activewindow), "state-changed", G_CALLBACK (active_window_state_changed), wbapplet);

		if (wbapplet->activewindow == wbapplet->umaxedwindow) {
			// maximized window is on top
			for (i=0; i<WB_BUTTONS; i++) {
				wbapplet->button[i]->state |= WB_BUTTON_STATE_FOCUSED;
			}
		} else {
			if (wbapplet->prefs->only_maximized) {
				for (i=0; i<WB_BUTTONS; i++) {
					wbapplet->button[i]->state &= ~WB_BUTTON_STATE_FOCUSED;
				}
			}
		}

		updateImages(wbapplet);
	}
}

/* Trigger when activewindow's state changes */
static void active_window_state_changed (WnckWindow *window,
                                         WnckWindowState changed_mask,
                                         WnckWindowState new_state,
                                         WBApplet *wbapplet) {
	gint i;
	
	wbapplet->umaxedwindow = getUpperMaximized(wbapplet);
	wbapplet->rootwindow = getRootWindow(wbapplet->activescreen);

	if ( new_state & (WNCK_WINDOW_STATE_MAXIMIZED_HORIZONTALLY | WNCK_WINDOW_STATE_MAXIMIZED_VERTICALLY) ) {
		for (i=0; i<WB_BUTTONS; i++) {
			wbapplet->button[i]->state |= WB_BUTTON_STATE_FOCUSED;
		}
	} else {
		if (wbapplet->prefs->only_maximized) {
			for (i=0; i<WB_BUTTONS; i++) {
				wbapplet->button[i]->state &= ~WB_BUTTON_STATE_FOCUSED;
			}
		}
	}
	
	updateImages(wbapplet);
}

/* Triggers when umaxedwindow's state changes */
static void umaxed_window_state_changed (WnckWindow *window,
                                          WnckWindowState changed_mask,
                                          WnckWindowState new_state,
                                          WBApplet *wbapplet) {

	wbapplet->umaxedwindow = getUpperMaximized(wbapplet);
	wbapplet->rootwindow = getRootWindow(wbapplet->activescreen);
	
	updateImages(wbapplet); // need to hide buttons after click if desktop is below
}

/* Triggers when user changes viewports on Compiz */
// We ONLY need this for Compiz (Metacity doesn't use viewports)
static void viewports_changed (WnckScreen *screen,
                               WBApplet *wbapplet)
{
	wbapplet->activeworkspace = wnck_screen_get_active_workspace(screen);
	wbapplet->activewindow = wnck_screen_get_active_window(screen);
	wbapplet->rootwindow = getRootWindow(wbapplet->activescreen);
	wbapplet->umaxedwindow = getUpperMaximized(wbapplet);

	// active_window_changed should do this too, because this one will be too sooner
	updateImages(wbapplet);
}

/* Triggers when user changes workspace on Metacity (?) */
static void active_workspace_changed (WnckScreen *screen,
                                      WnckWorkspace *previous,
                                      WBApplet *wbapplet) {

	wbapplet->activeworkspace = wnck_screen_get_active_workspace(screen);
	/* // apparently active_window_changed handles this (?)
	wbapplet->activewindow = wnck_screen_get_active_window(screen);
	wbapplet->rootwindow = getRootWindow(wbapplet->activescreen);
	wbapplet->umaxedwindow = getUpperMaximized(wbapplet);
	*/

	//updateImages(wbapplet);
}

/* Called when we release the click on a button */
static gboolean button_release (GtkWidget *event_box, GdkEventButton *event, WBApplet *wbapplet) {
	
	WnckWindow *controlledwindow;
	const GdkPixbuf *imgpb;
	gint i, imgw, imgh;
								   
	if (event->button != 1) return FALSE;

	// Find our button:
	for (i=0; i<WB_BUTTONS; i++) {
		if (GTK_EVENT_BOX(event_box) == wbapplet->button[i]->eventbox) {
			break;
		}
	}

	if (wbapplet->prefs->click_effect) {
		wbapplet->button[i]->state &= ~WB_BUTTON_STATE_CLICKED;
	}

	imgpb = gtk_image_get_pixbuf(wbapplet->button[i]->image);
	imgw = gdk_pixbuf_get_width(imgpb);
	imgh = gdk_pixbuf_get_height(imgpb);

	if (!(event->x<0 || event->y<0 || event->x>imgw || event->y>imgh)) {
		if (wbapplet->prefs->only_maximized) {
			controlledwindow = wbapplet->umaxedwindow;
		} else {
			controlledwindow = wbapplet->activewindow;
		}
		
		switch (i) {
			case WB_BUTTON_MINIMIZE:
				wnck_window_minimize(controlledwindow);
				break;
			case WB_BUTTON_UMAXIMIZE:
				if (wnck_window_is_maximized(controlledwindow)) {
					wnck_window_unmaximize(controlledwindow);
					wnck_window_activate(controlledwindow, gtk_get_current_event_time()); // make unmaximized window active
				} else {
					wnck_window_maximize(controlledwindow);
				}
				break;
			case WB_BUTTON_CLOSE:
				wnck_window_close(controlledwindow, GDK_CURRENT_TIME);
				break;
		}
	}

	updateImages(wbapplet);
	
	return TRUE;
}

/* Called when we click on a button */
static gboolean button_press (GtkWidget *event_box,
                             GdkEventButton *event,
                             WBApplet *wbapplet) {
	gint i;
								 
	if (event->button != 1) return FALSE;

	// Find our button:
	if (wbapplet->prefs->click_effect) {
		for (i=0; i<WB_BUTTONS; i++) {
			if (GTK_EVENT_BOX(event_box) == wbapplet->button[i]->eventbox) {
				wbapplet->button[i]->state |= WB_BUTTON_STATE_CLICKED;
				break;
			}
		}
	
		updateImages(wbapplet);
	}

	return TRUE;
}

/* Makes the button 'glow' when the mouse enters it */
static gboolean hover_enter (GtkWidget *widget,
                         GdkEventCrossing *event,
                         WBApplet *wbapplet) {
	gint i;

	// Find our button:
	if (wbapplet->prefs->hover_effect) {
		for (i=0; i<WB_BUTTONS; i++) {	
			if (GTK_EVENT_BOX(widget) == wbapplet->button[i]->eventbox) {
				wbapplet->button[i]->state |= WB_BUTTON_STATE_HOVERED;
				break;
			}
		}
		 
		updateImages(wbapplet);
	}
							 
	return TRUE;
}

/* Makes the button stop 'glowing' when the mouse leaves it */
static gboolean hover_leave (GtkWidget *widget,
                         GdkEventCrossing *event,
                         WBApplet *wbapplet) {
	gint i;

	// Find our button:
	if (wbapplet->prefs->hover_effect) {
		for (i=0; i<WB_BUTTONS; i++) {
			if (GTK_EVENT_BOX(widget) == wbapplet->button[i]->eventbox) {	
				wbapplet->button[i]->state &= ~WB_BUTTON_STATE_HOVERED;
				break;
			}
		}
		
		updateImages(wbapplet);
	}

	return TRUE;
}

WindowButton **createButtons (WBApplet *wbapplet) {
	WindowButton **button = g_new(WindowButton*, WB_BUTTONS);
	gint i;

	for (i=0; i<WB_BUTTONS; i++) {	
		button[i] = g_new0(WindowButton, 1);
		button[i]->eventbox = GTK_EVENT_BOX(gtk_event_box_new());
		button[i]->image = GTK_IMAGE(gtk_image_new());
	
		// Woohooo! This line eliminates PanelApplet borders - no more workarounds!
		gtk_widget_set_can_focus(GTK_WIDGET(button[i]->eventbox), TRUE);

		// Toggle tooltips (this is pointless because it is overridden by gtk_widget_set_tooltip_text())
		//gtk_widget_set_has_tooltip (GTK_WIDGET(button[i]->image), wbapplet->prefs->show_tooltips);
		
		button[i]->state = 0;
		button[i]->state |= WB_BUTTON_STATE_FOCUSED;
		if (wbapplet->prefs->button_hidden[i]) {
			button[i]->state |= WB_BUTTON_STATE_HIDDEN;
		} else {
			button[i]->state &= ~WB_BUTTON_STATE_HIDDEN;
		}
	
		gtk_container_add (GTK_CONTAINER (button[i]->eventbox), GTK_WIDGET(button[i]->image));
		gtk_event_box_set_visible_window (button[i]->eventbox, FALSE);
	
		// Add hover events to eventboxes:
		gtk_widget_add_events (GTK_WIDGET (button[i]->eventbox), GDK_ENTER_NOTIFY_MASK); //add the "enter" signal
		gtk_widget_add_events (GTK_WIDGET (button[i]->eventbox), GDK_LEAVE_NOTIFY_MASK); //add the "leave" signal

		// Connect buttons to their respective callback functions
		g_signal_connect (G_OBJECT (button[i]->eventbox), "button-release-event", G_CALLBACK (button_release), wbapplet);
		g_signal_connect (G_OBJECT (button[i]->eventbox), "button-press-event", G_CALLBACK (button_press), wbapplet);
		g_signal_connect (G_OBJECT (button[i]->eventbox), "enter-notify-event", G_CALLBACK (hover_enter), wbapplet);
		g_signal_connect (G_OBJECT (button[i]->eventbox), "leave-notify-event", G_CALLBACK (hover_leave), wbapplet);	
	}

	return button;
}

// Places our buttons in correct order
void placeButtons(WBApplet *wbapplet) {
	gint i, j;

	// Determine angle for pixmaps, box orientation and packing direction
	// TODO: It's a bit messy, there's got to be a way to simplify this cluster!
	if (wbapplet->prefs->orientation == 1) {
		// Horizontal position: The pixmaps need to be rotated to 0° in every case
		wbapplet->angle = GDK_PIXBUF_ROTATE_NONE; //0;
		wbapplet->packtype = GTK_PACK_START;		
		gtk_orientable_set_orientation(GTK_ORIENTABLE(wbapplet->box), GTK_ORIENTATION_HORIZONTAL);
	} else if (wbapplet->prefs->orientation == 2) {
		// Vertical position: The pixmaps need to be rotated to: Left/Down=270°, Right/Up=90°
		if (wbapplet->orient == PANEL_APPLET_ORIENT_LEFT || wbapplet->orient == PANEL_APPLET_ORIENT_DOWN) {
			wbapplet->angle = GDK_PIXBUF_ROTATE_CLOCKWISE; // = 270°
			wbapplet->packtype = GTK_PACK_START;
		} else {
			wbapplet->angle = GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE; // = 90°
			wbapplet->packtype = GTK_PACK_END;
		}
		gtk_orientable_set_orientation(GTK_ORIENTABLE(wbapplet->box), GTK_ORIENTATION_VERTICAL);
	} else {
		// Automatic position (default setting)
		if (wbapplet->orient == PANEL_APPLET_ORIENT_RIGHT) {
			wbapplet->angle = GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE;
			wbapplet->packtype = GTK_PACK_END;
			gtk_orientable_set_orientation(GTK_ORIENTABLE(wbapplet->box), GTK_ORIENTATION_VERTICAL);
		} else if (wbapplet->orient == PANEL_APPLET_ORIENT_LEFT) {
			wbapplet->angle = GDK_PIXBUF_ROTATE_CLOCKWISE;
			wbapplet->packtype = GTK_PACK_START;
			gtk_orientable_set_orientation(GTK_ORIENTABLE(wbapplet->box), GTK_ORIENTATION_VERTICAL);
		} else {
			wbapplet->angle = GDK_PIXBUF_ROTATE_NONE;
			wbapplet->packtype = GTK_PACK_START;
			gtk_orientable_set_orientation(GTK_ORIENTABLE(wbapplet->box), GTK_ORIENTATION_HORIZONTAL);
		}
	}

	if (wbapplet->prefs->reverse_order) {
		wbapplet->packtype = (wbapplet->packtype==GTK_PACK_START)?GTK_PACK_END:GTK_PACK_START;
	}
	
	// Add eventboxes to box in preferred order	
	for (i=0; i<WB_BUTTONS; i++) {
		for (j=0; j<WB_BUTTONS; j++) {
			if (wbapplet->prefs->eventboxposition[j] == i) {
				if (wbapplet->packtype == GTK_PACK_START) {
					gtk_box_pack_start(wbapplet->box, GTK_WIDGET(wbapplet->button[j]->eventbox), TRUE, TRUE, 0);
				} else if (wbapplet->packtype == GTK_PACK_END) {
					gtk_box_pack_end(wbapplet->box, GTK_WIDGET(wbapplet->button[j]->eventbox), TRUE, TRUE, 0);
				}
				break;
			}
		}
	}

	// Rotate pixmaps
	for (i=0; i<WB_IMAGE_STATES; i++) {
		for (j=0; j<WB_IMAGES; j++) {
			wbapplet->pixbufs[i][j] = gdk_pixbuf_rotate_simple(wbapplet->pixbufs[i][j], wbapplet->angle);
		}
	}
}

void reloadButtons(WBApplet *wbapplet) {
	gint i;

	// Remove eventbuttons from box:
	for (i=0; i<WB_BUTTONS; i++) {
		g_object_ref(wbapplet->button[i]->eventbox);
		gtk_container_remove(GTK_CONTAINER(wbapplet->box), GTK_WIDGET(wbapplet->button[i]->eventbox));
	}

	placeButtons(wbapplet);

	for (i=0; i<WB_BUTTONS; i++) {
		g_object_unref(wbapplet->button[i]->eventbox);
	}
}

/* Triggered when a different panel orientation is detected */
void applet_change_orient (PanelApplet *panelapplet, PanelAppletOrient orient, WBApplet *wbapplet) {
	if (orient != wbapplet->orient) {
		wbapplet->orient = orient;
		wbapplet->pixbufs = getPixbufs(wbapplet->prefs->images);	
		reloadButtons(wbapplet);
		updateImages(wbapplet);
	}
}

/* Do the actual applet initialization */
static void init_wbapplet(PanelApplet *applet) {
	WBApplet *wbapplet = g_new0 (WBApplet, 1);

	wbapplet->applet = applet;
	wbapplet->prefs = loadPreferences(wbapplet);
	wbapplet->activescreen = wnck_screen_get_default();
	wnck_screen_force_update(wbapplet->activescreen);
	wbapplet->activeworkspace = wnck_screen_get_active_workspace(wbapplet->activescreen);
	wbapplet->activewindow = wnck_screen_get_active_window(wbapplet->activescreen);
	wbapplet->umaxedwindow = getUpperMaximized(wbapplet);
	wbapplet->rootwindow = getRootWindow(wbapplet->activescreen);
	wbapplet->prefbuilder = gtk_builder_new();
	wbapplet->box = GTK_BOX(gtk_hbox_new(FALSE, 0));
	wbapplet->button = createButtons(wbapplet);
	wbapplet->orient = panel_applet_get_orient(wbapplet->applet);
	wbapplet->pixbufs = getPixbufs(wbapplet->prefs->images);
	
	// Rotate & place buttons
	placeButtons(wbapplet);	
	
	// Add box to applet
	gtk_container_add (GTK_CONTAINER(wbapplet->applet), GTK_WIDGET(wbapplet->box));

	// Global window tracking
	g_signal_connect(wbapplet->activescreen, "active-window-changed", G_CALLBACK (active_window_changed), wbapplet);
	g_signal_connect(wbapplet->activescreen, "viewports-changed", G_CALLBACK (viewports_changed), wbapplet);
	g_signal_connect(wbapplet->activescreen, "active-workspace-changed", G_CALLBACK (active_workspace_changed), wbapplet);
	g_signal_connect(wbapplet->activescreen, "window-closed", G_CALLBACK (window_closed), wbapplet);
	g_signal_connect(wbapplet->activescreen, "window-opened", G_CALLBACK (window_opened), wbapplet);

//	g_signal_connect(G_OBJECT (wbapplet->applet), "change-background", G_CALLBACK (applet_change_background), wbapplet);
	g_signal_connect(G_OBJECT (wbapplet->applet), "change-orient", G_CALLBACK (applet_change_orient), wbapplet);

	// ???: Is this still necessary?
	wbapplet->active_handler = 
		g_signal_connect(G_OBJECT (wbapplet->activewindow), "state-changed", G_CALLBACK (active_window_state_changed), wbapplet);

	// Setup applet right-click menu
	GtkActionGroup *action_group = gtk_action_group_new ("WindowButtons Applet Actions");
	gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);
	gtk_action_group_add_actions (action_group, windowbuttons_menu_actions, G_N_ELEMENTS (windowbuttons_menu_actions), wbapplet);
	panel_applet_setup_menu (applet, windowbuttons_menu_items, action_group);
	panel_applet_set_background_widget (wbapplet->applet, GTK_WIDGET (wbapplet->applet)); // Automatic background update
	
	toggleHidden (wbapplet);	// Properly hide or show stuff
	updateImages (wbapplet);
}

/* We need this because things have to be hidden after we 'show' the applet */
void toggleHidden (WBApplet *wbapplet) {
	gint j;
	for (j=0; j<WB_BUTTONS; j++) {
		if (wbapplet->button[j]->state & WB_BUTTON_STATE_HIDDEN) {
			gtk_widget_hide(GTK_WIDGET(wbapplet->button[j]->eventbox));
		} else {
			gtk_widget_show(GTK_WIDGET(wbapplet->button[j]->eventbox));
		}
	}

	if (!gtk_widget_get_visible(GTK_WIDGET(wbapplet->box)))
		gtk_widget_show_all(GTK_WIDGET(wbapplet->box));
	if (!gtk_widget_get_visible(GTK_WIDGET(wbapplet->applet)))
		gtk_widget_show_all(GTK_WIDGET(wbapplet->applet));
}

// Initial function that creates the applet
static gboolean windowbuttons_applet_factory (PanelApplet *applet, const gchar *iid, gpointer data) {
	if (g_strcmp0(iid, APPLET_OAFIID) != 0) return FALSE;

	g_set_application_name (APPLET_NAME);
	panel_applet_set_flags (applet, PANEL_APPLET_EXPAND_MINOR);
	panel_applet_add_preferences (applet, GCONF_PREFS, NULL);
	
	init_wbapplet(applet);

	return TRUE;
}

PANEL_APPLET_OUT_PROCESS_FACTORY (APPLET_OAFIID_FACTORY,
                                  PANEL_TYPE_APPLET,
                                  (PanelAppletFactoryCallback) windowbuttons_applet_factory,
                                  NULL)