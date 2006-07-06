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

#ifndef __GSWITCHIT_PLUGINS_H__
#define __GSWITCHIT_PLUGINS_H__

#include <glib-object.h>
#include <gtk/gtkwidget.h>
#include <glade/glade-xml.h>

#include "libgswitchit/gswitchit-config.h"
#include "libgswitchit/gswitchit-plugin-manager.h"
#include "libgswitchit/gswitchit-util.h"

typedef struct _GSwitchItPluginsCapplet {
	GSwitchItPluginContainer plugin_container;
	GtkWidget *capplet;
	GSwitchItConfig cfg;
	GSwitchItAppletConfig applet_cfg;
	GSwitchItKbdConfig kbd_cfg;
	GSwitchItPluginManager plugin_manager;
	XklEngine *engine;
	XklConfigRegistry *config_registry;
} GSwitchItPluginsCapplet;

#define NAME_COLUMN 0
#define FULLPATH_COLUMN 1

#define CappletGetGladeWidget( gswic, name ) \
  glade_xml_get_widget( \
    GLADE_XML( g_object_get_data( G_OBJECT( (gswic)->capplet ), \
                                  "gladeData" ) ), \
    name )

extern void CappletFillActivePluginList (GSwitchItPluginsCapplet * gswic);

extern char *CappletGetSelectedPluginPath (GtkTreeView * plugins_list,
					   GSwitchItPluginsCapplet *
					   gswic);

extern void CappletEnablePlugin (GtkWidget * btnAdd,
				 GSwitchItPluginsCapplet * gswic);

#endif
