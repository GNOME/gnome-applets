/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <config.h>

#include <sys/stat.h>

#include <gdk/gdkx.h>
#include <gnome.h>
#include <glade/glade.h>
#include <libbonobo.h>

#include <libxklavier/xklavier.h>
#include <libxklavier/xklavier_config.h>

#include <libgswitchit/gswitchit_util.h>

#include "gswitchit-applet.h"

#define GLADE_DATA_PROP "gladeData"

#define CappletGetGladeWidget( sia, name ) \
  glade_xml_get_widget( \
    GLADE_XML( g_object_get_data( G_OBJECT( (sia)->propsDialog ), \
                                  GLADE_DATA_PROP ) ), \
    name )

static GtkWidget *
create_hig_catagory (GtkWidget *main_box, const char *title)
{
	GtkWidget *vbox, *vbox2, *hbox;
	GtkWidget *label;
	gchar *tmp;
	
	vbox = gtk_vbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (main_box), vbox, FALSE, FALSE, 0);

	tmp = g_strdup_printf ("<b>%s</b>", title);
	label = gtk_label_new (NULL);
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_label_set_markup (GTK_LABEL (label), tmp);
	g_free (tmp);
	gtk_box_pack_start (GTK_BOX (vbox), label, TRUE, FALSE, 0);
	
	hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
	
	label = gtk_label_new ("    ");
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	
	vbox2 = gtk_vbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (hbox), vbox2, TRUE, TRUE, 0);

	return vbox2;		
}

static void
CappletUI2Config (GSwitchItApplet * sia)
{
	int i = XklGetNumGroups ();
	int mask = 1 << (i - 1);

	for (; --i >= 0; mask >>= 1) {
		char sz[30];
		GtkWidget *sec;

		g_snprintf (sz, sizeof (sz), "secondary.%d", i);
		sec = GTK_WIDGET (gtk_object_get_data
				  (GTK_OBJECT (sia->propsDialog), sz));

		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (sec)))
			sia->appletConfig.secondaryGroupsMask |= mask;
		else
			sia->appletConfig.secondaryGroupsMask &= ~mask;
	}

	sia->appletConfig.groupPerApp =
	    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
					  (CappletGetGladeWidget (sia,
								  "groupPerApp")));

	sia->appletConfig.handleIndicators =
	    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
					  (CappletGetGladeWidget (sia,
								  "handleIndicators")));

	sia->appletConfig.showFlags =
	    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
					  (CappletGetGladeWidget (sia,
								  "showFlags")));

	sia->appletConfig.defaultGroup =
	    GPOINTER_TO_INT (gtk_object_get_data
			     (GTK_OBJECT
			      (gtk_menu_get_active
			       (GTK_MENU
				(gtk_option_menu_get_menu
				 (GTK_OPTION_MENU
				  (CappletGetGladeWidget
				   (sia, "defaultGroupsOMenu")))))),
			      "group"));

}

static void
CappletCommitConfig (GtkWidget * w, GSwitchItApplet * sia)
{
	CappletUI2Config (sia);
	GSwitchItAppletConfigSave (&sia->appletConfig, &sia->xkbConfig);
}

static void
CappletGroupPerWindowChanged (GtkWidget * w, GSwitchItApplet * sia)
{
	GtkWidget *handleIndicators;

	CappletCommitConfig (w, sia);

	handleIndicators = CappletGetGladeWidget (sia, "handleIndicators");
	gtk_widget_set_sensitive (handleIndicators,
				  sia->appletConfig.groupPerApp);
}

static void
CappletShowFlagsChanged (GtkWidget * w, GSwitchItApplet * sia)
{
	CappletCommitConfig (w, sia);
}

static void
CappletSecChanged (GtkWidget * w, GSwitchItApplet * sia)
{
	CappletCommitConfig (w, sia);

	if ((sia->appletConfig.secondaryGroupsMask + 1) == (1 << XklGetNumGroups ()))	// all secondaries?
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w),
					      FALSE);
}

static void
CappletResponse (GtkDialog * capplet, gint id, GSwitchItApplet * sia)
{
	if (id == GTK_RESPONSE_HELP) {
		GSwitchItHelp (GTK_WINDOW (sia->propsDialog),
			       "gswitchitPropsCapplet");
		return;
	}
	gtk_widget_destroy (GTK_WIDGET (capplet));
	sia->propsDialog = NULL;
}

void
GSwitchItAppletPropsCreate (GSwitchItApplet * sia)
{
	GladeXML *data;
	GtkWidget *capplet;

	GtkWidget *groupPerApp;
	GtkWidget *handleIndicators;
	GtkWidget *showFlags;
	GtkWidget *menuItem;
	GtkWidget *vboxFlags;
	GtkWidget *defaultGroupsOMenu;
	GtkWidget *defaultGroupsMenu;
	int i;
	GtkTooltips *tooltips;
	const char *groupName, *iconFile;
	GroupDescriptionsBuffer groupNames;

	const char *secondaryTooltip =
	    _
	    ("Make the layout accessible from the applet popup menu ONLY.\n"
	     "No way to switch to this layout using the keyboard.");

	data = glade_xml_new (GLADE_DIR "/gswitchit-properties.glade", "gswitchit_capplet", NULL);	// default domain!
	XklDebug (125, "data: %p\n", data);

	sia->propsDialog = capplet =
	    glade_xml_get_widget (data, "gswitchit_capplet");

	iconFile = gnome_program_locate_file (NULL,
					      GNOME_FILE_DOMAIN_PIXMAP,
					      "gswitchit-properties-capplet.png",
					      TRUE, NULL);
	if (iconFile != NULL)
		gtk_window_set_icon_from_file (GTK_WINDOW (capplet),
					       iconFile, NULL);

	gtk_object_set_data (GTK_OBJECT (capplet), GLADE_DATA_PROP, data);

	gtk_dialog_set_default_response (GTK_DIALOG (capplet),
					 GTK_RESPONSE_CLOSE);

	g_signal_connect_swapped (GTK_OBJECT (capplet), "destroy",
				  G_CALLBACK (g_object_unref), data);

	groupPerApp = glade_xml_get_widget (data, "groupPerApp");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (groupPerApp),
				      sia->appletConfig.groupPerApp);

	tooltips = gtk_tooltips_data_get (groupPerApp)->tooltips;

	g_signal_connect (G_OBJECT (groupPerApp),
			  "toggled",
			  G_CALLBACK (CappletGroupPerWindowChanged), sia);

	showFlags = glade_xml_get_widget (data, "showFlags");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (showFlags),
				      sia->appletConfig.showFlags);
	g_signal_connect (G_OBJECT (showFlags),
			  "toggled", G_CALLBACK (CappletShowFlagsChanged),
			  sia);

	handleIndicators = glade_xml_get_widget (data, "handleIndicators");

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (handleIndicators),
				      sia->appletConfig.handleIndicators);

	g_signal_connect (GTK_OBJECT (handleIndicators),
			  "toggled", G_CALLBACK (CappletCommitConfig),
			  sia);

	vboxFlags = glade_xml_get_widget (data, "gswitchit_capplet_vbox");

	GSwitchItAppletConfigLoadGroupDescriptionsUtf8 (&sia->appletConfig,
							groupNames);
	groupName = (const char *) groupNames;
	for (i = 0; i < XklGetNumGroups ();
	     i++, groupName += sizeof (groupNames[0])) {
		gchar sz[30];
		GtkWidget *secondary;
		GtkWidget *frameFlag;
		GtkWidget *vboxPerGroup;

		frameFlag = create_hig_catagory (vboxFlags, groupName);
		g_snprintf (sz, sizeof (sz), "frameFlag.%d", i);
		gtk_widget_set_name (frameFlag, sz);
		gtk_object_set_data (GTK_OBJECT (capplet), sz, frameFlag);
		//gtk_widget_show( frameFlag );

		gtk_frame_set_shadow_type (GTK_FRAME (frameFlag),
					   GTK_SHADOW_ETCHED_OUT);

		vboxPerGroup = gtk_vbox_new (FALSE, 0);
		g_snprintf (sz, sizeof (sz), "vboxPerGroup.%d", i);
		gtk_widget_set_name (vboxPerGroup, sz);
		gtk_object_set_data (GTK_OBJECT (capplet), sz,
				     vboxPerGroup);

		secondary =
		    gtk_check_button_new_with_mnemonic (_
						     ("_Exclude from keyboard switching"));
		g_snprintf (sz, sizeof (sz), "secondary.%d", i);
		gtk_widget_set_name (secondary, sz);
		gtk_object_set_data (GTK_OBJECT (capplet), sz, secondary);

		gtk_object_set_data (GTK_OBJECT (secondary),
				     "idx", GINT_TO_POINTER (i));

		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
					      (secondary),
					      (sia->appletConfig.
					       secondaryGroupsMask & (1 <<
								      i))
					      != 0);
		gtk_tooltips_set_tip (tooltips, secondary,
				      secondaryTooltip, secondaryTooltip);

		gtk_box_pack_start (GTK_BOX (vboxPerGroup), secondary,
				    FALSE, FALSE, 0);
		gtk_container_add (GTK_CONTAINER (frameFlag),
				   vboxPerGroup);

		g_signal_connect (GTK_OBJECT (secondary), "toggled",
				  G_CALLBACK (CappletSecChanged), sia);
	}

	defaultGroupsMenu = gtk_menu_new ();

	gtk_object_set_data (GTK_OBJECT (capplet), "defaultGroupsMenu",
			     defaultGroupsMenu);

	menuItem = gtk_menu_item_new_with_label (_("Not used"));
	gtk_object_set_data (GTK_OBJECT (menuItem), "group",
			     GINT_TO_POINTER (-1));
	g_signal_connect (GTK_OBJECT (menuItem), "activate",
			  G_CALLBACK (CappletCommitConfig), sia);

	gtk_menu_shell_append (GTK_MENU_SHELL (defaultGroupsMenu),
			       menuItem);
	gtk_widget_show (menuItem);
	groupName = (const char *) groupNames;
	for (i = 0; i < XklGetNumGroups ();
	     i++, groupName += sizeof (groupNames[0])) {
		menuItem = gtk_menu_item_new_with_label (groupName);
		gtk_object_set_data (GTK_OBJECT (menuItem), "group",
				     GINT_TO_POINTER (i));
		g_signal_connect (GTK_OBJECT (menuItem), "activate",
				  G_CALLBACK (CappletCommitConfig), sia);
		gtk_menu_shell_append (GTK_MENU_SHELL (defaultGroupsMenu),
				       menuItem);
		gtk_widget_show (menuItem);
	}

	defaultGroupsOMenu =
	    glade_xml_get_widget (data, "defaultGroupsOMenu");
	gtk_option_menu_set_menu (GTK_OPTION_MENU (defaultGroupsOMenu),
				  defaultGroupsMenu);
	// initial value - ( group no + 1 )
	gtk_option_menu_set_history (GTK_OPTION_MENU (defaultGroupsOMenu),
				     (sia->appletConfig.defaultGroup <
				      XklGetNumGroups ())? sia->
				     appletConfig.defaultGroup + 1 : 0);

	g_signal_connect (G_OBJECT (capplet), "response",
			  G_CALLBACK (CappletResponse), sia);

	CappletGroupPerWindowChanged (groupPerApp, sia);
	CappletShowFlagsChanged (showFlags, sia);

	gtk_widget_show_all (capplet);
#ifndef ENABLE_FLAGS
	gtk_widget_hide (showFlags);
#endif

}
