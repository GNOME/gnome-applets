/* GNOME clock & mailcheck applet
 * (C) 2000 John Ellis
 *
 * Author: John Ellis
 *
 */

#include "clockmail.h"
#include "theme.h"
#include "item.h"
#include "button.h"
#include "number.h"
#include "clock.h"
#include "skin.h"
#include "update.h"

/*
 *--------------------------------------------------------------------
 * the loader functions
 *--------------------------------------------------------------------
 */

static HandData *get_hand(gchar *path, gchar *name)
{
	HandData *hd;
	gchar **vector = NULL;
	gint length;
	gchar *filename;
	gint xo, yo;

	gnome_config_get_vector(name, &length, &vector);

	if (!vector || length < 3)
		{
		g_strfreev (vector);
		return NULL;
		}

	filename = g_strconcat(path, "/", vector[0], NULL);
	xo = strtol(vector[1], NULL, 10);
	yo = strtol(vector[2], NULL, 10);

	g_strfreev (vector);
	
	hd = hand_new_from_file(filename, xo, yo);
	g_free(filename);

	return hd;
}

static ClockData *get_clock(gchar *path)
{
	ClockData *cd;
	HandData *hour;
	HandData *min;
	HandData *sec;
	gchar **vector = NULL;
	gint length;
	gchar *filename;
	gint x, y, w, h;

	gnome_config_get_vector("Clock_Back=", &length, &vector);

	if (!vector || length < 5)
		{
		g_strfreev (vector);
		return NULL;
		}

	filename = g_strdup(vector[0]);
	w = strtol(vector[1], NULL, 10);
	h = strtol(vector[2], NULL, 10);
	x = strtol(vector[3], NULL, 10);
	y = strtol(vector[4], NULL, 10);

	g_strfreev (vector);

	if (filename && strcmp(filename, "none") == 0)
		{
		g_free(filename);
		filename = NULL;
		}
	else
		{
		gchar *buf = g_strconcat(path, "/", filename, NULL);
		g_free(filename);
		filename = buf;
		}

	hour = get_hand(path, "Clock_Hour_Hand=");
	min = get_hand(path, "Clock_Minute_Hand=");
	sec = get_hand(path, "Clock_Second_Hand=");

	if (filename)
		{
		cd = clock_new_from_file(hour, min, sec, filename, x, y);
		g_free(filename);
		}
	else
		{
		cd = clock_new_from_data(hour, min, sec, NULL, x, y, w, h);
		}

	return cd;

}

static ItemData *get_item(gchar *path, gchar *name, gint sections)
{
	ItemData *item;
	gchar **vector = NULL;
	gint length;
	gchar *filename;
	gchar *lang_filename = NULL;
	gint x;
	gint y;

	gnome_config_get_vector(name, &length, &vector);
	lang_filename = gnome_config_get_translated_string (name);

	if (sections > 0)
		{
		if (!vector || length < 3)
			{
			g_free(lang_filename);
			g_strfreev (vector);
			return NULL;
			}
		}
	else
		{
		if (!vector || length < 4)
			{
			g_free(lang_filename);
			g_strfreev (vector);
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

	g_strfreev (vector);

	if (!g_file_exists(filename))
		{
		g_free(filename);
		return NULL;
		}

	item = item_new_from_file(filename, sections, x, y);
	g_free(filename);
	return item;
}

static DigitData *get_digit(gchar *path, gchar *name)
{
	DigitData *digit;
	gchar *filename;
	gchar *buf = NULL;

	buf = gnome_config_get_string(name);

	if (!buf) return NULL;

	filename = g_strconcat(path, "/", buf, NULL);
	g_free(buf);

	if (!g_file_exists(filename))
		{
		g_free(filename);
		return NULL;
		}

	digit = digit_new_from_file(filename);
	g_free(filename);
	return digit;
}

static NumberData *get_number(gchar *path, gchar *name, gint count, gint zeros, SkinData *skin)
{
	NumberData *number;
	DigitData *digit;
	gchar **vector = NULL;
	gint length;
	gchar *filename;
	gint x;
	gint y;
	gint centered = FALSE;

	gnome_config_get_vector(name, &length, &vector);

	if (!vector || length < 3)
		{
		g_strfreev (vector);
		return NULL;
		}

	filename = g_strdup(vector[0]);
	x = strtol(vector[1], NULL, 0);
	y = strtol(vector[2], NULL, 0);

	if (length > 3 && (*vector[3] == 'T' || *vector[3] == 't') )
		{
		centered = TRUE;
		}

	if (length > 4)
		{
		count = strtol(vector[4], NULL, 0);;
		}

	g_strfreev (vector);

	if (!strcasecmp(filename, "Large"))
		digit = skin->dig_large;
	else if (!strcasecmp(filename, "Small"))
		digit = skin->dig_small;
	else
		{
		g_free(filename);
		return NULL;
		}

	number = number_new(digit, count, zeros, centered, x, y);
	g_free(filename);
	return number;
}

static GdkPixbuf *get_background(gchar *path)
{
	GdkPixbuf *pb;
	gchar *buf = NULL;
	gchar *filename;

	buf = gnome_config_get_string("Background=");

	if (!buf) return NULL;

	filename = g_strconcat(path, "/", buf, NULL);
	g_free(buf);

	if (!g_file_exists(filename))
		{
		g_free(filename);
		return NULL;
		}

	pb = gdk_pixbuf_new_from_file(filename);

	g_free(filename);
	return pb;
}

SkinData *skin_load(gchar *skin_path, gint vertical, gint size, AppData *ad)
{
	SkinData *s;
	gchar *datafile = g_strconcat(skin_path, "/clockmaildata", NULL);
	gchar *prefix;
	gchar *stext;

	if (!g_file_exists(datafile))
		{
		/* no file has been created yet */
		printf("Unable to open skin data file: %s\n",datafile);
		g_free(datafile);
		return NULL;
		}
	s = skin_new();

	switch (size)
		{
		case SIZEHINT_TINY:
			stext = "_Tiny";
			break;
		case SIZEHINT_SMALL:
			stext = "_Small";
			break;
		case SIZEHINT_LARGE:
			stext = "_Large";
			break;
		case SIZEHINT_HUGE:
			stext = "_Huge";
			break;
		case SIZEHINT_STANDARD:
		default:
			stext = "";
			break;
		}

	if (vertical)
		{
		prefix = g_strconcat ("=", datafile, "=/Vertical", stext, "/", NULL);
		}
	else
		{
		prefix = g_strconcat ("=", datafile, "=/Horizontal", stext, "/", NULL);
		}

	gnome_config_push_prefix (prefix);
	g_free(prefix);

	/* background */

	s->overlay = get_background(skin_path);
	if (!s->overlay)
		{
		skin_free(s);
		g_free(datafile);
		gnome_config_pop_prefix();
		return NULL;
		}

	s->width = gdk_pixbuf_get_width(s->overlay);
	s->height = gdk_pixbuf_get_height(s->overlay);

	s->pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, s->width, s->height);

	s->dig_small = get_digit(skin_path, "Digit_Small=");
	s->dig_large = get_digit(skin_path, "Digit_Large=");
	s->hour = get_number(skin_path, "Number_Hour=", 2, FALSE, s);
	s->min = get_number(skin_path, "Number_Minutes=", 2, TRUE, s);
	s->sec = get_number(skin_path, "Number_Seconds=", 2, TRUE, s);
	s->month = get_number(skin_path, "Number_Month=", 2, FALSE, s);
	s->day = get_number(skin_path, "Number_Day=", 2, FALSE, s);
	s->year = get_number(skin_path, "Number_Year=", 2, TRUE, s);
	s->mail = get_item(skin_path, "Mail_Image=", 0);
	s->mail_amount = get_item(skin_path, "Item_Mail_Percent=", 0);
	s->month_txt = get_item(skin_path, "Item_Month_Text=", 12);
	s->week_txt = get_item(skin_path, "Item_Week_text=", 7);
	s->mail_count = get_number(skin_path, "Number_Mail=", 4, FALSE, s);
	s->messages = get_number(skin_path, "Number_Messages=", 3, FALSE, s);
	s->button_pix = get_item(skin_path, "Item_Button=", 0);

	s->button = button_new(s->button_pix, s->width, s->height,
			       launch_mail_reader, ad, update_all, ad);

	s->clock = get_clock(skin_path);

	g_free(datafile);
	gnome_config_pop_prefix();
	return s;
}

