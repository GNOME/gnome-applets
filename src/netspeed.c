/*  netspeed.c
 *
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
 *  Netspeed Applet was writen by J�rgen Scheibengruber <mfcn@gmx.de>
 */

#include "config.h"

#include <gnome.h>
#include <panel-applet.h>
#include <panel-applet-gconf.h>
#include <gconf/gconf-client.h>
#include "backend.h"
#include "netspeed.h"

static const char 
netspeed_applet_menu_xml [] =
	"<popup name=\"button3\">\n"
	"   <menuitem name=\"Properties Item\" verb=\"NetspeedAppletDetails\" label=\"%s\"\n"
	"             pixtype=\"stock\" pixname=\"gtk-dialog-info\"/>\n"
	"   <separator/>\n"
	"   <menuitem name=\"Properties Item\" verb=\"NetspeedAppletProperties\" label=\"%s\"\n"
	"             pixtype=\"stock\" pixname=\"gtk-properties\"/>\n"
	"   <menuitem name=\"Help Item\" verb=\"NetspeedAppletHelp\" label=\"%s\"\n"
	"             pixtype=\"stock\" pixname=\"gtk-help\"/>\n"
	"   <menuitem name=\"About Item\" verb=\"NetspeedAppletAbout\" label=\"%s\"\n"
	"             pixtype=\"stock\" pixname=\"gnome-stock-about\"/>\n"
	"</popup>\n";

/* Adds a Pango markup "size" to a bytestring
 */
void
add_markup_size(char **string, int size)
{
	char *tmp = *string;
	*string = g_strdup_printf("<span size=\"%d\">%s</span>", size * 1000, tmp);
	g_free(tmp);
}

/* Adds a Pango markup "foreground" to a bytestring
 */
void
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
applet_change_size_or_orient(PanelApplet *applet_widget, int arg1, NetspeedApplet *applet)
{
	int size;
	PanelAppletOrient orient;
	
	g_assert(applet);
	
	size = panel_applet_get_size(applet_widget);
	orient = panel_applet_get_orient(applet_widget);
	
	gtk_widget_ref(applet->in_pix);
	gtk_widget_ref(applet->in_label);
	gtk_widget_ref(applet->out_pix);
	gtk_widget_ref(applet->out_label);
	gtk_widget_ref(applet->sum_pix);
	gtk_widget_ref(applet->sum_label);

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
		gtk_container_remove(GTK_CONTAINER(applet->sum_box), applet->sum_pix);
		gtk_widget_destroy(applet->sum_box);
	}
	if (applet->box) {
		gtk_widget_destroy(applet->box);
	}
		
	if (orient == PANEL_APPLET_ORIENT_LEFT || orient == PANEL_APPLET_ORIENT_RIGHT) {
		applet->box = gtk_vbox_new(FALSE, 0);
		if (size > 96) {
			applet->sum_box = gtk_hbox_new(FALSE, 2);
			applet->in_box = gtk_hbox_new(FALSE, 1);
			applet->out_box = gtk_hbox_new(FALSE, 1);
		} else {	
			applet->sum_box = gtk_vbox_new(FALSE, 0);
			applet->in_box = gtk_vbox_new(FALSE, 0);
			applet->out_box = gtk_vbox_new(FALSE, 0);
		}
		applet->labels_dont_shrink = FALSE;
	} else {
		applet->in_box = gtk_hbox_new(FALSE, 1);
		applet->out_box = gtk_hbox_new(FALSE, 1);
		if (size < 48) {
			applet->sum_box = gtk_hbox_new(FALSE, 2);
			applet->box = gtk_hbox_new(FALSE, 1);
			applet->labels_dont_shrink = TRUE;
		} else {
			applet->sum_box = gtk_vbox_new(FALSE, 0);
			applet->box = gtk_vbox_new(FALSE, 0);
			applet->labels_dont_shrink = !applet->show_sum;
		}
	}		
	
	gtk_box_pack_start(GTK_BOX(applet->in_box), applet->in_pix, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(applet->in_box), applet->in_label, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(applet->out_box), applet->out_pix, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(applet->out_box), applet->out_label, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(applet->sum_box), applet->sum_pix, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(applet->sum_box), applet->sum_label, TRUE, TRUE, 0);
	
	gtk_widget_unref(applet->in_pix);
	gtk_widget_unref(applet->in_label);
	gtk_widget_unref(applet->out_pix);
	gtk_widget_unref(applet->out_label);
	gtk_widget_unref(applet->sum_pix);
	gtk_widget_unref(applet->sum_label);

	if (applet->show_sum) {
		gtk_box_pack_start(GTK_BOX(applet->box), applet->sum_box, TRUE, TRUE, 0);
	} else {
		gtk_box_pack_start(GTK_BOX(applet->box), applet->in_box, TRUE, TRUE, 0);
		gtk_box_pack_start(GTK_BOX(applet->box), applet->out_box, TRUE, TRUE, 0);
	}		
	
	gtk_widget_show_all(applet->box);
	gtk_container_add(GTK_CONTAINER(applet->applet), applet->box);
}

/* Change the background of the applet according to
 * the panel background.
 */
static void 
change_background_cb(PanelApplet *applet_widget, 
				PanelAppletBackgroundType type,
				GdkColor *color, GdkPixmap *pixmap, 
				NetspeedApplet *applet)
{
	GtkStyle *style;
	GtkRcStyle *rc_style = gtk_rc_style_new ();
	gtk_widget_set_style (GTK_WIDGET (applet_widget), NULL);
	gtk_widget_modify_style (GTK_WIDGET (applet_widget), rc_style);
	gtk_rc_style_unref (rc_style);

	switch (type) {
		case PANEL_PIXMAP_BACKGROUND:
			style = gtk_style_copy (GTK_WIDGET (applet_widget)->style);
			if(style->bg_pixmap[GTK_STATE_NORMAL])
				g_object_unref (style->bg_pixmap[GTK_STATE_NORMAL]);
			style->bg_pixmap[GTK_STATE_NORMAL] = g_object_ref (pixmap);
			gtk_widget_set_style (GTK_WIDGET(applet_widget), style);
			g_object_unref (style);
			break;

		case PANEL_COLOR_BACKGROUND:
			gtk_widget_modify_bg(GTK_WIDGET(applet_widget), GTK_STATE_NORMAL, color);
			break;

		case PANEL_NO_BACKGROUND:
			break;
	}
}


/* Change the icons according to the selected device
 * The icons are assembled out of the arrows a transparent
 * "base" xpm and the icons themselves
 * Some really black magic/voodoo is going on in here
 * hopefully my comments are understandable
 */
static void
change_icons(NetspeedApplet *applet)
{
	GdkPixbuf *in, *out, *sum, *type, *down = NULL;
	GdkPixbuf *in_arrow, *out_arrow;
	int w, h, x = 0, y = 1;
	char *device = applet->devinfo.name;
	
	/* If the user wants a different icon then the eth0, we load it
	 */
	if (applet->change_icon) {
		switch(applet->devinfo.type) {
			case DEV_LO:
				type = gdk_pixbuf_new_from_xpm_data(ICON_LO);
				break;
			case DEV_PLIP: case DEV_SLIP:
				type = gdk_pixbuf_new_from_xpm_data(ICON_PLIP);
				break;
			case DEV_PPP:
				type = gdk_pixbuf_new_from_xpm_data(ICON_PPP);
				break;
			case DEV_WIRELESS:
				type = gdk_pixbuf_new_from_xpm_data(ICON_WLAN);
				break;
			default:
			type = gdk_pixbuf_new_from_xpm_data(ICON_ETH);
		}
	} else type = gdk_pixbuf_new_from_xpm_data(ICON_ETH);
	
	/* Load a blank icon for in. Load one for out, too, if the user
	 * hasn' choosen "show_sum"
	 */
	if (applet->show_sum) {
		guchar *pixels;
		sum = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, SUMWIDTH, HEIGHT);
		pixels = gdk_pixbuf_get_pixels(sum);
		memset(pixels, 0, SUMWIDTH * HEIGHT * COL_SAMPLES);
	} else {
		guchar *pixels;
		in = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, WIDTH, HEIGHT);
		out = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, WIDTH, HEIGHT);
		pixels = gdk_pixbuf_get_pixels(in);
		memset(pixels, 0, WIDTH * HEIGHT * COL_SAMPLES);
		pixels = gdk_pixbuf_get_pixels(out);
		memset(pixels, 0, WIDTH * HEIGHT * COL_SAMPLES);
	}
	/* If the interface is down, load an small red cross
	 */
	if (!applet->devinfo.running)
		down =  gdk_pixbuf_new_from_xpm_data(ICON_DISCONNECT);

	w = gdk_pixbuf_get_width(type);
	h = gdk_pixbuf_get_height(type);
	
	/* If the interface is up, and we show the sum, then
	 * the icon should be centered vertically. 
	 */
	if (applet->show_sum) {
		if (applet->devinfo.running)
			y = (gdk_pixbuf_get_height(sum) - h) / 2;
		else y = 1;
		gdk_pixbuf_composite(type, sum, 0, y, w, h, 0, y, 1, 1, GDK_INTERP_NEAREST, 0xFF);
	} else {
		gdk_pixbuf_composite(type, out, 0, 1, w, h, 0, 1, 1, 1, GDK_INTERP_NEAREST, 0xFF);
		gdk_pixbuf_composite(type, in, 0, 1, w, h, 0, 1, 1, 1, GDK_INTERP_NEAREST, 0xFF);
	}
	gdk_pixbuf_unref(type);

	/* If the interface is down, we put the down icon in the 
	 * lower left corner
	 */
	if (down) {	
		w = gdk_pixbuf_get_width(down);
		h = gdk_pixbuf_get_height(down);
		if (applet->show_sum) {
			y = gdk_pixbuf_get_height(sum) - h;
			gdk_pixbuf_composite(down, sum, 0, y, w, h, 0, y, 1, 1, GDK_INTERP_NEAREST, 0xFF);
		} else {
			y = gdk_pixbuf_get_height(in) - h;
			gdk_pixbuf_composite(down, in, 0, y, w, h, 0, y, 1, 1, GDK_INTERP_NEAREST, 0xFF);
			y = gdk_pixbuf_get_height(out) - h;
			gdk_pixbuf_composite(down, out, 0, y, w, h, 0, y, 1, 1, GDK_INTERP_NEAREST, 0xFF);
		}
		gdk_pixbuf_unref(down);
		down = NULL;
	}
	
	/* If the applet shows the sum, we are finished here
	 */
	if (applet->show_sum) {
		gtk_image_set_from_pixbuf(GTK_IMAGE(applet->sum_pix), sum);
		gdk_pixbuf_unref(sum);
		return;
	}	
	
	/* Load the 2 arrow icons and put them in the lower right
	 * corner of the in and out pixpufs
	 */
	in_arrow = gdk_pixbuf_new_from_xpm_data(ICON_IN_ARROW);
	out_arrow = gdk_pixbuf_new_from_xpm_data(ICON_OUT_ARROW);
	w = gdk_pixbuf_get_width(in_arrow);
	h = gdk_pixbuf_get_height(in_arrow);
	x = gdk_pixbuf_get_width(in) - w;
	y = gdk_pixbuf_get_height(in) - h;
	gdk_pixbuf_composite(in_arrow, in, x, y, w, h, x, y, 1, 1, GDK_INTERP_NEAREST, 0xFF);
	gdk_pixbuf_composite(out_arrow, out, x, y, w, h, x, y, 1, 1, GDK_INTERP_NEAREST, 0xFF);
	
	/* Set the images and free all pixbufs
	 */
	gtk_image_set_from_pixbuf(GTK_IMAGE(applet->out_pix), out);
	gtk_image_set_from_pixbuf(GTK_IMAGE(applet->in_pix), in);
	gdk_pixbuf_unref(in_arrow);
	gdk_pixbuf_unref(out_arrow);
	gdk_pixbuf_unref(out);
	gdk_pixbuf_unref(in);
}	

/* Converts a number of bytes into a human
 * readable string - in [M/k]bytes[/s]
 * The string has to be freed
 */
char* 
bytes_to_string(double bytes, gboolean per_sec, gboolean bits)
{
	if (bits)
		bytes *= 8;
	
	if (bytes < 1000)
		return g_strdup_printf("%.0f %s", bytes, 
						per_sec ? bits ? _("b/s") : _("B/s"): _("byte"));
	if (bytes < 100000)
		return g_strdup_printf("%.1f %s", bytes / 1024, 
						per_sec ? bits ? _("kb/s") :_("kB/s") : _("kbyte"));
	if (bytes < 1000000)
		return g_strdup_printf("%.0f %s", bytes / 1024, 
						per_sec ? bits ? _("kb/s") : _("kB/s") : _("kbyte"));
	return g_strdup_printf("%.1f %s", bytes / 1024 / 1024, 
						per_sec ? bits ? _("Mb/s") : _("MB/s") : _("Mbyte"));
}

/* Redraws the graph drawingarea
 * Some really black magic is going on in here ;-)
 */
void
redraw_graph(NetspeedApplet *applet)
{
	GdkGC *gc;
	GdkColor color;
	GtkWidget *da = GTK_WIDGET(applet->drawingarea);
	GdkWindow *window, *real_window = da->window;
	GdkRectangle ra;
	GtkStateType state;
	GdkPoint in_points[GRAPH_VALUES], out_points[GRAPH_VALUES];
	PangoLayout *layout;
	PangoRectangle logical_rect;
	char *text; 
	int i, offset, w, h;
	double max_val;
	
	gc = gdk_gc_new (real_window);
	gdk_drawable_get_size(real_window, &w, &h);

	/* use doublebuffering to avoid flickering */
	window = gdk_pixmap_new(real_window, w, h, -1);
	
	/* the graph hight should be: hight/2 <= applet->max_graph < hight */
	for (max_val = 1; max_val < applet->max_graph; max_val *= 2) ;
	
	/* calculate the polygons (GdkPoint[]) for the graphs */ 
	offset = 0;
	for (i = applet->index_graph + 1; applet->in_graph[i] < 0; i = (i + 1) % GRAPH_VALUES)
		offset++;
	for (i = offset + 1; i < GRAPH_VALUES; i++)
	{
		int index = (applet->index_graph + i) % GRAPH_VALUES;
		out_points[i].x = in_points[i].x = ((w - 8) * i) / GRAPH_VALUES + 2;
		in_points[i].y = h - 6 - (int)((h - 8) * applet->in_graph[index] / max_val);
		out_points[i].y = h - 6 - (int)((h - 8) * applet->out_graph[index] / max_val);
	}	
	in_points[offset].x = out_points[offset].x = ((w - 8) * offset) / GRAPH_VALUES + 2;
	in_points[offset].y = in_points[offset + 1].y;
	out_points[offset].y = out_points[offset + 1].y;
	
/* draw the background */
	gdk_gc_set_rgb_fg_color (gc, &da->style->fg[GTK_STATE_NORMAL]);
	gdk_draw_rectangle (window, gc, TRUE, 0, 0, w, h);
	
	color.red = 0x3a00; color.green = 0x8000; color.blue = 0x1400;
	gdk_gc_set_rgb_fg_color(gc, &color);
	gdk_draw_rectangle (window, gc, FALSE, 2, 2, w - 6, h - 6);
	
	for (i = 0; i < GRAPH_LINES; i++) {
		int y = 2 + ((h - 6) * i) / GRAPH_LINES; 
		gdk_draw_line(window, gc, 2, y, w - 4, y);
	}
	
/* draw the polygons */
	gdk_gc_set_line_attributes(gc, 2, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);
	gdk_gc_set_rgb_fg_color(gc, &applet->in_color);
	gdk_draw_lines(window, gc, in_points + offset, GRAPH_VALUES - offset);
	gdk_gc_set_rgb_fg_color(gc, &applet->out_color);
	gdk_draw_lines(window, gc, out_points + offset, GRAPH_VALUES - offset);

/* draw the 2 labels */
	state = GTK_STATE_NORMAL;
	ra.x = 0; ra.y = 0;
	ra.width = w; ra.height = h;
	
	text = bytes_to_string(max_val, TRUE, applet->show_bits);
	add_markup_fgcolor(&text, "white");
	layout = gtk_widget_create_pango_layout (da, NULL);
	pango_layout_set_markup(layout, text, -1);
	g_free (text);
	gtk_paint_layout(da->style, window, state,	FALSE, &ra, da, "max_graph", 3, 2, layout);
	g_object_unref(G_OBJECT(layout));

	text = bytes_to_string(0.0, TRUE, applet->show_bits);
	add_markup_fgcolor(&text, "white");
	layout = gtk_widget_create_pango_layout (da, NULL);
	pango_layout_set_markup(layout, text, -1);
	pango_layout_get_pixel_extents (layout, NULL, &logical_rect);
	g_free (text);
	gtk_paint_layout(da->style, window, state,	FALSE, &ra, da, "max_graph", 3, h - 4 - logical_rect.height, layout);
	g_object_unref(G_OBJECT(layout));

/* draw the pixmap to the real window */	
	gdk_draw_drawable(real_window, gc, window, 0, 0, 0, 0, w, h);
	
	g_object_unref(G_OBJECT(gc));
	g_object_unref(G_OBJECT(window));
}

/* Find the first available device, that is running and != lo */
void
search_for_up_if(NetspeedApplet *applet)
{
	GList *devices, *tmp;
	DevInfo info;
	
	devices = get_available_devices();
	for (tmp = devices; tmp; tmp = g_list_next(tmp)) {
		if (!g_str_equal(tmp->data, "lo")) {
			info = get_device_info(tmp->data);
			if (info.running) {
				free_device_info(&applet->devinfo);
				applet->devinfo = info;
				applet->device_has_changed = TRUE;
				break;
			}
			free_device_info(&info);
		}
	}
	free_devices_list(devices);
}

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

/* Here happens the really interesting stuff */
static void
update_applet(NetspeedApplet *applet)
{
	guint64 indiff, outdiff;
	double inrate, outrate;
	char *in, *out, *sum;
	char *tooltip, *inbytes, *outbytes;
	int i;
	DevInfo oldinfo;
	
	if (!applet)	return;
	
/* First we try to figure out if the device has changed */
	oldinfo = applet->devinfo;
	applet->devinfo = get_device_info(oldinfo.name);
	if (compare_device_info(&applet->devinfo, &oldinfo))
		applet->device_has_changed = TRUE;
	free_device_info(&oldinfo);

/* If the device has changed, reintialize stuff */	
	if (applet->device_has_changed) {
		change_icons(applet);
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
		if (applet->devinfo.rx  < applet->in_old[OLD_VALUES - 1]) indiff = 0;
		else indiff = applet->devinfo.rx - applet->in_old[OLD_VALUES - 1];
		if (applet->devinfo.tx  < applet->out_old[OLD_VALUES - 1]) outdiff = 0;
		else outdiff = applet->devinfo.tx - applet->out_old[OLD_VALUES - 1];
		
		inrate = indiff * 1000.0;
		inrate /= (double)(applet->refresh_time * OLD_VALUES);
		outrate = outdiff * 1000.0;
		outrate /= (double)(applet->refresh_time * OLD_VALUES);
		
		applet->in_graph[applet->index_graph] = inrate;
		applet->out_graph[applet->index_graph] = outrate;
		applet->max_graph = max(inrate, applet->max_graph);
		applet->max_graph = max(outrate, applet->max_graph);
		
		in = bytes_to_string(inrate, TRUE, applet->show_bits);
		out = bytes_to_string(outrate, TRUE, applet->show_bits);
		sum = bytes_to_string(inrate + outrate, TRUE, applet->show_bits);
		
		if (applet->show_sum) {
			tooltip = g_strdup_printf(_("%s :%s\nin: %s out: %s"), 
				applet->devinfo.name, 
				applet->devinfo.ip ? applet->devinfo.ip : _("has no ip"),
				in, out);
		} else {
			tooltip = g_strdup_printf(_("%s: %s\nsum: %s"), 
				applet->devinfo.name, 
				applet->devinfo.ip ? applet->devinfo.ip : _("has no ip"), 
				sum);
		}
	} else {
		in = g_strdup("");
		out = g_strdup("");
		sum = g_strdup("");
		applet->in_graph[applet->index_graph] = 0;
		applet->out_graph[applet->index_graph] = 0;
		tooltip = g_strdup_printf(_("%s is down"), applet->devinfo.name);
	}
		
/* Refresh the text of the labels and tooltip */
	add_markup_size(&sum, applet->font_size);
	add_markup_size(&in, applet->font_size);
	add_markup_size(&out, applet->font_size);
	gtk_label_set_markup(GTK_LABEL(applet->sum_label), sum);
	gtk_label_set_markup(GTK_LABEL(applet->in_label), in);
	gtk_label_set_markup(GTK_LABEL(applet->out_label), out);
	gtk_tooltips_set_tip(applet->tooltips, GTK_WIDGET(applet->applet), tooltip, "");
	g_free(in);
	g_free(out);
	g_free(sum);
	g_free(tooltip);

/* Refresh the values of the Infodialog */
	if (applet->inbytes_text) {
		inbytes = bytes_to_string((double)applet->devinfo.rx, FALSE, FALSE);
		gtk_label_set_text(GTK_LABEL(applet->inbytes_text), inbytes);
		g_free(inbytes);
	}	
	if (applet->outbytes_text) {
		outbytes = bytes_to_string((double)applet->devinfo.tx, FALSE, FALSE);
		gtk_label_set_text(GTK_LABEL(applet->outbytes_text), outbytes);
		g_free(outbytes);
	}
/* Redraw the graph of the Infodialog */
	if (applet->drawingarea)
		redraw_graph(applet);
	
/* Save old values... */
	for (i = OLD_VALUES - 1; i > 0; i--)
	{
		applet->in_old[i] = applet->in_old[i - 1];
		applet->out_old[i] = applet->out_old[i - 1];
	}
	applet->in_old[0] = applet->devinfo.rx;
	applet->out_old[0] = applet->devinfo.tx;

/* Move the graphindex. Check if we can scale down again */
	applet->index_graph = (applet->index_graph + 1) % GRAPH_VALUES; 
	if (applet->index_graph % 20 == 0)
	{
		double max = 0;
		for (i = 0; i < GRAPH_VALUES; i++)
		{
			max = max(max, applet->in_graph[i]);
			max = max(max, applet->out_graph[i]);
		}
		applet->max_graph = max;
	}

/* If the device is down, lets look for a running one */
	if (!applet->devinfo.running && applet->auto_change_device)
		search_for_up_if(applet);
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

/* Opens gnome help application
 */
static void
help_cb (BonoboUIComponent *uic, gpointer data, const gchar *verbname)
{
	GError *error = NULL;

	gnome_help_display(PACKAGE, NULL, &error);
	if (error)
	{
		GtkWidget *dialog;
		dialog = gtk_message_dialog_new(NULL, 
				GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
				error->message);
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		g_error_free (error);
		error = NULL;
	}
}

/* Just the about window... If it's already open, just fokus it
 */
static void
about_cb(BonoboUIComponent *uic, gpointer data, const gchar *verbname)
{
	GdkPixbuf *icon = NULL;

	const char *authors[] = 
	{
		"Jörgen Scheibengruber <mfcn@gmx.de>", 
		"Dennis Cranston <dennis_cranston@yahoo.com> - HIGifing",
		NULL
	};

	/* Feel free to put your names here translators :-) */
	char *translators = _("TRANSLATORS");

	icon = gdk_pixbuf_new_from_xpm_data (ICON_APPLET);
	
	gtk_show_about_dialog (NULL, 
			       "name", _("Netspeed"), 
			       "version", VERSION, 
			       "copyright", "Copyright 2002 - 2003 Jörgen Scheibengruber",
			       "comments", _("A little applet that displays some information on the traffic on the specified network device"),
			       "authors", authors, 
			       "documenters", NULL, 
			       "translator-credits", strcmp ("TRANSLATORS", translators) ? translators : NULL, 
			       "logo", icon,
			       NULL);
	if (icon != NULL)
		gdk_pixbuf_unref (icon);
}



/* this basically just retrieves the new devicestring 
 * and then calls applet_device_change() and change_icons()
 */
static void
device_change_cb(GtkComboBox *combo, NetspeedApplet *applet)
{
	GList *ptr, *devices;
	int i, active;

	g_assert(combo);
	ptr = devices = g_object_get_data(G_OBJECT(combo), "devices");
	active = gtk_combo_box_get_active(combo);
	g_assert(active > -1);
	
	for (i = 0; i < active; i++) {
		ptr = g_list_next(ptr);
		g_assert(ptr);
	}
	g_assert(ptr);

	if (g_str_equal(ptr->data, applet->devinfo.name))
		return;
	free_device_info(&applet->devinfo);
	applet->devinfo = get_device_info(ptr->data);
	applet->device_has_changed = TRUE;
	update_applet(applet);
}

/* Display a section of netspeed help
 */
static void
display_help(const gchar *section)
{
	GError *error = NULL;

	gnome_help_display(PACKAGE, section, &error);
	if (error)
	{
		GtkWidget *dialog;
		dialog = gtk_message_dialog_new(NULL, 
				GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
				error->message);
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		g_error_free (error);
		error = NULL;
	}
}

/* Handle preference dialog response event
 */
static void
pref_response_cb (GtkDialog *dialog, gint id, gpointer data)
{
    NetspeedApplet *applet = data;
  
    if(id == GTK_RESPONSE_HELP){
        display_help("netspeed_applet-settings");
	return;
    }
    panel_applet_gconf_set_string(PANEL_APPLET(applet->applet), "device", applet->devinfo.name, NULL);
    panel_applet_gconf_set_int(PANEL_APPLET(applet->applet), "refresh_time", applet->refresh_time, NULL);
    panel_applet_gconf_set_int(PANEL_APPLET(applet->applet), "font_size", applet->font_size, NULL);
    panel_applet_gconf_set_bool(PANEL_APPLET(applet->applet), "show_sum", applet->show_sum, NULL);
    panel_applet_gconf_set_bool(PANEL_APPLET(applet->applet), "show_bits", applet->show_bits, NULL);
    panel_applet_gconf_set_bool(PANEL_APPLET(applet->applet), "change_icon", applet->change_icon, NULL);
    panel_applet_gconf_set_bool(PANEL_APPLET(applet->applet), "auto_change_device", applet->auto_change_device, NULL);
    panel_applet_gconf_set_bool(PANEL_APPLET(applet->applet), "have_settings", TRUE, NULL);

    gtk_widget_destroy(GTK_WIDGET(applet->settings));
    applet->settings = NULL;
}

/* Set the timeout to the new value
 */
static void
timeout_adjust_cb(GtkSpinButton *spinbutton, NetspeedApplet *applet)
{
	applet->refresh_time = gtk_spin_button_get_value_as_int(spinbutton);

	if (applet->timeout_id)
			gtk_timeout_remove(applet->timeout_id);
	applet->timeout_id = gtk_timeout_add(applet->refresh_time, (GtkFunction)timeout_function, (gpointer)applet);
}

/* Set the timeout to the new value
 */
static void
font_size_adjust_cb(GtkSpinButton *spinbutton, NetspeedApplet *applet)
{
	applet->font_size = gtk_spin_button_get_value_as_int(spinbutton);
	applet->width = 0;
}

/* Called when the showsum checkbutton is toggled...
 */
static void
showsum_change_cb(GtkToggleButton *togglebutton, NetspeedApplet *applet)
{
	applet->show_sum = gtk_toggle_button_get_active(togglebutton);
	applet_change_size_or_orient(applet->applet, -1, (gpointer)applet);
	change_icons(applet);
}

/* Called when the showbits checkbutton is toggled...
 */
static void
showbits_change_cb(GtkToggleButton *togglebutton, NetspeedApplet *applet)
{
	applet->show_bits = gtk_toggle_button_get_active(togglebutton);
}

/* Called when the changeicon checkbutton is toggled...
 */
static void
changeicon_change_cb(GtkToggleButton *togglebutton, NetspeedApplet *applet)
{
	applet->change_icon = gtk_toggle_button_get_active(togglebutton);
	change_icons(applet);
}

/* Called when the auto_change_device checkbutton is toggled...
 */
static void
auto_change_device_cb(GtkToggleButton *togglebutton, NetspeedApplet *applet)
{
	applet->auto_change_device = gtk_toggle_button_get_active(togglebutton);
}

/* Creates the settings dialog
 * After its been closed, take the new values and store
 * them in the gconf database
 */
static void
settings_cb(BonoboUIComponent *uic, gpointer data, const gchar *verbname)
{
	NetspeedApplet *applet = (NetspeedApplet*)data;
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *categories_vbox;
	GtkWidget *category_vbox;
	GtkWidget *controls_vbox;
	GtkWidget *category_header_label;
	GtkWidget *network_device_hbox;
	GtkWidget *network_device_label;
	GtkWidget *network_device_combo;
	GtkWidget *update_interval_hbox;
	GtkWidget *update_interval_hbox2;
	GtkWidget *update_interval_label;
	GtkWidget *update_interval_spinbutton;
	GtkObject *update_interval_spinbutton_adj;
	GtkWidget *label_font_size_hbox;
	GtkWidget *label_font_size_hbox2;
	GtkWidget *label_font_size_label;
	GtkWidget *label_font_size_spinbutton;
	GtkWidget *indent_label;
	GtkWidget *points_label;
	GtkWidget *millisecond_label;
	GtkWidget *show_sum_checkbutton;
	GtkWidget *show_bits_checkbutton;
	GtkWidget *change_icon_checkbutton;
	GtkWidget *auto_change_device_checkbutton;
	GtkSizeGroup *category_label_size_group;
	GtkSizeGroup *category_units_size_group;
  	gchar *header_str;
	GList *ptr, *devices;
	int i, active = -1;
	
	g_assert(applet);
	
	if (applet->settings)
	{
		gtk_window_present(GTK_WINDOW(applet->settings));
		return;
	}

	category_label_size_group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	category_units_size_group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
  
	applet->settings = GTK_DIALOG(gtk_dialog_new_with_buttons(_("Netspeed Preferences"), 
								  NULL, 
								  GTK_DIALOG_DESTROY_WITH_PARENT |
								  GTK_DIALOG_NO_SEPARATOR,	
								  GTK_STOCK_HELP, GTK_RESPONSE_HELP, 
								  GTK_STOCK_CLOSE, GTK_RESPONSE_ACCEPT, 
								  NULL));
	
	gtk_window_set_resizable(GTK_WINDOW(applet->settings), FALSE);
	gtk_window_set_screen(GTK_WINDOW(applet->settings), 
			      gtk_widget_get_screen(GTK_WIDGET(applet->settings)));
			       
	gtk_dialog_set_default_response(GTK_DIALOG(applet->settings), GTK_RESPONSE_CLOSE);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 12);

	categories_vbox = gtk_vbox_new(FALSE, 18);
	gtk_box_pack_start(GTK_BOX (vbox), categories_vbox, TRUE, TRUE, 0);

	category_vbox = gtk_vbox_new(FALSE, 6);
	gtk_box_pack_start(GTK_BOX (categories_vbox), category_vbox, TRUE, TRUE, 0);
	
	header_str = g_strconcat("<span weight=\"bold\">", _("General Settings"), "</span>", NULL);
	category_header_label = gtk_label_new(header_str);
	gtk_label_set_use_markup(GTK_LABEL(category_header_label), TRUE);
	gtk_label_set_justify(GTK_LABEL(category_header_label), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment(GTK_MISC (category_header_label), 0, 0.5);
	gtk_box_pack_start(GTK_BOX (category_vbox), category_header_label, FALSE, FALSE, 0);
	g_free(header_str);
	
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX (category_vbox), hbox, TRUE, TRUE, 0);

	indent_label = gtk_label_new("    ");
	gtk_label_set_justify(GTK_LABEL (indent_label), GTK_JUSTIFY_LEFT);
	gtk_box_pack_start(GTK_BOX (hbox), indent_label, FALSE, FALSE, 0);
		
	controls_vbox = gtk_vbox_new(FALSE, 10);
	gtk_box_pack_start(GTK_BOX(hbox), controls_vbox, TRUE, TRUE, 0);

	network_device_hbox = gtk_hbox_new(FALSE, 6);
	gtk_box_pack_start(GTK_BOX(controls_vbox), network_device_hbox, TRUE, TRUE, 0);
	
	network_device_label = gtk_label_new_with_mnemonic(_("Network _device:"));
	gtk_label_set_justify(GTK_LABEL(network_device_label), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment(GTK_MISC(network_device_label), 0.0f, 0.5f);
	gtk_size_group_add_widget(category_label_size_group, network_device_label);
	gtk_box_pack_start(GTK_BOX (network_device_hbox), network_device_label, FALSE, FALSE, 0);
	
	network_device_combo = gtk_combo_box_new_text();
	gtk_label_set_mnemonic_widget(GTK_LABEL(network_device_label), network_device_combo);
	gtk_box_pack_start (GTK_BOX (network_device_hbox), network_device_combo, TRUE, TRUE, 0);

	ptr = devices = get_available_devices();
	for (i = 0; ptr; ptr = g_list_next(ptr)) {
		gtk_combo_box_append_text(GTK_COMBO_BOX(network_device_combo), ptr->data);
		if (g_str_equal(ptr->data, applet->devinfo.name)) active = i;
		++i;
	}
	if (active < 0) active = 0;
	gtk_combo_box_set_active(GTK_COMBO_BOX(network_device_combo), active);
	g_object_set_data_full(G_OBJECT(network_device_combo), "devices", devices, (GDestroyNotify)free_devices_list);
	
	update_interval_hbox = gtk_hbox_new(FALSE, 6);
	gtk_box_pack_start(GTK_BOX(controls_vbox), update_interval_hbox, TRUE, TRUE, 0);
	
	update_interval_label = gtk_label_new_with_mnemonic(_("_Update interval:"));
	gtk_label_set_justify(GTK_LABEL(update_interval_label), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment(GTK_MISC(update_interval_label), 0.0f, 0.5f);
	gtk_size_group_add_widget(category_label_size_group, update_interval_label);
	gtk_box_pack_start(GTK_BOX(update_interval_hbox), update_interval_label, FALSE, FALSE, 0);

	update_interval_hbox2 = gtk_hbox_new(FALSE, 6);
	gtk_box_pack_start(GTK_BOX(update_interval_hbox), update_interval_hbox2, TRUE, TRUE, 0);

	update_interval_spinbutton_adj = gtk_adjustment_new((double)(applet->refresh_time), 50.0f, 10000.0f, 50.0f, 500.0f, 500.0f);;
	update_interval_spinbutton = gtk_spin_button_new(GTK_ADJUSTMENT (update_interval_spinbutton_adj), 50, 0);
	gtk_spin_button_set_snap_to_ticks(GTK_SPIN_BUTTON (update_interval_spinbutton), TRUE);
	gtk_spin_button_set_digits(GTK_SPIN_BUTTON (update_interval_spinbutton), 0);
	gtk_label_set_mnemonic_widget(GTK_LABEL(update_interval_label), update_interval_spinbutton);
	gtk_box_pack_start(GTK_BOX (update_interval_hbox2), update_interval_spinbutton, TRUE, TRUE, 0);
	
	millisecond_label = gtk_label_new(_("millisecond"));
	gtk_label_set_justify(GTK_LABEL(millisecond_label), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment(GTK_MISC(millisecond_label), 0.0f, 0.5f);
	gtk_size_group_add_widget(category_units_size_group, millisecond_label);
	gtk_box_pack_start(GTK_BOX(update_interval_hbox2), millisecond_label, FALSE, FALSE, 0);

	label_font_size_hbox = gtk_hbox_new(FALSE, 6);
	gtk_box_pack_start(GTK_BOX(controls_vbox), label_font_size_hbox, TRUE, TRUE, 0);

	label_font_size_label = gtk_label_new_with_mnemonic(_("Label _font size:"));
	gtk_label_set_justify(GTK_LABEL(label_font_size_label), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment(GTK_MISC(label_font_size_label), 0.0f, 0.5f);
	gtk_size_group_add_widget(category_label_size_group, label_font_size_label);
	gtk_box_pack_start(GTK_BOX(label_font_size_hbox), label_font_size_label, FALSE, FALSE, 0);
	
	label_font_size_hbox2 = gtk_hbox_new(FALSE, 6);
	gtk_box_pack_start(GTK_BOX(label_font_size_hbox), label_font_size_hbox2, TRUE, TRUE, 0);
	
	label_font_size_spinbutton = gtk_spin_button_new_with_range (5.0, 32.0, 1.0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(label_font_size_spinbutton), (double) applet->font_size);
	gtk_spin_button_set_snap_to_ticks(GTK_SPIN_BUTTON(label_font_size_spinbutton), TRUE);
	gtk_spin_button_set_digits(GTK_SPIN_BUTTON (label_font_size_spinbutton), 0);
	gtk_label_set_mnemonic_widget(GTK_LABEL(label_font_size_label), label_font_size_spinbutton);
	gtk_box_pack_start(GTK_BOX(label_font_size_hbox2), label_font_size_spinbutton, TRUE, TRUE, 0);
	
	points_label = gtk_label_new(_("points"));
	gtk_label_set_justify(GTK_LABEL (points_label), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment(GTK_MISC (points_label), 0.0f, 0.5f);
	gtk_size_group_add_widget(category_units_size_group, points_label);
	gtk_box_pack_start(GTK_BOX (label_font_size_hbox2), points_label, FALSE, FALSE, 0);
	
	show_sum_checkbutton = gtk_check_button_new_with_mnemonic(_("Show _sum instead of in & out"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(show_sum_checkbutton), applet->show_sum);
	gtk_box_pack_start(GTK_BOX(controls_vbox), show_sum_checkbutton, FALSE, FALSE, 0);
	
	show_bits_checkbutton = gtk_check_button_new_with_mnemonic(_("Show _bits/s (b/s) instead of bytes/s (B/s)"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(show_bits_checkbutton), applet->show_bits);
	gtk_box_pack_start(GTK_BOX(controls_vbox), show_bits_checkbutton, FALSE, FALSE, 0);
	
	change_icon_checkbutton = gtk_check_button_new_with_mnemonic(_("Change _icon according to the selected device"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(change_icon_checkbutton), applet->change_icon);
  	gtk_box_pack_start(GTK_BOX(controls_vbox), change_icon_checkbutton, FALSE, FALSE, 0);

	auto_change_device_checkbutton = 
		gtk_check_button_new_with_mnemonic(_("_Always monitor a connected device, if possible"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(auto_change_device_checkbutton), applet->auto_change_device);
  	gtk_box_pack_start(GTK_BOX(controls_vbox), auto_change_device_checkbutton, FALSE, FALSE, 0);

	g_signal_connect(G_OBJECT (network_device_combo), "changed",
			 G_CALLBACK(device_change_cb), (gpointer)applet);

	g_signal_connect(G_OBJECT (update_interval_spinbutton), "value-changed",
			 G_CALLBACK(timeout_adjust_cb), (gpointer)applet);

	g_signal_connect(G_OBJECT (label_font_size_spinbutton), "value-changed",
			 G_CALLBACK(font_size_adjust_cb), (gpointer)applet);

	g_signal_connect(G_OBJECT (show_sum_checkbutton), "toggled",
			 G_CALLBACK(showsum_change_cb), (gpointer)applet);

	g_signal_connect(G_OBJECT (show_bits_checkbutton), "toggled",
			 G_CALLBACK(showbits_change_cb), (gpointer)applet);

	g_signal_connect(G_OBJECT (change_icon_checkbutton), "toggled",
			 G_CALLBACK(changeicon_change_cb), (gpointer)applet);

	g_signal_connect(G_OBJECT (auto_change_device_checkbutton), "toggled",
			 G_CALLBACK(auto_change_device_cb), (gpointer)applet);

	g_signal_connect(G_OBJECT (applet->settings), "response",
			 G_CALLBACK(pref_response_cb), (gpointer)applet);
		      
	gtk_container_add(GTK_CONTAINER(applet->settings->vbox), vbox); 

	gtk_widget_show_all(GTK_WIDGET(applet->settings));
}

gboolean
da_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
	NetspeedApplet *applet = (NetspeedApplet*)data;
	
	redraw_graph(applet);
	
	return FALSE;
}	

void		
incolor_changed_cb(GnomeColorPicker *cp, guint r, guint g, guint b,
		      guint a, gpointer data)
{
	NetspeedApplet *applet = (NetspeedApplet*)data;
	gchar *color;
	
	applet->in_color.red = r;
	applet->in_color.green = g;
	applet->in_color.blue = b;
	
	color = g_strdup_printf("#%04x%04x%04x", r, g, b);
	panel_applet_gconf_set_string(PANEL_APPLET(applet->applet), "in_color", color, NULL);
	panel_applet_gconf_set_bool(PANEL_APPLET(applet->applet), "have_settings", TRUE, NULL);
	g_free(color);
}

void		
outcolor_changed_cb(GnomeColorPicker *cp, guint r, guint g, guint b,
		      guint a, gpointer data)
{
	NetspeedApplet *applet = (NetspeedApplet*)data;
	gchar *color;
	
	applet->out_color.red = r;
	applet->out_color.green = g;
	applet->out_color.blue = b;
	
	color = g_strdup_printf("#%04x%04x%04x", r, g, b);
	panel_applet_gconf_set_string(PANEL_APPLET(applet->applet), "out_color", color, NULL);
	g_free(color);
	panel_applet_gconf_set_bool(PANEL_APPLET(applet->applet), "have_settings", TRUE, NULL);
}

/* Handle info dialog response event
 */
static void
info_response_cb (GtkDialog *dialog, gint id, NetspeedApplet *applet)
{
  
	if(id == GTK_RESPONSE_HELP){
		display_help("netspeed_applet-details");
		return;
	}
	
	gtk_widget_destroy(GTK_WIDGET(applet->details));
	
	applet->details = NULL;
	applet->inbytes_text = NULL;
	applet->outbytes_text = NULL;
	applet->drawingarea = NULL;
}

/* Creates the details dialog
 */
static void
showinfo_cb(BonoboUIComponent *uic, gpointer data, const gchar *verbname)
{
	NetspeedApplet *applet = (NetspeedApplet*)data;
	GtkWidget *box, *hbox;
	GtkWidget *table, *da_frame;
	GtkWidget *ip_label, *netmask_label;
	GtkWidget *hwaddr_label, *ptpip_label;
	GtkWidget *ip_text, *netmask_text;
	GtkWidget *hwaddr_text, *ptpip_text;
	GtkWidget *inbytes_label, *outbytes_label;
	GtkWidget *incolor_sel, *incolor_label;
	GtkWidget *outcolor_sel, *outcolor_label;
	char *title;
	
	g_assert(applet);
	
	if (applet->details)
	{
		gtk_window_present(GTK_WINDOW(applet->details));
		return;
	}
	
	title = g_strdup_printf(_("Device Details for %s"), applet->devinfo.name);
	applet->details = GTK_DIALOG(gtk_dialog_new_with_buttons(title, 
		NULL, 
		GTK_DIALOG_DESTROY_WITH_PARENT |
	 	GTK_DIALOG_NO_SEPARATOR,
		GTK_STOCK_CLOSE, GTK_RESPONSE_ACCEPT, 
		GTK_STOCK_HELP, GTK_RESPONSE_HELP,
		NULL));
	g_free(title);

	gtk_dialog_set_default_response(GTK_DIALOG(applet->details), GTK_RESPONSE_CLOSE);
	
	box = gtk_vbox_new(FALSE, 10);
	gtk_container_set_border_width(GTK_CONTAINER(box), 12);
	
	table = gtk_table_new(3, 4, FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table), 10);
	gtk_table_set_col_spacings(GTK_TABLE(table), 15);
	
	da_frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(da_frame), GTK_SHADOW_IN);
	applet->drawingarea = GTK_DRAWING_AREA(gtk_drawing_area_new());
	gtk_widget_set_size_request(GTK_WIDGET(applet->drawingarea), -1, 180);
	gtk_container_add(GTK_CONTAINER(da_frame), GTK_WIDGET(applet->drawingarea));
	
	hbox = gtk_hbox_new(FALSE, 5);
	incolor_label = gtk_label_new_with_mnemonic(_("_In graph color"));
	outcolor_label = gtk_label_new_with_mnemonic(_("_Out graph color"));
	incolor_sel = gnome_color_picker_new();
	outcolor_sel = gnome_color_picker_new();
	gnome_color_picker_set_i16(GNOME_COLOR_PICKER(incolor_sel), 
		applet->in_color.red, 
		applet->in_color.green,
		applet->in_color.blue, 0xFF);
	gnome_color_picker_set_i16(GNOME_COLOR_PICKER(outcolor_sel), 
		applet->out_color.red, 
		applet->out_color.green,
		applet->out_color.blue, 0xFF);
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

	gtk_table_attach_defaults(GTK_TABLE(table), ip_label, 0, 1, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(table), ip_text, 1, 2, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(table), netmask_label, 2, 3, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(table), netmask_text, 3, 4, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(table), hwaddr_label, 0, 1, 1, 2);
	gtk_table_attach_defaults(GTK_TABLE(table), hwaddr_text, 1, 2, 1, 2);
	gtk_table_attach_defaults(GTK_TABLE(table), ptpip_label, 2, 3, 1, 2);
	gtk_table_attach_defaults(GTK_TABLE(table), ptpip_text, 3, 4, 1, 2);
	gtk_table_attach_defaults(GTK_TABLE(table), inbytes_label, 0, 1, 2, 3);
	gtk_table_attach_defaults(GTK_TABLE(table), applet->inbytes_text, 1, 2, 2, 3);
	gtk_table_attach_defaults(GTK_TABLE(table), outbytes_label, 2, 3, 2, 3);
	gtk_table_attach_defaults(GTK_TABLE(table), applet->outbytes_text, 3, 4, 2, 3);
	
	g_signal_connect(G_OBJECT(applet->drawingarea), "expose_event",
                           GTK_SIGNAL_FUNC(da_expose_event),
                           (gpointer)applet);
	g_signal_connect(G_OBJECT(incolor_sel), "color_set", 
							G_CALLBACK(incolor_changed_cb),
							(gpointer)applet);
	g_signal_connect(G_OBJECT(outcolor_sel), "color_set",
							G_CALLBACK(outcolor_changed_cb),
							(gpointer)applet);
	g_signal_connect(G_OBJECT(applet->details), "response",
			 				G_CALLBACK(info_response_cb), (gpointer)applet);

	gtk_box_pack_start(GTK_BOX(box), da_frame, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), table, FALSE, FALSE, 0);

	gtk_container_add(GTK_CONTAINER(applet->details->vbox), box); 
	gtk_widget_show_all(GTK_WIDGET(applet->details));
}	

static const BonoboUIVerb
netspeed_applet_menu_verbs [] = 
{
		BONOBO_UI_VERB("NetspeedAppletDetails", showinfo_cb),
		BONOBO_UI_VERB("NetspeedAppletProperties", settings_cb),
		BONOBO_UI_VERB("NetspeedAppletHelp", help_cb),
		BONOBO_UI_VERB("NetspeedAppletAbout", about_cb),
	
		BONOBO_UI_VERB_END
};

/* Block the size_request signal emit by the label if the
 * text changes. Only if the label wants to grow, we give
 * permission. This will eventually result in the maximal
 * size of the applet and prevents the icons and labels from
 * "jumping around" in the panel which looks uggly
 */
static void
label_size_request_cb(GtkWidget *widget, GtkRequisition *requisition, NetspeedApplet *applet)
{
	if (applet->labels_dont_shrink) {
		if (requisition->width <= applet->width)
			requisition->width = applet->width;
		else
			applet->width = requisition->width;
	}
}	

/* Tries to get the desktop font size out of gconf
 * database
 */
int
get_default_font_size()
{
	int ret = 12;
	char *buf, *ptr;
	
	GConfClient *client = NULL;
	client = gconf_client_get_default();
	if (!client)
		return 12;
	buf = gconf_client_get_string(client, "/desktop/gnome/interface/font_name", NULL);
	if (!buf)
		return 12;
	ptr = strrchr(buf, ' ');
	if (ptr)
		sscanf(ptr, "%d", &ret);
	g_free(buf);
		
	return ret;
}

gboolean 
applet_button_press(GtkWidget *widget, GdkEventButton *event, NetspeedApplet *applet)
{
	if (event->button == 1)
	{
		GError *error = NULL;
		
		if (applet->connect_dialog) 
		{	
			gtk_window_present(GTK_WINDOW(applet->connect_dialog));
			return FALSE;
		}
		
		if (applet->up_cmd && applet->down_cmd)
		{
			char *question;
			int response;
			
			if (applet->devinfo.up) 
			{
				question = g_strdup_printf(_("Do you want to disconnect %s now?"), applet->devinfo.name); 
			} 
			else
			{
				question = g_strdup_printf(_("Do you want to connect %s now?"), applet->devinfo.name);
			}
			
			applet->connect_dialog = gtk_message_dialog_new(NULL, 
					GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
					question);
			response = gtk_dialog_run(GTK_DIALOG(applet->connect_dialog));
			gtk_widget_destroy (applet->connect_dialog);
			applet->connect_dialog = NULL;
			g_free(question);
			
			if (response == GTK_RESPONSE_YES)
			{
				GtkWidget *dialog;
				char *command;
				
				command = g_strdup_printf("%s %s", 
					applet->devinfo.up ? applet->down_cmd : applet->up_cmd,
					applet->devinfo.name);

				if (!g_spawn_command_line_async(command, &error))
				{
					dialog = gtk_message_dialog_new(NULL, 
							GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
							GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
							error->message);
					gtk_dialog_run (GTK_DIALOG (dialog));
					gtk_widget_destroy (dialog);
					g_error_free (error);
				}
				g_free(command);
			} 
		}	
	}
	
	return FALSE;
}	

/* Frees the applet and all the data it contains
 * Removes the timeout_cb
 */
static void
applet_destroy(PanelApplet *applet_widget, NetspeedApplet *applet)
{
	g_assert(applet);
	
	gtk_timeout_remove(applet->timeout_id);
	applet->timeout_id = 0;
	
	if (applet->up_cmd)
		g_free(applet->up_cmd);
	if (applet->down_cmd)
		g_free(applet->down_cmd);
	
	/* Should never be NULL */
	free_device_info(&applet->devinfo);
	g_free(applet);
	return;
}

/* The "main" function of the applet
 */
gboolean
netspeed_applet_factory(PanelApplet *applet_widget, const gchar *iid, gpointer data)
{
	NetspeedApplet *applet;
	int i;
	char* menu_string;
	GdkPixbuf *icon;
	GList *iconlist = NULL;
	
	if (strcmp (iid, "OAFIID:GNOME_NetspeedApplet"))
		return FALSE;
	
/* Set the windowmanager icon for the applet
 */
	icon = gdk_pixbuf_new_from_xpm_data(ICON_ETH);
	iconlist = g_list_append(NULL, (gpointer)icon);
	gtk_window_set_default_icon_list(iconlist);
	gdk_pixbuf_unref(icon);
	
/* Alloc the applet. The "NULL-setting" is really redudant
 * but aren't we paranoid?
 */
	applet = g_malloc0(sizeof(NetspeedApplet));
	applet->applet = applet_widget;
	memset(&applet->devinfo, 0, sizeof(DevInfo));
	applet->refresh_time = 1000.0;
	applet->show_sum = FALSE;
	applet->show_bits = FALSE;
	applet->change_icon = TRUE;
	applet->font_size = -1;
	applet->auto_change_device = FALSE;

/* Set the default colors of the graph
 */
	applet->in_color.red = 0xdf00;
	applet->in_color.green = 0x2800;
	applet->in_color.blue = 0x4700;
	applet->out_color.red = 0x3700;
	applet->out_color.green = 0x2800;
	applet->out_color.blue = 0xdf00;
	
	for (i = 0; i < GRAPH_VALUES; i++)
	{
		applet->in_graph[i] = -1;
		applet->out_graph[i] = -1;
	}	
	
/* Get stored settings from the gconf database
 */
	if (panel_applet_gconf_get_bool(PANEL_APPLET(applet->applet), "have_settings", NULL))
	{	
		char *tmp = NULL;
		
		applet->refresh_time = panel_applet_gconf_get_int(applet_widget, "refresh_time", NULL);
		applet->show_sum = panel_applet_gconf_get_bool(applet_widget, "show_sum", NULL);
		applet->show_bits = panel_applet_gconf_get_bool(applet_widget, "show_bits", NULL);
		applet->change_icon = panel_applet_gconf_get_bool(applet_widget, "change_icon", NULL);
		applet->auto_change_device = panel_applet_gconf_get_bool(applet_widget, "auto_change_device", NULL);
		applet->font_size = panel_applet_gconf_get_int(applet_widget, "font_size", NULL);
		if (!applet->font_size)
			applet->font_size = -1;
		if ((applet->refresh_time <= 50) || (applet->refresh_time > 10000))
			applet->refresh_time = 1000;

		tmp = panel_applet_gconf_get_string(applet_widget, "device", NULL);
		if (tmp && strcmp(tmp, "")) 
		{
			applet->devinfo = get_device_info(tmp);
			g_free(tmp);
		}
		tmp = panel_applet_gconf_get_string(applet_widget, "up_command", NULL);
		if (tmp && strcmp(tmp, "")) 
		{
			applet->up_cmd = g_strdup(tmp);
			g_free(tmp);
		}
		tmp = panel_applet_gconf_get_string(applet_widget, "down_command", NULL);
		if (tmp && strcmp(tmp, "")) 
		{
			applet->down_cmd = g_strdup(tmp);
			g_free(tmp);
		}
		
		tmp = panel_applet_gconf_get_string(applet_widget, "in_color", NULL);
		if (tmp)
		{
			gdk_color_parse(tmp, &applet->in_color);
			g_free(tmp);
		}
		tmp = panel_applet_gconf_get_string(applet_widget, "out_color", NULL);
		if (tmp)
		{
			gdk_color_parse(tmp, &applet->out_color);
			g_free(tmp);
		}
	}

	if (applet->font_size < 1)
		applet->font_size = get_default_font_size();
	
	if (!applet->devinfo.name)  {
		GList *ptr, *devices = get_available_devices();
		ptr = devices;
		while (ptr) { 
			if (!g_str_equal(ptr->data, "lo"))
				applet->devinfo = get_device_info(ptr->data);
			ptr = g_list_next(ptr);
		}
		free_devices_list(devices);		
	}
	if (!applet->devinfo.name) applet->devinfo = get_device_info("lo");	
	applet->device_has_changed = TRUE;	
	
	applet->tooltips = gtk_tooltips_new();
	
	applet->in_label = gtk_label_new("");
	applet->out_label = gtk_label_new("");
	applet->sum_label = gtk_label_new("");
	
	applet->in_pix = gtk_image_new();
	applet->out_pix = gtk_image_new();
	applet->sum_pix = gtk_image_new();
	
	applet_change_size_or_orient(applet_widget, -1, (gpointer)applet);

	update_applet(applet);
	gtk_widget_show_all(GTK_WIDGET(applet_widget));

	panel_applet_set_flags(applet_widget, PANEL_APPLET_EXPAND_MINOR);
	
	applet->timeout_id = gtk_timeout_add(applet->refresh_time, (GtkFunction)timeout_function, (gpointer)applet);
	g_signal_connect(G_OBJECT(applet_widget), "change_size",
                           G_CALLBACK(applet_change_size_or_orient),
                           (gpointer)applet);

	g_signal_connect(G_OBJECT(applet_widget), "change_orient",
                           G_CALLBACK(applet_change_size_or_orient),
                           (gpointer)applet);

	g_signal_connect(G_OBJECT(applet->in_label), "size_request",
                           G_CALLBACK(label_size_request_cb),
                           (gpointer)applet);

	g_signal_connect(G_OBJECT(applet_widget), "change_background",
                           G_CALLBACK(change_background_cb),
			   (gpointer)applet);
		       
	g_signal_connect(G_OBJECT(applet->out_label), "size_request",
                           G_CALLBACK(label_size_request_cb),
                           (gpointer)applet);

	g_signal_connect(G_OBJECT(applet_widget), "destroy",
                           G_CALLBACK(applet_destroy),
                           (gpointer)applet);

	g_signal_connect(G_OBJECT(applet_widget), "button-press-event",
                           G_CALLBACK(applet_button_press),
                           (gpointer)applet);

	menu_string = g_strdup_printf(netspeed_applet_menu_xml, _("Device _Details"), _("_Preferences..."), _("_Help"), _("_About..."));
	panel_applet_setup_menu(applet_widget, menu_string,
                                 netspeed_applet_menu_verbs,
                                 (gpointer)applet);
	g_free(menu_string);
	
	return TRUE;
}

PANEL_APPLET_BONOBO_FACTORY("OAFIID:GNOME_NetspeedApplet_Factory", PANEL_TYPE_APPLET,
			PACKAGE, VERSION, (PanelAppletFactoryCallback)netspeed_applet_factory, NULL)
