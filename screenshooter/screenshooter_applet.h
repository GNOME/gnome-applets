/* screenshooter_applet.c
 *
 * Copyright (C) 1999 Tom Gilbert
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include <config.h>

#include <gnome.h>
#include <applet-widget.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <wordexp.h>
#include <sys/wait.h>
#include <unistd.h>
#include "window_icon.xpm"
#include "root_icon.xpm"

typedef struct
{
  /* The property window widgets */
  GtkWidget *propwindow;
  GtkWidget *quality_entry;
  GtkWidget *directory_entry;
  GtkWidget *filename_entry;
  GtkWidget *delay_entry;
  GtkWidget *app_entry;
  GtkWidget *thumb_filename_entry;
  GtkWidget *script_entry;
  GtkWidget *spurious_pref_vbox;
  GtkWidget *spurious_pref_vbox2;
  GtkWidget *spurious_pref_vbox3;

  int quality;
  int thumb_quality;
  int thumb_size;
  int delay;
  int beep;
  int frame_size;
  int rotate_degrees;
  int blur_factor;
  int charcoal_factor;
  int edge_factor;
  int implode_factor;
  int paint_radius;
  int sharpen_factor;
  int solarize_factor;
  int spread_radius;
  int swirl_degrees;
  float gamma_factor;
  gboolean monochrome;
  gboolean negate;
  gboolean decoration;
  gboolean view;
  gboolean post_process;
  gboolean thumb;
  gboolean thumbnail_intermediate;
  gboolean frame;
  gboolean equalize;
  gboolean normalize;
  gboolean gamma;
  gboolean flip;
  gboolean flop;
  gboolean emboss;
  gboolean enhance;
  gboolean despeckle;
  gboolean use_script;
  gboolean blur;
  gboolean charcoal;
  gboolean edge;
  gboolean implode;
  gboolean paint;
  gboolean solarize;
  gboolean spread;
  gboolean swirl;
  gboolean spurious;
  gchar *directory;
  gchar *filename;
  gchar *app;
  gchar *thumb_filename;
  gchar *script_filename;
}
user_preferences;

void cb_about (AppletWidget * widget, gpointer data);
void cb_properties_dialog (AppletWidget * widget, gpointer data);
void property_apply_cb (GtkWidget * w, gpointer data);
gint property_destroy_cb (GtkWidget * w, gpointer data);
void expand_cb (GtkWidget * w, gpointer data);
void spurious_cb (GtkWidget * w, gpointer data);
void cb_applet_change_orient (GtkWidget * w, PanelOrientType o,
			      gpointer data);
void window_button_press (GtkWidget * button, user_preferences * options);
void desktop_button_press (GtkWidget * button, user_preferences * options);
static void properties_save (gchar * path, gpointer data);
void grab_shot (user_preferences * opt, gboolean root);
void change_orientation (PanelOrientType o, gboolean size_is_tiny);
gboolean need_to_change_orientation (PanelOrientType o,
				     gboolean size_is_tiny);
void set_tooltip (GtkWidget * w, const gchar * tip);
#ifdef HAVE_PANEL_PIXEL_SIZE
static void applet_change_pixel_size (GtkWidget * w, int s, gpointer data);
#endif
void slider_option_cb (GtkWidget * w, gpointer data);
void boolean_option_cb (GtkWidget * w, gpointer data);
GtkWidget *create_bool_option (const gchar * label, int *opt,
			       GtkWidget * target);
GtkWidget *
create_slider_option (gchar * label, GtkWidget * target, int *option, gfloat a,
		     gfloat b, gfloat c, gfloat d, gfloat e);

/* Global variables */
GtkWidget *applet;
GtkWidget *hframe, *vframe;
gchar *lastimage = NULL;
user_preferences options;
user_preferences old_options;
