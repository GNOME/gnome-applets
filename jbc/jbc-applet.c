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

#define APPLET_VERSION_MAJ 0
#define APPLET_VERSION_MIN 0
#define APPLET_VERSION_REV 9

static gint
about_jbc ()
{
  GtkWidget *about;
  const gchar *authors[2];
  gchar version[32];

  g_snprintf (version, sizeof (version), "%d.%d.%d",
	      APPLET_VERSION_MAJ,
	      APPLET_VERSION_MIN,
	      APPLET_VERSION_REV);

  authors[0] = "Jon Anhold <jon@snoopy.net>";
  authors[1] = NULL;

  about = gnome_about_new (_ ("Jon's Binary Clock"), version,
			   "(C) 1999",
			   authors,
		       _ ("Released under the GNU general public license.\n"
			  "Displays time in Binary Coded Decimal\nhttp://snoopy.net/~jon/jbc/"
			  "."),
			   NULL);
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


int
main (int argc, char **argv)
{

  GtkWidget *applet;
  GtkWidget *canvas;

  int ytmp;
  float xtmp;

  applet_widget_init ("jbc_applet", "1.0", argc, argv, NULL, 0, NULL);


  applet = applet_widget_new ("jbc_applet");
  if (!applet)
    g_error ("Can't create applet!\n");

  canvas = gnome_canvas_new ();
  gtk_widget_set_usize (canvas, CANVAS_WIDTH, CANVAS_HEIGHT);
  gnome_canvas_set_scroll_region (GNOME_CANVAS (canvas), 0.0, 0.0, CANVAS_WIDTH, CANVAS_HEIGHT);

  applet_widget_add (APPLET_WIDGET (applet), canvas);
  gtk_widget_show (canvas);

  gtk_widget_show (applet);


  applet_widget_register_stock_callback (APPLET_WIDGET (applet),
					 "about",
					 GNOME_STOCK_MENU_ABOUT,
					 _ ("About..."),
					 (AppletCallbackFunc)about_jbc, NULL);


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
