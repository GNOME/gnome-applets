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

#include <gnome.h>
#include <panel-applet.h>
#include <gconf/gconf-client.h>

#ifndef __STICKYNOTES_APPLET_H__
#define __STICKYNOTES_APPLET_H__

#define GCONF_PATH "/apps/stickynotes_applet"
#define GLADE_PATH STICKYNOTES_GLADEDIR "/stickynotes_applet.glade"

typedef struct
{
	GtkWidget *applet;		/* The applet */
	
	gint size;			/* Panel size */
	gboolean pressed;		/* Whether applet is pressed */

	GList *notes;			/* Linked-List of all the sticky notes */
	gboolean hidden;		/* Whether sticky notes are hidden */
	
	GdkPixbuf *pixbuf_normal;	/* Pixbuf for normal applet */
	GdkPixbuf *pixbuf_prelight;	/* Pixbuf for prelighted applet */
	GtkWidget *image;		/* Generated Image for applet */
	
	GConfClient *gconf_client;	/* GConf Client */
	GtkTooltips *tooltips;		/* Tooltips */
} StickyNotesApplet;

/* Prelight the applet */
void stickynotes_applet_highlight(PanelApplet *applet, gboolean highlight);

/* Sticky Notes Applet settings instance */
extern StickyNotesApplet *stickynotes;

#endif /* __STICKYNOTES_APPLET_H__ */
