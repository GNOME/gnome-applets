/* GNOME modemlights applet
 * (C) 2000 John Ellis
 *
 * Authors: John Ellis
 *          Martin Baulig
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Cambridge, MA 
 * 02139, USA.
 *
 */

#include "modemlights.h"
#include <libgnomeui/gnome-window-icon.h>

#include "digits.xpm"

#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <ctype.h>
#include <sys/stat.h>

#ifdef __OpenBSD__
#include <net/if_ppp.h>
#endif

#ifdef __linux__
#include <linux/if_ppp.h>
#include <linux/isdn.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>

#ifndef ISDN_MAX_CHANNELS
#define ISDN_MAX_CHANNELS 64
#endif

static unsigned long *isdn_stats = NULL;
#endif

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

/* this holds all the possible layout configurations */
static DisplayData layout_data[] = {
	{ LAYOUT_HORIZONTAL, 46, 22,
	  0, 0, 16, 22,
	  16, 0, 30, 22,
	  1, 1, 18, 20,
	  19, 1, 19, 10,
	  -1, -1, -1, -1, FALSE
	},
	{ LAYOUT_HORIZONTAL_EXTENDED, 74, 22,
	  0, 0, 16, 22,
	  16, 0, 58, 22,
	  29, 1, 18, 20,
	  47, 1, 47, 10,
	  2, 3, 2, 12, TRUE
	},
	{ LAYOUT_VERTICAL, 20, 46,
	  0, 30, 20, 16,
	  0, 0, 20, 30,
	  1, 11, 18, 18,
	  1, 1, 10, 1,
	  -1, -1, -1, -1, FALSE
	},
	{ LAYOUT_VERTICAL_EXTENDED, 30, 58,
	  0, 42, 30, 16,
	  0, 0, 30, 42,
	  1, 1, 18, 20,
	  19, 1, 19, 10,
	  2, 23, 2, 32, TRUE
	},
	{ LAYOUT_SQUARE, 46, 46,
	  0, 0, 16, 46,
	  16, 0, 30, 46,
	  1, 1, 18, 20,
	  19, 1, 19, 10,
	  2, 24, 2, 36, FALSE
	}
};

GdkColor display_color[COLOR_COUNT];
gchar *display_color_text[COLOR_COUNT];

gint UPDATE_DELAY = 5;		/* status lights update interval in Hz (1 - 20) */
gchar *lock_file;		/* the modem lock file */
gint verify_lock_file = TRUE;	/* do we verify the pid inside the lockfile? */
gchar *device_name;		/* the device name eg:ppp0 */
gchar *command_connect;		/* connection commands */
gchar *command_disconnect;
gint ask_for_confirmation = TRUE;	/* do we ask for confirmation? */
gint use_ISDN = FALSE;		/* do we use ISDN? */
gint show_extra_info = FALSE;	/* display larger version with time/byte count */
gint status_wait_blink = FALSE;	/* blink connection light during wait status */

static gint button_blinking = FALSE;
static gint button_blink_on = 0;
static gint button_blink_id = -1;

GtkWidget *applet;
static GtkWidget *frame;
static GtkWidget *display_area = NULL;
static GtkWidget *button;
static GtkWidget *button_pixmap;
static GdkPixmap *display = NULL;
static GdkPixmap *digits = NULL;
static GdkPixmap *lights = NULL;
static GdkPixmap *button_on = NULL;
static GdkPixmap *button_off = NULL;
static GdkPixmap *button_wait = NULL;
static GdkBitmap *button_mask = NULL;
static GdkGC *gc = NULL;

static int update_timeout_id = FALSE;
static int ip_socket;
static int load_hist[120];
static int load_hist_pos = 0;
static int load_hist_rx[20];
static int load_hist_tx[20];

static int confirm_dialog = FALSE;

static PanelOrientType orient;

#ifdef HAVE_PANEL_PIXEL_SIZE
static gint sizehint;
#endif

static DisplayLayout layout = LAYOUT_HORIZONTAL;
static DisplayData *layout_current = NULL;

static gint setup_done = FALSE;


static void about_cb (AppletWidget *widget, gpointer data)
{
	static GtkWidget *about = NULL;
	const gchar *authors[8];

	if (about != NULL)
	{
		gdk_window_show(about->window);
		gdk_window_raise(about->window);
		return;
	}

	authors[0] = "John Ellis <johne@bellatlantic.net>";
	authors[1] = "Martin Baulig <martin@home-of-linux.org> - ISDN";
	authors[2] = NULL;

        about = gnome_about_new ( _("Modem Lights Applet"), VERSION,
			"(C) 2000",
			authors,
			_("Released under the GNU general public license.\n"
			"A modem status indicator and dialer.\n"
			"Lights in order from the top or left are Send data and Receive data."),
			NULL);
	gtk_signal_connect( GTK_OBJECT(about), "destroy",
			    GTK_SIGNAL_FUNC(gtk_widget_destroyed), &about );
	gtk_widget_show (about);
	return;
	widget = NULL;
	data = NULL;
}

static int is_Modem_on(void)
{
	FILE *f = 0;
	gchar buf[64];
	pid_t pid = -1;

	f = fopen(lock_file, "r");

	if(!f) return FALSE;

	if (verify_lock_file)
		{
		if (fgets(buf, sizeof(buf), f) == NULL)
			{
			fclose(f);
			return FALSE;
			}
		}

	fclose(f);

	if (verify_lock_file)
		{
		pid = (pid_t)strtol(buf, NULL, 10);
		if (pid < 1 || (kill (pid, 0) == -1 && errno != EPERM)) return FALSE;
		}

	return TRUE;
}

static int is_ISDN_on(void)
{
#ifdef __linux__

	/* Perhaps I should try to explain this code a little bit.
	 *
	 * ------------------------------------------------------------
	 * This is from the manpage of isdninfo(4):
	 *
	 * DESCRIPTION
	 *   /dev/isdninfo  is  a character device with major number 45
	 *   and minor number 255.  It delivers status information from
	 *   the Linux ISDN subsystem to user level.
	 *
	 * DATA FORMAT
	 *   When  reading  from this device, the current status of the
	 *   Linux ISDN subsystem is delivered in 6 lines of text. Each
	 *   line  starts  with  a  tag  string followed by a colon and
	 *   whitespace. After that the status values are appended sep-
	 *   arated by whitespace.
	 *
	 *   flags  is the tag of line 5. In this line for every driver
	 *          slot, it's B-Channel status is shown. If no  driver
	 *          is  registered  in a slot, a ? is shown.  For every
	 *          established B-Channel of the driver, a bit  is  set
	 *          in  the  shown value. The driver's first channel is
	 *          mapped to bit 0, the second channel to bit 1 and so
	 *          on.
	 * ------------------------------------------------------------
	 *
	 * So we open /dev/isdninfo, discard the first four lines of text
	 * and then check whether we have something that is not `0' or `?'
	 * in one of the flags fields.
	 *
	 * Sounds complicated, but I don't see any other way to check whether
	 * we are connected. Also, this is the method some other ISDN tools
	 * for Linux use.
	 *
	 * Martin
	 *
	 * ---
	 *
	 * With Linux kernel 2.4, glibc 2.2 (*) and certain providers with
	 * long phone numbers, the contents of /dev/isdninfo can be more than
	 * 1024 bytes and not be properly processed by multiple subsequent
	 * fgets()'s, so we (try to atomically) read() BUFSIZE bytes into
	 * buffer[] and process this instead of /dev/isdninfo directly.
	 *
	 * (*): dont't nail me on this combination
	 *
	 * Nils
	 */

	char buffer [BUFSIZ], *p;
	int i;
	int fd;
	int length;

	fd = open ("/dev/isdninfo", O_RDONLY | O_NDELAY);

	if (fd < 0) {
		perror ("/dev/isdninfo");
		return FALSE;
	}

	if ((length = read (fd, buffer, BUFSIZ - 1)) < 0) {
		perror ("/dev/isdninfo");
		close (fd);
		return FALSE;
	}

	buffer[length+1] = (char)0;

	p = buffer;

	/* Seek for the fifth line */
	for (i = 1; i < 5; i++) {
		if ((p = strchr (p, '\n')) == NULL) {
			close (fd);
			return FALSE;
		}
		p++;
	}

	/* *p is the first character of the fifth line now */
	if (strncmp (p, "flags:", 6)) {
		close (fd);
		return FALSE;
	}

	p += 6;

	while (*p && (*p != '\n')) {
		char *end = p;

		if (isspace (*p)) {
			p++;
			continue;
		}

		for (end = p; *end && !isspace (*end); end++)
			;

		if (*end == 0)
			break;
		else
			*end = 0;

		if (!strcmp (p, "?") || !strcmp (p, "0")) {
			p = end+1;
			continue;
		}

		close (fd);
		return TRUE;
	}

	close (fd);

	return FALSE;
#else
	return FALSE;
#endif
}

static int is_connected(void)
{
	if (use_ISDN)
		return is_ISDN_on();
	else
		return is_Modem_on();
}

static int get_modem_stats(int *in, int *out)
{
	struct 	ifreq ifreq;
	struct 	ppp_stats stats;

	memset(&ifreq, 0, sizeof(ifreq));
	strncpy(ifreq.ifr_name, device_name, IFNAMSIZ);
	ifreq.ifr_name[IFNAMSIZ-1] = '\0';
	ifreq.ifr_ifru.ifru_data = (caddr_t)&stats;

#ifdef SIOCGPPPSTATS
	if (ioctl(ip_socket, SIOCGPPPSTATS, (caddr_t)&ifreq) < 0)
#else
	if (TRUE)
#endif
		{
		/* failure means ppp is not up */
		*in = *out = 0;
		return FALSE;
		}
	else
		{
		*in = stats.p.ppp_ibytes;
		*out = stats.p.ppp_obytes;
		return TRUE;
		}
}

static int get_ISDN_stats(int *in, int *out)
{
#ifdef __linux__
	int fd, i;
	unsigned long *ptr;

	*in = *out = 0;

	if (!isdn_stats)
		isdn_stats = g_new0 (unsigned long, ISDN_MAX_CHANNELS  *  2);

	fd = open("/dev/isdninfo", O_RDONLY);

	if (fd < 0)
		return FALSE;

	if ((ioctl (fd, IIOCGETCPS, isdn_stats) < 0) && (errno != 0)) {
		close (fd);
		
		return FALSE;
	}

	for (i = 0, ptr = isdn_stats; i < ISDN_MAX_CHANNELS; i++) {
		*in  += *ptr++; *out += *ptr++;
	}

	close (fd);

	return TRUE;

#else
	*in = *out = 0;

	return FALSE;
#endif
}

static int get_stats(int *in, int *out)
{
	if (use_ISDN)
		return get_ISDN_stats(in, out);
	else
		return get_modem_stats(in, out);
}

static gint get_ISDN_connect_time(gint recalc_start)
{
	/* this is a bad hack just to get some (not very accurate) timing */
	static time_t start_time = (time_t)0;

	if (recalc_start)
		{
		start_time = time(0);
		}

	if (start_time != (time_t)0)
		return (gint)(time(0) - start_time);
	else
		return -1;
}

static gint get_modem_connect_time(gint recalc_start)
{
	static time_t start_time = (time_t)0;
	struct stat st;

	if (recalc_start)
		{
		if (stat (lock_file, &st) == 0)
			start_time = st.st_mtime;
		else
			start_time = (time_t)0;
		}

	if (start_time != (time_t)0)
		return (gint)(time(0) - start_time);
	else
		return -1;
}

static gint get_connect_time(gint recalc_start)
{
	if (use_ISDN)
		return get_ISDN_connect_time(recalc_start);
	else
		return get_modem_connect_time(recalc_start);
}

static void command_connect_cb(gint button, gpointer data)
{
	confirm_dialog = FALSE;
	if (!button) gnome_execute_shell(NULL, command_connect);
        return;
        data = NULL;
}

static void command_disconnect_cb(gint button, gpointer data)
{
	confirm_dialog = FALSE;
	if (!button) gnome_execute_shell(NULL, command_disconnect);
        return;
        data = NULL;
}

static void confirm_dialog_destroy(GtkObject *o, gpointer data)
{
 	confirm_dialog = FALSE;
        return;
        o = NULL;
}

static void dial_cb(GtkWidget *widget, gpointer data)
{
	GtkWidget *dialog;

	if (is_connected()) {
	  if (ask_for_confirmation) {
	    if (confirm_dialog) return;
	    confirm_dialog = TRUE;
	    dialog = gnome_question_dialog (_("You are currently connected.\n"
					      "Do you want to disconnect?"),
					    (GnomeReplyCallback)command_disconnect_cb,
					    NULL);
	    gtk_signal_connect (GTK_OBJECT (dialog), "destroy",
				GTK_SIGNAL_FUNC (confirm_dialog_destroy),
				NULL);
	  } else {
	    gnome_execute_shell(NULL, command_disconnect);
	    confirm_dialog = FALSE;
	  }
	} else {
	  if (ask_for_confirmation) {
	    if (confirm_dialog) return;
	    confirm_dialog = TRUE;
	    dialog = gnome_question_dialog (_("Do you want to connect?"),
					    (GnomeReplyCallback)command_connect_cb,
					    NULL);
	    gtk_signal_connect (GTK_OBJECT (dialog), "destroy",
				GTK_SIGNAL_FUNC (confirm_dialog_destroy),
				NULL);
	  } else {
	    gnome_execute_shell(NULL, command_connect);
	    confirm_dialog = FALSE;
	  }
	}
}

static void update_tooltip(int connected, int rx, int tx)
{
	gchar *text;
	if (connected)
		{
		gint t;
		gint h, m;

		t = get_connect_time(FALSE);

		h = t / 3600;
		m = (t - (h * 3600)) / 60;

		text = g_strdup_printf(_("%#.1fMb received / %#.1fMb sent / time: %.1d:%.2d"),
				       (float)rx / 1000000, (float)tx / 1000000, h, m);
		}
	else
		{
		text = g_strdup(_("not connected"));
		}

	applet_widget_set_widget_tooltip(APPLET_WIDGET(applet), button, text);
	g_free(text);
}

/*
 *-------------------------------------
 * display drawing
 *-------------------------------------
 */

static void redraw_display(void)
{
	gdk_window_set_back_pixmap(display_area->window,display,FALSE);
	gdk_window_clear(display_area->window);
}

static void draw_digit(gint n, gint x, gint y)
{
	gdk_draw_pixmap (display, display_area->style->fg_gc[GTK_WIDGET_STATE(display_area)],
			 digits, n * 5, 0, x, y, 5, 7);
}

static void draw_timer(gint seconds, gint force)
{
	gint x, y;

	if (!layout_current || layout_current->time_x < 0) return;

	x = layout_current->time_x;
	y = layout_current->time_y;

	if (seconds > -1)
		{
		gint a, b;

		if (seconds >= 3600) seconds /= 60; /* HH:MM, else MM:SS */

		a = seconds / 60;
		b = seconds % 60;

		draw_digit(a / 10, x, y);
		draw_digit(a % 10, x+5, y);

		draw_digit(b / 10, x+15, y);
		draw_digit(b % 10, x+20, y);

		draw_digit(15, x+10, y);
		}
	else
		{
		if (force)
			{
			draw_digit(10, x, y);
			draw_digit(10, x+5, y);

			draw_digit(10, x+15, y);
			draw_digit(10, x+20, y);
			}

		draw_digit(14, x+10, y);
		}
}

static void draw_bytes(gint bytes)
{
	gint x, y;

	if (!layout_current || layout_current->bytes_x < 0) return;

	x = layout_current->bytes_x;
	y = layout_current->bytes_y;

	if (bytes > -1)
		{
		gint dig;
		if (bytes > 9999)
			{
			bytes /= 1024;
			draw_digit(12, x + 20, y);
			}
		else
			{
			draw_digit(13, x + 20, y);
			}

		dig = bytes / 1000;
		draw_digit(dig, x, y);
		bytes %= 1000;
		dig = bytes / 100;
		draw_digit(dig, x+5, y);
		bytes %= 100;
		dig = bytes / 10;
		draw_digit(dig, x+10, y);
		bytes %= 10;
		draw_digit(bytes, x+15, y);
		}
	else
		{
		draw_digit(10, x, y);
		draw_digit(10, x+5, y);
		draw_digit(10, x+10, y);
		draw_digit(10, x+15, y);

		draw_digit(11, x+20, y);
		}
}

static gint update_extra_info(int rx_bytes, gint force)
{
	static gint old_timer = -1;
	static gint old_bytes = -1;
	static gint old_rx_bytes = -1;
	static gint update_counter = 0;
	gint redraw = FALSE;
	gint new_timer;
	gint new_bytes;

	if (!layout_current || (layout_current->bytes_x < 0 && layout_current->time_x < 0)) return FALSE;

	update_counter++;
	if (update_counter < UPDATE_DELAY && !force) return FALSE;
	update_counter = 0;

	if (rx_bytes != -1)
		new_timer = get_connect_time(FALSE);
	else
		new_timer = -1;

	if (new_timer != old_timer)
		{
		old_timer = new_timer;
		redraw = TRUE;
		draw_timer(new_timer, FALSE);
		}
	else if (force)
		{
		redraw = TRUE;
		draw_timer(old_timer, TRUE);
		}

	if (rx_bytes == -1 || old_rx_bytes == -1)
		{
		new_bytes = -1;
		}
	else
		{
		new_bytes = rx_bytes - old_rx_bytes;
		}

	if (new_bytes != old_bytes)
		{
		old_bytes = new_bytes;
		redraw = TRUE;
		draw_bytes(new_bytes);
		}
	else if (force)
		{
		redraw = TRUE;
		draw_bytes(old_bytes);
		}

	old_rx_bytes = rx_bytes;

	return redraw;
}

static void draw_load(int rxbytes, int txbytes)
{
	int load_max = 0;
	int i;
	int x, y, dot_height;
	float bytes_per_dot;

	if (!layout_current) return;

	x = layout_current->load_x + 1;
	y = layout_current->load_y + layout_current->load_h - 2;
	dot_height = layout_current->load_h - 2;

	/* sanity check: */
	if (rxbytes <0) rxbytes = 0;
	if (txbytes <0) txbytes = 0;

	load_hist_pos++;
	if (load_hist_pos > 119) load_hist_pos = 0;
	if (txbytes > rxbytes)
		load_hist[load_hist_pos] = txbytes;
	else
		load_hist[load_hist_pos] = rxbytes;
	for (i=0;i<120;i++)
		if (load_max < load_hist[i]) load_max = load_hist[i];

	for (i=0;i<15;i++)
		{
		load_hist_rx[i] = load_hist_rx[i+1];
		load_hist_tx[i] = load_hist_tx[i+1];
		}
	load_hist_rx[15] = rxbytes;
	load_hist_tx[15] = txbytes;

	if (load_max < dot_height)
		bytes_per_dot = 1.0;
	else
		bytes_per_dot = (float)load_max / (dot_height - 1);

	gdk_gc_set_foreground( gc, &display_color[COLOR_TEXT_BG] );
	gdk_draw_rectangle(display, gc, TRUE, x, y - dot_height + 1, 16, dot_height);

	gdk_gc_set_foreground( gc, &display_color[COLOR_RX] );
	for (i=0;i<16;i++)
		{
		if( load_hist_rx[i] )
			gdk_draw_line(display, gc, x+i, y ,
				x+i, y - ((float)load_hist_rx[i] / bytes_per_dot) + 1);
		}

	gdk_gc_set_foreground( gc, &display_color[COLOR_TX] );
	for (i=0;i<16;i++)
		{
		if( load_hist_tx[i] )
			gdk_draw_line(display, gc, x+i, y,
				x+i, y - ((float)load_hist_tx[i] / bytes_per_dot) + 1);
		}

	redraw_display();
}

static void draw_light(int lit, int x, int y, ColorType color)
{
	gint p;

	if (lit)
		p = 9;
	else
		p = 0;

	if (color == COLOR_RX) p += 18;
	

	gdk_draw_pixmap (display, display_area->style->fg_gc[GTK_WIDGET_STATE(display_area)],
				 lights, 0, p, x, y, 9, 9);
}

static void update_button(StatusType type)
{
	switch(type)
		{
		case STATUS_ON:
			gtk_pixmap_set(GTK_PIXMAP(button_pixmap), button_on, button_mask);
			break;
		case STATUS_WAIT:
			gtk_pixmap_set(GTK_PIXMAP(button_pixmap), button_wait, button_mask);
			break;
		default:
			gtk_pixmap_set(GTK_PIXMAP(button_pixmap), button_off, button_mask);
			break;
		}
}

/* to minimize drawing (pixmap manipulations) we only draw a light if it has changed */
static void update_lights(int rx, int tx, int cd, int rx_bytes, gint force)
{
	static int o_rx = FALSE;
	static int o_tx = FALSE;
	static int o_cd = FALSE;
	int redraw_required = FALSE;

	if (!layout_current) return;

	if (rx != o_rx || force)
		{
		o_rx = rx;
		draw_light(rx , layout_current->rx_x, layout_current->rx_y, COLOR_RX);
		redraw_required = TRUE;
		}
	if (tx != o_tx || force)
		{
		o_tx = tx;
		draw_light(tx, layout_current->tx_x, layout_current->tx_y, COLOR_TX);
		redraw_required = TRUE;
		}
	if ((cd != o_cd && !button_blinking) || force)
		{
		o_cd = cd;
		update_button(cd ? STATUS_ON : STATUS_OFF);
		}

	/* we do the extra info redraws here too */
	if (show_extra_info && update_extra_info(rx_bytes, force))
		{
		redraw_required = TRUE;
		}

	if (redraw_required) redraw_display();
}

static gint button_blink_cb(gpointer data)
{
	if (button_blink_on && status_wait_blink)
		{
		button_blink_on = FALSE;
		}
	else
		{
		button_blink_on = TRUE;
		}
	update_button(button_blink_on ? STATUS_WAIT : STATUS_OFF);

	return TRUE;
	data = NULL;
}

static void button_blink(gint blink, gint status)
{
	if (button_blinking == blink) return;

	if (blink)
		{
		if (button_blink_id == -1)
			{
			button_blink_id = gtk_timeout_add(BUTTON_BLINK_INTERVAL, button_blink_cb, NULL);
			}
		}
	else
		{
		if (button_blink_id != -1)
			{
			gtk_timeout_remove(button_blink_id);
			button_blink_id = -1;
			}
		}

	button_blinking = blink;
	button_blink_on = status;
	if (blink)
		{
		update_button(button_blink_on ? STATUS_WAIT : STATUS_OFF);
		}	
	else
		{
		update_button(button_blink_on ? STATUS_ON : STATUS_OFF);
		}
}


/*
 *-------------------------------------
 * the main callback loop (1 sec)
 *-------------------------------------
 */

static gint update_display(void)
{
	static int old_rx,old_tx;
	static int load_count;
	static int modem_was_on = FALSE;
	static gint last_time_was_connected = FALSE;
	int rx, tx;
	int light_rx = FALSE;
	int light_tx = FALSE;

	load_count++;

	if (is_connected())
		{
		if (!last_time_was_connected)
			{
			get_connect_time(TRUE); /* reset start time */
			last_time_was_connected = TRUE;
			}

		if (!get_stats (&rx, &tx) || (rx == 0 && tx == 0))
			{
			old_rx = old_tx = 0;
			button_blink(TRUE, TRUE);
			}
		else
			{
			button_blink(FALSE, TRUE);
			if (rx > old_rx) light_rx = TRUE;
			if (tx > old_tx) light_tx = TRUE;
			}
		
		update_lights(light_rx, light_tx, TRUE, rx, FALSE);
		if (load_count > UPDATE_DELAY * 2)
			{
			static int load_rx, load_tx;
			static int tooltip_counter;

			tooltip_counter++;
			if (tooltip_counter > 10)
				{
				update_tooltip(TRUE, rx, tx);
				tooltip_counter = 0;
				}

	/* This is a check to see if the modem was running before the program
	started. if it was, we set the past bytes to the current bytes. Otherwise
	the first load calculation could have a very high number of bytes.
	(the bytes that accumulated before program start will make max_load too high) */
			if (!modem_was_on)
				{
				load_rx = rx;
				load_tx = tx;
				update_tooltip(TRUE,rx,tx);
				modem_was_on = TRUE;
				}
		
			load_count = 0;
			draw_load(rx - load_rx, tx - load_tx);
			load_rx = rx;
			load_tx = tx;
			}
		old_rx = rx;
		old_tx = tx;
		}
	else
		{
		if (load_count > UPDATE_DELAY * 2)
			{
			load_count = 0;
			draw_load(0,0);
			}
		button_blink(FALSE, FALSE);
		update_lights(FALSE, FALSE, FALSE, -1, FALSE);
		if (modem_was_on)
			{
			update_tooltip(FALSE,0,0);
			modem_was_on = FALSE;
			}
		if (last_time_was_connected)
			{
			last_time_was_connected = FALSE;
			}
		}

	return TRUE;
}

/*
 *-------------------------------------
 * setup and init
 *-------------------------------------
 */

/* start or change the update callback timeout interval */
void start_callback_update(void)
{
	gint delay;
	delay = 1000 / UPDATE_DELAY;
	if (update_timeout_id) gtk_timeout_remove(update_timeout_id);
	update_timeout_id = gtk_timeout_add(delay, (GtkFunction)update_display, NULL);

}

static void draw_shadow_box(GdkPixmap *window, gint x, gint y, gint w, gint h,
			    gint etched_in, GdkGC *bgc)
{
	GdkGC *gc1;
	GdkGC *gc2;

	if (!etched_in)
		{
		gc1 = applet->style->light_gc[GTK_STATE_NORMAL];
		gc2 = applet->style->dark_gc[GTK_STATE_NORMAL];
		}
	else
		{
		gc1 = applet->style->dark_gc[GTK_STATE_NORMAL];
		gc2 = applet->style->light_gc[GTK_STATE_NORMAL];
		}

	gdk_draw_line(window, gc1, x, y + h - 1, x, y);
	gdk_draw_line(window, gc1, x, y, x + w - 2, y);
	gdk_draw_line(window, gc2, x + w - 1, y, x + w - 1, y + h - 1);
	gdk_draw_line(window, gc2, x + w - 2, y + h - 1, x + 1, y + h - 1);

	if (bgc)
		{
		gdk_draw_rectangle(window, bgc, TRUE, x + 1, y + 1, w - 2, h - 2);
		}
}

static void create_background_pixmap(void)
{
	if (!layout_current) return;

	if (display) gdk_pixmap_unref(display);

	display = gdk_pixmap_new(display_area->window,
				 layout_current->display_w, layout_current->display_h, -1);

	/* main border */
	draw_shadow_box(display, 0, 0, layout_current->display_w, layout_current->display_h,
			FALSE, applet->style->bg_gc[GTK_STATE_NORMAL]);

	/* load border */
	gdk_gc_set_foreground( gc, &display_color[COLOR_TEXT_BG] );
	draw_shadow_box(display, layout_current->load_x, layout_current->load_y,
			layout_current->load_w, layout_current->load_h, TRUE, gc);

	/* text border(s) */
	if (layout_current->bytes_x >= 0)
		{
		draw_shadow_box(display, layout_current->bytes_x - 1, layout_current->bytes_y - 2,
				28, layout_current->merge_extended_box ? 20 : 11,
				TRUE, gc);
		}
	if (layout_current->time_x >= 0 && !layout_current->merge_extended_box)
		{
		draw_shadow_box(display, layout_current->time_x - 1, layout_current->time_y - 2,
				28, 11, TRUE, gc);
		}
}

static void draw_button_light(GdkPixmap *pixmap, gint x, gint y, gint s, gint etched_in, ColorType color)
{
	GdkGC *gc1;
	GdkGC *gc2;

	s -= 8;

	gdk_gc_set_foreground( gc, &display_color[color] );

	if (etched_in)
		{
		gc1 = applet->style->dark_gc[GTK_STATE_NORMAL];
		gc2 = applet->style->light_gc[GTK_STATE_NORMAL];
		}
	else
		{
		gc1 = applet->style->light_gc[GTK_STATE_NORMAL];
		gc2 = applet->style->dark_gc[GTK_STATE_NORMAL];
		}

	/* gdk_draw_arc was always off by one in my attempts (?) */

	gdk_draw_line(pixmap, gc1, x, y + 3, x, y + 4 + s);
	gdk_draw_line(pixmap, gc1, x + 3, y, x + 4 + s, y);
	gdk_draw_line(pixmap, gc1, x + 1, y + 2, x + 1, y + 5 + s);
	gdk_draw_line(pixmap, gc1, x + 2, y + 1, x + 5 + s, y + 1);

	gdk_draw_line(pixmap, gc2, x + 7 + s, y + 3, x + 7 + s, y + 4 + s);
	gdk_draw_line(pixmap, gc2, x + 3, y + 7 + s, x + 4 + s, y + 7 + s);
	gdk_draw_line(pixmap, gc2, x + 6 + s, y + 2, x + 6 + s, y + 5 + s);
	gdk_draw_line(pixmap, gc2, x + 2, y + 6 + s, x + 5 + s, y + 6 + s);

	gdk_draw_rectangle(pixmap, gc, TRUE, x + 3, y + 1, 2 + s, 6 + s);
	gdk_draw_rectangle(pixmap, gc, TRUE, x + 1, y + 3, 6 + s, 2 + s);
	gdk_draw_rectangle(pixmap, gc, TRUE, x + 2, y + 2, 4 + s, 4 + s);
}

static void pixmap_set_colors(GdkPixmap *pixmap, GdkColor *bg, GdkColor *fg, GdkColor *mid)
{
	gint w;
	gint h;
	gint x, y;
	GdkImage *image;
	guint32 bg_pixel = 0x00000000;
	guint32 fg_pixel = 0x00000000;
	gint have_fg = FALSE;

	gdk_window_get_size(pixmap, &w, &h);

	image = gdk_image_get(pixmap, 0, 0, w, h);

	/* always assume 0, 0 is background color */
	bg_pixel = gdk_image_get_pixel(image, 0, 0);

	for (x = 0; x < w; x++)
		{
		for (y = 0; y < h; y++)
			{
			guint32 pixel;

			pixel = gdk_image_get_pixel(image, x, y);
			if (pixel == bg_pixel)
				{
				gdk_gc_set_foreground( gc, bg );
				}
			else if (!have_fg || pixel == fg_pixel)
				{
				if (!have_fg)
					{
					have_fg = TRUE;
					fg_pixel = pixel;
					}
				gdk_gc_set_foreground( gc, fg );
				}
			else
				{
				gdk_gc_set_foreground( gc, mid );
				}
			gdk_draw_point(pixmap, gc, x, y);
			}
		}

	gdk_image_destroy(image);
}

static void update_pixmaps(void)
{
	GtkStyle *style;
	style = gtk_widget_get_style(applet);

	if (!digits)
		{
		digits = gdk_pixmap_create_from_xpm_d(display_area->window, NULL,
			&style->bg[GTK_STATE_NORMAL], (gchar **)digits_xpm);
		}
	/* change digit colors */
	pixmap_set_colors(digits,
			  &display_color[COLOR_TEXT_BG],
			  &display_color[COLOR_TEXT_FG],
			  &display_color[COLOR_TEXT_MID]);

	if (!button_on) button_on = gdk_pixmap_new(display_area->window, 10, 10, -1);
	if (!button_off) button_off = gdk_pixmap_new(display_area->window, 10, 10, -1);
	if (!button_wait) button_wait = gdk_pixmap_new(display_area->window, 10, 10, -1);
	if (!button_mask)
		{
		GdkGC *mask_gc = NULL;

		button_mask = gdk_pixmap_new(NULL, 10, 10, 1);

		mask_gc = gdk_gc_new(button_mask);

		gdk_gc_set_foreground(mask_gc, &style->black);
		gdk_draw_rectangle(button_mask, mask_gc, TRUE, 0, 0, 10, 10);
		gdk_gc_set_foreground(mask_gc, &style->white);
		/* gdk_draw_arc was always off by one in my attempts (?) */
		gdk_draw_rectangle(button_mask, mask_gc, TRUE, 3, 0, 4, 10);
		gdk_draw_rectangle(button_mask, mask_gc, TRUE, 0, 3, 10, 4);
		gdk_draw_rectangle(button_mask, mask_gc, TRUE, 2, 1, 6, 8);
		gdk_draw_rectangle(button_mask, mask_gc, TRUE, 1, 2, 8, 6);

		gdk_gc_unref(mask_gc);
		}

	draw_button_light(button_on, 0, 0, 10, TRUE, COLOR_STATUS_OK);
	draw_button_light(button_off, 0, 0, 10, TRUE, COLOR_STATUS_BG);
	draw_button_light(button_wait, 0, 0, 10, TRUE, COLOR_STATUS_WAIT);

	if (!lights) lights = gdk_pixmap_new(display_area->window, 9, 36, -1);

	gdk_draw_rectangle(lights, applet->style->bg_gc[GTK_STATE_NORMAL], TRUE, 0, 0, 9, 36);
	draw_button_light(lights, 0, 0, 9, FALSE, COLOR_TX_BG);
	draw_button_light(lights, 0, 9, 9, FALSE, COLOR_TX);
	draw_button_light(lights, 0, 18, 9, FALSE, COLOR_RX_BG);
	draw_button_light(lights, 0, 27, 9, FALSE, COLOR_RX);
}

static void setup_colors(void)
{
	static gint allocated = FALSE;
	GdkColormap *colormap;
	gint i;
	gboolean success[COLOR_COUNT];

        colormap = gtk_widget_get_colormap(display_area);

	if (allocated)
		{
		gdk_colormap_free_colors(colormap, display_color, COLOR_COUNT);
		}

	for (i = 0; i < COLOR_COUNT; i++)
		{
		if (!display_color_text[i]) display_color_text[i] = g_strdup("#000000");
		gdk_color_parse(display_color_text[i], &display_color[i]);
		}
	gdk_colormap_alloc_colors(colormap, display_color, COLOR_COUNT, FALSE, TRUE, success);
	for (i = 0; i < COLOR_COUNT; i++)
		{
		if (!success[i]) printf("unable to allocate color %s\n", display_color_text[i]);
		}

	allocated = TRUE;

	if (!gc)
		{
		gc = gdk_gc_new( display_area->window );
        	gdk_gc_copy( gc, display_area->style->white_gc );
		}
}

void reset_orientation(void)
{
	if (sizehint >= PIXEL_SIZE_STANDARD)
		{
		if (show_extra_info)
			{
			layout = LAYOUT_SQUARE;
			}
		else if (orient == ORIENT_LEFT || orient == ORIENT_RIGHT)
			{
			layout = LAYOUT_HORIZONTAL;
			}
		else
			{
			layout = LAYOUT_VERTICAL;
			}
		}
	else
		{
		if (orient == ORIENT_LEFT || orient == ORIENT_RIGHT)
			{
			if (show_extra_info)
				{
				layout = LAYOUT_VERTICAL_EXTENDED;
				}
			else
				{
				layout = LAYOUT_VERTICAL;
				}
			}
		else
			{
			if (show_extra_info)
				{
				layout = LAYOUT_HORIZONTAL_EXTENDED;
				}
			else
				{
				layout = LAYOUT_HORIZONTAL;
				}
			}
		}

	if (layout < LAYOUT_HORIZONTAL || layout > LAYOUT_SQUARE) layout = LAYOUT_HORIZONTAL;
	layout_current = &layout_data[layout];

#if 0
	printf("Test layout = %d\n", layout_current->layout);
#endif

	create_background_pixmap();
	update_pixmaps();

	gtk_widget_set_usize(frame, layout_current->width, layout_current->height);
	gtk_drawing_area_size(GTK_DRAWING_AREA(display_area),
			      layout_current->display_w, layout_current->display_h);

	gtk_widget_set_usize(button, layout_current->button_w, layout_current->button_h);
	gtk_fixed_move(GTK_FIXED(frame), display_area, layout_current->display_x, layout_current->display_y);
	gtk_fixed_move(GTK_FIXED(frame), button, layout_current->button_x, layout_current->button_y);

	/* we set the lights to off so they will be correct on the next update */
	update_lights(FALSE, FALSE, FALSE, -1, TRUE);
	redraw_display();
}

void reset_colors(void)
{
	setup_colors();
	create_background_pixmap();
	update_pixmaps();
	update_lights(FALSE, FALSE, FALSE, -1, TRUE);
	redraw_display();
}

/* this is called when the applet's style changes (meaning probably a theme/color change) */
static void applet_style_change_cb(GtkWidget *widget, GtkStyle *previous_style, gpointer data)
{
	reset_orientation();
	return;
	widget = NULL;
	previous_style = NULL;
	data = NULL;
}

static void applet_change_orient(GtkWidget *w, PanelOrientType o, gpointer data)
{
	orient = o;
	if (setup_done) reset_orientation();
	return;
	w = NULL;
	data = NULL;
}

#ifdef HAVE_PANEL_PIXEL_SIZE
static void applet_change_pixel_size(GtkWidget *w, int s, gpointer data)
{
	sizehint = s;
	if (setup_done) reset_orientation();
        return;
        w = NULL;
        data = NULL;
}
#endif

static void show_help_cb(AppletWidget *applet, gpointer data)
{
	static GnomeHelpMenuEntry help_entry = { NULL, "index.html"};

	help_entry.name = gnome_app_id;
    
	gnome_help_display (NULL, &help_entry);
}

static gint applet_save_session(GtkWidget *widget, char *privcfgpath, char *globcfgpath)
{
	property_save(privcfgpath, FALSE);
        return FALSE;
        widget = NULL;
        globcfgpath = NULL;
}

int main (int argc, char *argv[])
{
	gint i;

        /* Initialize the i18n stuff */
        bindtextdomain (PACKAGE, GNOMELOCALEDIR);
        textdomain (PACKAGE);

	for (i=0;i<119;i++)
		load_hist[i] = 0;

	for (i=0;i<19;i++)
		{
		load_hist_rx[i] = 0;
		load_hist_tx[i] = 0;
		}
	for (i = 0; i < COLOR_COUNT; i++)
		{
		display_color_text[i] = NULL;
		display_color[i] = (GdkColor){ 0, 0, 0, 0 };
		}

	layout_current = &layout_data[LAYOUT_HORIZONTAL];

	applet_widget_init("modemlights_applet", VERSION, argc, argv,
				    NULL, 0, NULL);

	gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gnome-modem.png");

	if (g_file_exists("/dev/modem"))
		{
		lock_file = g_strdup("/var/lock/LCK..modem");
		}
	else
		{
		lock_file = g_strdup("/var/lock/LCK..ttyS0");
		}

	device_name = g_strdup("ppp0");
	command_connect = g_strdup("pppon");
	command_disconnect = g_strdup("pppoff");
	orient = ORIENT_UP;

	/* open ip socket */
	if ((ip_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		{
		g_print("could not open an ip socket\n");
		return 1;
		}


	applet = applet_widget_new("modemlights_applet");
	if (!applet)
		g_error("Can't create applet!\n");

#ifdef HAVE_PANEL_PIXEL_SIZE
	sizehint = applet_widget_get_panel_pixel_size(APPLET_WIDGET(applet));
#endif

	property_load(APPLET_WIDGET(applet)->privcfgpath);

	/* frame for all widgets */
	frame = gtk_fixed_new();
	gtk_widget_show(frame);

	display_area = gtk_drawing_area_new();
	gtk_drawing_area_size(GTK_DRAWING_AREA(display_area),5,5);
	gtk_fixed_put(GTK_FIXED(frame),display_area,0,0);
	gtk_widget_show(display_area);

	button = gtk_button_new();
	gtk_widget_set_usize(button,5,5);
	gtk_fixed_put(GTK_FIXED(frame),button,5,0);
	gtk_signal_connect(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(dial_cb),NULL);
	gtk_widget_show(button);

	gtk_signal_connect(GTK_OBJECT(applet),"change_orient",
				GTK_SIGNAL_FUNC(applet_change_orient),
				NULL);
#ifdef HAVE_PANEL_PIXEL_SIZE
	gtk_signal_connect(GTK_OBJECT(applet),"change_pixel_size",
				GTK_SIGNAL_FUNC(applet_change_pixel_size),
				NULL);
#endif

	applet_widget_add(APPLET_WIDGET(applet), frame);
	gtk_widget_realize(applet);
	gtk_widget_realize(display_area);

	setup_colors();
	update_pixmaps();

	button_pixmap = gtk_pixmap_new(button_off, button_mask);
	gtk_container_add(GTK_CONTAINER(button), button_pixmap);
	gtk_widget_show(button_pixmap);

	update_tooltip(FALSE,0,0);

	/* by now we know the geometry */
	setup_done = TRUE;
	reset_orientation();
	gtk_widget_show(applet);

	gtk_signal_connect(GTK_OBJECT(applet),"style_set",
		GTK_SIGNAL_FUNC(applet_style_change_cb), NULL);

	gtk_signal_connect(GTK_OBJECT(applet),"save_session",
		GTK_SIGNAL_FUNC(applet_save_session), NULL);

	applet_widget_register_stock_callback(APPLET_WIDGET(applet),
					      "properties",
					      GNOME_STOCK_MENU_PROP,
					      _("Properties..."),
					      property_show,
					      NULL);

	applet_widget_register_stock_callback(APPLET_WIDGET(applet),
					      "help",
					      GNOME_STOCK_PIXMAP_HELP,
					      _("Help..."),
					      show_help_cb, NULL);

	applet_widget_register_stock_callback(APPLET_WIDGET(applet),
					      "about",
					      GNOME_STOCK_MENU_ABOUT,
					      _("About..."),
					      about_cb, NULL);
	start_callback_update();

	applet_widget_gtk_main();
	return 0;
}


