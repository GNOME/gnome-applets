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
 
#include "../pixmaps/in_arrow.xpm"
#include "../pixmaps/out_arrow.xpm"
#include "../pixmaps/ethernet.xpm"
#include "../pixmaps/ppp.xpm"
#include "../pixmaps/loopback.xpm"
#include "../pixmaps/plip.xpm"
#include "../pixmaps/netspeed_applet.xpm"
#include "../pixmaps/disconnect.xpm"
#include "../pixmaps/wavelan.xpm"

#define ICON_IN_ARROW (const char**)in_arrow_xpm
#define ICON_OUT_ARROW (const char**)out_arrow_xpm
#define ICON_PPP (const char**)ppp_xpm
#define ICON_ETH (const char**)ethernet_xpm
#define ICON_LO (const char**)loopback_xpm
#define ICON_PLIP (const char**)plip_xpm
#define ICON_WLAN (const char**)wavelan_xpm
#define ICON_DISCONNECT (const char**)disconnect_xpm
#define ICON_APPLET (const char**)netspeed_applet_xpm

/* Some values for the pixbufs used
 */
#define COL_SAMPLES 4
#define HEIGHT 22
#define WIDTH 23
#define SUMWIDTH 16

#define SPACING 1

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
	*out_box, *out_label, *out_pix;
	
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
	
	guint64 in_old[OLD_VALUES], out_old[OLD_VALUES];
	double max_graph, in_graph[GRAPH_VALUES], out_graph[GRAPH_VALUES];
	int index_graph;
	
	GtkWidget *connect_dialog;
	
	GtkTooltips *tooltips;
} NetspeedApplet;
 
 #endif
