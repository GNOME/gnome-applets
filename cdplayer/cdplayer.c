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

#include "led.h"
#include "cdrom-interface.h"
#include "cdplayer.h"

#include "images/cdplayer-stop.xpm"
#include "images/cdplayer-play-pause.xpm"
#include "images/cdplayer-prev.xpm"
#include "images/cdplayer-next.xpm"
#include "images/cdplayer-eject.xpm"

#define TIMEOUT_VALUE 500

/* Function prototypes */
static gboolean applet_factory (PanelApplet *applet, const gchar *iid, gpointer data);
static gboolean applet_fill (PanelApplet *applet);
static void cdplayer_load_config(CDPlayerData *cd);
static void cdplayer_save_config(CDPlayerData *cd);

static void cdplayer_destroy(GtkWidget * widget, gpointer data);
static void cdplayer_realize(GtkWidget *cdplayer, CDPlayerData *cd);
static int cdplayer_timeout_callback(gpointer data);

static void start_gtcd_cb(GtkWidget *w, gpointer data);
static void properties_cb (GtkWidget *w, gpointer data);
static void help_cb (GtkWidget *w, gpointer data);
static void about_cb(GtkWidget *w, gpointer data);

static void applet_change_size(GtkWidget *w, int size, gpointer data);
static void applet_change_orient(GtkWidget *w, PanelAppletOrient o, gpointer data);
static void ui_component_event (BonoboUIComponent *comp, const gchar *path, Bonobo_UIComponent_EventType type, const gchar *state_string, CDPlayerData *data);

static void setup_box(CDPlayerData* cd);
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

/* Bonobo Verbs for our popup menu */
static const BonoboUIVerb applet_menu_verbs [] = {
    BONOBO_UI_UNSAFE_VERB ("RunGTCD", start_gtcd_cb),
    BONOBO_UI_UNSAFE_VERB ("Preferences", properties_cb),
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

    cd->panel.box = NULL;
    cd->panel.play_control.stop = control_button_factory(stop_xpm, G_CALLBACK(cdplayer_stop), cd);
    gtk_tooltips_set_tip (tooltips, cd->panel.play_control.stop, _("Stop"), NULL);
    cd->panel.play_control.play_pause = control_button_factory(play_pause_xpm, G_CALLBACK(cdplayer_play_pause), cd);
    gtk_tooltips_set_tip (tooltips, cd->panel.play_control.play_pause, _("Play / Pause"), NULL);
    cd->panel.play_control.eject = control_button_factory(eject_xpm, G_CALLBACK(cdplayer_eject), cd);
    gtk_tooltips_set_tip (tooltips, cd->panel.play_control.eject, _("Eject"), NULL);
    cd->panel.track_control.prev = control_button_factory(prev_xpm, G_CALLBACK(cdplayer_prev), cd);
    gtk_tooltips_set_tip (tooltips, cd->panel.track_control.prev, _("Previous Track"), NULL);
    cd->panel.track_control.next = control_button_factory(next_xpm, G_CALLBACK(cdplayer_next), cd);
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
start_gtcd_cb(GtkWidget *w, gpointer data)
{
    GError *err = NULL;
    g_spawn_command_line_async ("gnome-cd", &err);
    if (err) {
      g_print ("%s\n", err->message);
      g_error_free (err);
    }
}

static void
response_cb (GtkDialog *dialog, gint id, gpointer data)
{
    gtk_widget_destroy (GTK_WIDGET (dialog));
    
}

static void
activate_cb (GtkEntry *entry, gpointer data)
{
    CDPlayerData *cd = data;
    gchar *newpath;
    
    newpath = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
    if(newpath && strlen(newpath) > 2 && strcmp(cd->devpath, newpath))
            {
                cd_close(cd);
                cd->devpath = g_strdup(newpath);
                if (!cd_try_open(cd)) {
                    GtkWidget *dialog;
                    dialog = gtk_message_dialog_new (NULL, GTK_DIALOG_DESTROY_WITH_PARENT,
                                  		     GTK_MESSAGE_ERROR,
                                  		     GTK_BUTTONS_CLOSE,
                                  		     "%s is not a proper device path",
                                  		     cd->devpath, NULL);
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
focus_out_cb (GtkWidget *widget, GdkEventFocus *event, gpointer data)
{
    CDPlayerData *cd = data;
   
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
    
/* FIXME: change the fn name later to reflect Preferences */
static void
properties_cb (GtkWidget *w, gpointer data)
{
    CDPlayerData *cd;
    GtkWidget *dialog;
    GtkWidget *box;
    GtkWidget *button;
    GtkWidget *hbox;
    GtkWidget *label;
    GtkWidget *image;
    GtkWidget *entry;
    gint response;

    cd = (CDPlayerData *) data;

    dialog = gtk_dialog_new_with_buttons(_("CD Player Preferences"),
                                         NULL, GTK_DIALOG_DESTROY_WITH_PARENT,
                                         GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
                                         NULL);
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
    		      G_CALLBACK (response_cb), NULL);

    gtk_widget_show_all(dialog);
    
}

static void
help_cb (GtkWidget *w, gpointer data)
{
    GError *error = NULL;

    gnome_help_display ("cdplayer", NULL, &error);
    if (error) {
        g_warning ("help error: %s\n", error->message);
        g_error_free (error);
    }

    return;
}

static void
about_cb(GtkWidget *w, gpointer data)
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

    if (about != NULL)
    {
        gdk_window_show(about->window);
        gdk_window_raise(about->window);
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
    gnome_window_icon_set_from_file (GTK_WINDOW (about), GNOME_ICONDIR"/gnome-cdplayer-icon.png");	
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
    GtkWidget *hbox;

    if(cd->panel.box)
        destroy_box(cd);

    if ((cd->orient == PANEL_APPLET_ORIENT_DOWN || cd->orient == PANEL_APPLET_ORIENT_UP) && cd->size < 36  )
    {
        /* tiny horizontal panel */
        cd->panel.box = gtk_hbox_new(FALSE, 0);
        gtk_container_add(GTK_CONTAINER(cd->panel.frame), cd->panel.box);
        pack_thing(cd->panel.box, cd->panel.track_control.prev, FALSE);
        pack_thing(cd->panel.box, cd->panel.track_control.display, TRUE);
        pack_thing(cd->panel.box, cd->panel.track_control.next, FALSE);
        pack_thing(cd->panel.box, cd->panel.time, TRUE);
        pack_thing(cd->panel.box, cd->panel.play_control.stop, FALSE);
        pack_thing(cd->panel.box, cd->panel.play_control.play_pause, FALSE);
        pack_thing(cd->panel.box, cd->panel.play_control.eject, FALSE);
        gtk_widget_show(cd->panel.box);

    }
    else if ((cd->orient == PANEL_APPLET_ORIENT_DOWN || cd->orient == PANEL_APPLET_ORIENT_UP) && cd->size < 48  )
    {
        /* small horizontal panel */
        cd->panel.box = gtk_vbox_new(FALSE, 0);
        gtk_container_add(GTK_CONTAINER(cd->panel.frame), cd->panel.box);
        hbox = pack_make_hbox(cd);
        pack_thing(hbox, cd->panel.time, TRUE);
        pack_thing(hbox, cd->panel.track_control.display, TRUE);
        hbox = pack_make_hbox(cd);
        pack_thing(hbox, cd->panel.play_control.stop, FALSE);
        pack_thing(hbox, cd->panel.play_control.play_pause, FALSE);
        pack_thing(hbox, cd->panel.play_control.eject, FALSE);
        pack_thing(hbox, cd->panel.track_control.prev, FALSE);
        pack_thing(hbox, cd->panel.track_control.next, FALSE);
        gtk_widget_show(cd->panel.box);
    }
    else if ((cd->orient == PANEL_APPLET_ORIENT_LEFT || cd->orient == PANEL_APPLET_ORIENT_RIGHT) && cd->size < 48  )
    {
        cd->panel.box = gtk_vbox_new(FALSE, 0);
        gtk_container_add(GTK_CONTAINER(cd->panel.frame), cd->panel.box);
        pack_thing(cd->panel.box, cd->panel.time, TRUE);
        hbox = pack_make_hbox(cd);
        pack_thing(hbox, cd->panel.play_control.stop, FALSE);
        pack_thing(hbox, cd->panel.play_control.play_pause, TRUE);
        hbox = pack_make_hbox(cd);
        pack_thing(hbox, cd->panel.play_control.eject, FALSE);
        pack_thing(hbox, cd->panel.track_control.display, TRUE);
        hbox = pack_make_hbox(cd);
        pack_thing(hbox, cd->panel.track_control.prev, TRUE);
        pack_thing(hbox, cd->panel.track_control.next, TRUE);
        gtk_widget_show(cd->panel.box);
    }
    else
    {
        /* other panel sizes/orientations should go here */
        cd->panel.box = gtk_vbox_new(FALSE, 0);
        gtk_container_add(GTK_CONTAINER(cd->panel.frame), cd->panel.box);
        pack_thing(cd->panel.box, cd->panel.time, TRUE);
        hbox = pack_make_hbox(cd);
        pack_thing(hbox, cd->panel.play_control.stop, FALSE);
        pack_thing(hbox, cd->panel.play_control.play_pause, FALSE);
        pack_thing(hbox, cd->panel.play_control.eject, FALSE);
        hbox = pack_make_hbox(cd);
        pack_thing(hbox, cd->panel.track_control.prev, FALSE);
        pack_thing(hbox, cd->panel.track_control.display, TRUE);
        pack_thing(hbox, cd->panel.track_control.next, FALSE);
        gtk_widget_show(cd->panel.box);
    }
    gtk_widget_show_all(cd->panel.frame);
}

static gboolean 
button_press_hack (GtkWidget *w, GdkEventButton *event, gpointer data)
{
    PanelApplet *applet = PANEL_APPLET (data);
    /* Pass the right click to the PanelApplet */
    if (event->button == 3) {
    	GTK_WIDGET_CLASS (PANEL_APPLET_GET_CLASS (applet))->
			  button_press_event (data, event);
	return TRUE;
    }
    
    return FALSE;
    
}

static GtkWidget *
control_button_factory(gchar * pixmap_data[], GCallback func, CDPlayerData * cd)
{
    GtkWidget *w;
    GdkPixbuf *pixbuf;
    GtkWidget *image;

    w = gtk_button_new();
    GTK_WIDGET_UNSET_FLAGS(w, GTK_CAN_DEFAULT);
    pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **)pixmap_data);
    image = gtk_image_new_from_pixbuf(pixbuf);
    if (pixbuf)
    	g_object_unref (pixbuf);
    gtk_widget_show(image);
    gtk_container_add(GTK_CONTAINER(w), image);
    /* This is a hack to get the right click menu working with buttons
    ** Should be removed when libpanelapplet is fixed */
    g_signal_connect (G_OBJECT (w), "button_press_event",
    		      G_CALLBACK (button_press_hack), cd->panel.applet);
    g_signal_connect(G_OBJECT(w), "clicked",
                     G_CALLBACK(func), cd);
    gtk_widget_show(w);
    gtk_widget_ref(w);
    return(w);
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
                    break;
                case DISC_PAUSED:
                    led_paused(cd->panel.time,
                           stat.relative_address.minute,
                           stat.relative_address.second,
                           cd->panel.track_control.display,
                           stat.track);
                    description = TRUE;
                    break;
                case DISC_COMPLETED:
                case DISC_STOP:
                case DISC_ERROR:
                    led_stop(cd->panel.time, cd->panel.track_control.display);
                    break;
                default:
                    break;
            }
        }
        cd_close(cd);
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

    if(!cd_try_open(cd))
        return;

    status = cdrom_get_status(cd->cdrom_device, &stat);
    if (status == DISC_NO_ERROR) {
        switch (stat.audio_status) {
        case DISC_PLAY:
            cdrom_pause(cd->cdrom_device);
            break;
        case DISC_PAUSED:
            cdrom_resume(cd->cdrom_device);
            break;
        case DISC_COMPLETED:
        case DISC_STOP:
        case DISC_ERROR:
            cdrom_read_track_info(cd->cdrom_device);
            cdrom_play(cd->cdrom_device, cd->cdrom_device->track0,
                   cd->cdrom_device->track1);
            break;
        }
    } else if(status == DISC_TRAY_OPEN) {
        cdrom_load(cd->cdrom_device);
        cdrom_read_track_info(cd->cdrom_device);
        cdrom_play(cd->cdrom_device, cd->cdrom_device->track0,
               cd->cdrom_device->track1);
    }

    cd_close(cd);
}

static void 
cdplayer_stop(GtkWidget * w, gpointer data)
{
    CDPlayerData *cd = data;
    if(!cd_try_open(cd))
        return;
    cdrom_stop(cd->cdrom_device);

    cd_close(cd);
}

static void 
cdplayer_prev(GtkWidget * w, gpointer data)
{
    CDPlayerData *cd = data;
    if(!cd_try_open(cd))
        return;
    cdrom_prev(cd->cdrom_device);

    cd_close(cd);
}

static void 
cdplayer_next(GtkWidget * w, gpointer data)
{
    CDPlayerData *cd = data;
    if(!cd_try_open(cd))
        return;
    cdrom_next(cd->cdrom_device);

    cd_close(cd);
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
