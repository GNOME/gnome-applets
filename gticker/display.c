/*###################################################################*/
/*##                                                               ##*/
/*###################################################################*/

#include "gticker.h"
#include "back.xpm"
#include "noimage.xpm"

static	InfoData *create_info_line(gchar *text,  gchar *icon_path, GtkWidget *icon,
				gint offset, gint center, gint show_count, gint delay);
static void free_info_line(AppData *ad, InfoData *id);
static gint check_info_for_removal(AppData *ad, InfoData *id);
static GList *next_info_line(AppData *ad);
static void redraw_display(AppData *ad);
static void draw_pixmap(AppData *ad, GdkPixmap *smap, GdkPixmap *tmap, gint x, gint y, gint w, gint h, gint xo, gint yo);
static void scroll_display_up(AppData *ad, gint m);
static void draw_display_line(AppData *ad);
static gint update_display_cb(gpointer data);
static void register_click_func(AppData *ad, InfoData *id);
static void scroll_click_func(AppData *ad, gint m);
static int display_click(GtkWidget *w, GdkEventButton *event, gpointer data);
static void display_motion(GtkWidget *w, GdkEventMotion *event, gpointer data);

static	InfoData *create_info_line(gchar *text,  gchar *icon_path, GtkWidget *icon,
				gint offset, gint center, gint show_count, gint delay)
{
	InfoData *id;
	if (!text) return NULL;
	id = g_new0(InfoData, 1);

	id->click_func = NULL;
	id->data = NULL;
	id->free_func = NULL;

	id->text = g_strdup(text);

	if (icon_path)
		id->icon_path = g_strdup(icon_path);
	else
		id->icon_path = NULL;

	if (icon)
		{
		gint width, height;
		id->icon = icon;
		gdk_window_get_size (GNOME_PIXMAP(id->icon)->pixmap, &width, &height);
		id->icon_w = width;
		id->icon_h = height;
		}
	else
		{
		id->icon = NULL;
		id->icon_w = 0;
		id->icon_h = 0;
		}

	id->length = strlen(text);

	if (id->icon_w == 0 || offset > id->icon_w + 4)
		id->offset = offset;
	else
		{
		if (id->icon_w > 0)
			id->offset = id->icon_w + 4;
		else
			id->offset = 0;
		}

	id->center = center;

	id->shown = FALSE;
	id->show_count = show_count;

	id->end_delay = delay;

	return id;
}

static void free_info_line(AppData *ad, InfoData *id)
{
	GList *list = ad->click_list;

	if (!id) return;

	while(list)
		{
		GList *w = list;
		ClickData *cd = w->data;
		list = list->next;

		if (cd->line_id == id)
			{
			ad->click_list = g_list_remove(ad->click_list, cd);
			g_free(cd);
			}
		}

	g_free(id->text);
	g_free(id->icon_path);

	if (id->icon) gtk_widget_destroy(id->icon);

	if (id->data && id->free_func) id->free_func(id->data);

	g_free(id);
}

void free_all_info_lines(AppData *ad)
{
	GList *list = ad->text;

	if (!list) return;
	while(list)
		{
		InfoData *id = list->data;
		free_info_line(ad, id);
		list = list->next;
		}
	g_list_free(ad->text);
	ad->text = NULL;
}

InfoData *add_info_line(AppData *ad, gchar *text, gchar *icon_path, gint offset, gint center,
		   gint show_count, gint delay)
{
	InfoData *id;
	GtkWidget *icon = NULL;

	if (icon_path)
		{
		if (g_file_exists(icon_path))
			icon = gnome_pixmap_new_from_file(icon_path);
		if (!icon)
			icon = gnome_pixmap_new_from_xpm_d(noimage_xpm);
		}

	id = create_info_line(text, icon_path, icon, offset, center, show_count, delay);
	if (id)
		{
		ad->text = g_list_append(ad->text, id);
		ad->text_lines++;
		}

	return id;
}

InfoData *add_info_line_with_pixmap(AppData *ad, gchar *text, GtkWidget *icon, gint offset, gint center,
		   gint show_count, gint delay)
{
	InfoData *id;
	id = create_info_line(text, NULL, icon, offset, center, show_count, delay);
	if (id)
		{
		ad->text = g_list_append(ad->text, id);
		ad->text_lines++;
		}

	return id;
}

void remove_info_line(AppData *ad, InfoData *id)
{
	if (!id) return;
	if (ad->current_text && ad->current_text->data == id)
		{
		/* if current line is being displayed, schedule to remove */
		id->show_count = 1;
		}
	else
		{
		ad->text = g_list_remove(ad->text, id);
		free_info_line(ad, id);
		ad->text_lines--;
		}
}

void remove_all_lines(AppData *ad)
{
	GList *list;

	list = ad->text;
	while (list)
		{
		InfoData *id = list->data;
		list = list->next;
		remove_info_line(ad, id);
		}
}

void set_info_click_signal(InfoData *id, void (*click_func)(AppData *ad, gpointer data),
		gpointer data, void (*free_func)(gpointer data))
{
	if (!id) return;
	id->click_func = click_func;
	id->data = data;
	id->free_func = free_func;
}

static gint check_info_for_removal(AppData *ad, InfoData *id)
{
	if (id->show_count > 0)
		{
		id->show_count--;
		if (id->show_count == 0)
			{
			remove_info_line(ad, id);
			if (!ad->text)
				{
				ad->current_text = NULL;
				return TRUE;
				}
			}
		}
	return FALSE;
}

static GList *next_info_line(AppData *ad)
{
	GList *list = ad->text;

	/* check for the first unshown info item, they have priority*/
	while(list)
		{
		InfoData *id = list->data;
		if (!id->shown) return list;
		list = list->next;
		}

	/* all have been shown, just return the next line */
	list = ad->current_text->next;
	if (!list) list = ad->text;
	return list;
}

static void redraw_display(AppData *ad)
{
	gdk_window_set_back_pixmap(ad->draw_area->window,ad->display,FALSE);
	gdk_window_clear(ad->draw_area->window);
}

static void draw_pixmap(AppData *ad, GdkPixmap *smap, GdkPixmap *tmap, gint x, gint y, gint w, gint h, gint xo, gint yo)
{
	gdk_draw_pixmap(tmap,
                        ad->draw_area->style->fg_gc[GTK_WIDGET_STATE(ad->draw_area)],
                        smap, xo, yo, x, y, w, h);
}

static void scroll_display_up(AppData *ad, gint m)
{
	draw_pixmap (ad, ad->display, ad->disp_buf, 0, 0, ad->width, ad->height - m, 0, m);
	draw_pixmap (ad, ad->disp_buf, ad->display, 0, 0, ad->width, ad->height - m, 0, 0);
	draw_pixmap (ad, ad->background, ad->display, 0, ad->height - m, ad->width, m, 0, ad->height - m);
	if (ad->current_text)
		{
		InfoData *id = ad->current_text->data;
		if (id->icon && id->icon_h > ad->current_line_pos * ad->scroll_height)
			{
			gint x,y;
			gint xo, yo;
			gint w, h;

			x = 2;
			y = ad->height - m;
			xo = 0;
			if (m == ad->scroll_height)
				yo = ad->current_line_pos * ad->scroll_height;
			else
				yo = (ad->current_line_pos * ad->scroll_height) + ad->scroll_height - ad->scroll_pos;
			w = id->icon_w;
			h = m;
			if (h + yo > id->icon_h) h = id->icon_h - yo;
			if (yo < id->icon_h && h > 0)
				draw_pixmap (ad, GNOME_PIXMAP(id->icon)->pixmap, ad->display,
						x, y, w, h, xo, yo);
			}
		}
	redraw_display(ad);
	scroll_click_func(ad, m);
}

static void draw_display_line(AppData *ad)
{
	InfoData *id;
	gint new_x_pos;
	gchar c;
	GdkFont *font;

	if (!ad->text) return;
	if (!ad->current_text) ad->current_text = ad->text;
	id = ad->current_text->data;
	if (!id) return;

	font = ad->draw_area->style->font;

	if (ad->new_line)
		{
		if (ad->text_pos > id->length)
			{
			GList *temp;
			if (id->icon && id->icon_h > ad->current_line_pos * ad->text_height)
				{
				ad->scroll_pos = ad->scroll_height;
				ad->current_line_pos ++;
				if (id->icon_h <= ad->current_line_pos * ad->text_height)
					ad->scroll_count = id->end_delay / 10 * (1000 / UPDATE_DELAY);
				return;
				}
			id->shown = TRUE;
			ad->text_pos = 0;
			temp = next_info_line(ad);
			ad->current_text = NULL;
			ad->current_line_pos = 0;
			check_info_for_removal(ad, id);
			if (!ad->text || !temp) return;
			ad->current_text = temp;
			id = ad->current_text->data;
			}
		ad->new_line = FALSE;
		ad->x_pos = 0;
		if (id->icon && id->icon_h > ad->current_line_pos * ad->text_height)
			{
			gint x,y;
			gint xo, yo;
			gint w, h;
			x = 2;
			y = ad->height - ad->text_height;
			xo = 0;
			yo = ad->current_line_pos * ad->text_height;
			w = id->icon_w;
			h = id->icon_h - (ad->current_line_pos * ad->text_height);
			draw_pixmap (ad, GNOME_PIXMAP(id->icon)->pixmap, ad->display, x, y, w, h, xo, yo);
			}
		if (id->center && !id->icon)
			{
			gint l = gdk_text_width(font, id->text, id->length);
			if (l < ad->width)
				{
				ad->x_pos = ( ad->width - l ) / 2;
				}
			else
				{
				ad->x_pos = id->offset;
				}
			}
		else
			ad->x_pos = id->offset;

		/* add the click check */
		if (id->click_func && ad->current_line_pos == 0)
			{
			register_click_func(ad, id);
			}
		}

	c = id->text[ad->text_pos];
	if (ad->x_pos == id->offset && c == ' ' &&
			ad->text_pos < id->length - 1 && id->text[ad->text_pos +1] != ' ')
		{
		ad->text_pos++;
		c = id->text[ad->text_pos];
		}
	new_x_pos = ad->x_pos + gdk_char_width (font, c);

	gdk_draw_text(ad->display, font,
			ad->draw_area->style->fg_gc[GTK_WIDGET_STATE(ad->draw_area)],
			ad->x_pos, ad->height - ad->text_y_line, id->text + ad->text_pos, 1);

	ad->text_pos ++;
	if (ad->text_pos > id->length)
		{
		ad->new_line = TRUE;
		}
	else
		{
		ad->x_pos = new_x_pos;
		c = id->text[ad->text_pos];
		if (ad->x_pos + gdk_char_width (font, c) > ad->width)
			{
			ad->new_line = TRUE;
			}
		else if (c == '\n')
			{
			ad->text_pos++;
			ad->new_line = TRUE;
			}
		else if (c == ' ' && ad->text_pos < id->length)
			{
			gint word_length;
			gint l = strlen(id->text);
			gint p;
			p = ad->text_pos + 1;
			while (p < l && id->text[p] != ' ' && id->text[p] != '\n') p++;
			word_length = gdk_text_width(font, id->text + ad->text_pos,
						     p - ad->text_pos);
			if (ad->x_pos + word_length > ad->width)
				{
				ad->text_pos++;
				ad->new_line = TRUE;
				}
			}
		}

	if (ad->new_line)
		{
		ad->current_line_pos ++;
		ad->scroll_pos = ad->scroll_height;

		if (ad->text_pos > id->length)
			ad->scroll_count = id->end_delay / 10 * (1000 / UPDATE_DELAY);
		else		
			ad->scroll_count = ad->scroll_delay / 10 * (1000 / UPDATE_DELAY);

		}
}

static gint update_display_cb(gpointer data)
{
	AppData *ad = data;
	if (!ad) return FALSE;

	if (ad->scroll_count > 0)
		{
		ad->scroll_count --;
		return TRUE;
		}

	if (ad->new_line && ad->scroll_pos > 0)
		{
		/* draw the scrolling */
		if (ad->smooth_scroll)
			{
			gint s = ad->scroll_speed;
			if (ad->scroll_pos - s < 1) s = ad->scroll_pos - 1;
			scroll_display_up(ad, s);
			}
		else
			{
			if (ad->scroll_pos - ad->scroll_speed < 1)
				scroll_display_up(ad, ad->scroll_height);
			}
		ad->scroll_pos -= ad->scroll_speed;
		return TRUE;
		}

	if (ad->smooth_type)
		{
		draw_display_line(ad);
		redraw_display(ad);
		}
	else
		{
		draw_display_line(ad);
		while(!ad->new_line) draw_display_line(ad);
		redraw_display(ad);
		}
	return TRUE;
}

void set_mouse_cursor (AppData *ad, gint icon)
{
        GdkCursor *cursor = gdk_cursor_new (icon);
        gdk_window_set_cursor (ad->draw_area->window, cursor);
        gdk_cursor_destroy (cursor);
}

static void register_click_func(AppData *ad, InfoData *id)
{
	ClickData *cd;

	if (!id || !id->click_func) return;

	cd = g_new0(ClickData, 1);

	cd->line_id = id;
	cd->click_func = id->click_func;
	cd->data = id->data;

	cd->x = 0;
	cd->y = ad->height - ad->scroll_height;
	cd->w = ad->width;
	cd->h = ad->scroll_height;

	ad->click_list = g_list_append(ad->click_list, cd);

	scroll_click_func(ad, 0);
}

static void scroll_click_func(AppData *ad, gint m)
{
	GList *list = ad->click_list;
	gint proximity = FALSE;

	while(list)
		{
		GList *w = list;
		ClickData *cd = w->data;
		list = list->next;

		cd->y -= m;

		if (ad->current_text)
			{
			InfoData *id = ad->current_text->data;
			if (id == cd->line_id && ad->text_pos <= id->length) cd->h += m;
			}

		if (cd->y + cd->h < 1)
			{
			ad->click_list = g_list_remove(ad->click_list, cd);
			g_free(cd);
			}
		else
			{
			GdkModifierType modmask;
			gint x, y;
			gdk_window_get_pointer (ad->draw_area->window, &x, &y, &modmask);
			if (x >= cd->x && x < cd->x + cd->w && y >= cd->y && y < cd->y + cd->h)
				{
				proximity = TRUE;
				}
			}
		}

	if (proximity)
		set_mouse_cursor (ad, GDK_HAND2);
	else
		set_mouse_cursor (ad, GDK_LEFT_PTR);
}

static int display_click(GtkWidget *w, GdkEventButton *event, gpointer data)
{
	AppData *ad = data;
	GList *list;
	gint x = (float)event->x;
	gint y = (float)event->y;

	if (event->button != 1)
		{
		return FALSE;
		}

	list = ad->click_list;
	while(list)
		{
		ClickData *cd = list->data;

		if (x >= cd->x && x < cd->x + cd->w && y >= cd->y && y < cd->y + cd->h)
			{
			if (cd->click_func) cd->click_func(ad, cd->data);
			}

		list = list->next;
		}

	return TRUE;
}

static void display_motion(GtkWidget *w, GdkEventMotion *event, gpointer data)
{
	AppData *ad = data;
	GList *list;
	gint proximity = FALSE;
	gint x = (float)event->x;
	gint y = (float)event->y;

	list = ad->click_list;
	while(list)
		{
		ClickData *cd = list->data;

		if (x >= cd->x && x < cd->x + cd->w && y >= cd->y && y < cd->y + cd->h)
			{
			proximity = TRUE;
			}

		list = list->next;
		}

	if (proximity)
		set_mouse_cursor (ad, GDK_HAND2);
	else
		set_mouse_cursor (ad, GDK_LEFT_PTR);
}

void init_app_display(AppData *ad)
{
	gint w, h;

	ad->scroll_delay = 20;
	ad->current_line = 0;
	ad->current_line_pos = 0;
	ad->scroll_count = 0;
	ad->scroll_pos = 0;
	ad->new_line = TRUE;
	ad->text_pos = 0;

	ad->text = NULL;
	ad->current_text = NULL;
	ad->click_list = NULL;

	ad->display_w = gnome_pixmap_new_from_xpm_d(back_xpm);
	ad->display = GNOME_PIXMAP(ad->display_w)->pixmap;
	ad->disp_buf_w = gnome_pixmap_new_from_xpm_d(back_xpm);
	ad->disp_buf = GNOME_PIXMAP(ad->disp_buf_w)->pixmap;
	ad->background_w = gnome_pixmap_new_from_xpm_d(back_xpm);
	ad->background = GNOME_PIXMAP(ad->background_w)->pixmap;

	gdk_window_get_size(ad->display, &w, &h);

	ad->width = w;
	ad->height = h;

	ad->frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(ad->frame), GTK_SHADOW_IN);
	gtk_widget_show(ad->frame);

	ad->draw_area = gtk_drawing_area_new();
	gtk_drawing_area_size(GTK_DRAWING_AREA(ad->draw_area), w, h);
	gtk_widget_set_events (ad->draw_area, GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK);
	gtk_signal_connect(GTK_OBJECT(ad->draw_area),"motion_notify_event",
		(GtkSignalFunc) display_motion, ad);
	gtk_signal_connect(GTK_OBJECT(ad->draw_area),"button_press_event",
		(GtkSignalFunc) display_click, ad);
	gtk_container_add(GTK_CONTAINER(ad->frame), ad->draw_area);
        gtk_widget_show(ad->draw_area);

        applet_widget_add(APPLET_WIDGET(ad->applet), ad->frame);

        gtk_widget_realize(ad->draw_area);

	ad->text_height = ad->draw_area->style->font->ascent +
		ad->draw_area->style->font->descent;
	ad->text_y_line = ad->draw_area->style->font->descent;

	ad->scroll_height = ad->text_height;

	redraw_display(ad);

	ad->display_timeout_id = gtk_timeout_add(UPDATE_DELAY, (GtkFunction)update_display_cb, ad);
}

