/*#####################################################*/
/*##           modemlights applet 0.3.2              ##*/
/*#####################################################*/

#include "modemlights.h"

#include "backgrnd.xpm"
#include "backgrnd_s.xpm"
#include "lights.xpm"
#include "button_off.xpm"
#include "button_on.xpm"

#include <errno.h>
#include <ctype.h>

#ifdef __linux__
#include <linux/isdn.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>

static unsigned long *isdn_stats = NULL;
#endif

gint UPDATE_DELAY = 5;		/* status lights update interval in Hz (1 - 20) */
gchar *lock_file;		/* the modem lock file */
gchar *device_name;		/* the device name eg:ppp0 */
gchar *command_connect;		/* connection commands */
gchar *command_disconnect;
gint ask_for_confirmation = TRUE;	/* do we ask for confirmation? */
gint use_ISDN = FALSE;		/* do we use ISDN? */

GtkWidget *applet;
static GtkWidget *frame;
static GtkWidget *display_area;
static GtkWidget *button;
static GtkWidget *button_pixmap;
static GdkPixmap *display;
static GdkPixmap *display_back;
static GdkPixmap *display_back_s;
static GdkPixmap *lights;
static GdkPixmap *button_on;
static GdkPixmap *button_off;
static GdkGC *gc;
static GdkColor rxcolor;
static GdkColor txcolor;

static int update_timeout_id = FALSE;
static int ip_socket;
static int load_hist[120];
static int load_hist_pos = 0;
static int load_hist_rx[20];
static int load_hist_tx[20];

static int confirm_dialog = FALSE;

static PanelOrientType orient;

static void about_cb (AppletWidget *widget, gpointer data);
static int is_Modem_on();
static int is_ISDN_on();
static int is_connected();
static void command_connect_cb( gint button, gpointer data);
static void command_disconnect_cb( gint button, gpointer data);
static void dial_cb();
static void update_tooltip(int connected, int rx, int tx);
static void redraw_display();
static void draw_load(int rxbytes,int txbytes);
static void draw_light(int lit,int x,int y);
static void update_lights(int rx,int tx,int cd);
static gint update_display();
static void create_pixmaps();
static void setup_colors();
static void applet_change_orient(GtkWidget *w, PanelOrientType o, gpointer data);
static gint applet_save_session(GtkWidget *widget, char *privcfgpath, char *globcfgpath);

static int get_modem_stats(int *in, int *out);
static int get_ISDN_stats(int *in, int *out);
static int get_stats(int *in, int *out);

static void about_cb (AppletWidget *widget, gpointer data)
{
	GtkWidget *about;
	const gchar *authors[8];
	gchar version[32];

	sprintf(version,"%d.%d.%d",MODEMLIGHTS_APPLET_VERSION_MAJ,
		MODEMLIGHTS_APPLET_VERSION_MIN, MODEMLIGHTS_APPLET_VERSION_REV);

	authors[0] = "John Ellis <johne@bellatlantic.net>";
	authors[1] = "Martin Baulig <martin@home-of-linux.org> - ISDN";
	authors[2] = NULL;

        about = gnome_about_new ( _("Modem Lights Applet"), version,
			"(C) 1998",
			authors,
			_("Released under the GNU general public license.\n"
			"A modem status indicator and dialer."
			"Lights in order from the top or left are RX and TX"),
			NULL);
	gtk_widget_show (about);
}

static int is_Modem_on()
{
	FILE *f = 0;

	f = fopen(lock_file, "r");

	if(!f) return FALSE;

	fclose(f);
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
#ifndef __FreeBSD__
	strncpy(ifreq.ifr_ifrn.ifrn_name, device_name, IFNAMSIZ);
#else
	strncpy(ifreq.ifr_name, device_name, IFNAMSIZ);
#endif /* __FreeBSD__ */
	ifreq.ifr_ifru.ifru_data = (caddr_t)&stats;
#ifndef __FreeBSD__
	if ((ioctl(ip_socket,SIOCDEVPRIVATE,(caddr_t)&ifreq) < 0))
#else
		if ((ioctl(ip_socket,SIOCGPPPSTATS,(caddr_t)&ifreq) < 0))
#endif /* __FreeBSD__ */
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

static void command_connect_cb( gint button, gpointer data)
{
	confirm_dialog = FALSE;
	if (!button) gnome_execute_shell(NULL, command_connect);
}

static void command_disconnect_cb( gint button, gpointer data)
{
	confirm_dialog = FALSE;
	if (!button) gnome_execute_shell(NULL, command_disconnect);
}

static void dial_cb()
{
	if (is_connected()) {
	  if (ask_for_confirmation) {
	    if (confirm_dialog) return;
	    confirm_dialog = TRUE;
	    gnome_question_dialog (_("You are currently connected.\nDo you want to disconnect?"),
				   (GnomeReplyCallback)command_disconnect_cb,
				   NULL);
	  } else {
	    system(command_disconnect);
	    confirm_dialog = FALSE;
	  }
	} else {
	  if (ask_for_confirmation) {
	    if (confirm_dialog) return;
	    confirm_dialog = TRUE;
	    gnome_question_dialog (_("Do you want to connect?"),
				   (GnomeReplyCallback)command_connect_cb,
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
		sprintf(text,"%#.1fM / %#.1fM",(float)rx / 1000000, (float)tx / 1000000);
		}
	else
		strcpy(text, _("not connected"));

	applet_widget_set_widget_tooltip(APPLET_WIDGET(applet),button,text);
}

static void redraw_display()
{
	gdk_window_set_back_pixmap(display_area->window,display,FALSE);
	gdk_window_clear(display_area->window);
}

static void draw_load(int rxbytes,int txbytes)
{
	int load_max = 0;
	int i;
	int x,y;
	float bytes_per_dot;

	if (orient == ORIENT_LEFT || orient == ORIENT_RIGHT)
		{
		x = 2;
		y = 17;
		}
	else
		{
		x = 2;
		y = 27;
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

	if (load_max < 16)
		bytes_per_dot = 1.0;
	else
		bytes_per_dot = (float)load_max / 15;

	gdk_draw_rectangle(display, display_area->style->black_gc, TRUE, x, y - 15, 16, 16);

	gdk_gc_set_foreground( gc, &rxcolor );
	for (i=0;i<16;i++)
		{
		if( load_hist_rx[i] )
			gdk_draw_line(display, gc, x+i, y,
				x+i, y - ((float)load_hist_rx[i] / bytes_per_dot ) + 1);
		}

	gdk_gc_set_foreground( gc, &txcolor );
	for (i=0;i<16;i++)
		{
		if( load_hist_tx[i] )
			gdk_draw_line(display, gc, x+i, y,
				x+i, y - ((float)load_hist_tx[i] / bytes_per_dot ) + 1);
		}

	redraw_display();
}

static void draw_light(int lit,int x,int y)
{
	/* if the orientation is sideways (left or right panel), we swap x and y */
	if (orient == ORIENT_LEFT || orient == ORIENT_RIGHT)
		{
		int t;
		t = y;
		y = x;
		x = 21 - t;
		}

	/* draw the light */
	if (lit)
		gdk_draw_pixmap (display,display_area->style->fg_gc[GTK_WIDGET_STATE(display_area)], lights, 0, 9, x, y, 9, 9);
	else
		gdk_draw_pixmap (display,display_area->style->fg_gc[GTK_WIDGET_STATE(display_area)], lights, 0, 0, x, y, 9, 9);
}

/* to minimize drawing (pixmap manipulations) we only draw a light if it has changed */
static void update_lights(int rx,int tx,int cd)
{
	static int o_rx = FALSE;
	static int o_tx = FALSE;
	static int o_cd = FALSE;
	int redraw_required = FALSE;

	if (rx != o_rx)
		{
		o_rx = rx;
		draw_light(rx,1,1);
		redraw_required = TRUE;
		}
	if (tx != o_tx)
		{
		o_tx = tx;
		draw_light(tx,10,1);
		redraw_required = TRUE;
		}
	if (cd != o_cd)
		{
		o_cd = cd;
		if (cd)
			gtk_pixmap_set(GTK_PIXMAP(button_pixmap), button_on, NULL);
		else
			gtk_pixmap_set(GTK_PIXMAP(button_pixmap), button_off, NULL);
		}
	if (redraw_required) redraw_display();
}

static gint update_display()
{
	static int old_rx,old_tx;
	static int load_count;
	static int modem_was_on = FALSE;
	int rx, tx;
	int light_rx = FALSE;
	int light_tx = FALSE;

	load_count++;

	if (is_connected()) {
		if (!get_stats (&rx, &tx)) {
			old_rx = old_tx = 0;
		} else {
			if (rx > old_rx) light_rx = TRUE;
			if (tx > old_tx) light_tx = TRUE;
		}
		
		update_lights(light_rx,light_tx,TRUE);
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
		update_lights(FALSE,FALSE,FALSE);
		if (modem_was_on)
			{
			update_tooltip(FALSE,0,0);
			modem_was_on = FALSE;
			}
		}

	return TRUE;
}

/* start or change the update callback timeout interval */
void start_callback_update(void)
{
	gint delay;
	delay = 1000 / UPDATE_DELAY;
	if (update_timeout_id) gtk_timeout_remove(update_timeout_id);
	update_timeout_id = gtk_timeout_add(delay, (GtkFunction)update_display, NULL);

}

static void create_pixmaps()
{
	GdkBitmap *mask;
	GtkStyle *style;
	style = gtk_widget_get_style(applet);

	display_back = gdk_pixmap_create_from_xpm_d(display_area->window, &mask,
		&style->bg[GTK_STATE_NORMAL], (gchar **)backgrnd_xpm);
	display_back_s = gdk_pixmap_create_from_xpm_d(display_area->window, &mask,
		&style->bg[GTK_STATE_NORMAL], (gchar **)backgrnd_s_xpm);
	lights = gdk_pixmap_create_from_xpm_d(display_area->window, &mask,
		&style->bg[GTK_STATE_NORMAL], (gchar **)lights_xpm);
	button_off = gdk_pixmap_create_from_xpm_d(applet->window, &mask,
		&style->bg[GTK_STATE_NORMAL], (gchar **)button_off_xpm);
	button_on = gdk_pixmap_create_from_xpm_d(applet->window, &mask,
		&style->bg[GTK_STATE_NORMAL], (gchar **)button_on_xpm);

}

static void setup_colors()
{
	GdkColormap *colormap;

        colormap = gtk_widget_get_colormap(display_area);

	gdk_color_parse("#FF0000", &rxcolor);
	gdk_color_alloc(colormap, &rxcolor);

	gdk_color_parse("#00FF00", &txcolor);
        gdk_color_alloc(colormap, &txcolor);

	gc = gdk_gc_new( display_area->window );
        gdk_gc_copy( gc, display_area->style->white_gc );
}

static void applet_change_orient(GtkWidget *w, PanelOrientType o, gpointer data)
{
	/* resize the applet and set the proper background pixmap */
	static int first_call_flag = FALSE;
	orient = o;

	/* catch 22 with the display_area realize and the signal connect for the orient
	   first time through only needs to get the orient, errors if anything is drawn */
	if (!first_call_flag)
		{
		first_call_flag = TRUE;
		return;
		}

	if (orient == ORIENT_LEFT || orient == ORIENT_RIGHT)
		{
		gtk_widget_set_usize(frame, 46, 20);
		display = gdk_pixmap_new(display_area->window,30,20,-1);
		gtk_drawing_area_size(GTK_DRAWING_AREA(display_area),30,20);
		gdk_draw_pixmap(display,display_area->style->fg_gc[GTK_WIDGET_STATE(display_area)],
			display_back_s, 0, 0, 0, 0, 30, 20);
		gtk_widget_set_usize(button,16,20);
		gtk_fixed_move(GTK_FIXED(frame),display_area,16,0);
		gtk_fixed_move(GTK_FIXED(frame),button,0,0);
		}
	else
		{
		gtk_widget_set_usize(frame, 20, 46);
		display = gdk_pixmap_new(display_area->window,20,30,-1);
		gtk_drawing_area_size(GTK_DRAWING_AREA(display_area),20,30);
		gdk_draw_pixmap(display,display_area->style->fg_gc[GTK_WIDGET_STATE(display_area)],
			display_back, 0, 0, 0, 0, 20, 30);
		gtk_widget_set_usize(button,20,16);
		gtk_fixed_move(GTK_FIXED(frame),display_area,0,0);
		gtk_fixed_move(GTK_FIXED(frame),button,0,30);
		}
	/* we set the lights to off so they will be correct on the next update */
	update_lights(FALSE, FALSE, FALSE);
	redraw_display();
}

static gint applet_save_session(GtkWidget *widget, char *privcfgpath, char *globcfgpath)
{
	property_save(privcfgpath);
        return FALSE;
}

int main (int argc, char *argv[])
{
	GtkStyle *style;
	GdkColor button_color = { 0, 0x7F7F, 0x7F7F, 0x7F7F };
	GdkColor button_color1 = { 0, 0x7A7A, 0x7A7A, 0x7A7A };
	GdkColor button_color2 = { 0, 0x8A8A, 0x8A8A, 0x8A8A };
	int i;

	for (i=0;i<119;i++)
		load_hist[i] = 0;

	for (i=0;i<19;i++)
		{
		load_hist_rx[i] = 0;
		load_hist_tx[i] = 0;
		}

	applet_widget_init("modemlights_applet", VERSION, argc, argv,
				    NULL, 0, NULL);

	lock_file = g_strdup("/var/lock/LCK..modem");
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

	property_load(APPLET_WIDGET(applet)->privcfgpath);

	/* frame for all widgets */
	frame = gtk_fixed_new();
	gtk_widget_show(frame);

	display_area = gtk_drawing_area_new();
	gtk_drawing_area_size(GTK_DRAWING_AREA(display_area),20,30);
	gtk_fixed_put(GTK_FIXED(frame),display_area,0,0);
	gtk_widget_show(display_area);

	button = gtk_button_new();

	/* set button's color */
	style = gtk_style_new();
        style->bg[GTK_STATE_NORMAL] = button_color;
        style->bg[GTK_STATE_ACTIVE] = button_color1;
        style->bg[GTK_STATE_PRELIGHT] = button_color2;
        gtk_widget_set_style(button, style);

	gtk_widget_set_usize(button,20,16);
	gtk_fixed_put(GTK_FIXED(frame),button,0,30);
	gtk_signal_connect(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(dial_cb),NULL);
	gtk_widget_show(button);

	gtk_signal_connect(GTK_OBJECT(applet),"change_orient",
				GTK_SIGNAL_FUNC(applet_change_orient),
				NULL);
	applet_widget_add(APPLET_WIDGET(applet), frame);
	gtk_widget_realize(applet);
	gtk_widget_realize(display_area);

	setup_colors();
	create_pixmaps();
	applet_change_orient(applet, orient, NULL);

	button_pixmap = gtk_pixmap_new(button_off, NULL);
	gtk_container_add(GTK_CONTAINER(button), button_pixmap);
	gtk_widget_show(button_pixmap);

	update_tooltip(FALSE,0,0);
	gtk_widget_show(applet);

	gtk_signal_connect(GTK_OBJECT(applet),"save_session",
		GTK_SIGNAL_FUNC(applet_save_session), NULL);

	applet_widget_register_stock_callback(APPLET_WIDGET(applet),
					      "properties",
					      GNOME_STOCK_MENU_PROP,
					      _("Properties..."),
					      property_show,
					      NULL);
	applet_widget_register_stock_callback(APPLET_WIDGET(applet),
					      "about",
					      GNOME_STOCK_MENU_ABOUT,
					      _("About..."),
					      about_cb, NULL);

	start_callback_update();

	applet_widget_gtk_main();
	return 0;
}
