/* GNOME cpuload/memload panel applet
 * (C) 2002 The Free Software Foundation
 *
 * Authors: 
 *		  Todd Kulesza
 *
 *
 */

#include <gnome.h>
#include <panel-applet.h>
#include <panel-applet-gconf.h>

#include "global.h"

MultiloadApplet *multiload_applet;

void
properties_close_cb(GtkWidget *widget, gint arg, gpointer data)
{
	
	if (arg == GTK_RESPONSE_CLOSE)
		gtk_widget_destroy(widget);
	
	return;
}

void
property_toggled_cb(GtkWidget *widget, gpointer name)
{
	panel_applet_gconf_set_bool(multiload_applet->applet, (gchar *)name, 
			gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)), NULL);
	
	return;
}

void
fill_properties(GtkWidget *dialog)
{
	GtkWidget *notebook;
	GtkWidget *hbox;
	GtkWidget *check_box;
	GtkWidget *frame;
	
	frame = gtk_frame_new(_("Monitored Resources"));
	gtk_container_set_border_width(GTK_CONTAINER(frame), 5);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), frame, FALSE, FALSE, 0);
	
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 3);
	gtk_container_add(GTK_CONTAINER(frame), hbox);
	
	check_box = gtk_check_button_new_with_mnemonic(_("_Processor"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_box),
				panel_applet_gconf_get_bool(multiload_applet->applet, "view_cpuload", NULL));
	g_signal_connect(G_OBJECT(check_box), "toggled",
				G_CALLBACK(property_toggled_cb), "view_cpuload");
	gtk_box_pack_start(GTK_BOX(hbox), check_box, FALSE, FALSE, 2);
	
	check_box = gtk_check_button_new_with_mnemonic(_("_Memory"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_box),
				panel_applet_gconf_get_bool(multiload_applet->applet, "view_memload", NULL));
	g_signal_connect(G_OBJECT(check_box), "toggled",
				G_CALLBACK(property_toggled_cb), "view_memload");
	gtk_box_pack_start(GTK_BOX(hbox), check_box, FALSE, FALSE, 2);
	
	check_box = gtk_check_button_new_with_mnemonic(_("_Network"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_box),
				panel_applet_gconf_get_bool(multiload_applet->applet, "view_netload", NULL));
	g_signal_connect(G_OBJECT(check_box), "toggled",
				G_CALLBACK(property_toggled_cb), "view_netload");
	gtk_box_pack_start(GTK_BOX(hbox), check_box, FALSE, FALSE, 2);
	
	check_box = gtk_check_button_new_with_mnemonic(_("_Swap File"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_box),
				panel_applet_gconf_get_bool(multiload_applet->applet, "view_swapload", NULL));
	g_signal_connect(G_OBJECT(check_box), "toggled",
				G_CALLBACK(property_toggled_cb), "view_swapload");
	gtk_box_pack_start(GTK_BOX(hbox), check_box, FALSE, FALSE, 2);

/*	
	check_box = gtk_check_button_new_with_mnemonic(_("_Average"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_box),
				panel_applet_gconf_get_bool(multiload_applet->applet, "view_loadavg", NULL));
	gtk_box_pack_start(GTK_BOX(hbox), check_box, FALSE, FALSE, 0);
*/
	return;
}

/* show properties dialog */
void
multiload_properties_cb(BonoboUIComponent *uic, gpointer data, const gchar *name)
{
	static GtkWidget *dialog = NULL;
	
	if (dialog != NULL)
	{
	    gdk_window_show(dialog->window);
	    gdk_window_raise(dialog->window);
	    return;
	}
	
	dialog = gtk_dialog_new_with_buttons(_("SystemLoad Properties"), NULL, GTK_DIALOG_MODAL, GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE, NULL);
	
	fill_properties(dialog);
	
	g_signal_connect (G_OBJECT(dialog), "destroy",
			G_CALLBACK(gtk_widget_destroyed), &dialog);
	g_signal_connect(G_OBJECT(dialog), "response",
			G_CALLBACK(properties_close_cb), NULL);
			
	gtk_widget_show_all(dialog);
	
	return;
}
