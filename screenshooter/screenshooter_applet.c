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
#include <libgnomeui/gnome-window-icon.h>

void showHelp (AppletWidget * applet, gpointer data);

void
cb_about (AppletWidget * widget, gpointer data)
{
  static GtkWidget *about = NULL;
  GtkWidget *my_url;
  static const char *authors[] =
    { "Tom Gilbert <gilbertt@tomgilbert.freeserve.co.uk>",
    "Telsa Gwynne <hobbit@aloss.ukuu.org.uk> (typo fixes, documentation)",
    "The Sirius Cybernetics Corporation <Ursa-minor>", NULL
  };

  if (about != NULL)
  {
  	gdk_window_show(about->window);
	gdk_window_raise(about->window);
	return;	
  }

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
  gtk_signal_connect( GTK_OBJECT(about), "destroy",
		      GTK_SIGNAL_FUNC(gtk_widget_destroyed), &about );

  my_url = gnome_href_new ("http://www.linuxbrit.co.uk/",
			   _("Visit the author's Website"));
  gtk_widget_show (my_url);
  gtk_box_pack_start (GTK_BOX ((GNOME_DIALOG (about))->vbox), my_url, FALSE,
		      FALSE, 0);

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
  gnome_config_set_string ("screenshooter/app", options.app);
  gnome_config_set_bool ("screenshooter/decoration", options.decoration);
  gnome_config_set_bool ("screenshooter/monochrome", options.monochrome);
  gnome_config_set_bool ("screenshooter/negate", options.negate);
  gnome_config_set_bool ("screenshooter/view", options.view);
  gnome_config_set_bool ("screenshooter/equalize", options.equalize);
  gnome_config_set_bool ("screenshooter/normalize", options.normalize);
  gnome_config_set_bool ("screenshooter/beep", options.beep);
  gnome_config_set_bool ("screenshooter/frame", options.frame);
  gnome_config_set_bool ("screenshooter/thumb", options.thumb);
  gnome_config_set_bool ("screenshooter/flip", options.flip);
  gnome_config_set_bool ("screenshooter/flop", options.flop);
  gnome_config_set_bool ("screenshooter/despeckle", options.despeckle);
  gnome_config_set_bool ("screenshooter/enhance", options.enhance);
  gnome_config_set_bool ("screenshooter/use_script", options.use_script);
  gnome_config_set_bool ("screenshooter/emboss", options.emboss);
  gnome_config_set_bool ("screenshooter/blur", options.blur);
  gnome_config_set_bool ("screenshooter/charcoal", options.charcoal);
  gnome_config_set_bool ("screenshooter/edge", options.edge);
  gnome_config_set_bool ("screenshooter/implode", options.implode);
  gnome_config_set_bool ("screenshooter/paint", options.paint);
  gnome_config_set_bool ("screenshooter/spurious", options.spurious);
  gnome_config_set_bool ("screenshooter/solarize", options.solarize);
  gnome_config_set_bool ("screenshooter/spread", options.spread);
  gnome_config_set_bool ("screenshooter/swirl", options.swirl);
  gnome_config_set_bool ("screenshooter/thumbnail_intermediate",
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
	      ("screenshooter/filename=`date +%Y_%m_%d_%H%M%S`_shot.jpg"));
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

gboolean need_to_change_orientation (PanelOrientType o, gboolean size_tiny)
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
  int panelsize = applet_widget_get_panel_pixel_size (APPLET_WIDGET (applet));
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
  static GnomeHelpMenuEntry helpEntry = { NULL, "preferences" };
  GtkWidget *frame;
  GtkWidget *pref_vbox;
  GtkWidget *pref_vbox_2;
  GtkWidget *pref_hbox;
  GtkWidget *label;
  GtkWidget *button;
  GtkWidget *entry;


  if (ad->propwindow)
    {
      gdk_window_raise (ad->propwindow->window);
      return;
    }

  helpEntry.name = gnome_app_id;

  /* Grab old property settings for comparison */
  memcpy (&old_options, &options, sizeof (user_preferences));

  ad->propwindow = gnome_property_box_new ();
  gtk_window_set_title (GTK_WINDOW
			(&GNOME_PROPERTY_BOX (ad->propwindow)->dialog.window),
			_ ("Screen-Shooter Settings"));

  pref_vbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
  gtk_container_set_border_width (GTK_CONTAINER (pref_vbox), GNOME_PAD_SMALL);

  create_bool_option (_
		      ("Capture WM decorations when grabbing a window"),
		      &(ad->decoration), pref_vbox);

  create_bool_option (_
		      ("Give audio feedback using the keyboard bell"),
		      &(ad->beep), pref_vbox);

  button =
    gtk_check_button_new_with_label (_
				     ("Display Spurious Options (I got carried away)"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), ad->spurious);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      (GtkSignalFunc) spurious_cb, &(ad->spurious));
  gtk_box_pack_start (GTK_BOX (pref_vbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  create_slider_option (_
			("Delay (seconds) before taking shot "
			 "(only for entire desktop shots)"),
			pref_vbox, &(ad->delay), 0.0, 60.0, 1.0, 1.0, 0.0);

  create_slider_option (_
			("Compressed Quality (JPEG/MIFF/PNG mode) "
			 "High: good quality/large file"),
			pref_vbox, &(ad->quality), 0.0, 100.0, 10.0, 1.0,
			0.0);

  ad->spurious_pref_vbox3 = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
  gtk_container_set_border_width (GTK_CONTAINER (ad->spurious_pref_vbox3), 0);
  gtk_box_pack_start (GTK_BOX (pref_vbox), ad->spurious_pref_vbox3, FALSE,
		      FALSE, 0);

  create_bool_option (_
		      ("Create monochrome image"),
		      &(ad->monochrome), pref_vbox);

  create_bool_option (_ ("Invert colours in image"),
		      &(ad->negate), pref_vbox);

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
  gnome_file_entry_set_directory (GNOME_FILE_ENTRY (ad->directory_entry),
				  TRUE);
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
  label = gtk_label_new (_ ("Image filename:"));
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

  button = gtk_button_new_with_label (_ ("Show expanded filename"));
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      (GtkSignalFunc) expand_cb, NULL);
  gtk_box_pack_start (GTK_BOX (pref_hbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

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
  /* The % sign screws over xgettext ;) */

  /* xgettext:no-c-format */
  label = gtk_label_new (_("Filetype is determined from filename suffix.\n"
			  "Default filetype (if no extension or unrecognised extension) "
			  "is miff.\n"
			  "Recognised suffixes are:\n"
			  "jpg, jpeg, bmp, cgm, cmyk, dib, eps, fig, fpx, gif, ico, map, miff, pcx,\n"
			  "pict, pix, png, pnm, ppm, psd, rgb, rgba, rla, rle, tiff, tga, xbm, xpm,\n"
			  "and several obscure ones. Try your luck with something interesting!\n"
			  "Eg .html for a client-side image map. You'll get a .gif, .html, and a .shtml\n\n"
			  "Recommendations:\n"
			  "Small file, pretty good quality:   jpg, 75% quality\n"
			  "Slightly larger file but lossless: png, as much compression as you like.\n"
			  "- compressing pngs takes longer at higher ratios, but is still lossless"));
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
  gtk_box_pack_start (GTK_BOX (pref_hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  create_bool_option (_ ("View screenshot after saving it"),
		      &(ad->view), pref_vbox);

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

  create_bool_option (_ ("Create thumbnail of image too"),
		      &(ad->thumb), pref_vbox);

  create_slider_option (_
			("Thumbnail Size (percentage of original)"),
			pref_vbox, &(ad->thumb_size), 1.0, 100.0, 10.0, 1.0,
			0.0);

  create_slider_option (_
			("Thumbnail compression (JPEG/MIFF/PNG mode) "
			 "High: good quality/large file"),
			pref_vbox, &(ad->thumb_quality), 0.0, 100.0, 10.0,
			1.0, 0.0);


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

  create_bool_option (_ ("Use high-quality intermediate "
			 "for generating thumbnail"),
		      &(ad->thumbnail_intermediate), pref_vbox_2);

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

  create_bool_option (_ ("Normalize image (Span full range of "
			 "color values to enhance contrast)"),
		      &(ad->normalize), pref_vbox);

  create_bool_option (_ ("Equalize image (Perform "
			 "histogram equalization)"),
		      &(ad->equalize), pref_vbox);

  pref_hbox = gtk_hbox_new (FALSE, GNOME_PAD_SMALL);
  gtk_box_pack_start (GTK_BOX (pref_vbox), pref_hbox, FALSE, FALSE, 0);
  gtk_widget_show (pref_hbox);

  create_bool_option (_ ("Enhance image (reduce noise)"),
		      &(ad->enhance), pref_hbox);

  create_bool_option (_ ("Despeckle Image (reduce spots)"),
		      &(ad->despeckle), pref_hbox);

  create_slider_option (_
			("Sharpen Image by what factor?"),
			pref_vbox, &(ad->sharpen_factor), 0.0, 100.0, 10.0,
			1.0, 0.0);

  create_slider_option (_
			("Rotate image clockwise by how many degrees?"),
			pref_vbox, &(ad->rotate_degrees), 0.0, 360.0, 10.0,
			1.0, 0.0);


  create_bool_option (_ ("Adjust gamma"), &(ad->gamma), pref_vbox);

  create_slider_option (_
			("Gamma value"),
			pref_vbox, &(ad->gamma), 0.8, 2.3, 0.1, 0.1, 0.0);

  pref_hbox = gtk_hbox_new (FALSE, GNOME_PAD_SMALL);
  gtk_box_pack_start (GTK_BOX (pref_vbox), pref_hbox, FALSE, FALSE, 0);
  gtk_widget_show (pref_hbox);

  label = gtk_label_new (_ ("Post Processing"));
  gtk_widget_show_all (pref_vbox);
  gnome_property_box_append_page (GNOME_PROPERTY_BOX (ad->propwindow),
				  pref_vbox, label);

  pref_vbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
  gtk_container_set_border_width (GTK_CONTAINER (pref_vbox), GNOME_PAD_SMALL);

  create_bool_option (_ ("Create frame around image"),
		      &(ad->frame), pref_vbox);

  create_slider_option (_
			("Frame Width (pixels)"),
			pref_vbox, &(ad->frame_size), 1.0, 50.0, 10.0, 1.0,
			0.0);

  pref_hbox = gtk_hbox_new (FALSE, GNOME_PAD_SMALL);
  gtk_box_pack_start (GTK_BOX (pref_vbox), pref_hbox, FALSE, FALSE, 0);
  gtk_widget_show (pref_hbox);

  create_bool_option (_ ("Mirror image vertically"), &(ad->flip), pref_hbox);

  create_bool_option (_ ("Mirror image horizontally"),
		      &(ad->flop), pref_hbox);

  create_bool_option (_ ("Emboss image"), &(ad->emboss), pref_vbox);

  frame = gtk_frame_new (NULL);
  gtk_box_pack_start (GTK_BOX (pref_vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  pref_vbox_2 = gtk_vbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (pref_vbox_2),
				  GNOME_PAD_SMALL);
  gtk_container_add (GTK_CONTAINER (frame), pref_vbox_2);
  gtk_widget_show (pref_vbox_2);

  create_bool_option (_ ("When finished, send image and "
			 "thumbnail filenames to script/program below"),
		      &(ad->use_script), pref_vbox_2);

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

  create_bool_option (_ ("Blur image"), &(ad->blur), ad->spurious_pref_vbox);

  create_slider_option (_
			("Blur image by what factor?"),
			ad->spurious_pref_vbox, &(ad->blur_factor), 0.0,
			100.0, 10.0, 1.0, 0.0);

  create_bool_option (_ ("Create Charcoal Effect"),
		      &(ad->charcoal), ad->spurious_pref_vbox);

  create_slider_option (_
			("Charcoal by what factor?"),
			ad->spurious_pref_vbox, &(ad->charcoal_factor),
			0.0, 100.0, 10.0, 1.0, 0.0);

  create_bool_option (_ ("Find Edges in Image"),
		      &(ad->edge), ad->spurious_pref_vbox);

  create_slider_option (_
			("Find edges by what factor?"),
			ad->spurious_pref_vbox, &(ad->edge_factor),
			0.0, 100.0, 10.0, 1.0, 0.0);

  create_bool_option (_ ("Implode Image"),
		      &(ad->implode), ad->spurious_pref_vbox);

  create_slider_option (_
			("Implode by what factor?"),
			ad->spurious_pref_vbox, &(ad->implode_factor),
			0.0, 100.0, 10.0, 1.0, 0.0);


  label = gtk_label_new (_ ("Spurious"));
  gnome_property_box_append_page (GNOME_PROPERTY_BOX (ad->propwindow),
				  ad->spurious_pref_vbox, label);

  ad->spurious_pref_vbox2 = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
  gtk_container_set_border_width (GTK_CONTAINER (ad->spurious_pref_vbox2),
				  GNOME_PAD_SMALL);

  create_bool_option (_ ("Create Painted Effect"),
		      &(ad->paint), ad->spurious_pref_vbox2);

  create_slider_option (_
			("Paint what radius around each pixel?"),
			ad->spurious_pref_vbox2, &(ad->paint_radius),
			0.0, 20.0, 1.0, 1.0, 0.0);

  create_bool_option (_ ("Solarise Image"),
		      &(ad->solarize), ad->spurious_pref_vbox2);

  create_slider_option (_
			("Solarize factor?"),
			ad->spurious_pref_vbox2, &(ad->solarize_factor),
			0.0, 100.0, 10.0, 1.0, 0.0);

  create_bool_option (_ ("Spread image pixels"),
		      &(ad->spread), ad->spurious_pref_vbox2);

  create_slider_option (_
			("Radius to around each pixel to spread?"),
			ad->spurious_pref_vbox2, &(ad->spread_radius),
			0.0, 20.0, 1.0, 1.0, 0.0);

  create_bool_option (_ ("Swirl pixels. My favorite :-)"),
		      &(ad->swirl), ad->spurious_pref_vbox2);

  create_slider_option (_
			("Radius to swirl pixels around?)"),
			ad->spurious_pref_vbox2, &(ad->swirl_degrees),
			0.0, 360.0, 10.0, 1.0, 0.0);

  label = gtk_label_new (_ ("Spurious 2"));
  gnome_property_box_append_page (GNOME_PROPERTY_BOX (ad->propwindow),
				  ad->spurious_pref_vbox2, label);

  gtk_signal_connect (GTK_OBJECT (ad->propwindow), "apply",
		      GTK_SIGNAL_FUNC (property_apply_cb), ad);
  gtk_signal_connect (GTK_OBJECT (ad->propwindow), "destroy",
		      GTK_SIGNAL_FUNC (property_destroy_cb), ad);
  gtk_signal_connect (GTK_OBJECT (ad->propwindow), "help",
		      GTK_SIGNAL_FUNC (gnome_help_pbox_display), &helpEntry);

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
  gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/screenshooter_applet.png");

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

  gtk_signal_connect (GTK_OBJECT (applet), "change_orient",
		      GTK_SIGNAL_FUNC (cb_applet_change_orient), NULL);

#ifdef HAVE_PANEL_PIXEL_SIZE
  gtk_signal_connect (GTK_OBJECT (applet), "change_pixel_size",
		      GTK_SIGNAL_FUNC (applet_change_pixel_size), NULL);
#endif

  gtk_signal_connect (GTK_OBJECT (applet), "save_session",
		      GTK_SIGNAL_FUNC (applet_save_session), NULL);

  applet_widget_add (APPLET_WIDGET (applet), mainbox);

  orient = applet_widget_get_panel_orient (APPLET_WIDGET (applet));
#ifdef HAVE_PANEL_PIXEL_SIZE
  init_size = applet_widget_get_panel_pixel_size (APPLET_WIDGET (applet));
  if (init_size < PIXEL_SIZE_STANDARD)
    init_size_tiny = TRUE;
#endif
  change_orientation (orient, init_size_tiny);

  applet_widget_register_stock_callback (APPLET_WIDGET (applet),
					 "properties",
					 GNOME_STOCK_MENU_PROP,
					 _ ("Properties..."),
					 (AppletCallbackFunc)
					 cb_properties_dialog, NULL);
  applet_widget_register_stock_callback (APPLET_WIDGET (applet),
					 "help",
					 GNOME_STOCK_PIXMAP_HELP,
					 _ ("Help"),
					 (AppletCallbackFunc) showHelp, NULL);
  applet_widget_register_stock_callback (APPLET_WIDGET (applet),
					 "about",
					 GNOME_STOCK_MENU_ABOUT,
					 _ ("About..."),
					 (AppletCallbackFunc) cb_about, NULL);

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
  gchar *sys = NULL;
  gchar qual_buf[10];
  gchar thumb_qual_buf[10];
  gchar *dir_buf = NULL;
  gchar *file_buf = NULL;
  gchar *temp_dir_file_buf = NULL;
  gchar dec_buf[10];
  gchar mono_buf[25];
  gchar neg_buf[15];
  gchar beep_buf[10];
  gchar delay_buf[15];
  gchar *thumb_image = NULL;
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
  system (sys);

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
	  cmd = g_strdup_printf ("%s \"%s\" \"%s\" &", opt->app, lastimage, thumb_image);
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
expand_cb (GtkWidget * w, gpointer data)
{
  wordexp_t mywordexp;
  int wordexpret = 0;
  gchar *filename = NULL;
  gchar *message = NULL;
  gchar *buf = NULL;

  filename = gtk_entry_get_text (GTK_ENTRY (options.filename_entry));

  if ((wordexpret = wordexp (filename, &mywordexp, 0) != 0))
    {
      gnome_ok_dialog (_("There was a word expansion error\n"
			  "I think you have stuck something funny in the filename box"));
      wordfree (&mywordexp);
      return;
    }
  else
    {
      buf = g_strjoinv ("", mywordexp.we_wordv);

      /* display expanded filename */
      message = g_strdup_printf
	(_
	 ("The specified filename:\n%s\nexpands to:\n%s\nwhen passed to a shell"),
	 filename, buf);
      gnome_ok_dialog (message);
      wordfree (&mywordexp);
    }
  return;
  data = NULL;
  w = NULL;
}

void
property_apply_cb (GtkWidget * w, gpointer data)
{
  user_preferences *ad = &options;
  user_preferences *old = &old_options;
  gchar *buf;
  gint info_changed = FALSE;

  buf =
    gtk_entry_get_text (GTK_ENTRY
			(GTK_COMBO
			 (GNOME_FILE_ENTRY (ad->directory_entry)->gentry)->
			 entry));

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

gint property_destroy_cb (GtkWidget * w, gpointer data)
{
  options.propwindow = NULL;
  return FALSE;
  w = NULL;
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

void
slider_option_cb (GtkWidget * w, gpointer data)
{
  int *option = data;
  *option = GTK_ADJUSTMENT (w)->value;
  gnome_property_box_changed (GNOME_PROPERTY_BOX (options.propwindow));
  return;
  data = NULL;
}

void
boolean_option_cb (GtkWidget * w, gpointer data)
{
  int *option = data;
  *option = GTK_TOGGLE_BUTTON (w)->active;
  gnome_property_box_changed (GNOME_PROPERTY_BOX (options.propwindow));
  return;
  data = NULL;
}

GtkWidget *
create_bool_option (const gchar * label, int *opt, GtkWidget * target)
{
  GtkWidget *button = gtk_check_button_new_with_label (label);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), *opt);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      (GtkSignalFunc) boolean_option_cb, opt);
  gtk_box_pack_start (GTK_BOX (target), button, FALSE, FALSE, 0);
  gtk_widget_show (button);
  return button;
}


GtkWidget *
create_slider_option (gchar * label, GtkWidget * target, int *option,
		      gfloat a, gfloat b, gfloat c, gfloat d, gfloat e)
{
  GtkWidget *pref_vbox, *hscale;
  GtkObject *adj;
  GtkWidget *frame = gtk_frame_new (label);
  gtk_box_pack_start (GTK_BOX (target), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  pref_vbox = gtk_hbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (pref_vbox), GNOME_PAD_SMALL);
  gtk_container_add (GTK_CONTAINER (frame), pref_vbox);
  gtk_widget_show (pref_vbox);

  adj = gtk_adjustment_new (*option, a, b, c, d, e);
  hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_range_set_update_policy (GTK_RANGE (hscale), GTK_UPDATE_DELAYED);
  gtk_scale_set_digits (GTK_SCALE (hscale), 0);
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		      GTK_SIGNAL_FUNC (slider_option_cb), option);
  gtk_box_pack_start (GTK_BOX (pref_vbox), hscale, TRUE, TRUE,
		      GNOME_PAD_SMALL);
  gtk_widget_show (hscale);
  return frame;
}

void
showHelp (AppletWidget * applet, gpointer data)
{
  static GnomeHelpMenuEntry help_entry = { NULL, "index.html" };

  help_entry.name = gnome_app_id;

  gnome_help_display (NULL, &help_entry);
  return;
  applet = NULL;
  data = NULL;
}
