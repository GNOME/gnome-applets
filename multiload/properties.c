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
	MultiloadApplet *ma;
	gboolean active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	
	ma = g_object_get_data(G_OBJECT(widget), "user_data");
	
	/* FIXME: the first toggle button to be checked/dechecked does not work, but after that everything is cool.  what gives? */
	panel_applet_gconf_set_bool(ma->applet, (gchar *)name, 
			active, NULL);
	
	panel_applet_gconf_set_bool(ma->applet, (gchar *)name, 
			active, NULL);
			
	multiload_applet_refresh(ma);
	
	return;
}

void
spin_button_changed_cb(GtkWidget *widget, gpointer name)
{
	MultiloadApplet *ma;
	gint value;
	
	ma = g_object_get_data(G_OBJECT(widget), "user_data");
	value = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
	
	/* FIXME: the first toggle button to be checked/dechecked does not work, but after that everything is cool.  what gives? */
	panel_applet_gconf_set_int(ma->applet, (gchar *)name, 
			value, NULL);
	
	panel_applet_gconf_set_int(ma->applet, (gchar *)name, 
			value, NULL);
	
	multiload_applet_refresh(ma);
	
	return;
}
	
/* creates the properties dialog using up-to-the-minute info from gconf */
void
fill_properties(GtkWidget *dialog, MultiloadApplet *ma)
{
	GtkWidget *notebook;
	GtkWidget *hbox, *vbox;
	GtkWidget *check_box;
	GtkWidget *frame;
	GtkWidget *spin_button;
	GtkWidget *label;
	PanelAppletOrient orient;
	gchar *label_text;
	
	frame = gtk_frame_new(_("Monitored Resources"));
	gtk_container_set_border_width(GTK_CONTAINER(frame), 5);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), frame, FALSE, FALSE, 0);
	
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 3);
	gtk_container_add(GTK_CONTAINER(frame), hbox);
	
	check_box = gtk_check_button_new_with_mnemonic(_("_Processor"));
	
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_box),
				panel_applet_gconf_get_bool(ma->applet, "view_cpuload", NULL));
	g_object_set_data(G_OBJECT(check_box), "user_data", ma);
	g_signal_connect(G_OBJECT(check_box), "toggled",
				G_CALLBACK(property_toggled_cb), "view_cpuload");
	gtk_box_pack_start(GTK_BOX(hbox), check_box, FALSE, FALSE, 2);
	
	check_box = gtk_check_button_new_with_mnemonic(_("_Memory"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_box),
				panel_applet_gconf_get_bool(ma->applet, "view_memload", NULL));
	g_object_set_data(G_OBJECT(check_box), "user_data", ma);
	g_signal_connect(G_OBJECT(check_box), "toggled",
				G_CALLBACK(property_toggled_cb), "view_memload");
	gtk_box_pack_start(GTK_BOX(hbox), check_box, FALSE, FALSE, 2);
	
	check_box = gtk_check_button_new_with_mnemonic(_("_Network"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_box),
				panel_applet_gconf_get_bool(ma->applet, "view_netload", NULL));
	g_object_set_data(G_OBJECT(check_box), "user_data", ma);
	g_signal_connect(G_OBJECT(check_box), "toggled",
				G_CALLBACK(property_toggled_cb), "view_netload");
	gtk_box_pack_start(GTK_BOX(hbox), check_box, FALSE, FALSE, 2);
	
	check_box = gtk_check_button_new_with_mnemonic(_("_Swap File"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_box),
				panel_applet_gconf_get_bool(ma->applet, "view_swapload", NULL));
	g_object_set_data(G_OBJECT(check_box), "user_data", ma);
	g_signal_connect(G_OBJECT(check_box), "toggled",
				G_CALLBACK(property_toggled_cb), "view_swapload");
	gtk_box_pack_start(GTK_BOX(hbox), check_box, FALSE, FALSE, 2);

	frame = gtk_frame_new(_("Options"));
	gtk_container_set_border_width(GTK_CONTAINER(frame), 5);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), frame, FALSE, FALSE, 0);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
	gtk_container_add(GTK_CONTAINER(frame), vbox);
	
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 1);
	
	orient = panel_applet_get_orient(ma->applet);
	if ( (orient == PANEL_APPLET_ORIENT_UP) || (orient == PANEL_APPLET_ORIENT_DOWN) )
		label_text = g_strdup(_("Load monitor width"));
	else
		label_text = g_strdup(_("Load monitor height"));
	
	label = gtk_label_new(label_text);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 3);
	spin_button = gtk_spin_button_new_with_range(0, 1000, 5);
	g_object_set_data(G_OBJECT(spin_button), "user_data", ma);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_button),
				(gdouble)panel_applet_gconf_get_int(ma->applet, "size", NULL));
	g_signal_connect(G_OBJECT(spin_button), "value_changed",
				G_CALLBACK(spin_button_changed_cb), "size");
	gtk_box_pack_start(GTK_BOX(hbox), spin_button, FALSE, FALSE, 3);
	
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 1);
	
	label = gtk_label_new(_("Load monitor speed"));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 3);
	spin_button = gtk_spin_button_new_with_range(0, 9999, 10);
	g_object_set_data(G_OBJECT(spin_button), "user_data", ma);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_button),
				(gdouble)panel_applet_gconf_get_int(ma->applet, "speed", NULL));
	g_signal_connect(G_OBJECT(spin_button), "value_changed",
				G_CALLBACK(spin_button_changed_cb), "speed");
	gtk_box_pack_start(GTK_BOX(hbox), spin_button, FALSE, FALSE, 3);
	
/*	
	check_box = gtk_check_button_new_with_mnemonic(_("_Average"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_box),
				panel_applet_gconf_get_bool(multiload_applet->applet, "view_loadavg", NULL));
	gtk_box_pack_start(GTK_BOX(hbox), check_box, FALSE, FALSE, 0);
*/
	g_free(label_text);
	
	return;
}

/* show properties dialog */
void
multiload_properties_cb(BonoboUIComponent *uic, gpointer data, const gchar *name)
{
	MultiloadApplet *ma;
	static GtkWidget *dialog = NULL;
	
	if (dialog != NULL)
	{
	    gdk_window_show(dialog->window);
	    gdk_window_raise(dialog->window);
	    return;
	}
	
	ma = (MultiloadApplet *)data;
	
	dialog = gtk_dialog_new_with_buttons(_("SystemLoad Properties"), NULL, GTK_DIALOG_MODAL, GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE, NULL);
				
	fill_properties(dialog, ma);
	
	g_signal_connect (G_OBJECT(dialog), "destroy",
			G_CALLBACK(gtk_widget_destroyed), &dialog);
	g_signal_connect(G_OBJECT(dialog), "response",
			G_CALLBACK(properties_close_cb), NULL);
			
	gtk_widget_show_all(dialog);
	
	return;
}
