/* GNOME sound-monitor applet
 * (C) 2000 John Ellis
 *
 * Author: John Ellis
 *
 */

#include <config.h>
#include "sound-monitor.h"
#include "manager.h"

enum {
	STREAM_TYPE,
	SAMPLE_TYPE
};

enum {
	STREAM_MONITOR,
	STREAM_PLAY,
	STREAM_RECORD,
	STREAM_SAMPLE,
	STREAM_ADPCM
};

typedef struct _StreamData StreamData;
struct _StreamData
{
	gint id;
	gchar *name;
	gint rate;
	gint vol_left;
	gint vol_right;
	esd_format_t format;

	gint bits;
	gint channels;
	gint type;

	gint dirty;
	gint updated;
};

typedef struct _SampleData SampleData;
struct _SampleData
{

	gint id;
	gchar *name;
	gint rate;
	gint vol_left;
	gint vol_right;
	esd_format_t format;
	gint length;

	gint bits;
	gint channels;
	gint loop;
	gint playing;

	gint dirty;
	gint updated;
};

typedef struct _MgrData MgrData;
struct _MgrData
{
	AppData *ad;
	GtkWidget *window;
	GtkWidget *notebook;
	GtkWidget *stream_clist;
	GtkWidget *sample_clist;
	GtkWidget *info_clist;
	GtkWidget *server_clist;

	GtkWidget *stream_vol_scale;
	GtkWidget *stream_pan_scale;
	GtkWidget *stream_id_entry;
	GtkWidget *stream_name_entry;
	StreamData *stream_data_item;

	GtkWidget *sample_vol_scale;
	GtkWidget *sample_pan_scale;
	GtkWidget *sample_id_entry;
	GtkWidget *sample_name_entry;
	SampleData *sample_data_item;

	GList *streams;
	GList *samples;

	gint samples_show_time;

	gint update_timeout_id;

	gint server_version;
	esd_format_t server_format;
	gint server_rate;
	gint server_bits;
	gint server_channels;
	gint server_update;
	gint server_stream_count;
	gint server_sample_count;
};

static StreamData *stream_data_new()
{
	StreamData *sd = g_new0(StreamData, 1);
	return sd;
}

static void stream_data_free(StreamData *sd)
{
	g_free(sd->name);
	g_free(sd);
}

static void stream_data_update(StreamData *sd, esd_player_info_t *stream)
{
	gint updated = FALSE;
	if (sd->id != stream->source_id)
		{
		sd->id = stream->source_id;
		updated = TRUE;
		}
	if ((sd->name && strcmp(sd->name, stream->name) != 0) ||
	    (!sd->name && strlen(stream->name) > 0))
		{
		g_free(sd->name);
		sd->name = g_strdup(stream->name);
		updated = TRUE;
		}
	if (sd->rate != stream->rate)
		{
		sd->rate = stream->rate;
		updated = TRUE;
		}
	if (sd->vol_left != stream->left_vol_scale)
		{
		sd->vol_left = stream->left_vol_scale;
		updated = TRUE;
		}
	if (sd->vol_right != stream->right_vol_scale)
		{
		sd->vol_right = stream->right_vol_scale;
		updated = TRUE;
		}
	if (sd->format != stream->format)
		{
		sd->format = stream->format;

		if ((sd->format & ESD_MASK_BITS) == ESD_BITS16)
			sd->bits = 16;
		else
			sd->bits = 8;

		if ((sd->format & ESD_MASK_CHAN) == ESD_STEREO)
			sd->channels = 2;
		else
			sd->channels = 1;

		if ((sd->format & ESD_MASK_MODE) == ESD_STREAM)
			{
			if ((sd->format & ESD_MASK_FUNC) == ESD_RECORD)
				sd->type = STREAM_RECORD;
			else if ((sd->format & ESD_MASK_FUNC) == ESD_PLAY)
				sd->type = STREAM_PLAY;
			else
				sd->type = STREAM_MONITOR;
			}
		else if ((sd->format & ESD_MASK_MODE) == ESD_SAMPLE)
			sd->type = STREAM_SAMPLE;
		else
			sd->type = STREAM_ADPCM;
			

		updated = TRUE;
		}

	sd->dirty = FALSE;
	sd->updated = updated;
}

static SampleData *sample_data_new()
{
	SampleData *sd = g_new0(SampleData, 1);
	return sd;
}

static void sample_data_free(SampleData *sd)
{
	g_free(sd->name);
	g_free(sd);
}

static void sample_data_update(SampleData *sd, esd_sample_info_t *sample)
{
	gint updated = FALSE;
	if (sd->id != sample->sample_id)
		{
		sd->id = sample->sample_id;
		updated = TRUE;
		}
	if ((sd->name && strcmp(sd->name, sample->name) != 0) ||
	    (!sd->name && strlen(sample->name) > 0))
		{
		g_free(sd->name);
		sd->name = g_strdup(sample->name);
		updated = TRUE;
		}
	if (sd->rate != sample->rate)
		{
		sd->rate = sample->rate;
		updated = TRUE;
		}
	if (sd->vol_left != sample->left_vol_scale)
		{
		sd->vol_left = sample->left_vol_scale;
		updated = TRUE;
		}
	if (sd->vol_right != sample->right_vol_scale)
		{
		sd->vol_right = sample->right_vol_scale;
		updated = TRUE;
		}
	if (sd->format != sample->format)
		{
		sd->format = sample->format;

		if ((sd->format & ESD_MASK_BITS) == ESD_BITS16)
			sd->bits = 16;
		else
			sd->bits = 8;

		if ((sd->format & ESD_MASK_CHAN) == ESD_STEREO)
			sd->channels = 2;
		else
			sd->channels = 1;

		sd->loop = ((sd->format & ESD_MASK_FUNC) == ESD_LOOP);
		sd->playing = ((sd->format & ESD_MASK_FUNC) == ESD_PLAY);

		updated = TRUE;
		}
	if (sd->length != sample->length)
		{
		sd->length = sample->length;
		updated = TRUE;
		}

	sd->dirty = FALSE;
	sd->updated = updated;
}

static MgrData *mgr_data_new()
{
	MgrData *md = g_new0(MgrData, 1);
	return md;
}

static void mgr_data_free(MgrData *md)
{
	if (md->streams)
		{
		g_list_foreach(md->streams, (GFunc)stream_data_free, NULL);
		g_list_free(md->streams);
		}
	if (md->samples)
		{
		g_list_foreach(md->samples, (GFunc)sample_data_free, NULL);
		g_list_free(md->samples);
		}
	g_free(md);
}

static void sample_play_by_id(MgrData *md, gint id)
{
	gint esd_fd;

	esd_fd = esd_open_sound(md->ad->esd_host);
	if (esd_fd >= 0)
		{
		esd_sample_play(esd_fd, id);
		esd_close(esd_fd);
		}
}

static void panning_set_by_id(gint id, gint left, gint right, MgrData *md, gint type)
{
	gint esd_fd;

	esd_fd = esd_open_sound(md->ad->esd_host);
	if (esd_fd >= 0)
		{
		if (type == STREAM_TYPE)
			esd_set_stream_pan(esd_fd, id, left, right);
		else
			esd_set_default_sample_pan(esd_fd, id, left, right);

		esd_close(esd_fd);
		}
}

/*
 * -------------------------------------------------------------------------
 * panning adjustment routines
 * -------------------------------------------------------------------------
 */

static void panning_widget_set(gint id, gchar *name, gint l, gint r, MgrData *md, gint type)
{
	GtkWidget *scale_pan;
	GtkWidget *scale_vol;
	GtkWidget *id_entry;
	GtkWidget *name_entry;

	if (type == STREAM_TYPE)
		{
		scale_pan = md->stream_pan_scale;
		scale_vol = md->stream_vol_scale;
		id_entry = md->stream_id_entry;
		name_entry = md->stream_name_entry;
		}
	else
		{
		scale_pan = md->sample_pan_scale;
		scale_vol = md->sample_vol_scale;
		id_entry = md->sample_id_entry;
		name_entry = md->sample_name_entry;
		}

	if (id == -1)
		{
		if (!GTK_WIDGET_HAS_GRAB(scale_pan))
			{
			gtk_widget_set_sensitive(scale_pan, FALSE);
			}
		if (!GTK_WIDGET_HAS_GRAB(scale_vol))
			{
			gtk_widget_set_sensitive(scale_vol, FALSE);
			}
		gtk_entry_set_text(GTK_ENTRY(id_entry), "");
		gtk_entry_set_text(GTK_ENTRY(name_entry), "");
		return;
		}
	else
		{
		gchar *buf;
		gfloat val;
		gtk_widget_set_sensitive(scale_pan, TRUE);

		if (l == r)
			{
			val = 0.0;
			}
		else if (l < r)
			{
			if (r == 0)
				val = 100.0;
			else
				val = 100.0 - (gfloat)l / r * 100;
			}
		else
			{
			if (l == 0)
				val = -100.0;
			else
				val = -100.0 + (gfloat)r / l * 100;
			}

		gtk_adjustment_set_value(GTK_ADJUSTMENT(GTK_RANGE(scale_pan)->adjustment), val);

		gtk_widget_set_sensitive(scale_vol, TRUE);

		if (l >= r)
			{
			val = l;
			}
		else
			{
			val = r;
			}

		gtk_adjustment_set_value(GTK_ADJUSTMENT(GTK_RANGE(scale_vol)->adjustment), val);

		buf = g_strdup_printf("%d", id);
		gtk_entry_set_text(GTK_ENTRY(id_entry), buf);
		g_free(buf);

		gtk_entry_set_text(GTK_ENTRY(name_entry), name);
		}
}

static void panning_widget_set_stream(StreamData *sd, MgrData *md)
{
	md->stream_data_item = NULL;

	if (sd)
		panning_widget_set(sd->id, sd->name, sd->vol_left, sd->vol_right, md, STREAM_TYPE);
	else
		panning_widget_set(-1, NULL, 0, 0, md, STREAM_TYPE);

	md->stream_data_item = sd;
}

static void panning_widget_set_sample(SampleData *sd, MgrData *md)
{
	md->sample_data_item = NULL;

	if (sd)
		panning_widget_set(sd->id, sd->name, sd->vol_left, sd->vol_right, md, SAMPLE_TYPE);
	else
		panning_widget_set(-1, NULL, 0, 0, md, SAMPLE_TYPE);

	md->sample_data_item = sd;
}

static void stream_vol_changed_cb(GtkObject *adj, gpointer data)
{
	MgrData *md = data;
	StreamData *sd;
	gfloat val;
	gfloat bal;
	gint l, r;

	if (!md->stream_data_item) return;

	sd = md->stream_data_item;

	val = GTK_ADJUSTMENT(adj)->value;

	if (sd->vol_left > sd->vol_right)
		{
		bal = 0 - (float)(sd->vol_left - sd->vol_right) / sd->vol_left;
		}
	else if (sd->vol_left < sd->vol_right)
		{
		bal = (float)(sd->vol_right - sd->vol_left) / sd->vol_right;
		}
	else
		{
		bal = 0.0;
		}

	if (bal == 0.0)
		{
		l = val;
		r = val;
		}
	else if (bal < 0.0)
		{
		l = val;
		r = val * (1.0 + bal);
		}
	else
		{
		l = val * (1.0 - bal);
		r = val;
		}

	panning_set_by_id(sd->id, l, r, md, STREAM_TYPE);
}

static void stream_pan_changed_cb(GtkObject *adj, gpointer data)
{
	MgrData *md = data;
	StreamData *sd;
	gfloat val;
	gint l, r;
	gint max;

	if (!md->stream_data_item) return;

	sd = md->stream_data_item;

	val = GTK_ADJUSTMENT(adj)->value;

	if (sd->vol_left >= sd->vol_right)
		{
		max = sd->vol_left;
		}
	else
		{
		max = sd->vol_right;
		}

	if (val > 3.0)
		{
		r = max;
		l = (gint)((gfloat)r * (100.0 - val) / 100);
		}
	else if (val < -3.0)
		{
		l = max;
		r = (gint)((gfloat)l * (100.0 + val) / 100);
		}
	else
		{
		l = r = max;
		}

	panning_set_by_id(sd->id, l, r, md, STREAM_TYPE);
}

static void sample_vol_changed_cb(GtkObject *adj, gpointer data)
{
	MgrData *md = data;
	SampleData *sd;
	gfloat val;
	gfloat bal;
	gint l, r;

	if (!md->sample_data_item) return;

	sd = md->sample_data_item;

	val = GTK_ADJUSTMENT(adj)->value;

	if (sd->vol_left > sd->vol_right)
		{
		bal = 0 - (float)(sd->vol_left - sd->vol_right) / sd->vol_left;
		}
	else if (sd->vol_left < sd->vol_right)
		{
		bal = (float)(sd->vol_right - sd->vol_left) / sd->vol_right;
		}
	else
		{
		bal = 0.0;
		}

	if (bal == 0.0)
		{
		l = val;
		r = val;
		}
	else if (bal < 0.0)
		{
		l = val;
		r = val * (1.0 + bal);
		}
	else
		{
		l = val * (1.0 - bal);
		r = val;
		}

	panning_set_by_id(sd->id, l, r, md, SAMPLE_TYPE);
}

static void sample_pan_changed_cb(GtkObject *adj, gpointer data)
{
	MgrData *md = data;
	SampleData *sd;
	gfloat val;
	gint l, r;
	gint max;

	if (!md->sample_data_item) return;

	sd = md->sample_data_item;

	val = GTK_ADJUSTMENT(adj)->value;

	if (sd->vol_left >= sd->vol_right)
		{
		max = sd->vol_left;
		}
	else
		{
		max = sd->vol_right;
		}

	if (val > 3.0)
		{
		r = max;
		l = (gint)((gfloat)r * (100.0 - val) / 100);
		}
	else if (val < -3.0)
		{
		l = max;
		r = (gint)((gfloat)l * (100.0 + val) / 100);
		}
	else
		{
		l = r = max;
		}

	panning_set_by_id(sd->id, l, r, md, SAMPLE_TYPE);
}

/*
 * -------------------------------------------------------------------------
 * stream list routines
 * -------------------------------------------------------------------------
 */

static void stream_list_mark_dirty(GList *list)
{
	GList *work = list;
	while (work)
		{
		StreamData *sd = work->data;
		sd->dirty = TRUE;
		work = work->next;
		}
}

static void stream_list_free_dirty(MgrData *md)
{
	GList *work = md->streams;
	while (work)
		{
		StreamData *sd = work->data;
		work = work->next;
		if (sd->dirty)
			{
			gint row;
			md->streams = g_list_remove(md->streams, sd);
			row = gtk_clist_find_row_from_data(GTK_CLIST(md->stream_clist), sd);
			if (row >= 0)
				{
				gtk_clist_remove(GTK_CLIST(md->stream_clist), row);
				}
			if (md->stream_data_item == sd)
				{
				panning_widget_set_stream(NULL, md);
				}
			stream_data_free(sd);
			}
		}
}

static GList *stream_list_register(GList *list, esd_player_info_t *stream)
{
	GList *work = list;
	gint updated = FALSE;

	while(work && !updated)
		{
		StreamData *sd = work->data;
		if (sd->id == stream->source_id)
			{
			stream_data_update(sd, stream);
			updated = TRUE;
			}
		work = work->next;
		}

	if (!updated)
		{
		StreamData *sd = stream_data_new();
		stream_data_update(sd, stream);
		list = g_list_append(list, sd);
		}

	return list;
}

static gchar *stream_type_as_text(gint type)
{
	gchar *text;
	if (type == STREAM_RECORD)
		text = "Record";
	else if (type == STREAM_PLAY)
		text = "Play";
	else if (type == STREAM_MONITOR)
		text = "Monitor";
	else if (type == STREAM_SAMPLE)
		text = "Sample";
	else
		text = "PCM";

	return g_strdup(text);
}

static gchar *stream_channels_as_text(gint channels)
{
	gchar *text;
	if (channels == 2)
		text = "Stereo";
	else
		text = "Mono";

	return g_strdup(text);
}

static void refresh_stream_list_window(MgrData *md)
{
	gint row;
	GList *work = md->streams;
	gint frozen = FALSE;
	while(work)
		{
		StreamData *sd = work->data;
		if (sd->updated)
			{
			gint i;
			gchar *buf[8];
			row = gtk_clist_find_row_from_data(GTK_CLIST(md->stream_clist), sd);
			buf[0] = g_strdup_printf("%d", sd->id);
			buf[1] = g_strdup(sd->name);
			buf[2] = stream_type_as_text(sd->type);
			buf[3] = g_strdup_printf("%d", sd->rate);
			buf[4] = g_strdup_printf("%d", sd->bits);
			buf[5] = stream_channels_as_text(sd->channels);
			buf[6] = g_strdup_printf("%d,%d", sd->vol_left, sd->vol_right);
			buf[7] = NULL;
			if (!frozen)
				{
				gtk_clist_freeze(GTK_CLIST(md->stream_clist));
				frozen = TRUE;
				}
			if (row <0)
				{
				row = gtk_clist_append(GTK_CLIST(md->stream_clist), buf);
				gtk_clist_set_row_data(GTK_CLIST(md->stream_clist), row, sd);
				}
			else
				{
				for (i = 0; i < 7; i++)
					gtk_clist_set_text(GTK_CLIST(md->stream_clist), row, i, buf[i]);
				}
			for (i = 0; i < 7; i++) g_free(buf[i]);
			sd->updated = FALSE;
			}
		work = work->next;
		}
	if (frozen) gtk_clist_thaw(GTK_CLIST(md->stream_clist));
}

static void stream_selected_cb(GtkWidget *widget, gint row, gint col,
				GdkEvent *event, gpointer data)
{
	MgrData *md = data;
	StreamData *sd;

	if (!event) return;

	sd = gtk_clist_get_row_data(GTK_CLIST(md->stream_clist), row);

	if (!sd) return;

	panning_widget_set_stream(sd, md);
	return;
	widget = NULL;
	col = 0;
}

/*
 * -------------------------------------------------------------------------
 * sample list routines
 * -------------------------------------------------------------------------
 */

static void sample_list_mark_updated(GList *list)
{
	GList *work = list;
	while (work)
		{
		SampleData *sd = work->data;
		sd->updated = TRUE;
		work = work->next;
		}
}

static void sample_list_mark_dirty(GList *list)
{
	GList *work = list;
	while (work)
		{
		SampleData *sd = work->data;
		sd->dirty = TRUE;
		work = work->next;
		}
}

static void sample_list_free_dirty(MgrData *md)
{
	GList *work = md->samples;
	while (work)
		{
		SampleData *sd = work->data;
		work = work->next;
		if (sd->dirty)
			{
			gint row;
			md->samples = g_list_remove(md->samples, sd);
			row = gtk_clist_find_row_from_data(GTK_CLIST(md->sample_clist), sd);
			if (row >= 0)
				{
				gtk_clist_remove(GTK_CLIST(md->sample_clist), row);
				}
			if (md->sample_data_item == sd)
				{
				panning_widget_set_sample(NULL, md);
				}
			sample_data_free(sd);
			}
		}
}

static GList *sample_list_register(GList *list, esd_sample_info_t *sample)
{
	GList *work = list;
	gint updated = FALSE;

	while(work && !updated)
		{
		SampleData *sd = work->data;
		if (sd->id == sample->sample_id)
			{
			sample_data_update(sd, sample);
			updated = TRUE;
			}
		work = work->next;
		}

	if (!updated)
		{
		SampleData *sd = sample_data_new();
		sample_data_update(sd, sample);
		list = g_list_append(list, sd);
		}

	return list;
}

static gchar *sample_channels_as_text(gint channels)
{
	gchar *text;
	if (channels == 2)
		text = "Stereo";
	else
		text = "Mono";

	return g_strdup(text);
}

static gchar *enabled_as_text(gint toggle)
{
	if (toggle)
		return g_strdup("x");
	else
		return g_strdup(" ");
}

static gchar *length_as_text(SampleData *sd, gint as_time)
{
	if (as_time && sd->rate && sd->bits && sd->channels)
		{
		gint minutes;
		gint seconds;
		gint tenths;
		gfloat total = (gfloat)sd->length / sd->rate / (sd->bits / 8) / sd->channels + 0.05;
		seconds = (gint)total;
		tenths = (gfloat)(total - seconds) * 10;
		minutes = seconds / 60;
		seconds -= minutes * 60;
		return g_strdup_printf("%0d:%02d.%0d", minutes, seconds, tenths);
		}

	return g_strdup_printf("%d", sd->length);
}

static void refresh_sample_list_window(MgrData *md)
{
	gint row;
	GList *work = md->samples;
	gint frozen = FALSE;
	while(work)
		{
		SampleData *sd = work->data;
		if (sd->updated)
			{
			gint i;
			gchar *buf[10];
			row = gtk_clist_find_row_from_data(GTK_CLIST(md->sample_clist), sd);
			buf[0] = enabled_as_text(sd->playing);
			buf[1] = g_strdup_printf("%d", sd->id);
			buf[2] = g_strdup(sd->name);
			buf[3] = length_as_text(sd, md->samples_show_time);
			buf[4] = g_strdup_printf("%d", sd->rate);
			buf[5] = g_strdup_printf("%d", sd->bits);
			buf[6] = sample_channels_as_text(sd->channels);
			buf[7] = g_strdup_printf("%d,%d", sd->vol_left, sd->vol_right);
			buf[8] = enabled_as_text(sd->loop);
			buf[9] = NULL;
			if (!frozen)
				{
				gtk_clist_freeze(GTK_CLIST(md->sample_clist));
				frozen = TRUE;
				}
			if (row <0)
				{
				row = gtk_clist_append(GTK_CLIST(md->sample_clist), buf);
				gtk_clist_set_row_data(GTK_CLIST(md->sample_clist), row, sd);
				}
			else
				{
				for (i = 0; i < 9; i++)
					gtk_clist_set_text(GTK_CLIST(md->sample_clist), row, i, buf[i]);
				}
			for (i = 0; i < 9; i++) g_free(buf[i]);
			sd->updated = FALSE;
			}
		work = work->next;
		}
	if (frozen) gtk_clist_thaw(GTK_CLIST(md->sample_clist));
}

static void sample_selected_cb(GtkWidget *widget, gint row, gint col,
				GdkEvent *event, gpointer data)
{
	MgrData *md = data;
	SampleData *sd;

	if (!event) return;

	sd = gtk_clist_get_row_data(GTK_CLIST(md->sample_clist), row);

	if (!sd) return;

	sample_play_by_id(md, sd->id);
	panning_widget_set_sample(sd, md);
	return;
	widget = NULL;
	col = 0;
}

static void sample_time_clicked_cb(GtkWidget *widget, gint col, gpointer data)
{
	MgrData *md = data;
	if (md->samples_show_time)
		{
		gtk_clist_set_column_title (GTK_CLIST(md->sample_clist), col, _("Length"));
		md->samples_show_time = FALSE;
		}
	else
		{
		gtk_clist_set_column_title (GTK_CLIST(md->sample_clist), col, _("Time"));
		md->samples_show_time = TRUE;
		}
	sample_list_mark_updated(md->samples);
	refresh_sample_list_window(md);
	return;
	widget = NULL;
}

/*
 * -------------------------------------------------------------------------
 * server info routines
 * -------------------------------------------------------------------------
 */

static void server_info_update(MgrData *md, esd_server_info_t *server,
			       gint stream_count, gint sample_count)
{
	md->server_update = FALSE;
	if (md->server_version != server->version)
		{
		md->server_version = server->version;
		md->server_update = TRUE;
		}
	if (md->server_format != server->format)
		{
		md->server_format = server->format;

		if ((server->format & ESD_MASK_BITS) == ESD_BITS16)
			md->server_bits = 16;
		else
			md->server_bits = 8;

		if ((server->format & ESD_MASK_CHAN) == ESD_STEREO)
			md->server_channels = 2;
		else
			md->server_channels = 1;

		md->server_update = TRUE;
		}
	if (md->server_rate != server->rate)
		{
		md->server_rate = server->rate;
		md->server_update = TRUE;
		}
	if (md->server_stream_count != stream_count)
		{
		md->server_stream_count = stream_count;
		md->server_update = TRUE;
		}
	if (md->server_sample_count != sample_count)
		{
		md->server_sample_count = sample_count;
		md->server_update = TRUE;
		}
}

static void refresh_server_info(MgrData *md)
{
	gchar *buf[2];
	gchar *text;
	if (!md->server_update) return;

	buf[1] = NULL;

	gtk_clist_freeze(GTK_CLIST(md->server_clist));
	gtk_clist_clear(GTK_CLIST(md->server_clist));

	buf[0] = g_strdup_printf("Esound version: %d", md->server_version);
	gtk_clist_append(GTK_CLIST(md->server_clist), buf);
	g_free(buf[0]);

	if (md->server_channels == 2)
		text = "Stereo";
	else
		text = "Mono";

	buf[0] = g_strdup_printf("Output: %d, %d bits %s",
		md->server_rate, md->server_bits, text);
	gtk_clist_append(GTK_CLIST(md->server_clist), buf);
	g_free(buf[0]);

	buf[0] = g_strdup_printf("Connected streams: %d",
		md->server_stream_count);
	gtk_clist_append(GTK_CLIST(md->server_clist), buf);
	g_free(buf[0]);

	buf[0] = g_strdup_printf("Cached samples: %d",
		md->server_sample_count);
	gtk_clist_append(GTK_CLIST(md->server_clist), buf);
	g_free(buf[0]);

	gtk_clist_thaw(GTK_CLIST(md->server_clist));
}

/*
 * -------------------------------------------------------------------------
 * update callback
 * -------------------------------------------------------------------------
 */

static gint manager_window_update_cb(gpointer data)
{
	MgrData *md = data;
	esd_info_t *info = NULL;
	esd_sample_info_t *sample = NULL;
	esd_player_info_t *stream = NULL;
	gint esd_fd;
	gint stream_count = 0;
	gint sample_count = 0;

	esd_fd = esd_open_sound(md->ad->esd_host);
                if (esd_fd>= 0)
                        {
			info = esd_get_all_info(esd_fd);
                        esd_close(esd_fd);
                        }
		else
			{
			return TRUE;
			}

	if (!info) return TRUE;

	/* process streams */

	stream_list_mark_dirty(md->streams);
	stream = info->player_list;
	while(stream)
		{
		md->streams = stream_list_register(md->streams, stream);
		stream = stream->next;
		stream_count++;
		}
	stream_list_free_dirty(md);

	/* process samples */

	sample_list_mark_dirty(md->samples);
	sample = info->sample_list;
	while(sample)
		{
		md->samples = sample_list_register(md->samples, sample);
		sample = sample->next;
		sample_count++;
		}
	sample_list_free_dirty(md);

	/* process server info */

	server_info_update(md, info->server, stream_count, sample_count);

	/* finish up */

	esd_free_all_info(info);

	refresh_server_info(md);
	refresh_stream_list_window(md);
	refresh_sample_list_window(md);

	return TRUE;
}

static void manager_window_close_cb(GtkWidget *w, gpointer data)
{
	MgrData *md = data;
	AppData *ad;
	ad = md->ad;

	gtk_widget_destroy(md->window);
	gtk_timeout_remove(md->update_timeout_id);
	mgr_data_free(md);

	ad->manager = NULL;
	return;
	w = NULL;
}

static void manager_window_delete_cb(GtkWidget *w, GdkEvent *event, gpointer data)
{
	MgrData *md = data;
	manager_window_close_cb(NULL, md);
	return;
	w = NULL;
	event = NULL;
}

void manager_window_close(AppData *ad)
{
	MgrData *md = ad->manager;

	if (!md) return;

	manager_window_close_cb(NULL, md);
}

void manager_window_show(AppData *ad)
{
	gint i;
	MgrData *md;
	GtkWidget *main_vbox;
	GtkWidget *vbox;
	GtkWidget *vbox1;
	GtkWidget *hbox;
	GtkWidget *button;
	GtkWidget *label;
	GtkWidget *frame;
	GtkWidget *scrolled;

	GtkObject *adj;

	gchar *stream_titles [] = { N_("Id"), N_("Name"), N_("Type"), N_("Rate"), N_("Bits"), N_("Format"), N_("Volume"), };
	gchar *sample_titles [] = { N_("P"), N_("Id"), N_("Name"), N_("Length"), N_("Rate"), N_("Bits"), N_("Format"), N_("Volume"), N_("L"), };

	for (i = 0; i < 6; i++)
	    stream_titles[i] = _(stream_titles[i]);
	
	for (i = 0; i < 9; i++)
	    sample_titles[i] = _(sample_titles[i]);
    
	if (ad->manager)
		{
		md = ad->manager;
		gdk_window_raise(md->window->window);
		return;
		}

	md = mgr_data_new();

	md->ad = ad;

	ad->manager = md;

	md->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_signal_connect (GTK_OBJECT (md->window), "delete_event", manager_window_delete_cb, md);
        gtk_window_set_policy (GTK_WINDOW(md->window), FALSE, TRUE, FALSE);
        gtk_window_set_title (GTK_WINDOW (md->window), _("Sound Monitor - Manager"));
	gtk_widget_set_usize(md->window, 500, 300);

	gtk_window_set_wmclass(GTK_WINDOW(md->window), "manager", "Sound Monitor applet");

	main_vbox = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
        gtk_container_set_border_width(GTK_CONTAINER(main_vbox), GNOME_PAD_SMALL);
	gtk_container_add(GTK_CONTAINER(md->window), main_vbox);
	gtk_widget_show(main_vbox);

	hbox = gtk_hbox_new (TRUE, 0);
	gtk_box_pack_end(GTK_BOX(main_vbox), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

	button = gnome_stock_button(GNOME_STOCK_BUTTON_CLOSE);
	gtk_signal_connect (GTK_OBJECT(button), "clicked", manager_window_close_cb, md);
	gtk_box_pack_end(GTK_BOX(hbox), button, FALSE, FALSE, GNOME_PAD);
	gtk_widget_show(button);

	md->notebook = gtk_notebook_new();
	gtk_notebook_set_tab_pos (GTK_NOTEBOOK(md->notebook), GTK_POS_TOP);
	gtk_box_pack_start (GTK_BOX(main_vbox), md->notebook, TRUE, TRUE, 0);

        /* server info */

	frame = gtk_frame_new(_("Server information:"));
	gtk_container_set_border_width (GTK_CONTAINER (frame), GNOME_PAD_SMALL);
	gtk_widget_show(frame);
	label = gtk_label_new(_("Server"));
	gtk_notebook_append_page (GTK_NOTEBOOK(md->notebook), frame, label);

	vbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), GNOME_PAD_SMALL);
	gtk_container_add (GTK_CONTAINER(frame), vbox);
	gtk_widget_show(vbox);

	scrolled = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
				GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
	gtk_box_pack_start(GTK_BOX(vbox), scrolled, TRUE, TRUE, 0);
	gtk_widget_show(scrolled);

	md->server_clist = gtk_clist_new(1);
	gtk_container_add(GTK_CONTAINER(scrolled), md->server_clist);
	gtk_widget_show(md->server_clist);

        /* stream info */

	frame = gtk_frame_new(_("Connected streams:"));
	gtk_container_set_border_width(GTK_CONTAINER(frame), GNOME_PAD_SMALL);
	gtk_widget_show(frame);
	label = gtk_label_new(_("Streams"));
	gtk_notebook_append_page (GTK_NOTEBOOK(md->notebook), frame, label);

	vbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), GNOME_PAD_SMALL);
	gtk_container_add (GTK_CONTAINER(frame),vbox);
	gtk_widget_show(vbox);

	scrolled = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
				GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
	gtk_box_pack_start(GTK_BOX(vbox), scrolled, TRUE, TRUE, 0);
	gtk_widget_show(scrolled);

	md->stream_clist = gtk_clist_new_with_titles(7, stream_titles);
	gtk_clist_column_titles_passive (GTK_CLIST(md->stream_clist));
	gtk_container_add(GTK_CONTAINER(scrolled), md->stream_clist);
	gtk_signal_connect (GTK_OBJECT(md->stream_clist), "select_row",
		(GtkSignalFunc) stream_selected_cb, md);
	gtk_widget_show(md->stream_clist);

	gtk_clist_set_column_width (GTK_CLIST(md->stream_clist), 0, 20);
	gtk_clist_set_column_width (GTK_CLIST(md->stream_clist), 1, 170);
	gtk_clist_set_column_width (GTK_CLIST(md->stream_clist), 2, 45);
	gtk_clist_set_column_width (GTK_CLIST(md->stream_clist), 3, 40);
	gtk_clist_set_column_width (GTK_CLIST(md->stream_clist), 4, 20);
	gtk_clist_set_column_width (GTK_CLIST(md->stream_clist), 5, 40);
	gtk_clist_set_column_width (GTK_CLIST(md->stream_clist), 6, 50);

	hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

	md->stream_id_entry = gtk_entry_new();
	gtk_widget_set_usize(md->stream_id_entry, 30, -1);
	gtk_box_pack_start(GTK_BOX(hbox), md->stream_id_entry, FALSE, FALSE, 0);
	gtk_widget_set_sensitive(md->stream_id_entry, FALSE);
	gtk_widget_show(md->stream_id_entry);

	md->stream_name_entry = gtk_entry_new();
	gtk_widget_set_usize(md->stream_name_entry, 190, -1);
	gtk_box_pack_start(GTK_BOX(hbox), md->stream_name_entry, FALSE, FALSE, 0);
	gtk_widget_set_sensitive(md->stream_name_entry, FALSE);
	gtk_widget_show(md->stream_name_entry);

	vbox1 = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), vbox1, TRUE, TRUE, GNOME_PAD_SMALL);
	gtk_widget_show(vbox1);

/* */	adj = gtk_adjustment_new(0.0, 0.0, 256.0, 1.0, 1.0, 0.0);
	md->stream_vol_scale = gtk_hscale_new (GTK_ADJUSTMENT(adj));
	gtk_range_set_update_policy (GTK_RANGE(md->stream_vol_scale), GTK_UPDATE_DELAYED);
	gtk_scale_set_digits (GTK_SCALE(md->stream_vol_scale), TRUE);
	gtk_signal_connect(GTK_OBJECT(adj), "value_changed", GTK_SIGNAL_FUNC(stream_vol_changed_cb), md);
	gtk_box_pack_start(GTK_BOX(vbox1), md->stream_vol_scale, FALSE, FALSE, 0);
        gtk_widget_show(md->stream_vol_scale);

	label = gtk_label_new(_("Volume"));
	gtk_box_pack_start(GTK_BOX(vbox1), label, FALSE, FALSE, 0);
	gtk_widget_show(label);

	vbox1 = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), vbox1, TRUE, TRUE, 0);
	gtk_widget_show(vbox1);

	adj = gtk_adjustment_new(0.0, -100.0, 100.0, 1.0, 1.0, 0.0);
	md->stream_pan_scale = gtk_hscale_new (GTK_ADJUSTMENT(adj));
	gtk_range_set_update_policy (GTK_RANGE(md->stream_pan_scale), GTK_UPDATE_DELAYED);
	gtk_scale_set_digits (GTK_SCALE(md->stream_pan_scale), TRUE);
	gtk_signal_connect(GTK_OBJECT(adj), "value_changed", GTK_SIGNAL_FUNC(stream_pan_changed_cb), md);
	gtk_box_pack_start(GTK_BOX(vbox1), md->stream_pan_scale, FALSE, FALSE, 0);
        gtk_widget_show(md->stream_pan_scale);

	label = gtk_label_new(_("Balance"));
	gtk_box_pack_start(GTK_BOX(vbox1), label, FALSE, FALSE, 0);
	gtk_widget_show(label);

        /* sample info */

	frame = gtk_frame_new(_("Cached samples: (select to play)"));
	gtk_container_set_border_width(GTK_CONTAINER(frame), GNOME_PAD_SMALL);
	gtk_widget_show(frame);
	label = gtk_label_new(_("Samples"));
	gtk_notebook_append_page (GTK_NOTEBOOK(md->notebook), frame, label);

	vbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), GNOME_PAD_SMALL);
	gtk_container_add (GTK_CONTAINER(frame),vbox);
	gtk_widget_show(vbox);

	scrolled = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
				GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
	gtk_box_pack_start(GTK_BOX(vbox), scrolled, TRUE, TRUE, 0);
	gtk_widget_show(scrolled);

	md->sample_clist = gtk_clist_new_with_titles(9, sample_titles);
	gtk_clist_column_titles_passive (GTK_CLIST(md->sample_clist));
	gtk_signal_connect (GTK_OBJECT(md->sample_clist), "select_row",
		(GtkSignalFunc) sample_selected_cb, md);
	gtk_container_add(GTK_CONTAINER(scrolled), md->sample_clist);
	gtk_widget_show(md->sample_clist);

	gtk_clist_column_title_active(GTK_CLIST(md->sample_clist),3);
	gtk_signal_connect (GTK_OBJECT(md->sample_clist), "click_column",
		(GtkSignalFunc) sample_time_clicked_cb, md);

	gtk_clist_set_column_width (GTK_CLIST(md->sample_clist), 0, 10);
	gtk_clist_set_column_width (GTK_CLIST(md->sample_clist), 1, 20);
	gtk_clist_set_column_width (GTK_CLIST(md->sample_clist), 2, 130);
	gtk_clist_set_column_width (GTK_CLIST(md->sample_clist), 3, 50);
	gtk_clist_set_column_width (GTK_CLIST(md->sample_clist), 4, 45);
	gtk_clist_set_column_width (GTK_CLIST(md->sample_clist), 5, 20);
	gtk_clist_set_column_width (GTK_CLIST(md->sample_clist), 6, 40);
	gtk_clist_set_column_width (GTK_CLIST(md->sample_clist), 7, 50);
	gtk_clist_set_column_width (GTK_CLIST(md->sample_clist), 8, 10);

	hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

	md->sample_id_entry = gtk_entry_new();
	gtk_widget_set_usize(md->sample_id_entry, 30, -1);
	gtk_box_pack_start(GTK_BOX(hbox), md->sample_id_entry, FALSE, FALSE, 0);
	gtk_widget_set_sensitive(md->sample_id_entry, FALSE);
	gtk_widget_show(md->sample_id_entry);

	md->sample_name_entry = gtk_entry_new();
	gtk_widget_set_usize(md->sample_name_entry, 190, -1);
	gtk_box_pack_start(GTK_BOX(hbox), md->sample_name_entry, FALSE, FALSE, 0);
	gtk_widget_set_sensitive(md->sample_name_entry, FALSE);
	gtk_widget_show(md->sample_name_entry);

	vbox1 = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), vbox1, TRUE, TRUE, GNOME_PAD_SMALL);
	gtk_widget_show(vbox1);

/* */	adj = gtk_adjustment_new(0.0, 0.0, 256.0, 1.0, 1.0, 0.0);
	md->sample_vol_scale = gtk_hscale_new (GTK_ADJUSTMENT(adj));
	gtk_range_set_update_policy (GTK_RANGE(md->sample_vol_scale), GTK_UPDATE_DELAYED);
	gtk_scale_set_digits (GTK_SCALE(md->sample_vol_scale), TRUE);
	gtk_signal_connect(GTK_OBJECT(adj), "value_changed", GTK_SIGNAL_FUNC(sample_vol_changed_cb), md);
	gtk_box_pack_start(GTK_BOX(vbox1), md->sample_vol_scale, FALSE, FALSE, 0);
        gtk_widget_show(md->sample_vol_scale);

	label = gtk_label_new(_("Volume"));
	gtk_box_pack_start(GTK_BOX(vbox1), label, FALSE, FALSE, 0);
	gtk_widget_show(label);

	vbox1 = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), vbox1, TRUE, TRUE, 0);
	gtk_widget_show(vbox1);

	adj = gtk_adjustment_new(0.0, -100.0, 100.0, 1.0, 1.0, 0.0);
	md->sample_pan_scale = gtk_hscale_new (GTK_ADJUSTMENT(adj));
	gtk_range_set_update_policy (GTK_RANGE(md->sample_pan_scale), GTK_UPDATE_DELAYED);
	gtk_scale_set_digits (GTK_SCALE(md->sample_pan_scale), TRUE);
	gtk_signal_connect(GTK_OBJECT(adj), "value_changed", GTK_SIGNAL_FUNC(sample_pan_changed_cb), md);
	gtk_box_pack_start(GTK_BOX(vbox1), md->sample_pan_scale, FALSE, FALSE, 0);
        gtk_widget_show(md->sample_pan_scale);

	label = gtk_label_new(_("Balance"));
	gtk_box_pack_start(GTK_BOX(vbox1), label, FALSE, FALSE, 0);
	gtk_widget_show(label);

	gtk_notebook_set_page (GTK_NOTEBOOK(md->notebook), 0);
        gtk_widget_show(md->notebook);

	manager_window_update_cb(md); /* force initial instant update */

	panning_widget_set_stream(NULL, md);
	panning_widget_set_sample(NULL, md);

        gtk_widget_show(md->window);

	md->update_timeout_id = gtk_timeout_add(500, (GtkFunction) manager_window_update_cb, md);
}

