/*  cdplayer.h
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Authors: Miguel de Icaza
 *          Federico Mena
 * GNOME 2 Action:
 *          Chris Phelps
 */

#include <glib.h>
#include <gtk/gtk.h>
#include <panel-applet.h>
#include "cdrom-interface.h"

#ifndef __CDPLAYER_H__
#define __CDPLAYER_H__

G_BEGIN_DECLS

/* themeable size - "panel-menu" */
#define CDPLAYER_DEFAULT_ICON_SIZE 20 

typedef struct
{
    int timeout;
    gboolean played_by_applet;
    cdrom_device_t cdrom_device;
    int size;
    PanelAppletOrient orient;
    char *devpath;
    gchar *time_description;
    gchar *track_description;
    
    GtkWidget *about_dialog;
    GtkWidget *play_image, *pause_image, *current_image;
    GtkWidget *prefs_dialog;
    GtkWidget *error_io_dialog;
    GtkWidget *error_busy_dialog;
    
    struct
    {
        GtkWidget *applet;
        GtkWidget *frame;
        /* Main box for all bits changes depending on orientation,size,etc. */ 
        GtkWidget *box;
        /* Time display LED */
        GtkWidget *time;
        /* Box holding the track stuff */
        struct
        {
            GtkWidget *display;
            GtkWidget *prev;
            GtkWidget *next;
        }track_control; 
        /* Box holding the play controls */
        struct
        {
            GtkWidget *play_pause;
            GtkWidget *stop;
            GtkWidget *eject;
        }play_control;
    }panel;
}CDPlayerData;

G_END_DECLS

#endif
