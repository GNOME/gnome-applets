/* Sticky Notes
 * Copyright (C) 2002-2003 Loban A Rahman
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifndef __STICKYNOTES_APPLET_H__
#define __STICKYNOTES_APPLET_H__

#include <gnome.h>
#include <panel-applet.h>
#include <gconf/gconf-client.h>
#include <glade/glade.h>

#define GCONF_PATH	"/apps/stickynotes_applet"
#define GLADE_PATH	STICKYNOTES_GLADEDIR "/stickynotes.glade"
#define XML_PATH	"/.gnome2/stickynotes_applet"
#define ICON_PATH	STICKYNOTES_ICONDIR

#define STICKYNOTES_STOCK_LOCK		"stickynotes-stock-lock"
#define STICKYNOTES_STOCK_UNLOCK	"stickynotes-stock-unlock"
#define STICKYNOTES_STOCK_CLOSE 	"stickynotes-stock-close"
#define STICKYNOTES_STOCK_RESIZE_SE 	"stickynotes-stock-resize-se"
#define STICKYNOTES_STOCK_RESIZE_SW	"stickynotes-stock-resize-sw"

/* Global Sticky Notes instance */
typedef struct
{
	GladeXML *about;		/* About dialog GladeXML*/
	GladeXML *prefs;		/* Preferences dialog GladeXML */

	GtkWidget *w_about;		/* The about dialog */
	
	GtkWidget *w_prefs;		/* The prefs dialog */
	GtkAdjustment *w_prefs_width;
	GtkAdjustment *w_prefs_height;
	GtkWidget *w_prefs_color;
	GtkWidget *w_prefs_system;
	GtkWidget *w_prefs_click;
	GtkWidget *w_prefs_sticky;
	GtkWidget *w_prefs_force;

	GList *notes;			/* Linked-List of all the sticky notes */
	GList *applets;			/* Linked-List of all the applets */
	
	GdkPixbuf *icon_normal;		/* Normal applet icon */
	GdkPixbuf *icon_prelight;	/* Prelighted applet icon */

	GConfClient *gconf;		/* GConf Client */
	GtkTooltips *tooltips;		/* Tooltips */

} StickyNotes;

/* Sticky Notes Icons */
typedef struct
{
	gchar *stock_id;
	gchar *filename;

} StickyNotesStockIcon;

/* Sticky Notes Applet */
typedef struct
{
	GtkWidget *w_applet;		/* The applet */
	GtkWidget *w_image;		/* The applet icon */

	gboolean prelighted;		/* Whether applet is prelighted */
	gboolean pressed;		/* Whether applet is pressed */
	
} StickyNotesApplet;
	
typedef enum
{
	STICKYNOTES_NEW = 0,
	STICKYNOTES_SET_VISIBLE,
	STICKYNOTES_SET_LOCKED

} StickyNotesDefaultAction;

extern StickyNotes *stickynotes;

void stickynotes_applet_init();
void stickynotes_applet_init_icons();
void stickynotes_applet_init_about();
void stickynotes_applet_init_prefs();

StickyNotesApplet * stickynotes_applet_new(PanelApplet *panel_applet);

void stickynotes_applet_update_icon(StickyNotesApplet *applet);
void stickynotes_applet_update_prefs();
void stickynotes_applet_update_menus();
void stickynotes_applet_update_tooltips();

void stickynotes_applet_do_default_action();

#endif /* __STICKYNOTES_APPLET_H__ */
