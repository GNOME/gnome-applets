/* Funky PPP Dialer Applet! v0.2.2
 * (Why hasn't anyone thought of it before!?!?)
 *
 * By Alex Roberts, 1998
 */

#include <config.h>
#include <gnome.h>
#include <applet-widget.h>

#include <signal.h>

#include "dialer.h"
#include "images/dialer-start.xpm"
#include "images/dialer-stop.xpm"

GtkWidget *applet;
GtkWidget *label;

static int dialer_stop(GtkWidget * w, gpointer data)
{
	/* Execute ppp-off (or 'killall pppd', take your pick, when i get Properties
					 working ;-)*/
	system("/etc/ppp/ppp-off");
	gtk_label_set_text(GTK_LABEL(label), "Off");
	return 0;
}

static int dialer_start(GtkWidget * w, gpointer data)
{
	/* Execute ppp-up */
	system("/etc/ppp/ppp-on");
	gtk_label_set_text(GTK_LABEL(label), "On");
	return 0;
}


static void about_cb (AppletWidget *widget, gpointer data)
{
	GtkWidget *about;
	static const gchar *authors[] = {
		"Alex Roberts (bse@dial.pipex.com)",
		NULL
		};

	about = gnome_about_new (_("PPP Dialer Applet"), "0.2.2",
			"(C) 1998 the Free Software Foundation",
			authors,
			_("A funky PPP dialer, how come no-one has thought of it before!?"),
			NULL);
	gtk_widget_show (about);

	return;
}


static GtkWidget *control_button_factory(GtkWidget * box_container, gchar * pixmap_data[], 
								 int (*func) (GtkWidget *, gpointer data),
		       					DialButtons * but)
{
	GtkWidget *w, *pixmap;

	w = gtk_button_new();
	GTK_WIDGET_UNSET_FLAGS(w, GTK_CAN_DEFAULT);
	GTK_WIDGET_UNSET_FLAGS(w, GTK_CAN_FOCUS);
	pixmap = gnome_pixmap_new_from_xpm_d (pixmap_data);
	gtk_box_pack_start(GTK_BOX(box_container), w, FALSE, TRUE, 0);
	gtk_widget_show(pixmap);
	gtk_container_add(GTK_CONTAINER(w), pixmap);
	gtk_signal_connect(GTK_OBJECT(w), "clicked",
			   GTK_SIGNAL_FUNC(func),
			   (gpointer) but);
	gtk_widget_show(w);
	return w;
}

static GtkWidget *create_dialer_panel(GtkWidget *window, DialButtons *but)
{
	GtkWidget *frame;
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *separator;

	frame = gtk_frame_new(NULL);
        gtk_frame_set_shadow_type(GTK_FRAME(frame),GTK_SHADOW_IN);
        
        label = gtk_label_new(_("Off"));
        gtk_widget_show(label);
        
	vbox = gtk_vbox_new(FALSE, FALSE);
	gtk_container_add(GTK_CONTAINER(frame), vbox);
	gtk_widget_show(vbox);

	gtk_box_pack_start(GTK_BOX(vbox),label, TRUE, TRUE, 0);

      separator = gtk_hseparator_new ();
      gtk_box_pack_start (GTK_BOX (vbox), separator, FALSE, TRUE, 0);
      gtk_widget_show (separator);


	hbox = gtk_hbox_new(FALSE, FALSE);
	gtk_box_pack_start_defaults(GTK_BOX(vbox), hbox);
	gtk_widget_show(hbox);

	but->stop = control_button_factory(hbox, stop_xpm,
						dialer_stop, but);
	but->start = control_button_factory(hbox, play_pause_xpm,
						dialer_start, but);



	/* label = gtk_label_new(_("Click to Connect!"));
	gtk_widget_show(label);*/
	
	/*applet_widget_add (APPLET_WIDGET (applet), label);
	
	gtk_widget_show(applet);*/
	

        
        return frame;
}

static GtkWidget *create_dialer_applet(GtkWidget *window)
{

	GtkWidget *dpanel;
	DialButtons *buttons;
	
	buttons = g_new(DialButtons, 1);

	dpanel = create_dialer_panel(window,buttons);
	
	return dpanel;
}

int main(int argc, char **argv)
{
	GtkWidget *dialer;
	
	/* i18n stuff */
	bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain (PACKAGE);
	
	applet_widget_init ("dialer_applet", VERSION, argc, argv, NULL, 0, NULL);
	
	applet = applet_widget_new("dialer_applet");
	if(!applet) g_print("Cannot create applet!\n");
	
	gtk_widget_realize(applet);
	
	dialer = create_dialer_applet(applet);
	gtk_widget_show(dialer);
	
	applet_widget_add (APPLET_WIDGET(applet), dialer);
	gtk_widget_show (applet);
	
	
	applet_widget_register_stock_callback(APPLET_WIDGET(applet),
					      "about",
					      GNOME_STOCK_MENU_ABOUT,
					      _("About..."),
					      about_cb,
					      NULL);

	
	applet_widget_gtk_main();
	
	return 0;
}
