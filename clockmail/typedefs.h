/* GNOME clock & mailcheck applet
 * (C) 2000 John Ellis
 *
 * Author: John Ellis
 *
 */

#ifndef TYPEDEFS_H
#define TYPEDEFS_H

/* all the structures and enums, etc. */

typedef struct _ItemData ItemData;
struct _ItemData
{
	GdkPixbuf *pixbuf;
	GdkPixbuf *overlay;
	gint sections;
	gint width;
	gint height;
	gint x;
	gint y;
};

typedef struct _DigitData DigitData;
struct _DigitData
{
	GdkPixbuf *overlay;
	gint width;
	gint height;
};

typedef struct _NumberData NumberData;
struct _NumberData
{
	GdkPixbuf *pixbuf;
	DigitData *digits;
	gint x;
	gint y;
	gint length;
	gint zeros;
	gint centered;
};

typedef struct _HandData HandData;
struct _HandData
{
	GdkPixbuf *pixbuf;
	gint xo;
	gint yo;
};

typedef struct _ClockData ClockData;
struct _ClockData
{
	HandData *hour;
	HandData *minute;
	HandData *second;

	GdkPixbuf *pixbuf;
	GdkPixbuf *overlay;
	gint width;
	gint height;
	gint x;
	gint y;
	gint cx;
	gint cy;
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
	gpointer click_data;
	void (*redraw_func)(gpointer);
	gpointer redraw_data;
};

typedef struct _SkinData SkinData;
struct _SkinData
{
	GdkPixbuf *pixbuf;
	GdkPixbuf *overlay;

	gint width;
	gint height;

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

	ClockData *clock;
};

enum {
	SIZEHINT_TINY,
	SIZEHINT_SMALL,
	SIZEHINT_STANDARD,
	SIZEHINT_LARGE,
	SIZEHINT_HUGE
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
	GtkWidget *display;
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

	ButtonData *active;
};

#endif

