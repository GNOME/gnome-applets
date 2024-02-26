/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  Netspeed Applet was writen by Jörgen Scheibengruber <mfcn@gmx.de>
 */

#include "config.h"
#include "netspeed-applet.h"

#include <math.h>
#include <libgnome-panel/gp-utils.h>
#include <gtk/gtk.h>
#include <glib/gi18n-lib.h>

#include "backend.h"
#include "label.h"
#include "preferences.h"

 /* Icons for the interfaces */
static const char* const dev_type_icon[DEV_UNKNOWN + 1] = {
	"netspeed-loopback",         /* DEV_LO */
	"network-wired",             /* DEV_ETHERNET */
	"network-wireless",          /* DEV_WIRELESS */
	"netspeed-ppp",              /* DEV_PPP */
	"netspeed-plip",             /* DEV_PLIP */
	"netspeed-plip",             /* DEV_SLIP */
	"network-workgroup",         /* DEV_UNKNOWN */
};

static const char* wireless_quality_icon[] = {
	"netspeed-wireless-25",
	"netspeed-wireless-50",
	"netspeed-wireless-75",
	"netspeed-wireless-100"
};

static const char IN_ICON[] = "go-down";
static const char OUT_ICON[] = "go-up";
static const char ERROR_ICON[] = "dialog-error";

/* How many old in out values do we store?
 * The value actually shown in the applet is the average
 * of these values -> prevents the value from
 * "jumping around like crazy"
 */
#define OLD_VALUES 5
#define GRAPH_VALUES 180
#define GRAPH_LINES 4

/* A struct containing all the "global" data of the 
 * applet
 */
struct _NetspeedApplet
{
	GpApplet        parent;

	gint            size;

	GtkWidget      *box;
	GtkWidget      *pix_box;
	GtkWidget      *in_box;
	GtkWidget      *in_label;
	GtkWidget      *in_pix;
	GtkWidget      *out_box;
	GtkWidget      *out_label;
	GtkWidget      *out_pix;
	GtkWidget      *sum_box;
	GtkWidget      *sum_label;
	GtkWidget      *dev_pix;
	GtkWidget      *qual_pix;
	GdkPixbuf      *qual_pixbufs[4];

	GtkWidget      *signalbar;

	DevInfo         devinfo;
	gboolean        device_has_changed;

	guint           timeout_id;
	gint            refresh_time;
	gchar          *up_cmd;
	gchar          *down_cmd;
	gboolean        show_sum;
	gboolean        show_bits;
	gboolean        change_icon;
	gboolean        auto_change_device;
	GdkRGBA         in_color;
	GdkRGBA         out_color;
	gint            width;

	GtkWidget      *inbytes_text;
	GtkWidget      *outbytes_text;
	GtkWidget      *details;
	GtkWidget      *preferences;
	GtkDrawingArea *drawingarea;
	
	guint           index_old;
	guint64         in_old[OLD_VALUES];
	guint64         out_old[OLD_VALUES];
	gdouble         max_graph;
	gdouble         in_graph[GRAPH_VALUES];
	gdouble         out_graph[GRAPH_VALUES];
	gint            index_graph;
	
	GtkWidget      *connect_dialog;
	
	gboolean        show_tooltip;

	GSettings      *settings;
};

G_DEFINE_TYPE (NetspeedApplet, netspeed_applet, GP_TYPE_APPLET)

static void
update_tooltip(NetspeedApplet* applet);

static void
netspeed_applet_setup (GpApplet *applet);

/* Adds a Pango markup "foreground" to a bytestring
 */
static void
add_markup_fgcolor(char **string, const char *color)
{
	char *tmp = *string;
	*string = g_strdup_printf("<span foreground=\"%s\">%s</span>", color, tmp);
	g_free(tmp);
}

/* Here some rearangement of the icons and the labels occurs
 * according to the panelsize and wether we show in and out
 * or just the sum
 */
static void
applet_change_size_or_orient (NetspeedApplet *applet,
                              GtkOrientation  orientation)
{
	int size;
	gboolean labels_dont_shrink;

	g_assert(applet);

	size = applet->size;

	g_object_ref(applet->pix_box);
	g_object_ref(applet->in_pix);
	g_object_ref(applet->in_label);
	g_object_ref(applet->out_pix);
	g_object_ref(applet->out_label);
	g_object_ref(applet->sum_label);

	if (applet->in_box) {
		gtk_container_remove(GTK_CONTAINER(applet->in_box), applet->in_label);
		gtk_container_remove(GTK_CONTAINER(applet->in_box), applet->in_pix);
		gtk_widget_destroy(applet->in_box);
	}
	if (applet->out_box) {
		gtk_container_remove(GTK_CONTAINER(applet->out_box), applet->out_label);
		gtk_container_remove(GTK_CONTAINER(applet->out_box), applet->out_pix);
		gtk_widget_destroy(applet->out_box);
	}
	if (applet->sum_box) {
		gtk_container_remove(GTK_CONTAINER(applet->sum_box), applet->sum_label);
		gtk_widget_destroy(applet->sum_box);
	}
	if (applet->box) {
		gtk_container_remove(GTK_CONTAINER(applet->box), applet->pix_box);
		gtk_widget_destroy(applet->box);
	}

	if (orientation == GTK_ORIENTATION_VERTICAL) {
		applet->box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
		if (size > 64) {
			applet->sum_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
			applet->in_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 1);
			applet->out_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 1);
		} else {	
			applet->sum_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
			applet->in_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
			applet->out_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
		}
		labels_dont_shrink = FALSE;
	} else {
		applet->in_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 1);
		applet->out_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 1);
		if (size < 48) {
			applet->sum_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
			applet->box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 1);
			labels_dont_shrink = TRUE;
		} else {
			applet->sum_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
			applet->box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
			labels_dont_shrink = !applet->show_sum;
		}
	}

	netspeed_label_set_dont_shrink (NETSPEED_LABEL (applet->in_label), labels_dont_shrink);
	netspeed_label_set_dont_shrink (NETSPEED_LABEL (applet->out_label), labels_dont_shrink);
	netspeed_label_set_dont_shrink (NETSPEED_LABEL (applet->sum_label), labels_dont_shrink);

	gtk_box_pack_start(GTK_BOX(applet->in_box), applet->in_pix, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(applet->in_box), applet->in_label, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(applet->out_box), applet->out_pix, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(applet->out_box), applet->out_label, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(applet->sum_box), applet->sum_label, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(applet->box), applet->pix_box, FALSE, FALSE, 0);
	
	g_object_unref(applet->pix_box);
	g_object_unref(applet->in_pix);
	g_object_unref(applet->in_label);
	g_object_unref(applet->out_pix);
	g_object_unref(applet->out_label);
	g_object_unref(applet->sum_label);

	if (applet->show_sum) {
		gtk_box_pack_start(GTK_BOX(applet->box), applet->sum_box, TRUE, TRUE, 0);
	} else {
		gtk_box_pack_start(GTK_BOX(applet->box), applet->in_box, TRUE, TRUE, 0);
		gtk_box_pack_start(GTK_BOX(applet->box), applet->out_box, TRUE, TRUE, 0);
	}		
	
	gtk_widget_show_all(applet->box);
	gtk_container_add (GTK_CONTAINER (applet), applet->box);
}

/* Change the icons according to the selected device
 */
static void
change_icons(NetspeedApplet *applet)
{
	GdkPixbuf *dev, *down;
	GdkPixbuf *in_arrow, *out_arrow;
	GtkIconTheme *icon_theme;
	
	icon_theme = gtk_icon_theme_get_default();
	/* If the user wants a different icon then the eth0, we load it */
	if (applet->change_icon) {
		dev = gtk_icon_theme_load_icon(icon_theme, 
                        dev_type_icon[applet->devinfo.type], 16, 0, NULL);
	} else {
        	dev = gtk_icon_theme_load_icon(icon_theme, 
					       dev_type_icon[DEV_UNKNOWN], 
					       16, 0, NULL);
	}
    
    	/* We need a fallback */
    	if (dev == NULL) 
		dev = gtk_icon_theme_load_icon(icon_theme, 
					       dev_type_icon[DEV_UNKNOWN],
					       16, 0, NULL);
        
    
	in_arrow = gtk_icon_theme_load_icon(icon_theme, IN_ICON, 16, 0, NULL);
	out_arrow = gtk_icon_theme_load_icon(icon_theme, OUT_ICON, 16, 0, NULL);

	gtk_image_set_from_pixbuf(GTK_IMAGE(applet->out_pix), out_arrow);
	gtk_image_set_from_pixbuf(GTK_IMAGE(applet->in_pix), in_arrow);
	g_object_unref(in_arrow);
	g_object_unref(out_arrow);
	
	if (applet->devinfo.running) {
		gtk_widget_show(applet->in_box);
		gtk_widget_show(applet->out_box);
	} else {
		GdkPixbuf *copy;
		gtk_widget_hide(applet->in_box);
		gtk_widget_hide(applet->out_box);

		/* We're not allowed to modify "dev" */
        	copy = gdk_pixbuf_copy(dev);
        
        	down = gtk_icon_theme_load_icon(icon_theme, ERROR_ICON, 16, 0, NULL);	
		gdk_pixbuf_composite(down, copy, 8, 8, 8, 8, 8, 8, 0.5, 0.5, GDK_INTERP_BILINEAR, 0xFF);
		g_object_unref(down);
	      	g_object_unref(dev);
		dev = copy;
	}		

	gtk_image_set_from_pixbuf(GTK_IMAGE(applet->dev_pix), dev);
	g_object_unref(dev);
}

static void
update_quality_icon(NetspeedApplet *applet)
{
	unsigned int q;
	
	q = (applet->devinfo.qual);
	q /= 25;
	q = MIN(q, 3); /* q out of range would crash when accessing qual_pixbufs[q] */
	gtk_image_set_from_pixbuf (GTK_IMAGE(applet->qual_pix), applet->qual_pixbufs[q]);
}

static void
init_quality_pixbufs(NetspeedApplet *applet)
{
	GtkIconTheme *icon_theme;
	int i;
	
	icon_theme = gtk_icon_theme_get_default();

	for (i = 0; i < 4; i++) {
		if (applet->qual_pixbufs[i])
			g_object_unref(applet->qual_pixbufs[i]);
		applet->qual_pixbufs[i] = gtk_icon_theme_load_icon(icon_theme, 
			wireless_quality_icon[i], 24, 0, NULL);
	}
}


static void
icon_theme_changed_cb(GtkIconTheme *icon_theme, gpointer user_data)
{
    NetspeedApplet *applet = (NetspeedApplet*)user_data;
    init_quality_pixbufs(user_data);
    if (applet->devinfo.type == DEV_WIRELESS && applet->devinfo.up)
        update_quality_icon(user_data);
    change_icons(user_data);
}    

/* Converts a number of bytes into a human
 * readable string - in [M/k]bytes[/s]
 * The string has to be freed
 */
static char* 
bytes_to_string(double bytes, gboolean per_sec, gboolean bits)
{
	const char *unit;
	guint kilo; /* no really a kilo : a kilo or kibi */

	if (bits) {
		bytes *= 8;
		kilo = 1000;
	} else
		kilo = 1024;

	if (bytes < kilo) {
		if (per_sec)
			unit = bits ? N_("b/s")   : N_("B/s");
		else
			unit = bits ? N_("bits")  : N_("bytes");

		return g_strdup_printf ("%.0f %s", bytes, gettext (unit));
	} else if (bytes < (kilo * kilo)) {
		bytes /= kilo;

		if (per_sec)
			unit = bits ? N_("kb/s") : N_("KiB/s");
		else
			unit = bits ? N_("kb")   : N_("KiB");

		return g_strdup_printf (bytes < (100 * kilo) ? "%.1f %s" : "%.0f %s",
		                        bytes, gettext (unit));
	} else {
		bytes /= kilo * kilo;

		if (per_sec)
			unit = bits ? N_("Mb/s") : N_("MiB/s");
		else
			unit = bits ? N_("Mb")   : N_("MiB");
	}

	return g_strdup_printf ("%.1f %s", bytes, gettext (unit));
}

static gboolean
set_applet_devinfo(NetspeedApplet* applet, const char* iface)
{
	DevInfo info;

	get_device_info(iface, &info);

	if (info.running) {
		free_device_info(&applet->devinfo);
		applet->devinfo = info;
		applet->device_has_changed = TRUE;
		return TRUE;
	}

	free_device_info(&info);
	return FALSE;
}

/* Find the first available device, that is running and != lo */
static void
search_for_up_if(NetspeedApplet *applet)
{
	const gchar *default_route;
	GList *devices, *tmp;

	default_route = get_default_route();

	if (default_route != NULL) {
		if (set_applet_devinfo(applet, default_route))
			return;
	}

	devices = get_available_devices();
	for (tmp = devices; tmp; tmp = g_list_next(tmp)) {
		if (is_dummy_device(tmp->data))
			continue;
		if (set_applet_devinfo(applet, tmp->data))
			break;
	}

	g_list_free_full (devices, g_free);
}

/* Here happens the really interesting stuff */
static void
update_applet(NetspeedApplet *applet)
{
	guint64 indiff, outdiff;
	double inrate, outrate;
	char *inbytes, *outbytes;
	int i;
	DevInfo oldinfo;
	
	if (!applet)	return;
	
	/* First we try to figure out if the device has changed */
	oldinfo = applet->devinfo;
	get_device_info(oldinfo.name, &applet->devinfo);
	if (compare_device_info(&applet->devinfo, &oldinfo))
		applet->device_has_changed = TRUE;
	free_device_info(&oldinfo);

	/* If the device has changed, reintialize stuff */	
	if (applet->device_has_changed) {
		change_icons(applet);
		if (applet->devinfo.type == DEV_WIRELESS &&
			applet->devinfo.up) {
			gtk_widget_show(applet->qual_pix);
		} else {
			gtk_widget_hide(applet->qual_pix);
		}	
		for (i = 0; i < OLD_VALUES; i++)
		{
			applet->in_old[i] = applet->devinfo.rx;
			applet->out_old[i] = applet->devinfo.tx;
		}
		for (i = 0; i < GRAPH_VALUES; i++)
		{
			applet->in_graph[i] = -1;
			applet->out_graph[i] = -1;
		}
		applet->max_graph = 0;
		applet->index_graph = 0;
		applet->device_has_changed = FALSE;
	}
		
	/* create the strings for the labels and tooltips */
	if (applet->devinfo.running)
	{	
		if (applet->devinfo.rx < applet->in_old[applet->index_old]) indiff = 0;
		else indiff = applet->devinfo.rx - applet->in_old[applet->index_old];
		if (applet->devinfo.tx < applet->out_old[applet->index_old]) outdiff = 0;
		else outdiff = applet->devinfo.tx - applet->out_old[applet->index_old];
		
		inrate = indiff * 1000.0;
		inrate /= (double)(applet->refresh_time * OLD_VALUES);
		outrate = outdiff * 1000.0;
		outrate /= (double)(applet->refresh_time * OLD_VALUES);
		
		applet->in_graph[applet->index_graph] = inrate;
		applet->out_graph[applet->index_graph] = outrate;
		applet->max_graph = MAX(inrate, applet->max_graph);
		applet->max_graph = MAX(outrate, applet->max_graph);
		
		applet->devinfo.rx_rate = bytes_to_string(inrate, TRUE, applet->show_bits);
		applet->devinfo.tx_rate = bytes_to_string(outrate, TRUE, applet->show_bits);
		applet->devinfo.sum_rate = bytes_to_string(inrate + outrate, TRUE, applet->show_bits);
	} else {
		applet->devinfo.rx_rate = g_strdup("");
		applet->devinfo.tx_rate = g_strdup("");
		applet->devinfo.sum_rate = g_strdup("");
		applet->in_graph[applet->index_graph] = 0;
		applet->out_graph[applet->index_graph] = 0;
	}
	
	if (applet->devinfo.type == DEV_WIRELESS) {
		if (applet->devinfo.up)
			update_quality_icon(applet);
		
		if (applet->signalbar) {
			float quality;
			char *text;

			quality = applet->devinfo.qual / 100.0f;
			if (quality > 1.0f)
				quality = 1.0;

			text = g_strdup_printf ("%d %%", applet->devinfo.qual);
			gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (applet->signalbar), quality);
			gtk_progress_bar_set_text (GTK_PROGRESS_BAR (applet->signalbar), text);
			g_free(text);
		}
	}

	update_tooltip(applet);

	/* Refresh the text of the labels and tooltip */
	if (applet->show_sum) {
		gtk_label_set_markup(GTK_LABEL(applet->sum_label), applet->devinfo.sum_rate);
	} else {
		gtk_label_set_markup(GTK_LABEL(applet->in_label), applet->devinfo.rx_rate);
		gtk_label_set_markup(GTK_LABEL(applet->out_label), applet->devinfo.tx_rate);
	}

	/* Refresh the values of the Infodialog */
	if (applet->inbytes_text) {
		inbytes = bytes_to_string((double)applet->devinfo.rx, FALSE, applet->show_bits);
		gtk_label_set_text(GTK_LABEL(applet->inbytes_text), inbytes);
		g_free(inbytes);
	}	
	if (applet->outbytes_text) {
		outbytes = bytes_to_string((double)applet->devinfo.tx, FALSE, applet->show_bits);
		gtk_label_set_text(GTK_LABEL(applet->outbytes_text), outbytes);
		g_free(outbytes);
	}
	/* Redraw the graph of the Infodialog */
	if (applet->drawingarea)
                gtk_widget_queue_draw (GTK_WIDGET (applet->drawingarea));

	/* Save old values... */
	applet->in_old[applet->index_old] = applet->devinfo.rx;
	applet->out_old[applet->index_old] = applet->devinfo.tx;
	applet->index_old = (applet->index_old + 1) % OLD_VALUES;

	/* Move the graphindex. Check if we can scale down again */
	applet->index_graph = (applet->index_graph + 1) % GRAPH_VALUES; 
	if (applet->index_graph % 20 == 0)
	{
		double max = 0;
		for (i = 0; i < GRAPH_VALUES; i++)
		{
			max = MAX(max, applet->in_graph[i]);
			max = MAX(max, applet->out_graph[i]);
		}
		applet->max_graph = max;
	}

	/* Always follow the default route */
	if (applet->auto_change_device) {
		gboolean change_device_now = !applet->devinfo.running;
		if (!change_device_now) {
			const gchar *default_route;
			default_route = get_default_route();
			change_device_now = (default_route != NULL
						&& strcmp(default_route,
							applet->devinfo.name));
		}
		if (change_device_now) {
			search_for_up_if(applet);
		}
	}
}

static gboolean
timeout_function(NetspeedApplet *applet)
{
	if (!applet)
		return FALSE;
	if (!applet->timeout_id)
		return FALSE;

	update_applet(applet);
	return TRUE;
}

static void
help_cb (GSimpleAction *action,
         GVariant      *parameter,
         gpointer       user_data)
{
  gp_applet_show_help (GP_APPLET (user_data), NULL);
}

static void
about_cb (GSimpleAction *action,
          GVariant      *parameter,
          gpointer       user_data)
{
  gp_applet_show_about (GP_APPLET (user_data));
}

static void
netspeed_applet_destory_preferences (GtkWidget *widget,
                                     gpointer   user_data)
{
	NetspeedApplet *netspeed;

	netspeed = NETSPEED_APPLET (user_data);

	netspeed->preferences = NULL;
}

static void
preferences_cb (GSimpleAction *action,
                GVariant      *parameter,
                gpointer       user_data)
{
	NetspeedApplet *netspeed;

	netspeed = NETSPEED_APPLET (user_data);

	if (netspeed->preferences) {
		gtk_window_present (GTK_WINDOW (netspeed->preferences));
		return;
	}

	netspeed->preferences = netspeed_preferences_new (netspeed);
	g_signal_connect (netspeed->preferences, "destroy",
	                  G_CALLBACK (netspeed_applet_destory_preferences), netspeed);
}

/* Redraws the graph drawingarea
 * Some really black magic is going on in here ;-)
 */
static gboolean
da_draw(GtkWidget *widget, cairo_t *cr, gpointer data)
{
	NetspeedApplet *applet = (NetspeedApplet*)data;
        GtkStyleContext *context;
        GtkStateFlags state;
	GdkWindow *window;
	GdkPoint in_points[GRAPH_VALUES], out_points[GRAPH_VALUES];
	PangoLayout *layout;
	PangoRectangle logical_rect;
        GdkColor color;
	char *text;
        gint width, height;
	int i, offset;
	double max_val;
        cairo_surface_t *surface;
        cairo_t *tmp_cr;

        state = gtk_widget_get_state_flags (widget);
        context = gtk_widget_get_style_context (widget);
        gtk_style_context_save (context);

        gtk_style_context_set_state (context, state);

        window = gtk_widget_get_window (widget);
        width = gtk_widget_get_allocated_width (widget);
        height = gtk_widget_get_allocated_height (widget);

	/* use doublebuffering to avoid flickering */
        surface = gdk_window_create_similar_surface (window, CAIRO_CONTENT_COLOR, width, height);

        tmp_cr = cairo_create (surface);

	/* the graph hight should be: hight/2 <= applet->max_graph < hight */
	for (max_val = 1; max_val < applet->max_graph; max_val *= 2) ;

	/* calculate the polygons (GdkPoint[]) for the graphs */
	offset = 0;
	for (i = (applet->index_graph + 1) % GRAPH_VALUES; applet->in_graph[i] < 0; i = (i + 1) % GRAPH_VALUES)
		offset++;
	for (i = offset + 1; i < GRAPH_VALUES; i++)
	{
		int index = (applet->index_graph + i) % GRAPH_VALUES;
		out_points[i].x = in_points[i].x = ((width - 6) * i) / GRAPH_VALUES + 4;
		in_points[i].y = height - 6 - (int)((height - 8) * applet->in_graph[index] / max_val);
		out_points[i].y = height - 6 - (int)((height - 8) * applet->out_graph[index] / max_val);
	}
	in_points[offset].x = out_points[offset].x = ((width - 6) * offset) / GRAPH_VALUES + 4;
	in_points[offset].y = in_points[(offset + 1) % GRAPH_VALUES].y;
	out_points[offset].y = out_points[(offset + 1) % GRAPH_VALUES].y;

	/* draw the background */
        cairo_set_source_rgb (tmp_cr, 0., 0., 0.);
        cairo_rectangle (tmp_cr, 0, 0, width, height);
        cairo_fill (tmp_cr);

        cairo_set_line_width (tmp_cr, 1.);
	color.red = 0x3a00; color.green = 0x8000; color.blue = 0x1400;
        gdk_cairo_set_source_color (tmp_cr, &color);
        cairo_rectangle (tmp_cr, 2.5, 2.5, width - 6.5, height - 6.5);
        cairo_stroke (tmp_cr);

	for (i = 0; i < GRAPH_LINES; i++) {
		int y = 2 + ((height - 6) * i) / GRAPH_LINES;
                cairo_move_to (tmp_cr, 2.5, y + 0.5);
                cairo_line_to (tmp_cr, width - 4.5, y - 0.5);
                cairo_stroke (tmp_cr);
	}

	/* draw the polygons */
        cairo_set_line_width (tmp_cr, 2.);
        cairo_set_line_join (tmp_cr, CAIRO_LINE_JOIN_ROUND);
        cairo_set_line_cap (tmp_cr, CAIRO_LINE_CAP_ROUND);

        gdk_cairo_set_source_rgba (tmp_cr, &applet->in_color);
        cairo_move_to (tmp_cr, in_points[offset].x, in_points[offset].y);
        for (i = offset + 1; i < GRAPH_VALUES; i++)
                cairo_line_to (tmp_cr, in_points[i].x, in_points[i].y);
        cairo_stroke (tmp_cr);

        gdk_cairo_set_source_rgba (tmp_cr, &applet->out_color);
        cairo_move_to (tmp_cr, out_points[offset].x, out_points[offset].y);
        for (i = offset + 1; i < GRAPH_VALUES; i++)
                cairo_line_to (tmp_cr, out_points[i].x, out_points[i].y);
        cairo_stroke (tmp_cr);

	/* draw the 2 labels */
        gtk_style_context_set_state (context, GTK_STATE_FLAG_NORMAL);

	text = bytes_to_string(max_val, TRUE, applet->show_bits);
	add_markup_fgcolor(&text, "white");
	layout = gtk_widget_create_pango_layout (widget, NULL);
	pango_layout_set_markup(layout, text, -1);
	g_free (text);
        gtk_render_layout (context, tmp_cr, 3, 2, layout);
	g_object_unref(G_OBJECT(layout));

	text = bytes_to_string(0.0, TRUE, applet->show_bits);
	add_markup_fgcolor(&text, "white");
	layout = gtk_widget_create_pango_layout (widget, NULL);
	pango_layout_set_markup(layout, text, -1);
	pango_layout_get_pixel_extents (layout, NULL, &logical_rect);
	g_free (text);
        gtk_render_layout (context, tmp_cr, 3, height - 4 - logical_rect.height, layout);
	g_object_unref(G_OBJECT(layout));

        cairo_destroy (tmp_cr);

	/* draw the surface to the real window */
        cairo_set_source_surface (cr, surface, 0, 0);
        cairo_paint (cr);
        cairo_surface_destroy (surface);

	return FALSE;
}

static void
incolor_changed_cb (GtkColorButton *button,
                    gpointer        user_data)
{
	NetspeedApplet *netspeed;
	GdkRGBA color;
	gchar *string;

	netspeed = NETSPEED_APPLET (user_data);

	gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER (button), &color);

	string = gdk_rgba_to_string (&color);
	g_settings_set_string (netspeed->settings, "in-color", string);
	g_free (string);
}

static void
outcolor_changed_cb (GtkColorButton *button,
                     gpointer        user_data)
{
	NetspeedApplet *netspeed;
	GdkRGBA color;
	gchar *string;

	netspeed = NETSPEED_APPLET (user_data);

	gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER (button), &color);

	string = gdk_rgba_to_string (&color);
	g_settings_set_string (netspeed->settings, "out-color", string);
	g_free (string);
}

/* Handle info dialog response event
 */
static void
info_response_cb (GtkDialog *dialog, gint id, NetspeedApplet *applet)
{
	if (id == GTK_RESPONSE_HELP) {
		gp_applet_show_help (GP_APPLET (applet), "netspeed_applet-details");
		return;
	}

	g_clear_pointer (&applet->details, gtk_widget_destroy);

	applet->inbytes_text = NULL;
	applet->outbytes_text = NULL;
	applet->drawingarea = NULL;
	applet->signalbar = NULL;
}

/* Creates the details dialog
 */
static void
details_cb (GSimpleAction *action,
            GVariant      *parameter,
            gpointer       user_data)
{
	NetspeedApplet *applet = (NetspeedApplet*)user_data;
	GtkWidget *box, *hbox;
	GtkWidget *grid, *da_frame;
	GtkWidget *ip_label, *netmask_label;
	GtkWidget *hwaddr_label, *ptpip_label;
	GtkWidget *ip_text, *netmask_text;
	GtkWidget *hwaddr_text, *ptpip_text;
	GtkWidget *inbytes_label, *outbytes_label;
	GtkWidget *incolor_sel, *incolor_label;
	GtkWidget *outcolor_sel, *outcolor_label;
        GtkWidget *dialog_vbox;
	char *title;
	
	g_assert(applet);
	
	if (applet->details)
	{
		gtk_window_present(GTK_WINDOW(applet->details));
		return;
	}
	
	title = g_strdup_printf(_("Device Details for %s"), applet->devinfo.name);
	applet->details = gtk_dialog_new_with_buttons (title,
	                                               NULL,
	                                               GTK_DIALOG_DESTROY_WITH_PARENT,
	                                               GTK_STOCK_CLOSE, GTK_RESPONSE_ACCEPT,
	                                               GTK_STOCK_HELP, GTK_RESPONSE_HELP,
	                                               NULL);
	g_free(title);

	gtk_dialog_set_default_response(GTK_DIALOG(applet->details), GTK_RESPONSE_CLOSE);
	
	box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 10);
	gtk_container_set_border_width(GTK_CONTAINER(box), 12);

	grid = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (grid), 10);
	gtk_grid_set_column_spacing (GTK_GRID (grid), 15);

	da_frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(da_frame), GTK_SHADOW_IN);
	applet->drawingarea = GTK_DRAWING_AREA(gtk_drawing_area_new());
	gtk_widget_set_size_request(GTK_WIDGET(applet->drawingarea), -1, 180);
	gtk_container_add(GTK_CONTAINER(da_frame), GTK_WIDGET(applet->drawingarea));
	
	hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);
	incolor_label = gtk_label_new_with_mnemonic(_("_In graph color"));
	outcolor_label = gtk_label_new_with_mnemonic(_("_Out graph color"));
	
	incolor_sel = gtk_color_button_new ();
	outcolor_sel = gtk_color_button_new ();
	
	gtk_color_chooser_set_rgba (GTK_COLOR_CHOOSER (incolor_sel),  &applet->in_color);
	gtk_color_chooser_set_rgba (GTK_COLOR_CHOOSER (outcolor_sel),  &applet->out_color);

	gtk_label_set_mnemonic_widget(GTK_LABEL(incolor_label), incolor_sel);
	gtk_label_set_mnemonic_widget(GTK_LABEL(outcolor_label), outcolor_sel);
	
	gtk_box_pack_start(GTK_BOX(hbox), incolor_sel, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), incolor_label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), outcolor_sel, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), outcolor_label, FALSE, FALSE, 0);
	
	ip_label = gtk_label_new(_("Internet Address:"));
	netmask_label = gtk_label_new(_("Netmask:"));
	hwaddr_label = gtk_label_new(_("Hardware Address:"));
	ptpip_label = gtk_label_new(_("P-t-P Address:"));
	inbytes_label = gtk_label_new(_("Bytes in:"));
	outbytes_label = gtk_label_new(_("Bytes out:"));
	
	ip_text = gtk_label_new(applet->devinfo.ip ? applet->devinfo.ip : _("none"));
	netmask_text = gtk_label_new(applet->devinfo.netmask ? applet->devinfo.netmask : _("none"));
	hwaddr_text = gtk_label_new(applet->devinfo.hwaddr ? applet->devinfo.hwaddr : _("none"));
	ptpip_text = gtk_label_new(applet->devinfo.ptpip ? applet->devinfo.ptpip : _("none"));
	applet->inbytes_text = gtk_label_new("0 byte");
	applet->outbytes_text = gtk_label_new("0 byte");

	gtk_label_set_selectable(GTK_LABEL(ip_text), TRUE);
	gtk_label_set_selectable(GTK_LABEL(netmask_text), TRUE);
	gtk_label_set_selectable(GTK_LABEL(hwaddr_text), TRUE);
	gtk_label_set_selectable(GTK_LABEL(ptpip_text), TRUE);
	
	gtk_misc_set_alignment(GTK_MISC(ip_label), 0.0f, 0.5f);
	gtk_misc_set_alignment(GTK_MISC(ip_text), 0.0f, 0.5f);
	gtk_misc_set_alignment(GTK_MISC(netmask_label), 0.0f, 0.5f);
	gtk_misc_set_alignment(GTK_MISC(netmask_text), 0.0f, 0.5f);
	gtk_misc_set_alignment(GTK_MISC(hwaddr_label), 0.0f, 0.5f);
	gtk_misc_set_alignment(GTK_MISC(hwaddr_text), 0.0f, 0.5f);
	gtk_misc_set_alignment(GTK_MISC(ptpip_label), 0.0f, 0.5f);
	gtk_misc_set_alignment(GTK_MISC(ptpip_text), 0.0f, 0.5f);
	gtk_misc_set_alignment(GTK_MISC(inbytes_label), 0.0f, 0.5f);
	gtk_misc_set_alignment(GTK_MISC(applet->inbytes_text), 0.0f, 0.5f);
	gtk_misc_set_alignment(GTK_MISC(outbytes_label), 0.0f, 0.5f);
	gtk_misc_set_alignment(GTK_MISC(applet->outbytes_text), 0.0f, 0.5f);

	gtk_widget_set_hexpand (ip_label, TRUE);
	gtk_widget_set_hexpand (ip_text, TRUE);
	gtk_widget_set_hexpand (netmask_label, TRUE);
	gtk_widget_set_hexpand (netmask_text, TRUE);
	gtk_widget_set_hexpand (hwaddr_label, TRUE);
	gtk_widget_set_hexpand (hwaddr_text, TRUE);
	gtk_widget_set_hexpand (ptpip_label, TRUE);
	gtk_widget_set_hexpand (inbytes_label, TRUE);
	gtk_widget_set_hexpand (ptpip_text, TRUE);
	gtk_widget_set_hexpand (applet->inbytes_text, TRUE);
	gtk_widget_set_hexpand (outbytes_label, TRUE);
	gtk_widget_set_hexpand (applet->outbytes_text, TRUE);

	gtk_grid_attach (GTK_GRID (grid), ip_label, 0, 0, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), ip_text, 1, 0, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), netmask_label, 2, 0, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), netmask_text, 3, 0, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), hwaddr_label, 0, 1, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), hwaddr_text, 1, 1, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), ptpip_label, 2, 1, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), ptpip_text, 3, 1, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), inbytes_label, 0, 2, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), applet->inbytes_text, 1, 2, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), outbytes_label, 2, 2, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), applet->outbytes_text, 3, 2, 1, 1);

	/* check if we got an ipv6 address */
	if (applet->devinfo.ipv6 && (strlen (applet->devinfo.ipv6) > 2)) {
		GtkWidget *ipv6_label, *ipv6_text;

		ipv6_label = gtk_label_new (_("IPv6 Address:"));
		ipv6_text = gtk_label_new (applet->devinfo.ipv6);
		
		gtk_label_set_selectable (GTK_LABEL (ipv6_text), TRUE);
		
		gtk_misc_set_alignment (GTK_MISC (ipv6_label), 0.0f, 0.5f);
		gtk_misc_set_alignment (GTK_MISC (ipv6_text), 0.0f, 0.5f);

		gtk_widget_set_hexpand (ipv6_label, TRUE);
		gtk_widget_set_hexpand (ipv6_text, TRUE);

		gtk_grid_attach (GTK_GRID (grid), ipv6_label, 0, 3, 1, 1);
		gtk_grid_attach (GTK_GRID (grid), ipv6_text, 1, 3, 1, 1);
	}

	if (applet->devinfo.type == DEV_WIRELESS) {
		GtkWidget *signal_label;
		GtkWidget *essid_label;
		GtkWidget *essid_text;
		float quality;
		char *text;

		/* _maybe_ we can add the encrypted icon between the essid and the signal bar. */

		applet->signalbar = gtk_progress_bar_new ();

		quality = applet->devinfo.qual / 100.0f;
		if (quality > 1.0f)
			quality = 1.0;

		text = g_strdup_printf ("%d %%", applet->devinfo.qual);
		gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (applet->signalbar), quality);
		gtk_progress_bar_set_text (GTK_PROGRESS_BAR (applet->signalbar), text);
		g_free(text);

		signal_label = gtk_label_new (_("Signal Strength:"));
		essid_label = gtk_label_new (_("ESSID:"));
		essid_text = gtk_label_new (applet->devinfo.essid);

		gtk_misc_set_alignment (GTK_MISC (signal_label), 0.0f, 0.5f);
		gtk_misc_set_alignment (GTK_MISC (essid_label), 0.0f, 0.5f);
		gtk_misc_set_alignment (GTK_MISC (essid_text), 0.0f, 0.5f);

		gtk_label_set_selectable (GTK_LABEL (essid_text), TRUE);

		gtk_widget_set_hexpand (signal_label, TRUE);
		gtk_widget_set_hexpand (applet->signalbar, TRUE);
		gtk_widget_set_hexpand (essid_label, TRUE);
		gtk_widget_set_hexpand (essid_text, TRUE);

		gtk_grid_attach (GTK_GRID (grid), signal_label, 2, 4, 1, 1);
		gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (applet->signalbar), 3, 4, 1, 1);
		gtk_grid_attach (GTK_GRID (grid), essid_label, 0, 4, 3, 1);
		gtk_grid_attach (GTK_GRID (grid), essid_text, 1, 4, 3, 1);
	}

	g_signal_connect(applet->drawingarea, "draw",
			 G_CALLBACK(da_draw),
			 (gpointer)applet);

	g_signal_connect (incolor_sel, "color-set",
	                  G_CALLBACK (incolor_changed_cb),
	                  applet);
	
	g_signal_connect (outcolor_sel, "color-set",
	                  G_CALLBACK (outcolor_changed_cb),
	                  applet);

	g_signal_connect(applet->details, "response",
			 G_CALLBACK(info_response_cb), (gpointer)applet);

	gtk_box_pack_start(GTK_BOX(box), da_frame, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (box), grid, FALSE, FALSE, 0);

        dialog_vbox = gtk_dialog_get_content_area (GTK_DIALOG (applet->details));
	gtk_container_add(GTK_CONTAINER(dialog_vbox), box);
	gtk_widget_show_all (applet->details);
}	

static void
update_tooltip(NetspeedApplet* applet)
{
  GString* tooltip;

  if (!applet->show_tooltip)
    return;

  tooltip = g_string_new("");

  if (!applet->devinfo.running)
    g_string_printf(tooltip, _("%s is down"), applet->devinfo.name);
  else {
    if (applet->show_sum) {
      g_string_printf(
		      tooltip,
		      _("%s: %s\nin: %s out: %s"),
		      applet->devinfo.name,
		      applet->devinfo.ip ? applet->devinfo.ip : _("has no ip"),
		      applet->devinfo.rx_rate,
		      applet->devinfo.tx_rate
		      );
    } else {
      g_string_printf(
		      tooltip,
		      _("%s: %s\nsum: %s"),
		      applet->devinfo.name,
		      applet->devinfo.ip ? applet->devinfo.ip : _("has no ip"),
		      applet->devinfo.sum_rate
		      );
    }
    if (applet->devinfo.type == DEV_WIRELESS)
      g_string_append_printf(
			     tooltip,
			     _("\nESSID: %s\nStrength: %d %%"),
			     applet->devinfo.essid ? applet->devinfo.essid : _("unknown"),
			     applet->devinfo.qual
			     );

  }

  gtk_widget_set_tooltip_text (GTK_WIDGET (applet), tooltip->str);
  gtk_widget_trigger_tooltip_query (GTK_WIDGET (applet));
  g_string_free(tooltip, TRUE);
}

static const GActionEntry menu_actions [] = {
	{ "details",     details_cb,     NULL, NULL, NULL },
	{ "preferences", preferences_cb, NULL, NULL, NULL },
	{ "help",        help_cb,        NULL, NULL, NULL },
	{ "about",       about_cb,       NULL, NULL, NULL },
	{ NULL }
};

static void
netspeed_applet_constructed (GObject *object)
{
  G_OBJECT_CLASS (netspeed_applet_parent_class)->constructed (object);
  netspeed_applet_setup (GP_APPLET (object));
}

static void
netspeed_applet_finalize (GObject *object)
{
	NetspeedApplet *netspeed;

	netspeed = NETSPEED_APPLET (object);

	g_object_disconnect (gtk_icon_theme_get_default (), "any_signal::changed",
	                     G_CALLBACK (icon_theme_changed_cb), netspeed,
	                     NULL);

	if (netspeed->timeout_id > 0) {
		g_source_remove (netspeed->timeout_id);
		netspeed->timeout_id = 0;
	}

	g_clear_object (&netspeed->settings);

	g_clear_pointer (&netspeed->details, gtk_widget_destroy);
	g_clear_pointer (&netspeed->preferences, gtk_widget_destroy);

	g_free (netspeed->up_cmd);
	g_free (netspeed->down_cmd);

	free_device_info (&netspeed->devinfo);

	G_OBJECT_CLASS (netspeed_applet_parent_class)->finalize (object);
}

static gboolean
netspeed_applet_button_press_event (GtkWidget      *widget,
                                    GdkEventButton *event)
{
	NetspeedApplet *netspeed;

	netspeed = NETSPEED_APPLET (widget);

	if (event->button == 1) {
		GError *error = NULL;

		if (netspeed->connect_dialog) {
			gtk_window_present (GTK_WINDOW (netspeed->connect_dialog));
			return FALSE;
		}

		if (netspeed->up_cmd && netspeed->down_cmd) {
			char *question;
			gint response;

			if (netspeed->devinfo.up)
				question = g_strdup_printf (_("Do you want to disconnect %s now?"),
				                            netspeed->devinfo.name);
			else
				question = g_strdup_printf (_("Do you want to connect %s now?"),
				                            netspeed->devinfo.name);

			netspeed->connect_dialog = gtk_message_dialog_new (NULL,
			                                                   GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			                                                   GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
			                                                   "%s", question);

			g_free (question);

			response = gtk_dialog_run (GTK_DIALOG (netspeed->connect_dialog));
			gtk_widget_destroy (netspeed->connect_dialog);
			netspeed->connect_dialog = NULL;

			if (response == GTK_RESPONSE_YES) {
				GtkWidget *dialog;
				char *command;

				command = g_strdup_printf ("%s %s",
				                           netspeed->devinfo.up ? netspeed->down_cmd : netspeed->up_cmd,
				                           netspeed->devinfo.name);

				if (!g_spawn_command_line_async (command, &error)) {
					dialog = gtk_message_dialog_new_with_markup (NULL,
					                                             GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
					                                             GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
					                                             _("<b>Running command %s failed</b>\n%s"),
					                                             command,
					                                             error->message);
					gtk_dialog_run (GTK_DIALOG (dialog));
					gtk_widget_destroy (dialog);
					g_error_free (error);
				}

				g_free(command);
			}
		}
	}

	return GTK_WIDGET_CLASS (netspeed_applet_parent_class)->button_press_event (widget, event);
}

static gboolean
netspeed_applet_leave_notify_event (GtkWidget        *widget,
                                    GdkEventCrossing *event)
{
	NetspeedApplet *netspeed;

	netspeed = NETSPEED_APPLET (widget);

	netspeed->show_tooltip = TRUE;
	update_tooltip (netspeed);

	return TRUE;
}

static gboolean
netspeed_applet_enter_notify_event (GtkWidget        *widget,
                                    GdkEventCrossing *event)
{
	NetspeedApplet *netspeed;

	netspeed = NETSPEED_APPLET (widget);

	netspeed->show_tooltip = FALSE;
	update_tooltip (netspeed);

	return TRUE;
}

static void
netspeed_applet_class_init (NetspeedAppletClass *netspeed_class)
{
	GObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = G_OBJECT_CLASS (netspeed_class);
	widget_class = GTK_WIDGET_CLASS (netspeed_class);

	object_class->constructed = netspeed_applet_constructed;
	object_class->finalize = netspeed_applet_finalize;

	widget_class->button_press_event = netspeed_applet_button_press_event;
	widget_class->leave_notify_event = netspeed_applet_leave_notify_event;
	widget_class->enter_notify_event = netspeed_applet_enter_notify_event;
}

static void
netspeed_applet_init (NetspeedApplet *netspeed)
{
	netspeed->size = 24;

	gtk_icon_theme_append_search_path (gtk_icon_theme_get_default (),
	                                   PKG_DATA_DIR G_DIR_SEPARATOR_S "icons");
}

static void
setup_menu (GpApplet *applet)
{
	const char *menu_resource;
	GAction *action;

	menu_resource = GRESOURCE_PREFIX "/ui/netspeed-menu.ui";
	gp_applet_setup_menu_from_resource (applet, menu_resource, menu_actions);

	action = gp_applet_menu_lookup_action (applet, "preferences");
	g_object_bind_property (applet, "locked-down",
	                        action, "enabled",
	                        G_BINDING_DEFAULT|G_BINDING_INVERT_BOOLEAN|G_BINDING_SYNC_CREATE);
}

static void
netspeed_applet_setup_timeout (NetspeedApplet *netspeed)
{
	if (netspeed->timeout_id > 0)
		g_source_remove (netspeed->timeout_id);

	netspeed->timeout_id = g_timeout_add (netspeed->refresh_time,
	                                      (GSourceFunc) timeout_function,
	                                      netspeed);
}

static void
device_changed (NetspeedApplet *netspeed)
{
	gchar *device;

	device = g_settings_get_string (netspeed->settings, "device");

	if (g_strcmp0 (device, netspeed->devinfo.name) == 0) {
		g_free (device);
		return;
	}

	if (g_strcmp0 (device, "") == 0) {
		g_free (device);
		device = netspeed_applet_get_auto_device_name ();
	}

	get_device_info (device, &netspeed->devinfo);
	g_free (device);

	netspeed->device_has_changed = TRUE;
}

static void
up_command_changed (NetspeedApplet *netspeed)
{
	g_free (netspeed->up_cmd);
	netspeed->up_cmd = g_settings_get_string (netspeed->settings, "up-command");
}

static void
down_command_changed (NetspeedApplet *netspeed)
{
	g_free (netspeed->up_cmd);
	netspeed->up_cmd = g_settings_get_string (netspeed->settings, "down-command");
}

static void
in_color_changed (NetspeedApplet *netspeed)
{
	gchar *color;

	color = g_settings_get_string (netspeed->settings, "in-color");

	if (!gdk_rgba_parse (&netspeed->in_color, color))
		gdk_rgba_parse (&netspeed->in_color, "#de2847");

	g_free (color);
}

static void
out_color_changed (NetspeedApplet *netspeed)
{
	gchar *color;

	color = g_settings_get_string (netspeed->settings, "out-color");

	if (!gdk_rgba_parse (&netspeed->out_color, color))
		gdk_rgba_parse (&netspeed->out_color, "#3728de");

	g_free (color);
}

static void
netspeed_applet_settings_changed (GSettings   *settings,
                                  const gchar *key,
                                  gpointer     user_data)
{
	NetspeedApplet *netspeed;
	GtkOrientation orientation;

	netspeed = NETSPEED_APPLET (user_data);
	orientation = gp_applet_get_orientation (GP_APPLET (netspeed));

	if (key == NULL || g_strcmp0 (key, "refresh-time") == 0) {
		netspeed->refresh_time = g_settings_get_int (netspeed->settings, "refresh-time");
		netspeed_applet_setup_timeout (netspeed);
	}

	if (key == NULL || g_strcmp0 (key, "show-sum") == 0)
		netspeed->show_sum = g_settings_get_boolean (netspeed->settings, "show-sum");

	if (key == NULL || g_strcmp0 (key, "show-bits") == 0)
		netspeed->show_bits = g_settings_get_boolean (netspeed->settings, "show-bits");

	if (key == NULL || g_strcmp0 (key, "change-icon") == 0) {
		netspeed->change_icon = g_settings_get_boolean (netspeed->settings, "change-icon");

		if (key != NULL)
			change_icons (netspeed);
	}

	if (key == NULL || g_strcmp0 (key, "auto-change-device") == 0)
		netspeed->auto_change_device = g_settings_get_boolean (netspeed->settings, "auto-change-device");

	if (key == NULL || g_strcmp0 (key, "device") == 0) {
		device_changed (netspeed);

		if (key != NULL)
			change_icons (netspeed);
	}

	if (key == NULL || g_strcmp0 (key, "up-command") == 0)
		up_command_changed (netspeed);

	if (key == NULL || g_strcmp0 (key, "down-command") == 0)
		down_command_changed (netspeed);

	if (key == NULL || g_strcmp0 (key, "in-color") == 0)
		in_color_changed (netspeed);

	if (key == NULL || g_strcmp0 (key, "out-color") == 0)
		out_color_changed (netspeed);

	if (key != NULL) {
		applet_change_size_or_orient (netspeed, orientation);
		update_applet (netspeed);
	}
}

static void
netspeed_applet_size_allocate (GtkWidget     *widget,
                               GtkAllocation *allocation,
                               gpointer       user_data)
{
	NetspeedApplet *netspeed;
	GtkOrientation orientation;
	gint old_size;

	netspeed = NETSPEED_APPLET (user_data);
	orientation = gp_applet_get_orientation (GP_APPLET (netspeed));
	old_size = netspeed->size;

	if (orientation == GTK_ORIENTATION_HORIZONTAL)
		netspeed->size = allocation->height;
	else
		netspeed->size = allocation->width;

	if (old_size == netspeed->size)
		return;

	applet_change_size_or_orient (netspeed, orientation);
}

static void
placement_changed_cb (GpApplet        *applet,
                      GtkOrientation   orientation,
                      GtkPositionType  position,
                      NetspeedApplet  *self)
{
	applet_change_size_or_orient (self, orientation);
}

static void
netspeed_applet_setup (GpApplet *applet)
{
	NetspeedApplet *netspeed;
	int i;
	GtkWidget *spacer, *spacer_box;

	netspeed = NETSPEED_APPLET (applet);

	glibtop_init();

	/* Alloc the applet. The "NULL-setting" is really redudant
 	 * but aren't we paranoid?
	 */
	memset(&netspeed->devinfo, 0, sizeof(DevInfo));

	for (i = 0; i < GRAPH_VALUES; i++)
	{
		netspeed->in_graph[i] = -1;
		netspeed->out_graph[i] = -1;
	}	

	netspeed->settings = gp_applet_settings_new (applet, "org.gnome.gnome-applets.netspeed");

	g_signal_connect (netspeed->settings, "changed",
	                  G_CALLBACK (netspeed_applet_settings_changed),
	                  netspeed);
	netspeed_applet_settings_changed (netspeed->settings, NULL, netspeed);

	netspeed->in_label = netspeed_label_new ();
	netspeed->out_label = netspeed_label_new ();
	netspeed->sum_label = netspeed_label_new ();

	gp_add_text_color_class (netspeed->in_label);
	gp_add_text_color_class (netspeed->out_label);
	gp_add_text_color_class (netspeed->sum_label);

	netspeed->in_pix = gtk_image_new ();
	netspeed->out_pix = gtk_image_new ();
	netspeed->dev_pix = gtk_image_new ();
	netspeed->qual_pix = gtk_image_new ();
	
	netspeed->pix_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	spacer = gtk_label_new("");
	gtk_box_pack_start (GTK_BOX (netspeed->pix_box), spacer, TRUE, TRUE, 0);
	spacer = gtk_label_new("");
	gtk_box_pack_end (GTK_BOX (netspeed->pix_box), spacer, TRUE, TRUE, 0);

	spacer_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
	gtk_box_pack_start (GTK_BOX (netspeed->pix_box), spacer_box, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (spacer_box), netspeed->qual_pix, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (spacer_box), netspeed->dev_pix, FALSE, FALSE, 0);

	init_quality_pixbufs (netspeed);

	applet_change_size_or_orient (netspeed, gp_applet_get_orientation (applet));
	gtk_widget_show_all (GTK_WIDGET (applet));
	update_applet (netspeed);

	gp_applet_set_flags (applet, GP_APPLET_FLAGS_EXPAND_MINOR);

	netspeed_applet_setup_timeout (netspeed);

	g_signal_connect (applet, "size-allocate",
	                  G_CALLBACK (netspeed_applet_size_allocate),
	                  netspeed);

	g_signal_connect (gtk_icon_theme_get_default (), "changed",
	                  G_CALLBACK (icon_theme_changed_cb),
	                  netspeed);

	g_signal_connect (applet, "placement-changed",
	                  G_CALLBACK(placement_changed_cb),
	                  netspeed);

	setup_menu (applet);
}

GSettings *
netspeed_applet_get_settings (NetspeedApplet *netspeed)
{
	return netspeed->settings;
}

const gchar *
netspeed_applet_get_current_device_name (NetspeedApplet *netspeed)
{
	return netspeed->devinfo.name;
}

gchar *
netspeed_applet_get_auto_device_name (void)
{
	GList *devices;
	GList *ptr;
	gchar *device = NULL;

	devices = get_available_devices ();

	for (ptr = devices; ptr; ptr = ptr->next) {
		if (g_strcmp0 (ptr->data, "lo") != 0) {
			device = g_strdup (ptr->data);
			break;
		}
	}

	g_list_free_full (devices, g_free);

	if (device != NULL)
		return device;

	return g_strdup ("lo");
}

void
netspeed_applet_setup_about (GtkAboutDialog *dialog)
{
  const char *comments;
  const char **authors;
  const char *copyright;

  comments = _("A little applet that displays some information on the "
               "traffic on the specified network device");

  authors = (const char *[])
    {
      "Jörgen Scheibengruber <mfcn@gmx.de>",
      "Dennis Cranston <dennis_cranston@yahoo.com>",
      "Pedro Villavicencio Garrido <pvillavi@gnome.org>",
      "Benoît Dejean <benoit@placenet.org>",
      NULL
    };

  copyright = "Copyright 2002 - 2010 Jörgen Scheibengruber";

  gtk_about_dialog_set_comments (dialog, comments);

  gtk_about_dialog_set_authors (dialog, authors);
  gtk_about_dialog_set_translator_credits (dialog, _("translator-credits"));
  gtk_about_dialog_set_copyright (dialog, copyright);
}
