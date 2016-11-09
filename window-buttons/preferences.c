/***************************************************************************
 *            preferences.c
 *
 *  Mon May  4 01:23:08 2009
 *  Copyright  2009  Andrej Belcijan
 *  <{andrejx} at {gmail.com}>
 ****************************************************************************/

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */

#include "preferences.h"

/* prototypes */
void cb_only_maximized (GtkButton *, WBApplet *);
void cb_click_effect (GtkButton *, WBApplet *);
void cb_hover_effect (GtkButton *, WBApplet *);
void cb_hide_on_unmaximized (GtkButton *, WBApplet *);
void cb_hide_decoration (GtkButton *, WBApplet *);
void cb_btn_minimize_hidden (GtkButton *, WBApplet *);
void cb_btn_maximize_hidden (GtkButton *, WBApplet *);
void cb_btn_close_hidden (GtkButton *, WBApplet *);
void properties_close (GtkButton *, WBApplet *);
void updateImages(WBApplet *);
void savePreferences(WBPreferences *, WBApplet *);
void loadThemeComboBox(GtkComboBox *, gchar *);
void loadThemeButtons(GtkWidget ***, GdkPixbuf ***, gchar ***);
void toggleCompizDecoration(gboolean);
void reloadButtons(WBApplet *);
const gchar *getImageCfgKey (gushort, gushort);
const gchar *getCheckBoxCfgKey (gushort);
const gchar *getImageCfgKey(gushort, gushort);
const gchar *getImageCfgKey4(gushort, gushort);
const gchar* getButtonImageState(int, const gchar*);
const gchar* getButtonImageState4(int);
const gchar* getButtonImageName(int);
GtkRadioButton **getOrientationButtons(GtkBuilder *);
GtkToggleButton **getHideButtons(GtkBuilder *);
GtkWidget ***getImageButtons(GtkBuilder *);
GdkPixbuf ***getPixbufs(gchar ***);
gchar ***getImages(gchar *);
gchar *getMetacityLayout (void);
gchar *getCfgValue(FILE *, gchar *);
gchar *fixThemeName(gchar *);
gshort *getEBPos(gchar *);
WBPreferences *loadPreferences(WBApplet *);
gboolean issetCompizDecoration(void);

void savePreferences(WBPreferences *wbp, WBApplet *wbapplet) {
#if PLAINTEXT_CONFIG == 0
	gint i, j;

	for (i=0; i<WB_BUTTONS; i++) {
		//panel_applet_gconf_set_bool (wbapplet->applet, getCheckBoxCfgKey(i), (wbapplet->button[i]->state & WB_BUTTON_STATE_HIDDEN), NULL);
		panel_applet_gconf_set_bool (wbapplet->applet, getCheckBoxCfgKey(i), wbp->button_hidden[i], NULL);
	}
	for (i=0; i<WB_IMAGE_STATES; i++) {
		for (j=0; j<WB_IMAGES; j++) {
			panel_applet_gconf_set_string (wbapplet->applet, getImageCfgKey(i,j), wbp->images[i][j], NULL);
		}
	}
	panel_applet_gconf_set_bool (wbapplet->applet, CFG_ONLY_MAXIMIZED, wbp->only_maximized, NULL);
	panel_applet_gconf_set_bool (wbapplet->applet, CFG_CLICK_EFFECT, wbp->click_effect, NULL);
	panel_applet_gconf_set_bool (wbapplet->applet, CFG_HOVER_EFFECT, wbp->hover_effect, NULL);
	panel_applet_gconf_set_bool (wbapplet->applet, CFG_HIDE_ON_UNMAXIMIZED, wbp->hide_on_unmaximized, NULL);
	panel_applet_gconf_set_bool (wbapplet->applet, CFG_USE_METACITY_LAYOUT, wbp->use_metacity_layout, NULL);
	panel_applet_gconf_set_bool (wbapplet->applet, CFG_REVERSE_ORDER, wbp->reverse_order, NULL);
	panel_applet_gconf_set_bool (wbapplet->applet, CFG_SHOW_TOOLTIPS, wbp->show_tooltips, NULL);
	panel_applet_gconf_set_int (wbapplet->applet, CFG_ORIENTATION, wbp->orientation, NULL);
	panel_applet_gconf_set_string (wbapplet->applet, CFG_THEME, wbp->theme, NULL);
	if (!wbp->use_metacity_layout) {
		// save only when we're using a custom layout
		panel_applet_gconf_set_string (wbapplet->applet, CFG_BUTTON_LAYOUT, wbp->button_layout, NULL);
	}
#else
	FILE *cfg = g_fopen (g_strconcat(g_get_home_dir(),"/",FILE_CONFIGFILE,NULL),"w");
	gint i, j;
	
	for (i=0; i<WB_BUTTONS; i++) {
		fprintf(cfg, "%s %d\n", getCheckBoxCfgKey(i), wbp->button_hidden[i]);
	}
	for (i=0; i<WB_IMAGE_STATES; i++) {
		for (j=0; j<WB_IMAGES; j++) {
			fprintf(cfg, "%s %s\n", getImageCfgKey(i,j), wbp->images[i][j]);
		}
	}
	fprintf(cfg, "%s %d\n", CFG_ONLY_MAXIMIZED, wbp->only_maximized);
	fprintf(cfg, "%s %d\n", CFG_CLICK_EFFECT, wbp->click_effect);
	fprintf(cfg, "%s %d\n", CFG_HOVER_EFFECT, wbp->hover_effect);
	fprintf(cfg, "%s %d\n", CFG_HIDE_ON_UNMAXIMIZED, wbp->hide_on_unmaximized);
	fprintf(cfg, "%s %d\n", CFG_USE_METACITY_LAYOUT, wbp->use_metacity_layout);
	fprintf(cfg, "%s %d\n", CFG_REVERSE_ORDER, wbp->reverse_order);
	fprintf(cfg, "%s %d\n", CFG_SHOW_TOOLTIPS, wbp->show_tooltips);
	fprintf(cfg, "%s %d\n", CFG_ORIENTATION, wbp->orientation);
	fprintf(cfg, "%s %s\n", CFG_THEME, wbp->theme);
	if (!wbp->use_metacity_layout) {
		fprintf(cfg, "%s %s\n", CFG_BUTTON_LAYOUT, wbp->button_layout);
	}
	
	fclose (cfg);
#endif
}

/* Get our properties (the only properties getter that should be called) */
WBPreferences *loadPreferences(WBApplet *wbapplet) {
	WBPreferences *wbp = g_new0(WBPreferences, 1);
	gint i;

	wbp->button_hidden = g_new(gboolean, WB_BUTTONS);
	wbp->images = g_new(gchar**, WB_IMAGE_STATES);
	for (i=0; i<WB_IMAGE_STATES; i++) {
		wbp->images[i] = g_new(gchar*, WB_IMAGES);
	}

#if PLAINTEXT_CONFIG == 0
	//gint i, j;
	gint j;

	for (i=0; i<WB_BUTTONS; i++) {
		wbp->button_hidden[i] = panel_applet_gconf_get_bool(wbapplet->applet, getCheckBoxCfgKey(i), NULL);
	}
	
	for (i=0; i<WB_IMAGE_STATES; i++) {
		for (j=0; j<WB_IMAGES; j++) {
			wbp->images[i][j] = panel_applet_gconf_get_string(wbapplet->applet, getImageCfgKey(i,j), NULL);
			if (!g_file_test(wbp->images[i][j], G_FILE_TEST_EXISTS | ~G_FILE_TEST_IS_DIR)) { // this is only good for upgrading where old gconf keys still exist
				wbp->images[i][j] = panel_applet_gconf_get_string(wbapplet->applet, getImageCfgKey4(i,j), NULL);
			}
		}
	}

	wbp->only_maximized = panel_applet_gconf_get_bool(wbapplet->applet, CFG_ONLY_MAXIMIZED, NULL);
	wbp->hide_on_unmaximized = panel_applet_gconf_get_bool(wbapplet->applet, CFG_HIDE_ON_UNMAXIMIZED, NULL);
	wbp->click_effect = panel_applet_gconf_get_bool(wbapplet->applet, CFG_CLICK_EFFECT, NULL);
	wbp->hover_effect = panel_applet_gconf_get_bool(wbapplet->applet, CFG_HOVER_EFFECT, NULL);
	wbp->use_metacity_layout = panel_applet_gconf_get_bool(wbapplet->applet, CFG_USE_METACITY_LAYOUT, NULL);
	wbp->reverse_order = panel_applet_gconf_get_bool(wbapplet->applet, CFG_REVERSE_ORDER, NULL);
	wbp->show_tooltips = panel_applet_gconf_get_bool(wbapplet->applet, CFG_SHOW_TOOLTIPS, NULL);
	wbp->orientation = panel_applet_gconf_get_int(wbapplet->applet, CFG_ORIENTATION, NULL);
	wbp->theme = panel_applet_gconf_get_string(wbapplet->applet, CFG_THEME, NULL);

	// read positions from GConf
	if (wbp->use_metacity_layout) {
		wbp->button_layout = getMetacityLayout();
	} else {
		wbp->button_layout = panel_applet_gconf_get_string(wbapplet->applet, CFG_BUTTON_LAYOUT, NULL);
	}
#else
	FILE *cfg = g_fopen (g_strconcat(g_get_home_dir(),"/",FILE_CONFIGFILE, NULL), "r");
	gint i,j;

	if (cfg) {
		for (i=0; i<WB_BUTTONS; i++) {
			wbp->button_hidden[i] = g_ascii_strtod(getCfgValue(cfg,(gchar*)getCheckBoxCfgKey(i)),NULL);
		}
		for (i=0; i<WB_IMAGE_STATES; i++) {
			for (j=0; j<WB_IMAGES; j++) {
				wbp->images[i][j] = getCfgValue(cfg,(gchar*)getImageCfgKey(i,j));
				if (!g_file_test(wbp->images[i][j], G_FILE_TEST_EXISTS | ~G_FILE_TEST_IS_DIR)) {
					wbp->images[i][j] = getCfgValue(cfg,(gchar*)getImageCfgKey4(i,j));
				}
			}
		}
		wbp->only_maximized = g_ascii_strtod(getCfgValue(cfg,CFG_ONLY_MAXIMIZED),NULL);
		wbp->hide_on_unmaximized = g_ascii_strtod(getCfgValue(cfg,CFG_HIDE_ON_UNMAXIMIZED),NULL);
		wbp->click_effect = g_ascii_strtod(getCfgValue(cfg,CFG_CLICK_EFFECT),NULL);
		wbp->hover_effect = g_ascii_strtod(getCfgValue(cfg,CFG_HOVER_EFFECT),NULL);
		wbp->reverse_order = g_ascii_strtod(getCfgValue(cfg,CFG_REVERSE_ORDER),NULL);
		wbp->show_tooltips = g_ascii_strtod(getCfgValue(cfg,CFG_SHOW_TOOLTIPS),NULL);
		wbp->orientation = g_ascii_strtod(getCfgValue(cfg,CFG_ORIENTATION),NULL);
		wbp->use_metacity_layout = g_ascii_strtod(getCfgValue(cfg,CFG_USE_METACITY_LAYOUT),NULL);
		if (wbp->use_metacity_layout) {
			// wbp->button_layout = getMetacityLayout(); // We're avoiding GConf, so let's not use that
			wbp->button_layout = "menu:minimize,maximize,close";
		} else {
			wbp->button_layout = getCfgValue(cfg,CFG_BUTTON_LAYOUT);
		}
		wbp->theme = getCfgValue(cfg,CFG_THEME);

		fclose (cfg);		
	} else {
		// Defaults if the file doesn't exist

		wbp->only_maximized = TRUE;
		wbp->hide_on_unmaximized = FALSE;
		wbp->click_effect = TRUE;
		wbp->hover_effect = TRUE;
		wbp->use_metacity_layout = TRUE;	
		wbp->reverse_order = FALSE;
		wbp->show_tooltips = FALSE;
		wbp->orientation = 0;
		wbp->button_layout = "menu:minimize,maximize,close";
		wbp->theme = "default";
		for (i=0; i<WB_BUTTONS; i++) {
			wbp->button_hidden[i] = 0;
		}
		for (i=0; i<WB_IMAGE_STATES; i++) {
			for (j=0; j<WB_IMAGES; j++) {
				wbp->images[i][j] = g_strconcat(PATH_THEMES,"/",wbp->theme,"/",getButtonImageName(j),"-",getButtonImageState(i,"-"),".",THEME_EXTENSION,NULL);
				if (!g_file_test(wbp->images[i][j], G_FILE_TEST_EXISTS | ~G_FILE_TEST_IS_DIR)) {
					wbp->images[i][j] = g_strconcat(PATH_THEMES,"/",wbp->theme,"/",getButtonImageName(j),"-",getButtonImageState4(i),".",THEME_EXTENSION,NULL);
				}
			}
		}

		savePreferences(wbp,wbapplet);
	}
#endif

	wbp->eventboxposition = getEBPos(wbp->button_layout);

	return wbp;
}

#if PLAINTEXT_CONFIG != 0
/* Returns a string value of the specified configuration parameter (key) */
gchar* getCfgValue(FILE *f, gchar *key) {
    gchar tmp[256] = {0x0};
	long int pos = ftell(f);
	
    while (f!=NULL && fgets(tmp,sizeof(tmp),f)!=NULL) {
		if (g_strrstr(tmp, key))
		    break;
    }

	gchar *r = g_strndup(tmp+strlen(key)+1,strlen(tmp)-strlen(key)+1);
	g_strstrip(r);

	fseek(f,pos,SEEK_SET);
    return r;
}
#endif

/* Parses Metacity's GConf entry to get the button order */
gshort *getEBPos(gchar *button_layout) {
	gshort *ebps = g_new(gshort, WB_BUTTONS);
	gint i, j;

	// in case we got a faulty button_layout:
	for (i=0; i<WB_BUTTONS; i++) ebps[i] = i;
		if (button_layout == NULL || *button_layout == '\0')
			return ebps;
	
//	for(i=0; i<WB_BUTTONS; i++) ebps[i] = -1; //set to -1 if we don't find some
	gchar **pch = g_strsplit_set(button_layout, ":, ", -1);
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

const gchar* getCheckBoxCfgKey(gushort checkbox_id) {
	switch (checkbox_id) {
		case 0: return CFG_MINIMIZE_HIDDEN;
		case 1: return CFG_UNMAXIMIZE_HIDDEN;
		case 2: return CFG_CLOSE_HIDDEN;
		default: return NULL;
	}
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

GtkToggleButton **getHideButtons(GtkBuilder *prefbuilder) {
	GtkToggleButton **chkb_btn_hidden = g_new(GtkToggleButton*, WB_BUTTONS);
	chkb_btn_hidden[0] = GTK_TOGGLE_BUTTON (gtk_builder_get_object(prefbuilder, "cb_btn0_visible")),
	chkb_btn_hidden[1] = GTK_TOGGLE_BUTTON (gtk_builder_get_object(prefbuilder, "cb_btn1_visible")),
	chkb_btn_hidden[2] = GTK_TOGGLE_BUTTON (gtk_builder_get_object(prefbuilder, "cb_btn2_visible"));
	return chkb_btn_hidden;
}

GtkRadioButton **getOrientationButtons(GtkBuilder *prefbuilder) {
	GtkRadioButton **radio_orientation = g_new(GtkRadioButton*, 3);
	radio_orientation[0] = GTK_RADIO_BUTTON (gtk_builder_get_object(prefbuilder, "orientation_automatic")),
	radio_orientation[1] = GTK_RADIO_BUTTON (gtk_builder_get_object(prefbuilder, "orientation_horizontal")),
	radio_orientation[2] = GTK_RADIO_BUTTON (gtk_builder_get_object(prefbuilder, "orientation_vertical"));
	return radio_orientation;
}

void select_new_image (GtkButton *object, gpointer user_data) {
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
		updateImages(wbapplet);	// reload images

		savePreferences(wbapplet->prefs, wbapplet);
	}
	gtk_widget_destroy (fileopendialog);
}

void cb_only_maximized(GtkButton *button, WBApplet *wbapplet) {
	wbapplet->prefs->only_maximized = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(button));
	savePreferences(wbapplet->prefs, wbapplet);
}

void cb_click_effect(GtkButton *button, WBApplet *wbapplet) {
	wbapplet->prefs->click_effect = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(button));
	savePreferences(wbapplet->prefs, wbapplet);
}

void cb_hover_effect(GtkButton *button, WBApplet *wbapplet) {
	wbapplet->prefs->hover_effect = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(button));
	savePreferences(wbapplet->prefs, wbapplet);
}

void cb_reverse_order(GtkButton *button, WBApplet *wbapplet) {
	wbapplet->prefs->reverse_order = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(button));
	reloadButtons(wbapplet);
	savePreferences(wbapplet->prefs, wbapplet);
}

void cb_hide_on_unmaximized(GtkButton *button, WBApplet *wbapplet) {
	wbapplet->prefs->hide_on_unmaximized = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(button));
	updateImages(wbapplet);
	savePreferences(wbapplet->prefs, wbapplet);
}

void cb_hide_decoration(GtkButton *button, WBApplet *wbapplet) {
	toggleCompizDecoration(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)));
}

void cb_show_tooltips(GtkButton *button, WBApplet *wbapplet) {
	wbapplet->prefs->show_tooltips = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(button));
	gint i;
	for (i=0; i<WB_BUTTONS; i++)
		gtk_widget_set_has_tooltip (GTK_WIDGET(wbapplet->button[i]->image), wbapplet->prefs->show_tooltips);
	savePreferences(wbapplet->prefs, wbapplet);
}

void cb_btn_hidden(GtkButton *button, gpointer user_data) {
	CheckBoxData *cbd = (CheckBoxData*)user_data;
	WBApplet *wbapplet = cbd->wbapplet;
	
	if (wbapplet->prefs->button_hidden[cbd->button_id]) {
		wbapplet->prefs->button_hidden[cbd->button_id] = 0;
	} else {
		wbapplet->prefs->button_hidden[cbd->button_id] = 1;
	}

	updateImages(wbapplet);
	savePreferences(wbapplet->prefs, wbapplet);
}

// "Use Metacity's button order" checkbox
void cb_metacity_layout(GtkButton *button, WBApplet *wbapplet) {
	GtkEntry *entry_custom_layout = GTK_ENTRY (gtk_builder_get_object(wbapplet->prefbuilder, CFG_BUTTON_LAYOUT));
	
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(button))) {
		wbapplet->prefs->use_metacity_layout = TRUE;
		wbapplet->prefs->button_layout = getMetacityLayout();
		gtk_widget_set_sensitive(GTK_WIDGET(entry_custom_layout), FALSE);
	} else {
		gtk_widget_set_sensitive(GTK_WIDGET(entry_custom_layout), TRUE);
		wbapplet->prefs->use_metacity_layout = FALSE;
		gchar *new_layout = g_strdup(gtk_entry_get_text(entry_custom_layout));
		wbapplet->prefs->button_layout = new_layout;
	}

	savePreferences(wbapplet->prefs, wbapplet);
	
	wbapplet->prefs->eventboxposition = getEBPos(wbapplet->prefs->button_layout);
	reloadButtons (wbapplet);
}

// "Reload" button clicked
void cb_reload_buttons(GtkButton *button, WBApplet *wbapplet) { 
	GtkEntry *entry_custom_layout = GTK_ENTRY (gtk_builder_get_object(wbapplet->prefbuilder, CFG_BUTTON_LAYOUT));
	gchar *new_layout = g_strdup(gtk_entry_get_text(entry_custom_layout));
	wbapplet->prefs->button_layout = new_layout;
	savePreferences(wbapplet->prefs, wbapplet);
	wbapplet->prefs->eventboxposition = getEBPos(wbapplet->prefs->button_layout);
	reloadButtons(wbapplet);
}

static void cb_theme_changed(GtkComboBox *combo, WBApplet *wbapplet) {
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
	updateImages(wbapplet);
	
	savePreferences(wbp, wbapplet);
}

void cb_orientation(GtkRadioButton *style, WBApplet *wbapplet) {
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
		updateImages(wbapplet);
		savePreferences(wbapplet->prefs, wbapplet);
	}
}

/* The Preferences Dialog */
//void properties_cb(BonoboUIComponent *uic, WBApplet *wbapplet, const char *verb) {
void properties_cb (GtkAction *action, WBApplet *wbapplet) { 
	GtkWidget		***btn;
	ImageOpenData 	***iod;
	gint i,j;

	// Create the Properties dialog from the GtkBuilder file
	if (wbapplet->window_prefs) {
		// Window already exists, only open
		gtk_window_present(GTK_WINDOW(wbapplet->window_prefs)); // CRASHES HERE BECAUSE window_prefs IS NOT NULL WHEN IT SHOULD BE!!!
	} else {
		// Create window from builder
		gtk_builder_add_from_file (wbapplet->prefbuilder, PATH_UI_PREFS, NULL);
		wbapplet->window_prefs = GTK_WIDGET (gtk_builder_get_object (wbapplet->prefbuilder, "properties"));
	}
	
	/* Get the widgets from GtkBuilder & Init data structures we'll pass to our buttons */
	btn = getImageButtons(wbapplet->prefbuilder);
	iod =  g_new(ImageOpenData**, WB_IMAGE_STATES);
	for (i=0; i<WB_IMAGE_STATES; i++) {
		iod[i] = g_new(ImageOpenData*, WB_IMAGES);
		for (j=0; j<WB_IMAGES; j++) {
			iod[i][j] = g_new0(ImageOpenData, 1);
			iod[i][j]->wbapplet = wbapplet;
			iod[i][j]->image_state = i;
			iod[i][j]->image_index = j;

			// Connect buttons to select_new_image callback
			g_signal_connect(G_OBJECT (btn[i][j]), "clicked", G_CALLBACK (select_new_image), iod[i][j]);
		}
	}
	
	GtkToggleButton
		*chkb_only_maximized = GTK_TOGGLE_BUTTON (gtk_builder_get_object(wbapplet->prefbuilder, CFG_ONLY_MAXIMIZED)),
		*chkb_click_effect = GTK_TOGGLE_BUTTON (gtk_builder_get_object(wbapplet->prefbuilder, CFG_CLICK_EFFECT)),
		*chkb_hover_effect = GTK_TOGGLE_BUTTON (gtk_builder_get_object(wbapplet->prefbuilder, CFG_HOVER_EFFECT)),
		*chkb_hide_on_unmaximized = GTK_TOGGLE_BUTTON (gtk_builder_get_object(wbapplet->prefbuilder, CFG_HIDE_ON_UNMAXIMIZED)),
		*chkb_reverse_order = GTK_TOGGLE_BUTTON (gtk_builder_get_object(wbapplet->prefbuilder, CFG_REVERSE_ORDER)),
		*chkb_hide_decoration = GTK_TOGGLE_BUTTON (gtk_builder_get_object(wbapplet->prefbuilder, CFG_HIDE_DECORATION)),
		*chkb_metacity_order = GTK_TOGGLE_BUTTON (gtk_builder_get_object(wbapplet->prefbuilder, CFG_USE_METACITY_LAYOUT)),
		*chkb_show_tooltips = GTK_TOGGLE_BUTTON (gtk_builder_get_object(wbapplet->prefbuilder, CFG_SHOW_TOOLTIPS));
	GtkButton
		*btn_reload_order = GTK_BUTTON (gtk_builder_get_object(wbapplet->prefbuilder, "btn_reload_order")),
		*btn_close = GTK_BUTTON (gtk_builder_get_object(wbapplet->prefbuilder, "btn_close"));
	GtkEntry *entry_custom_order = GTK_ENTRY (gtk_builder_get_object(wbapplet->prefbuilder, CFG_BUTTON_LAYOUT));
	GtkComboBox *combo_theme = GTK_COMBO_BOX (gtk_builder_get_object(wbapplet->prefbuilder, CFG_THEME)); 
	GtkToggleButton **chkb_btn_hidden = getHideButtons(wbapplet->prefbuilder);
	GtkRadioButton **radio_orientation = getOrientationButtons(wbapplet->prefbuilder);
	
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
						
	CheckBoxData **cbd = g_new(CheckBoxData*, WB_BUTTONS);
	for (i=0; i<WB_BUTTONS; i++) {
		cbd[i] = g_new(CheckBoxData,1);
		cbd[i]->button_id = i;
		cbd[i]->wbapplet = wbapplet;

		gtk_toggle_button_set_active (chkb_btn_hidden[i], wbapplet->prefs->button_hidden[i]);
		g_signal_connect(G_OBJECT(chkb_btn_hidden[i]), "clicked", G_CALLBACK (cb_btn_hidden), cbd[i]);
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
}

/* Close the Properties dialog - we're not saving anything (it's already saved) */
void properties_close (GtkButton *object, WBApplet *wbapplet) {
	gtk_widget_destroy(wbapplet->window_prefs);
	wbapplet->window_prefs = NULL;
}
