/*  netspeed.h
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
 *  Netspeed Applet was writen by Jörgen Scheibengruber <mfcn@gmx.de>
 */
 
 #ifndef _NETSPEED_H
 #define _NETSPEED_H
 
 
 /* Icons for the interfaces */
static const char* const dev_type_icon[DEV_UNKNOWN + 1] = {
	"gnome-dev-loopback",         //DEV_LO
	"gnome-dev-pci",              //DEV_ETHERNET
	"gnome-dev-wavelan",          //DEV_WIRELESS
	"gnome-dev-ppp",              //DEV_PPP,
	"gnome-dev-plip",             //DEV_PLIP,
	"gnome-dev-plip",             //DEV_SLIP
	"gnome-fs-network",           //DEV_UNKNOWN
};

static const char IN_ICON[] = "stock_navigate-next";
static const char OUT_ICON[] = "stock_navigate-prev";
static const char ERROR_ICON[] = "stock_dialog-error";
static const char LOGO_ICON[] = "netspeed_applet";

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
typedef struct
{
	PanelApplet *applet;
	GtkWidget *box,
	*in_box, *in_label, *in_pix,
	*out_box, *out_label, *out_pix,
	*sum_box, *sum_label, *dev_pix;
	
	gboolean labels_dont_shrink;
	
	DevInfo devinfo;
	gboolean device_has_changed;
		
	guint timeout_id;
	int refresh_time;
	char *up_cmd, *down_cmd;
	gboolean show_sum, show_bits;
	gboolean change_icon, auto_change_device;
	GdkColor in_color, out_color;
	int width;
	int font_size;
	
	GtkWidget *inbytes_text, *outbytes_text;
	GtkDialog *details, *settings;
	GtkDrawingArea *drawingarea;
	GtkWidget *network_device_combo;
	
	guint64 in_old[OLD_VALUES], out_old[OLD_VALUES];
	double max_graph, in_graph[GRAPH_VALUES], out_graph[GRAPH_VALUES];
	int index_graph;
	
	GtkWidget *connect_dialog;
	
	GtkTooltips *tooltips;
} NetspeedApplet;
 
 #endif
