/* $Id$
 *
 * Jon's Binary Clock, GNOMEified
 *
 * Jon Anhold <jon@snoopy.net>
 *
 * Copyright(c) 1999 Jon Anhold, All Rights Reserved.
 *
 * This program is licensed under the GNU General Public License
 * 
 * See COPYING for license text.
 *
 */

#include <config.h>
#include <gnome.h>
#include <time.h>
#include <unistd.h>
#include <applet-widget.h>
#include "jbc-applet.h"

#define CANVAS_WIDTH 82
#define CANVAS_HEIGHT  50


GnomeCanvasItem *item[16];
GdkImlibImage *pix[16], *tpix[2];
struct tm atime;
time_t thetime;
int i;
int d[8];
static int digits[8] = {0, 0, 0, 0, 0, 0};
int blink = 0;

static int panel_size = 48;
static gboolean panel_vertical = FALSE;

static gint
about_jbc ()
{
  static GtkWidget *about = NULL;
  const gchar *authors[2];
  /* gchar version[32]; */

  if (about != NULL)
  {
  	gdk_window_show(about->window);
	gdk_window_raise(about->window);
	return TRUE;
  }
  authors[0] = "Jon Anhold <jon@snoopy.net>";
  authors[1] = NULL;

  about = gnome_about_new (_("Jon's Binary Clock"), 
			   VERSION,
			   _("(C) 1999"),
			   authors,
		        _("Released under the GNU general public license.\n"
			  "Displays time in Binary Coded Decimal\n"
			  "http://snoopy.net/~jon/jbc/."),
			   NULL);
  gtk_signal_connect( GTK_OBJECT(about), "destroy",
		      GTK_SIGNAL_FUNC(gtk_widget_destroyed), &about );
  gtk_widget_show (about);

  return TRUE;
}


static gint
do_flicker ()
{
  thetime = time (NULL);
  localtime_r (&thetime, &atime);

  d[0] = atime.tm_hour / 10;
  d[1] = atime.tm_hour % 10;
  d[3] = atime.tm_min / 10;
  d[4] = atime.tm_min % 10;
  d[6] = atime.tm_sec / 10;
  d[7] = atime.tm_sec % 10;

  if (d[7] != digits[7])
    {
      if (blink == 0)
	{
	  gnome_canvas_item_set (item[2], "image", tpix[0], NULL);
	  gnome_canvas_item_set (item[5], "image", tpix[0], NULL);
	  blink = 1;
	}
      else
	{
	  gnome_canvas_item_set (item[2], "image", tpix[1], NULL);
	  gnome_canvas_item_set (item[5], "image", tpix[1], NULL);
	  blink = 0;
	}
    }
  for (i = 0; i < 8; ++i)
    {
      if (d[i] != digits[i])
	{
	  if (i != 2 || i != 5)
	    {
	      gnome_canvas_item_set (item[i], "image", pix[d[i]], NULL);
	      digits[i] = d[i];
	    }

	}
    }
  return 1;
}

static void
redo_size (GtkWidget *canvas)
{
	int w, h;
	double scale_factor;

	if (panel_vertical) {
		w = panel_size + 2;
		h = (CANVAS_HEIGHT*w)/CANVAS_WIDTH;
		scale_factor = (double)w / (double)CANVAS_WIDTH;
	} else {
		h = panel_size + 2;
		w = (CANVAS_WIDTH*h)/CANVAS_HEIGHT;
		scale_factor = (double)h / (double)CANVAS_HEIGHT;
	}
	gtk_widget_set_usize (canvas, w, h);
	gnome_canvas_set_pixels_per_unit (GNOME_CANVAS (canvas), scale_factor);
	gnome_canvas_scroll_to (GNOME_CANVAS (canvas), 0.0, 0.0);
}

static void
change_pixel_size (GtkWidget *applet, int size, GtkWidget *canvas)
{
	panel_size = size;
	redo_size (canvas);
}

static void
change_orient (GtkWidget *applet, PanelOrientType o, GtkWidget *canvas)
{
	if(o == ORIENT_UP ||
	   o == ORIENT_DOWN)
		panel_vertical = FALSE;
	else
		panel_vertical = TRUE;
	redo_size (canvas);
}

static void
help_cb (AppletWidget *applet, gpointer data)
{
    GnomeHelpMenuEntry help_entry = { "jbc_applet", "index.html"};
    gnome_help_display(NULL, &help_entry);
}


int
main (int argc, char **argv)
{

  GtkWidget *applet;
  GtkWidget *canvas;

  int ytmp;
  float xtmp = 0.0;

  bindtextdomain (PACKAGE, GNOMELOCALEDIR);
  textdomain (PACKAGE);

  applet_widget_init ("jbc_applet", VERSION, argc, argv, NULL, 0, NULL);


  applet = applet_widget_new ("jbc_applet");
  if (!applet)
    g_error (_("Can't create applet!\n"));

  canvas = gnome_canvas_new ();
  gtk_widget_set_usize (canvas, CANVAS_WIDTH, CANVAS_HEIGHT);
  gnome_canvas_set_scroll_region (GNOME_CANVAS (canvas), 0.0, 0.0, CANVAS_WIDTH, CANVAS_HEIGHT);

  gtk_signal_connect (GTK_OBJECT (applet), "change_orient",
		      GTK_SIGNAL_FUNC (change_orient), canvas);
  gtk_signal_connect (GTK_OBJECT (applet), "change_pixel_size",
		      GTK_SIGNAL_FUNC (change_pixel_size), canvas);

  applet_widget_add (APPLET_WIDGET (applet), canvas);
  gtk_widget_show (canvas);

  gtk_widget_show (applet);

  applet_widget_register_stock_callback (APPLET_WIDGET (applet),
					 "help",
					 GNOME_STOCK_PIXMAP_HELP,
					 _("Help"), help_cb, NULL);

  applet_widget_register_stock_callback (APPLET_WIDGET (applet),
					 "about",
					 GNOME_STOCK_MENU_ABOUT,
					 _("About..."),
					 (AppletCallbackFunc)about_jbc, NULL);

  /* black background */
  gnome_canvas_item_new (gnome_canvas_root (GNOME_CANVAS (canvas)),
			 gnome_canvas_rect_get_type (),
			 "x1", (double) 0.0,
			 "y1", (double) 0.0,
			 "x2", (double) CANVAS_WIDTH,
			 "y2", (double) CANVAS_HEIGHT,
			 "fill_color", "black",
			 NULL);

  tpix[0] = gdk_imlib_create_image_from_xpm_data (tcolon_xpm);
  tpix[1] = gdk_imlib_create_image_from_xpm_data (tbcolon_xpm);

  pix[0] = gdk_imlib_create_image_from_xpm_data (t0_xpm);
  pix[1] = gdk_imlib_create_image_from_xpm_data (t1_xpm);
  pix[2] = gdk_imlib_create_image_from_xpm_data (t2_xpm);
  pix[3] = gdk_imlib_create_image_from_xpm_data (t3_xpm);
  pix[4] = gdk_imlib_create_image_from_xpm_data (t4_xpm);
  pix[5] = gdk_imlib_create_image_from_xpm_data (t5_xpm);
  pix[6] = gdk_imlib_create_image_from_xpm_data (t6_xpm);
  pix[7] = gdk_imlib_create_image_from_xpm_data (t7_xpm);
  pix[8] = gdk_imlib_create_image_from_xpm_data (t8_xpm);
  pix[9] = gdk_imlib_create_image_from_xpm_data (t9_xpm);


  ytmp = 25;

  for (i = 0; i < 8; ++i)
    {
      if (i == 0)
	{
	  xtmp = 6;
	}
      else if (i == 2 || i == 3 || i == 5 || i == 6)
	{
	  xtmp = xtmp + 8.5;
	}
      else
	{
	  xtmp = xtmp + 12;
	}
      if (i == 2 || i == 5)
	{
	  item[i] = gnome_canvas_item_new (gnome_canvas_root (GNOME_CANVAS (canvas)),
					   gnome_canvas_image_get_type (),
					   "image", tpix[0],
					   "x", (double) xtmp,
					   "y", (double) ytmp,
					   "width", (double) 5,
					   "height", (double) 50,
					   NULL);
	}
      else
	{
	  item[i] = gnome_canvas_item_new (gnome_canvas_root (GNOME_CANVAS (canvas)),
					   gnome_canvas_image_get_type (),
					   "image", pix[0],
					   "x", (double) xtmp,
					   "y", (double) ytmp,
					   "width", (double) 12,
					   "height", (double) 50,
					   NULL);
	}
    }

  gtk_timeout_add (250, do_flicker, NULL);
  applet_widget_gtk_main ();
  return 0;
}
