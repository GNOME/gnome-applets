/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __GSWITCHIT_APPLET_H__
#define __GSWITCHIT_APPLET_H__

#include <panel-applet.h>

#include "libgswitchit/gswitchit_applet_config.h"
#include "libgswitchit/gswitchit_plugin_manager.h"

typedef struct _GSwitchItApplet {
	GSwitchItPluginContainer pluginContainer;

	GSwitchItAppletConfig appletConfig;
	GSwitchItXkbConfig xkbConfig;
	GSwitchItPluginManager pluginManager;

	GtkWidget *applet;
	GtkWidget *notebook;
	GtkWidget *ebox;
	GtkWidget *aboutDialog;
	GtkWidget *propsDialog;

	GroupDescriptionsBuffer groupNames;
} GSwitchItApplet;

extern void GSwitchItAppletRevalidate (GSwitchItApplet * sia);
extern void GSwitchItAppletRevalidateGroup (GSwitchItApplet * sia,
					    int group);

extern void GSwitchItAppletReinitUi (GSwitchItApplet * sia);

extern GdkFilterReturn GSwitchItAppletFilterXEvt (GdkXEvent * xevent,
						  GdkEvent * event,
						  GSwitchItApplet * sia);

extern void GSwitchItAppletPropsCreate (GSwitchItApplet * sia);

extern gboolean GSwitchItAppletNew (PanelApplet * applet);


#endif
