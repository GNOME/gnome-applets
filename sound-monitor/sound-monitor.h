/* GNOME Esound Monitor Control applet
 * (C) 1999 John Ellis
 *
 * Author: John Ellis
 *
 */

#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include <gnome.h>
#include <applet-widget.h>
#include <esd.h>

#define VUMETER_APPLET_VERSION_MAJ 0
#define VUMETER_APPLET_VERSION_MIN 7
#define VUMETER_APPLET_VERSION_REV 0

#define PEAK_MODE_OFF 0
#define PEAK_MODE_ACTIVE 1
#define PEAK_MODE_SMOOTH 2

#define ESD_STATUS_ERROR 0
#define ESD_STATUS_STANDBY 1
#define ESD_STATUS_AUTOSTANDBY 2
#define ESD_STATUS_READY 3

#define ESD_CONTROL_START 0
#define ESD_CONTROL_STANDBY 1
#define ESD_CONTROL_RESUME 2

/* these two defines should be in a esdcalls.h file... */
#define RATE   44100
/* A fairly good size buffer to keep resource (cpu) down */
#define NSAMP  2048

typedef struct _ItemData ItemData;
struct _ItemData
{
        GdkPixmap *pixmap;
	gint sections;
	gint current_section;
	gint width;
	gint height;
	gint x;
	gint y;
};

typedef struct _VuData VuData;
struct _VuData
{
        GdkPixmap *pixmap;
	gint x;
	gint y;
	gint width;
	gint height;
	gint sections;
	gint section_height;
	gint horizontal;

	gint value;
	gint peak;
	gint peak_fall;
	gint mode;

	gint old_val;
	gint old_peak;
};

typedef struct _ScopeData ScopeData;
struct _ScopeData
{
        GdkPixmap *pixmap;
	GdkBitmap *mask;
	GdkGC *mask_gc;

	GdkColor black;
	GdkColor white;

	gint x;
	gint y;
	gint width;
	gint height;
	gint res;
	gint horizontal;

	gint *pts;
	gint scale;
};

typedef struct _SkinData SkinData;
struct _SkinData
{
	gint width;
	gint height;
	GtkWidget *pixmap;
	GdkPixmap *background;
	GdkBitmap *mask;
	ItemData *status;
	VuData *vu_left;
	VuData *vu_right;
	ItemData *meter_left;
	ItemData *meter_right;
	ScopeData *scope;
};

typedef struct _AppData AppData;
struct _AppData
{
	GtkWidget *applet;
	GtkWidget *display_area;
	GtkTooltips *tooltips;
	PanelOrientType orient;
	gint update_timeout_id;

	gchar *theme_file;
	gint refresh_fps;

	gint esd_status;
	gint esd_status_menu;

	gint peak_mode;
	gint falloff_speed;

	gint scope_scale;

	/* sound oriented data */
	gint sound_fd;
	gint sound_input_cb_id;
	short aubuf[NSAMP];
	gint vu_l;
	gint vu_r;
	gint new_vu_data;

	gchar *esd_host;
	gint monitor_input;

	gint prev_vu_changed;

	gint no_data_check_count;
	gint esd_status_check_count;
	gint scope_flat;
	gint draw_scope_as_segments;

	/* the properties window widgets */
	GtkWidget *propwindow;
	gint p_refresh_fps;
	gint p_peak_mode;
	gint p_falloff_speed;
	gint p_scope_scale;
	gint p_draw_scope_as_segments;
	GtkWidget *theme_entry;
	gint p_monitor_input;
	GtkWidget *host_entry;

	SkinData *skin;
	SkinData *skin_v;
	SkinData *skin_h;

	gpointer manager;
};

void reset_fps_timeout(AppData *ad);
void set_widget_modes(AppData *ad);

void property_load(gchar *path, AppData *ad);
void property_save(gchar *path, AppData *ad);
void property_show(AppletWidget *applet, gpointer data);

void sync_window_to_skin(AppData *ad);
void free_skin(SkinData *s);
void redraw_skin(AppData *ad);
gint draw_item(ItemData *item, gint section, AppData *ad);
gint draw_item_by_percent(ItemData *item, gint percent, AppData *ad);
gint draw_vu_item(VuData *item, gint value, AppData *ad);
void set_vu_item_mode(VuData *item, gint mode, gint falloff);
void draw_scope_item(ScopeData *item, AppData *ad, gint flat);
void set_scope_item_scale(ScopeData *item, gint scale);
gint change_to_skin(gchar *path, AppData *ad);

gint init_sound(AppData *ad);
void stop_sound(AppData *ad);
void esd_sound_control(gint function, AppData *ad);
gint esd_check_status(AppData *ad);

void manager_window_show(AppData *ad);
void manager_window_close(AppData *ad);


