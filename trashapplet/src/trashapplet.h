/* trashapplet.h
 *
 * Copyright (c) 2004  Michiel Sikkes <michiel@eyesopened.nl>,
 *               2004  Emmanuele Bassi <ebassi@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License VERSION 2 as
 * published by the Free Software Foundation.  You are not allowed to
 * use any other version of the license; unless you got the explicit  
 * permission from the author to do so.  
 * 
 * This program is distributed in the hope that it will be useful,  
 * but WITHOUT ANY WARRANTY; without even the implied warranty of  
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the  
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License  
 * along with this program; if not, write to the Free Software  
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __TRASH_APPLET_H__
#define __TRASH_APPLET_H__

#include <panel-applet.h>
#include <gnome.h>
#include <libgnomevfs/gnome-vfs.h>
#include "trash-monitor.h"

#define TRASH_ICON_EMPTY	"gnome-fs-trash-empty"
#define TRASH_ICON_EMPTY_ACCEPT "gnome-fs-trash-empty-accept"
#define TRASH_ICON_FULL		"gnome-fs-trash-full"

typedef enum {
	TRASH_STATE_UNKNOWN,
	TRASH_STATE_EMPTY,
	TRASH_STATE_FULL,
	TRASH_STATE_ACCEPT
} TrashState;

typedef struct _TrashApplet	TrashApplet;
struct _TrashApplet
{
	PanelApplet *applet;
	PanelAppletOrient orient;

	GtkIconTheme *icon_theme;
	TrashState icon_state;
	GdkPixbuf *icon;
	GtkWidget *image;
	GtkTooltips *tooltips;

	guint size;
	gint item_count;
	gboolean is_empty;
	gboolean drag_hover;

	TrashMonitor *monitor;
	guint monitor_signal_id;
};

#endif /* __TRASH_APPLET_H__ */
