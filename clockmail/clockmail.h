/* GNOME clock & mailcheck applet
 * (C) 2000 John Ellis
 *
 * Author: John Ellis
 *
 */

#include <sys/types.h>
#include <config.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <gnome.h>
#include <applet-widget.h>

#include <gdk-pixbuf/gdk-pixbuf.h>

#include "typedefs.h"

void launch_mail_reader(gpointer data);
void check_mail_file_status (int reset, AppData *ad);
void set_tooltip(struct tm *time_data, AppData *ad);

void reload_skin(AppData *ad);

void property_load(const gchar *path, AppData *ad);
void property_save(const gchar *path, AppData *ad);
void property_show(AppletWidget *applet, gpointer data);

