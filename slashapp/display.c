/* tick-a-stat, a GNOME panel applet
 * (C) 1999 John Ellis
 *
 * Author: John Ellis
 *
 */

#include "noimage.xpm"
#include "slashapp.h"

static void redraw_display(AppData *ad);
static void draw_pixmap(AppData *ad, GdkPixmap *smap, GdkBitmap *mask, GdkPixmap *tmap, gint x, gint y, gint w, gint h, gint xo, gint yo);

static InfoData *create_info_line(const gchar *text,  const gchar *icon_path, GtkWidget *icon,
				gint offset, gint center, gint show_count, gint delay);
static void free_info_line(AppData *ad, InfoData *id);


static InfoData *next_info_line(AppData *ad);

static void scroll_display_up(AppData *ad, gint m);
static gint draw_display_line(AppData *ad);
static gint update_display_cb(gpointer data);


static gint line_is_in_click_list(AppData *ad, InfoData *id);
static void register_click_func(AppData *ad, InfoData *id);
static void scroll_click_func(AppData *ad, gint m);
static int display_click(GtkWidget *w, GdkEventButton *event, gpointer data);
static void display_motion(GtkWidget *w, GdkEventMotion *event, gpointer data);

static void calc_display_sizes(AppData *ad);
static void create_display_pixmaps(AppData *ad);

/*
 *----------------------------------------------------------------------------
 * display generic drawing
 *----------------------------------------------------------------------------
 */

static void redraw_display(AppData *ad)
{
	gdk_window_set_back_pixmap(ad->draw_area->window,ad->display,FALSE);
	gdk_window_clear(ad->draw_area->window);
}

static void draw_pixmap(AppData *ad, GdkPixmap *smap, GdkBitmap *mask, GdkPixmap *tmap, gint x, gint y, gint w, gint h, gint xo, gint yo)
{
	gdk_gc_set_clip_mask(ad->draw_area->style->fg_gc[GTK_WIDGET_STATE(ad->draw_area)], mask);
	gdk_gc_set_clip_origin(ad->draw_area->style->fg_gc[GTK_WIDGET_STATE(ad->draw_area)], x - xo, y - yo);

	gdk_draw_pixmap(tmap,
                        ad->draw_area->style->fg_gc[GTK_WIDGET_STATE(ad->draw_area)],
                        smap, xo, yo, x, y, w, h);

	gdk_gc_set_clip_mask(ad->draw_area->style->fg_gc[GTK_WIDGET_STATE(ad->draw_area)], NULL);
	gdk_gc_set_clip_origin(ad->draw_area->style->fg_gc[GTK_WIDGET_STATE(ad->draw_area)], 0, 0);
}

/*
 *----------------------------------------------------------------------------
 * display line addition/removal (private)
 *----------------------------------------------------------------------------
 */

static	InfoData *create_info_line(const gchar *text,  const gchar *icon_path, GtkWidget *icon,
				gint offset, gint center, gint show_count, gint delay)
{
	InfoData *id;
	if (!text) return NULL;
	id = g_new0(InfoData, 1);

	id->click_func = NULL;
	id->data = NULL;
	id->free_func = NULL;
	id->pre_func = NULL;
	id->end_func = NULL;

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
	if (!id) return;

	g_free(id->text);
	id->text = NULL;
	g_free(id->icon_path);
	id->icon_path = NULL;

	if (id->icon) gtk_widget_destroy(id->icon);
	id->icon = NULL;

	if (id->data && id->free_func) id->free_func(id->data);
	id->data = NULL;
	id->free_func = NULL;

	g_free(id);
	return;
	ad = NULL;
}


/*
 *----------------------------------------------------------------------------
 * display line addition/removal (public)
 *----------------------------------------------------------------------------
 */

InfoData *add_info_line(AppData *ad, const gchar *text, const gchar *icon_path, gint offset, gint center,
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
		ad->info_list = g_list_append(ad->info_list, id);
		ad->text = g_list_append(ad->text, id);
		ad->text_lines++;
		}

	return id;
}

InfoData *add_info_line_with_pixmap(AppData *ad, const gchar *text, GtkWidget *icon, gint offset, gint center,
		   gint show_count, gint delay)
{
	InfoData *id;
	id = create_info_line(text, NULL, icon, offset, center, show_count, delay);
	if (id)
		{
		ad->info_list = g_list_append(ad->info_list, id);
		}

	return id;
}

void set_info_click_signal(InfoData *id, void (*click_func)(gpointer data, InfoData *id, AppData *ad),
                gpointer data, void (*free_func)(gpointer data))
{
        if (!id) return;
        id->click_func = click_func;
        id->data = data;
        id->free_func = free_func;
}

/* *** unused ***
static void set_info_signals (InfoData *id, 
			      void (*click_func)(gpointer data, InfoData *id, AppData *ad),
			      void (*free_func)(gpointer data),
			      void (*pre_func)(gpointer data, InfoData *id, AppData *ad),
			      void (*end_func)(gpointer data, InfoData *id, AppData *ad),
			      gpointer data)
{
	if (!id) return;
	id->click_func = click_func;
	id->free_func = free_func;
	id->pre_func = pre_func;
	id->end_func = end_func;
	id->data = data;
}
*/

void remove_info_line(AppData *ad, InfoData *id)
{
	GList *work;
	if (!id) return;

	work = g_list_find(ad->info_list, id);

	if (work)
		{
		ad->info_list = g_list_remove(ad->info_list, id);
		if (!line_is_in_click_list(ad, id))
			{
			if (ad->info_current == id)
				ad->free_current = TRUE;
			else
				free_info_line(ad, id);
			}
		}
}

void remove_all_lines(AppData *ad)
{
	GList *work = ad->info_list;
	while (work)
		{
		InfoData *id = work->data;
		work = work->next;
		ad->info_list = g_list_remove(ad->info_list, id);
		ad->text = g_list_remove(ad->text, id);
		if (!line_is_in_click_list(ad, id)) free_info_line(ad, id);
		}
}

/*
 *----------------------------------------------------------------------------
 * display internal management
 *----------------------------------------------------------------------------
 */

static gint line_is_in_line_list(AppData *ad, InfoData *id)
{
	GList *work = NULL;
	work = g_list_find(ad->info_list, id);

	if (work) return TRUE;
	return FALSE;
}

static gint info_line_unref(AppData *ad, InfoData *id)
{
	if (id->show_count > 0)
		{
		id->show_count--;
		if (id->show_count < 1)
			{
			remove_info_line(ad, id);
			return TRUE;
			}
		}
	return FALSE;
}

static InfoData *first_unshown_info_line(GList *list)
{
	GList *work = list;
	InfoData *ret_id = NULL;
	while(work)
		{
		InfoData *id = list->data;
		if (!id->shown)
			{
			if (!ret_id || (ret_id))
				{
				ret_id = id;
				}
			}
		work = work->next;
		}

	return ret_id;
}

static InfoData *first_info_line(GList *list)
{
	InfoData *id = first_unshown_info_line(list);
	if (id) return id;

	if (list) return list->data;

	return NULL;
}


static InfoData *next_info_line_from_line(AppData *ad, InfoData *id)
{
	GList *work;
	InfoData *wid;

	work = g_list_find(ad->info_list, id);
	if (!work)
		{
		return first_info_line(ad->info_list);
		}

	wid = first_unshown_info_line(ad->info_list);
	if (wid) return wid;
	
	if (work->next)
		{
		work = work->next;
		return work->data;
		}

	if (ad->info_list->data == id && id->show_count != 1) return NULL;

	return ad->info_list->data;
}

static InfoData *next_info_line(AppData *ad)
{
	InfoData *id;

	if (!ad->info_list) return NULL;

	if (ad->info_next && line_is_in_line_list(ad, ad->info_next))
		{
		id = ad->info_next;
		ad->info_next = NULL;
		return id;
		}

	id = first_unshown_info_line(ad->info_list);
	if (id) return id;
	
	return ad->info_list->data;
}

static void display_info_finished(AppData *ad, InfoData *id)
{
	if (id->end_func) id->end_func(id->data, id, ad);
	if (id == ad->info_current)
		{
		ad->info_next = next_info_line_from_line(ad, id);
		if (ad->free_current)
			{
			free_info_line(ad, ad->info_current);
			ad->free_current = FALSE;
			}
		ad->info_current = NULL;
		}
	info_line_unref(ad, id);
}

/*
 *----------------------------------------------------------------------------
 * display icon drawing
 *----------------------------------------------------------------------------
 */

/* returns the number of lines drawn, 0 means end of icon */
static gint display_draw_icon(AppData *ad, InfoData *id, gint n)
{
	if (id->icon && id->icon_h >= ad->y_pos - n)
		{
		gint x,y;
		gint xo, yo;
		gint w, h;

		x = 2;
		y = ad->height - n;
		xo = 0;
		yo = ad->y_pos - n;
		w = id->icon_w;
		h = n;
		if (h > id->icon_h - yo) h = id->icon_h - yo;
		draw_pixmap (ad, GNOME_PIXMAP(id->icon)->pixmap, GNOME_PIXMAP(id->icon)->mask,
			     ad->display, x, y, w, h, xo, yo);
		return h;
		}

	return 0;
}

/*
 *----------------------------------------------------------------------------
 * display line drawing
 *----------------------------------------------------------------------------
 */

static gint draw_display_line(AppData *ad)
{
	InfoData *id;
	gchar c;
	GdkFont *font;
	gint new_line = FALSE;
	int len;	/* byte length of a character */

	if (!ad->info_current && !ad->info_list) return TRUE;

	if (!ad->info_current)
		{
		ad->info_current = next_info_line(ad);
		if (!ad->info_current) return TRUE;
		
		ad->text_pos = 0;
		ad->x_pos = 0;
		ad->y_pos = 0;
		}

	id = ad->info_current;
	id->shown = TRUE;

	font = ad->draw_area->style->font;

	if (ad->x_pos == 0)
		{
		/* add or do any callbacks if new info */
		if (ad->y_pos == 0)
			{
			if (id->pre_func) id->pre_func(id->data, id, ad);
			if (id->click_func) register_click_func(ad, id);
			ad->y_pos = ad->scroll_height;
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
			{
			display_draw_icon(ad, id, ad->scroll_height);
			ad->x_pos += id->offset;
			}
		}

	/* skip leading space on new line */
	c = id->text[ad->text_pos];
	len = mblen(&id->text[ad->text_pos], id->length);
	if (ad->x_pos == id->offset && c == ' ' &&
			ad->text_pos < id->length - 1 && id->text[ad->text_pos + len] != ' ')
		{
		ad->text_pos += len;
		c = id->text[ad->text_pos];
		len = mblen(&id->text[ad->text_pos], id->length);
		}

	gdk_draw_text(ad->display, font,
			ad->draw_area->style->fg_gc[GTK_WIDGET_STATE(ad->draw_area)],
			ad->x_pos, ad->height - ad->text_y_line, id->text + ad->text_pos, len);
	ad->x_pos += gdk_text_width(font, &id->text[ad->text_pos], len);
	ad->text_pos += len;

	if (ad->text_pos >= id->length)
		{
		new_line = TRUE;
		}
	else
		{
		c = id->text[ad->text_pos];
		len = mblen(&id->text[ad->text_pos], id->length);
		if (ad->x_pos + gdk_text_width(font, &id->text[ad->text_pos], len) > ad->width)
			{
			new_line = TRUE;
			}
		else if (c == '\n')
			{
			ad->text_pos++;
			new_line = TRUE;
			}
		else if (c == ' ' && ad->text_pos < id->length)
			{
			gint word_length;
			gint p;
			p = ad->text_pos + 1;
			while (p < id->length && id->text[p] != ' ' && id->text[p] != '\n') p++;
			word_length = gdk_text_width(font, id->text + ad->text_pos, p - ad->text_pos);
			if (ad->x_pos + word_length > ad->width)
				{
				ad->text_pos++;
				new_line = TRUE;
				}
			}
		}

	if (new_line)
		{
		ad->x_pos = 0;
		ad->scroll_pos = ad->scroll_height;
		if (ad->text_pos >= id->length)
			{
			ad->scroll_count = id->end_delay / 10 * (1000 / UPDATE_DELAY);
			if (!id->icon || id->icon_h <= ad->y_pos)
				{
				display_info_finished(ad, ad->info_current);
				}
			}
		else
			{
			ad->scroll_count = ad->scroll_delay / 10 * (1000 / UPDATE_DELAY);
			}
		}

	return new_line;
}

/*
 *----------------------------------------------------------------------------
 * display scrolling
 *----------------------------------------------------------------------------
 */

static void scroll_display_up(AppData *ad, gint n)
{
	draw_pixmap (ad, ad->display, NULL, ad->disp_buf, 0, 0, ad->width, ad->height - n, 0, n);
	draw_pixmap (ad, ad->disp_buf, NULL, ad->display, 0, 0, ad->width, ad->height - n, 0, 0);
	draw_pixmap (ad, ad->background, NULL, ad->display, 0, ad->height - n, ad->width, n, 0, ad->height - n);
	ad->y_pos += n;
	if (ad->info_current)
		{
		gint drawn = display_draw_icon(ad, ad->info_current, n);
		if (ad->text_pos >= ad->info_current->length)
			{
			if (drawn > 0)
				{
				ad->scroll_pos += drawn;
				}
			else
				{
				display_info_finished(ad, ad->info_current);
				}
			}
		}
	redraw_display(ad);
	scroll_click_func(ad, n);
}

/*
 *----------------------------------------------------------------------------
 * main display loop
 *----------------------------------------------------------------------------
 */

static gint update_display_cb(gpointer data)
{
	AppData *ad = data;
	if (!ad) return FALSE;

	if (ad->scroll_count > 0)
		{
		ad->scroll_count --;
		return TRUE;
		}

	if (!ad->info_list && !ad->info_current) return TRUE;

	/* draw the scrolling */
	if (ad->scroll_pos > 0)
		{
		gint n;
		if (ad->smooth_scroll)
			{
			n = ad->scroll_speed;
			if (ad->scroll_pos - n < 0) n = ad->scroll_pos;
			}
		else
			{
			n = ad->scroll_pos;
			}
		scroll_display_up(ad, n);
		ad->scroll_pos -= n;
		return TRUE;
		}

	if (ad->smooth_type)
		{
		draw_display_line(ad);
		redraw_display(ad);
		}
	else
		{
		while(!draw_display_line(ad));
		redraw_display(ad);
		}
	return TRUE;
}

/*
 *----------------------------------------------------------------------------
 * display mouse and pointer functions
 *----------------------------------------------------------------------------
 */

void set_mouse_cursor (AppData *ad, gint icon)
{
        GdkCursor *cursor = gdk_cursor_new (icon);
        gdk_window_set_cursor (ad->draw_area->window, cursor);
        gdk_cursor_destroy (cursor);
}

static gint line_is_in_click_list(AppData *ad, InfoData *id)
{
	GList *work = ad->click_list;

	while(work)
		{
		ClickData *cd = work->data;
		if (id == cd->line_id) return TRUE;
		work = work->next;
		}

	return FALSE;
}

static void register_click_func(AppData *ad, InfoData *id)
{
	ClickData *cd;

	if (!id || !id->click_func) return;

	cd = g_new0(ClickData, 1);

	cd->line_id = id;
	cd->click_func = id->click_func;

	cd->x = 0;
	cd->y = ad->height - ad->scroll_height;
	cd->w = ad->width;
	cd->h = ad->scroll_height;

	cd->free_id = FALSE;

	ad->click_list = g_list_append(ad->click_list, cd);

	scroll_click_func(ad, 0);
}

static void scroll_click_func(AppData *ad, gint n)
{
	GList *list = ad->click_list;
	gint proximity = FALSE;

	while(list)
		{
		GList *w = list;
		ClickData *cd = w->data;
		list = list->next;

		cd->y -= n;

		if (ad->info_current)
			{
			InfoData *id = ad->info_current;
			if (id == cd->line_id) cd->h += n;
			}

		if (cd->y + cd->h < 1)
			{
			ad->click_list = g_list_remove(ad->click_list, cd);
			if (!line_is_in_line_list(ad, cd->line_id) &&
			    !line_is_in_click_list(ad, cd->line_id))
				free_info_line(ad, cd->line_id);
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
			if (cd->click_func) 
				cd->click_func(cd->line_id->data, 
					       cd->line_id, ad);
			}

		list = list->next;
		}

	return TRUE;
	w = NULL;
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
	return;
	w = NULL;
}

/*
 *----------------------------------------------------------------------------
 * display initialization and resizing
 *----------------------------------------------------------------------------
 */

static void calc_display_sizes(AppData *ad)
{
	if (ad->follow_hint_width)
		{
#ifdef HAVE_PANEL_PIXEL_SIZE
		ad->width_hint = applet_widget_get_free_space(APPLET_WIDGET(ad->applet));
		if (ad->width_hint < 8) ad->width_hint = 200; /* no space or corner panel */
#endif
		ad->win_width = ad->width_hint;
		}
	else
		{
		ad->win_width = ad->user_width;
		}
	ad->width = ad->win_width - 4; /* hard coded, this is bound to break */

	if (ad->follow_hint_height)
		{
		ad->win_height = ad->sizehint;
		}
	else
		{
		ad->win_height = ad->user_height;
		}
	ad->height = ad->win_height - 4; /* hard coded, this is bound to break */
}

static void create_display_pixmaps(AppData *ad)
{
/* the old way ?
	ad->display_w = gnome_pixmap_new_from_xpm_d_at_size(back_xpm, ad->width, ad->height);
	ad->display = GNOME_PIXMAP(ad->display_w)->pixmap;
	ad->disp_buf_w = gnome_pixmap_new_from_xpm_d_at_size(back_xpm, ad->width, ad->height);
	ad->disp_buf = GNOME_PIXMAP(ad->disp_buf_w)->pixmap;
	ad->background_w = gnome_pixmap_new_from_xpm_d_at_size(back_xpm, ad->width, ad->height);
	ad->background = GNOME_PIXMAP(ad->background_w)->pixmap;
*/

	/* clear any current pixmaps */
	if (ad->display) gdk_pixmap_unref(ad->display);
	if (ad->disp_buf) gdk_pixmap_unref(ad->disp_buf);
	if (ad->background) gdk_pixmap_unref(ad->background);
	ad->display = NULL;
	ad->disp_buf = NULL;
	ad->background = NULL;

	ad->display = gdk_pixmap_new (ad->applet->window,
				      ad->width, ad->height, -1);

	ad->disp_buf = gdk_pixmap_new (ad->applet->window,
				       ad->width, ad->height, -1);
	ad->background = gdk_pixmap_new (ad->applet->window,
					 ad->width, ad->height, -1);

	gdk_draw_rectangle (ad->display, ad->applet->style->light_gc[0],
			    TRUE, 0, 0, ad->width, ad->height);
	gdk_draw_rectangle (ad->disp_buf, ad->applet->style->light_gc[0], 
			    TRUE, 0, 0, ad->width, ad->height);
	gdk_draw_rectangle (ad->background, ad->applet->style->light_gc[0],
			    TRUE, 0, 0, ad->width, ad->height);
}

void resized_app_display(AppData *ad, gint force)
{
	gint old_w = ad->width;
	gint old_h = ad->height;

	calc_display_sizes(ad);

	if (old_w == ad->width && old_h == ad->height && !force) return;

/* old way?
	gtk_widget_destroy(ad->display_w);
	gtk_widget_destroy(ad->disp_buf_w);
	gtk_widget_destroy(ad->background_w);
*/

	create_display_pixmaps(ad);
	gtk_widget_set_usize(ad->applet, ad->win_width, ad->win_height);
	gtk_drawing_area_size(GTK_DRAWING_AREA(ad->draw_area), ad->width, ad->height);

	ad->text_height = ad->draw_area->style->font->ascent +
		ad->draw_area->style->font->descent;
	ad->text_y_line = ad->draw_area->style->font->descent;

	ad->scroll_height = ad->text_height;

	redraw_display(ad);
}

/* this is called when themes and fonts change */
static void app_display_style_change_cb(GtkWidget *widget, GtkStyle *previous_style, gpointer data)
{
	AppData *ad = data;

	/* the size does not change, but this causes a redraw
	   in the case that the background color changed */
	resized_app_display(ad, TRUE);
	return;
	widget = NULL;
	previous_style = NULL;
}

void init_app_display(AppData *ad)
{
	ad->scroll_delay = 20;
	ad->scroll_count = 0;
	ad->scroll_pos = 0;

	ad->info_list = NULL;
	ad->free_current = FALSE;
	ad->info_current = NULL;
	ad->info_next = NULL;
	ad->click_list = NULL;

	ad->display = NULL;
	ad->disp_buf = NULL;
	ad->background = NULL;

	ad->frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(ad->frame), GTK_SHADOW_IN);
	gtk_widget_show(ad->frame);

	ad->draw_area = gtk_drawing_area_new();
	gtk_drawing_area_size(GTK_DRAWING_AREA(ad->draw_area), ad->width, ad->height);
	gtk_widget_set_events (ad->draw_area, GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK);
	gtk_signal_connect(GTK_OBJECT(ad->draw_area),"motion_notify_event",
		(GtkSignalFunc) display_motion, ad);
	gtk_signal_connect(GTK_OBJECT(ad->draw_area),"button_press_event",
		(GtkSignalFunc) display_click, ad);
	gtk_container_add(GTK_CONTAINER(ad->frame), ad->draw_area);
        gtk_widget_show(ad->draw_area);

        applet_widget_add(APPLET_WIDGET(ad->applet), ad->frame);

        gtk_widget_realize(ad->draw_area);

	calc_display_sizes(ad);
	create_display_pixmaps(ad);

	gtk_widget_set_usize(ad->applet, ad->win_width, ad->win_height);


	ad->text_height = ad->draw_area->style->font->ascent +
		ad->draw_area->style->font->descent;
	ad->text_y_line = ad->draw_area->style->font->descent;

	ad->scroll_height = ad->text_height;

	redraw_display(ad);

	gtk_signal_connect(GTK_OBJECT(ad->draw_area),"style_set",
		(GtkSignalFunc) app_display_style_change_cb, ad);

	ad->display_timeout_id = gtk_timeout_add(UPDATE_DELAY, (GtkFunction)update_display_cb, ad);
}

