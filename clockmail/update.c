/* GNOME clock & mailcheck applet
 * (C) 2000 John Ellis
 *
 * Author: John Ellis
 *
 */

#include "clockmail.h"
#include "update.h"
#include "item.h"
#include "number.h"
#include "clock.h"
#include "skin.h"

static void update_mail_display(int n, AppData *ad, gint force)
{
	if (force)
		{
		item_draw(ad->skin->mail, ad->old_n, ad->skin->pixbuf);
		return;
		}
	if (n != ad->old_n)
		{
		item_draw(ad->skin->mail, n, ad->skin->pixbuf);
		ad->old_n = n;
		}
}

static void update_mail_count(AppData *ad, gint force)
{
	if (!ad->skin->messages && !ad->skin->mail_amount) return;

	if (ad->mail_file && !force)
		{
		FILE *f = 0;
		gchar buf[1024];

		f = fopen(ad->mail_file, "r");

		if (f)
			{
			gint c = 0;
			while (fgets(buf, sizeof(buf), f) != NULL)
				{
				if (buf[0] == 'F' && !strncmp(buf, "From ", 5)) c++;
				/* Hack alert! this was added to ignore PINE's false mail entries */
				if (buf[0] == 'X' && !strncmp(buf, "X-IMAP:", 7) && c > 0) c--;
				/* emacs vm ? */
				if (buf[0] == 'X' && !strncmp(buf, "X-VM-v5-Data: ([nil nil", 23) && c > 0) c--;
				}
                        fclose(f);
			ad->message_count = c;
                        }
		else
			{
			ad->message_count = 0;
			}
		}

	number_draw(ad->skin->messages, ad->message_count, ad->skin->pixbuf);

	if (ad->skin->mail_amount)
		{
		gint p;

		if (ad->mail_max < 10) ad->mail_max = 10;
		if (ad->message_count >= ad->mail_max)
			p = ad->skin->mail_amount->sections - 1;
		else
			p = (float)ad->message_count / ad->mail_max * ad->skin->mail_amount->sections;
		if (p == 0 && ad->anymail && ad->skin->mail_amount->sections > 1) p = 1;
		item_draw(ad->skin->mail_amount, p, ad->skin->pixbuf);
		}
}

static void update_mail_amount_display(AppData *ad, gint force)
{
	if (ad->mailsize != ad->old_amount || force)
		{
		update_mail_count(ad, force);
		number_draw(ad->skin->mail_count, ad->mailsize / 1024, ad->skin->pixbuf);
		ad->old_amount = ad->mailsize;
		}
}

static void update_time_count(gint h, gint m, gint s, AppData *ad)
{
	if (ad->am_pm_enable)
		{
		if (h > 12) h -= 12;
		if (h == 0) h = 12;
		}
	number_draw(ad->skin->hour, h, ad->skin->pixbuf);
	number_draw(ad->skin->min, m, ad->skin->pixbuf);
	number_draw(ad->skin->sec, s, ad->skin->pixbuf);

	if (h > 12) h -= 12;
	clock_draw(ad->skin->clock, h, m, s, ad->skin->pixbuf);
}

static void update_date_displays(gint year, gint month, gint day, gint weekday, AppData *ad, gint force)
{
	/* no point in drawing things again if they don't change */
	if (ad->old_week == weekday && !force) return;

	number_draw(ad->skin->year, year, ad->skin->pixbuf);
	number_draw(ad->skin->month, month + 1, ad->skin->pixbuf);	/* month is 0-31 */
	number_draw(ad->skin->day, day, ad->skin->pixbuf);
	item_draw(ad->skin->month_txt, month, ad->skin->pixbuf);	/* but 0 is needed here */
	item_draw(ad->skin->week_txt, weekday, ad->skin->pixbuf);

	ad->old_week = weekday;
}

void update_time_and_date(AppData *ad, gint force)
{
	time_t current_time;
	struct tm *time_data;

	time(&current_time);

	/* if using a non-local time, calculate it from GMT */
	if (ad->use_gmt)
		{
		current_time += (time_t)ad->gmt_offset * 3600;
		time_data = gmtime(&current_time);
		}
	else
		time_data = localtime(&current_time);

	/* update time */
	update_time_count(time_data->tm_hour,time_data->tm_min, time_data->tm_sec, ad);

	/* update date */
	update_date_displays(time_data->tm_year + 1900, time_data->tm_mon, time_data->tm_mday,
			     time_data->tm_wday, ad, force);

	/* set tooltip to the date */
	set_tooltip(time_data, ad);
}

static gint blink_callback(gpointer data)
{
	AppData *ad = data;
	ad->blink_lit++;

	if (ad->blink_lit >= ad->mail_sections - 1)
		{
		ad->blink_lit = 0;
		ad->blink_count++;
		}
	update_mail_display(ad->blink_lit, ad, FALSE);

	if (ad->blink_count >= ad->blink_times || !ad->anymail)
		{
		if (ad->always_blink && ad->anymail)
			{
			if (ad->blink_count >= ad->blink_times) ad->blink_count = 1;
			}
		else
			{
			/* reset counters for next time */
			ad->blink_count = 0;
			ad->blink_lit = 0;
			if (ad->anymail)
				update_mail_display(ad->mail_sections - 1, ad, FALSE);
			else
				update_mail_display(0, ad, FALSE);
			ad->blinking = FALSE;
			skin_flush(ad);
			return FALSE;
			}
		}

	skin_flush(ad);

	return TRUE;
}

void update_mail_status(AppData *ad, gint force)
{
	if (force)
		{
		update_mail_amount_display(ad, TRUE);
		update_mail_display(0, ad, TRUE);
		return;
		}

	check_mail_file_status (FALSE, ad);

	if (!ad->blinking)
		{
		if (ad->anymail)
			if (ad->newmail || ad->always_blink)
				{
				ad->blinking = TRUE;
				ad->blink_timeout_id = gtk_timeout_add(ad->blink_delay,
						(GtkFunction)blink_callback, ad);
				if (ad->newmail && ad->exec_cmd_on_newmail)
					{
					if (ad->newmail_exec_cmd && strlen(ad->newmail_exec_cmd) > 0)
						gnome_execute_shell(NULL, ad->newmail_exec_cmd);
					}
				}
			else
				{
				update_mail_display(ad->mail_sections - 1, ad, FALSE);
				}
		else
			{
			update_mail_display(0, ad, FALSE);
			}
		}

	update_mail_amount_display(ad, FALSE);
}

void update_all(gpointer data)
{
	AppData *ad = data;

	applet_skin_redraw_all(ad);
	skin_flush(ad);
}

gint update_display(gpointer data)
{
	AppData *ad = data;

	update_time_and_date(ad, FALSE);
	update_mail_status(ad, FALSE);

	skin_flush(ad);

	return TRUE;
}


