/*
 * GNOME odometer panel applet
 * (C) 1999 The Free software Foundation
 * 
 * Author : Fabrice Bellet <Fabrice.Bellet@creatis.insa-lyon.fr>
 *          adapted from kodo/Xodometer/Mouspedometa
 *
 */

#include "odo.h"

static GdkImlibImage *
get_image (gchar *path, gchar *string)
{
  GdkImlibImage *image = NULL;
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
  image = gdk_imlib_load_image (filename);
  g_free (filename);
  return image;
}

static gint
load_theme (gchar *path, OdoApplet *oa)
{
  gchar *datafile = g_strconcat (path, "/themedata", NULL);
  gchar *prefix;
  gint area_width, area_height;
  gint digits_nb, digits_nb_in_image;
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
  gnome_config_push_prefix (prefix);
  g_free (prefix);

  /*
   * Loading images
   */
  if (oa->image[INTEGER])
  	gdk_imlib_destroy_image (oa->image[INTEGER]);
  if (oa->image[DECIMAL])
  	gdk_imlib_destroy_image (oa->image[DECIMAL]);
  oa->image[INTEGER] = get_image (path, "integer_image=");
  oa->image[DECIMAL] = get_image (path, "decimal_image=");


  /*
   * compute the ratio for the RGB image
   */
  oa->with_decimal_dot = gnome_config_get_int ("with_decimal_dot=");

  digits_nb_in_image=10;
  digits_nb=oa->digits_nb;
  if (oa->with_decimal_dot)
  	digits_nb_in_image++;
  ratioXY [INTEGER]=((float)oa->image [INTEGER]->rgb_width/
  	(float)digits_nb_in_image)/(float)oa->image[INTEGER]->rgb_height;
  ratioXY [DECIMAL]=((float)oa->image [DECIMAL]->rgb_width/
  	(float)digits_nb_in_image)/(float)oa->image[DECIMAL]->rgb_height;

  /*
   * define the pixmap size from the panel size
   * and the ratio of the RGB image
   */
  if ((oa->orient == ORIENT_LEFT) || (oa->orient == ORIENT_RIGHT)) {
	if (oa->scale_applet) {
#ifdef HAVE_PANEL_PIXEL_SIZE
		oa->size   = applet_widget_get_panel_pixel_size
				(APPLET_WIDGET(oa->applet));
#else
		oa->size   = SIZEHINT_DEFAULT;
#endif
	} else 
		oa->size = (oa->image [INTEGER]->rgb_width * digits_nb)
				/digits_nb_in_image;
  	oa->width = oa->size;
  	area_width = oa->size - 0;
  	oa->digit_width = area_width / digits_nb;
  	oa->digit_height = oa->digit_width / ratioXY [INTEGER];
  	area_height = oa->digit_height;
  	oa->height = (oa->digit_height << 1) + 2;
  } else {
	if (oa->scale_applet) {
#ifdef HAVE_PANEL_PIXEL_SIZE
		oa->size   = applet_widget_get_panel_pixel_size
				(APPLET_WIDGET(oa->applet));
#else
		oa->size   = SIZEHINT_DEFAULT;
#endif
	} else
		oa->size = (oa->image [INTEGER]->rgb_height << 1) + 2;
	oa->height = oa->size;
	area_height = (oa->size - 2) >> 1;
	oa->digit_height = area_height;
	oa->digit_width = oa->digit_height * ratioXY [INTEGER];
	area_width = oa->digit_width * digits_nb;
	oa->width = area_width + 0;
  }

  gdk_imlib_render (oa->image [INTEGER], 
  	oa->digit_width * digits_nb_in_image,
  	oa->digit_height);
  gdk_imlib_render (oa->image [DECIMAL],
  	oa->digit_width * digits_nb_in_image,
  	oa->digit_height);

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
