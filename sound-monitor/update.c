/* GNOME sound-monitor applet
 * (C) 2000 John Ellis
 *
 * Author: John Ellis
 *
 */

#include "sound-monitor.h"
#include "update.h"
#include "item.h"
#include "meter.h"
#include "scope.h"
#include "analyzer.h"
#include "skin.h"
#include "esdcalls.h"

void update_modes(AppData *ad)
{
	meter_mode_set(ad->skin->meter_left, ad->peak_mode, (float)ad->falloff_speed / ad->refresh_fps * 10.0);
	meter_mode_set(ad->skin->meter_right, ad->peak_mode, (float)ad->falloff_speed / ad->refresh_fps * 10.0);
	scope_scale_set(ad->skin->scope, ad->scope_scale);
	scope_type_set(ad->skin->scope, ad->draw_scope_as_segments ? ScopeType_Lines : ScopeType_Points);
}

void update_fps_timeout(AppData *ad)
{
	gtk_timeout_remove(ad->update_timeout_id);
	ad->update_timeout_id = gtk_timeout_add(1000 / ad->refresh_fps, (GtkFunction)update_display, ad);
	update_modes(ad);
}

static void update_status(AppData *ad)
{
	ad->status_check_count++;
	if (ad->status_check_count > ad->refresh_fps) /* 1 second checks should do */
		{
		gint new_status = esd_status(ad->esd_host);
		ad->status_check_count = 0;
		if (ad->esd_status != new_status)
			{
			ad->esd_status = new_status;
			if (ad->esd_status == Status_Error && ad->sound)
				{
				/* problem must have occured, disconnect from esd now! */
				sound_free(ad->sound);
				ad->sound = NULL;
				gnome_error_dialog("Sound Monitor has lost the connection to the Esound daemon,\n this usually means the daemon has exited and can simply be restarted.\n\n Or the daemon has crashed and needs some attention...");
				}
			}
		}

	if (ad->esd_status_menu != ad->esd_status) sync_esd_menu_item(ad);
}

gint update_display(gpointer data)
{
	AppData *ad = data;
	gint redraw = FALSE;

	update_status(ad);

	redraw |= item_draw(ad->skin->status, (gint)ad->esd_status, ad->skin->pixbuf);

	if (ad->sound)
		{
		gint new_data = ad->sound->new_data;

		if (new_data)
			{
			ad->no_data_count = 0;
			ad->scope_flat = FALSE;
			}
		else
			{
			if ((float)ad->no_data_count / (ad->refresh_fps ? 1 : ad->refresh_fps) < 0.333 )
				{
				/* no new data in .3 sec results in zeroing */
				ad->no_data_count ++;
				}
			else if (!ad->scope_flat)
				{
				ad->scope_flat = TRUE;
				scope_draw(ad->skin->scope, ad->skin->pixbuf, TRUE, NULL, 0);
				redraw = TRUE;
				}
			}

		if (ad->skin->meter_left || ad->skin->meter_right ||
		    ad->skin->item_left || ad->skin->item_right)
			{
			gint l, r;

			l = r = 0;

			if (new_data)
				{
				sound_get_volume(ad->sound, &l, &r);
				}

			redraw |= meter_draw(ad->skin->meter_left, l, ad->skin->pixbuf);
			redraw |= meter_draw(ad->skin->meter_right, r, ad->skin->pixbuf);

			redraw |= item_draw_by_percent(ad->skin->item_left, l, ad->skin->pixbuf);
			redraw |= item_draw_by_percent(ad->skin->item_right, r, ad->skin->pixbuf);
			}
		if (ad->skin->scope && !ad->scope_flat && new_data)
			{
			short *buffer;
			gint length;

			sound_get_buffer(ad->sound, &buffer, &length);

			scope_draw(ad->skin->scope, ad->skin->pixbuf, FALSE, buffer, length);

			redraw = TRUE;
			}
		if (ad->skin->analyzer)
			{
			if (ad->scope_flat)
				{
				analyzer_update(ad->skin->analyzer, ad->skin->pixbuf, NULL, ad->refresh_fps);
				redraw = TRUE;
				}
			else if (new_data)
				{
				short *buffer;
				gint length;

				sound_get_buffer(ad->sound, &buffer, &length);

				analyzer_update_w_fft(ad->skin->analyzer, ad->skin->pixbuf, buffer, length, ad->refresh_fps);
				redraw = TRUE;
				}
			}

		ad->sound->new_data = FALSE;
		}

	if (redraw) skin_flush(ad);

	return TRUE;
}

