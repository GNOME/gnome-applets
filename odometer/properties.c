/*
 * GNOME odometer panel applet
 * (C) 1999 The Free software Foundation
 * 
 * Author: Fabrice Bellet <Fabrice.Bellet@creatis.insa-lyon.fr>
 *         adapted from kodo/Xodometer/Mouspedometa
 *
 * The property dialog box is heavily inpired from the
 * sound-monitor applet. Thanks to John Ellis.
 *
 */

#include <config.h>
#include "odo.h"

static int
auto_reset_selected_cb (GtkWidget *b, gpointer data)
{
   OdoApplet *oa = data;

   oa->p_auto_reset=GTK_TOGGLE_BUTTON (b)->active;
   gnome_property_box_changed(GNOME_PROPERTY_BOX(oa->pbox));
   return FALSE;
}

static int
use_metric_selected_cb (GtkWidget *b, gpointer data)
{
   OdoApplet *oa = data;

   oa->p_use_metric=GTK_TOGGLE_BUTTON (b)->active;
   gnome_property_box_changed(GNOME_PROPERTY_BOX(oa->pbox));
   return FALSE;
}

static int
enabled_selected_cb (GtkWidget *b, gpointer data)
{
   OdoApplet *oa = data;

   oa->p_enabled=GTK_TOGGLE_BUTTON (b)->active;
   gnome_property_box_changed(GNOME_PROPERTY_BOX(oa->pbox));
   return FALSE;
}

static int
digits_number_changed_cb (GtkWidget *widget, gpointer data)
{
   OdoApplet *oa = data;

   oa->p_digits_nb = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(oa->spinner));
   gnome_property_box_changed(GNOME_PROPERTY_BOX(oa->pbox));
   return FALSE;
   widget = NULL;
}

static int
scale_applet_cb (GtkWidget *b, gpointer data)
{
   OdoApplet *oa = data;

   oa->p_scale_applet=GTK_TOGGLE_BUTTON (b)->active;
   gnome_property_box_changed(GNOME_PROPERTY_BOX(oa->pbox));
   return FALSE;
}

static void
properties_apply_cb (GtkWidget *b, gint page_num, gpointer data)
{
   OdoApplet *oa = data;
   gchar *buf;

   oa->use_metric = oa->p_use_metric;
   oa->auto_reset = oa->p_auto_reset;
   oa->enabled = oa->p_enabled;
   if (oa->digits_nb != oa->p_digits_nb) {
   	oa->digits_nb = oa->p_digits_nb;
   	change_digits_nb (oa);
   }
   if (oa->scale_applet != oa->p_scale_applet) {
	oa->scale_applet = oa->p_scale_applet;
	change_theme(oa->theme_file, oa);
   }

   buf = gtk_entry_get_text(GTK_ENTRY(oa->theme_entry));
   if (buf && oa->theme_file && strcmp(buf, oa->theme_file) != 0) {
   	g_free (oa->theme_file);
   	oa->theme_file = g_strdup(buf);
   	if (!change_theme(oa->theme_file, oa))
   		change_theme(NULL, oa);
   } else if (buf && strlen(buf) != 0) {
   	if (oa->theme_file) g_free (oa->theme_file);
   	oa->theme_file = g_strdup(buf);
   	if (!change_theme(oa->theme_file, oa))
   		change_theme(NULL, oa);
   } else {
   	if (oa->theme_file) g_free (oa->theme_file);
   	oa->theme_file = NULL;
   	change_theme(NULL, oa);
   }

   applet_widget_sync_config (APPLET_WIDGET (oa->applet));
   refresh(oa);
   return;
   b = NULL;
   page_num = 0;
}

static gint
properties_destroy_cb (GtkWidget *widget, gpointer data)
{
   OdoApplet *oa = data;
   oa->pbox = NULL;
   return FALSE;
   widget = NULL;
}

static void
theme_selected_cb (GtkWidget *clist,
	gint row,gint col,
	GdkEventButton *event,
	gpointer data)
{
  OdoApplet *oa = data;
  gchar *text = gtk_clist_get_row_data(GTK_CLIST(clist), row);
  if (text) gtk_entry_set_text(GTK_ENTRY(oa->theme_entry),text);
  return;
  col = 0;
  event = NULL;
}

static gint
sort_theme_list_cb(void *a, void *b)
{
  return strcmp((gchar *)a, (gchar *)b);
}

static void
populate_theme_list (GtkWidget *clist)
{
   DIR *dp;
   struct dirent *dir;
   struct stat ent_sbuf;
   gchar *buf[] = { "x", };
   gint row;
   gchar *themepath;
   GList *theme_list = NULL;
   GList *list;

   /* add default theme */
   buf[0] = _("None (default)");
   row = gtk_clist_append(GTK_CLIST(clist),buf);
   gtk_clist_set_row_data(GTK_CLIST(clist), row, "");

   themepath = gnome_unconditional_datadir_file("odometer");

   if((dp = opendir(themepath))==NULL) {
   	/* dir not found */
   	g_free(themepath);
   	return;
   }

   while ((dir = readdir(dp)) != NULL) {
   	/* skips removed files and also skips the default dir */
   	if (dir->d_ino > 0
   		&& strcmp (dir->d_name, "default")) {
   		gchar *name;
   		gchar *path;

   		name = dir->d_name;
   		path = g_strconcat(themepath, "/", name, NULL);
   		if (stat(path,&ent_sbuf) >= 0 && S_ISDIR(ent_sbuf.st_mode)) {
   			theme_list = g_list_insert_sorted(theme_list,
   				g_strdup(name),
   				(GCompareFunc) sort_theme_list_cb);

   		}
   		g_free(path);
   	}
   }
   closedir(dp);

   list = theme_list;
   while (list) {
   	gchar *themedata_file = g_strconcat(themepath, "/", list->data, "/themedata", NULL);
   	if (g_file_exists(themedata_file)) {
   		gchar *theme_file = g_strconcat (themepath, "/", list->data, NULL);
   		buf[0] = list->data;
   		row = gtk_clist_append(GTK_CLIST(clist),buf);
   		gtk_clist_set_row_data_full(GTK_CLIST(clist), row,
   			theme_file, (GtkDestroyNotify) g_free);
	}
   	g_free(themedata_file);
   	g_free(list->data);
   	list = list->next;
   }
   g_list_free(theme_list);
   g_free(themepath);
}

static void
phelp_cb (GtkWidget *w, gint tab, gpointer data)
{
	GnomeHelpMenuEntry help_entry = { 
		"odometer_applet", "index.html#ODOMETER-PREFS"
	};
	gnome_help_display (NULL, &help_entry);
}

/*
 * Callback to access properties of the applet: you can toggle:
 *   - the Metric mode
 *   - the trip auto-reset mode each time the applet is restarted
 *   - the enable/disable mode
 */
void
properties_cb (AppletWidget *applet, gpointer data)
{
   OdoApplet *oa = data;
   GtkWidget *label;
   GtkWidget *frame;
   GtkWidget *frame2;
   GtkWidget *hbox;
   GtkWidget *vbox;
   GtkWidget *scrolled;
   GtkWidget *theme_clist;
   GtkAdjustment *adj;
   gchar *theme_title[] = { N_("Themes:"), NULL };

   if (oa->pbox) {
   	gdk_window_raise (oa->pbox->window);
   	return;
   }

   oa->p_use_metric = oa->use_metric;
   oa->p_enabled = oa->enabled;
   oa->p_auto_reset = oa->auto_reset;
   oa->p_digits_nb = oa->digits_nb;
   oa->p_scale_applet = oa->scale_applet;

   oa->pbox=gnome_property_box_new();
   gtk_window_set_title (
   	GTK_WINDOW(&GNOME_PROPERTY_BOX(oa->pbox)->dialog.window),
   	_("Odometer setting"));

   /* General Tab */

   frame = gtk_frame_new(NULL);
   gtk_container_set_border_width (GTK_CONTAINER (frame), GNOME_PAD_SMALL);

   vbox = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
   gtk_container_set_border_width (GTK_CONTAINER (vbox), GNOME_PAD_SMALL);
   gtk_container_add(GTK_CONTAINER(frame), vbox);

   label = gtk_check_button_new_with_label (_("Use Metric"));
   gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(label),oa->p_use_metric);
   gtk_signal_connect (GTK_OBJECT(label), "clicked", (GtkSignalFunc)
   	use_metric_selected_cb,oa);

   label = gtk_check_button_new_with_label (_("auto_reset"));
   gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(label),oa->p_auto_reset);
   gtk_signal_connect (GTK_OBJECT(label), "clicked", (GtkSignalFunc)
   	auto_reset_selected_cb,oa);

   label = gtk_check_button_new_with_label (_("enabled"));
   gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(label),oa->p_enabled);
   gtk_signal_connect (GTK_OBJECT(label), "clicked", (GtkSignalFunc)
   	enabled_selected_cb,oa);

   frame2 = gtk_frame_new (_("Digits number"));
   gtk_box_pack_start (GTK_BOX (vbox), frame2, FALSE, FALSE, 0);
   gtk_container_set_border_width (GTK_CONTAINER (frame2), 8);

   adj = (GtkAdjustment *) gtk_adjustment_new ((gdouble)oa->digits_nb, 1.0, 10.0, 1.0, 1.0, 0.0);
   oa->spinner = gtk_spin_button_new (adj,1.0,0);
   gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (oa->spinner),TRUE);
   gtk_spin_button_set_shadow_type (GTK_SPIN_BUTTON (oa->spinner), GTK_SHADOW_IN);
   gtk_container_add (GTK_CONTAINER (frame2), oa->spinner);
   gtk_signal_connect (GTK_OBJECT (adj),"value_changed",
   	GTK_SIGNAL_FUNC (digits_number_changed_cb),oa);

   label = gtk_check_button_new_with_label (_("Scale size to panel"));
   gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(label),oa->p_scale_applet);
   gtk_signal_connect (GTK_OBJECT(label), "clicked", (GtkSignalFunc)
   	scale_applet_cb, oa);

   gtk_widget_show(frame);

   label = gtk_label_new(_("General"));
   gnome_property_box_append_page(GNOME_PROPERTY_BOX(oa->pbox),frame,label);

   /* Theme Tab */

   frame = gtk_frame_new(NULL);
   gtk_container_set_border_width (GTK_CONTAINER (frame), GNOME_PAD_SMALL);

   vbox = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
   gtk_container_set_border_width (GTK_CONTAINER (vbox), GNOME_PAD_SMALL);
   gtk_container_add(GTK_CONTAINER(frame), vbox);

   hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
   gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

   label = gtk_label_new(_("Theme file:"));
   gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

   oa->theme_entry = gtk_entry_new();
   if (oa->theme_file)
   	gtk_entry_set_text(GTK_ENTRY(oa->theme_entry), oa->theme_file);
   gtk_signal_connect_object(GTK_OBJECT(oa->theme_entry), "changed",
   	GTK_SIGNAL_FUNC(gnome_property_box_changed),
   	GTK_OBJECT(oa->pbox));
   gtk_box_pack_start(GTK_BOX(hbox),oa->theme_entry , TRUE, TRUE, 0);
   gtk_widget_show(oa->theme_entry);

   scrolled = gtk_scrolled_window_new(NULL, NULL);
   gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
   	GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
   gtk_box_pack_start(GTK_BOX(vbox), scrolled, TRUE, TRUE, 0);

   /* theme list */
   theme_title[0] = _(theme_title[0]);
   theme_clist=gtk_clist_new_with_titles (1, theme_title);
   gtk_clist_column_titles_passive (GTK_CLIST (theme_clist));
   gtk_signal_connect (GTK_OBJECT (theme_clist), "select_row",
   	GTK_SIGNAL_FUNC(theme_selected_cb), oa);
   gtk_container_add (GTK_CONTAINER (scrolled), theme_clist);

   populate_theme_list(theme_clist);

   label = gtk_label_new(_("Theme"));
   gtk_widget_show(frame);
   gnome_property_box_append_page( GNOME_PROPERTY_BOX(oa->pbox),frame,label);

   gtk_signal_connect (GTK_OBJECT (oa->pbox),
   	"apply", GTK_SIGNAL_FUNC (properties_apply_cb), oa);
   gtk_signal_connect (GTK_OBJECT (oa->pbox),
   	"destroy", GTK_SIGNAL_FUNC (properties_destroy_cb), oa);
   gtk_signal_connect (GTK_OBJECT (oa->pbox),
	"help", GTK_SIGNAL_FUNC (phelp_cb), NULL);
   gtk_widget_show_all(oa->pbox);
   return;
   applet = NULL;
}
