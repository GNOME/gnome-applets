/*###################################################################*/
/*##                     Slash applet for GNOME                    ##*/
/*###################################################################*/

#include <sys/types.h>
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <gnome.h>
#include "applet-lib.h"
#include "applet-widget.h"

#define APPLET_VERSION_MAJ 0
#define APPLET_VERSION_MIN 3
#define APPLET_VERSION_REV 0

#define UPDATE_DELAY 70

typedef struct _InfoData InfoData;
struct _InfoData
{
	gchar *text;		/* any length, but be reasonable */
	gint length;
	gchar *icon_path;	/* location of icon */
	GtkWidget *icon;	/* maximum 24 x 24 please (or be reasonable)*/
	gint icon_w;
	gint icon_h;
	gint offset;		/* offset of text from left, set to line text up even with no icon */
	gint center;		/* Center text, only has effect if text fits on one line */
	gint shown;
	gint show_count;	/* when 0 show forever, other wise times to show (1 or +) */
	gint end_delay;		/* time to delay after displaying last character of text
				   this number is calculated from tenths of a second */
};

typedef struct _AppData AppData;
struct _AppData
{
	GtkWidget *applet;
	GtkWidget *frame;
	GtkWidget *draw_area;
	gint height;
	gint width;

	gint scroll_delay;
	gint scroll_speed;
	gint scroll_height;
	gint smooth_scroll;
	gint smooth_type;

	GtkWidget *display_w;
	GtkWidget *disp_buf_w;
	GtkWidget *background_w;
	GdkPixmap *display;
	GdkPixmap *disp_buf;
	GdkPixmap *background;

	GdkColor text_color;

	GList *text;		/* GList of InfoData structs */
	GList *current_text;	/* GList of current_text */
	gint text_lines;

	gint current_line;
	gint current_line_pos;

	gint scroll_count;
	gint scroll_pos;
	gint new_line;
	gint x_pos;
	gint text_pos;

	gint text_height;
	gint text_y_line;

	gint display_timeout_id;

	/* properties stuff */

	GtkWidget *propwindow;

	gint p_smooth_scroll;
	gint p_smooth_type;
	gint p_scroll_delay;
	gint p_scroll_speed;

	/* per app stuff (when writing an actual applet based on this, put new vars here) */

	gint headline_timeout_id;
	gint startup_timeout_id;
};

	/* display.c */
void free_all_info_lines(GList *list);
void add_info_line(AppData *ad, gchar *text, gchar *icon_path, gint offset, gint center,
		   gint show_count, gint delay);
void remove_info_line(AppData *ad, InfoData *id);
void remove_all_lines(AppData *ad);
void init_app_display(AppData *ad);

	/* properties.c */
void property_load(gchar *path, AppData *ad);
void property_save(gchar *path, AppData *ad);
void property_show(AppletWidget *applet, gpointer data);

