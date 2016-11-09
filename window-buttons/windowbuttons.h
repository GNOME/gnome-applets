/***************************************************************************
 *            windowbuttons.h
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

#define PLAINTEXT_CONFIG				0

#ifndef __WB_APPLET_H__
#define __WB_APPLET_H__

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
#include <glib-object.h>
#include <glib/gi18n.h>
#include <gconf/gconf-client.h>
#include <panel-applet.h>
#include <panel-applet-gconf.h>
#include <gtk/gtk.h>

#if PLAINTEXT_CONFIG == 1
#include <glib/gstdio.h>
#endif

#ifndef WNCK_I_KNOW_THIS_IS_UNSTABLE
#define WNCK_I_KNOW_THIS_IS_UNSTABLE
#endif
#include <libwnck/libwnck.h>

/* static paths and stuff */
#define APPLET_NAME						"Window Buttons"
#define APPLET_OAFIID					"WindowButtonsApplet"
#define APPLET_OAFIID_FACTORY			"WindowButtonsAppletFactory"
#define PATH_BUILDER 					"/usr/share/gnome-applets/builder"
#define PATH_MAIN 						"/usr/share"
#define PATH_THEMES 					PATH_MAIN"/pixmaps/windowbuttons/themes"
#define PATH_UI_PREFS					PATH_MAIN"/windowbuttons/windowbuttons.ui"
#define PATH_LOGO						PATH_MAIN"/pixmaps/windowbuttons-applet.png"
#define METACITY_XML 					"metacity-theme-1.xml"
#define THEME_EXTENSION					"png"
#define GCONF_PREFS 					"/schemas/apps/windowbuttons-applet/prefs"
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

/* Key strings (used by GConf, Plaintext and GtkBuilder .ui file) */
#define CFG_ONLY_MAXIMIZED			"only_maximized"
#define CFG_HIDE_ON_UNMAXIMIZED 	"hide_on_unmaximized"
#define CFG_HIDE_DECORATION			"hide_decoration"
#define CFG_CLICK_EFFECT			"click_effect"
#define CFG_HOVER_EFFECT			"hover_effect"
#define CFG_USE_METACITY_LAYOUT		"use_metacity_layout"
#define CFG_MINIMIZE_HIDDEN			"button_minimize_hidden"
#define CFG_UNMAXIMIZE_HIDDEN		"button_maximize_hidden"
#define CFG_CLOSE_HIDDEN			"button_close_hidden"
#define CFG_BUTTON_LAYOUT			"button_layout"
#define CFG_REVERSE_ORDER			"reverse_order"
#define CFG_ORIENTATION				"orientation"
#define CFG_THEME					"theme"
#define CFG_SHOW_TOOLTIPS			"show_tooltips"

G_BEGIN_DECLS

#define WB_TYPE_APPLET                wb_applet_get_type()
#define WB_APPLET(obj)                (G_TYPE_CHECK_INSTANCE_CAST ((obj), WB_TYPE_APPLET, WBApplet))
#define WB_APPLET_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST ((klass), WB_TYPE_APPLET, WBAppletClass))
#define WB_IS_APPLET(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), WB_TYPE_APPLET))
#define WB_IS_APPLET_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass), WB_TYPE_APPLET))
#define WB_APPLET_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), WB_TYPE_APPLET, WBAppletClass))

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

/* WBApplet definition (inherits from PanelApplet) */
typedef struct {
    PanelApplet		*applet;			// The actual PanelApplet
	
	/* Widgets */
	GtkBox      	*box;				// Main container
	GtkWidget		*window_prefs;		// Preferences window
	
	/* Variables */
	WBPreferences	*prefs;				// Main preferences
	WindowButton	**button;			// Array of buttons
	WnckScreen 		*activescreen;		// Active screen
	WnckWorkspace	*activeworkspace;	// Active workspace
	WnckWindow		*umaxedwindow,		// Upper-most maximized window
					*activewindow,		// Active window
					*rootwindow;		// Root window (desktop)
	gulong			active_handler,		// activewindow's event handler ID
					umaxed_handler;		// umaxedwindow's event handler ID
	
	PanelAppletOrient orient;			// Panel orientation
	GdkPixbufRotation angle;			// Applet angle
	GtkPackType		packtype;			// Packaging direction of buttons
	
	GdkPixbuf		***pixbufs;			// Images in memory
	
	/* GtkBuilder */
	GtkBuilder 		*prefbuilder;
} WBApplet;

typedef struct {
        PanelAppletClass applet_class;
} WBAppletClass;

GType wb_applet_get_type (void);
WBApplet* wb_applet_new (void);

G_END_DECLS
#endif

/*
Applet structure:

              Panel
                |
                | 
              Applet
                |
                |
     _________ Box _________
    |           |           |
    |           |           | 
EventBox[0] EventBox[1] EventBox[2]
    |           |           |
    |           |           | 
  Image[0]   Image[1]    Image[2]

* note that EventBox/Image pairs (buttons) may be positioned in a different order

*/
