/* GNOME sound-monitor applet
 * (C) 2000 John Ellis
 *
 * Author: John Ellis
 *
 */


#ifndef TYPEDEFS_H
#define TYPEDEFS_H

/* input rate */
#define RATE   44100

/* buffer size */
#define NSAMP  2048

typedef struct _SoundData SoundData;
struct _SoundData
{
	gint fd;
	gint input_cb_id;
	short buffer[NSAMP];
	gint left;
	gint right;
	gint new_data;
};


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
	gint old_val;
};

typedef enum {
	PeakMode_Off,
	PeakMode_Active,
	PeakMode_Smooth
} PeakModeType;

typedef struct _MeterData MeterData;
struct _MeterData
{
        GdkPixbuf *pixbuf;
        GdkPixbuf *overlay;
	gint x;
	gint y;
	gint width;
	gint height;
	gint sections;
	gint section_height;
	gint horizontal;

	gint value;
	gfloat peak;
	gfloat peak_fall;
	PeakModeType mode;

	gint old_val;
	gfloat old_peak;
};

typedef enum {
	ScopeType_Points,
	ScopeType_Lines
} ScopeType;

typedef struct _ScopeData ScopeData;
struct _ScopeData
{
	GdkPixbuf *pixbuf;
	GdkPixbuf *overlay;

	gint x;
	gint y;
	gint width;
	gint height;
	gint horizontal;
	gint sections;

	ScopeType type;

	gint scale;
	gint *pts;
	gint pts_len;
};

typedef struct _AnalyzerData AnalyzerData;
struct _AnalyzerData
{
	GdkPixbuf *pixbuf;
	GdkPixbuf *overlay;

	gint x;
	gint y;
	gint width;
	gint height;

	gint horizontal;
	gint flip;

	gint decay_speed;

	gint peak_enable;
	gint peak_decay_speed;

	gint band_count;
	float *bands;
	float *peaks;

	gint rows;

	gint led_width;
	gint led_height;
};

typedef struct _SkinData SkinData;
struct _SkinData
{
	GdkPixbuf *pixbuf;
	GdkPixbuf *overlay;

	gint width;
	gint height;

	ItemData *status;
	MeterData *meter_left;
	MeterData *meter_right;
	ItemData *item_left;
	ItemData *item_right;
	ScopeData *scope;
	AnalyzerData *analyzer;
};

enum {
	SIZEHINT_TINY,
	SIZEHINT_SMALL,
	SIZEHINT_STANDARD,
	SIZEHINT_LARGE,
	SIZEHINT_HUGE
};

typedef enum {
	Status_Error,
	Status_Standby,
	Status_AutoStandby,
	Status_Ready
} StatusType;

typedef enum {
	Control_Start,
	Control_Standby,
	Control_Resume
} ControlType;

typedef struct _AppData AppData;
struct _AppData
{
	GtkWidget *applet;
	GtkWidget *display;

	PanelOrientType orient;
	gint sizehint;
	gint update_timeout_id;

	gchar *theme_file;
	gint refresh_fps;

	StatusType esd_status;
	StatusType esd_status_menu;

	PeakModeType peak_mode;
	gint falloff_speed;

	gint scope_scale;

	SoundData *sound;

	gchar *esd_host;
	gint monitor_input;

	gint prev_vu_changed;

	gint no_data_count;
	gint status_check_count;
	gint scope_flat;
	gint draw_scope_as_segments;

	/* the properties window widgets */
	GtkWidget *propwindow;
	gint p_refresh_fps;
	PeakModeType p_peak_mode;
	gint p_falloff_speed;
	gint p_scope_scale;
	gint p_draw_scope_as_segments;
	GtkWidget *theme_entry;
	gint p_monitor_input;
	GtkWidget *host_entry;

	SkinData *skin;

	gpointer manager;
};

#endif

