/*
 * Copyright (C) 2009 Andrej Belcijan
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *     Andrej Belcijan <{andrejx} at {gmail.com}>
 */

#include "config.h"
#include "window-buttons-private.h"
#include "preferences.h"

/* Prototypes */
static void init_wbapplet (WBApplet *);
static void active_window_state_changed (WnckWindow *, WnckWindowState, WnckWindowState, WBApplet *);
static void umaxed_window_state_changed (WnckWindow *, WnckWindowState, WnckWindowState, WBApplet *);
static WnckWindow *getRootWindow (WnckScreen *screen);
static WnckWindow *getUpperMaximized (WBApplet *);
void setCustomLayout (WBPreferences *, gchar *);
void rotatePixbufs(WBApplet *);
void placeButtons(WBApplet *);
void reloadButtons(WBApplet *);
void toggleHidden(WBApplet *);
void loadThemes(GtkComboBox *, gchar *);
WBPreferences *loadPreferences(WBApplet *);
gchar *getMetacityLayout(void);
GdkPixbuf ***getPixbufs(gchar ***);

G_DEFINE_TYPE (WBApplet, wb_applet, GP_TYPE_APPLET)

static void
about_cb (GSimpleAction *action,
          GVariant      *parameter,
          gpointer       user_data)
{
  gp_applet_show_about (GP_APPLET (user_data));
}

static const GActionEntry windowbuttons_menu_actions [] = {
	{ "preferences", wb_applet_properties_cb,  NULL, NULL, NULL },
	{ "about",       about_cb, NULL, NULL, NULL },
	{ NULL }
};

static void
wb_applet_constructed (GObject *object)
{
  G_OBJECT_CLASS (wb_applet_parent_class)->constructed (object);
  init_wbapplet (WB_APPLET (object));
}

static void
wb_applet_dispose (GObject *object)
{
  WBApplet *self;

  self = WB_APPLET (object);

  if (self->active_window_changed_id != 0)
    {
      g_signal_handler_disconnect (self->activescreen,
                                   self->active_window_changed_id);
      self->active_window_changed_id = 0;
    }

  if (self->viewports_changed_id != 0)
    {
      g_signal_handler_disconnect (self->activescreen,
                                   self->viewports_changed_id);
      self->viewports_changed_id = 0;
    }

  if (self->active_workspace_changed_id != 0)
    {
      g_signal_handler_disconnect (self->activescreen,
                                   self->active_workspace_changed_id);
      self->active_workspace_changed_id = 0;
    }

  if (self->window_closed_id != 0)
    {
      g_signal_handler_disconnect (self->activescreen,
                                   self->window_closed_id);
      self->window_closed_id = 0;
    }

  if (self->window_opened_id != 0)
    {
      g_signal_handler_disconnect (self->activescreen,
                                   self->window_opened_id);
      self->window_opened_id = 0;
    }

  if (self->activewindow != NULL)
    {
      if (self->active_handler != 0)
        {
          g_signal_handler_disconnect (self->activewindow,
                                       self->active_handler);
          self->active_handler = 0;
        }
    }

  g_clear_object (&self->handle);

  G_OBJECT_CLASS (wb_applet_parent_class)->dispose (object);
}

static void
wb_applet_class_init (WBAppletClass *self_class)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (self_class);

  object_class->constructed = wb_applet_constructed;
  object_class->dispose = wb_applet_dispose;
}

static void
wb_applet_init (WBApplet *self)
{
  gp_applet_set_flags (GP_APPLET (self), GP_APPLET_FLAGS_EXPAND_MINOR);
}

static WnckWindow *getRootWindow (WnckScreen *screen) {
	GList *winstack = wnck_screen_get_windows_stacked(screen);
	if (winstack) // we can't access data directly because sometimes we will get NULL
		return winstack->data;
	else
		return NULL;
}

/* Returns the highest maximized window */
static WnckWindow *
getUpperMaximized (WBApplet *wbapplet)
{
	GList *windows;
	WnckWindow *returnwindow;

	if (!wbapplet->prefs->only_maximized)
		return wbapplet->activewindow;

	windows = wnck_screen_get_windows_stacked(wbapplet->activescreen);
	returnwindow = NULL;

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

static gushort
getImageState (WBButtonState button_state)
{
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
void
wb_applet_update_images (WBApplet *wbapplet)
{
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
			gtk_widget_set_tooltip_text (GTK_WIDGET(wbapplet->button[WB_BUTTON_UMAXIMIZE]->image), _("Unmaximize")); // Set correct tooltip
	} else {
		gtk_image_set_from_pixbuf (wbapplet->button[WB_BUTTON_UMAXIMIZE]->image, wbapplet->pixbufs[getImageState(wbapplet->button[WB_BUTTON_UMAXIMIZE]->state)][WB_IMAGE_MAXIMIZE]);
		if (wbapplet->prefs->show_tooltips)
			gtk_widget_set_tooltip_text (GTK_WIDGET(wbapplet->button[WB_BUTTON_UMAXIMIZE]->image), _("Maximize")); // Set correct tooltip
	}
	//update close button
	gtk_image_set_from_pixbuf (wbapplet->button[WB_BUTTON_CLOSE]->image, wbapplet->pixbufs[getImageState(wbapplet->button[WB_BUTTON_CLOSE]->state)][WB_IMAGE_CLOSE]);

	//set remaining tooltips
	if (wbapplet->prefs->show_tooltips) {
		gtk_widget_set_tooltip_text (GTK_WIDGET(wbapplet->button[WB_BUTTON_MINIMIZE]->image), _("Minimize"));
		gtk_widget_set_tooltip_text (GTK_WIDGET(wbapplet->button[WB_BUTTON_CLOSE]->image), _("Close"));
	}
}

/* Triggers when a new window has been opened */
// in case a new maximized non-active window appears
static void window_opened (WnckScreen *screen,
                           WnckWindow *window,
                           WBApplet *wbapplet) {

	wbapplet->umaxedwindow = getUpperMaximized(wbapplet);

	//wb_applet_update_images(wbapplet); // not required(?)
}

/* Triggers when a window has been closed */
// in case the last maximized window was closed
static void window_closed (WnckScreen *screen,
                           WnckWindow *window,
                           WBApplet *wbapplet) {

	wbapplet->umaxedwindow = getUpperMaximized(wbapplet);

	wb_applet_update_images(wbapplet); //required to hide buttons when last window is closed
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

		wb_applet_update_images(wbapplet);
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

	wb_applet_update_images(wbapplet);
}

/* Triggers when umaxedwindow's state changes */
static void umaxed_window_state_changed (WnckWindow *window,
                                          WnckWindowState changed_mask,
                                          WnckWindowState new_state,
                                          WBApplet *wbapplet) {

	wbapplet->umaxedwindow = getUpperMaximized(wbapplet);
	wbapplet->rootwindow = getRootWindow(wbapplet->activescreen);

	wb_applet_update_images(wbapplet); // need to hide buttons after click if desktop is below
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
	wb_applet_update_images(wbapplet);
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

	//wb_applet_update_images(wbapplet);
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
		WnckWindowActions actions;

		if (wbapplet->prefs->only_maximized) {
			controlledwindow = wbapplet->umaxedwindow;
		} else {
			controlledwindow = wbapplet->activewindow;
		}

		actions = wnck_window_get_actions (controlledwindow);

		switch (i) {
			case WB_BUTTON_MINIMIZE:
				if ((actions & WNCK_WINDOW_ACTION_MINIMIZE) == WNCK_WINDOW_ACTION_MINIMIZE)
					wnck_window_minimize (controlledwindow);
				break;
			case WB_BUTTON_UMAXIMIZE:
				if (wnck_window_is_maximized (controlledwindow) &&
				    (actions & WNCK_WINDOW_ACTION_UNMAXIMIZE) == WNCK_WINDOW_ACTION_UNMAXIMIZE) {
					wnck_window_unmaximize(controlledwindow);
					wnck_window_activate(controlledwindow, gtk_get_current_event_time()); // make unmaximized window active
				} else if ((actions & WNCK_WINDOW_ACTION_MAXIMIZE) == WNCK_WINDOW_ACTION_MAXIMIZE) {
					wnck_window_maximize(controlledwindow);
				}
				break;
			case WB_BUTTON_CLOSE:
				if ((actions & WNCK_WINDOW_ACTION_CLOSE) == WNCK_WINDOW_ACTION_CLOSE)
					wnck_window_close (controlledwindow, GDK_CURRENT_TIME);
				break;
			default:
				g_assert_not_reached ();
				break;
		}
	}

	wb_applet_update_images(wbapplet);

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

		wb_applet_update_images(wbapplet);
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

		wb_applet_update_images(wbapplet);
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

		wb_applet_update_images(wbapplet);
	}

	return TRUE;
}

static WindowButton **
createButtons (WBApplet *wbapplet)
{
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
		if (wbapplet->orient == GTK_ORIENTATION_VERTICAL) {
			wbapplet->angle = GDK_PIXBUF_ROTATE_CLOCKWISE; // = 270°
			wbapplet->packtype = GTK_PACK_START;
		} else {
			wbapplet->angle = GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE; // = 90°
			wbapplet->packtype = GTK_PACK_END;
		}
		gtk_orientable_set_orientation(GTK_ORIENTABLE(wbapplet->box), GTK_ORIENTATION_VERTICAL);
	} else {
		// Automatic position (default setting)
		if (wbapplet->position == GTK_POS_LEFT) {
			wbapplet->angle = GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE;
			wbapplet->packtype = GTK_PACK_END;
			gtk_orientable_set_orientation(GTK_ORIENTABLE(wbapplet->box), GTK_ORIENTATION_VERTICAL);
		} else if (wbapplet->position == GTK_POS_RIGHT) {
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

static void
placement_changed_cb (GpApplet        *applet,
                      GtkOrientation   orientation,
                      GtkPositionType  position,
                      WBApplet        *self)
{
  if (self->orient == orientation && self->position == position)
    return;

  self->orient = orientation;
  self->position = position;

  self->pixbufs = getPixbufs (self->prefs->images);
  reloadButtons (self);

  wb_applet_update_images (self);
}

static void
init_wbapplet (WBApplet *wbapplet)
{
	const char *menu_resource;

	wbapplet->settings = gp_applet_settings_new (GP_APPLET (wbapplet), WINDOWBUTTONS_GSCHEMA);
	wbapplet->prefs = loadPreferences(wbapplet);
	wbapplet->handle = wnck_handle_new (WNCK_CLIENT_TYPE_PAGER);
	wbapplet->activescreen = wnck_handle_get_default_screen (wbapplet->handle);
	wnck_screen_force_update(wbapplet->activescreen);
	wbapplet->activeworkspace = wnck_screen_get_active_workspace(wbapplet->activescreen);
	wbapplet->activewindow = wnck_screen_get_active_window(wbapplet->activescreen);
	wbapplet->umaxedwindow = getUpperMaximized(wbapplet);
	wbapplet->rootwindow = getRootWindow(wbapplet->activescreen);
	wbapplet->prefbuilder = gtk_builder_new();
	wbapplet->box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
	wbapplet->button = createButtons(wbapplet);
	wbapplet->orient = gp_applet_get_orientation(GP_APPLET (wbapplet));
	wbapplet->position = gp_applet_get_position(GP_APPLET (wbapplet));
	wbapplet->pixbufs = getPixbufs(wbapplet->prefs->images);

	// Rotate & place buttons
	placeButtons(wbapplet);

	// Add box to applet
	gtk_container_add (GTK_CONTAINER(wbapplet), GTK_WIDGET(wbapplet->box));

	// Global window tracking
	// Global window tracking
	wbapplet->active_window_changed_id = g_signal_connect (wbapplet->activescreen,
	                                                       "active-window-changed",
	                                                       G_CALLBACK (active_window_changed),
	                                                       wbapplet);
	wbapplet->viewports_changed_id = g_signal_connect (wbapplet->activescreen,
	                                                   "viewports-changed",
	                                                   G_CALLBACK (viewports_changed),
	                                                   wbapplet);
	wbapplet->active_workspace_changed_id = g_signal_connect (wbapplet->activescreen,
	                                                          "active-workspace-changed",
	                                                          G_CALLBACK (active_workspace_changed),
	                                                          wbapplet);
	wbapplet->window_closed_id = g_signal_connect (wbapplet->activescreen,
	                                               "window-closed",
	                                               G_CALLBACK (window_closed),
	                                               wbapplet);
	wbapplet->window_opened_id = g_signal_connect (wbapplet->activescreen,
	                                               "window-opened",
	                                               G_CALLBACK (window_opened),
	                                               wbapplet);

	g_signal_connect (wbapplet, "placement-changed", G_CALLBACK (placement_changed_cb), wbapplet);

	// ???: Is this still necessary?
	wbapplet->active_handler =
		g_signal_connect(G_OBJECT (wbapplet->activewindow), "state-changed", G_CALLBACK (active_window_state_changed), wbapplet);

	// Setup applet right-click menu
	menu_resource = GRESOURCE_PREFIX "/ui/window-buttons-menu.ui";
	gp_applet_setup_menu_from_resource (GP_APPLET (wbapplet),
	                                    menu_resource,
	                                    windowbuttons_menu_actions);

	toggleHidden (wbapplet);	// Properly hide or show stuff
	wb_applet_update_images (wbapplet);
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
	if (!gtk_widget_get_visible(GTK_WIDGET(wbapplet)))
		gtk_widget_show_all(GTK_WIDGET(wbapplet));
}

void
wb_applet_setup_about (GtkAboutDialog *dialog)
{
  const char *comments;
  const char **authors;
  const char **artists;
  const char **documenters;
  const char *copyright;

  comments = _("Window buttons for your GNOME Panel.");

  authors = (const char *[])
    {
      "Andrej Belcijan <{andrejx}at{gmail.com}>",
      " ",
      "Also contributed:",
      "quiescens",
      NULL
    };

  artists = (const char *[])
    {
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

  documenters = (const char *[])
    {
      "Andrej Belcijan <{andrejx}at{gmail.com}>",
      NULL
    };

  copyright = "\xC2\xA9 2011 Andrej Belcijan";

  gtk_about_dialog_set_comments (dialog, comments);

  gtk_about_dialog_set_authors (dialog, authors);
  gtk_about_dialog_set_artists (dialog, artists);
  gtk_about_dialog_set_documenters (dialog, documenters);
  gtk_about_dialog_set_translator_credits (dialog, _("translator-credits"));
  gtk_about_dialog_set_copyright (dialog, copyright);

  gtk_about_dialog_set_website_label (dialog, _("Window Applets on Gnome-Look"));
  gtk_about_dialog_set_website (dialog, "http://www.gnome-look.org/content/show.php?content=103732");
}
