/* GNOME sound-monitor applet
 * (C) 2000 John Ellis
 *
 * Author: John Ellis
 *
 */

#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include <gnome.h>
#include <applet-widget.h>

#include <gdk-pixbuf/gdk-pixbuf.h>

#include <esd.h>

#include "typedefs.h"

void sync_esd_menu_item(AppData *ad);
void reload_skin(AppData *ad);

void property_load(const gchar *path, AppData *ad);
void property_save(const gchar *path, AppData *ad);
void property_show(AppletWidget *applet, gpointer data);


