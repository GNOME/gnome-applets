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

#ifndef __WB_APPLET_H__
#define __WB_APPLET_H__

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "window-buttons.h"

#include <glib.h>
#include <glib-object.h>
#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>

#ifndef WNCK_I_KNOW_THIS_IS_UNSTABLE
#define WNCK_I_KNOW_THIS_IS_UNSTABLE
#endif
#include <libwnck/libwnck.h>

/* static paths and stuff */
#define PATH_THEMES 					WB_DATA_DIR"/themes"
#define METACITY_XML 					"metacity-theme-1.xml"
#define THEME_EXTENSION					"png"
#define FILE_CONFIGFILE					".windowbuttons"

/* strings that identify button states and names */
#define BTN_STATE_FOCUSED				"focused"
#define BTN_STATE_UNFOCUSED				"unfocused"
#define BTN_STATE_NORMAL				"normal"
#define BTN_STATE_CLICKED				"clicked"
#define BTN_STATE_HOVERED				"hover"	//TODO: change to hovered

#define BTN_NAME_CLOSE					"close"
#define BTN_NAME_MINIMIZE				"minimize"
#define BTN_NAME_MAXIMIZE				"maximize"
#define BTN_NAME_UNMAXIMIZE				"unmaximize"

/* Key strings (used by GSettings, Plaintext and GtkBuilder .ui file) */
#define WINDOWBUTTONS_GSCHEMA		"org.gnome.gnome-applets.window-buttons"
#define CFG_ONLY_MAXIMIZED			"only-maximized"
#define CFG_HIDE_ON_UNMAXIMIZED 	"hide-on-unmaximized"
#define CFG_HIDE_DECORATION			"hide-decoration"
#define CFG_CLICK_EFFECT			"click-effect"
#define CFG_HOVER_EFFECT			"hover-effect"
#define CFG_USE_METACITY_LAYOUT		"use-metacity-layout"
#define CFG_MINIMIZE_HIDDEN			"button-minimize-hidden"
#define CFG_UNMAXIMIZE_HIDDEN		"button-maximize-hidden"
#define CFG_CLOSE_HIDDEN			"button-close-hidden"
#define CFG_BUTTON_LAYOUT			"button-layout"
#define CFG_REVERSE_ORDER			"reverse-order"
#define CFG_ORIENTATION				"orientation"
#define CFG_THEME					"theme"
#define CFG_SHOW_TOOLTIPS			"show-tooltips"

G_BEGIN_DECLS

/* we will index images for convenience */
typedef enum {
	WB_IMAGE_MINIMIZE = 0,
	WB_IMAGE_UNMAXIMIZE,
	WB_IMAGE_MAXIMIZE,
	WB_IMAGE_CLOSE,

	WB_IMAGES
} WBImageIndices;

/* we will also index image states for convenience */
typedef enum {
	WB_IMAGE_FOCUSED_NORMAL = 0,
	WB_IMAGE_FOCUSED_CLICKED,
	WB_IMAGE_FOCUSED_HOVERED,
	WB_IMAGE_UNFOCUSED_NORMAL,
	WB_IMAGE_UNFOCUSED_CLICKED,
	WB_IMAGE_UNFOCUSED_HOVERED,

	WB_IMAGE_STATES
} WBImageStates;

/* indexing of buttons */
typedef enum {
	WB_BUTTON_MINIMIZE = 0,	// minimize button
	WB_BUTTON_UMAXIMIZE,	// maximize/unmaximize button
	WB_BUTTON_CLOSE,		// close button

	WB_BUTTONS				// number of buttons
} WindowButtonIndices;

/* Button state masks */
typedef enum {
	WB_BUTTON_STATE_FOCUSED	= 1 << 0, // [0001]
	WB_BUTTON_STATE_CLICKED	= 1 << 1, // [0010]
	WB_BUTTON_STATE_HOVERED	= 1 << 2, // [0100]
	WB_BUTTON_STATE_HIDDEN	= 1 << 3  // [1000]
} WBButtonState;

/* Definition for our button */
typedef struct {
	GtkEventBox 	*eventbox;
	GtkImage 		*image;
	WBButtonState 	state;
} WindowButton;

/* Applet properties (things that get saved) */
typedef struct {
	gchar		*theme;					// Selected theme name [NULL = "Custom"]
	gchar		***images;				// Absolute paths to images
	gshort 		*eventboxposition;		// Position of eventboxes (left=0 to right=WB_BUTTONS-1)
										// The index represents the button [0=minimize,unmaximize,close]
										// Index of -1 means the button will not be put into box
	gshort		orientation;			// Orientation type (0=automatic, 1=horizontal, 2=vertical)
	gchar		*button_layout;			// Button layout string
	gboolean	*button_hidden;			// Indicates whether a button is hidden
	gboolean 	only_maximized,
				hide_on_unmaximized,
				use_metacity_layout,
				reverse_order,
				click_effect,
				hover_effect,
				show_tooltips;
} WBPreferences;

struct _WBApplet
{
	GpApplet parent;

	GSettings *settings;

	/* Widgets */
	GtkBox      	*box;				// Main container
	GtkWidget		*window_prefs;		// Preferences window

	/* Variables */
	WBPreferences	*prefs;				// Main preferences
	WindowButton	**button;			// Array of buttons
	WnckHandle *handle;
	WnckScreen 		*activescreen;		// Active screen

	gulong active_window_changed_id;
	gulong viewports_changed_id;
	gulong active_workspace_changed_id;
	gulong window_closed_id;
	gulong window_opened_id;

	WnckWorkspace	*activeworkspace;	// Active workspace
	WnckWindow		*umaxedwindow,		// Upper-most maximized window
					*activewindow,		// Active window
					*rootwindow;		// Root window (desktop)
	gulong			active_handler,		// activewindow's event handler ID
					umaxed_handler;		// umaxedwindow's event handler ID

	GtkOrientation orient;			// Panel orientation
	GtkPositionType position;
	GdkPixbufRotation angle;			// Applet angle
	GtkPackType		packtype;			// Packaging direction of buttons

	GdkPixbuf		***pixbufs;			// Images in memory

	/* GtkBuilder */
	GtkBuilder 		*prefbuilder;
};

void wb_applet_update_images (WBApplet *wbapplet);

G_END_DECLS

#endif
