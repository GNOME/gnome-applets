/* tick-a-stat, a GNOME panel applet
 * (C) 1999 John Ellis
 *
 * Author: John Ellis
 *
 */

#include "tickastat.h"
#include "modules.h"

#include <dirent.h>
#include <sys/stat.h>

#define NEWS_LINE_PRIORITY 1
#define NEWS_LINE_DELAY_MIN 1.0
#define NEWS_LINE_DELAY_MAX 120.0

/*
 *-------------------------------------------------------------------
 * The file new module for Tick-A-Stat
 * 
 * runs scripts to get news and information.
 * 
 *-------------------------------------------------------------------
 */

typedef struct _NewsData NewsData;
typedef struct _ScriptData ScriptData;
typedef struct _ArticleData ArticleData;

struct _ArticleData
{
	gchar *heading;
	gchar *topic;
	gchar *url;
	gchar *date;
	gchar *image_path;
	GList *body;
	ScriptData *sd;
	InfoData *id;
};

struct _ScriptData
{
	NewsData *nd;

	gint enabled;

	gchar *name;
	gchar *comment;
	gchar *binary;
	gchar *data_path;
	gchar *data_file;
	gchar *data_file_temp;

	gchar *path;
	gchar *config_id;

	gint update_min;
	gint update_max;
	gint update_default;
	gint update_val;

	gint line_delay;
	gint clear_on_refresh;

	gint show_date;
	gint show_topic;
	gint show_image;
	gint show_body;

	gint C_enabled;
	gint C_update_val;
	gint C_line_delay;

	gint C_show_date;
	gint C_show_topic;
	gint C_show_image;
	gint C_show_body;

	FILE *fp_id;
	gint fd_id;
	gint cb_id;

	GList *lines;

	gint check_count;
};

struct _NewsData
{
	ModuleData *md;
	AppData *ad;

	gint enabled;

	gint C_enabled;

	GtkWidget *C_clist;

	GtkWidget *C_script_button;
	GtkWidget *C_label;
	GtkObject * C_line_adj;
	GtkWidget *C_line_spin;
	GtkObject * C_update_adj;
	GtkWidget *C_update_spin;
	gint C_row;

	GtkWidget *C_date_button;
	GtkWidget *C_topic_button;
	GtkWidget *C_image_button;
	GtkWidget *C_body_button;

	GList *list;

	gint loop_cb_id;
};

static void script_edit_set_to_row(NewsData *nd, gint row);
static void script_start(ScriptData *sd);
static void script_stop(ScriptData *sd);

static void script_lines_clear(ScriptData *sd, gint keep_unshown)
{
	GList *work = sd->lines;
	while(work)
		{
		ArticleData *d = work->data;
		InfoData *id = d->id;
		work = work->next;
		sd->lines = g_list_remove(sd->lines, d);

		if (!id->shown && keep_unshown)
			{
			id->show_count = 1; /* set to delete after 1 show */
			}
		else
			{
			remove_info_line(sd->nd->ad, id);
			}
		}
}

static void script_data_free(ScriptData *sd)
{
	if (!sd) return;

	script_stop(sd);
	
	g_free(sd->name);
	g_free(sd->comment);
	g_free(sd->binary);
	g_free(sd->data_path);
	g_free(sd->data_file);
	g_free(sd->data_file_temp);

	g_free(sd->path);
	g_free(sd->config_id);

	g_free(sd);
}

static ScriptData *script_data_new(NewsData *nd, gchar *path, gchar *data_path)
{
	ScriptData *sd;
	gchar *buf;
	gchar *prefix;

	printf("load script: %s\n", path);

	prefix = g_strconcat("=", path, "=/main/", NULL);
	gnome_config_push_prefix(prefix);
	g_free(prefix);

	buf = gnome_config_get_string("config_id=");
	if (!buf || (buf && strlen(buf) == 0) )
		{
		gnome_config_pop_prefix();
		g_free(buf);
		return NULL;
		}

	sd = g_new0(ScriptData, 1);

	sd->nd = nd;
	sd->path = g_strndup(path, g_filename_index(path) - 1);
	sd->config_id = buf;

	sd->name = gnome_config_get_string("name=??");
	sd->comment = gnome_config_get_string("comment=");

	sd->data_path = g_strdup(data_path);

	buf = gnome_config_get_string("script=");
	sd->binary = g_concat_dir_and_file(sd->path, buf);
	g_free(buf);

	sd->data_file = gnome_config_get_string("datafile=");
	sd->data_file_temp = NULL;

	sd->update_min = gnome_config_get_int("update_min=1");
	sd->update_max = gnome_config_get_int("update_max=120");
	sd->update_default = gnome_config_get_int("update_default=20");

	sd->clear_on_refresh = gnome_config_get_int("clear_unshown_on_refresh=1");
	sd->line_delay = gnome_config_get_int("line_delay=4");

	sd->show_date = gnome_config_get_int("show_date=1");
	sd->show_topic = gnome_config_get_int("show_topic=0");
	sd->show_image = gnome_config_get_int("show_image=1");
	sd->show_body = gnome_config_get_int("show_body=0");

	gnome_config_pop_prefix();

	sd->enabled = FALSE;

	sd->lines = NULL;
	sd->update_val = sd->update_default;

	sd->fp_id = NULL;
	sd->fd_id = -1;
	sd->cb_id = -1;

	printf("%s = %d - %d (%d)\n", sd->name, sd->update_min, sd->update_max, sd->update_val);

	return sd;
}


/*
 *-------------------------------------------------------------------
 * dialog(s)
 *-------------------------------------------------------------------
 */



/*
 *-------------------------------------------------------------------
 * articles
 *-------------------------------------------------------------------
 */

static void article_free_data(ArticleData *d)
{
	GList *work;

	if (!d) return;

	g_free(d->heading);
	g_free(d->topic);
	g_free(d->url);
	g_free(d->date);
	g_free(d->image_path);

	work = d->body;
	while(work)
		{
		g_free(work->data);
		work = work->next;
		}

	g_list_free(d->body);

	g_free(d);
}

static void line_click_cb(ModuleData *md, gpointer data, InfoData *id, AppData *ad)
{
	ArticleData *d = data;

	if (!d) return;

	if (d->url) gnome_url_show(d->url);
        return;
        ad = NULL;
        md = NULL;
	id = NULL;
}

static void line_free_cb(ModuleData *md, gpointer data)
{
	ArticleData *d = data;
	ScriptData *sd;

	if (!d) return;

	sd = d->sd;
	sd->lines = g_list_remove(sd->lines, d);
	article_free_data(d);
        return;
        md = NULL;
}

static void line_add(ScriptData *sd, ArticleData *d, gint show_count)
{
	InfoData *id;
	GString *str;
	gchar *text = "";
	gchar *img = NULL;

	str = g_string_new(NULL);
	if (sd->show_body)
		{
		GList *work = d->body;
		while(work)
			{
			g_string_append(str, work->data);
			work = work->next;
			if (work) g_string_append(str, "\n");
			}
		}
	if (sd->show_image)
		{
		img = d->image_path;
		}
	if (sd->show_date && d->date)
		{
		g_string_prepend(str, "\n");
		g_string_prepend(str, d->date);
		}
	if (sd->show_topic && d->topic)
		{
		g_string_prepend(str, "\n");
		g_string_prepend(str, d->topic);
		}
	g_string_prepend(str, "\n");
	g_string_prepend(str, d->heading);

	if (str->str && strlen(str->str) > 0) text = str->str;

	id = add_info_line(sd->nd->ad, text, img, 0, FALSE,
		show_count, sd->line_delay * 10, NEWS_LINE_PRIORITY);

	if (d->url)
		set_info_signals(id, sd->nd->md, line_click_cb, line_free_cb, NULL, NULL, d);
	else
		set_info_signals(id, sd->nd->md, NULL, line_free_cb, NULL, NULL, d);

	d->id = id;

	g_string_free(str, TRUE);
}

static gchar *load_key(gchar *key, gint number)
{
	gchar *buf = g_strdup_printf("=/article_%d/%s=", number, key);
	gchar *ret = gnome_config_get_string(buf);
	g_free(buf);

	if (ret && strlen(ret) == 0)
		{
		g_free(ret);
		ret = NULL;
		}

	return ret;
}

static gint load_key_int(gchar *key, gint number)
{
	gchar *buf = g_strdup_printf("=/article_%d/%s=0", number, key);
	gint ret = gnome_config_get_int(buf);
	g_free(buf);
	return ret;
}

static void script_load_body(ScriptData *sd, ArticleData *d, gint number)
{
	gint i = 0;
	gint finished = FALSE;

	while(!finished)
		{
		gchar *key;
		gchar *buf;

		key = g_strdup_printf("body_%d", i);
		buf = load_key(key, number);
		g_free(key);

		if (buf)
			{
			d->body = g_list_append(d->body, buf);
			}
		else
			{
			finished = TRUE;
			}

		i++;
		}
        return;
        sd = NULL;
}

static gint script_load_article(ScriptData *sd, gint number)
{
	gchar *buf;
	ArticleData *d;
	gint show_count;

	buf = load_key("heading", number);

	if (!buf) return FALSE;

	d = g_new0(ArticleData, 1);
	d->sd = sd;

	d->heading = buf;

	d->topic = load_key("topic", number);
	d->url = load_key("url", number);
	d->date = load_key("date", number);

	d->image_path = NULL;
	show_count = load_key_int("display_count", number);

	buf = load_key("image", number);
	if (buf)
		{
		d->image_path = g_concat_dir_and_file(sd->data_path, buf);
		g_free(buf);
		}
	else
		{
		buf = load_key("image_lib", number);
		if (buf)
			{
			d->image_path = g_concat_dir_and_file(sd->path, buf);
			g_free(buf);
			}
		}

	d->body = NULL;

	script_load_body(sd, d, number);

	line_add(sd, d, show_count);

	sd->lines = g_list_append(sd->lines, d);

	return TRUE;	
}

static void script_load_all(ScriptData *sd)
{
	gint i;
	gint finished = FALSE;

	i = 0;
	while(!finished)
		{
		finished = !script_load_article(sd, i);
		i++;
		}
}

static void script_load_info(ScriptData *sd)
{
	gchar *prefix;
	gchar *new_file;

	script_lines_clear(sd, !sd->clear_on_refresh);
	if (!g_file_exists(sd->data_file_temp))
		{
		printf("unable to find input: %s\n", sd->data_file_temp);
		return;
		}

	prefix = g_strconcat("=", sd->data_file_temp, NULL);
	gnome_config_push_prefix(prefix);

	script_load_all(sd);

	gnome_config_pop_prefix();
	g_free(prefix);

	new_file = g_concat_dir_and_file(sd->data_path, sd->data_file);
	if (rename(sd->data_file_temp, new_file) < 0)
		{
		unlink (sd->data_file_temp);
		}
	g_free(sd->data_file_temp);
	sd->data_file_temp = NULL;
	g_free(new_file);
}

/*
 *-------------------------------------------------------------------
 * scripts
 *-------------------------------------------------------------------
 */

static gint script_update_cb(gpointer data)
{
	ScriptData *sd = data;
	gchar buf[200];
	gint is_done = FALSE;
	gint is_pass = FALSE;
	gint is_fail = FALSE;
	gint is_error = FALSE;
	GString *str = NULL;

	while (fgets(buf, 200, sd->fp_id) != NULL)
		{
		if (is_done || strncmp(buf, "done", 4) == 0)
			{
			is_done = TRUE;
			}
		else if (is_pass || strncmp(buf, "pass", 4) == 0)
			{
			is_pass = TRUE;
			}
		else if (strncmp(buf, "fail", 4) == 0)
			{
			is_fail = TRUE;
			}
		else if (strncmp(buf, "error", 5) == 0)
			{
			is_error = TRUE;
			}
		else
			{
			if (!is_fail) is_error = TRUE;
			/* collect error messages here*/
			if (!str) str = g_string_new(NULL);

			g_string_append(str, buf);
			}
		}

	if (is_done)
		{
		script_load_info(sd);
		}
	else if (is_pass)
		{
		}
	else if (is_fail)
		{
		}
	else if (is_error)
		{
		g_string_append(str, "\n\nThis results in no update.");
		g_string_prepend(str, "\" reported:\n");
		g_string_prepend(str, sd->name);
		g_string_prepend(str, _("script \""));
		g_string_prepend(str, _("Tick-a-Stat script encountered an error:\n"));
		gnome_warning_dialog(str->str);
		}

	g_string_free(str, TRUE);

	if (feof(sd->fp_id) != 0)
		{
		pclose(sd->fp_id);
		sd->fp_id = NULL;
		sd->fd_id = -1;
		sd->cb_id = -1;
		printf("closed: %s\n", sd->name);
		return FALSE;
		}

	return TRUE;
}

static void script_update(ScriptData *sd)
{
	gchar *stamp;
	gchar *command;

	if (!sd->enabled || sd->cb_id > -1) return;

	printf("Doing update for %s\n", sd->name);

	g_free(sd->data_file_temp);
	stamp = time_stamp(TRUE);
	sd->data_file_temp = g_strconcat(sd->data_path, "/", sd->data_file, "-", stamp, NULL);
	g_free(stamp);

	command = g_strconcat(sd->binary, " ", sd->data_file_temp, " ", sd->data_path, NULL);

	printf("Command is %s\n", command);

	sd->fp_id = popen(command, "r");
	g_free(command);
	if (sd->fp_id == NULL)
		{
		printf("Failed to do command for: %s\n", sd->name);
		return;
		}

	sd->fd_id = fileno (sd->fp_id);

	sd->cb_id = gtk_timeout_add(1000, script_update_cb, sd);
}

static void script_start(ScriptData *sd)
{
	if (!sd->enabled || sd->cb_id > -1) return;
	script_update(sd);
	sd->check_count = 0;
}

static void script_stop(ScriptData *sd)
{
	if (!sd) return;

	if (sd->cb_id > -1) gtk_timeout_remove(sd->cb_id);
	if (sd->fp_id != NULL) pclose(sd->fp_id);

	sd->cb_id = -1;
	sd->fp_id = NULL;
	sd->fd_id = -1;

	script_lines_clear(sd, FALSE);
}

static void script_lines_reset(ScriptData *sd)
{
	GList *work = sd->lines;
	while(work)
		{
		ArticleData *d = work->data;
		InfoData *id = d->id;
		work = work->next;
		id->data = NULL; /* orphan the data so that ArticleData is not freed */
		line_add(sd, d, id->show_count);
		remove_info_line(sd->nd->ad, id);
		}
}

static void script_line_delay_adjust(ScriptData *sd)
{
	GList *work;

	work = sd->lines;
	while(work)
		{
		ArticleData *d = work->data;
		InfoData *id = d->id;
		id->end_delay = sd->line_delay * 10;
		work = work->next;
		}
}

static void script_state_check(ScriptData *sd, gint restart)
{
	if (!sd->enabled)
		{
		script_stop(sd);
		return;
		}

	if (sd->enabled && (restart || sd->cb_id == -1))
		{
		if (sd->cb_id > -1) script_stop(sd);
		script_start(sd);
		return;
		}
}

/*
 *-------------------------------------------------------------------
 * main loop (called every minute)
 *-------------------------------------------------------------------
 */

static void script_do_ref(ScriptData *sd)
{
	if (!sd->enabled && sd->cb_id > -1)
		{
		script_stop(sd);
		return;
		}

	if (!sd->enabled) return;

	sd->check_count++;

	if ((sd->check_count <= 0 ||sd->check_count >= sd->update_val) && sd->cb_id == -1)
		{
		sd->check_count = 0;
		script_update(sd);
		}
}

static gint main_loop_cb(gpointer data)
{
	NewsData *nd = data;
	GList *work;

	work = nd->list;
	while(work)
		{
		ScriptData *sd = work->data;
		script_do_ref(sd);
		work = work->next;
		}

	return TRUE;
}

/*
 *-------------------------------------------------------------------
 * start and stop
 *-------------------------------------------------------------------
 */

static void news_start(NewsData *nd)
{
	GList *work;

	if (nd->loop_cb_id > -1) return;

	work = nd->list;
	while(work)
		{
		ScriptData *sd = work->data;
		sd->check_count = -1;
		work = work->next;
		}

	main_loop_cb(nd); /* get things to update now */
	nd->loop_cb_id = gtk_timeout_add(60000, main_loop_cb, nd);
}

static void news_stop(NewsData *nd)
{
	GList *work;

	if (nd->loop_cb_id == -1) return;

	gtk_timeout_remove(nd->loop_cb_id);
	nd->loop_cb_id = -1;

	work = nd->list;
	while(work)
		{
		ScriptData *sd = work->data;
		script_stop(sd);
		work = work->next;
		}
}

/*
 *-------------------------------------------------------------------
 * config callbacks
 *-------------------------------------------------------------------
 */

static void enabled_checkbox_cb(GtkWidget *widget, gpointer data)
{
	NewsData *nd = data;
	nd->C_enabled = GTK_TOGGLE_BUTTON (widget)->active;
	property_changed(nd->md, nd->ad);
}

static gint generic_toggle(GtkWidget *widget, NewsData *nd, gint column)
{
	gchar *text = "";
	gint n;

	n = GTK_TOGGLE_BUTTON (widget)->active;

	if (n) text = "X";
	gtk_clist_set_text(GTK_CLIST(nd->C_clist), nd->C_row, column, text);

	property_changed(nd->md, nd->ad);

	return n;
}

static void show_date_cb(GtkWidget *widget, gpointer data)
{
	NewsData *nd = data;
	ScriptData *sd = gtk_clist_get_row_data(GTK_CLIST(nd->C_clist), nd->C_row);
	if (!sd) return;

	sd->C_show_date = generic_toggle(widget, data, 4);
}

static void show_topic_cb(GtkWidget *widget, gpointer data)
{
	NewsData *nd = data;
	ScriptData *sd = gtk_clist_get_row_data(GTK_CLIST(nd->C_clist), nd->C_row);
	if (!sd) return;

	sd->C_show_topic = generic_toggle(widget, data, 5);
}

static void show_image_cb(GtkWidget *widget, gpointer data)
{
	NewsData *nd = data;
	ScriptData *sd = gtk_clist_get_row_data(GTK_CLIST(nd->C_clist), nd->C_row);
	if (!sd) return;

	sd->C_show_image = generic_toggle(widget, data, 6);
}

static void show_body_cb(GtkWidget *widget, gpointer data)
{
	NewsData *nd = data;
	ScriptData *sd = gtk_clist_get_row_data(GTK_CLIST(nd->C_clist), nd->C_row);
	if (!sd) return;

	sd->C_show_body = generic_toggle(widget, data, 7);
}

static void enabled_script_cb(GtkWidget *widget, gpointer data)
{
	NewsData *nd = data;
	ScriptData *sd = gtk_clist_get_row_data(GTK_CLIST(nd->C_clist), nd->C_row);
	if (!sd) return;

	sd->C_enabled = generic_toggle(widget, data, 0);
}

static void line_delay_cb(GtkObject *adj, gpointer data)
{
        NewsData *nd = data;
	ScriptData *sd = gtk_clist_get_row_data(GTK_CLIST(nd->C_clist), nd->C_row);
	gchar *text;

	if (!sd) return;

	if (GTK_IS_ADJUSTMENT(adj))
		sd->C_line_delay = (gint)GTK_ADJUSTMENT(adj)->value;
	else
		sd->C_line_delay = (gint)GTK_ADJUSTMENT(GTK_SPIN_BUTTON(adj)->adjustment)->value;

	text = g_strdup_printf("%d", sd->C_line_delay);
	gtk_clist_set_text(GTK_CLIST(nd->C_clist), nd->C_row, 2, text);
	g_free(text);

	property_changed(nd->md, nd->ad);
}

static void update_delay_cb(GtkObject *adj, gpointer data)
{
        NewsData *nd = data;
	ScriptData *sd = gtk_clist_get_row_data(GTK_CLIST(nd->C_clist), nd->C_row);
	gchar *text;

	if (!sd) return;

	if (GTK_IS_ADJUSTMENT(adj))
		sd->C_update_val = (gint)GTK_ADJUSTMENT(adj)->value;
	else
		sd->C_update_val = (gint)GTK_ADJUSTMENT(GTK_SPIN_BUTTON(adj)->adjustment)->value;

	text = g_strdup_printf("%d", sd->C_update_val);
	gtk_clist_set_text(GTK_CLIST(nd->C_clist), nd->C_row, 3, text);
	g_free(text);

	property_changed(nd->md, nd->ad);
}

static void script_row_select_cb(GtkWidget *widget, gint row, gint col,
				 GdkEvent *event, gpointer data)
{
	NewsData *nd = data;
	ScriptData *sd;

	sd = gtk_clist_get_row_data(GTK_CLIST(nd->C_clist), row);

	if (!sd)
		{
		script_edit_set_to_row(nd, -1);
		return;
		}

	script_edit_set_to_row(nd, row);
        return;
	widget = NULL;
	col = 0;
	event = NULL;
}

/*
 *-------------------------------------------------------------------
 * config window utils
 *-------------------------------------------------------------------
 */

static void script_edit_set_sensitive(NewsData *nd, gchar *text)
{
	gint s = FALSE;

	if (text)
		{
		s = TRUE;
		}
	else
		{
		text = "";
		}

	gtk_widget_set_sensitive(nd->C_script_button, s);
	gtk_entry_set_text(GTK_ENTRY(nd->C_label), text);
	gtk_widget_set_sensitive(nd->C_line_spin, s);
	gtk_widget_set_sensitive(nd->C_update_spin, s);
	gtk_widget_set_sensitive(nd->C_date_button, s);
	gtk_widget_set_sensitive(nd->C_topic_button, s);
	gtk_widget_set_sensitive(nd->C_image_button, s);
	gtk_widget_set_sensitive(nd->C_body_button, s);
}

static void script_edit_set_to_row(NewsData *nd, gint row)
{
	ScriptData *sd;

	sd = gtk_clist_get_row_data(GTK_CLIST(nd->C_clist), row);

	if (!sd)
		{
		nd->C_row = -1;
		script_edit_set_sensitive(nd, NULL);
		return;
		}

	nd->C_row = row;

	script_edit_set_sensitive(nd, sd->name);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(nd->C_script_button), sd->C_enabled);

	gtk_adjustment_set_value(GTK_ADJUSTMENT(nd->C_line_adj), (gfloat)sd->C_line_delay);
	gtk_adjustment_value_changed(GTK_ADJUSTMENT(nd->C_line_adj));

	
	GTK_ADJUSTMENT(nd->C_update_adj)->lower = (gfloat)sd->update_min;
	GTK_ADJUSTMENT(nd->C_update_adj)->upper = (gfloat)sd->update_max;
	gtk_adjustment_changed(GTK_ADJUSTMENT(nd->C_update_adj));
	gtk_adjustment_set_value(GTK_ADJUSTMENT(nd->C_update_adj), (gfloat)sd->C_update_val);
	gtk_adjustment_value_changed(GTK_ADJUSTMENT(nd->C_update_adj));

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(nd->C_date_button), sd->C_show_date);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(nd->C_topic_button), sd->C_show_topic);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(nd->C_image_button), sd->C_show_image);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(nd->C_body_button), sd->C_show_body);
}

static gchar *enabled_as_text(gint val)
{
	if (val) return "X";

	return " ";
}

static void populate_script_clist(GtkCList *clist, NewsData *nd)
{
	GList *work;
	gint row;
	gchar *buf[10];

	buf [8] = NULL;

	work = nd->list;
	while(work)
		{
		ScriptData *sd = work->data;

		buf[0] = enabled_as_text(sd->C_enabled);
		buf[1] = sd->name;

		buf[2] = g_strdup_printf("%d", sd->C_line_delay);
		buf[3] = g_strdup_printf("%d", sd->C_update_val);

		buf[4] = enabled_as_text(sd->C_show_date);
		buf[5] = enabled_as_text(sd->C_show_topic);
		buf[6] = enabled_as_text(sd->C_show_image);
		buf[7] = enabled_as_text(sd->C_show_body);

		row = gtk_clist_append(clist, buf);
		gtk_clist_set_row_data(clist, row, sd);

		g_free(buf[2]);
		g_free(buf[3]);

		work = work->next;
		}
}

/*
 *-------------------------------------------------------------------
 * setup
 *-------------------------------------------------------------------
 */

static void append_script_list(NewsData *nd, gchar *dir_path, GList **old)
{
	DIR *dp;
	struct dirent *dir;
	struct stat ent_sbuf;
	GList *work;
	gchar *data_path;

	printf("%s\n", dir_path);


	if((dp = opendir(dir_path))==NULL)
		{
		return;
		}

	data_path = gnome_util_home_file("tickastat");

	while ((dir = readdir(dp)) != NULL)
		{
		if (dir->d_ino > 0)
			{
			gchar *name;
			gchar *path;

			name = dir->d_name; 
			path = g_concat_dir_and_file(dir_path, name);
			printf("%s\n", path);

			if (stat(path,&ent_sbuf) >= 0 && !S_ISDIR(ent_sbuf.st_mode) &&
			    strlen(name) > 5 && strcasecmp(name + strlen(name) - 5, ".tick") == 0)
				{
				gint found = FALSE;
				work = *old;
				while (work && !found)
					{
					ScriptData *sd = work->data;
					work = work->next;
					if (strcmp(path, sd->path) == 0)
						{
						nd->list = g_list_append(nd->list, sd);
						*old = g_list_remove(*old, sd);
						found = TRUE;
						}
					}
				if (!found)
					{
					ScriptData *sd = script_data_new(nd, path, data_path);
					if (sd)
						{
						nd->list = g_list_append(nd->list, sd);
						}
					}
				}
			g_free(path);
			}
		}
	closedir(dp);

	g_free(data_path);
}

static void update_script_list(NewsData *nd)
{
	GList *old = nd->list;
	GList *work;
	gchar *buf;

	nd->list = NULL;

	/* the system scripts */
	buf = gnome_unconditional_datadir_file("tickastat/news");
	append_script_list(nd, buf, &old);
	g_free(buf);

	/* the user scripts */
	buf = gnome_util_home_file("tickastat/news");
	append_script_list(nd, buf, &old);
	g_free(buf);

	/* clean up old ones that are gone */
	work = old;
	while(work)
		{
		ScriptData *sd = work->data;
		work = work->next;
		old = g_list_remove(old, sd);
		script_data_free(sd);
		}
}

static NewsData *init(ModuleData *md, AppData *ad)
{
	NewsData *nd = g_new0(NewsData, 1);

	nd->enabled = FALSE;
	nd->md = md;
	nd->ad = ad;

	nd->list = NULL;
	nd->C_clist = NULL;
	nd->C_label = NULL;

	nd->loop_cb_id = -1;

	update_script_list(nd);

	return nd;
}

/*
 *===================================================================
 * public interface follows
 *===================================================================
 */

/*
 *-------------------------------------------------------------------
 * preference window
 *-------------------------------------------------------------------
 */

static void mod_news_config_apply(gpointer data, AppData *ad)
{
	NewsData *nd = data;
	GList *work;
	gint changed = FALSE;

	if (nd->enabled != nd->C_enabled) changed = TRUE;

	nd->enabled = nd->C_enabled;

	work = nd->list;
	while(work)
		{
		ScriptData *sd = work->data;
		gint s_changed = FALSE;
		gint l_changed = FALSE;

		if (sd->enabled != sd->C_enabled) s_changed = TRUE;

		sd->enabled = sd->C_enabled;
		sd->update_val = sd->C_update_val;

		if (sd->show_date != sd->C_show_date ||
		    sd->show_topic != sd->C_show_topic ||
		    sd->show_image != sd->C_show_image ||
		    sd->show_body != sd->C_show_body)
			{
			l_changed = TRUE;
			}

		sd->show_date = sd->C_show_date;
		sd->show_topic = sd->C_show_topic;
		sd->show_image = sd->C_show_image;
		sd->show_body = sd->C_show_body;

		if (l_changed)
			{
			script_lines_reset(sd);
			}

		if (sd->line_delay != sd->C_line_delay)
			{
			sd->line_delay = sd->C_line_delay;
			script_line_delay_adjust(sd);
			}

		if (s_changed && !changed)
			{
			script_state_check(sd, FALSE);
			}
		work = work->next;
		}

	if (changed)
		{
		if (nd->enabled)
			{
			news_start(nd);
			}
		else
			{
			news_stop(nd);
			}
		}
        return;
        ad = NULL;
}

static void mod_news_config_close(gpointer data, AppData *ad)
{
/*	NewsData *nd = data;*/
        return;
        ad = NULL;
	data = NULL;
}

/* remember that the returned widget is gtk_widget_destroy()ed
 * the widget is also gtk_widget_shown(), do not do it here.
 */
static GtkWidget *mod_news_config_show(gpointer data, AppData *ad)
{
	NewsData *nd = data;
	GtkWidget *frame;
	GtkWidget *frame1;
	GtkWidget *vbox;
	GtkWidget *vbox1;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *button;
	GtkWidget *scrolled;
	gchar *titles[9];

	titles[0] = _("Enabled");
	titles[1] = _("Name");
	titles[2] = _("Line delay");
	titles[3] = _("Update interval");
	titles[4] = _("D");
	titles[5] = _("T");
	titles[6] = _("I");
	titles[7] = _("B");
	titles[8] = NULL;

	nd->C_row = -1;

	frame = gtk_frame_new(_("News and Information Module"));

	vbox = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_container_set_border_width(GTK_CONTAINER (vbox), GNOME_PAD_SMALL);
	gtk_container_add(GTK_CONTAINER(frame), vbox);
	gtk_widget_show(vbox);

	button = gtk_check_button_new_with_label (_("Enable this module"));
	gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), nd->C_enabled);
	gtk_signal_connect (GTK_OBJECT(button),"clicked",(GtkSignalFunc) enabled_checkbox_cb, nd);
	gtk_widget_show(button);

	scrolled = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
				GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
	gtk_box_pack_start(GTK_BOX(vbox), scrolled, TRUE, TRUE, 0);
	gtk_widget_show(scrolled);

	nd->C_clist = gtk_clist_new_with_titles(8, titles);
	gtk_container_add (GTK_CONTAINER (scrolled), nd->C_clist);
	populate_script_clist(GTK_CLIST(nd->C_clist), nd);
	gtk_clist_set_column_auto_resize (GTK_CLIST(nd->C_clist), 1, TRUE);
	gtk_clist_set_column_auto_resize (GTK_CLIST(nd->C_clist), 4, TRUE);
	gtk_clist_set_column_auto_resize (GTK_CLIST(nd->C_clist), 5, TRUE);
	gtk_clist_set_column_auto_resize (GTK_CLIST(nd->C_clist), 6, TRUE);
	gtk_clist_set_column_auto_resize (GTK_CLIST(nd->C_clist), 7, TRUE);
	gtk_signal_connect (GTK_OBJECT(nd->C_clist), "select_row",
			    (GtkSignalFunc) script_row_select_cb, nd);
	gtk_widget_show(nd->C_clist);

	frame1 = gtk_frame_new(NULL);
	gtk_box_pack_start(GTK_BOX(vbox), frame1, FALSE, FALSE, 0);
	gtk_widget_show(frame1);

	vbox1 = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(frame1), vbox1);
	gtk_widget_show(vbox1);

	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(vbox1), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

	nd->C_script_button = gtk_check_button_new_with_label (_("enabled"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(nd->C_script_button), FALSE);
	gtk_signal_connect (GTK_OBJECT(nd->C_script_button),"clicked",(GtkSignalFunc) enabled_script_cb, nd);
	gtk_box_pack_start(GTK_BOX(hbox), nd->C_script_button, FALSE, FALSE, 0);
	gtk_widget_show(nd->C_script_button);

	nd->C_label = gtk_entry_new();
	gtk_entry_set_editable(GTK_ENTRY(nd->C_label), FALSE);
	gtk_box_pack_start(GTK_BOX(hbox), nd->C_label, TRUE, TRUE, 0);
	gtk_widget_show(nd->C_label);
	/* This is the number of lines shown in the display. */
	label = gtk_label_new(_("Line (s):"));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_widget_show(label);

	nd->C_line_adj = gtk_adjustment_new(2.0, NEWS_LINE_DELAY_MIN, NEWS_LINE_DELAY_MAX, 1, 1, 1 );
	nd->C_line_spin = gtk_spin_button_new(GTK_ADJUSTMENT(nd->C_line_adj), 1, 0 );
	gtk_box_pack_start(GTK_BOX(hbox), nd->C_line_spin, FALSE, FALSE, 0);
	gtk_signal_connect(GTK_OBJECT(nd->C_line_adj),"value_changed",
			GTK_SIGNAL_FUNC(line_delay_cb), nd);
	gtk_signal_connect( GTK_OBJECT(nd->C_line_spin),"changed",
			GTK_SIGNAL_FUNC(line_delay_cb), nd);
	gtk_spin_button_set_update_policy(GTK_SPIN_BUTTON(nd->C_line_spin), GTK_UPDATE_ALWAYS);
	gtk_widget_show(nd->C_line_spin);
	/* The update interval in minutes. */
	label = gtk_label_new(_("Update (m):"));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_widget_show(label);

	nd->C_update_adj = gtk_adjustment_new(20.0, 1.0, 120.0, 1, 1, 1 );
	nd->C_update_spin = gtk_spin_button_new(GTK_ADJUSTMENT(nd->C_update_adj), 1, 0 );
	gtk_box_pack_start(GTK_BOX(hbox), nd->C_update_spin, FALSE, FALSE, 0);
	gtk_signal_connect(GTK_OBJECT(nd->C_update_adj),"value_changed",
			GTK_SIGNAL_FUNC(update_delay_cb), nd);
	gtk_signal_connect( GTK_OBJECT(nd->C_update_spin),"changed",
			GTK_SIGNAL_FUNC(update_delay_cb), nd);
	gtk_spin_button_set_update_policy(GTK_SPIN_BUTTON(nd->C_update_spin), GTK_UPDATE_ALWAYS);
	gtk_widget_show(nd->C_update_spin);

	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(vbox1), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

	nd->C_body_button = gtk_check_button_new_with_label (_("body"));
	gtk_signal_connect (GTK_OBJECT(nd->C_body_button),"clicked",(GtkSignalFunc) show_body_cb, nd);
	gtk_box_pack_end(GTK_BOX(hbox), nd->C_body_button, FALSE, FALSE, 0);
	gtk_widget_show(nd->C_body_button);

	nd->C_image_button = gtk_check_button_new_with_label (_("image"));
	gtk_signal_connect (GTK_OBJECT(nd->C_image_button),"clicked",(GtkSignalFunc) show_image_cb, nd);
	gtk_box_pack_end(GTK_BOX(hbox), nd->C_image_button, FALSE, FALSE, 0);
	gtk_widget_show(nd->C_image_button);

	nd->C_topic_button = gtk_check_button_new_with_label (_("topic"));
	gtk_signal_connect (GTK_OBJECT(nd->C_topic_button),"clicked",(GtkSignalFunc) show_topic_cb, nd);
	gtk_box_pack_end(GTK_BOX(hbox), nd->C_topic_button, FALSE, FALSE, 0);
	gtk_widget_show(nd->C_topic_button);

	nd->C_date_button = gtk_check_button_new_with_label (_("date"));
	gtk_signal_connect (GTK_OBJECT(nd->C_date_button),"clicked",(GtkSignalFunc) show_date_cb, nd);
	gtk_box_pack_end(GTK_BOX(hbox), nd->C_date_button, FALSE, FALSE, 0);
	gtk_widget_show(nd->C_date_button);

	label = gtk_label_new(_("Show:"));
	gtk_box_pack_end(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_widget_show(label);

	script_edit_set_sensitive(nd, NULL);

	label = gtk_label_new(nd->md->description);
	gtk_box_pack_end(GTK_BOX(vbox), label, FALSE, FALSE, 0);
	gtk_widget_show(label);

	return frame;
        ad = NULL;
}

static void mod_news_config_hide(gpointer data, AppData *ad)
{
	NewsData *nd = data;

	nd->C_clist = NULL;
	nd->C_label = NULL;
        return;
        ad = NULL;
}

static void mod_news_config_init(gpointer data, AppData *ad)
{
	NewsData *nd = data;
	GList *work;

	nd->C_enabled = nd->enabled;

	work = nd->list;
	while(work)
		{
		ScriptData *sd = work->data;
		sd->C_enabled = sd->enabled;
		sd->C_update_val = sd->update_val;
		sd->C_line_delay = sd->line_delay;
		sd->C_show_date = sd->show_date;
		sd->C_show_topic = sd->show_topic;
		sd->C_show_image = sd->show_image;
		sd->C_show_body = sd->show_body;
		work = work->next;
		}
        return;
        ad = NULL;
}

/*
 *-------------------------------------------------------------------
 * save and load options
 *-------------------------------------------------------------------
 */

static gint mod_news_load_options_util(ScriptData *sd, gchar *key, gint val)
{
	gchar *prefix = g_strdup_printf("module_news_%s%s=%d", sd->config_id, key, val);
	gint n = gnome_config_get_int(prefix);
	g_free(prefix);
	return n;
}

static void mod_news_load_options(gpointer data, AppData *ad)
{
	NewsData *nd = data;
	GList *work;

	nd->enabled = gnome_config_get_int("module_news/enabled=0");

	work = nd->list;
	while(work)
		{
		ScriptData *sd = work->data;
		gint n;

		sd->enabled = mod_news_load_options_util(sd, "/enabled", 0);

		n = mod_news_load_options_util(sd, "/update", 0);
		if (n >= sd->update_min && n <= sd->update_max)
			{
			sd->update_val = n;
			}
		else
			{
			sd->update_val = sd->update_default;
			}

		n = mod_news_load_options_util(sd, "/line_delay", 0);
		if (n >= 1 && n <= 120)
			{
			sd->line_delay = n;
			}

		sd->show_date = mod_news_load_options_util(sd, "/show_date", sd->show_date);
		sd->show_topic = mod_news_load_options_util(sd, "/show_topic", sd->show_topic);
		sd->show_image = mod_news_load_options_util(sd, "/show_image", sd->show_image);
		sd->show_body = mod_news_load_options_util(sd, "/show_body", sd->show_body);

		printf("loading state of %s: %d - %d (%d)\n", sd->name, sd->update_min, sd->update_max, sd->update_val);

		work = work->next;
		}
        return;
        ad = NULL;
}

static void mod_news_save_options_util(ScriptData *sd, gchar *key, gint val)
{
	gchar *prefix = g_strconcat("module_news_", sd->config_id, key, NULL);
	gnome_config_set_int(prefix, val);
	g_free(prefix);
}

static void mod_news_save_options(gpointer data, AppData *ad)
{
	NewsData *nd = data;
	GList *work;

	gnome_config_set_int("module_news/enabled", nd->enabled);

	work = nd->list;
	while(work)
		{
		ScriptData *sd = work->data;

		mod_news_save_options_util(sd, "/enabled", sd->enabled);
		mod_news_save_options_util(sd, "/update", sd->update_val);
		mod_news_save_options_util(sd, "/line_delay", sd->line_delay);

		mod_news_save_options_util(sd, "/show_date", sd->show_date);
		mod_news_save_options_util(sd, "/show_topic", sd->show_topic);
		mod_news_save_options_util(sd, "/show_image", sd->show_image);
		mod_news_save_options_util(sd, "/show_body", sd->show_body);

		work = work->next;
		}
        return;
        ad = NULL;
}

/*
 *-------------------------------------------------------------------
 * initialization and setup, exit
 *-------------------------------------------------------------------
 */

static void mod_news_start(gpointer data, AppData *ad)
{
	NewsData *nd = data;

	if (nd->enabled)
		{
		news_start(nd);
		}
        return;
        ad = NULL;
}

static void mod_news_destroy(gpointer data, AppData *ad)
{
	NewsData *nd = data;

	news_stop(nd);

	while(nd->list)
		{
		ScriptData *sd = nd->list->data;
		nd->list = g_list_remove(nd->list, sd);
		script_data_free(sd);
		}

	g_free(nd);
        return;
        ad = NULL;
}

ModuleData *mod_news_init(AppData *ad)
{
	ModuleData *md;

	md = g_new0(ModuleData, 1);

	md->ad = ad;
	md->title = _("News and information ticker");
	md->description = _("This module can display news and other information.");

	md->start_func = mod_news_start;

	md->config_init_func =  mod_news_config_init;
	md->config_show_func =  mod_news_config_show;
	md->config_hide_func =  mod_news_config_hide;
	md->config_apply_func =  mod_news_config_apply;
	md->config_close_func =  mod_news_config_close;
	md->config_load_func =  mod_news_load_options;
	md->config_save_func =  mod_news_save_options;
	md->destroy_func =  mod_news_destroy;

	md->internal_data = (gpointer)init(md, ad);

	return md;
}


