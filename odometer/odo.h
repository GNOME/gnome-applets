/*
 * GNOME odometer panel applet
 * (C) 1999 The Free software Foundation
 * 
 * Author : Fabrice Bellet <Fabrice.Bellet@creatis.insa-lyon.fr>
 *          adapted from kodo/Xodometer/Mouspedometa
 */

#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include <gnome.h>
#include <applet-widget.h>

#define ODO_VERSION "0.7"
#define UPDATE_TIMEOUT 75

#define SIZEHINT_DEFAULT 46

#define INCH 0
#define FOOT 1
#define MILE 2
#define MAX_UNIT 3
#undef	DEBUG

#define INTEGER 0
#define DECIMAL 1

typedef int Units;

typedef struct _conversionEntry conversionEntry;
struct _conversionEntry {
    Units from_unit;
    char *from_unit_tag;
    char *from_unit_tag_plural;
    double max_from_before_next;
    double conversion_factor;
    char *to_unit_tag;
    char *to_unit_tag_plural;
    double max_to_before_next;
    int print_precision;
};

typedef struct _OdoApplet OdoApplet;
struct _OdoApplet {
	/* Applet */
	GtkWidget *applet;
	GtkWidget *darea1,*darea2;
	GtkWidget *vbox;
	GdkPixmap *digits;
	PanelOrientType orient;
	int size;

	/* Theme */
	gchar *theme_file;
	GtkWidget *theme_entry;
	GdkImlibImage *image[2];
	gint width;
	gint height;
	gint digit_width_in_image;
	gint digit_width;
	gint digit_height;
	gboolean with_decimal_dot;

	/* Properties */
	GtkWidget *pbox;
	GtkWidget *spinner;
	gboolean use_metric,p_use_metric;
	gboolean enabled,p_enabled;
	gboolean auto_reset,p_auto_reset;
	gint digits_nb,p_digits_nb;
	gboolean scale_applet,p_scale_applet;

	/* Odometer */
	int last_x_coord;
	int last_y_coord;
	int x_coord;
	int y_coord;
	double distance;
	double trip_distance;
	double last_distance;

	int cycles_since_last_save;

	gfloat h_pixels_per_mm,v_pixels_per_mm;
	
	Units distance_unit;
	Units trip_distance_unit;
};

void properties_cb (AppletWidget *applet, gpointer data);

void refresh (OdoApplet *oa);
gint change_digits_nb (OdoApplet *oa);

gint change_theme (gchar *path, OdoApplet *oa);
