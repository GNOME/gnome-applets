/*
#
#   GNotes!
#   Copyright (C) 1998-1999  spoon@ix.netcom.com
#   Copyright (C) 1999 dres@debian.org
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef _GNOTES_APPLET_H_
#define _GNOTES_APPLET_H_

/* uncomment this to turn on debugging */
/* #define GNOTE_DEBUG (1) */

#ifdef __GNUC__
#ifdef GNOTE_DEBUG
#  define g_debug(format, args...) \
    g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "GNotes: "format, ##args)
#else
#  define g_debug(format, args...) 
#endif

#define g_critical(format, args...) \
  g_log(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "GNotes: "format, ##args)

#define g_info(format, args...) \
  g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "GNotes: "format, ##args)
#else /* !__GNUC__ */
static void
g_debug (const gchar *format,
	 ...)
{
#ifdef GNOTE_DEBUG
  va_list args;
  va_start (args, format);
  g_logv (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "GNotes: "format, args);
  va_end (args);
#endif
}
static void
g_critical (const gchar *format,
	    ...)
{
  va_list args;
  va_start (args, format);
  g_logv (G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "GNotes: "format, args);
  va_end (args);
}
static void
g_info (const gchar *format,
	...)
{
  va_list args;
  va_start (args, format);
  g_logv (G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "GNotes: "format, args);
  va_end (args);
}
#endif


#include <gnome.h>
#include <applet-widget.h>
#include "config.h"

/* various actions */
#define GNOTES_SAVE    "Save Notes"
#define GNOTES_LOAD    "Load Notes"
#define GNOTES_RAISE   "Raise Notes"
#define GNOTES_LOWER   "Lower Notes"
#define GNOTES_HIDE    "Hide Notes"
#define GNOTES_SHOW    "Show Notes"
#define GNOTE_RESIZE   "Resize Note"
#define GNOTE_HIDE     "Hide Note"
#define GNOTE_DELETE   "Delete Note"

#define GNOTE_FORMAT "#GNOTE-1"

#define GNOTES_DIR    ".gnome/gnotes.d"

typedef struct _gnotes_prefs {
    gint height;
    gint width;
    gint x;
    gint y;
    gboolean onbottom;
    gboolean sticky;
} gnotes_prefs;

struct _GNotes
{
    GtkWidget *applet;
    GtkWidget *pixmap;
    GtkWidget *button;
    gnotes_prefs defaults;
};

typedef struct _GNotes GNotes;

GNotes *gnotes_get_main_info(void);

void gnotes_init(void);

const gchar *get_gnotes_dir();

void properties_show(AppletWidget*, gpointer);
void gnotes_preferences_save(const char *, GNotes *);
void gnotes_preferences_load(const char *, GNotes *);
void gnotes_copy_defaults_to_defaults(gnotes_prefs *from, gnotes_prefs *to);

#endif /* _GNOTES_APPLET_H_ */

