/* GNOME Esound Monitor Control applet
 * (C) 1999 John Ellis
 *
 * Author: John Ellis
 *
 */

#include "sound-monitor.h"

#include "def_theme/back_h.xpm"
#include "def_theme/led_h.xpm"
#include "def_theme/status_h.xpm"
#include "def_theme/back_v.xpm"
#include "def_theme/led_v.xpm"
#include "def_theme/status_v.xpm"

static GdkPixmap *get_pixmap_from_data(gchar **data);
static GdkPixmap *get_pixmap_from_file(gchar *path);
static ItemData *new_item(GdkPixmap *pixmap, gint sections, gint x, gint y);
static ItemData *new_item_from_data(gchar **data, gint sections, gint x, gint y);
static ItemData *new_item_from_file(gchar *file, gint sections, gint x, gint y);
static VuData *new_vu_item(GdkPixmap *pixmap, gint sections, gint x, gint y, gint horizontal);
static VuData *new_vu_item_from_data(gchar **data, gint sections, gint x, gint y, gint horizontal);
static VuData *new_vu_item_from_file(gchar *file, gint sections, gint x, gint y, gint horizontal);
static void free_item(ItemData *item);
static void free_vu_item(VuData *item);
static SkinData *new_skin();
static SkinData *load_default_skin_h();
static SkinData *load_default_skin_v();
static ItemData *get_item(gchar *path, gchar *datafile, gchar *name, gint sections, gint vertical);
static VuData *get_vu_item(gchar *path, gchar *datafile, gchar *name, gint vertical);
static GtkWidget *get_background(gchar *path, gchar *datafile, gint vertical);
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

/*
 *----------------------------------------------------------------------------
 * Item widget
 *----------------------------------------------------------------------------
 */

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

	item->current_section = -1;

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

gint draw_item(ItemData *item, gint section, AppData *ad)
{
	if (!item) return FALSE;

	if (item->current_section == section) return FALSE;

	item->current_section = section;

	gdk_draw_pixmap (ad->skin->background,
		ad->display_area->style->fg_gc[GTK_WIDGET_STATE(ad->display_area)],
		item->pixmap, 0, item->height * section, item->x, item->y, item->width, item->height);

	return TRUE;
}

gint draw_item_by_percent(ItemData *item, gint percent, AppData *ad)
{
	gint section;
	if (!item) return FALSE;

	section = (float)item->sections * percent / 100;
	if (section == 0 && percent > 0) section = 1;
	if (section > item->sections - 1) section = item->sections - 1;

	return draw_item(item, section, ad);
}

/*
 *----------------------------------------------------------------------------
 * Vu widget
 *----------------------------------------------------------------------------
 */

static VuData *new_vu_item(GdkPixmap *pixmap, gint sections, gint x, gint y, gint horizontal)
{
	VuData *item;
	gint width;
	gint height;

	item = g_new0(VuData, 1);

	gdk_window_get_size (pixmap, &width, &height);

	item->pixmap = pixmap;
	item->sections = sections;
	if (horizontal)
		{
		item->width = width / 3;
		item->height = height;
		item->section_height = item->width / sections;
		}
	else
		{
		item->width = width;
		item->height = height / 3;
		item->section_height = item->height / sections;
		}
	item->x = x;
	item->y = y;
	item->horizontal = horizontal;

	item->peak = 99;
	item->old_peak = sections;
	item->value = 0;
	item->old_val = 0;

	return item;
}

static VuData *new_vu_item_from_data(gchar **data, gint sections, gint x, gint y, gint horizontal)
{
	GdkPixmap *pixmap;

	pixmap = get_pixmap_from_data(data);

	return new_vu_item(pixmap, sections, x, y, horizontal);
}

static VuData *new_vu_item_from_file(gchar *file, gint sections, gint x, gint y, gint horizontal)
{
	GdkPixmap *pixmap;

	pixmap = get_pixmap_from_file(file);
	if (!pixmap) return NULL;

	return new_vu_item(pixmap, sections, x, y, horizontal);
}

static void free_vu_item(VuData *item)
{
	if (!item) return;
	if (item->pixmap) gdk_imlib_free_pixmap(item->pixmap);
	g_free(item);
}

/* value is 0 - 100 (and we assume called every 1/10th second) */
gint draw_vu_item(VuData *item, gint value, AppData *ad)
{
	gint new_val;
	gint new_peak;

	gint old_val;
	gint old_peak;

	if (!item) return FALSE;

	old_val = item->value;
	old_peak = item->peak;

	item->value = value;

	if (item->mode == PEAK_MODE_OFF)
		{
		item->peak = 0;
		}
	else if (item->mode == PEAK_MODE_ACTIVE)
		{
		if (item->value >= item->peak)
			item->peak = item->value;
		else
			{
			item->peak -= item->peak_fall; /* 1 = 10% per second... */
			if (item->peak < item->value) item->peak = item->value;
			}
		}
	else /* assume now that (item->mode == PEAK_MODE_SMOOTH) */
		{
		if (item->value > item->peak)
			{
			item->peak += item->peak_fall;
			if (item->peak > item->value) item->peak = item->value;
			}
		else if (item->value < item->peak)
			{
			item->peak -= item->peak_fall;
			if (item->peak < item->value) item->peak = item->value;
			}
		}

	new_val = (float)item->value / 100.0 * item->sections;
/*	if (new_val == 0 && item->value > 0) new_val = 1;*/
	if (new_val > item->sections) new_val = item->sections;

	if (item->peak > 0)
		{
		new_peak = (float)item->peak / 100.0 * item->sections;
/*		if (new_peak == 0) new_peak = 1; */
		if (new_peak > item->sections) new_peak = item->sections;
		}
	else
		{
		new_peak = 0;
		}


	/* now to the drawing */

	if (new_val != item->old_val || new_peak != item->old_peak)
		{
		gdk_draw_pixmap (ad->skin->background,
			ad->display_area->style->fg_gc[GTK_WIDGET_STATE(ad->display_area)],
			item->pixmap, 0, 0, item->x, item->y, item->width, item->height);

		if (item->horizontal)
			{
			gdk_draw_pixmap (ad->skin->background,
				ad->display_area->style->fg_gc[GTK_WIDGET_STATE(ad->display_area)],
				item->pixmap, item->width, 0,
				item->x, item->y,
				item->section_height * new_val, item->height);

			if (new_peak > 0)
				gdk_draw_pixmap (ad->skin->background,
					ad->display_area->style->fg_gc[GTK_WIDGET_STATE(ad->display_area)],
					item->pixmap, (item->width * 2) + item->section_height * new_peak, 0,
					item->x + (item->section_height * new_peak), item->y,
					item->section_height, item->height);
			}
		else
			{
			gdk_draw_pixmap (ad->skin->background,
				ad->display_area->style->fg_gc[GTK_WIDGET_STATE(ad->display_area)],
				item->pixmap, 0, (item->height * 2) - (item->section_height * new_val),
				item->x, item->y + ((item->sections - new_val) * item->section_height),
				item->width, item->section_height * new_val);

			if (new_peak > 0)
				gdk_draw_pixmap (ad->skin->background,
					ad->display_area->style->fg_gc[GTK_WIDGET_STATE(ad->display_area)],
					item->pixmap, 0, (item->height * 3) - item->section_height * new_peak,
					item->x, item->y + ((item->sections - new_peak) * item->section_height),
					item->width, item->section_height);
			}

		item->old_val = new_val;
		item->old_peak = new_peak;
		}

	if (old_val != item->value || old_peak != item->peak) return TRUE;

	return FALSE;
}

void set_vu_item_mode(VuData *item, gint mode, gint falloff)
{
	if (!item) return;
	item->mode = mode;
	item->peak_fall = falloff;
}

/*
 *----------------------------------------------------------------------------
 * Scope widget
 *----------------------------------------------------------------------------
 */

static ScopeData *new_scope_item(GdkPixmap *pixmap, gint x, gint y, gint horizontal)
{
	ScopeData *item;
	gint width;
	gint height;

	item = g_new0(ScopeData, 1);

	gdk_window_get_size (pixmap, &width, &height);

	item->pixmap = pixmap;
	item->width = width;
	item->height = height / 2;
	item->x = x;
	item->y = y;
	item->horizontal = horizontal;
	item->mask = NULL;
	item->scale = 5;
	if (horizontal)
		item->pts = g_malloc(sizeof(gint) * (item->height + 4));
	else
		item->pts = g_malloc(sizeof(gint) * (item->width + 4));

	return item;
}

/*
 * simply add this back if the default theme ever needs it
 *
static ScopeData *new_scope_item_from_data(gchar **data, gint x, gint y, gint horizontal)
{
	GdkPixmap *pixmap;

	pixmap = get_pixmap_from_data(data);

	return new_scope_item(pixmap, x, y, horizontal);
}
 */

static ScopeData *new_scope_item_from_file(gchar *file, gint x, gint y, gint horizontal)
{
	GdkPixmap *pixmap;

	pixmap = get_pixmap_from_file(file);
	if (!pixmap) return NULL;

	return new_scope_item(pixmap, x, y, horizontal);
}

static void free_scope_item(ScopeData *item)
{
	if (!item) return;
	if (item->pixmap) gdk_imlib_free_pixmap(item->pixmap);
	if (item->mask) gdk_imlib_free_bitmap(item->mask);
	if (item->mask_gc) gdk_gc_unref(item->mask_gc);
	g_free(item->pts);
	g_free(item);
}

void draw_scope_item(ScopeData *item, AppData *ad, gint flat)
{
	gint i;
	gint pnt_num;

	if (!item) return;

	if (item->horizontal)
		pnt_num = item->height;
	else
		pnt_num = item->width;

	if (flat)
		{
		for (i = 0; i < pnt_num; i++) item->pts[i] = 0;
		}
	else
		{
		gint p;
		gint scale;
		gint len, step;
		gfloat fact;

		if (NSAMP / 2 >= item->scale * pnt_num)
			scale = item->scale;
		else
			scale = NSAMP / 2 / pnt_num;

		step = scale * 2;
		len = pnt_num * step;
		if (item->horizontal)
			fact = (gfloat)65536 * scale / item->width;
		else
			fact = (gfloat)65536 * scale / item->height;
		i = p = 0;
		while(i < len)
			{
			gint k;
			item->pts[p] = 0;
			for (k = 0; k < step; k += 2)
				{
				item->pts[p] += (ad->aubuf[i + k] + ad->aubuf[i+ k + 1]) / 2;
				}
			item->pts[p] = (gfloat)item->pts[p] / fact;
			p++;
			i += step;
			}
		}

	if (!item->mask)
		{
		item->mask = gdk_pixmap_new(NULL, item->width, item->height, 1);
		item->mask_gc = gdk_gc_new(item->mask);

		gdk_color_parse("black", &(item->black));
		gdk_color_alloc(gtk_widget_get_colormap(ad->display_area), &(item->black));

		gdk_color_parse("white", &(item->white));
		gdk_color_alloc(gtk_widget_get_colormap(ad->display_area), &(item->white));
		}

	gdk_gc_set_foreground(item->mask_gc, &(item->black));
	gdk_draw_rectangle(item->mask, item->mask_gc, TRUE, 0, 0, item->width, item->height);

	gdk_gc_set_foreground(item->mask_gc, &(item->white));

	if (ad->draw_scope_as_segments)
		{
		if (item->horizontal)
			{
			for (i = 0; i < item->height - 1; i++)
				gdk_draw_line(item->mask, item->mask_gc, (item->width / 2) + item->pts[i], i, (item->width / 2) + item->pts[i+1], i+1);
			}
		else
			{
			for (i = 0; i < item->width - 1; i++)
				gdk_draw_line(item->mask, item->mask_gc, i, (item->height / 2) - item->pts[i], i+1, (item->height / 2) - item->pts[i+1]);
			}
		}
	else
		{
		if (item->horizontal)
			{
			for (i = 0; i < item->height; i++)
				gdk_draw_point(item->mask, item->mask_gc, (item->width / 2) + item->pts[i], i);
			}
		else
			{
			for (i = 0; i < item->width; i++)
				gdk_draw_point(item->mask, item->mask_gc, i, (item->height / 2) - item->pts[i]);
			}
		}

	gdk_draw_pixmap (ad->skin->background,
		ad->display_area->style->fg_gc[GTK_WIDGET_STATE(ad->display_area)],
		item->pixmap, 0, 0,
		item->x, item->y,
		item->width, item->height);

	gdk_gc_set_clip_mask(ad->display_area->style->fg_gc[GTK_WIDGET_STATE(ad->display_area)],
		item->mask);
	gdk_gc_set_clip_origin(ad->display_area->style->fg_gc[GTK_WIDGET_STATE(ad->display_area)],
		item->x, item->y);
		
	gdk_draw_pixmap (ad->skin->background,
		ad->display_area->style->fg_gc[GTK_WIDGET_STATE(ad->display_area)],
		item->pixmap, 0, item->height,
		item->x, item->y,
		item->width, item->height);

	gdk_gc_set_clip_origin(ad->display_area->style->fg_gc[GTK_WIDGET_STATE(ad->display_area)],
		0, 0);
	gdk_gc_set_clip_mask(ad->display_area->style->fg_gc[GTK_WIDGET_STATE(ad->display_area)],
		NULL);

}

void set_scope_item_scale(ScopeData *item, gint scale)
{
	if (!item) return;
	item->scale = scale;
}

/*
 *----------------------------------------------------------------------------
 * skin
 *----------------------------------------------------------------------------
 */

static SkinData *new_skin()
{
	SkinData *s;
	s = g_new0(SkinData, 1);
	return s;
}

void free_skin(SkinData *s)
{
	if (!s) return;
	if(s->pixmap) gtk_widget_destroy(s->pixmap);
	free_item(s->status);
	free_vu_item(s->vu_left);
	free_vu_item(s->vu_right);
	free_item(s->meter_left);
	free_item(s->meter_right);
	free_scope_item(s->scope);
	g_free(s);
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
	/* attempt to set the applets shape */
	gdk_window_shape_combine_mask (ad->display_area->window, ad->skin->mask, 0, 0);

	/* we are forcing some redraws here */
	set_widget_modes(ad);
}

/*
 *----------------------------------------------------------------------------
 * The default skin
 *----------------------------------------------------------------------------
 */

static SkinData *load_default_skin_h()
{
	SkinData *s;
	gint width, height;

	s = new_skin();

	s->pixmap = gnome_pixmap_new_from_xpm_d(back_h_xpm);
	s->background = GNOME_PIXMAP(s->pixmap)->pixmap;
	s->mask = GNOME_PIXMAP(s->pixmap)->mask;

	gdk_window_get_size (s->background, &width, &height);

	s->width = width;
	s->height = height;

	s->vu_left = new_vu_item_from_data((gchar **)led_h_xpm, 14, 1, 1, FALSE);
	s->vu_right = new_vu_item_from_data((gchar **)led_h_xpm, 14, 10, 1, FALSE);

	s->status = new_item_from_data((gchar **)status_h_xpm, 4, 1, 45);

	return s;
}

static SkinData *load_default_skin_v()
{
	SkinData *s;
	gint width, height;

	s = new_skin();

	s->pixmap = gnome_pixmap_new_from_xpm_d(back_v_xpm);
	s->background = GNOME_PIXMAP(s->pixmap)->pixmap;
	s->mask = GNOME_PIXMAP(s->pixmap)->mask;

	gdk_window_get_size (s->background, &width, &height);

	s->width = width;
	s->height = height;

	s->vu_left = new_vu_item_from_data((gchar **)led_v_xpm, 14, 5, 1, TRUE);
	s->vu_right = new_vu_item_from_data((gchar **)led_v_xpm, 14, 5, 10, TRUE);

	s->status = new_item_from_data((gchar **)status_v_xpm, 4, 1, 1);

	return s;
}

/*
 *----------------------------------------------------------------------------
 * Load external skin
 *----------------------------------------------------------------------------
 */

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

	item = new_item_from_file(filename, sections, x, y);
	g_free(filename);
	return item;
}

static VuData *get_vu_item(gchar *path, gchar *datafile, gchar *name, gint vertical)
{
	VuData *item;
	gchar **vector = NULL;
	gint length;
	gchar *filename;
	gint x;
	gint y;
	gint horizontal = FALSE;
	gint sections;
	gchar *prefix;

	if (vertical)
		prefix = g_strconcat ("=", datafile, "=/Vertical/", NULL);
	else
		prefix = g_strconcat ("=", datafile, "=/Horizontal/", NULL);

	gnome_config_push_prefix (prefix);
	g_free (prefix);

	gnome_config_get_vector(name, &length, &vector);
	gnome_config_pop_prefix();

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

	item = new_vu_item_from_file(filename, sections, x, y, horizontal);
	g_free(filename);
	return item;
}

static ScopeData *get_scope_item(gchar *path, gchar *datafile, gchar *name, gint vertical)
{
	ScopeData *item;
	gchar **vector = NULL;
	gint length;
	gchar *filename;
	gint x;
	gint y;
	gint horizontal = FALSE;
	gchar *prefix;

	if (vertical)
		prefix = g_strconcat ("=", datafile, "=/Vertical/", NULL);
	else
		prefix = g_strconcat ("=", datafile, "=/Horizontal/", NULL);

	gnome_config_push_prefix (prefix);
	g_free (prefix);

	gnome_config_get_vector(name, &length, &vector);
	gnome_config_pop_prefix();

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

	item = new_scope_item_from_file(filename, x, y, horizontal);
	g_free(filename);
	return item;
}

static GtkWidget *get_background(gchar *path, gchar *datafile, gint vertical)
{
	GtkWidget *pixmap = NULL;
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

	pixmap = gnome_pixmap_new_from_file(filename);

	g_free(filename);
	return pixmap;
}

static SkinData *load_skin(gchar *skin_path, gint vertical)
{
	SkinData *s;
	gchar *datafile = g_strconcat(skin_path, "/themedata", NULL);

	if (!g_file_exists(datafile))
		{
		/* no file has been created yet */
		printf("Unable to open skin data file: %s\n",datafile);
		g_free(datafile);
		return NULL;
		}
	s = new_skin();

	/* background */

	s->pixmap = NULL;
	s->pixmap = get_background(skin_path, datafile, vertical);
	if (s->pixmap)
		{
		gint width, height;
		s->background = GNOME_PIXMAP(s->pixmap)->pixmap;
		s->mask = GNOME_PIXMAP(s->pixmap)->mask;
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

	s->status = get_item(skin_path, datafile, "Item_Status=", 4, vertical);
	s->vu_left = get_vu_item(skin_path, datafile, "Item_Meter_Left=", vertical);
	s->vu_right = get_vu_item(skin_path, datafile, "Item_Meter_Right=", vertical);
	s->meter_left = get_item(skin_path, datafile, "Item_Misc_Left=", 0, vertical);
	s->meter_right = get_item(skin_path, datafile, "Item_Misc_Right=", 0, vertical);
	s->scope = get_scope_item(skin_path, datafile, "Scope=", vertical);
	g_free(datafile);
	return s;
}

/*
 *----------------------------------------------------------------------------
 * Change the current skin
 *----------------------------------------------------------------------------
 */

gint change_to_skin(gchar *path, AppData *ad)
{
	SkinData *nsh = NULL;
	SkinData *nsv = NULL;
	SkinData *osh = ad->skin_h;
	SkinData *osv = ad->skin_v;

	if (!path)
		{
		nsh = load_default_skin_h();
		nsv = load_default_skin_v();
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
