/* tick-a-stat, a GNOME panel applet
 * (C) 1999 John Ellis
 *
 * Author: John Ellis
 *
 */

#include "tickastat.h"
#include "modules.h"

#include <fcntl.h>

#define TAIL_LINE_PRIORITY 8
#define TAIL_LINE_DELAY 10

/*
 *-------------------------------------------------------------------
 * The file tail module for Tick-A-Stat
 * 
 * Monitors the output of a file like tail -f
 * 
 *-------------------------------------------------------------------
 */

typedef struct _TailData TailData;
struct _TailData
{
	ModuleData *md;
	AppData *ad;

	gint enabled;
	gchar *path;
	gint show_popup;

	gint C_enabled;
	gchar *C_path;
	gint C_show_popup;

	GtkWidget *path_entry;

	GtkWidget *pop_dialog;
	GtkWidget *log_dialog;

	FILE *fp_id;
	gint fd_id;
	gint cb_id;
};

/*
 *-------------------------------------------------------------------
 * dialog(s)
 *-------------------------------------------------------------------
 */

static void log_dialog_close(GtkWidget *widget, gpointer data)
{
	TailData *td = data;

	if (GTK_WIDGET(widget) == td->log_dialog)
		{
		td->log_dialog = NULL;
		}
}

static void show_log_file(TailData *td)
{
	GtkWidget *gless;
	GtkWidget *label;

	if (td->log_dialog)
		{
		gdk_window_raise(td->log_dialog->window);
		return;
		}


	td->log_dialog = gnome_dialog_new(_("File tail view file (Tick-a-Stat)"),
					 GNOME_STOCK_BUTTON_CLOSE, NULL);
	gnome_dialog_set_close(GNOME_DIALOG(td->log_dialog), TRUE);
	gtk_widget_set_usize(td->log_dialog, 500, 400);
	gtk_window_set_policy(GTK_WINDOW(td->log_dialog), TRUE, TRUE, FALSE);
	gtk_signal_connect(GTK_OBJECT(td->log_dialog), "destroy", GTK_SIGNAL_FUNC(log_dialog_close), td);

	if (td->path)
		label = gtk_label_new(td->path);
	else
		label = gtk_label_new(_("no file specified"));
	gtk_box_pack_start(GTK_BOX(GNOME_DIALOG(td->log_dialog)->vbox), label, FALSE, FALSE, 0);
	gtk_widget_show(label);

	gless = gnome_less_new();
	gtk_box_pack_start(GTK_BOX(GNOME_DIALOG(td->log_dialog)->vbox), gless, TRUE, TRUE, 0);
	gtk_widget_show(gless);

	gtk_object_set_user_data(GTK_OBJECT(td->log_dialog), gless);

	if (!g_file_exists(td->path) ||
	    !gnome_less_show_file(GNOME_LESS(gless), td->path))
                {
		gnome_less_show_string(GNOME_LESS(gless), "Unable to open file.");
                }
	gtk_widget_show(td->log_dialog);
}

static void line_click_cb(ModuleData *md, gpointer data, InfoData *id, AppData *ad)
{
	TailData *td = data;
	show_log_file(td);
        return;
	ad = NULL;
	id = NULL;
	md = NULL;
}

static void popup_dialog_close(GtkWidget *widget, gpointer data)
{
	TailData *td = data;

	if (GTK_WIDGET(widget) == td->pop_dialog)
		{
		td->pop_dialog = NULL;
		}
}

static void create_popup_dialog(TailData *td)
{
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *text_box;
	GtkWidget *scrollbar;
	GtkWidget *label;
	gchar *buf;

	td->pop_dialog = gnome_dialog_new(_("File tail information (Tick-a-Stat)"),
				GNOME_STOCK_BUTTON_CLOSE, NULL);
	gtk_window_set_policy(GTK_WINDOW(td->pop_dialog), TRUE, TRUE, FALSE);
	gtk_widget_set_usize(td->pop_dialog, 500, 400);
	gnome_dialog_set_close(GNOME_DIALOG(td->pop_dialog), TRUE);
	gtk_signal_connect(GTK_OBJECT(td->pop_dialog), "destroy", GTK_SIGNAL_FUNC(popup_dialog_close), td);

	vbox = GNOME_DIALOG(td->pop_dialog)->vbox;

	buf = g_strconcat(_("File tail for "), td->path + g_filename_index(td->path), NULL);
	label = gtk_label_new(buf);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
	gtk_widget_show(label);
	g_free(buf);

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
	gtk_widget_show(hbox);

	text_box = gtk_text_new(NULL, NULL);
	gtk_text_set_editable(GTK_TEXT(text_box), FALSE);
	gtk_box_pack_start(GTK_BOX(hbox), text_box, TRUE, TRUE, 0);
	gtk_widget_show(text_box);

	scrollbar = gtk_vscrollbar_new(GTK_TEXT(text_box)->vadj);
	gtk_box_pack_start(GTK_BOX(hbox), scrollbar, FALSE, FALSE, 0);
	gtk_widget_show(scrollbar);

	gtk_object_set_user_data(GTK_OBJECT(td->pop_dialog), text_box);

	gtk_widget_show(td->pop_dialog);
}

static void add_line_to_popup(TailData *td, gchar *text)
{
	GtkWidget *text_box;

	/* keep the log window updated, if open */
	if (td->log_dialog)
		{
		GtkWidget *gless;
		gless = gtk_object_get_user_data(GTK_OBJECT(td->log_dialog));
		gtk_text_freeze(GTK_TEXT(GNOME_LESS(gless)->text));
		gtk_text_insert(GTK_TEXT(GNOME_LESS(gless)->text), NULL, NULL, NULL,
					 text, strlen(text));
		gtk_text_insert(GTK_TEXT(GNOME_LESS(gless)->text), NULL, NULL, NULL,
					 "\n", strlen("\n"));
		gtk_text_thaw(GTK_TEXT(GNOME_LESS(gless)->text));
		}

	if (!td->show_popup) return;

	if (!td->pop_dialog)
		{
		create_popup_dialog(td);
		}

	text_box = gtk_object_get_user_data(GTK_OBJECT(td->pop_dialog));

	gtk_text_freeze(GTK_TEXT(text_box));
	gtk_text_insert(GTK_TEXT(text_box), NULL, NULL, NULL,
				 text, strlen(text));
	gtk_text_insert(GTK_TEXT(text_box), NULL, NULL, NULL,
				 "\n", strlen("\n"));
	gtk_text_thaw(GTK_TEXT(text_box));
}

/*
 *-------------------------------------------------------------------
 * file monitoring
 *-------------------------------------------------------------------
 */

static gint tail_read_data(gpointer data)
{
	TailData *td = data;
	InfoData *id;
	gchar i_buf[2048];

	while (fgets(i_buf, 2048, td->fp_id))
                {
		gchar *p = i_buf;
		while(*p != '\0')
			{
			if (*p == '\n' || *p == '\r') *p = '\0';
			p++;
			}
		id = add_info_line(td->ad, i_buf, NULL, 0, FALSE, 1, TAIL_LINE_DELAY, TAIL_LINE_PRIORITY);
		set_info_signals(id, td->md, line_click_cb, NULL, NULL, NULL, td);

		add_line_to_popup(td, i_buf);
		}

	return TRUE;
}

static void tail_stop(TailData *td)
{
	if (td->fd_id == -1) return;

	if (td->cb_id != -1)
		{
		gtk_timeout_remove(td->cb_id);
		td->cb_id = -1;
		}

	if (td->fd_id != -1)
		{
		close(td->fd_id);
		td->fd_id = -1;
		td->fp_id = NULL;
		}
}

static gint tail_start(TailData *td)
{
	tail_stop(td);

	if (!td->path) return FALSE;

	td->fd_id = open(td->path, O_RDONLY | O_NONBLOCK);

	if (td->fd_id < 0)
		{
		printf("Failed to open file: %s\n", td->path);
		return FALSE;
		}

	td->fp_id = fdopen (td->fd_id, "r");

	if (fseek (td->fp_id, 0, SEEK_END) != 0)
		{
		printf("Failed to seek to end of file: %s\n", td->path);
		close(td->fd_id);
		td->fd_id = -1;
		td->fp_id = NULL;
		return FALSE;
		}

	td->cb_id = gtk_timeout_add(1000, tail_read_data, td);

	return TRUE;
}

/*
 *-------------------------------------------------------------------
 * callbacks
 *-------------------------------------------------------------------
 */

static void enabled_checkbox_cb(GtkWidget *widget, gpointer data)
{
	TailData *td = data;
	td->C_enabled = GTK_TOGGLE_BUTTON (widget)->active;
	property_changed(td->md, td->ad);
}

static void popup_checkbox_cb(GtkWidget *widget, gpointer data)
{
	TailData *td = data;
	td->C_show_popup = GTK_TOGGLE_BUTTON (widget)->active;
	property_changed(td->md, td->ad);
}

/*
 *-------------------------------------------------------------------
 * 
 *-------------------------------------------------------------------
 */

static TailData *init(ModuleData *md, AppData *ad)
{
	TailData *td = g_new0(TailData, 1);

	td->enabled = FALSE;
	td->md = md;
	td->ad = ad;

	td->path = g_strdup("/var/log/log.txt");
	td->path_entry = NULL;

	td->fd_id = -1;
	td->cb_id = -1;

	td->show_popup = FALSE;
	td->pop_dialog = NULL;
	td->log_dialog = NULL;

	return td;
}

static void misc_entry_cb(GtkWidget *widget, gpointer data)
{
	TailData *td = data;
	property_changed(td->md, td->ad);
        return;
        widget = NULL;
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

static void mod_tail_config_apply(gpointer data, AppData *ad)
{
	TailData *td = data;
	gint changed = FALSE;

	if (td->path_entry)
		{
		gchar *buf = gtk_entry_get_text(GTK_ENTRY(td->path_entry));
		if (buf && strlen(buf) > 0)
			{
			g_free(td->C_path);
			td->C_path = g_strdup(buf);
			}
		}

	if ((td->path && !td->C_path) || (!td->path && td->C_path) ||
	    (strcmp(td->path, td->C_path) != 0))
		{
		changed = TRUE;
		}

	g_free(td->path);
	td->path = g_strdup(td->C_path);
	if (td->enabled != td->C_enabled || changed)
		{
		/* so that pop ups do not mix for change */
		td->pop_dialog = NULL;
		td->log_dialog = NULL;

		td->enabled = td->C_enabled;
		if (td->enabled)
			{
			tail_start(td);
			}
		else
			{
			tail_stop(td);
			}
		}

	td->show_popup = td->C_show_popup;
        return;
        ad = NULL;
}

static void mod_tail_config_close(gpointer data, AppData *ad)
{
	TailData *td = data;

	g_free(td->C_path);
	td->C_path = NULL;
        return;
        ad = NULL;
}

/* remember that the returned widget is gtk_widget_destroy()ed
 * the widget is also gtk_widget_shown(), do not do it here.
 */
static GtkWidget *mod_tail_config_show(gpointer data, AppData *ad)
{
	TailData *td = data;
	GtkWidget *frame;
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *button;

	frame = gtk_frame_new(_("File Tail Module"));

	vbox = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_container_set_border_width (GTK_CONTAINER(vbox), GNOME_PAD_SMALL);
	gtk_container_add(GTK_CONTAINER(frame), vbox);
	gtk_widget_show(vbox);

	button = gtk_check_button_new_with_label (_("Enable this module"));
	gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), td->C_enabled);
	gtk_signal_connect (GTK_OBJECT(button),"clicked",(GtkSignalFunc) enabled_checkbox_cb, td);
	gtk_widget_show(button);

	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

	label = gtk_label_new(_("Path to tail:"));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_widget_show(label);

	label = gnome_file_entry_new ("tickastat tail file",
				      _("Choose a file to tail"));
	td->path_entry = gnome_file_entry_gtk_entry(GNOME_FILE_ENTRY(label));
	if (td->C_path)
		gtk_entry_set_text(GTK_ENTRY(td->path_entry), td->C_path);
	gtk_signal_connect(GTK_OBJECT(td->path_entry), "changed",
			   GTK_SIGNAL_FUNC(misc_entry_cb), td);
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
	gtk_widget_show(label);

	button = gtk_check_button_new_with_label (_("Show pop up dialog for new lines."));
	gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), td->C_show_popup);
	gtk_signal_connect (GTK_OBJECT(button),"clicked",(GtkSignalFunc) popup_checkbox_cb, td);
	gtk_widget_show(button);

	label = gtk_label_new(td->md->description);
	gtk_box_pack_end(GTK_BOX(vbox), label, FALSE, FALSE, 0);
	gtk_widget_show(label);
	
	return frame;
        ad = NULL;
}

static void mod_tail_config_hide(gpointer data, AppData *ad)
{
	TailData *td = data;
	gchar *buf;

	buf = gtk_entry_get_text(GTK_ENTRY(td->path_entry));
	if (buf && strlen(buf) > 0)
		{
		g_free(td->C_path);
		td->C_path = g_strdup(buf);
		}
	td->path_entry = NULL;
        return;
        ad = NULL;
}

static void mod_tail_config_init(gpointer data, AppData *ad)
{
	TailData *td = data;

	td->C_enabled = td->enabled;
	td->C_path = g_strdup(td->path);

	td->path_entry = NULL;

	td->C_show_popup = td->show_popup;
        return;
        ad = NULL;
}

/*
 *-------------------------------------------------------------------
 * save and load options
 *-------------------------------------------------------------------
 */

static void mod_tail_load_options(gpointer data, AppData *ad)
{
	TailData *td = data;
	gchar *buf;

	td->enabled = gnome_config_get_int("module_tail/enabled=0");

	buf = gnome_config_get_string("module_tail/path=");
	if (buf && strlen(buf) > 0)
		{
		g_free(td->path);
		td->path = g_strdup(buf);
		}

	td->show_popup = gnome_config_get_int("module_tail/popup_dialog=0");
        return;
        ad = NULL;
}

static void mod_tail_save_options(gpointer data, AppData *ad)
{
	TailData *td = data;

	gnome_config_set_int("module_tail/enabled", td->enabled);
	gnome_config_set_string("module_tail/path", td->path);

	gnome_config_set_int("module_tail/popup_dialog", td->show_popup);
        return;
        ad = NULL;
}

/*
 *-------------------------------------------------------------------
 * initialization and setup, exit
 *-------------------------------------------------------------------
 */

static void mod_tail_start(gpointer data, AppData *ad)
{
	TailData *td = data;

	if (td->enabled)
		{
		tail_start(td);
		}
        return;
        ad = NULL;
}

static void mod_tail_destroy(gpointer data, AppData *ad)
{
	TailData *td = data;

	tail_stop(td);
	g_free(td->path);

	g_free(td);
        return;
        ad = NULL;
}

ModuleData *mod_tail_init(AppData *ad)
{
	ModuleData *md;

	md = g_new0(ModuleData, 1);

	md->ad = ad;
	md->title = _("File tailer");
	md->description = _("This module monitors a file for appended text,\n and prints it. Useful for log files.");

	md->start_func = mod_tail_start;

	md->config_init_func =  mod_tail_config_init;
	md->config_show_func =  mod_tail_config_show;
	md->config_hide_func =  mod_tail_config_hide;
	md->config_apply_func =  mod_tail_config_apply;
	md->config_close_func =  mod_tail_config_close;
	md->config_load_func =  mod_tail_load_options;
	md->config_save_func =  mod_tail_save_options;
	md->destroy_func =  mod_tail_destroy;

	md->internal_data = (gpointer)init(md, ad);

	return md;
}


