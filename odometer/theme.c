/*
 * GNOME odometer panel applet
 * (C) 1999 The Free software Foundation
 * 
 * Author : Fabrice Bellet <Fabrice.Bellet@creatis.insa-lyon.fr>
 *          adapted from kodo/Xodometer/Mouspedometa
 *
 */

#include "odo.h"

#include "def_theme/Default-black.xpm"
#include "def_theme/Default-red.xpm"

static gint
load_default_theme (OdoApplet *oa)
{
  GdkBitmap *mask = NULL;
  GtkStyle *style;
  gint area_width, area_height;

  if (oa->int_pixmap) gdk_pixmap_unref (oa->int_pixmap);
  if (oa->dec_pixmap) gdk_pixmap_unref (oa->dec_pixmap);
  /*
   * build the first pixmap
   */
  style=gtk_widget_get_style(oa->applet);
  oa->int_pixmap = gdk_pixmap_create_from_xpm_d(oa->darea1->window, &mask,
	&style->bg[GTK_STATE_NORMAL], (gchar **)Default_black_xpm);
  gdk_window_get_size (oa->int_pixmap, &oa->width, &oa->height);
  oa->digit_width=oa->width/10;
  oa->digit_height=oa->height;
  oa->with_decimal_dot=0;

  /*
   * build the pixmaps for the decimal digits
   */
  oa->dec_pixmap = gdk_pixmap_create_from_xpm_d(oa->darea1->window, &mask,
	&style->bg[GTK_STATE_NORMAL], (gchar **)Default_red_xpm);

  /*
   * Here, we assumes that both default pixmaps have the same size.
   */
  area_width=oa->digit_width*oa->digits_nb;
  area_height=oa->digit_height;
  gtk_drawing_area_size(GTK_DRAWING_AREA(oa->darea1),area_width,area_height);
  gtk_drawing_area_size(GTK_DRAWING_AREA(oa->darea2),area_width,area_height);
  return TRUE;
}

static GdkPixmap *
get_pixmap (gchar *path, gchar *string)
{
  GtkWidget *pixmap = NULL;
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
  pixmap = gnome_pixmap_new_from_file (filename);
  g_free (filename);
  return GNOME_PIXMAP(pixmap)->pixmap;
}

static gint
load_theme (gchar *path, OdoApplet *oa)
{
  gchar *datafile = g_strconcat (path, "/themedata", NULL);
  gchar *prefix;
  gint area_width, area_height;
  gint nb_digits_in_pixmap;
  gint decimal_pixmap_width, decimal_pixmap_height;

  if (!g_file_exists (datafile)) {
  	g_print ("Unable to open theme data file: %s\n", datafile);
  	g_free (datafile);
  	return FALSE;
  }
  prefix = g_strconcat ("=", datafile, "=/Default/", NULL);
  gnome_config_push_prefix (prefix);
  g_free (prefix);

  oa->with_decimal_dot = gnome_config_get_int ("with_decimal_dot=");
  oa->digit_width = gnome_config_get_int ("digit_width=");

  /*
   * Loading pixmap for the integer part of the counter
   */
  if (oa->int_pixmap)	gdk_pixmap_unref (oa->int_pixmap);
  oa->int_pixmap = get_pixmap (path, "integer_pixmap=");
  if (oa->int_pixmap)
  	gdk_window_get_size (oa->int_pixmap, &oa->width, &oa->height);
  else {
  	g_free (datafile);
  	gnome_config_pop_prefix();
  	return FALSE;
  }
  /*
   * Checking size consistencies
   */
  if (oa->with_decimal_dot)
  	nb_digits_in_pixmap = 11;
  else
  	nb_digits_in_pixmap = 10;
  if (nb_digits_in_pixmap * oa->digit_width != oa->width) {
  	gdk_pixmap_unref (oa->int_pixmap);
  	g_free (datafile);
  	gnome_config_pop_prefix();
  	return FALSE;
  }

  /*
   * Loading pixmap for the decimal part of the counter
   */
  if (oa->dec_pixmap)	gdk_pixmap_unref (oa->dec_pixmap);
  oa->dec_pixmap = get_pixmap (path, "decimal_pixmap=");
  if (oa->dec_pixmap)
  	gdk_window_get_size (oa->int_pixmap,
  		&decimal_pixmap_width,
  		&decimal_pixmap_height);
  else {
  	gdk_pixmap_unref (oa->int_pixmap);
  	g_free (datafile);
  	gnome_config_pop_prefix();
  	return FALSE;
  }
  /*
   * Checking size consistencies
   */
  if (oa->width != decimal_pixmap_width
  	|| oa->height != decimal_pixmap_height) {
  	gdk_pixmap_unref (oa->dec_pixmap);
  	gdk_pixmap_unref (oa->int_pixmap);
  	g_free (datafile);
  	gnome_config_pop_prefix();
  	return FALSE;

  }

  /*
   * Finishing setup
   */
  oa->digit_height = oa->height;
  area_width = oa->digit_width * oa->digits_nb;
  area_height = oa->height;
  gtk_drawing_area_size (GTK_DRAWING_AREA(oa->darea1),area_width,area_height);
  gtk_drawing_area_size (GTK_DRAWING_AREA(oa->darea2),area_width,area_height);
  return TRUE;
}

gint
change_theme (gchar *path, OdoApplet *oa)
{
  if (!path || !*path)
  	return load_default_theme (oa);
  else
  	if (!load_theme (path, oa))
  		return load_default_theme (oa);
	else
		return TRUE;
}
