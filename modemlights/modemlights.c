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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "modemlights.h"
#include <panel-applet.h>
#include <egg-screen-help.h>
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
#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>

#ifndef ISDN_MAX_CHANNELS
#define ISDN_MAX_CHANNELS 64
#endif

#ifndef IIOCGETCPS
#define IIOCGETCPS  _IO('I',21)
#endif

#endif

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

static void about_cb (BonoboUIComponent *uic,
		      MLData       *mldata,
		      const gchar       *verbname)
{
	PanelApplet *applet = PANEL_APPLET (mldata->applet);
	GdkPixbuf 	 *pixbuf;
	GError		 *error = NULL;
	gchar		 *file;
	
	static const gchar *authors[] = {
		"John Ellis <johne@bellatlantic.net>",
		"Martin Baulig <martin@home-of-linux.org> - ISDN",
		NULL
	};

	const gchar *documenters[] = {
	        "Chee Bin HOH <cbhoh@gnome.org>",
		NULL
	};

	const gchar *translator_credits = _("translator_credits");

	if (mldata->about_dialog) {
		gtk_window_set_screen (GTK_WINDOW (mldata->about_dialog),
				       gtk_widget_get_screen (GTK_WIDGET (applet)));
		
		gtk_window_present (GTK_WINDOW (mldata->about_dialog));
		return;
	}

	file = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_PIXMAP,
					  "gnome-modem.png", FALSE, NULL);
	pixbuf = gdk_pixbuf_new_from_file (file, &error);
	g_free (file);
	
	if (error) {
		g_warning (G_STRLOC ": cannot open %s: %s", file, error->message);
		g_error_free (error);
	}

	mldata->about_dialog = gnome_about_new ( _("Modem Lights"), VERSION,
						"(C) 2000",
						_("Released under the GNU general public license.\n"
						"A modem status indicator and dialer.\n"
						"Lights in order from the top or left are Send data and Receive data."),
						authors,
						documenters,
						strcmp (translator_credits, "translator_credits") != 0 ? translator_credits : NULL,
						pixbuf);

	if (pixbuf)
		g_object_unref (pixbuf);

	gtk_window_set_screen (GTK_WINDOW (mldata->about_dialog),
			       gtk_widget_get_screen (GTK_WIDGET (applet)));

	gtk_window_set_wmclass (GTK_WINDOW (mldata->about_dialog),
				"modem lights", "Modem Lights");

	g_signal_connect (G_OBJECT (mldata->about_dialog), "destroy",
			  G_CALLBACK (gtk_widget_destroyed), &mldata->about_dialog);

	gtk_widget_show (mldata->about_dialog);
}

static int is_Modem_on(MLData *mldata)
{
	FILE *f = 0;
	gchar buf[64];
	pid_t pid = -1;

	f = fopen(mldata->lock_file, "r");

	if(!f) return FALSE;

	if (mldata->verify_lock_file)
		{
		if (fgets(buf, sizeof(buf), f) == NULL)
			{
			fclose(f);
			return FALSE;
			}
		}

	fclose(f);

	if (mldata->verify_lock_file)
		{
		pid = (pid_t)strtol(buf, NULL, 10);
		if (pid < 1 || (kill (pid, 0) == -1 && errno != EPERM)) return FALSE;
		}

	return TRUE;
}

static int is_ISDN_on(MLData *mldata)
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

static int is_connected(MLData *mldata)
{
	if (mldata->use_ISDN)
		return is_ISDN_on(mldata);
	else
		return is_Modem_on(mldata);
}

static int get_modem_stats(MLData *mldata, int *in, int *out)
{
	struct 	ifreq ifreq;
	struct 	ppp_stats stats;

	memset(&ifreq, 0, sizeof(ifreq));
	strncpy(ifreq.ifr_name, mldata->device_name, IFNAMSIZ);
	ifreq.ifr_name[IFNAMSIZ-1] = '\0';
	ifreq.ifr_ifru.ifru_data = (caddr_t)&stats;

#ifdef SIOCGPPPSTATS
	if (ioctl(mldata->ip_socket, SIOCGPPPSTATS, (caddr_t)&ifreq) < 0)
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

static int get_ISDN_stats(MLData *mldata, int *in, int *out)
{
#ifdef __linux__
	int fd, i;
	unsigned long *ptr;

	*in = *out = 0;

	if (!mldata->isdn_stats)
		mldata->isdn_stats = g_new0 (unsigned long, ISDN_MAX_CHANNELS  *  2);

	fd = open("/dev/isdninfo", O_RDONLY);

	if (fd < 0)
		return FALSE;

	if ((ioctl (fd, IIOCGETCPS, mldata->isdn_stats) < 0) && (errno != 0)) {
		close (fd);
		
		return FALSE;
	}

	for (i = 0, ptr = mldata->isdn_stats; i < ISDN_MAX_CHANNELS; i++) {
		*in  += *ptr++; *out += *ptr++;
	}

	close (fd);

	return TRUE;

#else
	*in = *out = 0;

	return FALSE;
#endif
}

static int get_stats(MLData *mldata, int *in, int *out)
{
	if (mldata->use_ISDN)
		return get_ISDN_stats(mldata, in, out);
	else
		return get_modem_stats(mldata, in, out);
}

static gint get_ISDN_connect_time(MLData *mldata, gint recalc_start)
{
	/* this is a bad hack just to get some (not very accurate) timing */

	if (recalc_start)
		{
		mldata->start_time = time(0);
		}

	if (mldata->start_time != (time_t)0)
		return (gint)(time(0) - mldata->start_time);
	else
		return -1;
}

static gint get_modem_connect_time(MLData *mldata, gint recalc_start)
{
	struct stat st;

	if (recalc_start)
		{
		if (stat (mldata->lock_file, &st) == 0)
			mldata->start_time = st.st_mtime;
		else
			mldata->start_time = (time_t)0;
		}

	if (mldata->start_time != (time_t)0)
		return (gint)(time(0) - mldata->start_time);
	else
		return -1;
}

static gint get_connect_time(MLData *mldata, gint recalc_start)
{
	if (mldata->use_ISDN)
		return get_ISDN_connect_time(mldata, recalc_start);
	else
		return get_modem_connect_time(mldata, recalc_start);
}

static void
run_response_cb(GtkDialog *dialog, gint id, MLData *mldata)
{
	gtk_widget_destroy(GTK_WIDGET (dialog));

	mldata->run_dialog = NULL;
}

static void execute_command(gchar *command, GtkWidget *parent, MLData *mldata)
{
	gboolean ret;
	GError *error = NULL;

	if (mldata->run_dialog) {
		gtk_window_set_screen(GTK_WINDOW(mldata->run_dialog),
				      gtk_widget_get_screen (parent));

		gtk_window_present(GTK_WINDOW(mldata->run_dialog));

		return;
	}

	ret = g_spawn_command_line_async(command, &error);
	if (!ret) {
		mldata->run_dialog = gtk_message_dialog_new(NULL, 0,
							    GTK_MESSAGE_ERROR,
							    GTK_BUTTONS_CLOSE,
							    error->message);

		gtk_window_set_screen(GTK_WINDOW(mldata->run_dialog),
				      gtk_widget_get_screen(parent));

		gtk_widget_show_all(mldata->run_dialog);
		
		gtk_window_present(GTK_WINDOW(mldata->run_dialog));

		g_signal_connect(mldata->run_dialog, "response",
				 G_CALLBACK(run_response_cb),
				 mldata);
	}
}

static void disconnect_dialog_response(GtkDialog *dialog, gint response, MLData *mldata)
{
		if (response == GTK_RESPONSE_OK)
			execute_command(mldata->command_disconnect, GTK_WIDGET (dialog), mldata);

		gtk_widget_destroy(GTK_WIDGET(dialog));
		mldata->connect_dialog = NULL;
}

static void connect_dialog_response(GtkDialog *dialog, gint response, MLData *mldata)
{
		if (response == GTK_RESPONSE_OK)
			execute_command(mldata->command_connect, GTK_WIDGET (dialog), mldata);

		gtk_widget_destroy(GTK_WIDGET(dialog));
		mldata->connect_dialog = NULL;
}

static void dial_cb(GtkWidget *widget, MLData *mldata)
{
	if (is_connected(mldata)) {

		if (!mldata->ask_for_confirmation) {
			execute_command(mldata->command_disconnect, mldata->applet, mldata);
			return;
		}			

		if (mldata->connect_dialog) {
			gtk_window_set_screen(GTK_WINDOW(mldata->connect_dialog),
					      gtk_widget_get_screen(mldata->applet));

			gtk_window_present(GTK_WINDOW(mldata->connect_dialog));

			return;
		}

		mldata->connect_dialog = gtk_message_dialog_new(NULL, 0,
								GTK_MESSAGE_QUESTION,
								GTK_BUTTONS_NONE,
								_("You are currently connected.\n"
								"Do you want to disconnect?"));

		gtk_window_set_screen(GTK_WINDOW(mldata->connect_dialog),
				      gtk_widget_get_screen(mldata->applet));
		
		gtk_dialog_add_buttons(GTK_DIALOG(mldata->connect_dialog),
				       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				       _("_Disconnect"), GTK_RESPONSE_OK,
				       NULL);

		g_signal_connect(G_OBJECT(mldata->connect_dialog), "response",
				 G_CALLBACK(disconnect_dialog_response), mldata);

		gtk_widget_show_all(mldata->connect_dialog);

	} else {

		if (!mldata->ask_for_confirmation) {
			execute_command(mldata->command_connect, mldata->applet, mldata);
			return;
		}

		if (mldata->connect_dialog) {
			gtk_window_set_screen(GTK_WINDOW(mldata->connect_dialog),
					      gtk_widget_get_screen(mldata->applet));

			gtk_window_present(GTK_WINDOW(mldata->connect_dialog));

			return;
		}
 
		mldata->connect_dialog = gtk_message_dialog_new(NULL, 0,
								GTK_MESSAGE_QUESTION,
								GTK_BUTTONS_NONE,
								_("Do you want to connect?"));

		gtk_window_set_screen(GTK_WINDOW(mldata->connect_dialog),
				      gtk_widget_get_screen(mldata->applet));
		
		gtk_dialog_add_buttons(GTK_DIALOG(mldata->connect_dialog),
				       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				       _("C_onnect"), GTK_RESPONSE_OK,
				       NULL);

		g_signal_connect(GTK_DIALOG(mldata->connect_dialog), "response",
				 G_CALLBACK(connect_dialog_response), mldata);

		gtk_widget_show_all(mldata->connect_dialog);
	}
}

static void update_tooltip(MLData *mldata, int connected, int rx, int tx)
{
	gchar *text;

	if (connected)
		{
		gint t;
		gint t1, t2;

		t = get_connect_time(mldata, FALSE);

		if (t < 360000) {
		  t1 = t / 3600; /* hours */
		  t2 = (t1 - (t1 * 3600)) / 60; /* minutes */
		} else {
		  t1 = (t/3600)/24; /* days */
		  t2 = (t1 - (t1*3600*24)) / 3600; /* hours */
		}
		text = g_strdup_printf(_("%#.1fMb received / %#.1fMb sent / time: %.1d:%.2d"),
				       (float)rx / 1000000, (float)tx / 1000000, t1, t2);
		}
	else
		{
		text = g_strdup(_("not connected"));
		}

	gtk_tooltips_set_tip(mldata->tooltips, mldata->applet, text, NULL);
	g_free(text);

}


/*
 *-------------------------------------
 * display drawing
 *-------------------------------------
 */

static void redraw_display(MLData *mldata)
{
	gdk_window_set_back_pixmap(mldata->display_area->window,mldata->display,FALSE);

	gdk_window_clear(mldata->display_area->window);

}

static void draw_digit(MLData *mldata, gint n, gint x, gint y)
{
	gdk_draw_drawable (mldata->display,
			   mldata->display_area->style->fg_gc[GTK_WIDGET_STATE(mldata->display_area)],
			   mldata->digits, n * 5, 0, x, y, 5, 7);
}

static void draw_timer(MLData *mldata, gint seconds, gint force)
{
	gint x, y;

	if (!mldata->layout_current || mldata->layout_current->time_x < 0) return;

	x = mldata->layout_current->time_x;
	y = mldata->layout_current->time_y;

	if (seconds > -1)
		{
		gint a, b;

		if (seconds >= 3600) seconds /= 60; /* HH:MM, else MM:SS */

		a = seconds / 60;
		b = seconds % 60;

		draw_digit(mldata, a / 10, x, y);
		draw_digit(mldata, a % 10, x+5, y);

		draw_digit(mldata, b / 10, x+15, y);
		draw_digit(mldata, b % 10, x+20, y);

		draw_digit(mldata, 15, x+10, y);
		}
	else
		{
		if (force)
			{
			draw_digit(mldata, 10, x, y);
			draw_digit(mldata, 10, x+5, y);

			draw_digit(mldata, 10, x+15, y);
			draw_digit(mldata, 10, x+20, y);
			}

		draw_digit(mldata, 14, x+10, y);
		}
}

static void draw_bytes(MLData *mldata, gint bytes)
{
	gint x, y;

	if (!mldata->layout_current || mldata->layout_current->bytes_x < 0) return;

	x = mldata->layout_current->bytes_x;
	y = mldata->layout_current->bytes_y;

	if (bytes > -1)
		{
		gint dig;
		if (bytes > 9999)
			{
			bytes /= 1024;
			draw_digit(mldata, 12, x + 20, y);
			}
		else
			{
			draw_digit(mldata, 13, x + 20, y);
			}

		dig = bytes / 1000;
		draw_digit(mldata, dig, x, y);
		bytes %= 1000;
		dig = bytes / 100;
		draw_digit(mldata, dig, x+5, y);
		bytes %= 100;
		dig = bytes / 10;
		draw_digit(mldata, dig, x+10, y);
		bytes %= 10;
		draw_digit(mldata, bytes, x+15, y);
		}
	else
		{
		draw_digit(mldata, 10, x, y);
		draw_digit(mldata, 10, x+5, y);
		draw_digit(mldata, 10, x+10, y);
		draw_digit(mldata, 10, x+15, y);

		draw_digit(mldata, 11, x+20, y);
		}
}

static gint update_extra_info(MLData *mldata, int rx_bytes, gint force)
{	
	gint redraw = FALSE;
	gint new_timer;
	gint new_bytes;

	if (!mldata->layout_current || (mldata->layout_current->bytes_x < 0 && mldata->layout_current->time_x < 0)) return FALSE;

	mldata->update_counter++;
	if (mldata->update_counter < mldata->UPDATE_DELAY && !force) return FALSE;
	mldata->update_counter = 0;

	if (rx_bytes != -1)
		new_timer = get_connect_time(mldata, FALSE);
	else
		new_timer = -1;

	if (new_timer != mldata->old_timer)
		{
		mldata->old_timer = new_timer;
		redraw = TRUE;
		draw_timer(mldata, new_timer, FALSE);
		}
	else if (force)
		{
		redraw = TRUE;
		draw_timer(mldata, mldata->old_timer, TRUE);
		}

	if (rx_bytes == -1 || mldata->old_rx_bytes == -1)
		{
		new_bytes = -1;
		}
	else
		{
		new_bytes = rx_bytes - mldata->old_rx_bytes;
		}

	if (new_bytes != mldata->old_bytes)
		{
		mldata->old_bytes = new_bytes;
		redraw = TRUE;
		draw_bytes(mldata, new_bytes);
		}
	else if (force)
		{
		redraw = TRUE;
		draw_bytes(mldata, mldata->old_bytes);
		}

	mldata->old_rx_bytes = rx_bytes;

	return redraw;
}

static void draw_load(MLData *mldata, int rxbytes, int txbytes)
{
	int load_max = 0;
	int i;
	int x, y, dot_height;
	float bytes_per_dot;

	if (!mldata->layout_current) return;

	x = mldata->layout_current->load_x + 1;
	y = mldata->layout_current->load_y + mldata->layout_current->load_h - 2;
	dot_height = mldata->layout_current->load_h - 2;

	/* sanity check: */
	if (rxbytes <0) rxbytes = 0;
	if (txbytes <0) txbytes = 0;

	mldata->load_hist_pos++;
	if (mldata->load_hist_pos > 119) mldata->load_hist_pos = 0;
	if (txbytes > rxbytes)
		mldata->load_hist[mldata->load_hist_pos] = txbytes;
	else
		mldata->load_hist[mldata->load_hist_pos] = rxbytes;
	for (i=0;i<120;i++)
		if (load_max < mldata->load_hist[i]) load_max = mldata->load_hist[i];

	for (i=0;i<15;i++)
		{
		mldata->load_hist_rx[i] = mldata->load_hist_rx[i+1];
		mldata->load_hist_tx[i] = mldata->load_hist_tx[i+1];
		}
	mldata->load_hist_rx[15] = rxbytes;
	mldata->load_hist_tx[15] = txbytes;

	if (load_max < dot_height)
		bytes_per_dot = 1.0;
	else
		bytes_per_dot = (float)load_max / (dot_height - 1);

	gdk_gc_set_foreground( mldata->gc, &mldata->display_color[COLOR_TEXT_BG] );
	gdk_draw_rectangle(mldata->display, mldata->gc, TRUE, x, y - dot_height + 1, 16, dot_height);

	gdk_gc_set_foreground( mldata->gc, &mldata->display_color[COLOR_RX] );
	for (i=0;i<16;i++)
		{
		if( mldata->load_hist_rx[i] )
			gdk_draw_line(mldata->display, mldata->gc, x+i, y ,
				x+i, y - ((float)mldata->load_hist_rx[i] / bytes_per_dot) + 1);
		}

	gdk_gc_set_foreground( mldata->gc, &mldata->display_color[COLOR_TX] );
	for (i=0;i<16;i++)
		{
		if( mldata->load_hist_tx[i] )
			gdk_draw_line(mldata->display, mldata->gc, x+i, y,
				x+i, y - ((float)mldata->load_hist_tx[i] / bytes_per_dot) + 1);
		}

	redraw_display(mldata);
}

static void draw_light(MLData *mldata, int lit, int x, int y, ColorType color)
{
	gint p;

	if (lit)
		p = 9;
	else
		p = 0;

	if (color == COLOR_RX) p += 18;
	

	gdk_draw_drawable (mldata->display,
			   mldata->display_area->style->fg_gc[GTK_WIDGET_STATE(mldata->display_area)],
			   mldata->lights, 0, p, x, y, 9, 9);
}

static void update_button(MLData *mldata, StatusType type)
{
	switch(type)
		{
		case STATUS_ON:
			gtk_image_set_from_pixmap(GTK_IMAGE(mldata->button_image), mldata->button_on, mldata->button_mask);
			break;
		case STATUS_WAIT:
			gtk_image_set_from_pixmap(GTK_IMAGE(mldata->button_image), mldata->button_wait, mldata->button_mask);
			break;
		default:
			gtk_image_set_from_pixmap(GTK_IMAGE(mldata->button_image), mldata->button_off, mldata->button_mask);
			break;
		}
}

/* to minimize drawing (pixmap manipulations) we only draw a light if it has changed */
static void update_lights(MLData *mldata, int rx, int tx, int cd, int rx_bytes, gint force)
{	
	int redraw_required = FALSE;

	if (!mldata->layout_current) return;

	if (rx != mldata->o_rx || force)
		{
		mldata->o_rx = rx;
		draw_light(mldata, rx , mldata->layout_current->rx_x, mldata->layout_current->rx_y, COLOR_RX);
		redraw_required = TRUE;
		}
	if (tx != mldata->o_tx || force)
		{
		mldata->o_tx = tx;
		draw_light(mldata, tx, mldata->layout_current->tx_x, mldata->layout_current->tx_y, COLOR_TX);
		redraw_required = TRUE;
		}
	if ((cd != mldata->o_cd && !mldata->button_blinking) || force)
		{
		mldata->o_cd = cd;
		update_button(mldata, cd ? STATUS_ON : STATUS_OFF);
		}

	/* we do the extra info redraws here too */
	if (mldata->show_extra_info && update_extra_info(mldata, rx_bytes, force))
		{
		redraw_required = TRUE;
		}

	if (redraw_required) redraw_display(mldata);
}

static gint button_blink_cb(gpointer data)
{
	MLData *mldata = data;
	if (mldata->button_blink_on && mldata->status_wait_blink)
		{
		mldata->button_blink_on = FALSE;
		}
	else
		{
		mldata->button_blink_on = TRUE;
		}
	update_button(mldata, mldata->button_blink_on ? STATUS_WAIT : STATUS_OFF);

	return TRUE;
	data = NULL;
}

static void button_blink(MLData *mldata, gint blink, gint status)
{
	if (mldata->button_blinking == blink) return;

	if (blink)
		{
		if (mldata->button_blink_id == -1)
			{
			mldata->button_blink_id = g_timeout_add(BUTTON_BLINK_INTERVAL, button_blink_cb, mldata);
			}
		}
	else
		{
		if (mldata->button_blink_id != -1)
			{
			g_source_remove(mldata->button_blink_id);
			mldata->button_blink_id = -1;
			}
		}

	mldata->button_blinking = blink;
	mldata->button_blink_on = status;
	if (blink)
		{
		update_button(mldata, mldata->button_blink_on ? STATUS_WAIT : STATUS_OFF);
		}	
	else
		{
		update_button(mldata, mldata->button_blink_on ? STATUS_ON : STATUS_OFF);
		}
}


/*
 *-------------------------------------
 * the main callback loop (1 sec)
 *-------------------------------------
 */

static gint update_display(MLData *mldata)
{	
	int rx, tx;
	int light_rx = FALSE;
	int light_tx = FALSE;

	mldata->load_count++;

	if (is_connected(mldata))
		{
		if (!mldata->last_time_was_connected)
			{
			get_connect_time(mldata, TRUE); /* reset start time */
			mldata->last_time_was_connected = TRUE;
			}

		if (!get_stats (mldata, &rx, &tx) || (rx == 0 && tx == 0))
			{
			mldata->old_rx = mldata->old_tx = 0;
			button_blink(mldata, TRUE, TRUE);
			}
		else
			{
			button_blink(mldata, FALSE, TRUE);
			if (rx > mldata->old_rx) light_rx = TRUE;
			if (tx > mldata->old_tx) light_tx = TRUE;
			}
		
		update_lights(mldata, light_rx, light_tx, TRUE, rx, FALSE);
		if (mldata->load_count > mldata->UPDATE_DELAY * 2)
			{
			mldata->tooltip_counter++;
			if (mldata->tooltip_counter > 10)
				{
				update_tooltip(mldata,TRUE, rx, tx);
				mldata->tooltip_counter = 0;
				}

	/* This is a check to see if the modem was running before the program
	started. if it was, we set the past bytes to the current bytes. Otherwise
	the first load calculation could have a very high number of bytes.
	(the bytes that accumulated before program start will make max_load too high) */
			if (!mldata->modem_was_on)
				{
				mldata->load_rx = rx;
				mldata->load_tx = tx;
				update_tooltip(mldata, TRUE,rx,tx);
				mldata->modem_was_on = TRUE;
				}
		
			mldata->load_count = 0;
			draw_load(mldata, rx - mldata->load_rx, tx - mldata->load_tx);
			mldata->load_rx = rx;
			mldata->load_tx = tx;
			}
		mldata->old_rx = rx;
		mldata->old_tx = tx;
		}
	else
		{
		if (mldata->load_count > mldata->UPDATE_DELAY * 2)
			{
			mldata->load_count = 0;
			draw_load(mldata, 0,0);
			}
		button_blink(mldata, FALSE, FALSE);
		update_lights(mldata, FALSE, FALSE, FALSE, -1, FALSE);
		if (mldata->tooltip_counter > 0)
			{
			update_tooltip(mldata, FALSE, 0, 0);
			mldata->tooltip_counter = 0;
			}
		if (mldata->last_time_was_connected)
			{
			mldata->last_time_was_connected = FALSE;
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
void start_callback_update(MLData *mldata)
{
	gint delay;
	delay = 1000 / mldata->UPDATE_DELAY;
	if (mldata->update_timeout_id) g_source_remove(mldata->update_timeout_id);
	mldata->update_timeout_id = g_timeout_add(delay, (GSourceFunc)update_display, mldata);

}

static void draw_shadow_box(MLData *mldata, GdkPixmap *window, gint x, gint y, gint w, gint h,
			    gint etched_in, GdkGC *bgc)
{
	GdkGC *gc1;
	GdkGC *gc2;

	if (!etched_in)
		{
		gc1 = mldata->applet->style->light_gc[GTK_STATE_NORMAL];
		gc2 = mldata->applet->style->dark_gc[GTK_STATE_NORMAL];
		}
	else
		{
		gc1 = mldata->applet->style->dark_gc[GTK_STATE_NORMAL];
		gc2 = mldata->applet->style->light_gc[GTK_STATE_NORMAL];
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

static void create_background_pixmap(MLData *mldata)
{
	if (!mldata->layout_current) return;

	if (mldata->display) g_object_unref(mldata->display);

	mldata->display = gdk_pixmap_new(mldata->display_area->window,
				 mldata->layout_current->display_w, mldata->layout_current->display_h, -1);

	/* main border */
	draw_shadow_box(mldata, mldata->display, 0, 0, mldata->layout_current->display_w, mldata->layout_current->display_h,
			FALSE, mldata->applet->style->bg_gc[GTK_STATE_NORMAL]);

	/* load border */
	gdk_gc_set_foreground( mldata->gc, &mldata->display_color[COLOR_TEXT_BG] );
	draw_shadow_box(mldata, mldata->display, mldata->layout_current->load_x, mldata->layout_current->load_y,
			mldata->layout_current->load_w, mldata->layout_current->load_h, TRUE, mldata->gc);

	/* text border(s) */
	if (mldata->layout_current->bytes_x >= 0)
		{
		draw_shadow_box(mldata, mldata->display, mldata->layout_current->bytes_x - 1, mldata->layout_current->bytes_y - 2,
				28, mldata->layout_current->merge_extended_box ? 20 : 11,
				TRUE, mldata->gc);
		}
	if (mldata->layout_current->time_x >= 0 && !mldata->layout_current->merge_extended_box)
		{
		draw_shadow_box(mldata, mldata->display, mldata->layout_current->time_x - 1, mldata->layout_current->time_y - 2,
				28, 11, TRUE, mldata->gc);
		}
}

static void draw_button_light(MLData *mldata, GdkPixmap *pixmap, gint x, gint y, gint s, gint etched_in, ColorType color)
{
	GdkGC *gc1;
	GdkGC *gc2;

	s -= 8;

	gdk_gc_set_foreground( mldata->gc, &mldata->display_color[color] );

	if (etched_in)
		{
		gc1 = mldata->applet->style->dark_gc[GTK_STATE_NORMAL];
		gc2 = mldata->applet->style->light_gc[GTK_STATE_NORMAL];
		}
	else
		{
		gc1 = mldata->applet->style->light_gc[GTK_STATE_NORMAL];
		gc2 = mldata->applet->style->dark_gc[GTK_STATE_NORMAL];
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

	gdk_draw_rectangle(pixmap, mldata->gc, TRUE, x + 3, y + 1, 2 + s, 6 + s);
	gdk_draw_rectangle(pixmap, mldata->gc, TRUE, x + 1, y + 3, 6 + s, 2 + s);
	gdk_draw_rectangle(pixmap, mldata->gc, TRUE, x + 2, y + 2, 4 + s, 4 + s);
}

static void pixmap_set_colors(MLData *mldata, GdkPixmap *pixmap, GdkColor *bg, GdkColor *fg, GdkColor *mid)
{
	gint w;
	gint h;
	gint x, y;
	GdkImage *image;
	guint32 bg_pixel = 0x00000000;
	guint32 fg_pixel = 0x00000000;
	gint have_fg = FALSE;

	gdk_drawable_get_size(pixmap, &w, &h);

	image = gdk_drawable_get_image(pixmap, 0, 0, w, h);

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
				gdk_gc_set_foreground( mldata->gc, bg );
				}
			else if (!have_fg || pixel == fg_pixel)
				{
				if (!have_fg)
					{
					have_fg = TRUE;
					fg_pixel = pixel;
					}
				gdk_gc_set_foreground( mldata->gc, fg );
				}
			else
				{
				gdk_gc_set_foreground( mldata->gc, mid );
				}
			gdk_draw_point(pixmap, mldata->gc, x, y);
			}
		}

	g_object_unref(image);
}

static void update_pixmaps(MLData *mldata)
{
	GtkStyle *style;
	style = gtk_widget_get_style(mldata->applet);

	if (!mldata->digits)
		{
		mldata->digits = gdk_pixmap_create_from_xpm_d(mldata->display_area->window, NULL,
			&style->bg[GTK_STATE_NORMAL], (gchar **)digits_xpm);
		}
	/* change digit colors */
	pixmap_set_colors(mldata, mldata->digits,
			  &mldata->display_color[COLOR_TEXT_BG],
			  &mldata->display_color[COLOR_TEXT_FG],
			  &mldata->display_color[COLOR_TEXT_MID]);

	if (!mldata->button_on) mldata->button_on = gdk_pixmap_new(mldata->display_area->window, 10, 10, -1);
	if (!mldata->button_off) mldata->button_off = gdk_pixmap_new(mldata->display_area->window, 10, 10, -1);
	if (!mldata->button_wait) mldata->button_wait = gdk_pixmap_new(mldata->display_area->window, 10, 10, -1);
	if (!mldata->button_mask)
		{
		GdkGC *mask_gc = NULL;

		mldata->button_mask = gdk_pixmap_new(mldata->display_area->window, 10, 10, 1);

		mask_gc = gdk_gc_new(mldata->button_mask);

		gdk_gc_set_foreground(mask_gc, &style->black);
		gdk_draw_rectangle(mldata->button_mask, mask_gc, TRUE, 0, 0, 10, 10);
		gdk_gc_set_foreground(mask_gc, &style->white);
		/* gdk_draw_arc was always off by one in my attempts (?) */
		gdk_draw_rectangle(mldata->button_mask, mask_gc, TRUE, 3, 0, 4, 10);
		gdk_draw_rectangle(mldata->button_mask, mask_gc, TRUE, 0, 3, 10, 4);
		gdk_draw_rectangle(mldata->button_mask, mask_gc, TRUE, 2, 1, 6, 8);
		gdk_draw_rectangle(mldata->button_mask, mask_gc, TRUE, 1, 2, 8, 6);

		g_object_unref(mask_gc);
		}

	draw_button_light(mldata, mldata->button_on, 0, 0, 10, TRUE, COLOR_STATUS_OK);
	draw_button_light(mldata, mldata->button_off, 0, 0, 10, TRUE, COLOR_STATUS_BG);
	draw_button_light(mldata, mldata->button_wait, 0, 0, 10, TRUE, COLOR_STATUS_WAIT);

	if (!mldata->lights) mldata->lights = gdk_pixmap_new(mldata->display_area->window, 9, 36, -1);

	gdk_draw_rectangle(mldata->lights, mldata->applet->style->bg_gc[GTK_STATE_NORMAL], TRUE, 0, 0, 9, 36);
	draw_button_light(mldata, mldata->lights, 0, 0, 9, FALSE, COLOR_TX_BG);
	draw_button_light(mldata, mldata->lights, 0, 9, 9, FALSE, COLOR_TX);
	draw_button_light(mldata, mldata->lights, 0, 18, 9, FALSE, COLOR_RX_BG);
	draw_button_light(mldata, mldata->lights, 0, 27, 9, FALSE, COLOR_RX);
}

static void setup_colors(MLData *mldata)
{
	GdkColormap *colormap;
	gint i;
	gboolean success[COLOR_COUNT];

        colormap = gtk_widget_get_colormap(mldata->display_area);

	if (mldata->allocated)
		{
		gdk_colormap_free_colors(colormap, mldata->display_color, COLOR_COUNT);
		}

	for (i = 0; i < COLOR_COUNT; i++)
		{
		if (!mldata->display_color_text[i]) mldata->display_color_text[i] = g_strdup("#000000");
		gdk_color_parse(mldata->display_color_text[i], &mldata->display_color[i]);
		}
	gdk_colormap_alloc_colors(colormap, mldata->display_color, 
					        COLOR_COUNT, FALSE, TRUE, success);
	for (i = 0; i < COLOR_COUNT; i++)
		{
		if (!success[i]) printf("unable to allocate color %s\n", mldata->display_color_text[i]);
		}

	mldata->allocated = TRUE;

	if (!mldata->gc)
		{
		mldata->gc = gdk_gc_new( mldata->display_area->window );
        	gdk_gc_copy( mldata->gc, mldata->display_area->style->white_gc );
		}

}

void reset_orientation(MLData *mldata)
{
	if (mldata->sizehint >= GNOME_Vertigo_PANEL_MEDIUM)
		{
		if (mldata->show_extra_info)
		        {
		        if (mldata->orient == PANEL_APPLET_ORIENT_LEFT || mldata->orient == PANEL_APPLET_ORIENT_RIGHT)
		              {
			      if (mldata->sizehint >= 74)
			              mldata->layout = LAYOUT_HORIZONTAL_EXTENDED;
			      else
			              mldata->layout = LAYOUT_SQUARE;
			      }
		        else
			      {
			      if (mldata->sizehint >= 58)
			              mldata->layout = LAYOUT_VERTICAL_EXTENDED;
			      else
			              mldata->layout = LAYOUT_SQUARE;
			      }
		        }
		else if (mldata->orient == PANEL_APPLET_ORIENT_LEFT || mldata->orient == PANEL_APPLET_ORIENT_RIGHT)
			{
			mldata->layout = LAYOUT_HORIZONTAL;
			}
		else
			{
			mldata->layout = LAYOUT_VERTICAL;
			}
		}
	else
		{
		if (mldata->orient == PANEL_APPLET_ORIENT_LEFT || mldata->orient == PANEL_APPLET_ORIENT_RIGHT)
			{
			if (mldata->show_extra_info)
				{
				mldata->layout = LAYOUT_VERTICAL_EXTENDED;
				}
			else
				{
				mldata->layout = LAYOUT_VERTICAL;
				}
			}
		else
			{
			if (mldata->show_extra_info)
				{
				mldata->layout = LAYOUT_HORIZONTAL_EXTENDED;
				}
			else
				{
				mldata->layout = LAYOUT_HORIZONTAL;
				}
			}
		}

	if (mldata->layout < LAYOUT_HORIZONTAL || mldata->layout > LAYOUT_SQUARE) mldata->layout = LAYOUT_HORIZONTAL;
	mldata->layout_current = &layout_data[mldata->layout];
	
	create_background_pixmap(mldata);
	update_pixmaps(mldata);

	gtk_widget_set_size_request(mldata->frame, mldata->layout_current->width, mldata->layout_current->height);
	gtk_widget_set_size_request(GTK_WIDGET(mldata->display_area),
			      mldata->layout_current->display_w, mldata->layout_current->display_h);

	gtk_widget_set_size_request(mldata->button, mldata->layout_current->button_w, mldata->layout_current->button_h);
	gtk_fixed_move(GTK_FIXED(mldata->frame), mldata->display_area, mldata->layout_current->display_x, mldata->layout_current->display_y);
	gtk_fixed_move(GTK_FIXED(mldata->frame), mldata->button, mldata->layout_current->button_x, mldata->layout_current->button_y);

	/* we set the lights to off so they will be correct on the next update */
	update_lights(mldata, FALSE, FALSE, FALSE, -1, TRUE);
	redraw_display(mldata);
}

void reset_colors(MLData *mldata)
{
	setup_colors(mldata);
	create_background_pixmap(mldata);
	update_pixmaps(mldata);
	update_lights(mldata, FALSE, FALSE, FALSE, -1, TRUE);
	redraw_display(mldata);
}

/* this is called when the applet's style changes (meaning probably a theme/color change) */
static void applet_style_change_cb(GtkWidget *widget, GtkStyle *previous_style, gpointer data)
{
	MLData *mldata = data;
	reset_orientation(mldata);
	return;
}

static void applet_change_orient(PanelApplet *applet, PanelAppletOrient o, gpointer data)
{
	MLData *mldata = data;
	mldata->orient = o;
	if (mldata->setup_done) reset_orientation(mldata);
	return;
}


static void applet_change_pixel_size(PanelApplet *applet, gint size, gpointer data)
{
	MLData *mldata = data;
	mldata->sizehint = size;
	if (mldata->setup_done) reset_orientation(mldata);
        return;
}

/* This is a hack around the fact that gtk+ doesn't
 * propogate button presses on button2/3.
 */
static gboolean
button_press_hack (GtkWidget      *widget,
                   GdkEventButton *event,
                   GtkWidget      *applet)
{
    if (event->button == 3 || event->button == 2) {
	gtk_propagate_event (applet, (GdkEvent *) event);

	return TRUE;
    }

    return FALSE;
}


static void show_help_cb (BonoboUIComponent *uic,
			  MLData       *mldata,
			  const char        *verbname)
{
	PanelApplet *applet = PANEL_APPLET (mldata->applet);
	egg_help_display_on_screen (
		"modemlights", NULL,
		gtk_widget_get_screen (GTK_WIDGET (applet)),
		NULL);
}

static const BonoboUIVerb modem_applet_menu_verbs [] = {
        BONOBO_UI_UNSAFE_VERB ("Props", property_show),
        BONOBO_UI_UNSAFE_VERB ("Help", show_help_cb),
        BONOBO_UI_UNSAFE_VERB ("About", about_cb),

        BONOBO_UI_VERB_END
};

static void
destroy_cb (GtkWidget *widget, gpointer data)
{
	MLData *mldata = data;

	g_source_remove (mldata->update_timeout_id);
	
	if (mldata->about_dialog)
		gtk_widget_destroy (mldata->about_dialog);
	if (mldata->propwindow)
		gtk_widget_destroy (mldata->propwindow);
	if (mldata->connect_dialog)
		gtk_widget_destroy (mldata->connect_dialog);
	if (mldata->run_dialog)
		gtk_widget_destroy (mldata->run_dialog);
	if (mldata->tooltips)
		g_object_unref (mldata->tooltips);
	g_free (data);
}

static gboolean
modemlights_applet_fill (PanelApplet *applet)
{
	MLData *mldata;
	gint i;
	AtkObject *atk_obj;

	mldata = g_new0 (MLData, 1);
	
	mldata->applet = GTK_WIDGET (applet);
	mldata->layout = LAYOUT_HORIZONTAL;
	mldata->tooltips = gtk_tooltips_new ();
	g_object_ref (mldata->tooltips);
	gtk_object_sink (GTK_OBJECT (mldata->tooltips));
	mldata->button_blinking = FALSE;
	mldata->button_blink_on = 0;
	mldata->button_blink_id = -1;
	mldata->update_timeout_id = FALSE;
	mldata->about_dialog = NULL;
	mldata->setup_done = FALSE;
	mldata->start_time = (time_t)0;
	mldata->old_timer = -1;
	mldata->old_bytes = -1;
	mldata->old_rx_bytes = -1;
	mldata->update_counter = 0;
	mldata->o_rx = FALSE;
	mldata->o_tx = FALSE;
	mldata->o_cd = FALSE;
	mldata->modem_was_on = FALSE;
	mldata->last_time_was_connected = FALSE;
	mldata->allocated = FALSE;
	mldata->isdn_stats = NULL;
	mldata->connect_dialog = NULL;
	mldata->run_dialog = NULL;

	gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gnome-modem.png");
	
	panel_applet_add_preferences (applet, "/schemas/apps/modemlights/prefs", NULL);
		
	mldata->load_hist_pos = 0;
	for (i=0;i<119;i++)
		mldata->load_hist[i] = 0;

	for (i=0;i<19;i++)
		{
		mldata->load_hist_rx[i] = 0;
		mldata->load_hist_tx[i] = 0;
		}
	for (i = 0; i < COLOR_COUNT; i++)
		{
		mldata->display_color_text[i] = NULL;
		mldata->display_color[i] = (GdkColor){ 0, 0, 0, 0 };
		}

	mldata->layout_current = &layout_data[LAYOUT_HORIZONTAL];
	
	if (g_file_test("/dev/modem", G_FILE_TEST_EXISTS))
		mldata->lock_file = g_strdup("/var/lock/LCK..modem");
	else
		mldata->lock_file = g_strdup("/var/lock/LCK..ttyS0");


	mldata->device_name = g_strdup("ppp0");
	mldata->command_connect = g_strdup("pppon");
	mldata->command_disconnect = g_strdup("pppoff");
	mldata->orient = panel_applet_get_orient (applet);

	/* open ip socket */
#ifdef AF_INET6
	if ((mldata->ip_socket = socket(AF_INET6, SOCK_DGRAM, 0)) < 0)
#endif
		{
		if ((mldata->ip_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
			{
			g_print("could not open an ip socket\n");
			return FALSE;
			}
		}


	property_load(mldata);

	/* frame for all widgets */
	mldata->frame = gtk_fixed_new();
	gtk_widget_show(mldata->frame);
	
	gtk_container_add (GTK_CONTAINER (applet), mldata->frame);

	mldata->display_area = gtk_drawing_area_new();
	gtk_widget_set_size_request(GTK_WIDGET(mldata->display_area),5,5);
	gtk_fixed_put(GTK_FIXED(mldata->frame),mldata->display_area,0,0);
	gtk_widget_show(mldata->display_area);

	mldata->button = gtk_button_new();
	gtk_widget_set_size_request(mldata->button,5,5);
	gtk_fixed_put(GTK_FIXED(mldata->frame),mldata->button,5,0);
	g_signal_connect(G_OBJECT(mldata->button), "clicked",
			 G_CALLBACK(dial_cb), mldata);
	g_signal_connect(G_OBJECT (mldata->button), "button_press_event",
			 G_CALLBACK(button_press_hack), GTK_WIDGET (applet));
	gtk_widget_show(mldata->button);

	gtk_widget_realize(GTK_WIDGET (applet));
	gtk_widget_realize(mldata->display_area);
	
	setup_colors(mldata);
	update_pixmaps(mldata);	

	mldata->button_image = gtk_image_new_from_pixmap(mldata->button_off, mldata->button_mask);
	gtk_container_add(GTK_CONTAINER(mldata->button), mldata->button_image);
	gtk_widget_show(mldata->button_image);
	

	g_signal_connect(G_OBJECT(applet),"change_orient",
			 G_CALLBACK(applet_change_orient),
			 mldata);

	g_signal_connect(G_OBJECT(applet),"change_size",
			 G_CALLBACK(applet_change_pixel_size),
			 mldata);

	g_signal_connect(G_OBJECT(applet),"style_set",
			 G_CALLBACK(applet_style_change_cb), mldata);
		
	g_signal_connect (G_OBJECT (applet), "destroy",
			  G_CALLBACK (destroy_cb), mldata);
				
	mldata->sizehint = panel_applet_get_size (PANEL_APPLET (applet));
	
	panel_applet_setup_menu_from_file (PANEL_APPLET (applet),
					   NULL,
					   "GNOME_ModemlightsApplet.xml",
					   NULL,
					   modem_applet_menu_verbs,
					   mldata);

	if (panel_applet_get_locked_down (PANEL_APPLET (applet))) {
		BonoboUIComponent *popup_component;

		popup_component = panel_applet_get_popup_component (PANEL_APPLET (applet));

		bonobo_ui_component_set_prop (popup_component,
					      "/commands/Props",
					      "hidden", "1",
					      NULL);
	}
	update_tooltip(mldata, FALSE, 0, 0);

	/* by now we know the geometry */
	mldata->setup_done = TRUE;

	reset_orientation(mldata);
	
	start_callback_update(mldata);

	atk_obj = gtk_widget_get_accessible (mldata->button);
	if (GTK_IS_ACCESSIBLE (atk_obj))
		atk_object_set_name (atk_obj, _("Modem Lights"));

	gtk_widget_show_all (GTK_WIDGET (applet));
	  
	return TRUE;
}


static gboolean
modemlights_applet_factory (PanelApplet *applet,
			    const gchar *iid,
			    gpointer     data)
{
	gboolean retval;
    
	if (!strcmp (iid, "OAFIID:GNOME_ModemLightsApplet"))
		retval = modemlights_applet_fill (applet); 
    
	return retval;
}

PANEL_APPLET_BONOBO_FACTORY ("OAFIID:GNOME_ModemLightsApplet_Factory",
			     PANEL_TYPE_APPLET,
			     "modemlights",
			     "0",
			     modemlights_applet_factory,
			     NULL)
