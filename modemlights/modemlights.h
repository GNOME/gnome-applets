/* GNOME modemlights applet
 * (C) 2000 John Ellis
 *
 * Authors: John Ellis
 *          Martin Baulig
 *
 */

#include <panel-applet.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <time.h>
#include <net/if.h>

#ifdef __OpenBSD__
#  include <net/bpf.h>
#  include <net/if_pppvar.h>
#  include <net/if_ppp.h>
#endif /* __OpenBSD__ */

#ifdef __FreeBSD__
#    include <net/if_ppp.h>
#endif /* __FreeBSD__ */

#include <net/ppp_defs.h>

#include <gnome.h>

typedef enum {
	COLOR_RX = 0,
	COLOR_RX_BG,
	COLOR_TX,
	COLOR_TX_BG,
	COLOR_STATUS_BG,
	COLOR_STATUS_OK,
	COLOR_STATUS_WAIT,
	COLOR_TEXT_BG,
	COLOR_TEXT_FG,
	COLOR_TEXT_MID
} ColorType;

#define COLOR_COUNT 10
#define BUTTON_BLINK_INTERVAL 700

typedef enum {
	LAYOUT_HORIZONTAL = 0,
	LAYOUT_HORIZONTAL_EXTENDED = 1,
	LAYOUT_VERTICAL = 2,
	LAYOUT_VERTICAL_EXTENDED = 3,
	LAYOUT_SQUARE = 4
} DisplayLayout;

typedef enum {
	STATUS_OFF,
	STATUS_ON,
	STATUS_WAIT
} StatusType;

typedef struct _DisplayData DisplayData;
struct _DisplayData {
	DisplayLayout layout;
	gint width;
	gint height;
	gint button_x;	/* connect button */
	gint button_y;
	gint button_w;
	gint button_h;
	gint display_x;	/* display pixmap */
	gint display_y;
	gint display_w;
	gint display_h;
	gint load_x;	/* load meter */
	gint load_y;
	gint load_w;	/* this must always be 18 (hard coded) */
	gint load_h;
	gint tx_x;	/* TX light */
	gint tx_y;
	gint rx_x;	/* RX light */
	gint rx_y;
	gint bytes_x;	/* bytes throughput , -1 to disable */
	gint bytes_y;
	gint time_x;	/* time connected, -1 to disable */
	gint time_y;
	gint merge_extended_box;	/* save space ? */
};

typedef struct _MLData MLData;

struct _MLData
{
	GtkWidget *applet;
	
	int load_hist[120];
	int load_hist_pos;
	int load_hist_rx[20];
	int load_hist_tx[20];
	gint old_timer;
	gint old_bytes;
	gint old_rx_bytes;
	gint update_counter;
	int o_rx;
	int o_tx;
	int o_cd;
	int old_rx,old_tx;
	int load_count;
	int modem_was_on;
	gint last_time_was_connected;
	GdkColor display_color[COLOR_COUNT];
	gchar *display_color_text[COLOR_COUNT];
	gint UPDATE_DISPLAY;
	gint UPDATE_DELAY;
	gchar *lock_file;
	gint verify_lock_file;
	gchar *device_name;
	gchar *command_connect;
	gchar *command_disconnect;
	gint ask_for_confirmation;
	gint use_ISDN;
	gint show_extra_info;
	gint status_wait_blink;
	
	DisplayLayout layout;
	DisplayData *layout_current;

	GtkTooltips *tooltips;	
	GtkWidget *frame;
	GtkWidget *display_area;
	GtkWidget *button;
	GtkWidget *button_image;
	GdkPixmap *display;
	GdkPixmap *digits;
	GdkPixmap *lights;
	GdkPixmap *button_on;
	GdkPixmap *button_off;
	GdkPixmap *button_wait;
	GdkBitmap *button_mask;

	GdkGC *gc;
	
	GtkWidget *propwindow;
	GtkWidget *connect_entry;
	GtkWidget *disconnect_entry;
	GtkWidget *lockfile_entry;
	GtkWidget *device_entry;
	GtkWidget *verify_checkbox;
	
	gint button_blinking;
	gint button_blink_on;
	gint button_blink_id;
	
	int update_timeout_id;
	int ip_socket;
	int confirm_dialog;
	PanelAppletOrient orient;
	gint sizehint;
	gint setup_done;
	gint allocated;
	
	
	/* ISDN stuff */
	time_t start_time;
	unsigned long *isdn_stats;

};


void start_callback_update(MLData *mldata);
void reset_orientation(MLData *mldata);
void reset_colors(MLData *mldata);

void property_load (MLData *mldata);
void property_show (BonoboUIComponent *uic,
		    MLData       *mldata,
		    const gchar       *verbname);
