/*###################################################################*/
/*##                         clock & mail applet 0.2.0             ##*/
/*###################################################################*/

#include "clockmail.h"

#include "backgrnd.xpm"
#include "digmed.xpm"
#include "mailpics.xpm"

static GdkPixmap *get_pixmap_from_data(gchar **data);
static GdkPixmap *get_pixmap_from_file(gchar *path);
static DigitData *new_digit(GdkPixmap *pixmap);
static DigitData *new_digit_from_data(gchar **data);
static DigitData *new_digit_from_file(gchar *file);
static void free_digit(DigitData *digit);
static NumberData *new_number(DigitData *digits, gint length, gint zeros, gint x, gint y);
static void free_number(NumberData *number);
static ItemData *new_item(GdkPixmap *pixmap, gint sections, gint x, gint y);
static ItemData *new_item_from_data(gchar **data, gint sections, gint x, gint y);
static ItemData *new_item_from_file(gchar *file, gint sections, gint x, gint y);
static void free_item(ItemData *item);
static SkinData *new_skin();
static void draw_digit(DigitData *digit, gint n, gint x, gint y, AppData *ad);
static SkinData *load_default_skin();
static ItemData *get_item(gchar *path, gchar *datafile, gchar *name, gint sections, gint vertical);
static DigitData *get_digit(gchar *path, gchar *datafile, gchar *name, gint vertical);
static NumberData *get_number(gchar *path, gchar *datafile, gchar *name, gint zeros, gint vertical, SkinData *skin);
static GdkPixmap *get_background(gchar *path, gchar *datafile, gint vertical);
static SkinData *load_skin(gchar *skin_path, gint vertical);

static GdkPixmap *get_pixmap_from_data(gchar **data)
{
	GdkPixmap *pixmap = NULL;
	GdkBitmap *mask = NULL;

	gdk_imlib_data_to_pixmap(data, &pixmap, &mask);
	if (mask) gdk_imlib_free_bitmap(mask);
	return pixmap;
}

static GdkPixmap *get_pixmap_from_file(gchar *path)
{
	GdkPixmap *pixmap = NULL;
	GdkBitmap *mask = NULL;

	if (!g_file_exists(path)) return NULL;

	gdk_imlib_load_file_to_pixmap(path, &pixmap, &mask);
	if (mask) gdk_imlib_free_bitmap(mask);
	return pixmap;
}

void redraw_skin(AppData *ad)
{
	gdk_window_set_back_pixmap(ad->display_area->window,ad->skin->background,FALSE);
	gdk_window_clear(ad->display_area->window);
}

static DigitData *new_digit(GdkPixmap *pixmap)
{
	DigitData *digit;
	gint width;
	gint height;

	digit = g_new0(DigitData, 1);

	gdk_window_get_size (pixmap, &width, &height);

	digit->pixmap = pixmap;
	digit->width = width / 11;
	digit->height = height;

	return digit;
}

static DigitData *new_digit_from_data(gchar **data)
{
	GdkPixmap *pixmap;

	pixmap = get_pixmap_from_data(data);

	return new_digit(pixmap);
}

static DigitData *new_digit_from_file(gchar *file)
{
	GdkPixmap *pixmap;

	pixmap = get_pixmap_from_file(file);
	if (!pixmap) return NULL;

	return new_digit(pixmap);
}

static void free_digit(DigitData *digit)
{
	if (!digit) return;
	if (digit->pixmap) gdk_imlib_free_pixmap(digit->pixmap);
	g_free(digit);
}

static NumberData *new_number(DigitData *digits, gint length, gint zeros, gint x, gint y)
{
	NumberData *number;
	number = g_new0(NumberData, 1);

	number->digits = digits;
	number->length = length;
	number->zeros = zeros;
	number->x = x;
	number->y = y;

	return number;
}

static void free_number(NumberData *number)
{
	if (!number) return;
	g_free(number);
}

static ItemData *new_item(GdkPixmap *pixmap, gint sections, gint x, gint y)
{
	ItemData *item;
	gint width;
	gint height;

	item = g_new0(ItemData, 1);

	gdk_window_get_size (pixmap, &width, &height);

	item->pixmap = pixmap;
	item->width = width;
	item->height = height / sections;
	item->sections = sections;
	item->x = x;
	item->y = y;

	return item;
}

static ItemData *new_item_from_data(gchar **data, gint sections, gint x, gint y)
{
	GdkPixmap *pixmap;

	pixmap = get_pixmap_from_data(data);

	return new_item(pixmap, sections, x, y);
}

static ItemData *new_item_from_file(gchar *file, gint sections, gint x, gint y)
{
	GdkPixmap *pixmap;

	pixmap = get_pixmap_from_file(file);
	if (!pixmap) return NULL;

	return new_item(pixmap, sections, x, y);
}

static void free_item(ItemData *item)
{
	if (!item) return;
	if (item->pixmap) gdk_imlib_free_pixmap(item->pixmap);
	g_free(item);
}

static SkinData *new_skin()
{
	SkinData *s;
	s = g_new0(SkinData, 1);
	return s;
}

void sync_window_to_skin(AppData *ad)
{
	if (!ad->skin) return;

	/* Fix ME! Fix ME! Fix ME! Fix ME! Fix ME! Fix ME! Fix ME! */
	/* all this, and I can't make the applet smaller */
	gtk_drawing_area_size(GTK_DRAWING_AREA(ad->display_area), ad->skin->width, ad->skin->height);
	gtk_widget_set_usize(ad->display_area, ad->skin->width, ad->skin->height);
	gtk_widget_set_usize(ad->applet, ad->skin->width, ad->skin->height);
	redraw_skin(ad);

	if (ad->skin->mail)
		ad->mail_sections = ad->skin->mail->sections;
	else
		ad->mail_sections = 1;

	/* we are forcing some redraws here */
	ad->old_week = -1;
	ad->old_n = -1;
}

void free_skin(SkinData *s)
{
	if (!s) return;
	if (s->background) gdk_imlib_free_pixmap(s->background);
	free_number(s->hour);
	free_number(s->min);
	free_number(s->sec);
	free_number(s->month);
	free_number(s->day);
	free_number(s->year);
	free_digit(s->dig_small);
	free_digit(s->dig_large);
	free_item(s->mail);
	free_item(s->month_txt);
	free_item(s->week_txt);
	g_free(s);
}

static void draw_digit(DigitData *digit, gint n, gint x, gint y, AppData *ad)
{
	if (!digit) return;

	if (n == -1)
		gdk_draw_pixmap(ad->skin->background,
			ad->display_area->style->fg_gc[GTK_WIDGET_STATE(ad->display_area)],
			digit->pixmap, digit->width * 10, 0, x, y, digit->width, digit->height);
	if (n >= 0 && n <= 9)
		gdk_draw_pixmap(ad->skin->background,
			ad->display_area->style->fg_gc[GTK_WIDGET_STATE(ad->display_area)],
			digit->pixmap, n * digit->width, 0, x, y, digit->width, digit->height);
}

void draw_number(NumberData *number, gint n, AppData *ad)
{
	gint i, d;
	gint t = 1;
	gint x;
	gint z;
	DigitData *digit;

	if (!number) return;

	x = number->x;
	z = number->zeros;
	digit = number->digits;

	for (i=0; i< number->length - 1; i++) t = t * 10;
	x += number->length * digit->width;
	for (i=number->length; i > 0; i--)
		{
		d = n / t;
		n -= d * t;
		t = t / 10;
		if (d == 0 && i>1 && (!z))
			d = -1;
		else
			z = TRUE;
		draw_digit(digit, d, x - (i * digit->width), number->y, ad);
		}
}

void draw_item(ItemData *item, gint section, AppData *ad)
{
	if (!item) return;

	gdk_draw_pixmap (ad->skin->background,
		ad->display_area->style->fg_gc[GTK_WIDGET_STATE(ad->display_area)],
		item->pixmap, 0, item->height * section, item->x, item->y, item->width, item->height);
	
}

static SkinData *load_default_skin()
{
	SkinData *s;
	GdkPixmap *background;
	gint width, height;

	s = new_skin();

	background = get_pixmap_from_data((gchar **)backgrnd_xpm);

	gdk_window_get_size (background, &width, &height);

	s->background = background;
	s->width = width;
	s->height = height;

	s->dig_large = new_digit_from_data((gchar **)digmed_xpm);

	s->hour = new_number(s->dig_large, 2, FALSE, 3, 4);
	s->min = new_number(s->dig_large, 2, TRUE, 26, 4);

	s->mail = new_item_from_data((gchar **)mailpics_xpm, 10, 3, 23);
	return s;
}

/* the following are functions for loading external skins */

static ItemData *get_item(gchar *path, gchar *datafile, gchar *name, gint sections, gint vertical)
{
	ItemData *item;
	gchar **vector = NULL;
	gint length;
	gchar *filename;
	gchar *lang_filename = NULL;
	gint x;
	gint y;
	gchar *prefix;

	if (vertical)
		prefix = g_strconcat ("=", datafile, "=/Vertical/", NULL);
	else
		prefix = g_strconcat ("=", datafile, "=/Horizontal/", NULL);

	gnome_config_push_prefix (prefix);
	g_free (prefix);

	gnome_config_get_vector(name, &length, &vector);
	lang_filename = gnome_config_get_translated_string (name);
	gnome_config_pop_prefix();

	if (sections > 0)
		{
		if (!vector || length < 3)
			{
			g_free(lang_filename);
			gnome_string_array_free (vector);
			return NULL;
			}
		}
	else
		{
		if (!vector || length < 4)
			{
			g_free(lang_filename);
			gnome_string_array_free (vector);
			return NULL;
			}
		}

	/* if the language specific file exists, load it instead of the default */
	if (lang_filename)
		{
		filename = g_strconcat(path, "/", lang_filename, NULL);
		g_free (lang_filename);
		if (!g_file_exists(filename))
			{
			g_free(filename);
			filename = g_strconcat(path, "/", vector[0], NULL);
			}
		}
	else
		filename = g_strconcat(path, "/", vector[0], NULL);

	if (sections > 0)
		{
		x = strtol(vector[1], NULL, 0);
		y = strtol(vector[2], NULL, 0);
		}
	else
		{
		sections = strtol(vector[1], NULL, 0);
		x = strtol(vector[2], NULL, 0);
		y = strtol(vector[3], NULL, 0);
		}

	gnome_string_array_free (vector);

	if (!g_file_exists(filename))
		{
		g_free(filename);
		return NULL;
		}

	item = new_item_from_file(filename, sections, x, y);
	g_free(filename);
	return item;
}

static DigitData *get_digit(gchar *path, gchar *datafile, gchar *name, gint vertical)
{
	DigitData *digit;
	gchar *filename;
	gchar *buf = NULL;
	gchar *prefix;

	if (vertical)
		prefix = g_strconcat ("=", datafile, "=/Vertical/", NULL);
	else
		prefix = g_strconcat ("=", datafile, "=/Horizontal/", NULL);

	gnome_config_push_prefix (prefix);
	g_free (prefix);

	buf = gnome_config_get_string(name);
	gnome_config_pop_prefix();

	if (!buf)
		{
		g_free(buf);
		return NULL;
		}

	filename = g_strconcat(path, "/", buf, NULL);
	g_free(buf);

	if (!g_file_exists(filename))
		{
		g_free(filename);
		return NULL;
		}

	digit = new_digit_from_file(filename);
	g_free(filename);
	return digit;
}

static NumberData *get_number(gchar *path, gchar *datafile, gchar *name, gint zeros, gint vertical, SkinData *skin)
{
	NumberData *number;
	DigitData *digit;
	gchar **vector = NULL;
	gint length;
	gchar *filename;
	gint x;
	gint y;
	gchar *prefix;

	if (vertical)
		prefix = g_strconcat ("=", datafile, "=/Vertical/", NULL);
	else
		prefix = g_strconcat ("=", datafile, "=/Horizontal/", NULL);

	gnome_config_push_prefix (prefix);
	g_free (prefix);

	gnome_config_get_vector(name, &length, &vector);
	gnome_config_pop_prefix();

	if (!vector || length < 3)
		{
		gnome_string_array_free (vector);
		return NULL;
		}

	filename = g_strdup(vector[0]);
	x = strtol(vector[1], NULL, 0);
	y = strtol(vector[2], NULL, 0);

	gnome_string_array_free (vector);

	if (!strcasecmp(filename, "Large"))
		digit = skin->dig_large;
	else if (!strcasecmp(filename, "Small"))
		digit = skin->dig_small;
	else
		{
		g_free(filename);
		return NULL;
		}

	number = new_number(digit, 2, zeros, x, y);
	g_free(filename);
	return number;
}

static GdkPixmap *get_background(gchar *path, gchar *datafile, gint vertical)
{
	GdkPixmap *pixmap;
	gchar *buf = NULL;
	gchar *filename;
	gchar *prefix;

	if (vertical)
		prefix = g_strconcat ("=", datafile, "=/Vertical/", NULL);
	else
		prefix = g_strconcat ("=", datafile, "=/Horizontal/", NULL);

	gnome_config_push_prefix (prefix);
	g_free (prefix);

	buf = gnome_config_get_string("Background=");
	gnome_config_pop_prefix();

	if (!buf)
		{
		g_free(buf);
		return NULL;
		}

	filename = g_strconcat(path, "/", buf, NULL);
	g_free(buf);

	if (!g_file_exists(filename))
		{
		g_free(filename);
		return NULL;
		}

	pixmap = get_pixmap_from_file(filename);
	g_free(filename);
	return pixmap;
}

static SkinData *load_skin(gchar *skin_path, gint vertical)
{
	SkinData *s;
	gchar *datafile = g_strconcat(skin_path, "/clockmaildata", NULL);

	if (!g_file_exists(datafile))
		{
		/* no file has been created yet */
		printf("Unable to open skin data file: %s\n",datafile);
		g_free(datafile);
		return NULL;
		}
	s = new_skin();

	/* background */

	s->background = get_background(skin_path, datafile, vertical);
	if (s->background)
		{
		gint width, height;
		gdk_window_get_size (s->background, &width, &height);
		s->width = width;
		s->height = height;
		}
	else
		{
		free_skin(s);
		g_free(datafile);
		return NULL;
		}

	s->dig_small = get_digit(skin_path, datafile, "Digit_Small=", vertical);
	s->dig_large = get_digit(skin_path, datafile, "Digit_Large=", vertical);
	s->hour = get_number(skin_path, datafile, "Number_Hour=", FALSE, vertical, s);
	s->min = get_number(skin_path, datafile, "Number_Minutes=", TRUE, vertical, s);
	s->sec = get_number(skin_path, datafile, "Number_Seconds=", TRUE, vertical, s);
	s->month = get_number(skin_path, datafile, "Number_Month=", FALSE, vertical, s);
	s->day = get_number(skin_path, datafile, "Number_Day=", FALSE, vertical, s);
	s->year = get_number(skin_path, datafile, "Number_Year=", TRUE, vertical, s);
	s->mail = get_item(skin_path, datafile, "Mail_Image=", 0, vertical);
	s->month_txt = get_item(skin_path, datafile, "Item_Month_Text=", 12, vertical);
	s->week_txt = get_item(skin_path, datafile, "Item_Week_text=", 7, vertical);

	g_free(datafile);
	return s;
}

gint change_to_skin(gchar *path, AppData *ad)
{
	SkinData *nsh = NULL;
	SkinData *nsv = NULL;
	SkinData *osh = ad->skin_h;
	SkinData *osv = ad->skin_v;

	if (!path)
		{
		nsh = load_default_skin();
		}
	else
		{
		nsh = load_skin(path, FALSE);
		nsv = load_skin(path, TRUE);
		}

	if (!nsh) return FALSE;

	ad->skin_h = nsh;
	ad->skin_v = nsv;

	ad->skin = nsh;
	if (nsv && (ad->orient == ORIENT_LEFT || ad->orient == ORIENT_RIGHT))
		{
		ad->skin = nsv;
		}

	sync_window_to_skin(ad);
	free_skin(osh);
	free_skin(osv);
	return TRUE;
}
