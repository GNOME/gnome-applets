/*
 * GNOME odometer panel applet
 * (C) 1999 The Free software Foundation
 * 
 * Author : Fabrice Bellet <Fabrice.Bellet@creatis.insa-lyon.fr>
 *          adapted from kodo/Xodometer/Mouspedometa
 *	
 */

/*
 * MouspedometMouspedometaa
 *      Based on the original Xodometer VMS/Motif sources.
 *
 * Written by Armen Nakashian
 *            Compaq Computer Corporation
 *            Houston TX
 *            22 May 1998
 *
 * If you make improvements or enhancements to Mouspedometa, please send
 * them back to the author at any of the following addresses:
 *
 *              armen@nakashian.com
 *
 * Thanks to Mark Granoff for writing the original Xodometer, and
 * the whole KDE team for making such a nice environment to write
 * programs in.
 *
 *
 * This software is provided as is with no warranty of any kind,
 * expressed or implied. Neither Digital Equipment Corporation nor
 * Armen Nakashian will be held accountable for your use of this
 * software.
 */

/*	 
**  XodometeXodometerr
**  Written by Mark H. Granoff/mhg
**             Digital Equipment Corporation
**             Littleton, MA, USA
**             17 March 1993
**  
**  If you make improvements or enhancements to Xodometer, please send them
**  back to the author at any of the following addresses:
**
**		granoff@keptin.lkg.dec.com
**		granoff@UltraNet.com
**		72301.1177@CompuServe.com
**
**
**  Thanks to my friend and colleague Bob Harris for his suggestions and help.
**  
**  This software is provided as is with no warranty of any kind, expressed or
**  implied. Neither Digital Equipment Corporation nor Mark Granoff will be
**  held accountable for your use of this software.
**  
**  This software is released into the public domain and may be redistributed
**  freely provided that all source module headers remain intact in their
**  entirety, and that all components of this kit are redistributed together.
**  
**  Modification History
**  --------------------
**
**  19 Mar 1993	mhg	v1.0	Initial release.
**   3 Aug 1993 mhg	v1.2	Added automatic trip reset feature.
**   5 Jan 1994 mhg	v1.3	Ported to Alpha; moved mi/km button into popup
**				menu; removed 'Trip' fixed label; added font
**				resource.
**   6 Jan 1994 mhg	x1.4	Main window no longer grabs input focus when
**				realized.
**  21 Nov 1994	mhg	x1.4	Added saveFile resource to make location and
**				name of odometer save file customizable.
**   6 Mar 1995 mhg	x1.5	Added automatic html generation.
**   9 Mar 1995 mhg	x1.6	Converted file to format with verion info!
**				Changed .format resource to .units.
**				Improved measurement unit handling and update
**				accuracy. Removed unneccesary label updates.
**
**  19 Apr 1995 mhg	V2.0	Removed OpenVMS-specific AST code in favor of
**				XtAppAddTimeout. (Works a lot better now, too.)
**  26 Apr 1995 mhg	X2.1-1  Fix trip reset so units is also reset.
**  27 Apr 1995 mhg	X2.1-2  Changed auto_reset timer to be its own timeout
**				on a relative timer based on seconds until
**				midnight.
**  28 Apr 1995	mhg	X2.1-3  Added pollInterval and saveFrequency resources.
**   1 May 1995 mhg	X2.1-4	Make disk writing (data, html) deferred so as
**				to allow for "continuous" display update while
**				mouse is in motion.
**  27 Oct 1995 mhg	V2.1	Final cleanup for this version and public
**                              release.
**
**
**  18 May 1998 asn     V3.0    Broke the code in all sorts of ways to turn
**                              the mild-mannered Motif version into a
**				modern KDE program.  Code turned into C++,
**				all Xm references were removed, and VMS
**				support was dumped!
**
** 22 Aug 1998 asn      V3.1    Minor updates to make it compile more 
**                              gracefully on modern C++ compilers,
**                              and updated things to KDE 1.0 specs.
**                              Added a proper About Box.
*/

#include <ctype.h>		/* needed for isdigit() */
#include <math.h>
#include <config.h>
#include <locale.h>             /* needed for setlocale () */
#include "odo.h"
#include <libgnomeui/gnome-window-icon.h>

static conversionEntry conversion_table[MAX_UNIT] = {
{ INCH, "inch", "inches", 12.0,   2.54,     "cm",     "cm",     100.0,  2 },
{ FOOT, "foot", "feet",   5280.0, 0.3048,   "meter",  "meters", 1000.0, 1 },
{ MILE, "mile", "miles",  -1.0,   1.609344, "km",     "km",     -1.0,   1 }
};

static void
reset_cb (AppletWidget *widget, gpointer data)
{
   OdoApplet *oa = data;
   oa->trip_distance=0.;
   refresh(oa);
   return;
   widget = NULL;
}

static void
about_cb (AppletWidget *widget, gpointer data)
{
   static GtkWidget *about = NULL;

   const gchar *authors[] = {
   	"Fabrice Bellet <Fabrice.Bellet@creatis.insa-lyon.fr>",NULL};

   if (about != NULL)
   {
	gdk_window_show(about->window);
	gdk_window_raise(about->window);
	return;
   }

   about=gnome_about_new (_("Odometer"),
   	ODO_VERSION,
   	_("(C) 1999 The Free Software Foundation"),
   	authors,
   	_("a GNOME applet that tracks and measures the movements "
   	"of your mouse pointer across the desktop."),
   	NULL);
   gtk_signal_connect( GTK_OBJECT(about), "destroy",
		       GTK_SIGNAL_FUNC(gtk_widget_destroyed), &about );
   gtk_widget_show (about);
   return;
   widget = NULL;
   data = NULL;
}

static void
properties_save (gchar *path, OdoApplet *oa)
{
#ifdef DEBUG
   printf ("properties_save\nTrip=%f\ndistance=%f\n",
   	oa->trip_distance,oa->distance);
#endif
   gnome_config_push_prefix (path);
   gnome_config_set_bool ("odometer/use_metric",oa->use_metric);
   gnome_config_set_bool ("odometer/auto_reset",oa->auto_reset);
   gnome_config_set_bool ("odometer/enabled",oa->enabled);
   gnome_config_set_int ("odometer/digits_nb",oa->digits_nb);
   gnome_config_set_bool ("odometer/scale_applet",oa->scale_applet);
   gnome_config_set_float ("odometer/trip_distance",oa->trip_distance);
   gnome_config_set_float ("odometer/distance",oa->distance);
   gnome_config_set_string ("odometer/theme_file",oa->theme_file);
   gnome_config_sync ();
   gnome_config_drop_all ();
   gnome_config_pop_prefix ();
}

static void
properties_load (gchar *path,OdoApplet *oa)
{
   gnome_config_push_prefix (path);
   oa->use_metric=gnome_config_get_bool_with_default ("odometer/use_metric=true",NULL);
   oa->auto_reset=gnome_config_get_bool_with_default ("odometer/auto_reset=true",NULL);
   oa->enabled=gnome_config_get_bool_with_default ("odometer/enabled=true",NULL);
   oa->digits_nb=gnome_config_get_int_with_default ("odometer/digits_nb=5",NULL);
   oa->scale_applet=gnome_config_get_bool_with_default ("odometer/scale_applet=true",NULL);
   oa->trip_distance=gnome_config_get_float_with_default ("odometer/trip_distance=0",NULL);
   oa->distance=gnome_config_get_float_with_default ("odometer/distance=0",NULL);

   oa->theme_file=gnome_config_get_string("odometer/theme_file");
#ifdef DEBUG
   printf ("properties_load\nTrip=%f\ndistance=%f\n",
   	oa->trip_distance,oa->distance);
#endif
   gnome_config_pop_prefix ();
   if (oa->auto_reset) oa->trip_distance=0;
#ifdef HAVE_PANEL_PIXEL_SIZE
   oa->orient = applet_widget_get_panel_orient (APPLET_WIDGET(oa->applet));
   oa->size   = applet_widget_get_panel_pixel_size (APPLET_WIDGET(oa->applet));
#else
   oa->orient = ORIENT_UP;
   oa->size   = SIZEHINT_DEFAULT;
#endif
}

/*
 * Save session callback. Just save the current distance in the properties
 * stuff
 */
static gint
save_session_cb (GtkWidget *widget,
	gchar *privcfgpath,
	gchar *globcfgpath,
	gpointer data)
{
   OdoApplet *oa = data;
#ifdef DEBUG
   g_print ("save_session_cb\n");
#endif
   properties_save (privcfgpath,oa);
   return FALSE;
   widget = NULL;
   globcfgpath = NULL;
}

static void
delete_cb (GtkWidget *widget,GdkEvent *event, gpointer data)
{
#ifdef DEBUG
   g_print ("delete_cb\n");
#endif
   gtk_main_quit();
   return;
   widget = NULL;
   event = NULL;
   data = NULL;
}

gint
change_digits_nb (OdoApplet *oa)
{
  gint aw,ah;

  aw=oa->digit_width*oa->digits_nb;
  ah=oa->digit_height;
  gtk_drawing_area_size(GTK_DRAWING_AREA(oa->darea1),aw,ah);
  gtk_drawing_area_size(GTK_DRAWING_AREA(oa->darea2),aw,ah);
  return TRUE;
}

/*
 * This procedure update the global variables distance and
 * trip_distance. These values are kept (and saved) as inches,
 * and will be converted to a more convenient unit or in
 * metric values if needed.
 * 
 * Return value : true if the mouse moved since previous call.
 */
static gboolean
Calcdistance(OdoApplet *oa)
{
   gdouble X,Y;
   gdouble distMM, distInches;

   if (oa->last_x_coord == 0 && oa->last_y_coord == 0)
      return FALSE;
   if (oa->last_x_coord == oa->x_coord &&
   	oa->last_y_coord == oa->y_coord)
      return FALSE;
   X = (oa->x_coord - oa->last_x_coord)/oa->h_pixels_per_mm;
   Y = (oa->y_coord - oa->last_y_coord)/oa->v_pixels_per_mm;
   distMM=sqrt (X*X+Y*Y);
   distInches = distMM * 0.04;
   oa->distance += distInches;
   oa->trip_distance += distInches;
   return TRUE;
}

/*
 * Draw the Nth digit from the pixmap on the drawing area
 * at the location defined by x.
 */
static void
draw_digit(GtkWidget *darea,gint n,gint x,gint image_number,OdoApplet *oa)
{
   gdk_draw_pixmap (darea->window,darea->style->fg_gc[GTK_WIDGET_STATE(darea)],
   	oa->image[image_number]->pixmap,
   	n*oa->digit_width,0,
   	x*oa->digit_width,0,
   	oa->digit_width,oa->digit_height-1);
}

/*
 * This procedure prints the value given in string.
 * Only the first oa->digits_nb chars are printed. '.' and '-' is
 * parsed. We are not supposed to encounter something else here.
 */
static void
draw_value(GtkWidget *darea,gchar *string,OdoApplet *oa)
{
   gushort i;
   gushort nb_drawn_digits;
   gushort n;
   /*
    * this value is used to distinguish digits from the integer part
    * of the value to digits from the decimal part.
    */
   gint image_number = INTEGER;

   for (i=0, nb_drawn_digits=0; nb_drawn_digits<oa->digits_nb; i++) {
   	if (string[i] == '-')
   		n=0;
	else if (string[i] == '.') {
		if (oa->with_decimal_dot)
			n=10;
		else {
			image_number = DECIMAL;
			continue;
		}
	} else if (isdigit (string[i]))
		n=string[i]-'0';
	else 
		break;
	draw_digit (darea,n,nb_drawn_digits++,image_number,oa);
   }
}

/*
 * this procedure converts the distance in_dist (in inches)
 * to the most relevant unit. Metric conversion is achieved
 * if needed.
 *
 * Return value : the unit name as a  pointer to the corresponding 
 * string in conversion_table[].
 */
static Units
format_distance(gdouble in_dist,gchar **tag,gchar *string,OdoApplet *oa)
{
   gchar *string2;
   gdouble out_dist;
   gint precision;
   gint digits_nb;
   Units unit=INCH;
   gushort l;

   /* 
    * FIXME: Remove this declaration after transition to Gnome 2.0.
    * More information below in the call to g_snprintf.
    */
   gchar *temp_locale_str;

   if (oa->use_metric)
   {
   	/*
   	 * do metric conversion
   	 */
   	out_dist=in_dist * conversion_table[INCH].conversion_factor;
   	/*
   	 * convert the out_dist value, until the correct unit is
   	 * found
   	 */
   	while (out_dist > conversion_table[unit].max_to_before_next &&
   		conversion_table[unit].max_to_before_next != -1.0)
   	   out_dist = out_dist/conversion_table[unit++].max_to_before_next;
    	if (out_dist == 1.0)
    		*tag = conversion_table[unit].to_unit_tag;
    	else
    		*tag = conversion_table[unit].to_unit_tag_plural;
   }
   else
   {
   	out_dist=in_dist;
   	while (out_dist > conversion_table[unit].max_from_before_next &&
   		conversion_table[unit].max_from_before_next != -1.0)
   	   out_dist = out_dist/conversion_table[unit++].max_from_before_next;
   	if (out_dist == 1.0)
   		*tag = conversion_table[unit].from_unit_tag;
	else
		*tag = conversion_table[unit].from_unit_tag_plural;
   }
   /*
    * format according to the number of digits after the decimal dot
    *
    * We need to add a supplemental digit to replace the missing dot,
    * when the dot is not shown.
    */
   if (oa->with_decimal_dot)
   	digits_nb=oa->digits_nb;
   else
   	digits_nb=oa->digits_nb+1;

   string2 = g_malloc0 (digits_nb+1);
   precision = conversion_table[unit].print_precision;

   /* 
    * We have to make the call to g_snprintf in the C locale to parse the
    * string correctly later.
    * FIXME: Whem we move to Gnome 2.0, it is better to use the functions
    *   gnome_i18n_push_c_numeric_locale ()
    *   gnome_i18n_pop_c_numeric_locale ()
    */
   temp_locale_str = g_strdup (setlocale (LC_NUMERIC, NULL));
   setlocale (LC_NUMERIC, "C");

   g_snprintf (string2,digits_nb+1,"%.*f", precision, out_dist);

   setlocale (LC_NUMERIC, temp_locale_str);
   g_free (temp_locale_str);

   l=strlen(string2);
   strcpy  (string+digits_nb-l,string2);
   g_free (string2);
   return unit;
}

/*
 * This procedure update the screen output of the applet.
 * Also uptime the tooltip that gives the unit names.
 * TODO : provide two different tooltips for both counters,
 * ie both drawing areas.
 */
void
refresh(OdoApplet *oa)
{
   gchar *distance_s;
   gchar *trip_s;
   GString *ttip;
   /*
    * unit names (for tooltip update)
    */
   static gchar *lastDTag=NULL;
   static gchar *lastTTag=NULL;
   gchar *DTag=NULL;
   gchar *TTag=NULL;

   distance_s = g_strnfill (oa->digits_nb+1, '0');
   trip_s = g_strnfill (oa->digits_nb+1, '0');
   if (oa->enabled) {
	oa->distance_unit = format_distance (oa->distance,&DTag,distance_s,oa);
	oa->trip_distance_unit = format_distance (oa->trip_distance,&TTag,trip_s,oa);
   } else {
	strcpy (distance_s,"-----");
	strcpy (trip_s,"-----");
   }

   /*
    * Pointer comparison is valid here, because they refer to strings
    * in the static struct conversionEntry conversion_table[]
    */
   if (oa->enabled && (lastDTag!=DTag || lastTTag!=TTag)) {
   	lastDTag=DTag;
   	lastTTag=TTag;
  	ttip=g_string_new(TTag);
  	g_string_prepend(ttip,"/");
  	g_string_prepend(ttip,DTag);
#ifdef DEBUG
   	printf ("%s\n",ttip->str);
#endif
   	applet_widget_set_tooltip (APPLET_WIDGET(oa->applet),ttip->str);
   	g_string_free(ttip,TRUE);
   }

   draw_value(oa->darea1,distance_s,oa);
   draw_value(oa->darea2,trip_s,oa);
   g_free (distance_s);
   g_free (trip_s);
}

/*
 * This procedure is called on timer expiration and evaluation
 * the mouse displacement since previous call. If the mouse position
 * changed, the output is updated.
 */
static gint
timer_cb(gpointer data)
{
   OdoApplet *oa = data;

   oa->last_x_coord=oa->x_coord;
   oa->last_y_coord=oa->y_coord;
   gdk_window_get_pointer (oa->darea1->window,
   	&oa->x_coord, &oa->y_coord, NULL);
   if (Calcdistance(oa))
   {
	refresh(oa);
	/*
	 * I used this hack to ensure that the properties are 
	 * frequentely saved, because they contain the current
	 * distance value. The "save_session" signal seems to be
	 * sent once.
	 */
	oa->cycles_since_last_save++;
	if (oa->cycles_since_last_save>100) {
		oa->cycles_since_last_save=0;
		properties_save (APPLET_WIDGET (oa->applet)->privcfgpath,oa);
	};
   }
   return TRUE;
}

static gint
darea_expose (GtkWidget *widget, GdkEventExpose *event,gpointer data)
{
   OdoApplet *oa = data;
#ifdef DEBUG
   g_print ("darea_expose event catched!\n");
#endif
   refresh(oa);
   return FALSE;
   widget = NULL;
   event = NULL;
}

static void
sized_render (OdoApplet *oa)
{
   change_theme (oa->theme_file, oa);
}

static void
odo_change_orient (GtkWidget *widget, PanelOrientType orient, gpointer data)
{
   OdoApplet *oa = data;

#ifdef DEBUG
   g_print ("odo_change_orient event catched :");
   switch (orient) {
	case ORIENT_LEFT : g_print ("ORIENT_LEFT\n"); break;
	case ORIENT_RIGHT : g_print ("ORIENT_RIGHT\n"); break;
	case ORIENT_UP : g_print ("ORIENT_UP\n"); break;
	case ORIENT_DOWN : g_print ("ORIENT_DOWN\n"); break;
	default : g_assert_not_reached();
   }
#endif

   /*
    * FIXME : why must I use applet_widget_get_panel_orient() here to
    * get the correct orientation ? The callback 2nd parameter is
    * always zero except at the first time this callback is called.
    */

   /* oa->orient = orient; */

   oa->orient = applet_widget_get_panel_orient
   	(APPLET_WIDGET (oa->applet));
   sized_render (oa);
   return;
   widget = NULL;
   orient = ORIENT_UP;
}

#ifdef HAVE_PANEL_PIXEL_SIZE
static void 
odo_change_pixel_size (GtkWidget *widget, int size, gpointer data)
{
   OdoApplet *oa = data;

#ifdef DEBUG
   g_print ("odo_change_pixel_size event catched!\n");
#endif
   oa->size = size;
   sized_render (oa);
   return;
   widget = NULL;
}
#endif

static void
create_odo(OdoApplet *oa)
{
   gint aw,ah;

   /*
    * Here we create two drawing areas for both counters
    * The area size depends on the digits size loaded from file.
    */
   aw=oa->digit_width*oa->digits_nb;
   ah=oa->digit_height;

   oa->darea1=gtk_drawing_area_new();
   gtk_drawing_area_size(GTK_DRAWING_AREA(oa->darea1),aw,ah);
   gtk_widget_show(oa->darea1);
   oa->darea2=gtk_drawing_area_new();
   gtk_drawing_area_size(GTK_DRAWING_AREA(oa->darea2),aw,ah);
   gtk_widget_show(oa->darea2);

   oa->vbox=gtk_vbox_new (FALSE,0);
   gtk_box_pack_start (GTK_BOX(oa->vbox),oa->darea1,FALSE,FALSE,1);
   gtk_box_pack_start (GTK_BOX(oa->vbox),oa->darea2,FALSE,FALSE,1);
   gtk_widget_show (oa->vbox);
   applet_widget_add(APPLET_WIDGET(oa->applet),oa->vbox);

   /*
    * Get screen size in millimeters. 
    * h_pixels_per_mm and v_pixels_per_mm can differ ??? 
    */
   oa->h_pixels_per_mm=(gfloat)gdk_screen_width()/gdk_screen_width_mm();
   oa->v_pixels_per_mm=(gfloat)gdk_screen_height()/gdk_screen_height_mm();
}

static void 
init_applet (OdoApplet *oa)
{
   oa->last_x_coord=0;
   oa->last_y_coord=0;
   oa->x_coord=0;
   oa->y_coord=0;
   oa->distance=0.;
   oa->trip_distance=0.;
   oa->last_distance=0.;

   oa->cycles_since_last_save=0;

   oa->distance_unit=INCH;
   oa->trip_distance_unit=INCH;

   oa->use_metric=TRUE;
   oa->enabled=TRUE;
   oa->auto_reset=TRUE;
   oa->scale_applet=TRUE;

   oa->p_use_metric=TRUE;
   oa->p_enabled=TRUE;
   oa->p_auto_reset=TRUE;
   oa->p_scale_applet=TRUE;

   oa->digits_nb=9;
   oa->p_digits_nb=9;

   oa->digit_width=10;
   oa->digit_height=20;

   oa->image[INTEGER]=NULL;
   oa->image[DECIMAL]=NULL;
   oa->theme_file=NULL;
   oa->theme_entry=NULL;
}

static void
help_cb (AppletWidget *applet, gpointer data)
{
    GnomeHelpMenuEntry help_entry = { "odometer_applet", "index.html"};
    gnome_help_display(NULL, &help_entry);
}

int
main (int argc, char *argv[])
{
   OdoApplet *oa;

   /* Internationalization stuff */
   bindtextdomain (PACKAGE, GNOMELOCALEDIR);
   textdomain (PACKAGE);

   oa = g_new0(OdoApplet,1);
   init_applet (oa);

   applet_widget_init ("odometer_applet", VERSION, argc, argv,
   	NULL,0,NULL);
   gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gnome-odometer.png");

   oa->applet=applet_widget_new ("odometer_applet");
   if (!oa->applet)
	g_error (_("Can't create odometer applet!"));

   gtk_signal_connect (GTK_OBJECT (oa->applet),
   	"save_session",GTK_SIGNAL_FUNC(save_session_cb),oa);
   gtk_signal_connect (GTK_OBJECT (oa->applet),
   	"delete_event",GTK_SIGNAL_FUNC(delete_cb),NULL);
   gtk_signal_connect (GTK_OBJECT (oa->applet),
   	"expose_event",GTK_SIGNAL_FUNC(darea_expose),oa);
#ifdef HAVE_PANEL_PIXEL_SIZE
   gtk_signal_connect (GTK_OBJECT (oa->applet),
   	"change_pixel_size",GTK_SIGNAL_FUNC(odo_change_pixel_size),oa);
#endif
   gtk_signal_connect (GTK_OBJECT (oa->applet),
   	"change_orient",GTK_SIGNAL_FUNC(odo_change_orient),oa);

   applet_widget_register_callback (APPLET_WIDGET (oa->applet),
   	"reset",_("Reset"),GTK_SIGNAL_FUNC(reset_cb),oa);
   applet_widget_register_stock_callback (APPLET_WIDGET (oa->applet),
   	"properties",GNOME_STOCK_MENU_PROP,
   	_("Properties..."),GTK_SIGNAL_FUNC(properties_cb),oa);
   applet_widget_register_stock_callback (APPLET_WIDGET (oa->applet),
	"help", GNOME_STOCK_PIXMAP_HELP,
	_("Help"), help_cb, NULL);					  
   applet_widget_register_stock_callback (APPLET_WIDGET (oa->applet),
   	"about",GNOME_STOCK_MENU_ABOUT,
   	_("About..."),GTK_SIGNAL_FUNC(about_cb),oa);

   properties_load (APPLET_WIDGET (oa->applet)->privcfgpath,oa);
   create_odo (oa);
   gtk_widget_realize (oa->applet);
   gtk_widget_realize (oa->darea1);
   gtk_widget_realize (oa->darea2);
   change_theme (oa->theme_file,oa);
   gtk_timeout_add (UPDATE_TIMEOUT,(GtkFunction)timer_cb,oa);
   /*
    * need to call gtk_widget_realize() before 
    * the initialization of the pixmap
    */
   gtk_widget_show (oa->applet);
   applet_widget_gtk_main ();
   return 0;
}
