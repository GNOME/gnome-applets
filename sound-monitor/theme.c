/* GNOME sound-monitor applet
 * (C) 2000 John Ellis
 *
 * Author: John Ellis
 *
 */

#include "sound-monitor.h"
#include "theme.h"
#include "analyzer.h"
#include "item.h"
#include "meter.h"
#include "scope.h"
#include "skin.h"

static ItemData *get_item(const gchar *path, const gchar *name, gint sections)
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

static MeterData *get_meter(const gchar *path, const gchar *name)
{
	MeterData *meter;
	gchar **vector = NULL;
	gint length;
	gchar *filename;
	gint x;
	gint y;
	gint horizontal = FALSE;
	gint sections;

	gnome_config_get_vector(name, &length, &vector);

	if (!vector || length < 5)
		{
		g_strfreev (vector);
		return NULL;
		}

	filename = g_strconcat(path, "/", vector[0], NULL);

	sections = strtol(vector[1], NULL, 0);
	x = strtol(vector[2], NULL, 0);
	y = strtol(vector[3], NULL, 0);
	if (vector[4])
		{
		gchar *ptr = vector[4];
		if (ptr[0] == 'T' || ptr[0] == 't')
			horizontal = TRUE;
		else
			horizontal = FALSE;
		}

	g_strfreev (vector);

	if (!g_file_exists(filename))
		{
		g_free(filename);
		return NULL;
		}

	meter = meter_new_from_file(filename, sections, x, y, horizontal);
	g_free(filename);
	return meter;
}

static ScopeData *get_scope(const gchar *path, const gchar *name)
{
	ScopeData *scope;
	gchar **vector = NULL;
	gint length;
	gchar *filename;
	gint x;
	gint y;
	gint horizontal = FALSE;

	gnome_config_get_vector(name, &length, &vector);

	if (!vector || length < 4)
		{
		g_strfreev (vector);
		return NULL;
		}

	filename = g_strconcat(path, "/", vector[0], NULL);

	x = strtol(vector[1], NULL, 0);
	y = strtol(vector[2], NULL, 0);
	if (vector[3])
		{
		gchar *ptr = vector[3];
		if (ptr[0] == 'T' || ptr[0] == 't')
			horizontal = TRUE;
		else
			horizontal = FALSE;
		}

	g_strfreev (vector);

	if (!g_file_exists(filename))
		{
		g_free(filename);
		return NULL;
		}

	scope = scope_new_from_file(filename, x, y, horizontal);
	g_free(filename);
	return scope;
}

static AnalyzerData *get_analyzer(const gchar *path, const gchar *name)
{
	AnalyzerData *analyzer;
	gchar **vector = NULL;
	gint length;
	gchar *filename;
	gint x;
	gint y;
	gint horizontal = FALSE;
	gint bands;
	gint rows;

	gnome_config_get_vector(name, &length, &vector);

	if (!vector || length < 6)
		{
		g_strfreev (vector);
		return NULL;
		}

	filename = g_strconcat(path, "/", vector[0], NULL);

	x = strtol(vector[1], NULL, 10);
	y = strtol(vector[2], NULL, 10);
	if (vector[3])
		{
		gchar *ptr = vector[3];
		if (ptr[0] == 'T' || ptr[0] == 't')
			horizontal = TRUE;
		else
			horizontal = FALSE;
		}
	bands = strtol(vector[4], NULL, 10);
	rows = strtol(vector[5], NULL, 10);

	g_strfreev (vector);

	if (!g_file_exists(filename))
		{
		g_free(filename);
		return NULL;
		}

	analyzer = anaylzer_new_from_file(filename, x, y, horizontal, bands, rows);
	g_free(filename);
	return analyzer;
}

static GdkPixbuf *get_background(const gchar *path)
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

SkinData *skin_load(const gchar *skin_path, gint vertical, gint size, AppData *ad)
{
	SkinData *s;
	gchar *datafile = g_strconcat(skin_path, "/themedata", NULL);
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

	s->status = get_item(skin_path, "Item_Status=", 4);
	s->meter_left = get_meter(skin_path, "Item_Meter_Left=");
	s->meter_right = get_meter(skin_path, "Item_Meter_Right=");
	s->item_left = get_item(skin_path, "Item_Misc_Left=", 0);
	s->item_right = get_item(skin_path, "Item_Misc_Right=", 0);
	s->scope = get_scope(skin_path, "Scope=");
	s->analyzer = get_analyzer(skin_path, "Analyzer=");

	g_free(datafile);
	gnome_config_pop_prefix();
	return s;
}

