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

#include "gswitchit-capplet.h"

static GtkWidget *grabDialog;
static GtkWidget *userDefinedMenu;
static GtkWidget *switchcutsOMenu;

static void CappletUI2Config( GSwitchItCapplet * gswic )
{
  int i = XklGetNumGroups(  );
  int mask = 1 << ( i - 1 );

  for( ; --i >= 0; mask >>= 1 )
  {
    char sz[30];
    GtkWidget *sec;

    g_snprintf( sz, sizeof( sz ), "secondary.%d", i );
    sec = GTK_WIDGET( gtk_object_get_data
                      ( GTK_OBJECT( gswic->capplet ), sz ) );

    if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( sec ) ) )
      gswic->appletConfig.secondaryGroupsMask |= mask;
    else
      gswic->appletConfig.secondaryGroupsMask &= ~mask;
  }

  gswic->appletConfig.groupPerApp =
    gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON
                                  ( CappletGetGladeWidget( gswic,
                                                           "groupPerApp" ) ) );

  gswic->appletConfig.handleIndicators =
    gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON
                                  ( CappletGetGladeWidget( gswic,
                                                           "handleIndicators" ) ) );

  gswic->appletConfig.showFlags =
    gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON
                                  ( CappletGetGladeWidget( gswic,
                                                           "showFlags" ) ) );

  gswic->appletConfig.defaultGroup =
    GPOINTER_TO_INT( gtk_object_get_data
                     ( GTK_OBJECT
                       ( gtk_menu_get_active
                         ( GTK_MENU
                           ( gtk_option_menu_get_menu
                             ( GTK_OPTION_MENU
                               ( CappletGetGladeWidget
                                 ( gswic, "defaultGroupsOMenu" ) ) ) ) ) ),
                       "group" ) );

}

static void CappletCommitConfig( GtkWidget * w, GSwitchItCapplet * gswic )
{
  CappletUI2Config( gswic );
  GSwitchItAppletConfigSave( &gswic->appletConfig, &gswic->xkbConfig );
}

static void CappletGroupPerWindowChanged( GtkWidget * w,
                                          GSwitchItCapplet * gswic )
{
  GtkWidget *handleIndicators;

  CappletCommitConfig( w, gswic );

  handleIndicators = CappletGetGladeWidget( gswic, "handleIndicators" );
  gtk_widget_set_sensitive( handleIndicators,
                            gswic->appletConfig.groupPerApp );
}

static void CappletShowFlagsChanged( GtkWidget * w, GSwitchItCapplet * gswic )
{
  CappletCommitConfig( w, gswic );
}

static void CappletSecChanged( GtkWidget * w, GSwitchItCapplet * gswic )
{
  CappletCommitConfig( w, gswic );

  if( ( gswic->appletConfig.secondaryGroupsMask + 1 ) == ( 1 << XklGetNumGroups(  ) ) ) // all secondaries?
    gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( w ), FALSE );
}

static void CappletClose( GtkDialog * capplet, GSwitchItCapplet * si )
{
  bonobo_main_quit(  );
}

static void CappletHelp( GtkWidget * w, GSwitchItCapplet * si )
{
  GSwitchItHelp( GTK_WINDOW( si->capplet ), "gswitchitPropsCapplet" );
}

static void CappletSetup( GSwitchItCapplet * gswic )
{
  GladeXML *data;
  GtkWidget *widget, *capplet;

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
    _( "Make the layout accessible from the applet popup menu ONLY.\n"
       "No way to switch to this layout using the keyboard." );

  glade_gnome_init(  );

  data = glade_xml_new( GLADE_DIR "/gswitchit-properties.glade", "gswitchit_capplet", NULL );   // default domain!
  XklDebug( 125, "data: %p\n", data );

  gswic->capplet = capplet =
    glade_xml_get_widget( data, "gswitchit_capplet" );

  iconFile = gnome_program_locate_file( NULL,
                                        GNOME_FILE_DOMAIN_PIXMAP,
                                        "gswitchit-properties-capplet.png",
                                        TRUE, NULL );
  if( iconFile != NULL )
    gtk_window_set_icon_from_file( GTK_WINDOW( capplet ), iconFile, NULL );

  gtk_object_set_data( GTK_OBJECT( capplet ), "gladeData", data );

  g_signal_connect_swapped( GTK_OBJECT( capplet ), "destroy",
                            G_CALLBACK( g_object_unref ), data );

  g_signal_connect( GTK_OBJECT( capplet ), "close",
                    G_CALLBACK( gtk_main_quit ), data );

  groupPerApp = glade_xml_get_widget( data, "groupPerApp" );
  gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( groupPerApp ),
                                gswic->appletConfig.groupPerApp );

  tooltips = gtk_tooltips_data_get( groupPerApp )->tooltips;

  g_signal_connect( G_OBJECT( groupPerApp ),
                    "toggled",
                    G_CALLBACK( CappletGroupPerWindowChanged ), gswic );

  showFlags = glade_xml_get_widget( data, "showFlags" );
  gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( showFlags ),
                                gswic->appletConfig.showFlags );

  g_signal_connect( G_OBJECT( showFlags ),
                    "toggled", G_CALLBACK( CappletShowFlagsChanged ), gswic );

  handleIndicators = glade_xml_get_widget( data, "handleIndicators" );

  gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( handleIndicators ),
                                gswic->appletConfig.handleIndicators );

  g_signal_connect( GTK_OBJECT( handleIndicators ),
                    "toggled", G_CALLBACK( CappletCommitConfig ), gswic );

  vboxFlags = glade_xml_get_widget( data, "vboxFlags" );

  GSwitchItAppletConfigLoadGroupDescriptionsUtf8( &gswic->appletConfig,
                                                  groupNames );
  groupName = ( const char * ) groupNames;
  for( i = 0; i < XklGetNumGroups(  );
       i++, groupName += sizeof( groupNames[0] ) )
  {
    gchar sz[30];
    GtkWidget *secondary;
    GtkWidget *frameFlag;
    GtkWidget *vboxPerGroup;

    frameFlag = gtk_frame_new( groupName );
    g_snprintf( sz, sizeof( sz ), "frameFlag.%d", i );
    gtk_widget_set_name( frameFlag, sz );
    gtk_object_set_data( GTK_OBJECT( capplet ), sz, frameFlag );
    //gtk_widget_show( frameFlag );

    gtk_box_pack_start( GTK_BOX( vboxFlags ), frameFlag, FALSE, FALSE, 0 );

    gtk_frame_set_shadow_type( GTK_FRAME( frameFlag ),
                               GTK_SHADOW_ETCHED_OUT );

    vboxPerGroup = gtk_vbox_new( FALSE, 0 );
    g_snprintf( sz, sizeof( sz ), "vboxPerGroup.%d", i );
    gtk_widget_set_name( vboxPerGroup, sz );
    gtk_object_set_data( GTK_OBJECT( capplet ), sz, vboxPerGroup );

    secondary =
      gtk_check_button_new_with_label( _
                                       ( "Exclude from keyboard switching" ) );
    g_snprintf( sz, sizeof( sz ), "secondary.%d", i );
    gtk_widget_set_name( secondary, sz );
    gtk_object_set_data( GTK_OBJECT( capplet ), sz, secondary );

    gtk_object_set_data( GTK_OBJECT( secondary ),
                         "idx", GINT_TO_POINTER( i ) );

    gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( secondary ),
                                  ( gswic->appletConfig.
                                    secondaryGroupsMask & ( 1 << i ) ) != 0 );
    gtk_tooltips_set_tip( tooltips, secondary,
                          secondaryTooltip, secondaryTooltip );

    gtk_box_pack_start( GTK_BOX( vboxPerGroup ), secondary, FALSE, FALSE, 0 );
    gtk_container_add( GTK_CONTAINER( frameFlag ), vboxPerGroup );

    g_signal_connect( GTK_OBJECT( secondary ), "toggled",
                      G_CALLBACK( CappletSecChanged ), gswic );
  }

  defaultGroupsMenu = gtk_menu_new(  );

  gtk_object_set_data( GTK_OBJECT( capplet ), "defaultGroupsMenu",
                       defaultGroupsMenu );

  menuItem = gtk_menu_item_new_with_label( _( "not used" ) );
  gtk_object_set_data( GTK_OBJECT( menuItem ), "group",
                       GINT_TO_POINTER( -1 ) );
  g_signal_connect( GTK_OBJECT( menuItem ), "activate",
                    G_CALLBACK( CappletCommitConfig ), gswic );

  gtk_menu_shell_append( GTK_MENU_SHELL( defaultGroupsMenu ), menuItem );
  gtk_widget_show( menuItem );
  groupName = ( const char * ) groupNames;
  for( i = 0; i < XklGetNumGroups(  );
       i++, groupName += sizeof( groupNames[0] ) )
  {
    menuItem = gtk_menu_item_new_with_label( groupName );
    gtk_object_set_data( GTK_OBJECT( menuItem ), "group",
                         GINT_TO_POINTER( i ) );
    g_signal_connect( GTK_OBJECT( menuItem ), "activate",
                      G_CALLBACK( CappletCommitConfig ), gswic );
    gtk_menu_shell_append( GTK_MENU_SHELL( defaultGroupsMenu ), menuItem );
    gtk_widget_show( menuItem );
  }

  defaultGroupsOMenu = glade_xml_get_widget( data, "defaultGroupsOMenu" );
  gtk_option_menu_set_menu( GTK_OPTION_MENU( defaultGroupsOMenu ),
                            defaultGroupsMenu );
  // initial value - ( group no + 1 )
  gtk_option_menu_set_history( GTK_OPTION_MENU( defaultGroupsOMenu ),
                               ( gswic->appletConfig.defaultGroup <
                                 XklGetNumGroups(  ) )? gswic->appletConfig.
                               defaultGroup + 1 : 0 );
  glade_xml_signal_connect_data( data, "on_btnHelp_clicked",
                                 GTK_SIGNAL_FUNC( CappletHelp ), gswic );
  glade_xml_signal_connect_data( data, "on_btnClose_clicked",
                                 GTK_SIGNAL_FUNC( CappletClose ), gswic );
  glade_xml_signal_connect_data( data, "on_gswitchit_capplet_unrealize",
                                 GTK_SIGNAL_FUNC( CappletClose ), gswic );

  CappletGroupPerWindowChanged( groupPerApp, gswic );
  CappletShowFlagsChanged( showFlags, gswic );

  gtk_widget_show_all( capplet );
}

int main( int argc, char **argv )
{
  GSwitchItCapplet gswic;
  GError *gconf_error = NULL;
  GConfClient *confClient;
  bindtextdomain( PACKAGE, GNOMELOCALEDIR );
  bind_textdomain_codeset( GETTEXT_PACKAGE, "UTF-8" );
  textdomain( PACKAGE );
  memset( &gswic, 0, sizeof( gswic ) );

  gnome_program_init( "gswitchit-properties", VERSION, LIBGNOMEUI_MODULE,
                      argc, argv, GNOME_PARAM_NONE );

  if( !gconf_init( argc, argv, &gconf_error ) )
  {
    g_warning( _( "Failed to init GConf: %s\n" ), gconf_error->message );
    g_error_free( gconf_error );
    return 1;
  }
  gconf_error = NULL;

  //GSwitchItInstallGlibLogAppender(  );

  XklInit( GDK_DISPLAY(  ) );
  XklConfigInit(  );
  XklConfigLoadRegistry(  );
  confClient = gconf_client_get_default(  );
  GSwitchItXkbConfigInit( &gswic.xkbConfig, confClient );
  GSwitchItAppletConfigInit( &gswic.appletConfig, confClient );
  g_object_unref( confClient );
  GSwitchItXkbConfigLoad( &gswic.xkbConfig );
  GSwitchItAppletConfigLoad( &gswic.appletConfig );
  CappletSetup( &gswic );
  bonobo_main(  );
  GSwitchItAppletConfigTerm( &gswic.appletConfig );
  GSwitchItXkbConfigTerm( &gswic.xkbConfig );
  XklConfigFreeRegistry(  );
  XklConfigTerm(  );
  XklTerm(  );

  return 0;
}
