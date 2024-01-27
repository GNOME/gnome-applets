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
#include "window-title.h"

#include <glib/gi18n-lib.h>

#include "preferences.h"

static void init_wtapplet (WTApplet *wtapplet);
static void active_window_state_changed (WnckWindow *, WnckWindowState, WnckWindowState, WTApplet *);
static void active_window_nameicon_changed (WnckWindow *, WTApplet *);
static void umaxed_window_state_changed (WnckWindow *, WnckWindowState, WnckWindowState, WTApplet *);
static void umaxed_window_nameicon_changed (WnckWindow *, WTApplet *);

G_DEFINE_TYPE (WTApplet, wt_applet, GP_TYPE_APPLET)

static void
about_cb (GSimpleAction *action,
          GVariant      *parameter,
          gpointer       user_data)
{
  gp_applet_show_about (GP_APPLET (user_data));
}

static GActionEntry windowtitle_menu_actions [] = {
	{ "preferences", wt_applet_properties_cb,  NULL, NULL, NULL },
	{ "about",       about_cb, NULL, NULL, NULL },
	{ NULL }
};

static void
wt_applet_constructed (GObject *object)
{
  G_OBJECT_CLASS (wt_applet_parent_class)->constructed (object);
  init_wtapplet (WT_APPLET (object));
}

static void
wt_applet_dispose (GObject *object)
{
  WTApplet *self;

  self = WT_APPLET (object);

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
      if (self->active_handler_state != 0)
        {
          g_signal_handler_disconnect (self->activewindow,
                                       self->active_handler_state);
          self->active_handler_state = 0;
        }

      if (self->active_handler_name != 0)
        {
          g_signal_handler_disconnect (self->activewindow,
                                       self->active_handler_name);
          self->active_handler_name = 0;
        }

      if (self->active_handler_icon != 0)
        {
          g_signal_handler_disconnect (self->activewindow,
                                       self->active_handler_icon);
          self->active_handler_icon = 0;
        }
    }

  g_clear_object (&self->handle);

  G_OBJECT_CLASS (wt_applet_parent_class)->dispose (object);
}

static void
wt_applet_class_init (WTAppletClass *self_class)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (self_class);

  object_class->constructed = wt_applet_constructed;
  object_class->dispose = wt_applet_dispose;
}

static void
wt_applet_init (WTApplet *self)
{
  self->handle = wnck_handle_new (WNCK_CLIENT_TYPE_PAGER);
}

/* Safely returns the bottom-most (root) window */
static WnckWindow *getRootWindow (WnckScreen *screen) {
	GList *winstack = wnck_screen_get_windows_stacked(screen);
	if (winstack)
		return winstack->data;
	else
		return NULL;
}

static WnckWindow *
getUpperMaximized (WTApplet *wtapplet)
{
	GList *windows;
	WnckWindow *returnwindow;

	if (!wtapplet->prefs->only_maximized)
		return wtapplet->activewindow;

	windows = wnck_screen_get_windows_stacked(wtapplet->activescreen);
	returnwindow = NULL;

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
void
wt_applet_update_title (WTApplet *wtapplet)
{
	WnckWindow *controlledwindow;
	const char *title_text, *title_color, *title_font;
	GString *markup;
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
			title_text = _("Desktop");
		}
	} else {
		icon_pixbuf = wnck_window_get_icon(controlledwindow); // This only returns a pointer - it SHOULDN'T be unrefed!
		title_text = wnck_window_get_name(controlledwindow);
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
			title_color = "";
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

	markup = g_string_new ("<span");

	if (title_font != NULL && *title_font != '\0')
		g_string_append_printf (markup, " font=\"%s\"", title_font);

	if (title_color != NULL && *title_color != '\0')
		g_string_append_printf (markup, " color=\"%s\"", title_color);

	g_string_append_printf (markup, ">%s</span>", title_text);

	// Apply markup to label widget
	gtk_label_set_markup(GTK_LABEL(wtapplet->title), markup->str);
	g_string_free (markup, TRUE);

	if (icon_pixbuf == NULL) {
		gtk_image_clear(wtapplet->icon);
	} else {
		GdkPixbuf *ipb1;
		GdkPixbuf *ipb2;

		// We're updating window info (Careful! We've had pixbuf memory leaks here)
		ipb1 = gdk_pixbuf_scale_simple (icon_pixbuf, ICON_WIDTH, ICON_HEIGHT, GDK_INTERP_BILINEAR);

		if (controlledwindow == wtapplet->rootwindow) g_object_unref(icon_pixbuf); //this is stupid beyond belief, thanks to the retarded GTK framework

		ipb2 = gdk_pixbuf_rotate_simple (ipb1, wtapplet->angle);
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
void
wt_applet_toggle_expand (WTApplet *wtapplet)
{
	if (wtapplet->prefs->expand_applet) {
		gp_applet_set_flags (GP_APPLET (wtapplet), GP_APPLET_FLAGS_EXPAND_MINOR | GP_APPLET_FLAGS_EXPAND_MAJOR);
	} else {
		// We must have a handle due to bug https://bugzilla.gnome.org/show_bug.cgi?id=556355
		// gp_applet_set_flags (GP_APPLET (wtapplet), GP_APPLET_FLAGS_EXPAND_MINOR | GP_APPLET_FLAGS_EXPAND_MAJOR | GP_APPLET_FLAGS_HAS_HANDLE);
		gp_applet_set_flags (GP_APPLET (wtapplet), GP_APPLET_FLAGS_EXPAND_MINOR);
	}
	wt_applet_reload_widgets(wtapplet);
	wt_applet_set_alignment(wtapplet, (gdouble)wtapplet->prefs->alignment);
}

/* Hide/unhide stuff according to preferences */
void
wt_applet_toggle_hidden (WTApplet *wtapplet)
{
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
	if (!gtk_widget_get_visible(GTK_WIDGET(wtapplet)))
		gtk_widget_show_all(GTK_WIDGET(wtapplet));
}

static void
placement_changed_cb (GpApplet        *applet,
                      GtkOrientation   orientation,
                      GtkPositionType  position,
                      WTApplet        *wtapplet)
{
	if (position != wtapplet->position) {
		wtapplet->position = position;

		wt_applet_reload_widgets(wtapplet);
		wt_applet_update_title(wtapplet);
	}
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

	wt_applet_update_title(wtapplet);
}

/* Triggers when a window has been closed */
// in case the last maximized window was closed
static void window_closed (WnckScreen *screen,
                           WnckWindow *window,
                           WTApplet *wtapplet) {

	wtapplet->umaxedwindow = getUpperMaximized(wtapplet);

	wt_applet_update_title(wtapplet); // required when closing window in the background
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

		wt_applet_update_title(wtapplet);
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

	wt_applet_update_title(wtapplet);
}

/* Triggers when activewindow's name changes */
static void active_window_nameicon_changed (WnckWindow *window, WTApplet *wtapplet) {
	wt_applet_update_title(wtapplet);
}

/* Triggers when umaxedwindow's state changes */
static void umaxed_window_state_changed (WnckWindow *window,
                                          WnckWindowState changed_mask,
                                          WnckWindowState new_state,
                                          WTApplet *wtapplet)
{
	wtapplet->umaxedwindow = getUpperMaximized(wtapplet);
	wtapplet->rootwindow = getRootWindow(wtapplet->activescreen);

	wt_applet_update_title(wtapplet);
}

/* Triggers when umaxedwindow's name OR ICON changes */
static void umaxed_window_nameicon_changed(WnckWindow *window, WTApplet *wtapplet) {
	wt_applet_update_title(wtapplet);
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
	wt_applet_update_title(wtapplet);
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

	wt_applet_update_title(wtapplet);
	*/
}

static gboolean
icon_clicked (GtkWidget      *icon,
              GdkEventButton *event,
              WTApplet       *wtapplet)
{
	WnckWindow *controlledwindow;

	if (event->button != 1) return FALSE;

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

static gboolean
title_clicked (GtkWidget      *title,
               GdkEventButton *event,
               WTApplet       *wtapplet)
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
			GtkMenu *window_menu;

			wnck_window_activate(controlledwindow, gtk_get_current_event_time());

			window_menu = GTK_MENU(wnck_action_menu_new(controlledwindow));
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
static void
placeWidgets (WTApplet *wtapplet)
{
	gboolean swap_order;

	// TODO: merge all this... or not?
	if (wtapplet->position == GTK_POS_LEFT) {
		wtapplet->angle = GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE;
	} else if (wtapplet->position == GTK_POS_RIGHT) {
		wtapplet->angle = GDK_PIXBUF_ROTATE_CLOCKWISE;
	} else {
		wtapplet->angle = GDK_PIXBUF_ROTATE_NONE;
	}

	swap_order = wtapplet->prefs->swap_order;

	if (wtapplet->angle == GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE) {
		swap_order = !swap_order;
	}

	// set box orientation
	if (wtapplet->angle == GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE || wtapplet->angle == GDK_PIXBUF_ROTATE_CLOCKWISE) {
		gtk_orientable_set_orientation(GTK_ORIENTABLE(wtapplet->box), GTK_ORIENTATION_VERTICAL);
	} else {
		gtk_orientable_set_orientation(GTK_ORIENTABLE(wtapplet->box), GTK_ORIENTATION_HORIZONTAL);
	}

	// set packing order
	if (!swap_order) {
		gtk_box_pack_start(wtapplet->box, GTK_WIDGET(wtapplet->eb_icon), FALSE, TRUE, 0);
		gtk_box_pack_start(wtapplet->box, GTK_WIDGET(wtapplet->eb_title), TRUE, TRUE, 0);
	} else {
		gtk_box_pack_start(wtapplet->box, GTK_WIDGET(wtapplet->eb_title), TRUE, TRUE, 0);
		gtk_box_pack_start(wtapplet->box, GTK_WIDGET(wtapplet->eb_icon), FALSE, TRUE, 0);
	}

	// Set alignment/orientation
	gtk_label_set_angle( wtapplet->title, wtapplet->angle );
	wt_applet_set_alignment(wtapplet, (gdouble)wtapplet->prefs->alignment);
}

void
wt_applet_reload_widgets (WTApplet *wtapplet)
{
	g_object_ref(wtapplet->eb_icon);
	g_object_ref(wtapplet->eb_title);
	gtk_container_remove(GTK_CONTAINER(wtapplet->box), GTK_WIDGET(wtapplet->eb_icon));
	gtk_container_remove(GTK_CONTAINER(wtapplet->box), GTK_WIDGET(wtapplet->eb_title));
	placeWidgets(wtapplet);
	g_object_unref(wtapplet->eb_icon);
	g_object_unref(wtapplet->eb_title);
}

/* Sets alignment, min size, padding to title according to panel orientation */
void
wt_applet_set_alignment (WTApplet *wtapplet,
                         gdouble alignment)
{
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

static void
init_wtapplet (WTApplet *wtapplet)
{
	GpApplet *applet;
	const char *menu_resource;

	applet = GP_APPLET (wtapplet);

	wtapplet->settings = gp_applet_settings_new (applet, WINDOWTITLE_GSCHEMA);
	wtapplet->prefs = wt_applet_load_preferences(wtapplet);
	wtapplet->activescreen = wnck_handle_get_default_screen (wtapplet->handle);
	wnck_screen_force_update(wtapplet->activescreen);
	wtapplet->activeworkspace = wnck_screen_get_active_workspace(wtapplet->activescreen);
	wtapplet->activewindow = wnck_screen_get_active_window(wtapplet->activescreen);
	wtapplet->umaxedwindow = getUpperMaximized(wtapplet);
	wtapplet->rootwindow = getRootWindow(wtapplet->activescreen);
	wtapplet->prefbuilder = gtk_builder_new ();
	wtapplet->box = GTK_BOX(gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
	wtapplet->icon = GTK_IMAGE(gtk_image_new());
	wtapplet->title = GTK_LABEL(gtk_label_new(NULL));
	wtapplet->eb_icon = GTK_EVENT_BOX(gtk_event_box_new());
	wtapplet->eb_title = GTK_EVENT_BOX(gtk_event_box_new());
	wtapplet->position = gp_applet_get_position (applet);
	wtapplet->size_hints = g_new(gint,2);

	// Widgets to eventboxes, eventboxes to box
	gtk_widget_set_can_focus(GTK_WIDGET(wtapplet->icon), TRUE);
	gtk_widget_set_can_focus(GTK_WIDGET(wtapplet->title), TRUE);
	gtk_container_add (GTK_CONTAINER (wtapplet->eb_icon), GTK_WIDGET(wtapplet->icon));
	gtk_container_add (GTK_CONTAINER (wtapplet->eb_title), GTK_WIDGET(wtapplet->title));
	gtk_event_box_set_visible_window (wtapplet->eb_icon, FALSE);
	gtk_event_box_set_visible_window (wtapplet->eb_title, FALSE);

	// Rotate & place elements
	wt_applet_set_alignment(wtapplet, (gdouble)wtapplet->prefs->alignment);
	placeWidgets(wtapplet);

	// Add box to applet
	gtk_container_add (GTK_CONTAINER(wtapplet), GTK_WIDGET(wtapplet->box));

	// Set event handling (icon & title clicks)
	g_signal_connect(G_OBJECT (wtapplet->eb_icon), "button-press-event", G_CALLBACK (icon_clicked), wtapplet);
	g_signal_connect(G_OBJECT (wtapplet->eb_title), "button-press-event", G_CALLBACK (title_clicked), wtapplet);

	// Global window tracking
	wtapplet->active_window_changed_id = g_signal_connect (wtapplet->activescreen,
	                                                       "active-window-changed",
	                                                       G_CALLBACK (active_window_changed),
	                                                       wtapplet); // <-- this thing is crashing with compiz !!!
	wtapplet->viewports_changed_id = g_signal_connect (wtapplet->activescreen,
	                                                   "viewports-changed",
	                                                   G_CALLBACK (viewports_changed),
	                                                   wtapplet);
	wtapplet->active_workspace_changed_id = g_signal_connect (wtapplet->activescreen,
	                                                          "active-workspace-changed",
	                                                          G_CALLBACK (active_workspace_changed),
	                                                          wtapplet);
	wtapplet->window_closed_id = g_signal_connect (wtapplet->activescreen,
	                                               "window-closed",
	                                               G_CALLBACK (window_closed),
	                                               wtapplet);
	wtapplet->window_opened_id = g_signal_connect (wtapplet->activescreen,
	                                               "window-opened",
	                                               G_CALLBACK (window_opened),
	                                               wtapplet);

	// g_signal_connect(G_OBJECT (wtapplet->title), "size-request", G_CALLBACK (applet_title_size_request), wtapplet);
	g_signal_connect(G_OBJECT (wtapplet), "size-allocate", G_CALLBACK (applet_size_allocate), wtapplet);
	g_signal_connect(G_OBJECT (wtapplet), "placement-changed", G_CALLBACK (placement_changed_cb), wtapplet);

	// Track active window changes
	wtapplet->active_handler_state =
		g_signal_connect(G_OBJECT (wtapplet->activewindow), "state-changed", G_CALLBACK (active_window_state_changed), wtapplet);
	wtapplet->active_handler_name =
		g_signal_connect(G_OBJECT (wtapplet->activewindow), "name-changed", G_CALLBACK (active_window_nameicon_changed), wtapplet);
	wtapplet->active_handler_icon =
		g_signal_connect(G_OBJECT (wtapplet->activewindow), "icon-changed", G_CALLBACK (active_window_nameicon_changed), wtapplet);

	// Setup applet right-click menu
	menu_resource = GRESOURCE_PREFIX "/ui/window-title-menu.ui";
	gp_applet_setup_menu_from_resource (applet, menu_resource, windowtitle_menu_actions);

	wt_applet_toggle_expand (wtapplet);
	wt_applet_toggle_hidden (wtapplet);	// Properly hide or show stuff
	wt_applet_update_title (wtapplet);
}

void
wt_applet_setup_about (GtkAboutDialog *dialog)
{
  const char *comments;
  const char **authors;
  const char **artists;
  const char **documenters;
  const char *copyright;

  comments = _("Window title for your GNOME Panel.");

  authors = (const char *[])
    {
      "Andrej Belcijan <{andrejx}at{gmail.com}>",
      " ",
      "Also contributed:",
      "Niko Bellic <{yurik81}at{gmail.com}>",
      NULL
    };

  artists = (const char *[])
    {
      "Nasser Alshammari <{designernasser}at{gmail.com}>",
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
