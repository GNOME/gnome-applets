/* tick-a-stat, a GNOME panel applet
 * (C) 1999 John Ellis
 *
 * Author: John Ellis
 *
 */

#include <sys/types.h>
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <gnome.h>
#include <applet-widget.h>

#define APPLET_VERSION_MAJ 0
#define APPLET_VERSION_MIN 2
#define APPLET_VERSION_REV 1

#define UPDATE_DELAY 70

typedef struct _AppData AppData;
typedef struct _ModuleData ModuleData;
typedef struct _InfoData InfoData;
typedef struct _ClickData ClickData;

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

/* now we just create the pixmaps ourselves?
	GtkWidget *display_w;
	GtkWidget *disp_buf_w;
	GtkWidget *background_w;
*/
	GdkPixmap *display;
	GdkPixmap *disp_buf;
	GdkPixmap *background;

	GdkColor text_color;
	gint text_height;		/* total pixel height of font's text */
	gint text_y_line;		/* descending pixel height of font's text */

	GList *info_list;		/* GList of InfoData structs */
	GList *click_list;		/* GList of clickable targets on display */

	gint free_current;		/* if TRUE, free info_current when done */
	InfoData *info_current;		/* current into line_info pointer */
	InfoData *info_next;		/* recommended next line_info pointer */
	gint text_pos;			/* position in line's text */
	gint x_pos;			/* pixel position in display of drawing text */
	gint y_pos;			/* pixel position in display height */

	gint scroll_count;
	gint scroll_pos;

	gint display_timeout_id;	/* main loop id */

	/* sizing */

	PanelOrientType orient;
	gint sizehint;
	gint width_hint;

	gint win_width;
	gint win_height;
	gint follow_hint_width;
	gint follow_hint_height;
	gint user_width;
	gint user_height;

	/* properties stuff */

	GtkWidget *propwindow;

	gint p_smooth_scroll;
	gint p_smooth_type;
	gint p_scroll_delay;
	gint p_scroll_speed;

	gint p_follow_hint_width;
	gint p_follow_hint_height;
	gint p_user_width;
	gint p_user_height;

	/* per app stuff (when writing an actual applet based on this, put new vars here) */
	GList *modules;		/* list of modules */
	ModuleData *prop_current_module;
	GtkWidget *prop_current_module_widget;
	GtkWidget *prop_module_pane;

	/* log file stuff */
	FILE *log_fd;
	gint log_was_written;
	gint log_close_timeout_id;
	gchar *log_path;
	gint enable_log;

	gint p_enable_log;
	GtkWidget *log_path_entry;
};

struct _ModuleData
{
	AppData *ad;
	gchar *title;
	gchar *description;
	void (*start_func)(gpointer data, AppData *ad);
	void (*config_init_func)(gpointer data, AppData *ad);
	GtkWidget *(*config_show_func)(gpointer data, AppData *ad);
	void (*config_hide_func)(gpointer data, AppData *ad);
	void (*config_apply_func)(gpointer data, AppData *ad);
	void (*config_close_func)(gpointer data, AppData *ad);
	void (*config_load_func)(gpointer data, AppData *ad);
	void (*config_save_func)(gpointer data, AppData *ad);
	void (*destroy_func)(gpointer data, AppData *ad);
	gpointer internal_data;
};

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
	gint priority;		/* priority, 1 - 10, highest is displayed first */

	/* click callback stuff */

	void (*click_func)(ModuleData *md, gpointer data, InfoData *id, AppData *ad);	/* func to call when clicked */
	gpointer data;					/* pointer to pass to above func */
	void (*free_func)(ModuleData *md, gpointer data);	/* func to free data */
	void (*pre_func)(ModuleData *md, gpointer data, InfoData *id, AppData *ad);	/* func to call before display */
	void (*end_func)(ModuleData *md, gpointer data, InfoData *id, AppData *ad);	/* func to call after display */

	ModuleData *module;
};

struct _ClickData
{
	InfoData *line_id;
	gint free_id;
	gint x;
	gint y;
	gint w;
	gint h;
	void (*click_func)(ModuleData *md, gpointer data, InfoData *id, AppData *ad);
};

	/* main.c */
void print_to_log(gchar *text, AppData *ad);
gchar *time_stamp(gint filename_format);

	/* display.c */
void free_all_info_lines(AppData *ad);
InfoData *add_info_line(AppData *ad, gchar *text, gchar *icon_path, gint offset, gint center,
		   gint show_count, gint delay, gint priority);
InfoData *add_info_line_with_pixmap(AppData *ad, gchar *text, GtkWidget *icon, gint offset, gint center,
		   gint show_count, gint delay, gint priority);
void remove_info_line(AppData *ad, InfoData *id);
void remove_all_lines(AppData *ad);
void set_info_signals(InfoData *id, ModuleData *md,
				    void (*click_func)(ModuleData *md, gpointer data, InfoData *id, AppData *ad),
				    void (*free_func)(ModuleData *md, gpointer data),
				    void (*pre_func)(ModuleData *md, gpointer data, InfoData *id, AppData *ad),
				    void (*end_func)(ModuleData *md, gpointer data, InfoData *id, AppData *ad),
				    gpointer data);
void set_mouse_cursor (AppData *ad, gint icon);
void resized_app_display(AppData *ad, gint force);
void init_app_display(AppData *ad);

	/* properties.c */
void property_load(gchar *path, AppData *ad);
void property_save(gchar *path, AppData *ad);
void property_changed(ModuleData *md, AppData *ad);
void property_show(AppletWidget *applet, gpointer data);

	/* modules.c */
GList *modules_build_list(AppData *ad);


