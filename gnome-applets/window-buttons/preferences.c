/*
 * Copyright (C) 2009 Andrej Belcijan
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *     Andrej Belcijan <{andrejx} at {gmail.com}>
 */

#include "preferences.h"

void loadThemeComboBox(GtkComboBox *, gchar *);
void loadThemeButtons(GtkWidget ***, GdkPixbuf ***, gchar ***);
void toggleCompizDecoration(gboolean);
void reloadButtons(WBApplet *);
const gchar* getButtonImageState(int, const gchar*);
const gchar* getButtonImageState4(int);
const gchar* getButtonImageName(int);
GtkWidget ***getImageButtons(GtkBuilder *);
GdkPixbuf ***getPixbufs(gchar ***);
gchar ***getImages(gchar *);
gchar *getMetacityLayout (void);
gshort *getEBPos(gchar *);
WBPreferences *loadPreferences(WBApplet *);
gboolean issetCompizDecoration(void);

static const gchar *
getCheckBoxCfgKey (gushort checkbox_id)
{
  switch (checkbox_id)
    {
      case 0:
        return CFG_MINIMIZE_HIDDEN;
      case 1:
        return CFG_UNMAXIMIZE_HIDDEN;
      case 2:
        return CFG_CLOSE_HIDDEN;
      default:
        return NULL;
    }
}

static const gchar *
getImageCfgKey (gushort image_state,
                gushort image_index)
{
  return g_strconcat ("btn-", getButtonImageState(image_state,"-"),
                      "-", getButtonImageName(image_index), NULL);
}

static void
savePreferences (WBPreferences *wbp,
                 WBApplet      *wbapplet)
{
	gint i, j;

	for (i=0; i<WB_BUTTONS; i++) {
		g_settings_set_boolean (wbapplet->settings, getCheckBoxCfgKey(i), wbp->button_hidden[i]);
	}
	for (i=0; i<WB_IMAGE_STATES; i++) {
		for (j=0; j<WB_IMAGES; j++) {
			g_settings_set_string (wbapplet->settings, getImageCfgKey(i,j), wbp->images[i][j]);
		}
	}
	g_settings_set_boolean (wbapplet->settings, CFG_ONLY_MAXIMIZED, wbp->only_maximized);
	g_settings_set_boolean (wbapplet->settings, CFG_CLICK_EFFECT, wbp->click_effect);
	g_settings_set_boolean (wbapplet->settings, CFG_HOVER_EFFECT, wbp->hover_effect);
	g_settings_set_boolean (wbapplet->settings, CFG_HIDE_ON_UNMAXIMIZED, wbp->hide_on_unmaximized);
	g_settings_set_boolean (wbapplet->settings, CFG_USE_METACITY_LAYOUT, wbp->use_metacity_layout);
	g_settings_set_boolean (wbapplet->settings, CFG_REVERSE_ORDER, wbp->reverse_order);
	g_settings_set_boolean (wbapplet->settings, CFG_SHOW_TOOLTIPS, wbp->show_tooltips);
	g_settings_set_int (wbapplet->settings, CFG_ORIENTATION, wbp->orientation);
	g_settings_set_string (wbapplet->settings, CFG_THEME, wbp->theme);
	if (!wbp->use_metacity_layout) {
		// save only when we're using a custom layout
		g_settings_set_string (wbapplet->settings, CFG_BUTTON_LAYOUT, wbp->button_layout);
	}
}

/* Get our properties (the only properties getter that should be called) */
WBPreferences *
loadPreferences (WBApplet *wbapplet)
{
	WBPreferences *wbp = g_new0(WBPreferences, 1);
	gint i;
	gint j;

	wbp->button_hidden = g_new(gboolean, WB_BUTTONS);
	wbp->images = g_new(gchar**, WB_IMAGE_STATES);
	for (i=0; i<WB_IMAGE_STATES; i++) {
		wbp->images[i] = g_new(gchar*, WB_IMAGES);
	}

	for (i=0; i<WB_BUTTONS; i++) {
		wbp->button_hidden[i] = g_settings_get_boolean(wbapplet->settings, getCheckBoxCfgKey(i));
	}

	for (i=0; i<WB_IMAGE_STATES; i++) {
		for (j=0; j<WB_IMAGES; j++) {
			wbp->images[i][j] = g_settings_get_string(wbapplet->settings, getImageCfgKey(i,j));
		}
	}

	wbp->only_maximized = g_settings_get_boolean(wbapplet->settings, CFG_ONLY_MAXIMIZED);
	wbp->hide_on_unmaximized = g_settings_get_boolean(wbapplet->settings, CFG_HIDE_ON_UNMAXIMIZED);
	wbp->click_effect = g_settings_get_boolean(wbapplet->settings, CFG_CLICK_EFFECT);
	wbp->hover_effect = g_settings_get_boolean(wbapplet->settings, CFG_HOVER_EFFECT);
	wbp->use_metacity_layout = g_settings_get_boolean(wbapplet->settings, CFG_USE_METACITY_LAYOUT);
	wbp->reverse_order = g_settings_get_boolean(wbapplet->settings, CFG_REVERSE_ORDER);
	wbp->show_tooltips = g_settings_get_boolean(wbapplet->settings, CFG_SHOW_TOOLTIPS);
	wbp->orientation = g_settings_get_int(wbapplet->settings, CFG_ORIENTATION);
	wbp->theme = g_settings_get_string(wbapplet->settings, CFG_THEME);

	// read positions from GSettings
	if (wbp->use_metacity_layout) {
		wbp->button_layout = getMetacityLayout();
	} else {
		wbp->button_layout = g_settings_get_string(wbapplet->settings, CFG_BUTTON_LAYOUT);
	}

	wbp->eventboxposition = getEBPos(wbp->button_layout);

	return wbp;
}

/* Parses Metacity's GSettings entry to get the button order */
gshort *getEBPos(gchar *button_layout) {
	gshort *ebps = g_new(gshort, WB_BUTTONS);
	gint i, j;
	gchar **pch;

	// in case we got a faulty button_layout:
	for (i=0; i<WB_BUTTONS; i++) ebps[i] = i;
		if (button_layout == NULL || *button_layout == '\0')
			return ebps;

	pch = g_strsplit_set(button_layout, ":, ", -1);
	i = 0; j = 0;
	while (pch[j]) {
		if (!g_strcmp0(pch[j], "minimize")) ebps[0] = i++;
		if (!g_strcmp0(pch[j], "maximize")) ebps[1] = i++;
		if (!g_strcmp0(pch[j], "close"))	ebps[2] = i++;
		j++;
	}

	g_strfreev(pch);
	return ebps;
}

/* Returns a 2D array of GtkWidget image buttons */
GtkWidget ***getImageButtons(GtkBuilder *prefbuilder) {
	gint i,j;
	GtkWidget ***btn = g_new(GtkWidget**, WB_IMAGE_STATES);
	for (i=0; i<WB_IMAGE_STATES; i++) {
		btn[i] = g_new(GtkWidget*, WB_IMAGES);
		for (j=0; j<WB_IMAGES; j++) {
			btn[i][j] = GTK_WIDGET (gtk_builder_get_object(prefbuilder, getImageCfgKey(i,j)));
		}
	}
	return btn;
}

static GtkToggleButton **
getHideButtons (GtkBuilder *prefbuilder)
{
	GtkToggleButton **chkb_btn_hidden = g_new(GtkToggleButton*, WB_BUTTONS);
	chkb_btn_hidden[0] = GTK_TOGGLE_BUTTON (gtk_builder_get_object(prefbuilder, "cb_btn0_visible")),
	chkb_btn_hidden[1] = GTK_TOGGLE_BUTTON (gtk_builder_get_object(prefbuilder, "cb_btn1_visible")),
	chkb_btn_hidden[2] = GTK_TOGGLE_BUTTON (gtk_builder_get_object(prefbuilder, "cb_btn2_visible"));
	return chkb_btn_hidden;
}

static GtkRadioButton **
getOrientationButtons (GtkBuilder *prefbuilder)
{
	GtkRadioButton **radio_orientation = g_new(GtkRadioButton*, 3);
	radio_orientation[0] = GTK_RADIO_BUTTON (gtk_builder_get_object(prefbuilder, "orientation_automatic")),
	radio_orientation[1] = GTK_RADIO_BUTTON (gtk_builder_get_object(prefbuilder, "orientation_horizontal")),
	radio_orientation[2] = GTK_RADIO_BUTTON (gtk_builder_get_object(prefbuilder, "orientation_vertical"));
	return radio_orientation;
}

static void
select_new_image (GtkButton *object,
                  gpointer   user_data)
{
	GtkWidget *fileopendialog;
	ImageOpenData *iod = (ImageOpenData*)user_data;
	WBApplet *wbapplet = iod->wbapplet;
	fileopendialog = gtk_file_chooser_dialog_new
					(
	    				"Select New Image",
						GTK_WINDOW (wbapplet->window_prefs),
						GTK_FILE_CHOOSER_ACTION_OPEN,
						GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
						GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
						NULL
	    			);
	if (gtk_dialog_run (GTK_DIALOG (fileopendialog)) == GTK_RESPONSE_ACCEPT) {
		gchar *filename;
		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (fileopendialog));

		// save new value to memory
		wbapplet->prefs->images[iod->image_state][iod->image_index] = filename;

		//wbapplet->prefs->theme = "custom"; //TODO: change combo box
		//GtkComboBox *combo_theme = GTK_COMBO_BOX (gtk_builder_get_object(wbapplet->prefbuilder, "combo_theme"));
		//GtkListStore *store = gtk_combo_box_get_model(combo_theme);
		/*
		GtkTreeIter iter;
		gboolean valid = gtk_tree_model_get_iter_first (GTK_TREE_MODEL(store), &iter);
		while( valid ){
			// do stuff as
			gtk_tree_model_get (GTK_TREE_MODEL(store), &iter, ... )
			//
			valid = gtk_tree_model_iter_next (GTK_TREE_MODEL(store), &iter);
		}
		 */

		wbapplet->pixbufs = getPixbufs(wbapplet->prefs->images); // reload pixbufs from files
		loadThemeButtons(getImageButtons(wbapplet->prefbuilder), wbapplet->pixbufs, wbapplet->prefs->images); // set pref button images from pixbufs
		wb_applet_update_images(wbapplet); // reload images

		savePreferences(wbapplet->prefs, wbapplet);
	}
	gtk_widget_destroy (fileopendialog);
}

static void
cb_only_maximized (GtkButton *button,
                   WBApplet  *wbapplet)
{
	wbapplet->prefs->only_maximized = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(button));
	savePreferences(wbapplet->prefs, wbapplet);
}

static void
cb_click_effect (GtkButton *button,
                 WBApplet  *wbapplet)
{
	wbapplet->prefs->click_effect = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(button));
	savePreferences(wbapplet->prefs, wbapplet);
}

static void
cb_hover_effect (GtkButton *button,
                 WBApplet  *wbapplet)
{
	wbapplet->prefs->hover_effect = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(button));
	savePreferences(wbapplet->prefs, wbapplet);
}

static void
cb_reverse_order (GtkButton *button,
                  WBApplet  *wbapplet)
{
	wbapplet->prefs->reverse_order = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(button));
	reloadButtons(wbapplet);
	savePreferences(wbapplet->prefs, wbapplet);
}

static void
cb_hide_on_unmaximized (GtkButton *button,
                        WBApplet  *wbapplet)
{
	wbapplet->prefs->hide_on_unmaximized = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(button));
	wb_applet_update_images(wbapplet);
	savePreferences(wbapplet->prefs, wbapplet);
}

static void
cb_hide_decoration (GtkButton *button,
                    WBApplet  *wbapplet)
{
	toggleCompizDecoration(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)));
}

static void
cb_show_tooltips (GtkButton *button,
                  WBApplet  *wbapplet)
{
	gint i;

	wbapplet->prefs->show_tooltips = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(button));

	for (i=0; i<WB_BUTTONS; i++)
		gtk_widget_set_has_tooltip (GTK_WIDGET(wbapplet->button[i]->image), wbapplet->prefs->show_tooltips);
	savePreferences(wbapplet->prefs, wbapplet);
}

static void
cb_btn_hidden (GtkButton *button,
               gpointer   user_data)
{
	CheckBoxData *cbd = (CheckBoxData*)user_data;
	WBApplet *wbapplet = cbd->wbapplet;

	if (wbapplet->prefs->button_hidden[cbd->button_id]) {
		wbapplet->prefs->button_hidden[cbd->button_id] = 0;
	} else {
		wbapplet->prefs->button_hidden[cbd->button_id] = 1;
	}

	wb_applet_update_images(wbapplet);
	savePreferences(wbapplet->prefs, wbapplet);
}

static void
cb_metacity_layout (GtkButton *button,
                    WBApplet  *wbapplet)
{
	GtkEntry *entry_custom_layout = GTK_ENTRY (gtk_builder_get_object(wbapplet->prefbuilder, CFG_BUTTON_LAYOUT));

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(button))) {
		wbapplet->prefs->use_metacity_layout = TRUE;
		wbapplet->prefs->button_layout = getMetacityLayout();
		gtk_widget_set_sensitive(GTK_WIDGET(entry_custom_layout), FALSE);
	} else {
		gchar *new_layout;

		gtk_widget_set_sensitive(GTK_WIDGET(entry_custom_layout), TRUE);
		wbapplet->prefs->use_metacity_layout = FALSE;
		new_layout = g_strdup(gtk_entry_get_text(entry_custom_layout));
		wbapplet->prefs->button_layout = new_layout;
	}

	savePreferences(wbapplet->prefs, wbapplet);

	wbapplet->prefs->eventboxposition = getEBPos(wbapplet->prefs->button_layout);
	reloadButtons (wbapplet);
}

static void
cb_reload_buttons (GtkButton *button,
                   WBApplet  *wbapplet)
{
	GtkEntry *entry_custom_layout = GTK_ENTRY (gtk_builder_get_object(wbapplet->prefbuilder, CFG_BUTTON_LAYOUT));
	gchar *new_layout = g_strdup(gtk_entry_get_text(entry_custom_layout));
	wbapplet->prefs->button_layout = new_layout;
	savePreferences(wbapplet->prefs, wbapplet);
	wbapplet->prefs->eventboxposition = getEBPos(wbapplet->prefs->button_layout);
	reloadButtons(wbapplet);
}

static void
cb_theme_changed (GtkComboBox *combo,
                  WBApplet    *wbapplet)
{
	WBPreferences *wbp = wbapplet->prefs;
	GtkTreeIter   iter;
    gchar        *theme = NULL;
    GtkTreeModel *model;

    if (gtk_combo_box_get_active_iter( combo, &iter )) {
        model = gtk_combo_box_get_model( combo );
        gtk_tree_model_get( model, &iter, 0, &theme, -1 );
    }

	wbp->theme = theme;
	wbp->images = getImages(g_strconcat(PATH_THEMES,"/",wbp->theme,"/",NULL)); // rebuild image paths
	wbapplet->pixbufs = getPixbufs(wbp->images); // reload pixbufs from files
	loadThemeButtons(getImageButtons(wbapplet->prefbuilder), wbapplet->pixbufs, wbp->images); // set pref button images from pixbufs
	wb_applet_update_images(wbapplet);

	savePreferences(wbp, wbapplet);
}

static void
cb_orientation (GtkRadioButton *style,
                WBApplet       *wbapplet)
{
	// This thing gets executed twice with every selection
	// (Supposedly this is going to be resolved with GtkRadioGroup in later versions of GTK+)
	// So we need to eliminate the "untoggle" event
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(style))) {
		GtkRadioButton **radio_orientation = getOrientationButtons(wbapplet->prefbuilder);
		gushort i;
		for (i=0;i<3;i++)
			if (style == radio_orientation[i])
				break;
		if (i<3)
			wbapplet->prefs->orientation = i;

		wbapplet->pixbufs = getPixbufs(wbapplet->prefs->images); // reload pixbufs before rotating!
		reloadButtons(wbapplet); // reload and rotate buttons
		loadThemeButtons(getImageButtons(wbapplet->prefbuilder), wbapplet->pixbufs, wbapplet->prefs->images); // set pref button images from pixbufs
		wb_applet_update_images(wbapplet);
		savePreferences(wbapplet->prefs, wbapplet);

		g_free (radio_orientation);
	}
}

static void
properties_close (GtkButton *object,
                  WBApplet  *wbapplet)
{
  gtk_widget_destroy(wbapplet->window_prefs);
  wbapplet->window_prefs = NULL;
}

static void
free_image_open_data (gpointer  data,
                      GClosure *closure)
{
  g_free (data);
}

static void
free_check_box_data (gpointer  data,
                     GClosure *closure)
{
  g_free (data);
}

void
wb_applet_properties_cb (GSimpleAction *action,
                         GVariant      *parameter,
                         gpointer       user_data)
{
	WBApplet *wbapplet;
	GtkWidget		***btn;
	gint i,j;
	GtkToggleButton *chkb_only_maximized;
	GtkToggleButton *chkb_click_effect;
	GtkToggleButton *chkb_hover_effect;
	GtkToggleButton *chkb_hide_on_unmaximized;
	GtkToggleButton *chkb_reverse_order;
	GtkToggleButton *chkb_hide_decoration;
	GtkToggleButton *chkb_metacity_order;
	GtkToggleButton *chkb_show_tooltips;
	GtkButton *btn_reload_order;
	GtkButton *btn_close;
	GtkEntry *entry_custom_order;
	GtkComboBox *combo_theme;
	GtkToggleButton **chkb_btn_hidden;
	GtkRadioButton **radio_orientation;

	wbapplet = (WBApplet *) user_data;

	// Create the Properties dialog from the GtkBuilder file
	if (wbapplet->window_prefs) {
		// Window already exists, only open
		gtk_window_present(GTK_WINDOW(wbapplet->window_prefs)); // CRASHES HERE BECAUSE window_prefs IS NOT NULL WHEN IT SHOULD BE!!!
	} else {
		// Create window from builder
		gtk_builder_set_translation_domain (wbapplet->prefbuilder, GETTEXT_PACKAGE);
		gtk_builder_add_from_resource (wbapplet->prefbuilder, GRESOURCE_PREFIX "/ui/window-buttons.ui", NULL);
		wbapplet->window_prefs = GTK_WIDGET (gtk_builder_get_object (wbapplet->prefbuilder, "properties"));
	}

	/* Get the widgets from GtkBuilder & Init data structures we'll pass to our buttons */
	btn = getImageButtons(wbapplet->prefbuilder);

	for (i=0; i<WB_IMAGE_STATES; i++) {
		for (j=0; j<WB_IMAGES; j++) {
			ImageOpenData *iod;

			iod = g_new0 (ImageOpenData, 1);
			iod->wbapplet = wbapplet;
			iod->image_state = i;
			iod->image_index = j;

			// Connect buttons to select_new_image callback
			g_signal_connect_data (btn[i][j],
			                       "clicked",
			                       G_CALLBACK (select_new_image),
			                       iod,
			                       free_image_open_data,
			                       0);
		}
	}

	chkb_only_maximized = GTK_TOGGLE_BUTTON (gtk_builder_get_object(wbapplet->prefbuilder, CFG_ONLY_MAXIMIZED));
	chkb_click_effect = GTK_TOGGLE_BUTTON (gtk_builder_get_object(wbapplet->prefbuilder, CFG_CLICK_EFFECT));
	chkb_hover_effect = GTK_TOGGLE_BUTTON (gtk_builder_get_object(wbapplet->prefbuilder, CFG_HOVER_EFFECT));
	chkb_hide_on_unmaximized = GTK_TOGGLE_BUTTON (gtk_builder_get_object(wbapplet->prefbuilder, CFG_HIDE_ON_UNMAXIMIZED));
	chkb_reverse_order = GTK_TOGGLE_BUTTON (gtk_builder_get_object(wbapplet->prefbuilder, CFG_REVERSE_ORDER));
	chkb_hide_decoration = GTK_TOGGLE_BUTTON (gtk_builder_get_object(wbapplet->prefbuilder, CFG_HIDE_DECORATION));
	chkb_metacity_order = GTK_TOGGLE_BUTTON (gtk_builder_get_object(wbapplet->prefbuilder, CFG_USE_METACITY_LAYOUT));
	chkb_show_tooltips = GTK_TOGGLE_BUTTON (gtk_builder_get_object(wbapplet->prefbuilder, CFG_SHOW_TOOLTIPS));
	btn_reload_order = GTK_BUTTON (gtk_builder_get_object(wbapplet->prefbuilder, "btn_reload_order"));
	btn_close = GTK_BUTTON (gtk_builder_get_object(wbapplet->prefbuilder, "btn_close"));
	entry_custom_order = GTK_ENTRY (gtk_builder_get_object(wbapplet->prefbuilder, CFG_BUTTON_LAYOUT));
	combo_theme = GTK_COMBO_BOX (gtk_builder_get_object(wbapplet->prefbuilder, CFG_THEME));
	chkb_btn_hidden = getHideButtons(wbapplet->prefbuilder);
	radio_orientation = getOrientationButtons(wbapplet->prefbuilder);

	loadThemeComboBox(combo_theme, wbapplet->prefs->theme);
	loadThemeButtons(btn, wbapplet->pixbufs, wbapplet->prefs->images);

	// set the checkboxes according to preferences
	gtk_widget_set_sensitive(GTK_WIDGET(entry_custom_order), !wbapplet->prefs->use_metacity_layout);
	gtk_toggle_button_set_active (chkb_only_maximized, wbapplet->prefs->only_maximized);
	gtk_toggle_button_set_active (chkb_click_effect, wbapplet->prefs->click_effect);
	gtk_toggle_button_set_active (chkb_hover_effect, wbapplet->prefs->hover_effect);
	gtk_toggle_button_set_active (chkb_hide_on_unmaximized, wbapplet->prefs->hide_on_unmaximized);
	gtk_toggle_button_set_active (chkb_hide_decoration, issetCompizDecoration());
	gtk_toggle_button_set_active (chkb_metacity_order, wbapplet->prefs->use_metacity_layout);
	gtk_toggle_button_set_active (chkb_reverse_order, wbapplet->prefs->reverse_order);
	gtk_toggle_button_set_active (chkb_show_tooltips, wbapplet->prefs->show_tooltips);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(radio_orientation[wbapplet->prefs->orientation]), TRUE);
	gtk_entry_set_text (entry_custom_order, (const gchar*)wbapplet->prefs->button_layout);

	for (i=0; i<WB_BUTTONS; i++) {
		CheckBoxData *cbd;

		cbd = g_new(CheckBoxData,1);
		cbd->button_id = i;
		cbd->wbapplet = wbapplet;

		gtk_toggle_button_set_active (chkb_btn_hidden[i], wbapplet->prefs->button_hidden[i]);
		g_signal_connect_data (chkb_btn_hidden[i],
		                       "clicked",
		                       G_CALLBACK (cb_btn_hidden),
		                       cbd,
		                       free_check_box_data,
		                       0);
	}

	for (i=0; i<3; i++)
		g_signal_connect(G_OBJECT(radio_orientation[i]), "clicked", G_CALLBACK (cb_orientation), wbapplet);
	g_signal_connect(G_OBJECT(chkb_only_maximized), "clicked", G_CALLBACK (cb_only_maximized), wbapplet);
	g_signal_connect(G_OBJECT(chkb_click_effect), "clicked", G_CALLBACK (cb_click_effect), wbapplet);
	g_signal_connect(G_OBJECT(chkb_hover_effect), "clicked", G_CALLBACK (cb_hover_effect), wbapplet);
	g_signal_connect(G_OBJECT(chkb_hide_on_unmaximized), "clicked", G_CALLBACK (cb_hide_on_unmaximized), wbapplet);
	g_signal_connect(G_OBJECT(chkb_hide_decoration), "clicked", G_CALLBACK (cb_hide_decoration), wbapplet);
	g_signal_connect(G_OBJECT(chkb_metacity_order), "clicked", G_CALLBACK (cb_metacity_layout), wbapplet);
	g_signal_connect(G_OBJECT(chkb_reverse_order), "clicked", G_CALLBACK (cb_reverse_order), wbapplet);
	g_signal_connect(G_OBJECT(chkb_show_tooltips), "clicked", G_CALLBACK (cb_show_tooltips), wbapplet);
	g_signal_connect(G_OBJECT(btn_reload_order), "clicked", G_CALLBACK (cb_reload_buttons), wbapplet);
	g_signal_connect(G_OBJECT(combo_theme), "changed", G_CALLBACK(cb_theme_changed), wbapplet);
	g_signal_connect(G_OBJECT(btn_close), "clicked", G_CALLBACK (properties_close), wbapplet);
	g_signal_connect(G_OBJECT(wbapplet->window_prefs), "destroy", G_CALLBACK(properties_close), wbapplet);

	gtk_widget_show (wbapplet->window_prefs);

	g_free (chkb_btn_hidden);
	g_free (radio_orientation);
}
