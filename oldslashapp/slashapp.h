/*###################################################################*/
/*##                     Slash applet for GNOME                    ##*/
/*###################################################################*/

#include <sys/types.h>
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <gnome.h>
#include "applet-lib.h"
#include "applet-widget.h"

#define APPLET_VERSION_MAJ 0
#define APPLET_VERSION_MIN 3
#define APPLET_VERSION_REV 2

#define UPDATE_DELAY 70

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

	GList *click_list;		/* GList of clickable target on display */

	/* properties stuff */

	GtkWidget *propwindow;

	gint p_smooth_scroll;
	gint p_smooth_type;
	gint p_scroll_delay;
	gint p_scroll_speed;

	/* per app stuff (when writing an actual applet based on this, put new vars here) */

	gint headline_timeout_id;
	gint startup_timeout_id;

	gchar *slashapp_dir;

	gint show_images;
	gint show_info;
	gint show_department;
	gint p_show_images;
	gint p_show_info;
	gint p_show_department;

	gint new_browser_window;
	gint p_new_browser_window;

	gint article_delay;
	gint p_article_delay;

	GtkWidget *article_window;
	GtkWidget *article_list;
};

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

	/* click callback stuff */

	void (*click_func)(AppData *ad, gpointer data);
	gpointer data;
	void (*free_func)(gpointer data);
};

typedef struct _ClickData ClickData;
struct _ClickData
{
	InfoData *line_id;
	gint x;
	gint y;
	gint w;
	gint h;
	void (*click_func)(AppData *ad, gpointer data);
	gpointer data;
	
};

	/* display.c */
void free_all_info_lines(AppData *ad);
InfoData *add_info_line(AppData *ad, gchar *text, gchar *icon_path, gint offset, gint center,
		   gint show_count, gint delay);
InfoData *add_info_line_with_pixmap(AppData *ad, gchar *text, GtkWidget *icon, gint offset, gint center,
		   gint show_count, gint delay);
void remove_info_line(AppData *ad, InfoData *id);
void remove_all_lines(AppData *ad);
void set_info_click_signal(InfoData *id, void (*click_func)(AppData *ad, gpointer data),
		gpointer data, void (*free_func)(gpointer data));
void set_mouse_cursor (AppData *ad, gint icon);
void init_app_display(AppData *ad);

	/* properties.c */
void property_load(gchar *path, AppData *ad);
void property_save(gchar *path, AppData *ad);
void property_show(AppletWidget *applet, gpointer data);

