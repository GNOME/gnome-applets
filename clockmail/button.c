/* GNOME clock & mailcheck applet
 * (C) 2000 John Ellis
 *
 * Author: John Ellis
 *
 */

#include "clockmail.h"
#include "button.h"
#include "item.h"

ButtonData *button_new(ItemData *item, gint width, gint height,
		       void (*click_func)(gpointer), gpointer click_data,
		       void (*redraw_func)(gpointer), gpointer redraw_data)
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
		button->width = width;
		button->height = height;
		button->x = 0;
		button->y = 0;
		}

	button->pushed = 0;
	button->prelit = 0;

	button->click_func = click_func;
	button->redraw_func = redraw_func;

	button->click_data = click_data;
	button->redraw_data = redraw_data;

	return button;
}

void button_free(ButtonData *button)
{
	if (!button) return;

	g_free(button);
}

gint button_draw(ButtonData *button, gint prelight, gint pressed, gint force,
		 GdkPixbuf *pb, gint enable_draw_cb)
{
	gint redraw = FALSE;
	if (!button) return FALSE;

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
				button->prelit = 0;
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
		item_draw(button->item, button->pushed + (button->prelit * 2), pb);

		/* bah, now everything else needs redrawn because they may be within
		 * the button.
		 */

		if (enable_draw_cb && button->redraw_func) button->redraw_func(button->redraw_data);
		}
	else
		{
		redraw = FALSE;
		}

	return redraw;
}


