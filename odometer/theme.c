/*
 * GNOME odometer panel applet
 * (C) 1999 The Free software Foundation
 * 
 * Author : Fabrice Bellet <Fabrice.Bellet@creatis.insa-lyon.fr>
 *          adapted from kodo/Xodometer/Mouspedometa
 *
 */

#include "odo.h"

static GdkPixbuf *
get_image (gchar *path, gchar *string)
{
  GdkPixbuf *image = NULL;
  gchar * buf = NULL;
  gchar *filename;

  buf = gnome_config_get_string (string);
  if (!buf) {
  	g_free (buf);
  	return NULL;
  }
  filename = g_strconcat (path, "/", buf, NULL);
  g_free (buf);
  if (!g_file_exists (filename)) {
  	g_free (filename);
  	return NULL;
  }

  image = gdk_pixbuf_new_from_file (filename, NULL);
  g_free (filename);
  return image;
}

static gint
load_theme (gchar *path, OdoApplet *oa)
{
  GdkPixbuf *tmp1, *tmp2;
  gchar *datafile = g_strconcat (path, "/themedata", NULL);
  gchar *prefix;
  gint area_width, area_height;
  gint digits_nb, digits_nb_in_image;
  gint rgb_width[2], rgb_height[2];
  gdouble ratioXY [2];

  if (oa->size == 0) {
  	g_assert_not_reached();
  	return FALSE;
  }
  if (!g_file_exists (datafile)) {
  	g_print ("Unable to open theme data file: %s\n", datafile);
  	g_free (datafile);
  	return FALSE;
  }
  prefix = g_strconcat ("=", datafile, "=/Default/", NULL);
  g_free (datafile);
/* FIXME this should be gconfized */
  gnome_config_push_prefix (prefix);
  g_free (prefix);

  /*
   * Loading images
   */

  if (oa->pixbuf[INTEGER])
  	gdk_pixbuf_unref (oa->pixbuf[INTEGER]);
  if (oa->pixbuf[DECIMAL])
  	gdk_pixbuf_unref (oa->pixbuf[DECIMAL]);

  tmp1 = get_image (path, "integer_image=");
  tmp2 = get_image (path, "decimal_image=");
  rgb_width[0] = gdk_pixbuf_get_width (tmp1);
  rgb_height[0] = gdk_pixbuf_get_height (tmp1);
  rgb_width[1] = gdk_pixbuf_get_width (tmp2);
  rgb_height[1] = gdk_pixbuf_get_height (tmp2);
  /*
   * compute the ratio for the RGB image
   */
  oa->with_decimal_dot = gnome_config_get_int ("with_decimal_dot=");

  digits_nb_in_image=10;
  digits_nb=oa->digits_nb;
  if (oa->with_decimal_dot)
  	digits_nb_in_image++;

  ratioXY [INTEGER]=((float)rgb_width[0]/
  	(float)digits_nb_in_image)/(float)rgb_height[0];
  ratioXY [DECIMAL]=((float)rgb_width[1]/
  	(float)digits_nb_in_image)/(float)rgb_height[1];
  	
  /*
   * define the pixmap size from the panel size
   * and the ratio of the RGB image
   */
  if ((oa->orient == PANEL_APPLET_ORIENT_LEFT) || (oa->orient == PANEL_APPLET_ORIENT_RIGHT)) {
	if (oa->scale_applet) {
		oa->size   = panel_applet_get_size (PANEL_APPLET (oa->applet));

	} else 
		oa->size = (rgb_width[0] * digits_nb)
				/digits_nb_in_image;
	
  	oa->width = oa->size;
  	area_width = oa->size - 0;
  	oa->digit_width = area_width / digits_nb;
  	oa->digit_height = oa->digit_width / ratioXY [INTEGER];
  	area_height = oa->digit_height;
  	oa->height = (oa->digit_height << 1) + 2;
  } else {
	if (oa->scale_applet) {

		oa->size   = panel_applet_get_size (PANEL_APPLET (oa->applet));

	} else
		oa->size = (rgb_height[0] << 1) + 2;
	oa->height = oa->size;
	area_height = (oa->size - 2) >> 1;
	oa->digit_height = area_height;
	oa->digit_width = oa->digit_height * ratioXY [INTEGER];
	area_width = oa->digit_width * digits_nb;
	oa->width = area_width + 0;
  }
  
  oa->pixbuf[INTEGER] = gdk_pixbuf_scale_simple (tmp1, oa->digit_width * digits_nb_in_image,
  						 oa->digit_height, GDK_INTERP_HYPER);
  oa->pixbuf[DECIMAL] = gdk_pixbuf_scale_simple (tmp2, oa->digit_width * digits_nb_in_image,
  						 oa->digit_height, GDK_INTERP_HYPER);
  gdk_pixbuf_unref (tmp1);
  gdk_pixbuf_unref (tmp2);

  /*
   * Adjust widget sizes
   */

  gtk_drawing_area_size (GTK_DRAWING_AREA(oa->darea1),area_width,area_height);
  gtk_drawing_area_size (GTK_DRAWING_AREA(oa->darea2),area_width,area_height);
  gtk_widget_set_usize (GTK_WIDGET(oa->applet),oa->width,oa->height);

  return TRUE;
}

gint
change_theme (gchar *path, OdoApplet *oa)
{
  gchar *themefile;
  gint retval = TRUE;

  if (path && *path && load_theme (path, oa))
	  return TRUE;

  themefile = gnome_datadir_file ("odometer/default");
  retval = load_theme (themefile, oa);
  g_free (themefile);

  return retval;
}
