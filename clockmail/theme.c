/* GNOME clock & mailcheck applet
 * (C) 1999 John Ellis
 *
 * Author: John Ellis
 *
 */

#include "clockmail.h"
#include "rotate.h"

#include "backgrnd.xpm"
#include "digmed.xpm"
#include "digsml.xpm"
#include "mailpics.xpm"
#include "button.xpm"

#include "backgrnd_tiny.xpm"
#include "button_tiny.xpm"

static GdkPixmap *get_pixmap_from_data(gchar **data);
static GdkPixmap *get_pixmap_from_file(gchar *path);
static DigitData *new_digit(GdkPixmap *pixmap);
static DigitData *new_digit_from_data(gchar **data);
static DigitData *new_digit_from_file(gchar *file);
static void free_digit(DigitData *digit);
static NumberData *new_number(DigitData *digits, gint length, gint zeros, gint centered, gint x, gint y);
static void free_number(NumberData *number);
static ItemData *new_item(GdkPixmap *pixmap, gint sections, gint x, gint y);
static ItemData *new_item_from_data(gchar **data, gint sections, gint x, gint y);
static ItemData *new_item_from_file(gchar *file, gint sections, gint x, gint y);
static void free_item(ItemData *item);
static SkinData *new_skin();
static void draw_digit(DigitData *digit, gint n, gint x, gint y, AppData *ad);

static ButtonData *new_button(SkinData *skin, ItemData *item,
			      void (*click_func)(gpointer), void (*redraw_func)(gpointer));
static void free_button(ButtonData *button);
static void display_motion(GtkWidget *w, GdkEventMotion *event, gpointer data);
static void display_pressed(GtkWidget *w, GdkEventButton *event, gpointer data);
static void display_released(GtkWidget *w, GdkEventButton *event, gpointer data);
static void display_leave(GtkWidget *w, GdkEventCrossing *event, gpointer data);

static void free_hand(HandData *hd);
static HandData *new_hand(GdkPixmap *p, GdkBitmap *m, gint xo, gint yo);
static HandData *new_hand_from_data(gchar **data, gint xo, gint yo);
static HandData *new_hand_from_file(gchar *file, gint xo, gint yo);
static void free_clock(AnalogData *c);
static AnalogData *new_clock(HandData *h, HandData *m, HandData *s,
			     GdkPixmap *back, gint x, gint y);
static AnalogData *new_clock_from_data(HandData *h, HandData *m, HandData *s,
			gchar **data, gint x, gint y, gint width, gint height,
			AppData *ad, GdkPixmap *large_back);
static void draw_hand(HandData *h, gint x, gint y, gint r_deg, GdkImage *img, AppData *ad);
static AnalogData *new_clock_from_file(HandData *h, HandData *m, HandData *s,
				       gchar *file, gint x, gint y);

static SkinData *load_default_skin(AppData *ad);
static ItemData *get_item(gchar *path, gchar *name, gint sections);
static DigitData *get_digit(gchar *path, gchar *name);
static NumberData *get_number(gchar *path, gchar *name, gint count, gint zeros, SkinData *skin);
static GtkWidget *get_background(gchar *path);
static SkinData *load_skin(gchar *skin_path, gint vertical, gint size, AppData *ad);
static SkinData *load_best_skin_match(gchar *path, gint vertical, gint size, AppData *ad);

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

static NumberData *new_number(DigitData *digits, gint length, gint zeros, gint centered, gint x, gint y)
{
	NumberData *number;
	number = g_new0(NumberData, 1);

	number->digits = digits;
	number->length = length;
	number->zeros = zeros;
	number->centered = centered;
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

	gtk_drawing_area_size(GTK_DRAWING_AREA(ad->display_area), ad->skin->width, ad->skin->height);
	gtk_widget_set_usize(ad->display_area, ad->skin->width, ad->skin->height);
	gtk_widget_set_usize(ad->applet, ad->skin->width, ad->skin->height);

	if (ad->skin->mail)
		ad->mail_sections = ad->skin->mail->sections;
	else
		ad->mail_sections = 1;

	draw_button(ad->skin->button, FALSE, FALSE, TRUE, ad);
	redraw_skin(ad);

	/* attempt to set the applets shape */
	gdk_window_shape_combine_mask (ad->display_area->window, ad->skin->mask, 0, 0);

	/* force status redraws */
	ad->old_week = -1;
	ad->old_n = -1;
	ad->old_amount = -1;
}

void free_skin(SkinData *s)
{
	if (!s) return;
	if(s->pixmap) gtk_widget_destroy(s->pixmap);
	free_number(s->hour);
	free_number(s->min);
	free_number(s->sec);
	free_number(s->month);
	free_number(s->day);
	free_number(s->year);
	free_number(s->mail_count);
	free_number(s->messages);
	free_digit(s->dig_small);
	free_digit(s->dig_large);
	free_item(s->mail);
	free_item(s->month_txt);
	free_item(s->week_txt);
	free_item(s->mail_amount);
	free_item(s->button_pix);
	free_button(s->button);
	free_clock(s->clock);
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

	digit = number->digits;
	x = number->x + (number->length * digit->width);
	z = number->zeros;
	
	for (i=0; i< number->length - 1; i++) t *= 10;

	if (number->centered && n < t)
		{
		gint b = FALSE;
		for (i=number->length; i > 0; i--)
			{
			draw_digit(digit, -1, x - (i * digit->width), number->y, ad);
			}
		for (i=number->length; i > 0; i--)
			{
			d = n / t;
			n -= d * t;
			t = t / 10;
			if (!(d == 0 && i>1 && !b))
				{
				if (!b)
					{
					x = number->x + (number->length * digit->width / 2) + (i * digit->width / 2);
					b = TRUE;
					}
				draw_digit(digit, d, x - (i * digit->width), number->y, ad);
				}
			}
		return;
		}

	for (i=number->length; i > 0; i--)
		{
		d = n / t;
		n -= d * t;
		t = t / 10;
		if (d == 0 && i>1 && (!z))
			{
			d = -1;
			}
		else
			{
			z = TRUE;
			}
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

/*
 *--------------------------------------------------------------------
 * button functions
 *--------------------------------------------------------------------
 */

static ButtonData *new_button(SkinData *skin, ItemData *item,
			      void (*click_func)(gpointer), void (*redraw_func)(gpointer))
{
	ButtonData *button;

	button = g_new0(ButtonData, 1);

	button->item = item;
	if (button->item)
		{
		if (item->sections == 3)
			button->has_prelight = TRUE;
		else
			button->has_prelight = FALSE;

		button->width = item->width;
		button->height = item->height;
		button->x = item->x;
		button->y = item->y;
		}
	else
		{
		/* we create this widget anyway so that all themes
		 * support clicking, some just will not have visual feedback.
		 */
		button->has_prelight = FALSE;
		button->width = skin->width;
		button->height = skin->height;
		button->x = 0;
		button->y = 0;
		}

	button->pushed = FALSE;
	button->prelit = FALSE;

	button->click_func = click_func;
	button->redraw_func = redraw_func;

	return button;
}

static void free_button(ButtonData *button)
{
	if (!button) return;

	g_free(button);
}

void draw_button(ButtonData *button, gint prelight, gint pressed, gint force, AppData *ad)
{
	gint redraw = FALSE;
	if (!button) return;

	if (force)
		{
		redraw = TRUE;
		}
	else
		{
		if (button->has_prelight && prelight != button->prelit)
			{
			if (prelight)
				button->prelit = !pressed;
			else
				button->prelit = FALSE;
			redraw = TRUE;
			}
		if (button->pushed != pressed)
			{
			button->pushed = pressed;
			redraw = TRUE;
			}
		}
	if (button->item && redraw)
		{
		draw_item(button->item, button->pushed + (button->prelit * 2), ad);

		/* bah, now everything else needs redrawn because they may be within
		 * the button.
		 */
		if (button->redraw_func)
			button->redraw_func(ad);

		redraw_skin(ad);
		}
}

static void display_motion(GtkWidget *w, GdkEventMotion *event, gpointer data)
{
	AppData *ad = data;
	ButtonData *button;
	gint x = (float)event->x;
	gint y = (float)event->y;

	button = ad->skin->button;
	if (!button) return;

	if (x >= button->x && x < button->x + button->width &&
				y >= button->y && y < button->y + button->height)
		{
		if (ad->active == button)
			draw_button(button, FALSE, TRUE, FALSE, ad);
		else
			draw_button(button, TRUE, FALSE, FALSE, ad);
		}
	else
		{
		draw_button(button, FALSE, FALSE, FALSE, ad);
		}
}

static void display_pressed(GtkWidget *w, GdkEventButton *event, gpointer data)
{
	AppData *ad = data;
	ButtonData *button;
	gint x = (float)event->x;
	gint y = (float)event->y;

	button = ad->skin->button;
	if (!button) return;

	if (x >= button->x && x < button->x + button->width &&
				y >= button->y && y < button->y + button->height)
		{
		ad->active = button;
		draw_button(button, FALSE, TRUE, FALSE, ad);
		}
}

static void display_released(GtkWidget *w, GdkEventButton *event, gpointer data)
{
	AppData *ad = data;
	ButtonData *button;
	gint x = (float)event->x;
	gint y = (float)event->y;


	if (!ad->active) return;
	button = ad->active;
	ad->active = NULL;

	if (button->pushed && x >= button->x && x < button->x + button->width &&
				y >= button->y && y < button->y + button->height)
		{
		draw_button(button, FALSE, FALSE, FALSE, ad);
		if (button->click_func)
			button->click_func(ad);
		}
}

static void display_leave(GtkWidget *w, GdkEventCrossing *event, gpointer data)
{
	AppData *ad = data;
	ButtonData *button;

	button = ad->skin->button;
	if (!button) return;

	draw_button(button, FALSE, FALSE, FALSE, ad);
}

void skin_event_init(AppData *ad)
{
	gtk_widget_set_events (ad->display_area, GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK | GDK_POINTER_MOTION_MASK |
			GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
	gtk_signal_connect(GTK_OBJECT(ad->display_area),"motion_notify_event",(GtkSignalFunc) display_motion, ad);
	gtk_signal_connect(GTK_OBJECT(ad->display_area),"button_press_event",(GtkSignalFunc) display_pressed, ad);
	gtk_signal_connect(GTK_OBJECT(ad->display_area),"button_release_event",(GtkSignalFunc) display_released, ad);
	gtk_signal_connect(GTK_OBJECT(ad->display_area),"leave_notify_event",(GtkSignalFunc) display_leave, ad);
}

/*
 *--------------------------------------------------------------------
 * analog clock functions
 *--------------------------------------------------------------------
 */

static void free_hand(HandData *hd)
{
	if (!hd) return;

	gdk_image_destroy(hd->image);
	if (hd->mask) free(hd->mask);

	g_free(hd);
}

static HandData *new_hand(GdkPixmap *p, GdkBitmap *m, gint xo, gint yo)
{
	HandData *hd;
	gint width, height;

	if (!p) return NULL;

	hd = g_new0(HandData, 1);

	gdk_window_get_size(p, &width, &height);
	hd->image = gdk_image_get(p, 0, 0, width, height);

	if (m)
		{
		GdkImage *mi = NULL;
		gdk_window_get_size(m, &width, &height);
		mi = gdk_image_get(m, 0, 0, width, height);
		hd->mask = get_map(mi);
		gdk_image_destroy(mi);
		}
	else
		{
		hd->mask = NULL;
		}

	hd->xo = xo;
	hd->yo = yo;

	if (p) gdk_imlib_free_pixmap(p);
	if (m) gdk_imlib_free_bitmap(m);

	return hd;
}

static HandData *new_hand_from_data(gchar **data, gint xo, gint yo)
{
	GdkPixmap *p = NULL;
	GdkBitmap *m = NULL;

	gdk_imlib_data_to_pixmap(data, &p, &m);

	return new_hand(p, m, xo, yo);
}

static HandData *new_hand_from_file(gchar *file, gint xo, gint yo)
{
	GdkPixmap *p = NULL;
	GdkBitmap *m = NULL;

	gdk_imlib_load_file_to_pixmap(file, &p, &m);

	return new_hand(p, m, xo, yo);
}

static void free_clock(AnalogData *c)
{
	if (!c) return;

	free_hand(c->hour);
	free_hand(c->minute);
	free_hand(c->second);
	gdk_imlib_free_pixmap(c->back);
	g_free(c);
}

static AnalogData *new_clock(HandData *h, HandData *m, HandData *s,
			     GdkPixmap *back, gint x, gint y)
{
	AnalogData *c;
	gint width;
	gint height;

	if (!back) return NULL;
	gdk_window_get_size(back, &width, &height);

	c = g_new0(AnalogData, 1);

	c->hour = h;
	c->minute = m;
	c->second = s;

	c->back = back;
	c->width = width;
	c->height = height;
	c->x = x;
	c->y = y;
	c->cx = width / 2;
	c->cy = height / 2;

	return c;
}

static AnalogData *new_clock_from_data(HandData *h, HandData *m, HandData *s,
			gchar **data, gint x, gint y, gint width, gint height,
			AppData *ad, GdkPixmap *large_back)
{
	GdkPixmap *back = NULL;

	if (data)
		{
		back = get_pixmap_from_data(data);
		if (back) gdk_window_get_size(back, &width, &height);
		}
	else
		{
		back = gdk_pixmap_new (ad->applet->window, width, height, -1);
		gdk_draw_pixmap(back,
			ad->display_area->style->fg_gc[GTK_WIDGET_STATE(ad->display_area)],
			large_back, x, y, 0, 0, width, height);
		}

	return new_clock(h, m, s, back, x, y);
}

static AnalogData *new_clock_from_file(HandData *h, HandData *m, HandData *s,
				       gchar *file, gint x, gint y)
{
	GdkPixmap *back;

	back = get_pixmap_from_file(file);
	if (!back) return NULL;

	return new_clock(h, m, s, back, x, y);
}

static void draw_hand(HandData *h, gint x, gint y, gint r_deg, GdkImage *img, AppData *ad)
{
	if (!h) return;

	draw_image_rotated(img, h->image, h->mask,
			    x, y, h->xo, h->yo, r_deg, ad->display_area->window);
}

void draw_clock(AnalogData *c, gint h, gint m, gint s, AppData *ad)
{
	GdkImage *img = NULL;

	if (!c) return;

	img = gdk_image_get(c->back, 0, 0, c->width, c->height);

	draw_hand(c->hour,  c->cx, c->cy, h * 30 + ((float)m * 0.5), img, ad);
	draw_hand(c->minute, c->cx, c->cy, m * 6 + ((float)s * 0.1), img, ad);
	draw_hand(c->second, c->cx, c->cy, s * 6, img, ad);

	gdk_draw_image (ad->skin->background,
		ad->display_area->style->fg_gc[GTK_WIDGET_STATE(ad->display_area)],
		img, 0, 0, c->x, c->y, c->width, c->height);

	gdk_image_destroy(img);
}

/*
 *--------------------------------------------------------------------
 * skin loaders
 *--------------------------------------------------------------------
 */

static SkinData *load_default_skin(AppData *ad)
{
	SkinData *s;
	gint width, height;

	s = new_skin();

	if (ad->sizehint == SIZEHINT_TINY &&
	    (ad->orient == ORIENT_UP || ad->orient == ORIENT_DOWN))
		{
		/* this is the tiny size theme for a horizonatal panel*/

		s->pixmap = gnome_pixmap_new_from_xpm_d(backgrnd_tiny_xpm);
		s->background = GNOME_PIXMAP(s->pixmap)->pixmap;
		s->mask = GNOME_PIXMAP(s->pixmap)->mask;

		gdk_window_get_size (s->background, &width, &height);

		s->width = width;
		s->height = height;

		s->dig_large = new_digit_from_data((gchar **)digmed_xpm);
		s->dig_small = new_digit_from_data((gchar **)digsml_xpm);

		s->hour = new_number(s->dig_large, 2, FALSE, FALSE, 3, 4);
		s->min = new_number(s->dig_large, 2, TRUE, FALSE, 26, 4);

		s->messages = new_number(s->dig_small, 3, FALSE, FALSE, 72, 8);

		s->mail = new_item_from_data((gchar **)mailpics_xpm, 10, 49, 5);

		s->button_pix = new_item_from_data((gchar **)button_tiny_xpm, 3, 0, 0);
		s->button = new_button(s, s->button_pix, launch_mail_reader, redraw_all);

		return s;
		}

	/* this is the standard size theme */
	s->pixmap = gnome_pixmap_new_from_xpm_d(backgrnd_xpm);
	s->background = GNOME_PIXMAP(s->pixmap)->pixmap;
	s->mask = GNOME_PIXMAP(s->pixmap)->mask;

	gdk_window_get_size (s->background, &width, &height);

	s->width = width;
	s->height = height;

	s->dig_large = new_digit_from_data((gchar **)digmed_xpm);
	s->dig_small = new_digit_from_data((gchar **)digsml_xpm);

	s->hour = new_number(s->dig_large, 2, FALSE, FALSE, 3, 5);
	s->min = new_number(s->dig_large, 2, TRUE, FALSE, 26, 5);

	s->messages = new_number(s->dig_small, 3, FALSE, FALSE, 26, 32);

	s->mail = new_item_from_data((gchar **)mailpics_xpm, 10, 3, 29);

	s->button_pix = new_item_from_data((gchar **)button_xpm, 3, 0, 0);
	s->button = new_button(s, s->button_pix, launch_mail_reader, redraw_all);

	return s;
}

/* the following are functions for loading external skins */

static HandData *get_hand(gchar *path, gchar *name)
{
	HandData *h;
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
	
	h = new_hand_from_file(filename, xo, yo);
	g_free(filename);

	return h;
}

static AnalogData *get_clock(gchar *path, SkinData *s, AppData *ad)
{
	AnalogData *c;
	HandData *hd;
	HandData *md;
	HandData *sd;
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

	hd = get_hand(path, "Clock_Hour_Hand=");
	md = get_hand(path, "Clock_Minute_Hand=");
	sd = get_hand(path, "Clock_Second_Hand=");

	if (filename)
		{
		c = new_clock_from_file(hd, md, sd, filename, x, y);
		g_free(filename);
		}
	else
		{
		c = new_clock_from_data(hd, md, sd, NULL, x, y, w, h, ad, s->background);
		}

	return c;

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

	item = new_item_from_file(filename, sections, x, y);
	g_free(filename);
	return item;
}

static DigitData *get_digit(gchar *path, gchar *name)
{
	DigitData *digit;
	gchar *filename;
	gchar *buf = NULL;

	buf = gnome_config_get_string(name);

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

	number = new_number(digit, count, zeros, centered, x, y);
	g_free(filename);
	return number;
}

static GtkWidget *get_background(gchar *path)
{
	GtkWidget *pixmap = NULL;
	gchar *buf = NULL;
	gchar *filename;

	buf = gnome_config_get_string("Background=");

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

static SkinData *load_skin(gchar *skin_path, gint vertical, gint size, AppData *ad)
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
	s = new_skin();

	switch (size)
		{
		case SIZEHINT_TINY:
			stext = "_Tiny";
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

	s->pixmap = NULL;
	s->pixmap = get_background(skin_path);
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
		gnome_config_pop_prefix();
		return NULL;
		}

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

	s->button = new_button(s, s->button_pix, launch_mail_reader, redraw_all);

	s->clock = get_clock(skin_path, s, ad);

	g_free(datafile);
	gnome_config_pop_prefix();
	return s;
}

static SkinData *load_best_skin_match(gchar *path, gint vertical, gint size, AppData *ad)
{
	SkinData *s = NULL;

	s = load_skin(path, vertical, size, ad);
	if (s) return s;

	/* huge falls back to large, then standard */
	if (size == SIZEHINT_HUGE)
		{
		s = load_skin(path, vertical, SIZEHINT_LARGE, ad);
		if (s) return s;
		return load_skin(path, vertical, SIZEHINT_STANDARD, ad);
		}

	if (size == SIZEHINT_STANDARD) return NULL;

	/* if not standard and failed so far, try to fall back to it */
	return load_skin(path, vertical, SIZEHINT_STANDARD, ad);
}

gint change_to_skin(gchar *path, AppData *ad)
{
	SkinData *new_s = NULL;
	SkinData *old_s = ad->skin;

	if (!path)
		{
		new_s = load_default_skin(ad);
		}
	else
		{
		new_s = load_best_skin_match(path,
					     (ad->orient == ORIENT_LEFT || ad->orient == ORIENT_RIGHT),
					     ad->sizehint, ad);
		if (!new_s)
			{
			new_s = load_skin(path,
					  !(ad->orient == ORIENT_LEFT || ad->orient == ORIENT_RIGHT),
					  SIZE_STANDARD, ad);
			}
		}

	if (!new_s) return FALSE;

	ad->skin = new_s;

	sync_window_to_skin(ad);
	ad->active = NULL;

	free_skin(old_s);

	return TRUE;
}
