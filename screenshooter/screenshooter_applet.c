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


/* If you think this code is a mess, you should see my flat ;) */

#include "config.h"
#include "screenshooter_applet.h"

/* TODO For the application launches...
 * Use an array of pointers to strings, and execv, or popen instead
 * of all these system() calls. At least with popen I could get some
 * feedback from the app, and then be able to report errors... */

void
cb_about (AppletWidget * widget, gpointer data)
{
  GtkWidget *about;
  GtkWidget *my_url;
  static const char *authors[] =
    { "Tom Gilbert <gilbertt@tomgilbert.freeserve.co.uk>",
	"Telsa Gwynne <hobbit@aloss.ukuu.org.uk> (typo fixes, documentation)",
    "The Sirius Cybernetics Corporation <Ursa-minor>", NULL
  };

  about = gnome_about_new
    (_ ("Screen-Shooter Applet"),
     VERSION,
     _ ("Copyright (C) 1999 Tom Gilbert"),
     authors, _ ("A useful little screengrabber.\n"
		 "The left/top button allows you to grab either:\n"
		 "a window (click to select which one)\n"
		 "or an area (click and drag to select a rectangle.)\n"
		 "The right/bottom button will grab the entire desktop.\n"
		 "Share and Enjoy ;)"), NULL);

  my_url = gnome_href_new ("http://www.tomgilbert.freeserve.co.uk",
			   "Visit the author's Website");
  gtk_widget_show (my_url);
  gtk_box_pack_start (GTK_BOX ((GNOME_DIALOG(about))->vbox), my_url, FALSE, FALSE, 0);

  gtk_widget_show (about);
  data = NULL;
  widget = NULL;
}

static void
properties_save (gchar * path, gpointer data)
{
  gnome_config_push_prefix (path);
  gnome_config_set_int ("screenshooter/quality", options.quality);
  gnome_config_set_int ("screenshooter/thumb_quality", options.thumb_quality);
  gnome_config_set_int ("screenshooter/delay", options.delay);
  gnome_config_set_int ("screenshooter/thumb_size", options.thumb_size);
  gnome_config_set_int ("screenshooter/frame_size", options.frame_size);
  gnome_config_set_int ("screenshooter/rotate_degrees",
			options.rotate_degrees);
  gnome_config_set_int ("screenshooter/blur_factor", options.blur_factor);
  gnome_config_set_int ("screenshooter/charcoal_factor",
			options.charcoal_factor);
  gnome_config_set_int ("screenshooter/edge_factor", options.edge_factor);
  gnome_config_set_int ("screenshooter/implode_factor",
			options.implode_factor);
  gnome_config_set_int ("screenshooter/paint_radius", options.paint_radius);
  gnome_config_set_int ("screenshooter/sharpen_factor",
			options.sharpen_factor);
  gnome_config_set_int ("screenshooter/solarize_factor",
			options.solarize_factor);
  gnome_config_set_int ("screenshooter/spread_radius", options.spread_radius);
  gnome_config_set_int ("screenshooter/swirl_degrees", options.swirl_degrees);
  gnome_config_set_float ("screenshooter/gamma_factor", options.gamma_factor);
  gnome_config_set_string ("screenshooter/directory", options.directory);
  gnome_config_set_string ("screenshooter/filename", options.filename);
  gnome_config_set_string ("screenshooter/script_filename",
			   options.script_filename);
  gnome_config_set_string ("screenshooter/thumb_filename",
			   options.thumb_filename);
  gnome_config_set_bool ("screenshoter/decoration", options.decoration);
  gnome_config_set_bool ("screenshoter/monochrome", options.monochrome);
  gnome_config_set_bool ("screenshoter/negate", options.negate);
  gnome_config_set_bool ("screenshoter/view", options.view);
  gnome_config_set_bool ("screenshoter/equalize", options.equalize);
  gnome_config_set_bool ("screenshoter/normalize", options.normalize);
  gnome_config_set_bool ("screenshoter/beep", options.beep);
  gnome_config_set_bool ("screenshoter/frame", options.frame);
  gnome_config_set_bool ("screenshoter/thumb", options.thumb);
  gnome_config_set_bool ("screenshoter/flip", options.flip);
  gnome_config_set_bool ("screenshoter/flop", options.flop);
  gnome_config_set_bool ("screenshoter/despeckle", options.despeckle);
  gnome_config_set_bool ("screenshoter/enhance", options.enhance);
  gnome_config_set_bool ("screenshoter/use_script", options.use_script);
  gnome_config_set_bool ("screenshoter/emboss", options.emboss);
  gnome_config_set_bool ("screenshoter/blur", options.blur);
  gnome_config_set_bool ("screenshoter/charcoal", options.charcoal);
  gnome_config_set_bool ("screenshoter/edge", options.edge);
  gnome_config_set_bool ("screenshoter/implode", options.implode);
  gnome_config_set_bool ("screenshoter/paint", options.paint);
  gnome_config_set_bool ("screenshoter/spurious", options.spurious);
  gnome_config_set_bool ("screenshoter/solarize", options.solarize);
  gnome_config_set_bool ("screenshoter/spread", options.spread);
  gnome_config_set_bool ("screenshoter/swirl", options.swirl);
  gnome_config_set_bool ("screenshoter/thumbnail_intermediate",
			 options.thumbnail_intermediate);
  gnome_config_pop_prefix ();
  gnome_config_sync ();
  gnome_config_drop_all ();
  return;
  data = NULL;
}

static void
properties_load (gchar * path, gpointer data)
{
  gnome_config_push_prefix (path);
  options.propwindow = NULL;
  options.quality_entry = NULL;
  options.directory_entry = NULL;
  options.filename_entry = NULL;
  options.delay_entry = NULL;
  options.app_entry = NULL;
  options.script_entry = NULL;
  options.thumb_filename_entry = NULL;
  options.spurious_pref_vbox = NULL;
  options.spurious_pref_vbox2 = NULL;
  options.spurious_pref_vbox3 = NULL;

  options.quality = gnome_config_get_int ("screenshooter/quality=75");
  options.rotate_degrees =
    gnome_config_get_int ("screenshooter/rotate_degrees=0");
  options.blur_factor = gnome_config_get_int ("screenshooter/blur_factor=0");
  options.charcoal_factor =
    gnome_config_get_int ("screenshooter/charcoal_factor=0");
  options.edge_factor = gnome_config_get_int ("screenshooter/edge_factor=0");
  options.implode_factor =
    gnome_config_get_int ("screenshooter/implode_factor=0");
  options.paint_radius =
    gnome_config_get_int ("screenshooter/paint_radius=0");
  options.sharpen_factor =
    gnome_config_get_int ("screenshooter/sharpen_factor=0");
  options.solarize_factor =
    gnome_config_get_int ("screenshooter/solarize_factor=0");
  options.spread_radius =
    gnome_config_get_int ("screenshooter/spread_radius=0");
  options.swirl_degrees =
    gnome_config_get_int ("screenshooter/swirl_degrees=0");
  options.frame_size = gnome_config_get_int ("screenshooter/frame_size=6");
  options.thumb_quality =
    gnome_config_get_int ("screenshooter/thumb_quality=50");
  options.thumb_size = gnome_config_get_int ("screenshooter/thumb_size=25");
  options.gamma_factor =
    gnome_config_get_float ("screenshooter/gamma_factor=1.6");
  options.delay = gnome_config_get_int ("screenshooter/delay=0");
  options.directory =
    g_strdup (gnome_config_get_string ("screenshooter/directory=~/"));
  options.filename =
    g_strdup (gnome_config_get_string
	      ("screenshooter/filename=`date +%d-%m-%y_%H%M%S`_screenshot.jpg"));
  options.thumb_filename =
    g_strdup (gnome_config_get_string
	      ("screenshooter/thumb_filename=thumb-"));
  options.script_filename =
    g_strdup (gnome_config_get_string
	      ("screenshooter/script_filename=update_scrshot_page"));
  options.app =
    g_strdup (gnome_config_get_string ("screenshooter/app=display"));
  options.decoration =
    gnome_config_get_bool ("screenshooter/decoration=TRUE");
  options.monochrome =
    gnome_config_get_bool ("screenshooter/monochrome=FALSE");
  options.negate = gnome_config_get_bool ("screenshooter/negate=FALSE");
  options.view = gnome_config_get_bool ("screenshooter/view=FALSE");
  options.beep = gnome_config_get_bool ("screenshooter/beep=TRUE");
  options.thumb = gnome_config_get_bool ("screenshooter/thumb=FALSE");
  options.frame = gnome_config_get_bool ("screenshooter/frame=FALSE");
  options.equalize = gnome_config_get_bool ("screenshooter/equalize=FALSE");
  options.normalize = gnome_config_get_bool ("screenshooter/normalize=FALSE");
  options.flip = gnome_config_get_bool ("screenshooter/flip=FALSE");
  options.flop = gnome_config_get_bool ("screenshooter/flop=FALSE");
  options.enhance = gnome_config_get_bool ("screenshooter/enhance=FALSE");
  options.emboss = gnome_config_get_bool ("screenshooter/emboss=FALSE");
  options.blur = gnome_config_get_bool ("screenshooter/blur=FALSE");
  options.spurious = gnome_config_get_bool ("screenshooter/spurious=FALSE");
  options.charcoal = gnome_config_get_bool ("screenshooter/charcoal=FALSE");
  options.edge = gnome_config_get_bool ("screenshooter/edge=FALSE");
  options.implode = gnome_config_get_bool ("screenshooter/implode=FALSE");
  options.paint = gnome_config_get_bool ("screenshooter/paint=FALSE");
  options.solarize = gnome_config_get_bool ("screenshooter/solarize=FALSE");
  options.spread = gnome_config_get_bool ("screenshooter/spread=FALSE");
  options.swirl = gnome_config_get_bool ("screenshooter/swirl=FALSE");
  options.despeckle = gnome_config_get_bool ("screenshooter/despeckle=FALSE");
  options.use_script =
    gnome_config_get_bool ("screenshooter/use_script=FALSE");
  options.thumbnail_intermediate =
    gnome_config_get_bool ("screenshooter/thumbnail_intermediate=FALSE");
  gnome_config_pop_prefix ();
  return;
  data = NULL;
}

void
set_tooltip (GtkWidget * w, const gchar * tip)
{
  GtkTooltips *t = gtk_tooltips_new ();
  gtk_tooltips_set_tip (t, w, tip, NULL);
}

gboolean
need_to_change_orientation (PanelOrientType o, gboolean size_tiny)
{
  gboolean need_to = FALSE;
  switch (o)
    {
    case ORIENT_UP:
    case ORIENT_DOWN:
      {
	if ((size_tiny)
	    && ((GTK_WIDGET_VISIBLE (vframe))
		|| (!GTK_WIDGET_VISIBLE (hframe))))
	  need_to = TRUE;
	else if ((!size_tiny)
		 && ((GTK_WIDGET_VISIBLE (hframe))
		     || (!GTK_WIDGET_VISIBLE (vframe))))
	  need_to = TRUE;
	break;
      }
    case ORIENT_LEFT:
    case ORIENT_RIGHT:
      {
	if ((size_tiny)
	    && ((GTK_WIDGET_VISIBLE (hframe))
		|| (!GTK_WIDGET_VISIBLE (vframe))))
	  need_to = TRUE;
	else if ((!size_tiny)
		 && ((GTK_WIDGET_VISIBLE (vframe))
		     || (!GTK_WIDGET_VISIBLE (hframe))))
	  need_to = TRUE;
	break;
      }
    default:
      {
	g_assert_not_reached ();
	break;
      }
    }
  return need_to;
}

void
change_orientation (PanelOrientType o, gboolean panel_size_tiny)
{
  if (need_to_change_orientation (o, panel_size_tiny))
    {
      gtk_widget_hide (applet);
      switch (o)
	{
	case ORIENT_UP:
	case ORIENT_DOWN:
	  {
	    if (panel_size_tiny)
	      {
		gtk_widget_hide (vframe);
		gtk_widget_show (hframe);
	      }
	    else
	      {
		gtk_widget_hide (hframe);
		gtk_widget_show (vframe);
	      }
	    break;
	  }
	case ORIENT_LEFT:
	case ORIENT_RIGHT:
	  {
	    if (panel_size_tiny)
	      {
		gtk_widget_hide (hframe);
		gtk_widget_show (vframe);
	      }
	    else
	      {
		gtk_widget_hide (vframe);
		gtk_widget_show (hframe);
	      }
	    break;
	  }
	default:
	  {
	    g_assert_not_reached ();
	    break;
	  }
	}
      gtk_widget_show (applet);
    }
}

void
cb_applet_change_orient (GtkWidget * w, PanelOrientType o, gpointer data)
{
  gboolean size_tiny = FALSE;
#ifdef HAVE_PANEL_PIXEL_SIZE
  int panelsize =
    applet_widget_get_panel_pixel_size (APPLET_WIDGET (applet));
  /* Switch stuff so it lies flat on a tiny panel */
  if (panelsize < PIXEL_SIZE_STANDARD)
    size_tiny = TRUE;
  else
    size_tiny = FALSE;
#endif

  change_orientation (o, size_tiny);

  w = NULL;
  data = NULL;
}

#ifdef HAVE_PANEL_PIXEL_SIZE
static void
applet_change_pixel_size (GtkWidget * w, int s, gpointer data)
{
  gboolean size_tiny = FALSE;
  PanelOrientType orient =
    applet_widget_get_panel_orient (APPLET_WIDGET (applet));

  /* Switch stuff so it lies flat on a tiny panel */
  if (s < PIXEL_SIZE_STANDARD)
    size_tiny = TRUE;
  else
    size_tiny = FALSE;

  change_orientation (orient, size_tiny);

  w = NULL;
  data = NULL;
}
#endif

void
cb_properties_dialog (AppletWidget * widget, gpointer data)
{

  user_preferences *ad = &options;
  GtkWidget *frame;
  GtkWidget *pref_vbox;
  GtkWidget *pref_vbox_2;
  GtkWidget *pref_hbox;
/*  GtkWidget *pref_hbox5; */
  GtkWidget *label;
  GtkWidget *button;
/*  GtkWidget *directory_button; */
  GtkObject *adj;
  GtkWidget *hscale;
  GtkWidget *entry;

  if (ad->propwindow)
    {
      gdk_window_raise (ad->propwindow->window);
      return;
    }

  /* Grab old property settings for comparison */
  memcpy (&old_options, &options, sizeof (user_preferences));

  ad->propwindow = gnome_property_box_new ();
  gtk_window_set_title (GTK_WINDOW
			(&GNOME_PROPERTY_BOX (ad->propwindow)->dialog.window),
			_ ("Screen-Shooter Settings"));

  pref_vbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
  gtk_container_set_border_width (GTK_CONTAINER (pref_vbox), GNOME_PAD_SMALL);

  button =
    gtk_check_button_new_with_label (_
				     ("Capture WM decorations when grabbing a window"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), ad->decoration);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      (GtkSignalFunc) decoration_cb, NULL);
  gtk_box_pack_start (GTK_BOX (pref_vbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  button =
    gtk_check_button_new_with_label (_
				     ("Give audio feedback using the keyboard bell"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), ad->beep);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      (GtkSignalFunc) beep_cb, NULL);
  gtk_box_pack_start (GTK_BOX (pref_vbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  button =
    gtk_check_button_new_with_label (_
				     ("Display Spurious Options (I got carried away)"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), ad->spurious);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      (GtkSignalFunc) spurious_cb, NULL);
  gtk_box_pack_start (GTK_BOX (pref_vbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  frame =
    gtk_frame_new (_
		   ("Delay (seconds) before taking shot (only for entire desktop shots)"));
  gtk_box_pack_start (GTK_BOX (pref_vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  pref_vbox_2 = gtk_hbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (pref_vbox_2),
				  GNOME_PAD_SMALL);
  gtk_container_add (GTK_CONTAINER (frame), pref_vbox_2);
  gtk_widget_show (pref_vbox_2);

  adj = gtk_adjustment_new (ad->delay, 0.0, 60.0, 1.0, 1.0, 0.0);
  hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_range_set_update_policy (GTK_RANGE (hscale), GTK_UPDATE_DELAYED);
  gtk_scale_set_digits (GTK_SCALE (hscale), 0);
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		      GTK_SIGNAL_FUNC (delay_cb), ad);
  gtk_box_pack_start (GTK_BOX (pref_vbox_2), hscale, TRUE, TRUE,
		      GNOME_PAD_SMALL);
  gtk_widget_show (hscale);

  frame =
    gtk_frame_new (_
		   ("Compressed Quality (JPEG/MIFF/PNG mode) High: good quality/large file"));
  gtk_box_pack_start (GTK_BOX (pref_vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  pref_vbox_2 = gtk_hbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (pref_vbox_2),
				  GNOME_PAD_SMALL);
  gtk_container_add (GTK_CONTAINER (frame), pref_vbox_2);
  gtk_widget_show (pref_vbox_2);

  adj = gtk_adjustment_new (ad->quality, 0.0, 100.0, 10.0, 1.0, 0.0);
  hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_range_set_update_policy (GTK_RANGE (hscale), GTK_UPDATE_DELAYED);
  gtk_scale_set_digits (GTK_SCALE (hscale), 0);
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		      GTK_SIGNAL_FUNC (quality_cb), ad);
  gtk_box_pack_start (GTK_BOX (pref_vbox_2), hscale, TRUE, TRUE,
		      GNOME_PAD_SMALL);
  gtk_widget_show (hscale);

  ad->spurious_pref_vbox3 = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
  gtk_container_set_border_width (GTK_CONTAINER (ad->spurious_pref_vbox3), 0);
  gtk_box_pack_start (GTK_BOX (pref_vbox), ad->spurious_pref_vbox3, FALSE,
		      FALSE, 0);

  button = gtk_check_button_new_with_label (_ ("Create monochrome image"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), ad->monochrome);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      (GtkSignalFunc) monochrome_cb, NULL);
  gtk_box_pack_start (GTK_BOX (ad->spurious_pref_vbox3), button, FALSE, FALSE,
		      0);
  gtk_widget_show (button);

  button = gtk_check_button_new_with_label (_ ("Invert colours in image"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), ad->negate);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      (GtkSignalFunc) negate_cb, NULL);
  gtk_box_pack_start (GTK_BOX (ad->spurious_pref_vbox3), button, FALSE, FALSE,
		      0);
  gtk_widget_show (button);

  label = gtk_label_new (_ ("General"));
  gtk_widget_show (pref_vbox);
  gnome_property_box_append_page (GNOME_PROPERTY_BOX (ad->propwindow),
				  pref_vbox, label);

  pref_vbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
  gtk_container_set_border_width (GTK_CONTAINER (pref_vbox), GNOME_PAD_SMALL);
  gtk_widget_show (pref_vbox);

  pref_hbox = gtk_hbox_new (FALSE, GNOME_PAD_SMALL);
  gtk_box_pack_start (GTK_BOX (pref_vbox), pref_hbox, FALSE, FALSE, 0);
  gtk_widget_show (pref_hbox);

  label = gtk_label_new (_ ("Directory to save images in:"));
  gtk_box_pack_start (GTK_BOX (pref_hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  ad->directory_entry = gnome_file_entry_new ("output_dir",
					      "Select directory");
  gnome_file_entry_set_directory(GNOME_FILE_ENTRY(ad->directory_entry), TRUE);
  entry = GTK_COMBO (GNOME_FILE_ENTRY (ad->directory_entry)->gentry)->entry;
  
  if (ad->directory)
    gtk_entry_set_text (GTK_ENTRY (entry), ad->directory);

  gtk_signal_connect_object (GTK_OBJECT (entry), "changed",
			     GTK_SIGNAL_FUNC (gnome_property_box_changed),
			     GTK_OBJECT (ad->propwindow));

  gtk_box_pack_start (GTK_BOX (pref_hbox), ad->directory_entry, TRUE, TRUE,
		      0);
  gtk_widget_show (ad->directory_entry);

  pref_hbox = gtk_hbox_new (FALSE, GNOME_PAD_SMALL);
  gtk_box_pack_start (GTK_BOX (pref_vbox), pref_hbox, FALSE, FALSE, 0);
  gtk_widget_show (pref_hbox);
  label = gtk_label_new (_ ("Filename for images:"));
  gtk_box_pack_start (GTK_BOX (pref_hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  ad->filename_entry = gtk_entry_new ();
  if (ad->filename_entry)
    gtk_entry_set_text (GTK_ENTRY (ad->filename_entry), ad->filename);

  gtk_signal_connect_object (GTK_OBJECT (ad->filename_entry), "changed",
			     GTK_SIGNAL_FUNC (gnome_property_box_changed),
			     GTK_OBJECT (ad->propwindow));

  gtk_box_pack_start (GTK_BOX (pref_hbox), ad->filename_entry, TRUE, TRUE, 0);
  gtk_widget_show (ad->filename_entry);

  frame = gtk_frame_new (NULL);
  gtk_box_pack_start (GTK_BOX (pref_vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  pref_vbox_2 = gtk_vbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (pref_vbox_2),
				  GNOME_PAD_SMALL);
  gtk_container_add (GTK_CONTAINER (frame), pref_vbox_2);
  gtk_widget_show (pref_vbox_2);

  pref_hbox = gtk_hbox_new (FALSE, GNOME_PAD_SMALL);
  gtk_box_pack_start (GTK_BOX (pref_vbox_2), pref_hbox, FALSE, FALSE, 0);
  gtk_widget_show (pref_hbox);
  label = gtk_label_new (_
			 ("Filetype is determined from filename suffix.\n"
			  "Default filetype (if no extension or unrecognised extension) "
			  "is miff.\n"
			  "Recognised suffixes are:\n"
			  "jpg, jpeg, bmp, cgm, cmyk, dib, eps, fig, fpx, gif, ico, map, miff, pcx,\n"
			  "pict, pix, png, pnm, ppm, psd, rgb, rgba, rla, rle, tiff, tga, xbm, xpm,\n"
			  "and several obscure ones. Try your luck with something interesting!\n"
			  "Eg .html for a client-side image map. You'll get a .gif, .html, and a .shtml"));
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
  gtk_box_pack_start (GTK_BOX (pref_hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  button =
    gtk_check_button_new_with_label (_ ("View screenshot after saving it"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), ad->view);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      (GtkSignalFunc) view_cb, NULL);
  gtk_box_pack_start (GTK_BOX (pref_vbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);


  pref_hbox = gtk_hbox_new (FALSE, GNOME_PAD_SMALL);
  gtk_box_pack_start (GTK_BOX (pref_vbox), pref_hbox, FALSE, FALSE, 0);
  gtk_widget_show (pref_hbox);
  label = gtk_label_new (_ ("App to use for displaying screenshots:"));
  gtk_box_pack_start (GTK_BOX (pref_hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  ad->app_entry = gtk_entry_new ();
  if (ad->app_entry)
    gtk_entry_set_text (GTK_ENTRY (ad->app_entry), ad->app);

  gtk_signal_connect_object (GTK_OBJECT (ad->app_entry), "changed",
			     GTK_SIGNAL_FUNC (gnome_property_box_changed),
			     GTK_OBJECT (ad->propwindow));

  gtk_box_pack_start (GTK_BOX (pref_hbox), ad->app_entry, TRUE, TRUE, 0);
  gtk_widget_show (ad->app_entry);

  label = gtk_label_new (_ ("Files, Apps"));
  gnome_property_box_append_page (GNOME_PROPERTY_BOX (ad->propwindow),
				  pref_vbox, label);


  pref_vbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
  gtk_container_set_border_width (GTK_CONTAINER (pref_vbox), GNOME_PAD_SMALL);

  button =
    gtk_check_button_new_with_label (_ ("Create thumbnail of image too"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), ad->thumb);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      (GtkSignalFunc) thumb_cb, NULL);
  gtk_box_pack_start (GTK_BOX (pref_vbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);


  frame = gtk_frame_new (_ ("Thumbnail Size (percentage of original)"));
  gtk_box_pack_start (GTK_BOX (pref_vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  pref_vbox_2 = gtk_hbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (pref_vbox_2),
				  GNOME_PAD_SMALL);
  gtk_container_add (GTK_CONTAINER (frame), pref_vbox_2);
  gtk_widget_show (pref_vbox_2);

  adj = gtk_adjustment_new (ad->thumb_size, 1.0, 100.0, 10.0, 1.0, 0.0);
  hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_range_set_update_policy (GTK_RANGE (hscale), GTK_UPDATE_DELAYED);
  gtk_scale_set_digits (GTK_SCALE (hscale), 0);
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		      GTK_SIGNAL_FUNC (thumb_size_cb), ad);
  gtk_box_pack_start (GTK_BOX (pref_vbox_2), hscale, TRUE, TRUE, 0);
  gtk_widget_show (hscale);

  frame =
    gtk_frame_new (_
		   ("Thumbnail compression (JPEG/MIFF/PNG mode) "
		    "High: good quality/large file"));
  gtk_box_pack_start (GTK_BOX (pref_vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  pref_vbox_2 = gtk_hbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (pref_vbox_2),
				  GNOME_PAD_SMALL);
  gtk_container_add (GTK_CONTAINER (frame), pref_vbox_2);
  gtk_widget_show (pref_vbox_2);

  adj = gtk_adjustment_new (ad->thumb_quality, 0.0, 100.0, 10.0, 1.0, 0.0);
  hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_range_set_update_policy (GTK_RANGE (hscale), GTK_UPDATE_DELAYED);
  gtk_scale_set_digits (GTK_SCALE (hscale), 0);
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		      GTK_SIGNAL_FUNC (thumb_quality_cb), ad);
  gtk_box_pack_start (GTK_BOX (pref_vbox_2), hscale, TRUE, TRUE, 0);
  gtk_widget_show (hscale);

  pref_hbox = gtk_hbox_new (FALSE, GNOME_PAD_SMALL);
  gtk_box_pack_start (GTK_BOX (pref_vbox), pref_hbox, FALSE, FALSE, 0);
  gtk_widget_show (pref_hbox);

  label = gtk_label_new (_ ("Prefix to attach to thumbnail filename:"));
  gtk_box_pack_start (GTK_BOX (pref_hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  ad->thumb_filename_entry = gtk_entry_new ();
  if (ad->thumb_filename_entry)
    gtk_entry_set_text (GTK_ENTRY (ad->thumb_filename_entry),
			ad->thumb_filename);

  gtk_signal_connect_object (GTK_OBJECT (ad->thumb_filename_entry), "changed",
			     GTK_SIGNAL_FUNC (gnome_property_box_changed),
			     GTK_OBJECT (ad->propwindow));

  gtk_box_pack_start (GTK_BOX (pref_hbox), ad->thumb_filename_entry, TRUE,
		      TRUE, 0);
  gtk_widget_show (ad->thumb_filename_entry);


  frame = gtk_frame_new (NULL);
  gtk_box_pack_start (GTK_BOX (pref_vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  pref_vbox_2 = gtk_vbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (pref_vbox_2),
				  GNOME_PAD_SMALL);
  gtk_container_add (GTK_CONTAINER (frame), pref_vbox_2);
  gtk_widget_show (pref_vbox_2);

  button =
    gtk_check_button_new_with_label (_
				     ("Use high-quality intermediate "
				      "for generating thumbnail"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button),
				ad->thumbnail_intermediate);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      (GtkSignalFunc) thumbnail_intermediate_cb, NULL);
  gtk_box_pack_start (GTK_BOX (pref_vbox_2), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  pref_hbox = gtk_hbox_new (FALSE, GNOME_PAD_SMALL);
  gtk_box_pack_start (GTK_BOX (pref_vbox_2), pref_hbox, FALSE, FALSE, 0);
  gtk_widget_show (pref_hbox);
  label =
    gtk_label_new (_
		   ("(This'll chug loads of CPU, but will prevent compressing the "
		    "image twice if a\nlossy file format is used)\n"
		    "Don't use this if you're using a lossless compression format, or "
		    "if you cherish\nspeed and don't mind imperfection in your thumbnails."));
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
  gtk_box_pack_start (GTK_BOX (pref_hbox), label, FALSE, FALSE, 0);

  label = gtk_label_new (_ ("Thumbnails"));
  gtk_widget_show_all (pref_vbox);
  gnome_property_box_append_page (GNOME_PROPERTY_BOX (ad->propwindow),
				  pref_vbox, label);

  pref_vbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
  gtk_container_set_border_width (GTK_CONTAINER (pref_vbox), GNOME_PAD_SMALL);

  frame = gtk_frame_new (NULL);
  gtk_box_pack_start (GTK_BOX (pref_vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  pref_vbox_2 = gtk_vbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (pref_vbox_2),
				  GNOME_PAD_SMALL);
  gtk_container_add (GTK_CONTAINER (frame), pref_vbox_2);
  gtk_widget_show (pref_vbox_2);

  pref_hbox = gtk_hbox_new (FALSE, GNOME_PAD_SMALL);
  gtk_box_pack_start (GTK_BOX (pref_vbox_2), pref_hbox, FALSE, FALSE, 0);
  gtk_widget_show (pref_hbox);
  label =
    gtk_label_new (_
		   ("All post-processing, frill and spurious options "
		    "will chug more CPU than a\nsimple screenshot, "
		    "due to the creation and then conversion of an intermediate\n"
		    "image. Intensive operations may take some time on less spiffy CPUs."));
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
  gtk_box_pack_start (GTK_BOX (pref_hbox), label, FALSE, FALSE, 0);

  pref_hbox = gtk_hbox_new (FALSE, GNOME_PAD_SMALL);
  gtk_box_pack_start (GTK_BOX (pref_vbox), pref_hbox, FALSE, FALSE, 0);
  gtk_widget_show (pref_hbox);

  button =
    gtk_check_button_new_with_label (_
				     ("Normalize image (Span full range of "
				      "color values to enhance contrast)"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), ad->normalize);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      (GtkSignalFunc) normalize_cb, NULL);
  gtk_box_pack_start (GTK_BOX (pref_vbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  button =
    gtk_check_button_new_with_label (_
				     ("Equalize image (Perform "
				      "histogram equalization)"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), ad->equalize);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      (GtkSignalFunc) equalize_cb, NULL);
  gtk_box_pack_start (GTK_BOX (pref_vbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  pref_hbox = gtk_hbox_new (FALSE, GNOME_PAD_SMALL);
  gtk_box_pack_start (GTK_BOX (pref_vbox), pref_hbox, FALSE, FALSE, 0);
  gtk_widget_show (pref_hbox);

  button =
    gtk_check_button_new_with_label (_ ("Enhance image (reduce noise)"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), ad->enhance);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      (GtkSignalFunc) enhance_cb, NULL);
  gtk_box_pack_start (GTK_BOX (pref_hbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  button =
    gtk_check_button_new_with_label (_ ("Despeckle Image (reduce spots)"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), ad->despeckle);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      (GtkSignalFunc) despeckle_cb, NULL);
  gtk_box_pack_start (GTK_BOX (pref_hbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  frame = gtk_frame_new (_ ("Sharpen Image by what factor?"));
  gtk_box_pack_start (GTK_BOX (pref_vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  pref_vbox_2 = gtk_hbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (pref_vbox_2),
				  GNOME_PAD_SMALL);
  gtk_container_add (GTK_CONTAINER (frame), pref_vbox_2);
  gtk_widget_show (pref_vbox_2);

  adj = gtk_adjustment_new (ad->sharpen_factor, 0.0, 100.0, 10.0, 1.0, 0.0);
  hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_range_set_update_policy (GTK_RANGE (hscale), GTK_UPDATE_DELAYED);
  gtk_scale_set_digits (GTK_SCALE (hscale), 0);
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		      GTK_SIGNAL_FUNC (sharpen_factor_cb), ad);
  gtk_box_pack_start (GTK_BOX (pref_vbox_2), hscale, TRUE, TRUE, 0);
  gtk_widget_show (hscale);

  frame = gtk_frame_new (_ ("Rotate image clockwise by how many degrees?"));
  gtk_box_pack_start (GTK_BOX (pref_vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  pref_vbox_2 = gtk_hbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (pref_vbox_2),
				  GNOME_PAD_SMALL);
  gtk_container_add (GTK_CONTAINER (frame), pref_vbox_2);
  gtk_widget_show (pref_vbox_2);

  adj = gtk_adjustment_new (ad->rotate_degrees, 0.0, 360.0, 10.0, 1.0, 0.0);
  hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_range_set_update_policy (GTK_RANGE (hscale), GTK_UPDATE_DELAYED);
  gtk_scale_set_digits (GTK_SCALE (hscale), 0);
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		      GTK_SIGNAL_FUNC (rotate_degrees_cb), ad);
  gtk_box_pack_start (GTK_BOX (pref_vbox_2), hscale, TRUE, TRUE, 0);
  gtk_widget_show (hscale);

  button = gtk_check_button_new_with_label (_ ("Adjust gamma"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), ad->gamma);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      (GtkSignalFunc) gamma_cb, NULL);
  gtk_box_pack_start (GTK_BOX (pref_vbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  frame = gtk_frame_new (_ ("Gamma value"));
  gtk_box_pack_start (GTK_BOX (pref_vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  pref_vbox_2 = gtk_hbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (pref_vbox_2),
				  GNOME_PAD_SMALL);
  gtk_container_add (GTK_CONTAINER (frame), pref_vbox_2);
  gtk_widget_show (pref_vbox_2);

  adj = gtk_adjustment_new (ad->gamma_factor, 0.8, 2.3, 0.1, 0.1, 0.0);
  hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_range_set_update_policy (GTK_RANGE (hscale), GTK_UPDATE_DELAYED);
  gtk_scale_set_digits (GTK_SCALE (hscale), 1);
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		      GTK_SIGNAL_FUNC (gamma_factor_cb), ad);
  gtk_box_pack_start (GTK_BOX (pref_vbox_2), hscale, TRUE, TRUE, 0);
  gtk_widget_show (hscale);

  pref_hbox = gtk_hbox_new (FALSE, GNOME_PAD_SMALL);
  gtk_box_pack_start (GTK_BOX (pref_vbox), pref_hbox, FALSE, FALSE, 0);
  gtk_widget_show (pref_hbox);

  label = gtk_label_new (_ ("Post Processing"));
  gtk_widget_show_all (pref_vbox);
  gnome_property_box_append_page (GNOME_PROPERTY_BOX (ad->propwindow),
				  pref_vbox, label);

  pref_vbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
  gtk_container_set_border_width (GTK_CONTAINER (pref_vbox), GNOME_PAD_SMALL);

  button = gtk_check_button_new_with_label (_ ("Create frame around image"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), ad->frame);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      (GtkSignalFunc) frame_cb, NULL);
  gtk_box_pack_start (GTK_BOX (pref_vbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  frame = gtk_frame_new (_ ("Frame Width (pixels)"));
  gtk_box_pack_start (GTK_BOX (pref_vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  pref_vbox_2 = gtk_hbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (pref_vbox_2),
				  GNOME_PAD_SMALL);
  gtk_container_add (GTK_CONTAINER (frame), pref_vbox_2);
  gtk_widget_show (pref_vbox_2);

  adj = gtk_adjustment_new (ad->frame_size, 1.0, 50.0, 10.0, 1.0, 0.0);
  hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_range_set_update_policy (GTK_RANGE (hscale), GTK_UPDATE_DELAYED);
  gtk_scale_set_digits (GTK_SCALE (hscale), 0);
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		      GTK_SIGNAL_FUNC (frame_size_cb), ad);
  gtk_box_pack_start (GTK_BOX (pref_vbox_2), hscale, TRUE, TRUE, 0);
  gtk_widget_show (hscale);

  pref_hbox = gtk_hbox_new (FALSE, GNOME_PAD_SMALL);
  gtk_box_pack_start (GTK_BOX (pref_vbox), pref_hbox, FALSE, FALSE, 0);
  gtk_widget_show (pref_hbox);

  button =
    gtk_check_button_new_with_label (_ ("Mirror image vertically"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), ad->flip);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      (GtkSignalFunc) flip_cb, NULL);
  gtk_box_pack_start (GTK_BOX (pref_hbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  button =
    gtk_check_button_new_with_label (_ ("Mirror image horizontally"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), ad->flop);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      (GtkSignalFunc) flop_cb, NULL);
  gtk_box_pack_start (GTK_BOX (pref_hbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  button = gtk_check_button_new_with_label (_ ("Emboss image"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), ad->emboss);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      (GtkSignalFunc) emboss_cb, NULL);
  gtk_box_pack_start (GTK_BOX (pref_vbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  frame = gtk_frame_new (NULL);
  gtk_box_pack_start (GTK_BOX (pref_vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  pref_vbox_2 = gtk_vbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (pref_vbox_2),
				  GNOME_PAD_SMALL);
  gtk_container_add (GTK_CONTAINER (frame), pref_vbox_2);
  gtk_widget_show (pref_vbox_2);

  button =
    gtk_check_button_new_with_label (_
				     ("When finished, send image and "
				      "thumbnail filenames to script/program below"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), ad->use_script);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      (GtkSignalFunc) use_script_cb, NULL);
  gtk_box_pack_start (GTK_BOX (pref_vbox_2), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  pref_hbox = gtk_hbox_new (FALSE, GNOME_PAD_SMALL);
  gtk_box_pack_start (GTK_BOX (pref_vbox_2), pref_hbox, FALSE, FALSE, 0);
  gtk_widget_show (pref_hbox);

  label = gtk_label_new (_ ("Script or program to launch:"));
  gtk_box_pack_start (GTK_BOX (pref_hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  ad->script_entry = gtk_entry_new ();
  if (ad->script_entry)
    gtk_entry_set_text (GTK_ENTRY (ad->script_entry), ad->script_filename);

  gtk_signal_connect_object (GTK_OBJECT (ad->script_entry), "changed",
			     GTK_SIGNAL_FUNC (gnome_property_box_changed),
			     GTK_OBJECT (ad->propwindow));

  gtk_box_pack_start (GTK_BOX (pref_hbox), ad->script_entry, TRUE, TRUE, 0);
  gtk_widget_show (ad->script_entry);

  pref_hbox = gtk_hbox_new (FALSE, GNOME_PAD_SMALL);
  gtk_box_pack_start (GTK_BOX (pref_vbox_2), pref_hbox, FALSE, FALSE, 0);
  gtk_widget_show (pref_hbox);

  label =
    gtk_label_new (_
		   ("Script/program will be launched after image creation with the "
		    "image filenames\nspecified on the commandline as follows:\n"
		    "{Script_Name} {Image_Filename} {Thumbnail_filename_if_specified}\n"
		    "Hint: I use a script to update my website screenshot page.\n"
		    "An example script should've been included in the tarball, or at my website."));
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
  gtk_box_pack_start (GTK_BOX (pref_hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  label = gtk_label_new (_ ("Frills"));
  gtk_widget_show_all (pref_vbox);
  gnome_property_box_append_page (GNOME_PROPERTY_BOX (ad->propwindow),
				  pref_vbox, label);

  ad->spurious_pref_vbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
  gtk_container_set_border_width (GTK_CONTAINER (ad->spurious_pref_vbox),
				  GNOME_PAD_SMALL);

  button = gtk_check_button_new_with_label (_ ("Blur Image"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), ad->blur);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      (GtkSignalFunc) blur_cb, NULL);
  gtk_box_pack_start (GTK_BOX (ad->spurious_pref_vbox), button, FALSE, FALSE,
		      0);
  gtk_widget_show (button);

  frame = gtk_frame_new (_ ("Blur image by what factor?"));
  gtk_box_pack_start (GTK_BOX (ad->spurious_pref_vbox), frame, FALSE, FALSE,
		      0);
  gtk_widget_show (frame);

  pref_vbox_2 = gtk_hbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (pref_vbox_2),
				  GNOME_PAD_SMALL);
  gtk_container_add (GTK_CONTAINER (frame), pref_vbox_2);
  gtk_widget_show (pref_vbox_2);

  adj = gtk_adjustment_new (ad->blur_factor, 0.0, 100.0, 10.0, 1.0, 0.0);
  hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_range_set_update_policy (GTK_RANGE (hscale), GTK_UPDATE_DELAYED);
  gtk_scale_set_digits (GTK_SCALE (hscale), 0);
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		      GTK_SIGNAL_FUNC (blur_factor_cb), ad);
  gtk_box_pack_start (GTK_BOX (pref_vbox_2), hscale, TRUE, TRUE, 0);
  gtk_widget_show (hscale);


  button = gtk_check_button_new_with_label (_ ("Create Charcoal Effect"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), ad->charcoal);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      (GtkSignalFunc) charcoal_cb, NULL);
  gtk_box_pack_start (GTK_BOX (ad->spurious_pref_vbox), button, FALSE, FALSE,
		      0);
  gtk_widget_show (button);

  frame = gtk_frame_new (_ ("Charcoal by what factor?"));
  gtk_box_pack_start (GTK_BOX (ad->spurious_pref_vbox), frame, FALSE, FALSE,
		      0);
  gtk_widget_show (frame);

  pref_vbox_2 = gtk_hbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (pref_vbox_2),
				  GNOME_PAD_SMALL);
  gtk_container_add (GTK_CONTAINER (frame), pref_vbox_2);
  gtk_widget_show (pref_vbox_2);

  adj = gtk_adjustment_new (ad->charcoal_factor, 0.0, 100.0, 10.0, 1.0, 0.0);
  hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_range_set_update_policy (GTK_RANGE (hscale), GTK_UPDATE_DELAYED);
  gtk_scale_set_digits (GTK_SCALE (hscale), 0);
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		      GTK_SIGNAL_FUNC (charcoal_factor_cb), ad);
  gtk_box_pack_start (GTK_BOX (pref_vbox_2), hscale, TRUE, TRUE, 0);
  gtk_widget_show (hscale);


  button = gtk_check_button_new_with_label (_ ("Find Edges in Image"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), ad->edge);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      (GtkSignalFunc) edge_cb, NULL);
  gtk_box_pack_start (GTK_BOX (ad->spurious_pref_vbox), button, FALSE, FALSE,
		      0);
  gtk_widget_show (button);

  frame = gtk_frame_new (_ ("Find edges by what factor?"));
  gtk_box_pack_start (GTK_BOX (ad->spurious_pref_vbox), frame, FALSE, FALSE,
		      0);
  gtk_widget_show (frame);

  pref_vbox_2 = gtk_hbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (pref_vbox_2),
				  GNOME_PAD_SMALL);
  gtk_container_add (GTK_CONTAINER (frame), pref_vbox_2);
  gtk_widget_show (pref_vbox_2);

  adj = gtk_adjustment_new (ad->edge_factor, 0.0, 100.0, 10.0, 1.0, 0.0);
  hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_range_set_update_policy (GTK_RANGE (hscale), GTK_UPDATE_DELAYED);
  gtk_scale_set_digits (GTK_SCALE (hscale), 0);
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		      GTK_SIGNAL_FUNC (edge_factor_cb), ad);
  gtk_box_pack_start (GTK_BOX (pref_vbox_2), hscale, TRUE, TRUE, 0);
  gtk_widget_show (hscale);

  button = gtk_check_button_new_with_label (_ ("Implode Image"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), ad->implode);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      (GtkSignalFunc) implode_cb, NULL);
  gtk_box_pack_start (GTK_BOX (ad->spurious_pref_vbox), button, FALSE, FALSE,
		      0);
  gtk_widget_show (button);

  frame = gtk_frame_new (_ ("Implode by what factor?"));
  gtk_box_pack_start (GTK_BOX (ad->spurious_pref_vbox), frame, FALSE, FALSE,
		      0);
  gtk_widget_show (frame);

  pref_vbox_2 = gtk_hbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (pref_vbox_2),
				  GNOME_PAD_SMALL);
  gtk_container_add (GTK_CONTAINER (frame), pref_vbox_2);
  gtk_widget_show (pref_vbox_2);

  adj = gtk_adjustment_new (ad->implode_factor, 0.0, 100.0, 10.0, 1.0, 0.0);
  hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_range_set_update_policy (GTK_RANGE (hscale), GTK_UPDATE_DELAYED);
  gtk_scale_set_digits (GTK_SCALE (hscale), 0);
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		      GTK_SIGNAL_FUNC (implode_factor_cb), ad);
  gtk_box_pack_start (GTK_BOX (pref_vbox_2), hscale, TRUE, TRUE, 0);
  gtk_widget_show (hscale);


  label = gtk_label_new (_ ("Spurious"));
  gnome_property_box_append_page (GNOME_PROPERTY_BOX (ad->propwindow),
				  ad->spurious_pref_vbox, label);

  ad->spurious_pref_vbox2 = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
  gtk_container_set_border_width (GTK_CONTAINER (ad->spurious_pref_vbox2),
				  GNOME_PAD_SMALL);


  button = gtk_check_button_new_with_label (_ ("Create Painted Effect"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), ad->paint);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      (GtkSignalFunc) paint_cb, NULL);
  gtk_box_pack_start (GTK_BOX (ad->spurious_pref_vbox2), button, FALSE, FALSE,
		      0);
  gtk_widget_show (button);

  frame = gtk_frame_new (_ ("Paint what radius around each pixel?"));
  gtk_box_pack_start (GTK_BOX (ad->spurious_pref_vbox2), frame, FALSE, FALSE,
		      0);
  gtk_widget_show (frame);

  pref_vbox_2 = gtk_hbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (pref_vbox_2),
				  GNOME_PAD_SMALL);
  gtk_container_add (GTK_CONTAINER (frame), pref_vbox_2);
  gtk_widget_show (pref_vbox_2);

  adj = gtk_adjustment_new (ad->paint_radius, 0.0, 20.0, 1.0, 1.0, 0.0);
  hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_range_set_update_policy (GTK_RANGE (hscale), GTK_UPDATE_DELAYED);
  gtk_scale_set_digits (GTK_SCALE (hscale), 0);
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		      GTK_SIGNAL_FUNC (paint_radius_cb), ad);
  gtk_box_pack_start (GTK_BOX (pref_vbox_2), hscale, TRUE, TRUE, 0);
  gtk_widget_show (hscale);

  button = gtk_check_button_new_with_label (_ ("Solarise Image"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), ad->solarize);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      (GtkSignalFunc) solarize_cb, NULL);
  gtk_box_pack_start (GTK_BOX (ad->spurious_pref_vbox2), button, FALSE, FALSE,
		      0);
  gtk_widget_show (button);

  frame = gtk_frame_new (_ ("Solarize factor?"));
  gtk_box_pack_start (GTK_BOX (ad->spurious_pref_vbox2), frame, FALSE, FALSE,
		      0);
  gtk_widget_show (frame);

  pref_vbox_2 = gtk_hbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (pref_vbox_2),
				  GNOME_PAD_SMALL);
  gtk_container_add (GTK_CONTAINER (frame), pref_vbox_2);
  gtk_widget_show (pref_vbox_2);

  adj = gtk_adjustment_new (ad->solarize_factor, 0.0, 100.0, 10.0, 1.0, 0.0);
  hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_range_set_update_policy (GTK_RANGE (hscale), GTK_UPDATE_DELAYED);
  gtk_scale_set_digits (GTK_SCALE (hscale), 0);
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		      GTK_SIGNAL_FUNC (solarize_factor_cb), ad);
  gtk_box_pack_start (GTK_BOX (pref_vbox_2), hscale, TRUE, TRUE, 0);
  gtk_widget_show (hscale);

  button = gtk_check_button_new_with_label (_ ("Spread image pixels"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), ad->spread);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      (GtkSignalFunc) spread_cb, NULL);
  gtk_box_pack_start (GTK_BOX (ad->spurious_pref_vbox2), button, FALSE, FALSE,
		      0);
  gtk_widget_show (button);

  frame = gtk_frame_new (_ ("Radius to around each pixel to spread?"));
  gtk_box_pack_start (GTK_BOX (ad->spurious_pref_vbox2), frame, FALSE, FALSE,
		      0);
  gtk_widget_show (frame);

  pref_vbox_2 = gtk_hbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (pref_vbox_2),
				  GNOME_PAD_SMALL);
  gtk_container_add (GTK_CONTAINER (frame), pref_vbox_2);
  gtk_widget_show (pref_vbox_2);

  adj = gtk_adjustment_new (ad->spread_radius, 0.0, 20.0, 1.0, 1.0, 0.0);
  hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_range_set_update_policy (GTK_RANGE (hscale), GTK_UPDATE_DELAYED);
  gtk_scale_set_digits (GTK_SCALE (hscale), 0);
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		      GTK_SIGNAL_FUNC (spread_radius_cb), ad);
  gtk_box_pack_start (GTK_BOX (pref_vbox_2), hscale, TRUE, TRUE, 0);
  gtk_widget_show (hscale);

  button =
    gtk_check_button_new_with_label (_ ("Swirl pixels. My favorite :-)"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), ad->swirl);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      (GtkSignalFunc) swirl_cb, NULL);
  gtk_box_pack_start (GTK_BOX (ad->spurious_pref_vbox2), button, FALSE, FALSE,
		      0);
  gtk_widget_show (button);

  frame = gtk_frame_new (_ ("Radius to swirl pixels around?)"));
  gtk_box_pack_start (GTK_BOX (ad->spurious_pref_vbox2), frame, FALSE, FALSE,
		      0);
  gtk_widget_show (frame);

  pref_vbox_2 = gtk_hbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (pref_vbox_2),
				  GNOME_PAD_SMALL);
  gtk_container_add (GTK_CONTAINER (frame), pref_vbox_2);
  gtk_widget_show (pref_vbox_2);

  adj = gtk_adjustment_new (ad->swirl_degrees, 0.0, 360.0, 10.0, 1.0, 0.0);
  hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_range_set_update_policy (GTK_RANGE (hscale), GTK_UPDATE_DELAYED);
  gtk_scale_set_digits (GTK_SCALE (hscale), 0);
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		      GTK_SIGNAL_FUNC (swirl_degrees_cb), ad);
  gtk_box_pack_start (GTK_BOX (pref_vbox_2), hscale, TRUE, TRUE, 0);
  gtk_widget_show (hscale);

  label = gtk_label_new (_ ("Spurious 2"));
  gnome_property_box_append_page (GNOME_PROPERTY_BOX (ad->propwindow),
				  ad->spurious_pref_vbox2, label);

  gtk_signal_connect (GTK_OBJECT (ad->propwindow), "apply",
		      GTK_SIGNAL_FUNC (property_apply_cb), ad);
  gtk_signal_connect (GTK_OBJECT (ad->propwindow), "destroy",
		      GTK_SIGNAL_FUNC (property_destroy_cb), ad);

  if (ad->spurious)
    {
      gtk_widget_show_all (ad->spurious_pref_vbox);
      gtk_widget_show_all (ad->spurious_pref_vbox2);
      gtk_widget_show_all (ad->spurious_pref_vbox3);
    }

  gtk_widget_show (ad->propwindow);
  data = NULL;
  widget = NULL;
}

static gint
applet_save_session (GtkWidget * widget, gchar * privcfgpath,
		     gchar * globcfgpath, gpointer data)
{
  properties_save (privcfgpath, NULL);
  return FALSE;
  widget = NULL;
  globcfgpath = NULL;
  data = NULL;
}

int
main (int argc, char *argv[])
{
  GtkWidget *hbox;
  GtkWidget *vbox;
  GtkWidget *mainbox;
  GtkWidget *hwindow_button, *hroot_button;
  GtkWidget *vwindow_button, *vroot_button;
  GtkWidget *hwindowpixmap;
  GtkWidget *hrootpixmap;
  GtkWidget *vwindowpixmap;
  GtkWidget *vrootpixmap;
  PanelOrientType orient;
  gboolean init_size_tiny = FALSE;
#ifdef HAVE_PANEL_PIXEL_SIZE
  int init_size;
#endif

  /* Initialize the i18n stuff */
  bindtextdomain (PACKAGE, GNOMELOCALEDIR);
  textdomain (PACKAGE);

  applet_widget_init ("screenshooter_applet", VERSION, argc, argv, NULL, 0,
		      NULL);
  applet = applet_widget_new ("screenshooter_applet");
  if (!applet)
    g_error (_ ("Can't create applet!\n"));

  gtk_widget_realize (applet);

  properties_load (APPLET_WIDGET (applet)->privcfgpath, NULL);

  mainbox = gtk_vbox_new (FALSE, 0);
  hbox = gtk_hbox_new (FALSE, 1);
  vbox = gtk_vbox_new (FALSE, 1);

  vframe = gtk_frame_new (NULL);
  gtk_container_set_border_width (GTK_CONTAINER (vframe), 0);
  gtk_frame_set_shadow_type (GTK_FRAME (vframe), GTK_SHADOW_IN);

  hframe = gtk_frame_new (NULL);
  gtk_container_set_border_width (GTK_CONTAINER (hframe), 0);
  gtk_frame_set_shadow_type (GTK_FRAME (hframe), GTK_SHADOW_IN);

  gtk_container_add (GTK_CONTAINER (hframe), hbox);
  gtk_container_add (GTK_CONTAINER (vframe), vbox);
  gtk_box_pack_start (GTK_BOX (mainbox), hframe, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (mainbox), vframe, FALSE, FALSE, 0);

  hwindow_button = gtk_button_new ();
  vwindow_button = gtk_button_new ();
  hroot_button = gtk_button_new ();
  vroot_button = gtk_button_new ();
  gtk_widget_set_usize (hwindow_button, 18, 18);
  gtk_widget_set_usize (vwindow_button, 18, 18);
  gtk_widget_set_usize (hroot_button, 18, 18);
  gtk_widget_set_usize (vroot_button, 18, 18);
  gtk_box_pack_start (GTK_BOX (hbox), hwindow_button, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), vwindow_button, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), hroot_button, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), vroot_button, TRUE, TRUE, 0);
  gtk_signal_connect (GTK_OBJECT
		      (hwindow_button), "clicked",
		      GTK_SIGNAL_FUNC (window_button_press), &options);
  gtk_signal_connect (GTK_OBJECT
		      (hroot_button), "clicked",
		      GTK_SIGNAL_FUNC (desktop_button_press), &options);
  gtk_signal_connect (GTK_OBJECT
		      (vwindow_button), "clicked",
		      GTK_SIGNAL_FUNC (window_button_press), &options);
  gtk_signal_connect (GTK_OBJECT
		      (vroot_button), "clicked",
		      GTK_SIGNAL_FUNC (desktop_button_press), &options);
  GTK_WIDGET_UNSET_FLAGS (hwindow_button, GTK_CAN_DEFAULT);
  GTK_WIDGET_UNSET_FLAGS (hroot_button, GTK_CAN_DEFAULT);
  GTK_WIDGET_UNSET_FLAGS (hwindow_button, GTK_CAN_FOCUS);
  GTK_WIDGET_UNSET_FLAGS (hroot_button, GTK_CAN_FOCUS);
  GTK_WIDGET_UNSET_FLAGS (vwindow_button, GTK_CAN_DEFAULT);
  GTK_WIDGET_UNSET_FLAGS (vroot_button, GTK_CAN_DEFAULT);
  GTK_WIDGET_UNSET_FLAGS (vwindow_button, GTK_CAN_FOCUS);
  GTK_WIDGET_UNSET_FLAGS (vroot_button, GTK_CAN_FOCUS);

  set_tooltip (hwindow_button,
	       (_ ("Grab a shot of a specific window or area")));
  set_tooltip (hroot_button, (_ ("Grab a shot of your entire desktop")));
  set_tooltip (vwindow_button,
	       (_ ("Grab a shot of a specific window or area")));
  set_tooltip (vroot_button, (_ ("Grab a shot of your entire desktop")));

  vwindowpixmap = gnome_pixmap_new_from_xpm_d (window_icon_xpm);
  vrootpixmap = gnome_pixmap_new_from_xpm_d (root_icon_xpm);
  gtk_container_add (GTK_CONTAINER (vwindow_button), vwindowpixmap);
  gtk_container_add (GTK_CONTAINER (vroot_button), vrootpixmap);

  hwindowpixmap = gnome_pixmap_new_from_xpm_d (window_icon_xpm);
  hrootpixmap = gnome_pixmap_new_from_xpm_d (root_icon_xpm);
  gtk_container_add (GTK_CONTAINER (hwindow_button), hwindowpixmap);
  gtk_container_add (GTK_CONTAINER (hroot_button), hrootpixmap);

  gtk_widget_show (hwindowpixmap);
  gtk_widget_show (vwindow_button);
  gtk_widget_show (vroot_button);
  gtk_widget_show (hwindow_button);
  gtk_widget_show (hroot_button);
  gtk_widget_show (hrootpixmap);
  gtk_widget_show (vwindowpixmap);
  gtk_widget_show (vrootpixmap);
  gtk_widget_show (vbox);
  gtk_widget_show (hbox);
  gtk_widget_show (mainbox);

  applet_widget_add (APPLET_WIDGET (applet), mainbox);

  orient = applet_widget_get_panel_orient (APPLET_WIDGET (applet));
#ifdef HAVE_PANEL_PIXEL_SIZE
  init_size = applet_widget_get_panel_pixel_size (APPLET_WIDGET (applet));
  if (init_size < PIXEL_SIZE_STANDARD)
    init_size_tiny = TRUE;
#endif
  change_orientation (orient, init_size_tiny);

  applet_widget_register_stock_callback (APPLET_WIDGET (applet),
					 "about",
					 GNOME_STOCK_MENU_ABOUT,
					 _ ("About..."),
					 (AppletCallbackFunc) cb_about, NULL);

  applet_widget_register_stock_callback (APPLET_WIDGET (applet),
					 "properties",
					 GNOME_STOCK_MENU_PROP,
					 _ ("Properties..."),
					 (AppletCallbackFunc)
					 cb_properties_dialog, NULL);

  gtk_signal_connect (GTK_OBJECT (applet), "change_orient",
		      GTK_SIGNAL_FUNC (cb_applet_change_orient), NULL);

#ifdef HAVE_PANEL_PIXEL_SIZE
  gtk_signal_connect (GTK_OBJECT (applet), "change_pixel_size",
		      GTK_SIGNAL_FUNC (applet_change_pixel_size), NULL);
#endif

  gtk_signal_connect (GTK_OBJECT (applet), "save_session",
		      GTK_SIGNAL_FUNC (applet_save_session), NULL);

  gtk_widget_show (applet);
  applet_widget_gtk_main ();
  return 0;
}

void
window_button_press (GtkWidget * button, user_preferences * opt)
{
    grab_shot (opt, FALSE);
    return;
    button = NULL;
}

void
desktop_button_press (GtkWidget * button, user_preferences * opt)
{
  grab_shot (opt, TRUE);
    return;
    button = NULL;
}

void
grab_shot (user_preferences * opt, gboolean root)
{
  gchar *sys=NULL;
  gchar qual_buf[10];
  gchar thumb_qual_buf[10];
  gchar *dir_buf=NULL;
  gchar *file_buf=NULL;
  gchar *temp_dir_file_buf=NULL;
  gchar dec_buf[10];
  gchar mono_buf[25];
  gchar neg_buf[15];
  gchar beep_buf[10];
  gchar delay_buf[15];
  gchar *thumb_image=NULL;
  wordexp_t mywordexp;
  int wordexpret = 0;

  if (lastimage)
    {
      g_free (lastimage);
      lastimage = NULL;
    }

  /* If there is any post-processing to do, generate an intermediate
   * miff image, this prevents using lossy compression twice if for
   * example jpg mode is used. */
  if (((opt->thumb) && (opt->thumbnail_intermediate)) || (opt->frame)
      || (opt->equalize) || (opt->normalize) || (opt->gamma) || (opt->flip)
      || (opt->flop) || (opt->emboss) || (opt->enhance) || (opt->despeckle)
      || (opt->rotate_degrees > 0) || (opt->blur) || (opt->charcoal)
      || (opt->edge) || (opt->implode) || (opt->paint)
      || (opt->sharpen_factor > 0) || (opt->solarize) || (opt->spread)
      || (opt->swirl))
    opt->post_process = TRUE;
  else
    opt->post_process = FALSE;


  g_snprintf (qual_buf, sizeof (qual_buf), "%d", opt->quality);

  if (opt->thumb)
    g_snprintf (thumb_qual_buf, sizeof (thumb_qual_buf), "%d",
		opt->thumb_quality);

  if (opt->post_process)
    temp_dir_file_buf =
      g_concat_dir_and_file ("/tmp/", "screenshooter_temp.miff");

  if (opt->delay > 0)
    g_snprintf (delay_buf, sizeof (delay_buf), "sleep %d;", opt->delay);
  else
    delay_buf[0] = '\0';

  if (opt->decoration)
    g_snprintf (dec_buf, sizeof (dec_buf), "-frame");
  else
    dec_buf[0] = '\0';

  if (opt->monochrome)
    g_snprintf (mono_buf, sizeof (mono_buf), "-monochrome -dither");
  else
    mono_buf[0] = '\0';

  if (opt->negate)
    g_snprintf (neg_buf, sizeof (neg_buf), "-negate");
  else
    neg_buf[0] = '\0';

  if (opt->beep)
    beep_buf[0] = '\0';
  else
    g_snprintf (beep_buf, sizeof (beep_buf), "-silent");

/* Now perfom word expansion on the directory (in case user has used an
 * alias such as ~, as I do) */
  if ((wordexpret = wordexp (opt->directory, &mywordexp, 0) != 0))
    {
      gnome_ok_dialog (_ ("There was a word expansion error\n"
			  "I think you have stuck something funny in the directory box\n"
			  "Screenshot aborted"));
      wordfree (&mywordexp);
      return;
    }
  else
    {
      dir_buf = g_strjoinv ("", mywordexp.we_wordv);
      wordfree (&mywordexp);
    }

/* Now perfom word expansion on the filename (in case user has used an
 * embedded command such as date, as I do) */
  if ((wordexpret = wordexp (opt->filename, &mywordexp, 0) != 0))
    {
      gnome_ok_dialog (_ ("There was a word expansion error\n"
			  "I think you have stuck something funny in the filename box\n"
			  "Screenshot aborted"));
      wordfree (&mywordexp);
      return;
    }
  else
    {
      file_buf = g_strjoinv ("", mywordexp.we_wordv);
      lastimage = g_concat_dir_and_file (dir_buf, file_buf);
      if (opt->thumb)
	{
	  if (opt->thumb_filename)
	    thumb_image =
	      g_concat_dir_and_file (dir_buf,
				     g_strconcat (opt->thumb_filename,
						  file_buf, NULL));
	  else
	    thumb_image =
	      g_concat_dir_and_file (dir_buf,
				     g_strconcat ("thumb-", file_buf, NULL));
	}
      wordfree (&mywordexp);
    }

  if (opt->post_process)
    {
      if (root)
	{
	  /* Grab whole desktop to temp file for post processing */
	  sys = g_strjoin (" ",
			   delay_buf,
			   "import",
			   "-quality",
			   "100",
			   "-window",
			   "root",
			   neg_buf, beep_buf, mono_buf, temp_dir_file_buf,
			   NULL);
	}
      else
	{
	  /* Grab window to temp file for post processing */
	  sys = g_strjoin (" ",
			   "import",
			   "-quality",
			   "100",
			   dec_buf,
			   neg_buf, beep_buf, mono_buf, temp_dir_file_buf,
			   NULL);
	}
    }
  else if (!root)
    {
      /* Just grab whole desktop to specified filename */
      sys = g_strjoin (" ",
		       "import",
		       dec_buf,
		       neg_buf,
		       beep_buf, mono_buf, "-quality", qual_buf, lastimage,
		       NULL);
    }
  else
    {
      /* Just grab window to specified filename */
      sys = g_strjoin (" ",
		       delay_buf,
		       "import",
		       mono_buf,
		       neg_buf,
		       beep_buf,
		       "-window", "root", "-quality", qual_buf, lastimage,
		       NULL);
    }
  system(sys);

  if (opt->post_process)
    {
      /* Create the actual screenshot from the intermediate image, include
       * any extra post-processing options we can here */
      gchar *post_sys;
      gchar frame_buf[25];
      gchar equalize_buf[10];
      gchar normalize_buf[10];
      gchar flip_buf[10];
      gchar flop_buf[10];
      gchar emboss_buf[10];
      gchar enhance_buf[10];
      gchar despeckle_buf[10];
      gchar gamma_buf[25];
      gchar rotate_buf[20];
      gchar blur_buf[20];
      gchar charcoal_buf[20];
      gchar edge_buf[20];
      gchar implode_buf[20];
      gchar paint_buf[20];
      gchar sharpen_buf[20];
      gchar solarize_buf[20];
      gchar spread_buf[20];
      gchar swirl_buf[20];

      if (opt->frame)
	g_snprintf (frame_buf, sizeof (frame_buf), "-frame %dx%d",
		    opt->frame_size, opt->frame_size);
      else
	frame_buf[0] = '\0';

      if (opt->equalize)
	g_snprintf (equalize_buf, sizeof (equalize_buf), "-equalize");
      else
	equalize_buf[0] = '\0';

      if (opt->normalize)
	g_snprintf (normalize_buf, sizeof (normalize_buf), "-normalize");
      else
	normalize_buf[0] = '\0';

      if (opt->gamma)
	g_snprintf (gamma_buf, sizeof (gamma_buf), "-gamma %.1f",
		    opt->gamma_factor);
      else
	gamma_buf[0] = '\0';

      if (opt->flip)
	g_snprintf (flip_buf, sizeof (flip_buf), "-flip");
      else
	flip_buf[0] = '\0';

      if (opt->flop)
	g_snprintf (flop_buf, sizeof (flop_buf), "-flop");
      else
	flop_buf[0] = '\0';

      if (opt->enhance)
	g_snprintf (enhance_buf, sizeof (enhance_buf), "-enhance");
      else
	enhance_buf[0] = '\0';

      if (opt->emboss)
	g_snprintf (emboss_buf, sizeof (emboss_buf), "-emboss");
      else
	emboss_buf[0] = '\0';

      if (opt->despeckle)
	g_snprintf (despeckle_buf, sizeof (despeckle_buf), "-despeckle");
      else
	despeckle_buf[0] = '\0';

      if (opt->rotate_degrees > 0)
	g_snprintf (rotate_buf, sizeof (rotate_buf), "-rotate %d",
		    opt->rotate_degrees);
      else
	rotate_buf[0] = '\0';

      if (opt->edge)
	g_snprintf (edge_buf, sizeof (edge_buf), "-edge %d",
		    opt->edge_factor);
      else
	edge_buf[0] = '\0';

      if (opt->implode)
	g_snprintf (implode_buf, sizeof (implode_buf), "-implode %d",
		    opt->implode_factor);
      else
	implode_buf[0] = '\0';

      if (opt->paint)
	g_snprintf (paint_buf, sizeof (paint_buf), "-paint %d",
		    opt->paint_radius);
      else
	paint_buf[0] = '\0';

      if (opt->sharpen_factor > 0)
	g_snprintf (sharpen_buf, sizeof (sharpen_buf), "-sharpen %d",
		    opt->sharpen_factor);
      else
	sharpen_buf[0] = '\0';

      if (opt->solarize)
	g_snprintf (solarize_buf, sizeof (solarize_buf), "-solarize %d",
		    opt->solarize_factor);
      else
	solarize_buf[0] = '\0';

      if (opt->spread)
	g_snprintf (spread_buf, sizeof (spread_buf), "-spread %d",
		    opt->spread_radius);
      else
	spread_buf[0] = '\0';

      if (opt->swirl)
	g_snprintf (swirl_buf, sizeof (swirl_buf), "-swirl %d",
		    opt->swirl_degrees);
      else
	swirl_buf[0] = '\0';

      if (opt->charcoal)
	g_snprintf (charcoal_buf, sizeof (charcoal_buf), "-charcoal %d",
		    opt->charcoal_factor);
      else
	charcoal_buf[0] = '\0';

      if (opt->blur)
	g_snprintf (blur_buf, sizeof (blur_buf), "-blur %d",
		    opt->blur_factor);
      else
	blur_buf[0] = '\0';

      post_sys = g_strjoin (" ",
			    "convert",
			    "-quality",
			    qual_buf,
			    frame_buf,
			    equalize_buf,
			    normalize_buf,
			    flip_buf,
			    flop_buf,
			    enhance_buf,
			    emboss_buf,
			    despeckle_buf,
			    rotate_buf,
			    edge_buf,
			    implode_buf,
			    paint_buf,
			    sharpen_buf,
			    solarize_buf,
			    spread_buf,
			    swirl_buf,
			    blur_buf,
			    charcoal_buf,
			    gamma_buf, temp_dir_file_buf, lastimage, NULL);
      system (post_sys);
      g_free (post_sys);

      if (opt->thumb)
	{
	  /* Create thumbnail from intermediate image */
	  gchar *thumb_sys;
	  gchar thumb_size_buf[10];

	  g_snprintf (thumb_size_buf, sizeof (thumb_size_buf), "%d%%",
		      opt->thumb_size);

	  thumb_sys =
	    g_strjoin (" ", "convert",
		       "-quality",
		       thumb_qual_buf,
		       "-geometry",
		       thumb_size_buf,
		       equalize_buf,
		       flip_buf,
		       flop_buf,
		       enhance_buf,
		       emboss_buf,
		       despeckle_buf,
		       rotate_buf,
		       edge_buf,
		       implode_buf,
		       paint_buf,
		       sharpen_buf,
		       solarize_buf,
		       spread_buf,
		       swirl_buf,
		       blur_buf,
		       charcoal_buf,
		       gamma_buf,
		       normalize_buf, temp_dir_file_buf, thumb_image, NULL);
	  system (thumb_sys);
	  g_free (thumb_sys);
	}
    }
  else if (opt->thumb)
    {
      /* Create thumbnail from screenshot directly */
      gchar *thumb_sys;
      gchar thumb_size_buf[10];
      g_snprintf (thumb_size_buf, sizeof (thumb_size_buf), "%d%%",
		  opt->thumb_size);
      thumb_sys =
	g_strjoin (" ", "convert",
		   "-quality",
		   thumb_qual_buf,
		   "-geometry", thumb_size_buf, lastimage, thumb_image, NULL);
      system (thumb_sys);
      g_free (thumb_sys);
    }

  g_free (dir_buf);
  g_free (file_buf);
  g_free (sys);

  if ((opt->view) && (opt->app))
    {
      gchar *cmd;
      if (opt->thumb)
	{
	  cmd =
	    g_strdup_printf ("%s \"%s\" \"%s\" &", opt->app, lastimage,
			     thumb_image);
	}
      else
	{
	  cmd = g_strdup_printf ("%s \"%s\" &", opt->app, lastimage);
	}
      system (cmd);
      g_free (cmd);
    }

  if ((opt->use_script) && (opt->script_filename))
    {
      gchar *cmd;
      if (opt->thumb)
	{
	  cmd =
	    g_strdup_printf ("%s \"%s\" \"%s\" &", opt->script_filename,
			     lastimage, thumb_image);
	}
      else
	{
	  cmd =
	    g_strdup_printf ("%s \"%s\" &", opt->script_filename, lastimage);
	}
      system (cmd);
      g_free (cmd);
    }

  if ((opt->thumb) && (thumb_image))
    g_free (thumb_image);

  if (opt->post_process)
    {
      /* Delete intermediate image */
      unlink (temp_dir_file_buf);
      g_free (temp_dir_file_buf);
    }
}

void
property_apply_cb (GtkWidget * w, gpointer data)
{
  user_preferences *ad = &options;
  user_preferences *old = &old_options;
  gchar *buf;
  gint info_changed = FALSE;

  buf = gtk_entry_get_text (GTK_ENTRY (
    GTK_COMBO (GNOME_FILE_ENTRY (ad->directory_entry)->gentry)->entry));

  if ((buf) && (strcmp (buf, old->directory)))
    {
      /* Different directory selected */
      if (ad->directory)
	g_free (ad->directory);
      ad->directory = g_strdup (buf);
      info_changed = TRUE;
    }

  buf = gtk_entry_get_text (GTK_ENTRY (ad->filename_entry));

  if ((buf) && (strcmp (buf, old->filename)))
    {
      /* Different filename selected */
      if (ad->filename)
	g_free (ad->filename);
      ad->filename = g_strdup (buf);
      info_changed = TRUE;
    }

  buf = gtk_entry_get_text (GTK_ENTRY (ad->app_entry));

  if ((buf) && (strcmp (buf, old->app)))
    {
      /* Different app selected */
      if (ad->app)
	g_free (ad->app);
      ad->app = g_strdup (buf);
      info_changed = TRUE;
    }

  buf = gtk_entry_get_text (GTK_ENTRY (ad->thumb_filename_entry));

  if ((buf) && (strcmp (buf, old->thumb_filename)))
    {
      /* Different filename selected */
      if (ad->thumb_filename)
	g_free (ad->thumb_filename);
      ad->thumb_filename = g_strdup (buf);
      info_changed = TRUE;
    }

  buf = gtk_entry_get_text (GTK_ENTRY (ad->script_entry));

  if ((buf) && (strcmp (buf, old->script_filename)))
    {
      /* Different filename selected */
      if (ad->script_filename)
	g_free (ad->script_filename);
      ad->script_filename = g_strdup (buf);
      info_changed = TRUE;
    }

  if (memcmp (ad, old, sizeof (user_preferences)))
    info_changed = TRUE;

  if (info_changed)
    {
      /* User preferences save */
      /* properties_save (privpath, NULL); */
      /* Re-copy options for next comparison */
      memcpy (&old_options, &options, sizeof (user_preferences));
    }
  return;
  w = NULL;
  data = NULL;
}

void
quality_cb (GtkWidget * w, gpointer data)
{
  options.quality = GTK_ADJUSTMENT (w)->value;
  gnome_property_box_changed (GNOME_PROPERTY_BOX (options.propwindow));
  return;
  data = NULL;
}

void
gamma_factor_cb (GtkWidget * w, gpointer data)
{
  options.gamma_factor = GTK_ADJUSTMENT (w)->value;
  gnome_property_box_changed (GNOME_PROPERTY_BOX (options.propwindow));
  return;
  data = NULL;
}

void
frame_size_cb (GtkWidget * w, gpointer data)
{
  options.frame_size = GTK_ADJUSTMENT (w)->value;
  gnome_property_box_changed (GNOME_PROPERTY_BOX (options.propwindow));
  return;
  data = NULL;
}

void
thumb_quality_cb (GtkWidget * w, gpointer data)
{
  options.thumb_quality = GTK_ADJUSTMENT (w)->value;
  gnome_property_box_changed (GNOME_PROPERTY_BOX (options.propwindow));
  return;
  data = NULL;
}

void
thumb_size_cb (GtkWidget * w, gpointer data)
{
  options.thumb_size = GTK_ADJUSTMENT (w)->value;
  gnome_property_box_changed (GNOME_PROPERTY_BOX (options.propwindow));
  return;
  data = NULL;
}

void
delay_cb (GtkWidget * w, gpointer data)
{
  options.delay = GTK_ADJUSTMENT (w)->value;
  gnome_property_box_changed (GNOME_PROPERTY_BOX (options.propwindow));
  return;
  data = NULL;
}

void
directory_button_pressed (GtkWidget * w, gpointer data)
{
  gnome_ok_dialog ("I'll get there soon!");
  return;
  data = NULL;
  w = NULL;
}

void
decoration_cb (GtkWidget * w, gpointer data)
{
  options.decoration = GTK_TOGGLE_BUTTON (w)->active;
  gnome_property_box_changed (GNOME_PROPERTY_BOX (options.propwindow));
  return;
  data = NULL;
}

void
gamma_cb (GtkWidget * w, gpointer data)
{
  options.gamma = GTK_TOGGLE_BUTTON (w)->active;
  gnome_property_box_changed (GNOME_PROPERTY_BOX (options.propwindow));
  return;
  data = NULL;
}

void
enhance_cb (GtkWidget * w, gpointer data)
{
  options.enhance = GTK_TOGGLE_BUTTON (w)->active;
  gnome_property_box_changed (GNOME_PROPERTY_BOX (options.propwindow));
  return;
  data = NULL;
}

void
emboss_cb (GtkWidget * w, gpointer data)
{
  options.emboss = GTK_TOGGLE_BUTTON (w)->active;
  gnome_property_box_changed (GNOME_PROPERTY_BOX (options.propwindow));
  return;
  data = NULL;
}

void
normalize_cb (GtkWidget * w, gpointer data)
{
  options.normalize = GTK_TOGGLE_BUTTON (w)->active;
  gnome_property_box_changed (GNOME_PROPERTY_BOX (options.propwindow));
  return;
  data = NULL;
}

void
despeckle_cb (GtkWidget * w, gpointer data)
{
  options.despeckle = GTK_TOGGLE_BUTTON (w)->active;
  gnome_property_box_changed (GNOME_PROPERTY_BOX (options.propwindow));
  return;
  data = NULL;
}

void
equalize_cb (GtkWidget * w, gpointer data)
{
  options.equalize = GTK_TOGGLE_BUTTON (w)->active;
  gnome_property_box_changed (GNOME_PROPERTY_BOX (options.propwindow));
  return;
  data = NULL;
}

void
flip_cb (GtkWidget * w, gpointer data)
{
  options.flip = GTK_TOGGLE_BUTTON (w)->active;
  gnome_property_box_changed (GNOME_PROPERTY_BOX (options.propwindow));
  return;
  data = NULL;
}

void
flop_cb (GtkWidget * w, gpointer data)
{
  options.flop = GTK_TOGGLE_BUTTON (w)->active;
  gnome_property_box_changed (GNOME_PROPERTY_BOX (options.propwindow));
  return;
  data = NULL;
}

void
frame_cb (GtkWidget * w, gpointer data)
{
  options.frame = GTK_TOGGLE_BUTTON (w)->active;
  gnome_property_box_changed (GNOME_PROPERTY_BOX (options.propwindow));
  return;
  data = NULL;
}

void
thumbnail_intermediate_cb (GtkWidget * w, gpointer data)
{
  options.thumbnail_intermediate = GTK_TOGGLE_BUTTON (w)->active;
  gnome_property_box_changed (GNOME_PROPERTY_BOX (options.propwindow));
  return;
  data = NULL;
}

void
thumb_cb (GtkWidget * w, gpointer data)
{
  options.thumb = GTK_TOGGLE_BUTTON (w)->active;
  gnome_property_box_changed (GNOME_PROPERTY_BOX (options.propwindow));
  return;
  data = NULL;
}

void
monochrome_cb (GtkWidget * w, gpointer data)
{
  options.monochrome = GTK_TOGGLE_BUTTON (w)->active;
  gnome_property_box_changed (GNOME_PROPERTY_BOX (options.propwindow));
  return;
  data = NULL;
}

void
negate_cb (GtkWidget * w, gpointer data)
{
  options.negate = GTK_TOGGLE_BUTTON (w)->active;
  gnome_property_box_changed (GNOME_PROPERTY_BOX (options.propwindow));
  return;
  data = NULL;
}

void
beep_cb (GtkWidget * w, gpointer data)
{
  options.beep = GTK_TOGGLE_BUTTON (w)->active;
  gnome_property_box_changed (GNOME_PROPERTY_BOX (options.propwindow));
  return;
  data = NULL;
}

void
use_script_cb (GtkWidget * w, gpointer data)
{
  options.use_script = GTK_TOGGLE_BUTTON (w)->active;
  gnome_property_box_changed (GNOME_PROPERTY_BOX (options.propwindow));
  return;
  data = NULL;
}

void
view_cb (GtkWidget * w, gpointer data)
{
  options.view = GTK_TOGGLE_BUTTON (w)->active;
  gnome_property_box_changed (GNOME_PROPERTY_BOX (options.propwindow));
  return;
  data = NULL;
}

gint
property_destroy_cb (GtkWidget * w, gpointer data)
{
  options.propwindow = NULL;
  return FALSE;
  w = NULL;
  data = NULL;
}

void
rotate_degrees_cb (GtkWidget * w, gpointer data)
{
  options.rotate_degrees = GTK_ADJUSTMENT (w)->value;
  gnome_property_box_changed (GNOME_PROPERTY_BOX (options.propwindow));
  return;
  data = NULL;
}

void
blur_factor_cb (GtkWidget * w, gpointer data)
{
  options.blur_factor = GTK_ADJUSTMENT (w)->value;
  gnome_property_box_changed (GNOME_PROPERTY_BOX (options.propwindow));
  return;
  data = NULL;
}

void
charcoal_factor_cb (GtkWidget * w, gpointer data)
{
  options.charcoal_factor = GTK_ADJUSTMENT (w)->value;
  gnome_property_box_changed (GNOME_PROPERTY_BOX (options.propwindow));
  return;
  data = NULL;
}

void
edge_factor_cb (GtkWidget * w, gpointer data)
{
  options.edge_factor = GTK_ADJUSTMENT (w)->value;
  gnome_property_box_changed (GNOME_PROPERTY_BOX (options.propwindow));
  return;
  data = NULL;
}

void
implode_factor_cb (GtkWidget * w, gpointer data)
{
  options.implode_factor = GTK_ADJUSTMENT (w)->value;
  gnome_property_box_changed (GNOME_PROPERTY_BOX (options.propwindow));
  return;
  data = NULL;
}

void
swirl_degrees_cb (GtkWidget * w, gpointer data)
{
  options.swirl_degrees = GTK_ADJUSTMENT (w)->value;
  gnome_property_box_changed (GNOME_PROPERTY_BOX (options.propwindow));
  return;
  data = NULL;
}

void
paint_radius_cb (GtkWidget * w, gpointer data)
{
  options.paint_radius = GTK_ADJUSTMENT (w)->value;
  gnome_property_box_changed (GNOME_PROPERTY_BOX (options.propwindow));
  return;
  data = NULL;
}

void
sharpen_factor_cb (GtkWidget * w, gpointer data)
{
  options.sharpen_factor = GTK_ADJUSTMENT (w)->value;
  gnome_property_box_changed (GNOME_PROPERTY_BOX (options.propwindow));
  return;
  data = NULL;
}

void
solarize_factor_cb (GtkWidget * w, gpointer data)
{
  options.solarize_factor = GTK_ADJUSTMENT (w)->value;
  gnome_property_box_changed (GNOME_PROPERTY_BOX (options.propwindow));
  return;
  data = NULL;
}

void
spread_radius_cb (GtkWidget * w, gpointer data)
{
  options.spread_radius = GTK_ADJUSTMENT (w)->value;
  gnome_property_box_changed (GNOME_PROPERTY_BOX (options.propwindow));
  return;
  data = NULL;
}

void
blur_cb (GtkWidget * w, gpointer data)
{
  options.blur = GTK_TOGGLE_BUTTON (w)->active;
  gnome_property_box_changed (GNOME_PROPERTY_BOX (options.propwindow));
  return;
  data = NULL;
}

void
charcoal_cb (GtkWidget * w, gpointer data)
{
  options.charcoal = GTK_TOGGLE_BUTTON (w)->active;
  gnome_property_box_changed (GNOME_PROPERTY_BOX (options.propwindow));
  return;
  data = NULL;
}

void
edge_cb (GtkWidget * w, gpointer data)
{
  options.edge = GTK_TOGGLE_BUTTON (w)->active;
  gnome_property_box_changed (GNOME_PROPERTY_BOX (options.propwindow));
  return;
  data = NULL;
}

void
implode_cb (GtkWidget * w, gpointer data)
{
  options.implode = GTK_TOGGLE_BUTTON (w)->active;
  gnome_property_box_changed (GNOME_PROPERTY_BOX (options.propwindow));
  return;
  data = NULL;
}

void
paint_cb (GtkWidget * w, gpointer data)
{
  options.paint = GTK_TOGGLE_BUTTON (w)->active;
  gnome_property_box_changed (GNOME_PROPERTY_BOX (options.propwindow));
  return;
  data = NULL;
}

void
solarize_cb (GtkWidget * w, gpointer data)
{
  options.solarize = GTK_TOGGLE_BUTTON (w)->active;
  gnome_property_box_changed (GNOME_PROPERTY_BOX (options.propwindow));
  return;
  data = NULL;
}

void
swirl_cb (GtkWidget * w, gpointer data)
{
  options.swirl = GTK_TOGGLE_BUTTON (w)->active;
  gnome_property_box_changed (GNOME_PROPERTY_BOX (options.propwindow));
  return;
  data = NULL;
}

void
spread_cb (GtkWidget * w, gpointer data)
{
  options.spread = GTK_TOGGLE_BUTTON (w)->active;
  gnome_property_box_changed (GNOME_PROPERTY_BOX (options.propwindow));
  return;
  data = NULL;
}

void
spurious_cb (GtkWidget * w, gpointer data)
{
  options.spurious = GTK_TOGGLE_BUTTON (w)->active;
  if (options.spurious)
    {
      gtk_widget_show_all (options.spurious_pref_vbox);
      gtk_widget_show_all (options.spurious_pref_vbox2);
      gtk_widget_show_all (options.spurious_pref_vbox3);
    }
  else
    {
      gtk_widget_hide (options.spurious_pref_vbox);
      gtk_widget_hide (options.spurious_pref_vbox2);
      gtk_widget_hide (options.spurious_pref_vbox3);
    }
  gnome_property_box_changed (GNOME_PROPERTY_BOX (options.propwindow));
  return;
  data = NULL;
}
