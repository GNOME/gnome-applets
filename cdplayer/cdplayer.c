/*  cdplayer.c
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Authors: Miguel de Icaza
 *          Federico Mena
 * GNOME 2 Action:
 *          Chris Phelps
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>
#include <gdk/gdkx.h>
#include <panel-applet.h>
#include <panel-applet-gconf.h>
#include <egg-screen-exec.h>
#include <egg-screen-help.h>

#include "led.h"
#include "cdrom-interface.h"
#include "cdplayer.h"
#include "inlinepixbufs.h"

#define TIMEOUT_VALUE 500
#define CDPLAYER_STOP           "cdplayer-stop"
#define CDPLAYER_PLAY           "cdplayer-play"
#define CDPLAYER_PAUSE          "cdplayer-pause"
#define CDPLAYER_PREV           "cdplayer-prev"
#define CDPLAYER_NEXT           "cdplayer-next"
#define CDPLAYER_EJECT          "cdplayer-eject"

/* Function prototypes */
static gboolean applet_factory (PanelApplet *applet, const gchar *iid, gpointer data);
static gboolean applet_fill (PanelApplet *applet);
static void cdplayer_load_config(CDPlayerData *cd);
static void cdplayer_save_config(CDPlayerData *cd);

static void cdplayer_destroy(GtkWidget * widget, gpointer data);
static void cdplayer_realize(GtkWidget *cdplayer, CDPlayerData *cd);
static int cdplayer_timeout_callback(gpointer data);

static void start_gtcd_cb  (BonoboUIComponent *component,
			    CDPlayerData      *cd,
			    const char        *verb);
static void preferences_cb (BonoboUIComponent *component,
			    CDPlayerData      *cd,
			    const char        *verb);
static void help_cb        (BonoboUIComponent *component,
			    CDPlayerData      *cd,
			    const char        *verb);
static void about_cb       (BonoboUIComponent *component,
			    CDPlayerData      *cd,
			    const char        *verb);

static void phelp_cb (GtkDialog *dialog, gpointer data);
static void applet_change_size(GtkWidget *w, int size, gpointer data);
static void applet_change_orient(GtkWidget *w, PanelAppletOrient o, gpointer data);
static void ui_component_event (BonoboUIComponent *comp, const gchar *path, Bonobo_UIComponent_EventType type, const gchar *state_string, CDPlayerData *data);

static void setup_box(CDPlayerData* cd);

static GtkWidget *control_buttons_from_stock (const gchar * stock_id, GCallback func, CDPlayerData * cd);
static GtkWidget *control_button_factory(gchar * pixmap_data[], GCallback func, CDPlayerData * cd);
static void destroy_box(CDPlayerData* cd);
static void ref_and_remove(GtkWidget *w);
static GtkWidget *pack_make_hbox(CDPlayerData* cd);
static void pack_thing(GtkWidget *box, GtkWidget *w, gboolean expand);

static gboolean cd_try_open(CDPlayerData *cd);
static void cd_close (CDPlayerData *cd);

static void cd_panel_update(GtkWidget * cdplayer, CDPlayerData * cd);
static void cdplayer_play_pause(GtkWidget * w, gpointer data);
static void cdplayer_stop(GtkWidget * w, gpointer data);
static void cdplayer_prev(GtkWidget * w, gpointer data);
static void cdplayer_next(GtkWidget * w, gpointer data);
static void cdplayer_eject(GtkWidget * w, gpointer data);

static void set_atk_relation(GtkWidget *label, GtkWidget *entry);
static void set_atk_name_description(GtkWidget *widget, const gchar *name, const gchar *description);
static void make_applet_accessible(CDPlayerData *cd);

static void cdplayer_init_stock_icons (void);
static void register_cdplayer_stock_icons (GtkIconFactory *factory);
static void cdplayer_update_play_pause_button (CDPlayerData *cd, gint id);

/* Bonobo Verbs for our popup menu */
static const BonoboUIVerb applet_menu_verbs [] = {
    BONOBO_UI_UNSAFE_VERB ("RunGTCD", start_gtcd_cb),
    BONOBO_UI_UNSAFE_VERB ("Preferences", preferences_cb),
    BONOBO_UI_UNSAFE_VERB ("Help", help_cb),
    BONOBO_UI_UNSAFE_VERB ("About", about_cb),
    BONOBO_UI_VERB_END
};

PANEL_APPLET_BONOBO_FACTORY ("OAFIID:GNOME_CDPlayerApplet_Factory",
			     PANEL_TYPE_APPLET,
                             "cdplayer",
                             "0",
                              applet_factory,
                              NULL)

static gboolean
applet_factory (PanelApplet *applet,
        const gchar *iid,
        gpointer     data)
{
    gboolean retval = FALSE;

    if (!strcmp (iid, "OAFIID:GNOME_CDPlayerApplet"))
        retval = applet_fill (applet);

    return retval;
}

static gboolean
applet_fill (PanelApplet *applet)
{
    GtkWidget *cdplayer;
    CDPlayerData *cd;
    BonoboUIComponent *component;
    GtkTooltips *tooltips;
    int err;

    gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gnome-cdplayer-icon.png");
    
    cdplayer_init_stock_icons ();
      
    cd = g_new0(CDPlayerData, 1);
    cd->panel.applet = GTK_WIDGET (applet);
    
    /* the rest of the widgets go in here */
    cdplayer = cd->panel.frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(cdplayer), GTK_SHADOW_IN);
    g_object_set_data (G_OBJECT(cdplayer), "cd-info", cd);
    g_signal_connect_after (G_OBJECT (cdplayer), "realize", G_CALLBACK (cdplayer_realize), cd);
    gtk_widget_show(cdplayer);
    
    tooltips = gtk_tooltips_new (); 
    g_object_ref (tooltips);
    gtk_object_sink (GTK_OBJECT (tooltips));
    g_object_set_data (G_OBJECT (cd->panel.applet), "tooltips", tooltips);

    cd->panel.play_control.stop = control_buttons_from_stock (CDPLAYER_STOP, 
		    					      G_CALLBACK(cdplayer_stop), 
							      cd);
    gtk_tooltips_set_tip (tooltips, cd->panel.play_control.stop, _("Stop"), NULL);
    cd->panel.play_control.play_pause = control_buttons_from_stock (CDPLAYER_PLAY, 
		    						    G_CALLBACK(cdplayer_play_pause), 
								    cd);
    gtk_tooltips_set_tip (tooltips, cd->panel.play_control.play_pause, _("Play / Pause"), NULL);
    cd->panel.play_control.eject = control_buttons_from_stock (CDPLAYER_EJECT, 
		    					       G_CALLBACK(cdplayer_eject), 
							       cd);
    gtk_tooltips_set_tip (tooltips, cd->panel.play_control.eject, _("Eject"), NULL);
    cd->panel.track_control.prev = control_buttons_from_stock (CDPLAYER_PREV, 
		    					       G_CALLBACK(cdplayer_prev), 
							       cd);
    gtk_tooltips_set_tip (tooltips, cd->panel.track_control.prev, _("Previous Track"), NULL);
    cd->panel.track_control.next = control_buttons_from_stock (CDPLAYER_NEXT, 
		    					       G_CALLBACK(cdplayer_next), 
							       cd);
    gtk_tooltips_set_tip (tooltips, cd->panel.track_control.next, _("Next Track"), NULL);
    led_create_widgets(&cd->panel.time, &cd->panel.track_control.display, (gpointer)cd);

    gtk_container_add (GTK_CONTAINER (applet), cdplayer);
    gtk_tooltips_set_tip (tooltips, cd->panel.applet, _("CD Player"), NULL);
    make_applet_accessible (cd); 

    /* panel_applet_add_preferences (applet, "/schemas/apps/cdplayer-applet/prefs", NULL); */
    cdplayer_load_config(cd);

    cd->cdrom_device = cdrom_open(cd->devpath, &err);
    cd->orient = panel_applet_get_orient (PANEL_APPLET (applet));
    cd->size = panel_applet_get_size (PANEL_APPLET (applet));

    g_signal_connect (G_OBJECT (applet), "destroy", G_CALLBACK (cdplayer_destroy), cd);
    g_signal_connect (applet, "change_orient", G_CALLBACK (applet_change_orient), cd);
    g_signal_connect (applet, "change_size", G_CALLBACK (applet_change_size), cd);

    panel_applet_setup_menu_from_file (PANEL_APPLET (applet),
                                       NULL,
                                       "GNOME_CDPlayerApplet.xml",
                                       NULL,
                                       applet_menu_verbs,
                                       cd);

    component = panel_applet_get_popup_component (PANEL_APPLET (applet));

    g_signal_connect (component, "ui-event", G_CALLBACK (ui_component_event), cd);
    gtk_widget_show (GTK_WIDGET(applet));
    setup_box(cd);
    return(TRUE);
}

static void
cdplayer_load_config(CDPlayerData *cd)
{
    g_free(cd->devpath);
    cd->devpath = panel_applet_gconf_get_string(PANEL_APPLET(cd->panel.applet), "device_path", NULL);
    if (!cd->devpath || !strcmp(cd->devpath, "none"))
    {
        g_free(cd->devpath);
        cd->devpath = g_strdup(DEV_PATH);
    }
}

static void
cdplayer_save_config(CDPlayerData *cd)
{
    panel_applet_gconf_set_string(PANEL_APPLET(cd->panel.applet), "device_path", cd->devpath, NULL);
}

static void
cdplayer_destroy(GtkWidget * widget, gpointer data)
{
    CDPlayerData *cd = data;
    GtkTooltips *tooltips;

    if (cd->timeout != 0)
        gtk_timeout_remove(cd->timeout);
    cd->timeout = 0;
    /* Since the applet is being destroyed, stop playing cd */
    if(cd_try_open(cd))
        cdrom_stop(cd->cdrom_device);
    cd_close (cd);

    tooltips = g_object_get_data (G_OBJECT (cd->panel.applet), "tooltips");
    if (tooltips) {
        g_object_unref (tooltips);
        g_object_set_data (G_OBJECT (cd->panel.applet), "tooltips", NULL);
    }
   
    if (cd->time_description)
        g_free(cd->time_description);
    if (cd->track_description)
        g_free(cd->track_description);

    g_free (cd->devpath);
 
    if (cd->prefs_dialog)
      gtk_widget_destroy (cd->prefs_dialog);

    cd->devpath = NULL;
    g_free(cd);
}

static void
cdplayer_realize(GtkWidget *cdplayer, CDPlayerData *cd)
{
    /* Add all of the widgets to the box in the right order for the Orientation/Size types */
    setup_box(cd);
    cd->timeout = gtk_timeout_add(TIMEOUT_VALUE, cdplayer_timeout_callback, (gpointer)cdplayer);
}

static int 
cdplayer_timeout_callback(gpointer data)
{
    GtkWidget *cdplayer;
    CDPlayerData *cd;

    cdplayer = (GtkWidget *)data;
    cd = g_object_get_data(G_OBJECT(cdplayer), "cd-info");
    cd_panel_update(cdplayer, cd);
    return(1);
}

/* Signal handlers for the cdplayer popup menu*/
static void
start_gtcd_cb (BonoboUIComponent *component,
	       CDPlayerData      *cd,
	       const char        *verb)
{
    GError *error = NULL;

    egg_screen_execute_command_line_async (
		gtk_widget_get_screen (cd->panel.applet), "gnome-cd", &error);
    if (error) {
	GtkWidget *dialog;

	dialog = gtk_message_dialog_new (NULL,
					 GTK_DIALOG_DESTROY_WITH_PARENT,
					 GTK_MESSAGE_ERROR,
					 GTK_BUTTONS_CLOSE,
					 _("There was an error executing '%s' : %s"),
					 "gnome-cd",
					 error->message);

	g_signal_connect (dialog, "response",
			  G_CALLBACK (gtk_widget_destroy),
			  NULL);

	gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
	gtk_window_set_screen (GTK_WINDOW (dialog),
			       gtk_widget_get_screen (cd->panel.applet));

	gtk_widget_show (dialog);

	g_error_free (error);
   }
}

static void
response_cb (GtkDialog *dialog, gint id, gpointer data)
{
    CDPlayerData *cd = data;

    if(id == GTK_RESPONSE_HELP){
         phelp_cb (dialog,data);
	 return;
    }
    gtk_widget_destroy (GTK_WIDGET (dialog));
    cd->prefs_dialog = NULL;
}

static void
activate_cb (GtkEntry     *entry,
	     CDPlayerData *cd)
{
    gchar *newpath;
    
    newpath = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
    if(newpath && strlen(newpath) > 2 && strcmp(cd->devpath, newpath))
            {
		if(cd_try_open(cd))
		    cdrom_stop(cd->cdrom_device);
                cd_close(cd);
                cd->devpath = g_strdup(newpath);
                if (!cd_try_open(cd)) {
                    GtkWidget *dialog;
                    dialog = gtk_message_dialog_new (NULL, GTK_DIALOG_DESTROY_WITH_PARENT,
                                  		     GTK_MESSAGE_ERROR,
                                  		     GTK_BUTTONS_CLOSE,
                                  		     "%s is not a proper device path",
                                  		     cd->devpath, NULL);

		    gtk_window_set_screen (GTK_WINDOW (dialog),
					   gtk_widget_get_screen (cd->panel.applet));

                    g_signal_connect_swapped (GTK_OBJECT (dialog), "response",
                                              G_CALLBACK (gtk_widget_destroy),
                                              GTK_OBJECT (dialog));

		    gtk_widget_show_all (dialog);
                   
                }
                cdplayer_save_config(cd);  
            }
    if (newpath)
        g_free (newpath);
}

static gboolean
focus_out_cb (GtkWidget     *widget,
	      GdkEventFocus *event,
	      CDPlayerData  *cd)
{
    activate_cb (GTK_ENTRY (widget), cd);
    
    return FALSE;
}

static void
set_default_device (GtkButton *button, gpointer data)
{
    CDPlayerData *cd = data;
    GtkWidget *entry = g_object_get_data (G_OBJECT (button), "entry");
    
    if (!strcmp(cd->devpath, DEV_PATH))
    	return;
    	
    cd_close(cd);
    if (cd->devpath)
        g_free(cd->devpath);
    cd->devpath = g_strdup(DEV_PATH);
    cd_try_open(cd);
    gtk_entry_set_text (GTK_ENTRY (entry), cd->devpath);
    cdplayer_save_config(cd);  
}
    
static void
preferences_cb (BonoboUIComponent *component,
		CDPlayerData      *cd,
		const char        *verb)
{
    GtkWidget *dialog;
    GtkWidget *box;
    GtkWidget *button;
    GtkWidget *hbox;
    GtkWidget *label;
    GtkWidget *image;
    GtkWidget *entry;
    gint response;

    if (cd->prefs_dialog) {
      gtk_window_present (GTK_WINDOW (cd->prefs_dialog));
      return;
    }

    dialog = gtk_dialog_new_with_buttons(_("CD Player Preferences"),
                                         NULL, GTK_DIALOG_DESTROY_WITH_PARENT,
                                         GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
                                         GTK_STOCK_HELP, GTK_RESPONSE_HELP,
                                         NULL);
    cd->prefs_dialog = dialog;

    gtk_window_set_screen (GTK_WINDOW (dialog),
			   gtk_widget_get_screen (cd->panel.applet));
    gtk_dialog_set_default_response(GTK_DIALOG (dialog), GTK_RESPONSE_CLOSE);
    box = GTK_DIALOG(dialog)->vbox;
    
    hbox = gtk_hbox_new(FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), GNOME_PAD);
    gtk_box_pack_start(GTK_BOX(box), hbox, TRUE, TRUE, 0);
    gtk_widget_show(hbox);
    image = gtk_image_new_from_stock(GTK_STOCK_CDROM, GTK_ICON_SIZE_BUTTON);

    gtk_box_pack_start(GTK_BOX(hbox), image, FALSE, FALSE, 10);
    gtk_widget_show(image);
    set_atk_name_description(image, _("Disc Image"), _("An image of a cd-rom disc"));

    label = gtk_label_new_with_mnemonic(_("Device _Path:"));
    gtk_misc_set_padding (GTK_MISC (label), GNOME_PAD_SMALL, 0);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
    gtk_widget_show(label);

    entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 4);
    gtk_widget_show(entry);
    gtk_entry_set_text(GTK_ENTRY(entry), cd->devpath);
    set_atk_name_description(entry, _("Device Path"), _("Set the device path here"));
    set_atk_relation(label, entry);
    
    button = gtk_button_new_with_mnemonic (_("Use _Default"));
    gtk_box_pack_start (GTK_BOX(hbox), button, FALSE, FALSE, 0);
    g_object_set_data (G_OBJECT (button), "entry", entry);
    gtk_widget_show (button);
    
    g_signal_connect (G_OBJECT (entry), "activate",
    		      G_CALLBACK (activate_cb), cd);
    g_signal_connect (G_OBJECT (entry), "focus_out_event",
    		      G_CALLBACK (focus_out_cb), cd);
    g_signal_connect (G_OBJECT (button), "clicked",
    		      G_CALLBACK (set_default_device), cd);
    
    g_signal_connect (G_OBJECT (dialog), "response",
    		      G_CALLBACK (response_cb), cd);

    gtk_widget_show_all(dialog);
}

static void
help_cb (BonoboUIComponent *component,
	 CDPlayerData      *cd,
	 const char        *verb)
{
    GError *error = NULL;

    egg_help_display_on_screen (
		"cdplayer", NULL,
		gtk_widget_get_screen (cd->panel.applet),
		&error);

    if (error) { /* FIXME: the user needs to see this */
        g_warning ("help error: %s\n", error->message);
        g_error_free (error);
        error = NULL;
    }

    return;
}

static void
about_cb (BonoboUIComponent *component,
	  CDPlayerData      *cd,
	  const char        *verb)
{
    static GtkWidget   *about     = NULL;
    GdkPixbuf	       *pixbuf;
    GError	       *error     = NULL;
    gchar	       *file;

    static const gchar *authors[] =
    {
        "Miguel de Icaza <miguel@kernel.org>",
        "Federico Mena <quartic@gimp.org>",
        "Chris Phelps <chicane@renient.com>",
        NULL
    };

    const gchar *documenters[] =
    {
	    NULL
    };

    const gchar *translator_credits = _("translator_credits");

    if (about) {
	gtk_window_set_screen (GTK_WINDOW (about),
			       gtk_widget_get_screen (cd->panel.applet));
	gtk_window_present (GTK_WINDOW (about));
        return;
    }
    
    file = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_PIXMAP, "gnome-cdplayer-icon.png", FALSE, NULL);
    pixbuf = gdk_pixbuf_new_from_file (file, &error);
    g_free (file);
    
    if (error) {
    	g_warning (G_STRLOC ": cannot open %s: %s", file, error->message);
	g_error_free (error);
    }
    
    about = gnome_about_new (_("CD Player"), VERSION,
                             _("(C) 1997 The Free Software Foundation\n" \
                               "(C) 2001 Chris Phelps (GNOME 2 Port)"),
                             _("The CD Player applet is a simple audio CD player for your panel"),
                             authors,
			     documenters,
			     strcmp (translator_credits, "translator_credits") != 0 ? translator_credits : NULL,
			     pixbuf);
    if (pixbuf)
    	gdk_pixbuf_unref (pixbuf);

    gtk_window_set_wmclass (GTK_WINDOW (about), "cd player", "CD Player");
    gtk_window_set_screen (GTK_WINDOW (about),
			   gtk_widget_get_screen (cd->panel.applet));
    g_signal_connect (G_OBJECT(about), "destroy",
                      G_CALLBACK(gtk_widget_destroyed), &about);
    gtk_widget_show (about);
}

static void
applet_change_size(GtkWidget *w, int size, gpointer data)
{
    CDPlayerData *cd;

    cd = (CDPlayerData *)data;
    cd->size = size;
    setup_box(cd);
    cd_panel_update(cd->panel.frame, cd);
}

static void
applet_change_orient(GtkWidget *w, PanelAppletOrient o, gpointer data)
{
    CDPlayerData *cd;

    cd = (CDPlayerData *)data;
    cd->orient = o;
    setup_box(cd);
    cd_panel_update(cd->panel.frame, cd);
}

static void
ui_component_event (BonoboUIComponent *comp,
                    const gchar *path,
                    Bonobo_UIComponent_EventType  type,
                    const gchar *state_string,
                    CDPlayerData *data)
{
    gboolean state;

    g_print("(ui-component-event) path is %s and state is %s.\n", path, state);
}

static void
setup_box(CDPlayerData* cd)
{
    GtkWidget *hbox, *vbox;

    if(cd->panel.box)
        destroy_box(cd);

    if ((cd->orient == PANEL_APPLET_ORIENT_DOWN || cd->orient == PANEL_APPLET_ORIENT_UP) && cd->size < 48  )
    {
        /* tiny horizontal panel */
        cd->panel.box = gtk_hbox_new(FALSE, 0);
        gtk_container_add(GTK_CONTAINER(cd->panel.frame), cd->panel.box);
        pack_thing(cd->panel.box, cd->panel.track_control.prev, FALSE);
        pack_thing(cd->panel.box, cd->panel.track_control.display, TRUE);
        pack_thing(cd->panel.box, cd->panel.track_control.next, FALSE);
        pack_thing(cd->panel.box, cd->panel.time, TRUE);
        pack_thing(cd->panel.box, cd->panel.play_control.play_pause, FALSE);
        pack_thing(cd->panel.box, cd->panel.play_control.stop, FALSE);
        pack_thing(cd->panel.box, cd->panel.play_control.eject, FALSE);
        gtk_widget_show(cd->panel.box);

    }
    else if ((cd->orient == PANEL_APPLET_ORIENT_DOWN || cd->orient == PANEL_APPLET_ORIENT_UP) && cd->size < 80  )
    {
        /* small horizontal panel */
        cd->panel.box = gtk_vbox_new(FALSE, 0);
        gtk_container_add(GTK_CONTAINER(cd->panel.frame), cd->panel.box);
        hbox = pack_make_hbox(cd);
        pack_thing(hbox, cd->panel.time, TRUE);
        pack_thing(hbox, cd->panel.track_control.display, TRUE);
        hbox = pack_make_hbox(cd);
        pack_thing(hbox, cd->panel.play_control.play_pause, FALSE);
        pack_thing(hbox, cd->panel.play_control.stop, FALSE);
        pack_thing(hbox, cd->panel.play_control.eject, FALSE);
        pack_thing(hbox, cd->panel.track_control.prev, FALSE);
        pack_thing(hbox, cd->panel.track_control.next, FALSE);
        gtk_widget_show(cd->panel.box);
    }
    else if ((cd->orient == PANEL_APPLET_ORIENT_LEFT || cd->orient == PANEL_APPLET_ORIENT_RIGHT) && cd->size < 128  )
    {
        cd->panel.box = gtk_vbox_new(FALSE, 0);
	vbox = cd->panel.box;
        gtk_container_add(GTK_CONTAINER(cd->panel.frame), cd->panel.box);
        pack_thing(cd->panel.box, cd->panel.time, TRUE);
        pack_thing(vbox, cd->panel.track_control.display, TRUE);
        pack_thing(vbox, cd->panel.play_control.play_pause, TRUE);
        pack_thing(vbox, cd->panel.track_control.prev, TRUE);
        pack_thing(vbox, cd->panel.track_control.next, TRUE);
        pack_thing(vbox, cd->panel.play_control.stop, FALSE);
        pack_thing(vbox, cd->panel.play_control.eject, FALSE);
        gtk_widget_show(cd->panel.box);
    }
    else
    {
        /* other panel sizes/orientations should go here */
        cd->panel.box = gtk_vbox_new(FALSE, 0);
        gtk_container_add(GTK_CONTAINER(cd->panel.frame), cd->panel.box);
        pack_thing(cd->panel.box, cd->panel.time, TRUE);
        hbox = pack_make_hbox(cd);
        pack_thing(hbox, cd->panel.play_control.play_pause, FALSE);
        pack_thing(hbox, cd->panel.play_control.stop, FALSE);
        pack_thing(hbox, cd->panel.play_control.eject, FALSE);
        hbox = pack_make_hbox(cd);
        pack_thing(hbox, cd->panel.track_control.prev, FALSE);
        pack_thing(hbox, cd->panel.track_control.display, TRUE);
        pack_thing(hbox, cd->panel.track_control.next, FALSE);
        gtk_widget_show(cd->panel.box);
    }
    gtk_widget_show_all(cd->panel.frame);
}

/* This is a hack around the fact that gtk+ doesn't
 * propogate button presses on button2/3.
 */
static gboolean 
button_press_hack (GtkWidget      *widget,
		   GdkEventButton *event,
		   GtkWidget      *applet)
{
    if (event->button == 3 || event->button == 2) {
	gtk_propagate_event (applet, (GdkEvent *) event);
	return TRUE;
    }
    
    return FALSE;
}

static void
destroy_box(CDPlayerData* cd)
{
    g_return_if_fail(GTK_IS_CONTAINER(cd->panel.box));

    ref_and_remove(cd->panel.time);
    ref_and_remove(cd->panel.play_control.stop);
    ref_and_remove(cd->panel.play_control.play_pause);
    ref_and_remove(cd->panel.play_control.eject);
    ref_and_remove(cd->panel.track_control.prev);
    ref_and_remove(cd->panel.track_control.display);
    ref_and_remove(cd->panel.track_control.next);

    gtk_widget_destroy(cd->panel.box);
    cd->panel.box = NULL;
}

static void
ref_and_remove(GtkWidget *w)
{
    GtkWidget *parent = w->parent;
    if(parent) {
        g_object_ref(G_OBJECT(w));
        gtk_container_remove(GTK_CONTAINER(parent), w);
    }
}

static GtkWidget *
pack_make_hbox(CDPlayerData* cd)
{
    GtkWidget *hbox;

    hbox = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(hbox);
    gtk_box_pack_start(GTK_BOX(cd->panel.box), hbox, TRUE, TRUE, 0);
    return hbox;
}

static void
pack_thing(GtkWidget *box, GtkWidget *w, gboolean expand)
{
    gtk_box_pack_start(GTK_BOX(box), w, expand, TRUE, 0);
    g_object_unref(G_OBJECT(w));
}

/* Deal with the hardware stuff */
static gboolean
cd_try_open(CDPlayerData *cd)
{
    int err;
    if(cd->cdrom_device == NULL) {
        cd->cdrom_device = cdrom_open(cd->devpath, &err);
        return cd->cdrom_device != NULL;
    }
    return TRUE;
}

static void
cd_close (CDPlayerData *cd)
{
    if (cd->cdrom_device != NULL) {
        cdrom_close(cd->cdrom_device);
        cd->cdrom_device = NULL;
    }
}

static void 
cd_panel_update(GtkWidget * cdplayer, CDPlayerData * cd)
{
    cdrom_device_status_t stat;
    gboolean description = FALSE;

    if (cd_try_open(cd)) {
        if (cdrom_get_status(cd->cdrom_device, &stat) == DISC_NO_ERROR)
        {
            switch (stat.audio_status)
            {
                case DISC_PLAY:
                    led_time(cd->panel.time,
                         stat.relative_address.minute,
                         stat.relative_address.second,
                         cd->panel.track_control.display,
                         stat.track);
                    description = TRUE;
	    	    cdplayer_update_play_pause_button (cd, PAUSE_IMAGE);
                    break;
                case DISC_PAUSED:
                    led_paused(cd->panel.time,
                           stat.relative_address.minute,
                           stat.relative_address.second,
                           cd->panel.track_control.display,
                           stat.track);
                    description = TRUE;
	    	    cdplayer_update_play_pause_button (cd, PLAY_IMAGE);
                    break;
                case DISC_COMPLETED:
                case DISC_STOP:
	    	    cdplayer_update_play_pause_button (cd, PLAY_IMAGE);
                case DISC_ERROR:
                    led_stop(cd->panel.time, cd->panel.track_control.display);
                    break;
                default:
                    break;
            }
        }
    }
    else
    {
        led_nodisc(cd->panel.time, cd->panel.track_control.display);
    }
    if (description)
    {
        cd->time_description =
            g_strdup_printf("Time elapsed is %d minutes and %d seconds",
                stat.relative_address.minute,
                stat.relative_address.second);
        cd->track_description =
            g_strdup_printf("Current track number is %d",
                stat.track);
        set_atk_name_description(cd->panel.time, _("Elapsed time"),
            cd->time_description);
        set_atk_name_description(cd->panel.track_control.display,
            _("Track number"), cd->track_description);
        g_free (cd->time_description);
        g_free (cd->track_description);
        cd->time_description = NULL;
        cd->track_description = NULL;
    }
    else
    {
        set_atk_name_description(cd->panel.time, _("Elapsed time"), "");
        set_atk_name_description(cd->panel.track_control.display,
            _("Track number"), "");
    }
    return;
    cdplayer = NULL; 
}


/* Control button callbacks */
static void 
cdplayer_play_pause(GtkWidget * w, gpointer data)
{
    CDPlayerData *cd = data;
    cdrom_device_status_t stat;
    int status;
    int ret;

    if(!cd_try_open(cd))
        return;

    status = cdrom_get_status(cd->cdrom_device, &stat);
    if (status == DISC_NO_ERROR) {
        switch (stat.audio_status) {
        case DISC_PLAY:
	    cdplayer_update_play_pause_button (cd, PLAY_IMAGE);
            cdrom_pause(cd->cdrom_device);
            break;
        case DISC_PAUSED:
	    cdplayer_update_play_pause_button (cd, PAUSE_IMAGE);
            cdrom_resume(cd->cdrom_device);
            break;
        case DISC_COMPLETED:
        case DISC_STOP:
	    cdplayer_update_play_pause_button (cd, PAUSE_IMAGE);
        case DISC_ERROR:
            cdrom_read_track_info(cd->cdrom_device);
            ret = cdrom_play(cd->cdrom_device, cd->cdrom_device->track0,
                   cd->cdrom_device->track1);
            break;
        }
    } else if(status == DISC_TRAY_OPEN) {
        cdrom_load(cd->cdrom_device);
        cdrom_read_track_info(cd->cdrom_device);
        ret = cdrom_play(cd->cdrom_device, cd->cdrom_device->track0,
               cd->cdrom_device->track1);
    }

    if (ret == DISC_DEVICE_BUSY) {
		GtkWidget *dialog;
		dialog = gtk_message_dialog_new (NULL,
						 GTK_DIALOG_DESTROY_WITH_PARENT,
						 GTK_MESSAGE_ERROR,
						 GTK_BUTTONS_CLOSE,
						 "Audio Device is busy, or being used by another application",
						 NULL);
		gtk_window_set_screen (GTK_WINDOW (dialog),
				       gtk_widget_get_screen (cd->panel.applet));
		g_signal_connect_swapped (GTK_OBJECT (dialog), "response",
		                          G_CALLBACK (gtk_widget_destroy),
					  GTK_OBJECT (dialog));

		gtk_widget_show_all (dialog);
    }
}

static void 
cdplayer_stop(GtkWidget * w, gpointer data)
{
    CDPlayerData *cd = data;
    if(!cd_try_open(cd))
        return;
    cdrom_stop(cd->cdrom_device);

}

static void 
cdplayer_prev(GtkWidget * w, gpointer data)
{
    CDPlayerData *cd = data;
    if(!cd_try_open(cd))
        return;
    cdrom_prev(cd->cdrom_device);

}

static void 
cdplayer_next(GtkWidget * w, gpointer data)
{
    CDPlayerData *cd = data;
    if(!cd_try_open(cd))
        return;
    cdrom_next(cd->cdrom_device);

}

static void 
cdplayer_eject(GtkWidget * w, gpointer data)
{
    cdrom_device_status_t stat;
    CDPlayerData *cd = data;
    if(!cd_try_open(cd))
        return;
    if(cdrom_get_status(cd->cdrom_device, &stat) == DISC_TRAY_OPEN)
        /*
          FIXME: if there is no disc, we get TRAY_OPEN even
          if the tray is closed, is this a kernel bug?? or
          is this the inteded behaviour, we don't support this
          on solaris anyway, but unless we have a good way to
          find out if the tray is actually open, we can't do this
        */
        /* cdrom_load(cd->cdrom_device); */
        /*in the meantime let's just do eject so that at least that works */
        cdrom_eject(cd->cdrom_device);
    else
        cdrom_eject(cd->cdrom_device);
    cd_close(cd);
    return;
        w = NULL;
}

static void
make_applet_accessible(CDPlayerData *cd)
{
    /* Check if gail is loaded */
    if (GTK_IS_ACCESSIBLE(gtk_widget_get_accessible(cd->panel.play_control.stop))
        == FALSE) return;
	
    set_atk_name_description(cd->panel.play_control.stop, _("Stop"), 
        _("Click this button to Stop playing the CD"));
    set_atk_name_description(cd->panel.play_control.play_pause, _("Play / Pause"), 
        _("Click this button to Play or Pause the CD"));
    set_atk_name_description(cd->panel.play_control.eject, _("Eject"), 
        _("Click this button to eject the CD"));
    set_atk_name_description(cd->panel.track_control.prev, _("Previous Track"),
        _("Click this button to Play the previous track in the CD"));
    set_atk_name_description(cd->panel.track_control.next, _("Next Track"),
        _("Click this button to Play the next track in the CD"));
    set_atk_name_description(cd->panel.applet, _("CD Player"), 
        _("The CD Player applet is a simple audio CD player for your panel"));
}

static void
set_atk_name_description(GtkWidget *widget, const gchar *name,
    const gchar *description)
{	
    AtkObject *aobj;
	
    aobj = gtk_widget_get_accessible(widget);
    /* Check if gail is loaded */
    if (GTK_IS_ACCESSIBLE (aobj) == FALSE)
        return; 
    atk_object_set_name(aobj, name);
    atk_object_set_description(aobj, description);
}

static void
set_atk_relation(GtkWidget *label, GtkWidget *widget)
{
    AtkObject *atk_widget;
    AtkObject *atk_label;
    AtkRelationSet *relation_set;
    AtkRelation *relation;
    AtkObject *targets[1];

    atk_widget = gtk_widget_get_accessible(widget);
    atk_label = gtk_widget_get_accessible(label);

    /* Set the label-for relation */
    gtk_label_set_mnemonic_widget(GTK_LABEL (label), widget);	

    /* Check if gail is loaded */
    if (GTK_IS_ACCESSIBLE (atk_widget) == FALSE)
        return;

    /* Set the labelled-by relation */
    relation_set = atk_object_ref_relation_set(atk_widget);
    targets[0] = atk_label;
    relation = atk_relation_new(targets, 1, ATK_RELATION_LABELLED_BY);
    atk_relation_set_add(relation_set, relation);
    g_object_unref(G_OBJECT (relation));
}

static void
phelp_cb (GtkDialog *dialog, gpointer data)
{
    GError *error = NULL;

    egg_help_display_on_screen (
		"cdplayer", "cdplayer_applet_prefs",
		gtk_window_get_screen (GTK_WINDOW (dialog)),
		&error);
    
    if (error) { /* FIXME: the user needs to see this */
        g_warning ("help error: %s\n", error->message);
        g_error_free (error);
        error = NULL;
    }
}

static GtkWidget *
control_buttons_from_stock (const gchar * stock_id, GCallback func, CDPlayerData * cd)
{
    GtkWidget *w;
    GtkWidget *image;
    static gint first_time = 0;
    static GtkIconSize cdplayer_icon_size = 0;

    w = gtk_button_new ();
    GTK_WIDGET_UNSET_FLAGS(w, GTK_CAN_DEFAULT);

    if (!first_time) {
	cdplayer_icon_size = gtk_icon_size_register ("panel-menu",
						     CDPLAYER_DEFAULT_ICON_SIZE,
						     CDPLAYER_DEFAULT_ICON_SIZE);
     }
    first_time += 1;     

    if (!strcmp (stock_id, CDPLAYER_PLAY)) {
	cd->play_image = gtk_image_new_from_stock (stock_id, cdplayer_icon_size);
	gtk_widget_show (cd->play_image);
	g_object_ref (cd->play_image);

	gtk_container_add(GTK_CONTAINER(w), cd->play_image);
	cd->current_image = cd->play_image;

	cd->pause_image = gtk_image_new_from_stock (CDPLAYER_PAUSE, cdplayer_icon_size);
	gtk_widget_show (cd->pause_image);
	g_object_ref (cd->pause_image);
    }
    else {    
	image = gtk_image_new_from_stock (stock_id, cdplayer_icon_size);

	gtk_widget_show(image);
	gtk_container_add(GTK_CONTAINER(w), image);
    }

    g_signal_connect (w, "button_press_event",
		      G_CALLBACK (button_press_hack),
		      cd->panel.applet);

    g_signal_connect (w, "clicked",
		      G_CALLBACK(func), cd);
		
    gtk_widget_show(w);
    gtk_widget_ref(w);
    return(w);
}

typedef struct
{
    char *stock_id;
    const guint8 *icon_data;
} CdplayerStockIcon;

static void
register_cdplayer_stock_icons (GtkIconFactory *factory)
{
    gint i;

    CdplayerStockIcon items[] =
    {
        { CDPLAYER_STOP, cdplayer_stop_data },
	{ CDPLAYER_PLAY, cdplayer_play_data },
	{ CDPLAYER_PAUSE, cdplayer_pause_data },
	{ CDPLAYER_PREV, cdplayer_prev_data },
	{ CDPLAYER_NEXT, cdplayer_next_data },
	{ CDPLAYER_EJECT, cdplayer_eject_data }
    };

    for (i = 0; i < 6; ++i) {
	GtkIconSet *icon_set;
	GdkPixbuf *pixbuf;

	pixbuf = gdk_pixbuf_new_from_inline (-1, items[i].icon_data,
		 			     FALSE, NULL);

	icon_set = gtk_icon_set_new_from_pixbuf (pixbuf);
	gtk_icon_factory_add (factory, items[i].stock_id, icon_set);

	gtk_icon_set_unref (icon_set);
	g_object_unref (G_OBJECT (pixbuf));
    }
}

static void
cdplayer_init_stock_icons (void)
{
    GtkIconFactory *factory;

    factory = gtk_icon_factory_new ();
    gtk_icon_factory_add_default (factory);

    register_cdplayer_stock_icons (factory);

    g_object_unref (factory);
}

static void
cdplayer_update_play_pause_button (CDPlayerData *cd, gint id)
{
    if (id == PLAY_IMAGE) {
	if (cd->current_image == cd->pause_image) {
	    gtk_container_remove (GTK_CONTAINER (cd->panel.play_control.play_pause), cd->current_image);
	    gtk_container_add (GTK_CONTAINER (cd->panel.play_control.play_pause), cd->play_image);
	    cd->current_image = cd->play_image;
	}
    }
    else {
	if (cd->current_image == cd->play_image) {
	    gtk_container_remove (GTK_CONTAINER (cd->panel.play_control.play_pause), cd->current_image);
	    gtk_container_add (GTK_CONTAINER (cd->panel.play_control.play_pause), cd->pause_image);
	    cd->current_image = cd->pause_image;
	}
    }
}
