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
 * The load average module for Tick-A-Stat
 * 
 * Monitors the load average and alerts the user when it goes above
 * a specified threshhold.
 * 
 *-------------------------------------------------------------------
 */

typedef struct _LoadavgData LoadavgData;
struct _LoadavgData
{
	ModuleData *md;
	AppData *ad;

	gint enabled;

	gint update_interval;	/* in seconds, 10 - 300(5 min) */

	gint show_warning_popup;
	gint show_danger_popup;
	gint warning_point;
	gint danger_point;
	gchar *warning_text;
	gchar *danger_text;

	gint loadavg_check_timeout_id;

	gint C_enabled;
	gint C_update_interval;
	gint C_show_warning_popup;
	gint C_show_danger_popup;
	gint C_warning_point;
	gint C_danger_point;
	gchar *C_warning_text;
	gchar *C_danger_text;

	GtkWidget *warning_text_entry;
	GtkWidget *danger_text_entry;

	GtkWidget *pop_up_dialog;
};

typedef struct _LineData LineData;
struct _LineData
{
	gchar *reason; /* do not free */
	gchar *text;
	gchar *description;
	gchar *icon;
};

static LineData *line_data_new(gchar *reason, gchar *text, gchar *icon_path)
{
	LineData *ld = g_new0(LineData, 1);

	ld->reason = reason;
	ld->text = g_strdup(text);
	ld->description = NULL;
	ld->icon = g_strdup(icon_path);

	return ld;
}

static void line_data_free(LineData *ld)
{
	g_free(ld->text);
	g_free(ld->description);
	g_free(ld->icon);
	g_free(ld);
}

/*
 *-------------------------------------------------------------------
 * dialogs
 *-------------------------------------------------------------------
 */

static void loadavg_dialog_close(GtkWidget *widget, gpointer data)
{
	LoadavgData *lad = data;

	if (GTK_WIDGET(widget) == lad->pop_up_dialog)
		{
		lad->pop_up_dialog = NULL;
		}
}

static void show_loadavg_dialog(InfoData *id, LoadavgData *lad, gint pop)
{
	LineData *ld = id->data;
	GtkWidget *dialog;
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *label;

	dialog = gnome_dialog_new(_("Load Average information (Tick-a-Stat)"),
				  GNOME_STOCK_BUTTON_CLOSE, NULL);
	gnome_dialog_set_close(GNOME_DIALOG(dialog), TRUE);

	vbox = GNOME_DIALOG(dialog)->vbox;

	hbox = gtk_hbox_new(FALSE, GNOME_PAD);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

	if (ld->icon)
		{
		GtkWidget *icon = NULL;
		icon = gnome_pixmap_new_from_file(ld->icon);
		if (icon)
			{
			gtk_box_pack_start(GTK_BOX(hbox), icon, FALSE, FALSE, 0);
			gtk_widget_show(icon);
			}
		}

	label = gtk_label_new(id->text);
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
	gtk_widget_show(label);

	label = gtk_label_new(ld->description);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
	gtk_widget_show(label);

	if (pop)
		{
		if (lad->pop_up_dialog)
			{
			gtk_widget_destroy(lad->pop_up_dialog);
			}
		lad->pop_up_dialog = dialog;
		gtk_signal_connect(GTK_OBJECT(dialog), "destroy", GTK_SIGNAL_FUNC(loadavg_dialog_close), lad);
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
	LoadavgData *lad = (LoadavgData *)(md->internal_data);
	show_loadavg_dialog(id, lad, FALSE);
        return;
        ad = NULL;
	data = NULL;
}

static void free_line_cb(ModuleData *md, gpointer data)
{
	LineData *ld = data;
	line_data_free(ld);
        return;
        md = NULL;
}

/*
 *-------------------------------------------------------------------
 * loadavg processing
 *-------------------------------------------------------------------
 */

static void show_loadavg_line(gchar *reason, gchar *icon_name, gfloat n,
			      gchar *load_text, gint np, gchar *text,
			      LoadavgData *lad, gint pop)
{
	gchar *icon_path;
	InfoData *id;
	gchar *line_text;
	LineData *ld;
	gchar *stamp;

	icon_path = gnome_unconditional_pixmap_file(icon_name);

	line_text = g_strdup_printf("Load average %s (%.1f)\n%s", reason, n, text);

	ld = line_data_new(reason, line_text, icon_path);
	ld->description = g_strdup_printf("System load average %s:\nThe load (%.1f) has exceeded the preset threshhold of %d\nThe load averages were %s",
				   reason, n, np, load_text);

	id = add_info_line(lad->ad, line_text, icon_path, 0, FALSE, 1, 80, 8);
	set_info_signals(id, lad->md, clicked_cb, free_line_cb, NULL, NULL, ld);

	/* print log */
	stamp = time_stamp(FALSE);
	print_to_log("=======================================================[ Load average module ]==\n", lad->ad);
	print_to_log(stamp, lad->ad);
	g_free(stamp);
	print_to_log("\n", lad->ad);
	print_to_log(ld->description, lad->ad);
	print_to_log("\n", lad->ad);
	
	/* show dialog */
	if (pop) show_loadavg_dialog(id, lad, TRUE);

	g_free(icon_path);
	g_free(line_text);
}

static void do_avg_compare(gfloat n, gchar *text, LoadavgData *lad)
{
	gint l;

	l = (gint)n;

	if (n >= lad->danger_point)
		{
		show_loadavg_line("Alert", "gnome-warning.png", n, text,
				  lad->danger_point, lad->danger_text,
				  lad, lad->show_danger_popup);
		}
	else if (n >= lad->warning_point)
		{
		show_loadavg_line("Warning", "gnome-info.png", n, text,
				  lad->warning_point, lad->warning_text,
				  lad, lad->show_warning_popup);
		}
}


static gchar *get_loadavg(gfloat *l1, gfloat *l2, gfloat *l3)
{
	gchar *text = NULL;
	gchar buf[1024];
	FILE *f;
	gchar *ptr;

	buf[0] = '\0';
	*l1 = 0.0;
	*l2 = 0.0;
	*l3 = 0.0;

	f = popen("uptime", "r");
	if (!f)
		{
		printf("Unable to process uptime\n");
		return NULL;
		}

	while (fgets(buf, 1024, f) != NULL)
                {
		/* do nothing */
		}

	fclose(f);

	if (buf[0] == '\0') return NULL;

	ptr = strstr(buf, "load aver");

	if (!ptr)
		{
		printf("Unable to find load string\n");
		return NULL;
		}

	while(*ptr != ':' && *ptr != '\0') ptr++;
	if (*ptr == '\0') return NULL;
	ptr++;
	while(*ptr == ' ' && *ptr != '\0') ptr++;
	if (*ptr == '\0') return NULL;

	sscanf (ptr, "%f,%f,%f", l1, l2, l3);

	text = g_strdup_printf("%3.1f %3.1f %3.1f", *l1, *l2, *l3);

	return text;
}

static gint loadavg_check_cb(gpointer data)
{
	LoadavgData *lad = data;
	gchar *avg_text;
	gfloat n1, n2, n3;
	gfloat n;

	avg_text = get_loadavg(&n1, &n2, &n3);
	if (!avg_text) return TRUE;

	n = n1;	/* 1 min hardcoded here, FIXME! */

	do_avg_compare(n, avg_text, lad);

	g_free(avg_text);

	return TRUE;
}

/*
 *-------------------------------------------------------------------
 * 
 *-------------------------------------------------------------------
 */

static LoadavgData *init(ModuleData *md, AppData *ad)
{
	LoadavgData *lad = g_new0(LoadavgData, 1);

	lad->enabled = FALSE;
	lad->md = md;
	lad->ad = ad;

	lad->update_interval = 60;	/* in seconds, 10 - 300(5 min) */

	lad->show_warning_popup = TRUE;
	lad->show_danger_popup = TRUE;
	lad->warning_point = 15;
	lad->danger_point = 20;
	lad->warning_text = g_strdup(_("The Load average is very high."));
	lad->danger_text = g_strdup(_("Load average is at a critical point, help me!"));

	lad->loadavg_check_timeout_id = -1;

	lad->warning_text_entry = NULL;
	lad->danger_text_entry = NULL;

	lad->pop_up_dialog = NULL;

	return lad;
}

static void show_warning_popup_checkbox_cb(GtkWidget *widget, gpointer data)
{
	LoadavgData *lad = data;

	lad->C_show_warning_popup = GTK_TOGGLE_BUTTON (widget)->active;
	property_changed(lad->md, lad->ad);
}

static void show_danger_popup_checkbox_cb(GtkWidget *widget, gpointer data)
{
	LoadavgData *lad = data;

	lad->C_show_danger_popup = GTK_TOGGLE_BUTTON (widget)->active;
	property_changed(lad->md, lad->ad);
}

static void enabled_checkbox_cb(GtkWidget *widget, gpointer data)
{
	LoadavgData *lad = data;

	lad->C_enabled = GTK_TOGGLE_BUTTON (widget)->active;
	property_changed(lad->md, lad->ad);
}

static void interval_cb(GtkObject *obj, gpointer data)
{
	LoadavgData *lad = data;

	if (GTK_IS_ADJUSTMENT(obj))
                lad->C_update_interval = (gint)GTK_ADJUSTMENT(obj)->value;
        else
                lad->C_update_interval = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(obj));

	property_changed(lad->md, lad->ad);
}

static void warning_point_cb(GtkObject *obj, gpointer data)
{
	LoadavgData *lad = data;

	if (GTK_IS_ADJUSTMENT(obj))
                lad->C_warning_point = (gint)GTK_ADJUSTMENT(obj)->value;
        else
                lad->C_warning_point = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(obj));

	property_changed(lad->md, lad->ad);
}

static void danger_point_cb(GtkObject *obj, gpointer data)
{
	LoadavgData *lad = data;

	if (GTK_IS_ADJUSTMENT(obj))
                lad->C_danger_point = (gint)GTK_ADJUSTMENT(obj)->value;
        else
                lad->C_danger_point = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(obj));

	property_changed(lad->md, lad->ad);
}

static void misc_entry_cb(GtkWidget *widget, gpointer data)
{
	LoadavgData *lad = data;
	property_changed(lad->md, lad->ad);
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

static void mod_loadavg_config_apply(gpointer data, AppData *ad)
{
	LoadavgData *lad = data;

	if (lad->warning_text_entry)
		{
		gchar *buf = gtk_entry_get_text(GTK_ENTRY(lad->warning_text_entry));
		if (buf && strlen(buf) > 0)
			{
			g_free(lad->C_warning_text);
			lad->C_warning_text = g_strdup(buf);
			}
		}
	if (lad->danger_text_entry)
		{
		gchar *buf = gtk_entry_get_text(GTK_ENTRY(lad->danger_text_entry));
		if (buf && strlen(buf) > 0)
			{
			g_free(lad->C_danger_text);
			lad->C_danger_text = g_strdup(buf);
			}
		}

	g_free(lad->warning_text);
	lad->warning_text = g_strdup(lad->C_warning_text);
	g_free(lad->danger_text);
	lad->danger_text = g_strdup(lad->C_danger_text);

	if (lad->update_interval != lad->C_update_interval)
		{
		lad->update_interval = lad->C_update_interval;
		if (lad->enabled == lad->C_enabled && lad->enabled == TRUE &&
		    lad->loadavg_check_timeout_id != -1)
			{
			gtk_timeout_remove(lad->loadavg_check_timeout_id);
			lad->loadavg_check_timeout_id = gtk_timeout_add(lad->update_interval * 1000, (GtkFunction)loadavg_check_cb, lad);
			}
		}

	if (lad->enabled != lad->C_enabled)
		{
		lad->enabled = lad->C_enabled;
		if (lad->enabled)
			{
			if (lad->loadavg_check_timeout_id == -1)
				lad->loadavg_check_timeout_id = gtk_timeout_add(lad->update_interval * 1000, (GtkFunction)loadavg_check_cb, lad);
			}
		else
			{
			if (lad->loadavg_check_timeout_id != -1) gtk_timeout_remove(lad->loadavg_check_timeout_id);
			lad->loadavg_check_timeout_id = -1;
			}
		}

	lad->show_warning_popup = lad->C_show_warning_popup;
	lad->show_danger_popup = lad->C_show_danger_popup;

	lad->warning_point = lad->C_warning_point;
	lad->danger_point = lad->C_danger_point;
        return;
        ad = NULL;
}

static void mod_loadavg_config_close(gpointer data, AppData *ad)
{
	LoadavgData *lad = data;

	g_free(lad->C_warning_text);
	lad->C_warning_text = NULL;

	g_free(lad->C_danger_text);
	lad->C_danger_text = NULL;
        return;
        ad = NULL;
}

/* remember that the returned widget is gtk_widget_destroy()ed
 * the widget is also gtk_widget_shown(), do not do it here.
 */
static GtkWidget *mod_loadavg_config_show(gpointer data, AppData *ad)
{
	LoadavgData *lad = data;
	GtkWidget *frame;
	GtkWidget *frame1;
	GtkWidget *vbox;
	GtkWidget *vbox1;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *button;

	GtkWidget *spin;
	GtkObject *adj;

	frame = gtk_frame_new(_("Load Average Module"));

	vbox = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), GNOME_PAD_SMALL);
	gtk_container_add(GTK_CONTAINER(frame), vbox);
	gtk_widget_show(vbox);

	button = gtk_check_button_new_with_label (_("Enable this module"));
	gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), lad->C_enabled);
	gtk_signal_connect (GTK_OBJECT(button),"clicked",(GtkSignalFunc) enabled_checkbox_cb, lad);
	gtk_widget_show(button);

	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

	label = gtk_label_new(_("Check every:"));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_widget_show(label);

	adj = gtk_adjustment_new(lad->C_update_interval, 10.0, 300.0, 1, 1, 1 );
	spin = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 1, 0 );
	gtk_box_pack_start(GTK_BOX(hbox), spin, FALSE, FALSE, 0);
	gtk_signal_connect(GTK_OBJECT(adj),"value_changed",GTK_SIGNAL_FUNC(interval_cb), lad);
	gtk_signal_connect(GTK_OBJECT(spin),"changed",GTK_SIGNAL_FUNC(interval_cb), lad);
	gtk_spin_button_set_update_policy(GTK_SPIN_BUTTON(spin), GTK_UPDATE_ALWAYS);
        gtk_widget_show(spin);

	label = gtk_label_new(_("Seconds"));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_widget_show(label);

	/* Warning dialog */

	frame1 = gtk_frame_new(_("Warning options"));
	gtk_box_pack_start(GTK_BOX(vbox), frame1, FALSE, FALSE, 0);
	gtk_widget_show(frame1);

	vbox1 = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_container_set_border_width(GTK_CONTAINER(vbox1), GNOME_PAD_SMALL);
	gtk_container_add(GTK_CONTAINER(frame1), vbox1);
	gtk_widget_show(vbox1);

	button = gtk_check_button_new_with_label (_("Show pop-up dialog for this event"));
	gtk_box_pack_start(GTK_BOX(vbox1), button, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), lad->C_show_warning_popup);
	gtk_signal_connect (GTK_OBJECT(button),"clicked",(GtkSignalFunc) show_warning_popup_checkbox_cb, lad);
	gtk_widget_show(button);

	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(vbox1), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

	label = gtk_label_new(_("Load average threshhold:"));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_widget_show(label);

	adj = gtk_adjustment_new(lad->C_warning_point, 1.0, 300.0, 1, 1, 1 );
	spin = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 1, 0 );
	gtk_box_pack_start(GTK_BOX(hbox), spin, FALSE, FALSE, 0);
	gtk_signal_connect(GTK_OBJECT(adj),"value_changed",GTK_SIGNAL_FUNC(warning_point_cb), lad);
	gtk_signal_connect(GTK_OBJECT(spin),"changed",GTK_SIGNAL_FUNC(warning_point_cb), lad);
	gtk_spin_button_set_update_policy(GTK_SPIN_BUTTON(spin), GTK_UPDATE_ALWAYS);
        gtk_widget_show(spin);

	label = gtk_label_new(_("Text to display:"));
	gtk_box_pack_start(GTK_BOX(vbox1), label, FALSE, FALSE, 0);
	gtk_widget_show(label);

	lad->warning_text_entry = gtk_entry_new();
	if (lad->C_warning_text)
		gtk_entry_set_text(GTK_ENTRY(lad->warning_text_entry), lad->C_warning_text);
	gtk_signal_connect(GTK_OBJECT(lad->warning_text_entry), "changed",
			   GTK_SIGNAL_FUNC(misc_entry_cb), lad);
	gtk_box_pack_start(GTK_BOX(vbox1), lad->warning_text_entry, FALSE, FALSE, 0);
	gtk_widget_show(lad->warning_text_entry);

	/* Danger dialog */

	frame1 = gtk_frame_new(_("Alert options"));
	gtk_box_pack_start(GTK_BOX(vbox), frame1, FALSE, FALSE, 0);
	gtk_widget_show(frame1);

	vbox1 = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_container_set_border_width(GTK_CONTAINER(vbox1), GNOME_PAD_SMALL);
	gtk_container_add(GTK_CONTAINER(frame1), vbox1);
	gtk_widget_show(vbox1);

	button = gtk_check_button_new_with_label (_("Show pop-up dialog for this event"));
	gtk_box_pack_start(GTK_BOX(vbox1), button, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), lad->C_show_danger_popup);
	gtk_signal_connect (GTK_OBJECT(button),"clicked",(GtkSignalFunc) show_danger_popup_checkbox_cb, lad);
	gtk_widget_show(button);

	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(vbox1), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

	label = gtk_label_new(_("Load average threshhold:"));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_widget_show(label);

	adj = gtk_adjustment_new(lad->C_danger_point, 1.0, 300.0, 1, 1, 1 );
	spin = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 1, 0 );
	gtk_box_pack_start(GTK_BOX(hbox), spin, FALSE, FALSE, 0);
	gtk_signal_connect(GTK_OBJECT(adj),"value_changed",GTK_SIGNAL_FUNC(danger_point_cb), lad);
	gtk_signal_connect(GTK_OBJECT(spin),"changed",GTK_SIGNAL_FUNC(danger_point_cb), lad);
	gtk_spin_button_set_update_policy(GTK_SPIN_BUTTON(spin), GTK_UPDATE_ALWAYS);
        gtk_widget_show(spin);

	label = gtk_label_new(_("Text to display:"));
	gtk_box_pack_start(GTK_BOX(vbox1), label, FALSE, FALSE, 0);
	gtk_widget_show(label);

	lad->danger_text_entry = gtk_entry_new();
	if (lad->C_danger_text)
		gtk_entry_set_text(GTK_ENTRY(lad->danger_text_entry), lad->C_danger_text);
	gtk_signal_connect(GTK_OBJECT(lad->danger_text_entry), "changed",
			   GTK_SIGNAL_FUNC(misc_entry_cb), lad);
	gtk_box_pack_start(GTK_BOX(vbox1), lad->danger_text_entry, FALSE, FALSE, 0);
	gtk_widget_show(lad->danger_text_entry);

	label = gtk_label_new(lad->md->description);
	gtk_box_pack_end(GTK_BOX(vbox), label, FALSE, FALSE, 0);
	gtk_widget_show(label);
	
	return frame;
        ad = NULL;
}

static void mod_loadavg_config_hide(gpointer data, AppData *ad)
{
	LoadavgData *lad = data;
	gchar *buf;

	buf = gtk_entry_get_text(GTK_ENTRY(lad->warning_text_entry));
	if (buf && strlen(buf) > 0)
		{
		g_free(lad->C_warning_text);
		lad->C_warning_text = g_strdup(buf);
		}
	lad->warning_text_entry = NULL;

	buf = gtk_entry_get_text(GTK_ENTRY(lad->danger_text_entry));
	if (buf && strlen(buf) > 0)
		{
		g_free(lad->C_danger_text);
		lad->C_danger_text = g_strdup(buf);
		}
	lad->danger_text_entry = NULL;
        return;
        ad = NULL;
}

static void mod_loadavg_config_init(gpointer data, AppData *ad)
{
	LoadavgData *lad = data;

	lad->C_enabled = lad->enabled;
	lad->C_update_interval = lad->update_interval;
	lad->C_show_warning_popup = lad->show_warning_popup;
	lad->C_show_danger_popup = lad->show_danger_popup;
	lad->C_warning_point = lad->warning_point;
	lad->C_danger_point = lad->danger_point;
	lad->C_warning_text = g_strdup(lad->warning_text);
	lad->C_danger_text = g_strdup(lad->danger_text);

	lad->warning_text_entry = NULL;
	lad->danger_text_entry = NULL;
        return;
        ad = NULL;
}

/*
 *-------------------------------------------------------------------
 * save and load options
 *-------------------------------------------------------------------
 */

static void mod_loadavg_load_options(gpointer data, AppData *ad)
{
	LoadavgData *lad = data;
	gchar *buf;

	lad->enabled = gnome_config_get_int("module_loadavg/enabled=0");
	lad->update_interval = gnome_config_get_int("module_loadavg/interval=60");

	lad->show_warning_popup = gnome_config_get_int("module_loadavg/show_warning_popup=1");
	lad->show_danger_popup = gnome_config_get_int("module_loadavg/show_danger_popup=1");

	lad->warning_point = gnome_config_get_int("module_loadavg/warning_point=15");
	lad->danger_point = gnome_config_get_int("module_loadavg/danger_point=20");

	buf = gnome_config_get_string("module_loadavg/warning_text=");
	if (buf && strlen(buf) > 0)
		{
		g_free(lad->warning_text);
		lad->warning_text = g_strdup(buf);
		}

	buf = gnome_config_get_string("module_loadavg/danger_text=");
	if (buf && strlen(buf) > 0)
		{
		g_free(lad->danger_text);
		lad->danger_text = g_strdup(buf);
		}
        return;
        ad = NULL;
}

static void mod_loadavg_save_options(gpointer data, AppData *ad)
{
	LoadavgData *lad = data;

	gnome_config_set_int("module_loadavg/enabled", lad->enabled);
	gnome_config_set_int("module_loadavg/interval", lad->update_interval);

	gnome_config_set_int("module_loadavg/show_warning_popup", lad->show_warning_popup);
	gnome_config_set_int("module_loadavg/show_danger_popup", lad->show_danger_popup);

	gnome_config_set_int("module_loadavg/warning_point", lad->warning_point);
	gnome_config_set_int("module_loadavg/danger_point", lad->danger_point);

	gnome_config_set_string("module_loadavg/warning_text", lad->warning_text);
	gnome_config_set_string("module_loadavg/danger_text", lad->danger_text);
        return;
        ad = NULL;
}

/*
 *-------------------------------------------------------------------
 * initialization and setup, exit
 *-------------------------------------------------------------------
 */

static void mod_loadavg_start(gpointer data, AppData *ad)
{
	LoadavgData *lad = data;

	if (lad->enabled)
		{
		lad->loadavg_check_timeout_id = gtk_timeout_add(lad->update_interval * 1000, (GtkFunction)loadavg_check_cb, lad);
		}
        return;
        ad = NULL;
}

static void mod_loadavg_destroy(gpointer data, AppData *ad)
{
	LoadavgData *lad = data;

	if (lad->loadavg_check_timeout_id != -1) gtk_timeout_remove(lad->loadavg_check_timeout_id);

	g_free(lad->warning_text);
	g_free(lad->danger_text);

	g_free(lad);
        return;
        ad = NULL;
}

ModuleData *mod_loadavg_init(AppData *ad)
{
	ModuleData *md;

	md = g_new0(ModuleData, 1);

	md->ad = ad;
	md->title = _("Load average monitor");
	md->description = _("This module monitors the system load average,\n and displays warning when it rises above certain points.");

	md->start_func = mod_loadavg_start;

	md->config_init_func =  mod_loadavg_config_init;
	md->config_show_func =  mod_loadavg_config_show;
	md->config_hide_func =  mod_loadavg_config_hide;
	md->config_apply_func =  mod_loadavg_config_apply;
	md->config_close_func =  mod_loadavg_config_close;
	md->config_load_func =  mod_loadavg_load_options;
	md->config_save_func =  mod_loadavg_save_options;
	md->destroy_func =  mod_loadavg_destroy;

	md->internal_data = (gpointer)init(md, ad);

	return md;
}


