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

#include "gtk/gtk.h"
#include "libgswitchit/gswitchit_applet_config.h"
#include "libgswitchit/gswitchit_plugin_manager.h"
#include "libgswitchit/gswitchit_util.h"

typedef struct _GSwitchItPluginsCapplet
{
  GSwitchItPluginContainer pluginContainer;
  GtkWidget *capplet;
  GSwitchItAppletConfig appletConfig;
  GSwitchItXkbConfig xkbConfig;
  GSwitchItPluginManager pluginManager;
}
GSwitchItPluginsCapplet;

#define NAME_COLUMN 0
#define FULLPATH_COLUMN 1

#define CappletGetGladeWidget( gswic, name ) \
  glade_xml_get_widget( \
    GLADE_XML( g_object_get_data( G_OBJECT( (gswic)->capplet ), \
                                  "gladeData" ) ), \
    name )

extern void CappletFillActivePluginList( GSwitchItPluginsCapplet * gswic );

extern char *CappletGetSelectedPluginPath( GtkTreeView * pluginsList,
                                           GSwitchItPluginsCapplet * gswic );

extern void CappletEnablePlugin( GtkWidget * btnAdd,
                                 GSwitchItPluginsCapplet * gswic );

#endif
