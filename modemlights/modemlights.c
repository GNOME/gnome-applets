/*#####################################################*/
/*##           modemlights applet 0.2.0 alpha        ##*/
/*#####################################################*/

#include "modemlights.h"

#include "backgrnd.xpm"
#include "backgrnd_s.xpm"
#include "lights.xpm"

/* how many times per second to update the lights (1 - 20) */
gint UPDATE_DELAY = 10;

/* the modem lock file */
gchar lock_file[256];

static GtkWidget *applet;
static GtkWidget *frame;
static GtkWidget *display_area;
static GdkPixmap *display;
static GdkPixmap *display_back;
static GdkPixmap *display_back_s;
static GdkPixmap *lights;

static int update_timeout_id = FALSE;
static int ip_socket;

static PanelOrientType orient;

static void about_cb (AppletWidget *widget, gpointer data)
{
	GtkWidget *about;
	gchar *authors[2];
	gchar version[32];

	sprintf(version,"%d.%d.%d",MODEMLIGHTS_APPLET_VERSION_MAJ,
		MODEMLIGHTS_APPLET_VERSION_MIN, MODEMLIGHTS_APPLET_VERSION_REV);

	authors[0] = "John Ellis (gqview@geocities.com)";
	authors[1] = NULL;

        about = gnome_about_new ( _("Modem Lights Applet"), version,
			"(C) 1998",
			authors,
			_("Released under the GNU general public license.\n"
			"A modem status indicator. "
			"Lights in order from the top or left are RX, TX, and CD."),
			NULL);
	gtk_widget_show (about);
}

static int Modem_on()
{
	FILE *f = 0;

	f = fopen(lock_file, "r");

	if(!f) return FALSE;

	fclose(f);
	return TRUE;
}

static void redraw_display()
{
	gdk_window_set_back_pixmap(display_area->window,display,FALSE);
	gdk_window_clear(display_area->window);
}

static void draw_light(int lit,int x,int y)
{
	/* if the orientation is sideways (left or right panel), we swap x and y */
	if (orient == ORIENT_LEFT || orient == ORIENT_RIGHT)
		{
		int t;
		t = y;
		y = x;
		x = t;
		}

	/* draw the light */
	if (lit)
		gdk_draw_pixmap (display,display_area->style->fg_gc[GTK_WIDGET_STATE(display_area)], lights, 0, 10, x, y, 10, 10);
	else
		gdk_draw_pixmap (display,display_area->style->fg_gc[GTK_WIDGET_STATE(display_area)], lights, 0, 0, x, y, 10, 10);
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
		draw_light(rx,0,0);
		redraw_required = TRUE;
		}
	if (tx != o_tx)
		{
		o_tx = tx;
		draw_light(tx,0,10);
		redraw_required = TRUE;
		}
	if (cd != o_cd)
		{
		o_cd = cd;
		draw_light(cd,0,30);
		redraw_required = TRUE;
		}
	if (redraw_required) redraw_display();
}

static gint update_display()
{
	static int old_rx,old_tx;
	int rx, tx;
	int light_rx = FALSE;
	int light_tx = FALSE;
	struct 	ifreq ifreq;
	struct 	ppp_stats stats;


	if (Modem_on())
		{
		memset(&ifreq, 0, sizeof(ifreq));
		strcpy(ifreq.ifr_ifrn.ifrn_name, "ppp0");
		ifreq.ifr_ifru.ifru_data = (caddr_t)&stats;
		if ((ioctl(ip_socket,SIOCDEVPRIVATE,(caddr_t)&ifreq) < 0))
			{
			g_print("ioctl failed\n");
			old_tx = old_rx = 0;
			}
		else
			{
			rx = stats.p.ppp_ibytes;
			tx = stats.p.ppp_obytes;
			if (rx > old_rx) light_rx = TRUE;
			if (tx > old_tx) light_tx = TRUE;
			}

		update_lights(light_rx,light_tx,TRUE);
		old_rx = rx;
		old_tx = tx;
		}
	else
		update_lights(FALSE,FALSE,FALSE);

	return TRUE;
}

/* start or change the update callback timeout interval */
void start_callback_update()
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

}

static void applet_change_orient(GtkWidget *w, PanelOrientType o, gpointer data)
{
	/* resize the applet and set the proper background pixmap */
	orient = o;

	if (orient == ORIENT_LEFT || orient == ORIENT_RIGHT)
		{
		display = gdk_pixmap_new(display_area->window,40,10,-1);
		gtk_widget_set_usize(frame, 44, 14);
		gtk_drawing_area_size(GTK_DRAWING_AREA(display_area),40,10);
		gdk_draw_pixmap(display,display_area->style->fg_gc[GTK_WIDGET_STATE(display_area)],
			display_back_s, 0, 0, 0, 0, 40, 10);
		}
	else
		{
		display = gdk_pixmap_new(display_area->window,10,40,-1);
		gtk_widget_set_usize(frame, 14, 44);
		gtk_drawing_area_size(GTK_DRAWING_AREA(display_area),10,40);
		gdk_draw_pixmap(display,display_area->style->fg_gc[GTK_WIDGET_STATE(display_area)],
			display_back, 0, 0, 0, 0, 10, 40);
		}
	/* we set the lights to off so they will be correct on the next update */
	update_lights(FALSE, FALSE, FALSE);


}

static gint applet_session_save(GtkWidget *widget, char *cfgpath, char *globcfgpath)
{
	property_save(cfgpath);
        return FALSE;
}

int main (int argc, char *argv[])
{
	applet_widget_init_defaults("modemlights_applet", NULL, argc, argv, 0,
				    NULL,argv[0]);

	strcpy(lock_file,"/var/lock/LCK..modem");
	orient = ORIENT_UP;

	/* open ip socket */
	if ((ip_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		{
		g_print("could not open an ip socket\n");
		return 1;
		}


	applet = applet_widget_new();
	if (!applet)
		g_error("Can't create applet!\n");

	property_load(APPLET_WIDGET(applet)->cfgpath);

	/* frame for all widgets */
	frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_OUT);
	gtk_widget_show(frame);

	display_area = gtk_drawing_area_new();
	gtk_drawing_area_size(GTK_DRAWING_AREA(display_area),10,40);
	gtk_container_add(GTK_CONTAINER(frame),display_area);
	gtk_widget_show(display_area);

	applet_widget_add(APPLET_WIDGET(applet), frame);

	gtk_widget_realize(display_area);

	display = gdk_pixmap_new(display_area->window,10,40,-1);

	create_pixmaps();

	gtk_widget_show(frame);
	gtk_widget_show(applet);

	gtk_signal_connect(GTK_OBJECT(applet),"session_save",
		GTK_SIGNAL_FUNC(applet_session_save), NULL);

	applet_widget_register_callback(APPLET_WIDGET(applet),
					"properties",
					_("Properties..."),
					property_show,
					NULL);
	applet_widget_register_callback(APPLET_WIDGET(applet),
					"about",
					_("About..."),
					about_cb, NULL);

	gdk_draw_pixmap     (display,display_area->style->fg_gc[GTK_WIDGET_STATE(display_area)],
		display_back, 0, 0, 0, 0, 10, 40);

	gtk_signal_connect(GTK_OBJECT(applet),"change_orient",
				GTK_SIGNAL_FUNC(applet_change_orient),
				NULL);

	redraw_display();

	start_callback_update();

	applet_widget_gtk_main();
	return 0;
}
