/* tick-a-stat, a GNOME panel applet
 * (C) 1999 John Ellis
 *
 * Author: John Ellis
 *
 */

#include "tickastat.h"
#include "modules.h"

/*
 *-------------------------------------------------------------------
 * The core dump module for Tick-A-Stat
 * 
 * Monitors a given path for core dumps, when one is found, it
 * time-stamps the core (rename), then runs gdb to generate a
 * backtrace.  The backtrace is logged and an alert is displayed to
 * the user, with an option to pop up a backtrace dialog.
 * 
 *-------------------------------------------------------------------
 */

typedef struct _CoredumpData CoredumpData;
struct _CoredumpData
{
	ModuleData *md;
	AppData *ad;

	gint enabled;
	gint show_popup;
	gchar *core_path;
	gchar *core_file;

	gint core_check_timeout_id;

	gint C_enabled;
	gint C_show_popup;
	gchar *C_core_path;

	GtkWidget *core_path_entry;
};

typedef struct _LineData LineData;
struct _LineData
{
	gchar *binary;
	GList *text;
	gchar *filename;
};

static LineData *new_list_info(gchar *binary, gchar *filename, GList *text_list)
{
	LineData *ld = g_new0(LineData, 1);

	ld->text = text_list;
	ld->binary = g_strdup(binary);
	ld->filename = g_strdup(filename);

	return ld;
}

static void free_list_info(LineData *ld)
{
	GList *work = ld->text;
	while(work)
		{
		g_free(work->data);
		work = work->next;
		}
	if (ld->text) g_list_free(ld->text);
	g_free(ld->binary);
	g_free(ld->filename);
	g_free(ld);
}

/*
 *-------------------------------------------------------------------
 * dialogs
 *-------------------------------------------------------------------
 */

static void show_coredump_dialog(LineData *ld, gchar *text)
{
	GtkWidget *dialog;
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *text_box;
	GtkWidget *scrollbar;
	GList *work;

	dialog = gnome_dialog_new(_("Core dump information (Tick-a-Stat)"),
				  GNOME_STOCK_BUTTON_CLOSE, NULL);
	gnome_dialog_set_close(GNOME_DIALOG(dialog), TRUE);

	if (text)
		gtk_widget_set_usize(dialog, 500, 200);
	else
		gtk_widget_set_usize(dialog, 500, 400);

	gtk_window_set_policy(GTK_WINDOW(dialog), TRUE, TRUE, FALSE);

	vbox = GNOME_DIALOG(dialog)->vbox;

	if (text)
		{
		GtkWidget *icon = NULL;
		GtkWidget *label;
		gchar *icon_path;

		hbox = gtk_hbox_new(FALSE, GNOME_PAD);
		gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
		gtk_widget_show(hbox);

		icon_path = gnome_unconditional_pixmap_file ("mc/i-core.png");
		icon = gnome_pixmap_new_from_file(icon_path);
		g_free(icon_path);

		if (icon)
			{
			gtk_box_pack_start(GTK_BOX(hbox), icon, FALSE, FALSE, 0);
			gtk_widget_show(icon);
			}

		label = gtk_label_new(text);
		gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
		gtk_widget_show(label);
		}

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

	work = ld->text;
	while(work)
		{
		gchar *text = work->data;
		gtk_text_insert(GTK_TEXT(text_box), NULL, NULL, NULL,
				text, strlen(text));
		work = work->next;
		}

	gtk_widget_show(dialog);
}

/*
 *-------------------------------------------------------------------
 * callbacks
 *-------------------------------------------------------------------
 */

static void clicked_cb(ModuleData *md, gpointer data, InfoData *id, AppData *ad)
{
	LineData *ld = data;

	show_coredump_dialog(ld, NULL);
	return;
	md = NULL;
	id = NULL;
	ad = NULL;
}

static void free_line_cb(ModuleData *md, gpointer data)
{
	LineData *ld = data;
	free_list_info(ld);
        return;
        md = NULL;
}

/*
 *-------------------------------------------------------------------
 * core processing
 *-------------------------------------------------------------------
 */

static void generate_log(gchar *core_file, CoredumpData *cd)
{
	gchar *gdb_cmd;
	gchar buf[1024];
	FILE *f;
	gchar *binary = NULL;
	gchar *cmd_file;
	gchar *cmd_buf;
	GList *bt_list = NULL;
	GList *work;
	gchar *text;
	gchar *icon;
	LineData *ld;
	InfoData *id;
	GtkWidget *dialog = NULL;

	gdb_cmd = g_strconcat("gdb --batch --core=", core_file, NULL);

	f = popen(gdb_cmd, "r");
	g_free(gdb_cmd);

	if (!f)
		{
		printf("Unable to process core with gdb\n");
		return;
		}

	while (fgets(buf, 1024, f) != NULL)
                {
		if (!binary && !strncmp(buf, "Core was generated", 16))
			{
			gchar *s;
			gchar *ptr = buf;
			while (*ptr != '`' && *ptr !='\0') ptr++;
			if (*ptr == '`')
				{
				ptr++;
				s = ptr;
				while (*ptr != '\'' && *ptr !=' ' && *ptr !='\0') ptr++;
				*ptr = '\0';
				binary = g_strdup(s);
				}
			}
                }

	fclose(f);
	if (!binary)
		{
		printf("Unable to determine binary with gdb\n");
		return;
		}

	if (cd->show_popup)
		{
		GtkWidget *label;
		dialog = gnome_dialog_new(_("Core dump module (Tick-A-Stat)"), NULL);
		label = gtk_label_new(_("The core dump module is\nprocessing a core file..."));
		gtk_box_pack_start(GTK_BOX(GNOME_DIALOG(dialog)->vbox), label, FALSE, FALSE, 0);
		gtk_widget_show(label);
		gtk_widget_show(dialog);
		while(gtk_events_pending()) gtk_main_iteration(); /* fake responsiveness */
		}

	cmd_buf = gnome_util_home_file("tickastat/gdb_cmd_file");
	cmd_file = g_strconcat(cmd_buf, core_file + g_filename_index(core_file), NULL);
	g_free(cmd_buf);

	f = fopen(cmd_file, "w");
	if (!f)
		{
		printf("Failed to create cmd file\n");
		g_free(cmd_file);
		return;
		}

	fprintf(f, "backtrace\nquit\n");
	fclose(f);

	gdb_cmd = g_strconcat("gdb ", binary, " --core=", core_file, " --command=", cmd_file, NULL);

	f = popen(gdb_cmd, "r");
	g_free(gdb_cmd);
	
	if (!f)
		{
		printf("Unable to do second process of core with gdb\n");
		return;
		}

	while (fgets(buf, 1024, f) != NULL)
                {
		bt_list = g_list_append(bt_list, g_strdup(buf));
		while(gtk_events_pending()) gtk_main_iteration(); /* fake responsiveness */
                }

	fclose(f);

	if (unlink (cmd_file) < 0)
		{
		printf("Unable to remove gdb script file\n");
		}
	g_free(cmd_file);

	bt_list = g_list_prepend(bt_list, g_strdup("Backtrace follows:\n-------------\n"));
	bt_list = g_list_prepend(bt_list, g_strconcat("Core dump generated: ", core_file, "\nby: `", binary,"`\n", NULL));
	bt_list = g_list_prepend(bt_list, g_strdup("===========================================================[Core dump module]==\n"));
	bt_list = g_list_append(bt_list, g_strdup("-------------\n"));

	work = bt_list;
	while(work)
		{
		gchar *text = work->data;
		print_to_log(text, cd->ad);
		work = work->next;
		}

	if (dialog) gtk_widget_destroy(dialog);

	ld = new_list_info(binary, core_file, bt_list);

	text = g_strconcat("A core was just dumped by\n`", binary, "`\n ", NULL);
	icon = gnome_unconditional_pixmap_file ("mc/i-core.png");
	show_coredump_dialog(ld, text);
	id = add_info_line(cd->ad, text, icon, 0, FALSE, 1, 80, 5);
	set_info_signals(id, cd->md, clicked_cb, free_line_cb, NULL, NULL, ld);

	g_free(icon);
	g_free(text);
	
	g_free(binary);
}

static void process_core(CoredumpData *cd)
{
	gchar *new_name;
	gchar *stamp;

	stamp = time_stamp(TRUE);
	new_name = g_strconcat(cd->core_path, "/core-", stamp, NULL);
	g_free(stamp);

	if (rename(cd->core_file, new_name) < 0)
		{
		printf("Failed to rename core: %s\n", cd->core_file);
		g_free(new_name);
		return;
		}

	generate_log(new_name, cd);

	g_free(new_name);
}

static gint core_check_cb(gpointer data)
{
	CoredumpData *cd = data;

	if (!g_file_exists(cd->core_file)) return TRUE;

	/* we have a core! */

	process_core(cd);

	return TRUE;
}

static CoredumpData *init(ModuleData *md, AppData *ad)
{
	CoredumpData *cd = g_new0(CoredumpData, 1);

	cd->enabled = FALSE;
	cd->show_popup = TRUE;
	cd->md = md;
	cd->ad = ad;

	cd->core_path = g_strdup(g_get_home_dir());
	cd->core_file = g_concat_dir_and_file(cd->core_path, "core");

	cd->core_check_timeout_id = -1;

	cd->C_core_path = NULL;
	cd->core_path_entry = NULL;

	return cd;
}

static void show_popup_checkbox_cb(GtkWidget *widget, gpointer data)
{
	CoredumpData *cd = data;

	cd->C_show_popup = GTK_TOGGLE_BUTTON (widget)->active;
	property_changed(cd->md, cd->ad);
}

static void enabled_checkbox_cb(GtkWidget *widget, gpointer data)
{
	CoredumpData *cd = data;

	cd->C_enabled = GTK_TOGGLE_BUTTON (widget)->active;
	property_changed(cd->md, cd->ad);
}

static void path_entry_cb(GtkWidget *widget, gpointer data)
{
	CoredumpData *cd = data;
	property_changed(cd->md, cd->ad);
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

static void mod_coredump_config_apply(gpointer data, AppData *ad)
{
	CoredumpData *cd = data;

	if (cd->core_path_entry)
		{
		gchar *buf = gtk_entry_get_text(GTK_ENTRY(cd->core_path_entry));
		if (buf && strlen(buf) > 0)
			{
			g_free(cd->C_core_path);
			cd->C_core_path = g_strdup(buf);
			}
		}

	g_free(cd->core_path);
	cd->core_path = g_strdup(cd->C_core_path);
	g_free(cd->core_file);
	cd->core_file = g_concat_dir_and_file(cd->core_path, "core");

	if (cd->enabled != cd->C_enabled)
		{
		cd->enabled = cd->C_enabled;
		if (cd->enabled)
			{
			if (cd->core_check_timeout_id == -1)
				cd->core_check_timeout_id = gtk_timeout_add(1000, (GtkFunction)core_check_cb, cd);
			}
		else
			{
			if (cd->core_check_timeout_id != -1) gtk_timeout_remove(cd->core_check_timeout_id);
			cd->core_check_timeout_id = -1;
			}
		}

	cd->show_popup = cd->C_show_popup;
        return;
        ad = NULL;
}

static void mod_coredump_config_close(gpointer data, AppData *ad)
{
	CoredumpData *cd = data;

	g_free(cd->C_core_path);
	cd->C_core_path = NULL;
        return;
        ad = NULL;
}

/* remember that the returned widget is gtk_widget_destroy()ed
 * the widget is also gtk_widget_shown(), do not do it here.
 */
static GtkWidget *mod_coredump_config_show(gpointer data, AppData *ad)
{
	CoredumpData *cd = data;
	GtkWidget *frame;
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *button;

	frame = gtk_frame_new(_("Core Dump Module"));

	vbox = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), GNOME_PAD_SMALL);
	gtk_container_add(GTK_CONTAINER(frame), vbox);
	gtk_widget_show(vbox);

	button = gtk_check_button_new_with_label (_("Enable this module"));
	gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), cd->C_enabled);
	gtk_signal_connect (GTK_OBJECT(button),"clicked",(GtkSignalFunc) enabled_checkbox_cb, cd);
	gtk_widget_show(button);

	button = gtk_check_button_new_with_label (_("Show backtrace dialog on new core files"));
	gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), cd->C_show_popup);
	gtk_signal_connect (GTK_OBJECT(button),"clicked",(GtkSignalFunc) show_popup_checkbox_cb, cd);
	gtk_widget_show(button);

	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

	label = gtk_label_new(_("Path to monitor:"));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_widget_show(label);

	cd->core_path_entry = gtk_entry_new();
	if (cd->C_core_path)
		gtk_entry_set_text(GTK_ENTRY(cd->core_path_entry), cd->C_core_path);
	gtk_signal_connect(GTK_OBJECT(cd->core_path_entry), "changed",
			   GTK_SIGNAL_FUNC(path_entry_cb), cd);
	gtk_box_pack_start(GTK_BOX(hbox), cd->core_path_entry, TRUE, TRUE, 0);
	gtk_widget_show(cd->core_path_entry);

	label = gtk_label_new(cd->md->description);
	gtk_box_pack_end(GTK_BOX(vbox), label, FALSE, FALSE, 0);
	gtk_widget_show(label);
	
	return frame;
        ad = NULL;
}

static void mod_coredump_config_hide(gpointer data, AppData *ad)
{
	CoredumpData *cd = data;
	gchar *buf;

	buf = gtk_entry_get_text(GTK_ENTRY(cd->core_path_entry));
	if (buf && strlen(buf) > 0)
		{
		g_free(cd->C_core_path);
		cd->C_core_path = g_strdup(buf);
		}

	cd->core_path_entry = NULL;
        return;
        ad = NULL;
}

static void mod_coredump_config_init(gpointer data, AppData *ad)
{
	CoredumpData *cd = data;

	cd->C_enabled = cd->enabled;
	cd->C_show_popup = cd->show_popup;
	cd->C_core_path = g_strdup(cd->core_path);

	cd->core_path_entry = NULL;
        return;
        ad = NULL;
}

/*
 *-------------------------------------------------------------------
 * save and load options
 *-------------------------------------------------------------------
 */

static void mod_coredump_load_options(gpointer data, AppData *ad)
{
	CoredumpData *cd = data;
	gchar *buf;

	cd->enabled = gnome_config_get_int("module_coredump/enabled=0");
	cd->show_popup = gnome_config_get_int("module_coredump/show_popup=1");

	buf = gnome_config_get_string("module_coredump/path=");

	if (buf && strlen(buf) > 0)
		{
		g_free(cd->core_path);
		cd->core_path = g_strdup(buf);

		g_free(cd->core_file);
		cd->core_file = g_concat_dir_and_file(cd->core_path, "core");
		}
        return;
        ad = NULL;
}

static void mod_coredump_save_options(gpointer data, AppData *ad)
{
	CoredumpData *cd = data;

	gnome_config_set_int("module_coredump/enabled", cd->enabled);
	gnome_config_set_int("module_coredump/show_popup", cd->show_popup);

	gnome_config_set_string("module_coredump/path", cd->core_path);
        return;
        ad = NULL;
}

/*
 *-------------------------------------------------------------------
 * initialization and setup, exit
 *-------------------------------------------------------------------
 */

static void mod_coredump_start(gpointer data, AppData *ad)
{
	CoredumpData *cd = data;

	if (cd->enabled)
		{
		cd->core_check_timeout_id = gtk_timeout_add(1000, (GtkFunction)core_check_cb, cd);
		}
        return;
        ad = NULL;
}

static void mod_coredump_destroy(gpointer data, AppData *ad)
{
	CoredumpData *cd = data;

	if (cd->core_check_timeout_id != -1) gtk_timeout_remove(cd->core_check_timeout_id);

	g_free(cd->core_path);
	g_free(cd->core_file);

	g_free(cd);
        return;
        ad = NULL;
}

ModuleData *mod_coredump_init(AppData *ad)
{
	ModuleData *md;

	md = g_new0(ModuleData, 1);

	md->ad = ad;
	md->title = _("Core dump catcher");
	md->description = _("This module monitors a path for core dumps, if one is\n found it is time stamped and a backtrace logged (using gdb).");

	md->start_func = mod_coredump_start;

	md->config_init_func =  mod_coredump_config_init;
	md->config_show_func =  mod_coredump_config_show;
	md->config_hide_func =  mod_coredump_config_hide;
	md->config_apply_func =  mod_coredump_config_apply;
	md->config_close_func =  mod_coredump_config_close;
	md->config_load_func =  mod_coredump_load_options;
	md->config_save_func =  mod_coredump_save_options;
	md->destroy_func =  mod_coredump_destroy;

	md->internal_data = (gpointer)init(md, ad);

	return md;
}


