/* GNOME modemlights applet
 * (C) 1999 John Ellis
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
#include <linux/isdn.h>
#include <sys/ioctl.h>
#include <fcntl.h>

static unsigned long *isdn_stats = NULL;
#endif

typedef enum {
	COLOR_RX,
	COLOR_RX_BG,
	COLOR_TX,
	COLOR_TX_BG
} ColorType;

gint UPDATE_DELAY = 5;		/* status lights update interval in Hz (1 - 20) */
gchar *lock_file;		/* the modem lock file */
gint verify_lock_file = TRUE;	/* do we verify the pid inside the lockfile? */
gchar *device_name;		/* the device name eg:ppp0 */
gchar *command_connect;		/* connection commands */
gchar *command_disconnect;
gint ask_for_confirmation = TRUE;	/* do we ask for confirmation? */
gint use_ISDN = FALSE;		/* do we use ISDN? */
gint show_extra_info = FALSE;	/* display larger version with time/byte count */

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
static GdkBitmap *button_mask = NULL;
static GdkGC *gc = NULL;
static GdkColor rx_color;
static GdkColor rx_bgcolor;
static GdkColor tx_color;
static GdkColor tx_bgcolor;

static int update_timeout_id = FALSE;
static int ip_socket;
static int load_hist[120];
static int load_hist_pos = 0;
static int load_hist_rx[20];
static int load_hist_tx[20];

static int confirm_dialog = FALSE;

static PanelOrientType orient;

#ifdef HAVE_PANEL_PIXEL_SIZE
static int sizehint;
#endif

static gint panel_verticle = FALSE;
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
			"(C) 1999",
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

static int is_Modem_on()
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

/* FIX ME!** this is a slot for checking if isdn is connected,
 * add code please */
static int is_ISDN_on()
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
	 */

	FILE *f = 0;
	char buffer [BUFSIZ], *p;
	int i;

	f = fopen("/dev/isdninfo", "r");

	if (!f) return FALSE;

	for (i = 0; i < 5; i++) {
		if (fgets (buffer, BUFSIZ, f) == NULL) {
			fclose (f);
			return FALSE;
		}
	}

	if (strncmp (buffer, "flags:", 6)) {
		fclose (f);
		return FALSE;
	}

	p = buffer+6;

	while (*p) {
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

		fclose (f);

		return TRUE;
	}

	fclose (f);

	return FALSE;
#else
	return FALSE;
#endif
}

static int is_connected()
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
#if defined(__FreeBSD__) || defined(__OpenBSD__)
	strncpy(ifreq.ifr_name, device_name, IFNAMSIZ);
#else
	strncpy(ifreq.ifr_ifrn.ifrn_name, device_name, IFNAMSIZ);
#endif /* FreeBSD or OpenBSD */
	ifreq.ifr_ifru.ifru_data = (caddr_t)&stats;
#if defined(__FreeBSD__) || defined(__OpenBSD__)
		if ((ioctl(ip_socket,SIOCGPPPSTATS,(caddr_t)&ifreq) < 0))
#else
#ifdef SIOCDEVPRIVATE
	if ((ioctl(ip_socket,SIOCDEVPRIVATE,(caddr_t)&ifreq) < 0))
#else
	*in = *out = 0;
	return FALSE;
#endif
#endif /* FreeBSD or OpenBSD */
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

static void command_connect_cb( gint button, gpointer data)
{
	confirm_dialog = FALSE;
	if (!button) gnome_execute_shell(NULL, command_connect);
        return;
        data = NULL;
}

static void command_disconnect_cb( gint button, gpointer data)
{
	confirm_dialog = FALSE;
	if (!button) gnome_execute_shell(NULL, command_disconnect);
        return;
        data = NULL;
}

static void confirm_dialog_destroy (GtkObject *o)
{
 	confirm_dialog = FALSE;
        return;
        o = NULL;
}

static void dial_cb()
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
	    system(command_disconnect);
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
	    system(command_connect);
	    confirm_dialog = FALSE;
	  }
	}
}

static void update_tooltip(int connected, int rx, int tx)
{
	gchar text[64];
	if (connected)
		{
		g_snprintf(text,sizeof(text),"%#.1fM / %#.1fM",(float)rx / 1000000, (float)tx / 1000000);
		}
	else
		{
		strncpy(text, _("not connected"), sizeof(text));
		text[sizeof(text) - 1] = 0;
		}

	applet_widget_set_widget_tooltip(APPLET_WIDGET(applet),button,text);
}

/*
 *-------------------------------------
 * display drawing
 *-------------------------------------
 */

static void redraw_display()
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
	if (seconds > -1)
		{
		gint a, b;

		if (seconds >= 3600) seconds /= 60; /* HH:MM, else MM:SS */

		a = seconds / 60;
		b = seconds % 60;

		draw_digit(a / 10, 2, 36);
		draw_digit(a % 10, 7, 36);

		draw_digit(b / 10, 17, 36);
		draw_digit(b % 10, 22, 36);

		draw_digit(15, 12, 36);
		}
	else
		{
		if (force)
			{
			draw_digit(10, 2, 36);
			draw_digit(10, 7, 36);

			draw_digit(10, 17, 36);
			draw_digit(10, 22, 36);
			}

		draw_digit(14, 12, 36);
		}
}

static void draw_bytes(gint bytes)
{
	if (bytes > -1)
		{
		gint dig;
		if (bytes > 9999)
			{
			bytes /= 1024;
			draw_digit(12, 22, 24);
			}
		else
			{
			draw_digit(13, 22, 24);
			}

		dig = bytes / 1000;
		draw_digit(dig, 2, 24);
		bytes %= 1000;
		dig = bytes / 100;
		draw_digit(dig, 7, 24);
		bytes %= 100;
		dig = bytes / 10;
		draw_digit(dig, 12, 24);
		bytes %= 10;
		draw_digit(bytes, 17, 24);
		}
	else
		{
		draw_digit(10, 2, 24);
		draw_digit(10, 7, 24);
		draw_digit(10, 12, 24);
		draw_digit(10, 17, 24);

		draw_digit(11, 22, 24);
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

	if (!show_extra_info) return FALSE;

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

	if (show_extra_info)
		{
		x = 2;
		y = 19;
		dot_height = 18;
		}
	else if (panel_verticle)
		{
		x = 2;
		y = 17;
		dot_height = 16;
		}
	else
		{
		x = 2;
		y = 27;
		dot_height = 16;
		}

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

	gdk_draw_rectangle(display, display_area->style->black_gc, TRUE, x, y - dot_height + 1, 16, dot_height);

	gdk_gc_set_foreground( gc, &rx_color );
	for (i=0;i<16;i++)
		{
		if( load_hist_rx[i] )
			gdk_draw_line(display, gc, x+i, y,
				x+i, y - ((float)load_hist_rx[i] / bytes_per_dot ) + 1);
		}

	gdk_gc_set_foreground( gc, &tx_color );
	for (i=0;i<16;i++)
		{
		if( load_hist_tx[i] )
			gdk_draw_line(display, gc, x+i, y,
				x+i, y - ((float)load_hist_tx[i] / bytes_per_dot ) + 1);
		}

	redraw_display();
}

static void draw_light(int lit, int x, int y, ColorType color)
{
	gint p;

	/* if the orientation is sideways (left or right panel), we swap x and y */
	if (show_extra_info || panel_verticle)
		{
		int t;
		t = y;
		y = x;
		x = 21 - t;
		}

	if (lit)
		p = 9;
	else
		p = 0;

	if (color == COLOR_RX) p += 18;
	

	gdk_draw_pixmap (display, display_area->style->fg_gc[GTK_WIDGET_STATE(display_area)],
				 lights, 0, p, x, y, 9, 9);
}

/* to minimize drawing (pixmap manipulations) we only draw a light if it has changed */
static void update_lights(int rx, int tx, int cd, int rx_bytes, gint force)
{
	static int o_rx = FALSE;
	static int o_tx = FALSE;
	static int o_cd = FALSE;
	int redraw_required = FALSE;

	if (rx != o_rx || force)
		{
		o_rx = rx;
		draw_light(rx , 10, 1, COLOR_RX);
		redraw_required = TRUE;
		}
	if (tx != o_tx || force)
		{
		o_tx = tx;
		draw_light(tx, 1, 1, COLOR_TX);
		redraw_required = TRUE;
		}
	if (cd != o_cd)
		{
		o_cd = cd;
		if (cd)
			gtk_pixmap_set(GTK_PIXMAP(button_pixmap), button_on, button_mask);
		else
			gtk_pixmap_set(GTK_PIXMAP(button_pixmap), button_off, button_mask);
		}

	/* we do the extra info redraws here too */
	if (show_extra_info && update_extra_info(rx_bytes, force))
		{
		redraw_required = TRUE;
		}

	if (redraw_required) redraw_display();
}

/*
 *-------------------------------------
 * the main callback loop (1 sec)
 *-------------------------------------
 */

static gint update_display()
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

		if (!get_stats (&rx, &tx))
			{
			old_rx = old_tx = 0;
			}
		else
			{
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
				update_tooltip(TRUE,rx,tx);
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

static void create_background_pixmap()
{
	if (display) gdk_pixmap_unref(display);

	if (show_extra_info)
		{
		display = gdk_pixmap_new(display_area->window, 30, 46, -1);
		draw_shadow_box(display, 0, 0, 30, 46, FALSE, applet->style->bg_gc[GTK_STATE_NORMAL]);
		draw_shadow_box(display, 1, 1, 18, 20, TRUE, applet->style->black_gc);
		draw_shadow_box(display, 1, 22, 28, 11, TRUE, applet->style->black_gc);
		draw_shadow_box(display, 1, 34, 28, 11, TRUE, applet->style->black_gc);
		}
	else if (panel_verticle)
		{
		display = gdk_pixmap_new(display_area->window, 30, 20, -1);
		draw_shadow_box(display, 0, 0, 30, 20, FALSE, applet->style->bg_gc[GTK_STATE_NORMAL]);
		draw_shadow_box(display, 1, 1, 18, 18, TRUE, applet->style->black_gc);
		}
	else
		{
		display = gdk_pixmap_new(display_area->window, 20, 30, -1);
		draw_shadow_box(display, 0, 0, 20, 30, FALSE, applet->style->bg_gc[GTK_STATE_NORMAL]);
		draw_shadow_box(display, 1, 11, 18, 18, TRUE, applet->style->black_gc);
		}
}

static void draw_button_light(GdkPixmap *pixmap, gint x, gint y, gint s, gint etched_in, ColorType color)
{
	GdkGC *gc1;
	GdkGC *gc2;

	s -= 8;

	switch (color)
		{
		case COLOR_RX:
			gdk_gc_set_foreground( gc, &rx_color );
			break;
		case COLOR_RX_BG:
			gdk_gc_set_foreground( gc, &rx_bgcolor );
			break;
		case COLOR_TX:
			gdk_gc_set_foreground( gc, &tx_color );
			break;
		case COLOR_TX_BG:
		default:
			gdk_gc_set_foreground( gc, &tx_bgcolor );
			break;
		}

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

static void update_pixmaps()
{
	GtkStyle *style;
	style = gtk_widget_get_style(applet);

	if (!digits)
		{
		digits = gdk_pixmap_create_from_xpm_d(display_area->window, NULL,
			&style->bg[GTK_STATE_NORMAL], (gchar **)digits_xpm);
		}
	if (!button_on) button_on = gdk_pixmap_new(display_area->window, 10, 10, -1);
	if (!button_off) button_off = gdk_pixmap_new(display_area->window, 10, 10, -1);
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

	draw_button_light(button_on, 0, 0, 10, TRUE, COLOR_TX);
	draw_button_light(button_off, 0, 0, 10, TRUE, COLOR_TX_BG);

	if (!lights) lights = gdk_pixmap_new(display_area->window, 9, 36, -1);

	gdk_draw_rectangle(lights, applet->style->bg_gc[GTK_STATE_NORMAL], TRUE, 0, 0, 9, 36);
	draw_button_light(lights, 0, 0, 9, FALSE, COLOR_TX_BG);
	draw_button_light(lights, 0, 9, 9, FALSE, COLOR_TX);
	draw_button_light(lights, 0, 18, 9, FALSE, COLOR_RX_BG);
	draw_button_light(lights, 0, 27, 9, FALSE, COLOR_RX);
}

static void setup_colors()
{
	GdkColormap *colormap;

        colormap = gtk_widget_get_colormap(display_area);

	gdk_color_parse("#FF0000", &rx_color);
	gdk_color_alloc(colormap, &rx_color);

	gdk_color_parse("#4D0000", &rx_bgcolor);
	gdk_color_alloc(colormap, &rx_bgcolor);

	gdk_color_parse("#00FF00", &tx_color);
        gdk_color_alloc(colormap, &tx_color);

	gdk_color_parse("#004D00", &tx_bgcolor);
        gdk_color_alloc(colormap, &tx_bgcolor);

	gc = gdk_gc_new( display_area->window );
        gdk_gc_copy( gc, display_area->style->white_gc );
}

void reset_orientation(void)
{
#ifdef HAVE_PANEL_PIXEL_SIZE
	if (((orient == ORIENT_LEFT || orient == ORIENT_RIGHT) && sizehint >= PIXEL_SIZE_STANDARD) ||
	    ((orient == ORIENT_UP || orient == ORIENT_DOWN) && sizehint < PIXEL_SIZE_STANDARD))
#else
	if (orient == ORIENT_LEFT || orient == ORIENT_RIGHT)
#endif
		{
		panel_verticle = TRUE;
		}
	else
		{
		panel_verticle = FALSE;
		}

	create_background_pixmap();
	update_pixmaps();

	/* resize the applet and set the proper background pixmap */
	if (show_extra_info)
		{
		gtk_widget_set_usize(frame, 46, 46);
		gtk_drawing_area_size(GTK_DRAWING_AREA(display_area), 30, 46);
		gtk_widget_set_usize(button,16,46);
		gtk_fixed_move(GTK_FIXED(frame),display_area,16,0);
		gtk_fixed_move(GTK_FIXED(frame),button,0,0);
		}
	else if (panel_verticle)
		{
		gtk_widget_set_usize(frame, 46, 20);
		gtk_drawing_area_size(GTK_DRAWING_AREA(display_area),30,20);
		gtk_widget_set_usize(button,16,20);
		gtk_fixed_move(GTK_FIXED(frame),display_area,16,0);
		gtk_fixed_move(GTK_FIXED(frame),button,0,0);
		}
	else
		{
		gtk_widget_set_usize(frame, 20, 46);
		gtk_drawing_area_size(GTK_DRAWING_AREA(display_area),20,30);
		gtk_widget_set_usize(button,20,16);
		gtk_fixed_move(GTK_FIXED(frame),display_area,0,0);
		gtk_fixed_move(GTK_FIXED(frame),button,0,30);
		}
	/* we set the lights to off so they will be correct on the next update */
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
	property_save(privcfgpath);
        return FALSE;
        widget = NULL;
        globcfgpath = NULL;
}

int main (int argc, char *argv[])
{
	int i;

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

	applet_widget_init("modemlights_applet", VERSION, argc, argv,
				    NULL, 0, NULL);

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

#ifdef HAVE_PANEL_PIXEL_SIZE
	sizehint = PIXEL_SIZE_STANDARD;
#endif

	/* open ip socket */
	if ((ip_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		{
		g_print("could not open an ip socket\n");
		return 1;
		}


	applet = applet_widget_new("modemlights_applet");
	if (!applet)
		g_error("Can't create applet!\n");

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

	applet_widget_add(APPLET_WIDGET(applet), frame);
	gtk_widget_realize(applet);
	gtk_widget_realize(display_area);

	setup_colors();
	update_pixmaps();
	gtk_signal_connect(GTK_OBJECT(applet),"change_orient",
				GTK_SIGNAL_FUNC(applet_change_orient),
				NULL);
#ifdef HAVE_PANEL_PIXEL_SIZE
	gtk_signal_connect(GTK_OBJECT(applet),"change_pixel_size",
				GTK_SIGNAL_FUNC(applet_change_pixel_size),
				NULL);
#endif

	button_pixmap = gtk_pixmap_new(button_off, button_mask);
	gtk_container_add(GTK_CONTAINER(button), button_pixmap);
	gtk_widget_show(button_pixmap);

	update_tooltip(FALSE,0,0);
	gtk_widget_show(applet);

	/* by now we know the geometry */
	setup_done = TRUE;
	reset_orientation();

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

