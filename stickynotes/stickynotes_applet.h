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

#define GCONF_PATH "/apps/stickynotes_applet"
#define GLADE_PATH STICKYNOTES_GLADEDIR "/stickynotes.glade"
#define XML_PATH "stickynotes_applet"

typedef struct
{
	GtkWidget *applet;		/* The applet */
	GladeXML *about;		/* About dialog GladeXML*/
	GladeXML *prefs;		/* Preferences dialog GladeXML */
	
	gboolean pressed;		/* Whether applet is pressed */

	GList *notes;			/* Linked-List of all the sticky notes */
	
	GdkPixbuf *pixbuf_normal;	/* Pixbuf for normal applet */
	GdkPixbuf *pixbuf_prelight;	/* Pixbuf for prelighted applet */
	GtkWidget *image;		/* Generated Image for applet */
	
	GConfClient *gconf_client;	/* GConf Client */
	GtkTooltips *tooltips;		/* Tooltips */

} StickyNotesApplet;

typedef enum
{
	STICKYNOTES_NEW = 0,
	STICKYNOTES_SET_VISIBLE,
	STICKYNOTES_SET_LOCKED

} StickyNotesDefaultAction;

/* Modify the applet */
void stickynotes_applet_update_icon(StickyNotesApplet *stickynotes, gboolean highlighted);
void stickynotes_applet_update_tooltips(StickyNotesApplet *stickynotes);

void stickynotes_applet_do_default_action(StickyNotesApplet *stickynotes);

void stickynotes_applet_create_preferences(StickyNotesApplet *stickynotes);
void stickynotes_applet_create_about(StickyNotesApplet *stickynotes);

#endif /* __STICKYNOTES_APPLET_H__ */
