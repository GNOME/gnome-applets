/*###################################################################*/
/*##                         clock & mail applet 0.1.5             ##*/
/*###################################################################*/

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
#include "applet-lib.h"
#include "applet-widget.h"

#define CLOCKMAIL_APPLET_VERSION_MAJ 0
#define CLOCKMAIL_APPLET_VERSION_MIN 1
#define CLOCKMAIL_APPLET_VERSION_REV 5

typedef struct _AppData AppData;
struct _AppData
{
	gint blink_delay;
	gint blink_times;
	gint am_pm_enable;
	gint always_blink;
	gchar *mail_file;
	gchar *newmail_exec_cmd;
	gint exec_cmd_on_newmail;
	GtkWidget *applet;
	GtkWidget *frame;
	GtkWidget *display_area;
	GtkTooltips *tooltips;
	GdkPixmap *display;
	GdkPixmap *display_back;
	GdkPixmap *digmed;
	GdkPixmap *mailpics;
	gint update_timeout_id;
	gint blink_timeout_id;
	gint anymail;
	gint newmail;
	gint unreadmail;
	gint mailcleared;
	gint blinking;

	/* the properties window widgets */
	GtkWidget *propwindow;
	GtkWidget *mail_file_entry;
	GtkWidget *newmail_exec_cmd_entry;
	gint p_am_pm_enable;
	gint p_always_blink;
	gint p_exec_cmd_on_newmail;

	/* variables for mail status and remebering past states */
	off_t oldsize;
	time_t oldtime;
	gchar *tiptext;
	gint old_n;
	gint blink_lit;
	gint blink_count;
};

void check_mail_file_status (int reset, AppData *ad);

void property_load(gchar *path, AppData *ad);
void property_save(gchar *path, AppData *ad);
void property_show(AppletWidget *applet, gpointer data);

