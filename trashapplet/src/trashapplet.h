/* -*- mode: c; c-basic-offset: 8 -*-
 * trashapplet.h
 *
 * Copyright (c) 2004  Michiel Sikkes <michiel@eyesopened.nl>,
 *               2004  Emmanuele Bassi <ebassi@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
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
#include <glade/glade.h>
#include <libgnomevfs/gnome-vfs.h>
#include "trash-monitor.h"

#define TRASH_TYPE_APPLET (trash_applet_get_type ())
#define TRASH_APPLET(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), TRASH_TYPE_APPLET, TrashApplet))
#define TRASH_APPLET_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), TRASH_TYPE_APPLET, TrashAppletClass))
#define TRASH_IS_APPLET(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TRASH_TYPE_APPLET))
#define TRASH_IS_APPLET_CLASS(obj) (G_TYPE_CHECK_CLASS_TYPE ((obj), TRASH_TYPE_APPLET))
#define TRASH_APPLET_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), TRASH_TYPE_APPLET, TrashAppletClass))

#define TRASH_ICON_EMPTY	"gnome-fs-trash-empty"
#define TRASH_ICON_EMPTY_ACCEPT "gnome-fs-trash-empty-accept"
#define TRASH_ICON_FULL		"gnome-fs-trash-full"

typedef enum {
	TRASH_STATE_UNKNOWN,
	TRASH_STATE_EMPTY,
	TRASH_STATE_FULL,
	TRASH_STATE_ACCEPT
} TrashState;

typedef struct _TrashApplet	 TrashApplet;
typedef struct _TrashAppletClass TrashAppletClass;
struct _TrashApplet
{
	PanelApplet applet;

	guint size;
	guint new_size;
	PanelAppletOrient orient;

	GtkTooltips *tooltips;
	GtkWidget *image;
	TrashState icon_state;

	gint item_count;
	gboolean is_empty;
	gboolean drag_hover;

	GladeXML *xml;

	TrashMonitor *monitor;
	guint monitor_signal_id;

	guint update_id;
};
struct _TrashAppletClass {
	PanelAppletClass parent_class;
};

#endif /* __TRASH_APPLET_H__ */
