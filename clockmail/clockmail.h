/* GNOME clock & mailcheck applet
 * (C) 1999 John Ellis
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

#define CLOCKMAIL_APPLET_VERSION_MAJ 1
#define CLOCKMAIL_APPLET_VERSION_MIN 1
#define CLOCKMAIL_APPLET_VERSION_REV 0

enum {
	SIZEHINT_TINY,
	SIZEHINT_STANDARD,
	SIZEHINT_LARGE,
	SIZEHINT_HUGE
};

typedef struct _ItemData ItemData;
struct _ItemData
{
        GdkPixmap *pixmap;
	gint sections;
	gint width;
	gint height;
	gint x;
	gint y;
};

typedef struct _DigitData DigitData;
struct _DigitData
{
        GdkPixmap *pixmap;
	gint width;
	gint height;
};

typedef struct _NumberData NumberData;
struct _NumberData
{
	DigitData *digits;
	gint x;
	gint y;
	gint length;
	gint zeros;
};

typedef struct _ButtonData ButtonData;
struct _ButtonData
{
	ItemData *item;
	gint has_prelight;
	gint pushed;
	gint prelit;
	gint width;
	gint height;
	gint x;
	gint y;
	void (*click_func)(gpointer);
	void (*redraw_func)(gpointer);
};

typedef struct _SkinData SkinData;
struct _SkinData
{
	gint width;
	gint height;
	GtkWidget *pixmap;
	GdkPixmap *background;
	GdkBitmap *mask;
	ItemData *mail;
	ItemData *month_txt;
	ItemData *week_txt;
	ItemData *mail_amount;
	DigitData *dig_small;
	DigitData *dig_large;
	NumberData *hour;
	NumberData *min;
	NumberData *sec;
	NumberData *month;
	NumberData *day;
	NumberData *year;
	NumberData *mail_count;
	NumberData *messages;
	ItemData *button_pix;
	ButtonData *button;
};

typedef struct _AppData AppData;
struct _AppData
{
	gint blink_delay;
	gint blink_times;
	gint am_pm_enable;
	gint always_blink;
	gchar *mail_file;
	gchar *reader_exec_cmd;
	gchar *newmail_exec_cmd;
	gint exec_cmd_on_newmail;
	gchar *theme_file;
	GtkWidget *applet;
	GtkWidget *display_area;
	GtkTooltips *tooltips;
	PanelOrientType orient;
	gint sizehint;
	gint update_timeout_id;
	gint blink_timeout_id;
	gint anymail;
	gint newmail;
	gint unreadmail;
	gint mailcleared;
	gint blinking;
	gint mail_sections;

	gint use_gmt;
	gint gmt_offset;

	off_t mailsize;
	gint mail_max;

	gint message_count;

	/* the properties window widgets */
	GtkWidget *propwindow;
	GtkWidget *mail_file_entry;
	GtkWidget *reader_exec_cmd_entry;
	GtkWidget *newmail_exec_cmd_entry;
	gint p_am_pm_enable;
	gint p_always_blink;
	gint p_exec_cmd_on_newmail;
	gint p_use_gmt;
	gint p_gmt_offset;
	gint p_mail_max;
	GtkWidget *theme_entry;

	/* variables for mail status and remebering past states */
	time_t oldtime;
	gint old_yday;
	gint old_n;
	gint blink_lit;
	gint blink_count;
	gint old_week;
	gint old_amount;

	SkinData *skin;
	SkinData *skin_v;
	SkinData *skin_h;

	ButtonData *active;
};

void launch_mail_reader(gpointer data);
void check_mail_file_status (int reset, AppData *ad);
void redraw_all(gpointer data);
void reload_skin(AppData *ad);

void property_load(gchar *path, AppData *ad);
void property_save(gchar *path, AppData *ad);
void property_show(AppletWidget *applet, gpointer data);

void sync_window_to_skin(AppData *ad);
void free_skin(SkinData *s);
void redraw_skin(AppData *ad);
void draw_number(NumberData *number, gint n, AppData *ad);
void draw_item(ItemData *item, gint section, AppData *ad);
void draw_button(ButtonData *button, gint prelight, gint pressed, gint force, AppData *ad);
void skin_event_init(AppData *ad);
gint change_to_skin(gchar *path, AppData *ad);

