/* tick-a-stat, a GNOME panel applet
 * (C) 1999 John Ellis
 *
 * Author: John Ellis
 *
 */

#include "tickastat.h"
#include "modules.h"

typedef struct _TestData TestData;
struct _TestData
{
	ModuleData *md;
	AppData *ad;

	gint enabled;

	gint C_enabled;
};

/*
 *-------------------------------------------------------------------
 * This is just a test case module for .... testing ;)
 *-------------------------------------------------------------------
 */

static void free_cb(ModuleData *md, gpointer data)
{
	printf("Freeing [%s]\n", (gchar *)data);
	g_free(data);
	return;
	md = NULL;
}

static void clicked_cb(ModuleData *md, gpointer data, InfoData *id, AppData *ad)
{
	gnome_warning_dialog(data);
        return;
        md = NULL;
	id = NULL;
	ad = NULL;
}

static void post_show_cb(ModuleData *md, gpointer data, InfoData *id, AppData *ad)
{
	TestData *td = md->internal_data;
	if (!td->enabled) remove_info_line(ad, id);
        return;
        data = NULL;
}

static void show_a_line(TestData *td)
{
	InfoData *id;

	id = add_info_line(td->ad, _("Testing line from test module :)"), NULL, 0, FALSE, -1, 20, 1);
	set_info_signals(id, td->md, clicked_cb, free_cb, NULL, post_show_cb, g_strdup(_("You clicked on me")));
}

static TestData *init(ModuleData *md, AppData *ad)
{
	TestData *td = g_new0(TestData, 1);

	td->enabled = FALSE;
	td->md = md;
	td->ad = ad;

	return td;
}

static void enabled_checkbox_cb(GtkWidget *widget, gpointer data)
{
	TestData *td = data;

	td->C_enabled = GTK_TOGGLE_BUTTON (widget)->active;
	property_changed(td->md, td->ad);
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

static void mod_test_config_apply(gpointer data, AppData *ad)
{
	TestData *td = data;

	if (td->enabled != td->C_enabled)
		{
		td->enabled = td->C_enabled;
		if (td->enabled)
			{
			show_a_line(td);
			}
		}
        return;
        ad = NULL;
}

static void mod_test_config_close(gpointer data, AppData *ad)
{
        return;
	ad = NULL;
	data = NULL;
}

/* remember that the returned widget is gtk_widget_destroy()ed
 * the widget is also gtk_widget_shown(), do not do it here.
 */
static GtkWidget *mod_test_config_show(gpointer data, AppData *ad)
{
	TestData *td = data;
	GtkWidget *frame;
	GtkWidget *vbox;
	GtkWidget *label;
	GtkWidget *button;

	frame = gtk_frame_new(_("Test Module"));

	vbox = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_container_set_border_width (GTK_CONTAINER(vbox), GNOME_PAD_SMALL);
	gtk_container_add(GTK_CONTAINER(frame), vbox);
	gtk_widget_show(vbox);

	button = gtk_check_button_new_with_label (_("Enable this module"));
	gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), td->C_enabled);
	gtk_signal_connect (GTK_OBJECT(button),"clicked",(GtkSignalFunc) enabled_checkbox_cb, td);
	gtk_widget_show(button);

	label = gtk_label_new(td->md->description);
	gtk_box_pack_end(GTK_BOX(vbox), label, FALSE, FALSE, 0);
	gtk_widget_show(label);
	
	return frame;
        ad = NULL;
}

static void mod_test_config_hide(gpointer data, AppData *ad)
{
/*	TestData *td = data;*/
    return;
    data = NULL;
    ad = NULL;
}

static void mod_test_config_init(gpointer data, AppData *ad)
{
	TestData *td = data;

	td->C_enabled = td->enabled;
	return;
	ad = NULL;
}

/*
 *-------------------------------------------------------------------
 * save and load options
 *-------------------------------------------------------------------
 */

static void mod_test_load_options(gpointer data, AppData *ad)
{
	TestData *td = data;

	td->enabled = gnome_config_get_int("module_test/enabled=0");
	return;
	ad = NULL;
}

static void mod_test_save_options(gpointer data, AppData *ad)
{
	TestData *td = data;

	gnome_config_set_int("module_test/enabled", td->enabled);
	return;
	ad = NULL;
}

/*
 *-------------------------------------------------------------------
 * initialization and setup, exit
 *-------------------------------------------------------------------
 */

static void start(gpointer data, AppData *ad)
{
	TestData *td = data;

	if (td->enabled) show_a_line(td);
	return;
	ad = NULL;
}

static void mod_test_destroy(gpointer data, AppData *ad)
{
	TestData *td = data;
	g_free(td);
	return;
	ad = NULL;
}

ModuleData *mod_test_init(AppData *ad)
{
	ModuleData *md;

	md = g_new0(ModuleData, 1);

	md->ad = ad;
	md->title = _("Test Module");
	md->description = _("This is the test module's description line.");

	md->start_func =  start;

	md->config_init_func =  mod_test_config_init;
	md->config_show_func =  mod_test_config_show;
	md->config_hide_func =  mod_test_config_hide;
	md->config_apply_func =  mod_test_config_apply;
	md->config_close_func =  mod_test_config_close;
	md->config_load_func =  mod_test_load_options;
	md->config_save_func =  mod_test_save_options;
	md->destroy_func =  mod_test_destroy;

	md->internal_data = (gpointer)init(md, ad);

	return md;
}


