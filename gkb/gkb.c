#include <config.h>
#include <string.h>
#include <gnome.h>
#include <gdk_imlib.h>
#include <applet-widget.h>

typedef struct _gkb_properties gkb_properties;
struct _gkb_properties {
	char *command;
	char *image[2];
        char *dmap[2];
	int  curpix;
};

static gkb_properties defaults = {
	"xmodmap",
	{ NULL, NULL },
        { "us", "hu" },
	0
};

typedef struct _GKB GKB;
struct _GKB {
	gkb_properties properties;
	GtkWidget *applet;
	GtkWidget *frame;
	GtkWidget *darea;
	GtkWidget *aboutbox;
        GdkImlibImage *pix[2];
	GtkWidget *propbox;                                               
	int curpix;
};

	char *basemaps[36]= {
	"be", "bg", "ch", "cz", "de", "dk", "dvorak",
	"es", "fi", "fr", "gr", "hu", "il", "is", 
	"it", "la", "nl", "no", "pl", "pt", "qc", 
	"ru", "ru_koi8", "se", "sf", "sg", "si", "sk", 
	"th", "tr_f", "tr_q", "uk", "us", "yu", 0 
	};

static	gkb_properties temp_props;

static void
load_properties(GKB *gkb)
{
	char buf[256];
	
	gnome_config_push_prefix(APPLET_WIDGET(gkb->applet)->privcfgpath);
	g_free(gkb->properties.command);
	g_snprintf(buf,256,"gkb/name=%s",defaults.command);
	gkb->properties.command = gnome_config_get_string(buf);
	g_free(gkb->properties.image[0]);
	g_snprintf(buf,256,"gkb/image0=%s",gnome_unconditional_pixmap_file("gkb/us.xpm"));
	gkb->properties.image[0] = gnome_config_get_string(buf);
	g_free(gkb->properties.image[1]);
	g_snprintf(buf,256,"gkb/image1=%s",gnome_unconditional_pixmap_file("gkb/hu.xpm"));
	gkb->properties.image[1] = gnome_config_get_string(buf);
	g_free(gkb->properties.dmap[0]);
	g_snprintf(buf,256,"gkb/map0=%s",defaults.dmap[0]);
	gkb->properties.dmap[0] = gnome_config_get_string(buf);
	g_free(gkb->properties.dmap[1]);
	g_snprintf(buf,256,"gkb/map1=%s",defaults.dmap[1]);
	gkb->properties.dmap[1] = gnome_config_get_string(buf);
	gnome_config_pop_prefix();

	if(gkb->pix[0])
		gdk_imlib_destroy_image(gkb->pix[0]);
	if(gkb->pix[1])
		gdk_imlib_destroy_image(gkb->pix[1]);
	
	gkb->pix[0] = gdk_imlib_load_image(gkb->properties.image[0]);
	gkb->pix[1] = gdk_imlib_load_image(gkb->properties.image[1]);
        gdk_imlib_render (gkb->pix[0], gkb->pix[0]->rgb_width, gkb->pix[0]->rgb_height);
        gdk_imlib_render (gkb->pix[1], gkb->pix[1]->rgb_width, gkb->pix[1]->rgb_height);
}

static void
apply_cb(GnomePropertyBox * pb,
	GKB * gkb)
{
        gnome_property_box_changed(GNOME_PROPERTY_BOX(gkb->propbox));
}

static void
apply_callback(GtkWidget * pb,
	gint page,
	GKB * gkb)
{
        memcpy( &gkb->properties, &temp_props, sizeof(gkb_properties) );
}

static void
ch_xkb_cb(GtkWidget *widget,
	GKB * gkb)
{
    	temp_props.command = g_strdup("setxkbmap");
        gnome_property_box_changed(GNOME_PROPERTY_BOX(gkb->propbox)); 
}
static void 
ch_xmodmap_cb(GtkWidget *widget,
	GKB * gkb)
{
        temp_props.command = g_strdup("xmodmap");
        gnome_property_box_changed(GNOME_PROPERTY_BOX(gkb->propbox));
}

int
destroy_cb(GtkWidget *widget,
	GKB * gkb )
{
    	g_free(gkb->propbox);
	return FALSE;
}

/*
 * That's the prop-dialog ,,,
 * */


void
properties_dialog(AppletWidget *applet,
	GKB * gkb)
{
        static GnomeHelpMenuEntry help_entry = { NULL, "properties" };
        GtkWidget *vbox3;
        GtkWidget *hbox5;          
        GtkWidget *hbox6;          
        GtkWidget *hbox8;          
        GtkWidget *entry_1;        
        GtkWidget *entry_2;        
        GtkWidget *combo1;         
        GtkWidget *combo2;         
        GList *combo1_items = NULL;
        GList *combo2_items = NULL;
        GtkWidget *g2_menuitem;    
        GtkWidget *option1;        
        GtkWidget *option2;        
        GtkWidget *option1_menu;   
        GtkWidget *option2_menu;
        GtkWidget *table1;
        GtkWidget *table2;
        GtkWidget *frame2;
	int i;

	help_entry.name = gnome_app_id;

        if( gkb->propbox ) {
         gdk_window_raise(gkb->propbox->window);
         return;
        }
   
        memcpy(&temp_props,&gkb->properties,sizeof(gkb_properties));
   
        gkb->propbox = gnome_property_box_new();
        gtk_window_set_title(GTK_WINDOW(gkb->propbox), 
           _("GKB settings")
        );
        gtk_window_set_policy(GTK_WINDOW(gkb->propbox), FALSE, FALSE, TRUE);
   
        vbox3 = gtk_hbox_new (FALSE, 0);
        
        hbox6 = gtk_vbox_new (FALSE, 0);
        gtk_box_pack_start (GTK_BOX (vbox3), hbox6, TRUE, TRUE, 0);
   
        table1 = gtk_table_new(2,2,FALSE);
        gtk_container_set_border_width(GTK_CONTAINER (table1), 3);
   
        entry_1 = create_icon_entry(table1,"tile_file1",1,
            			_("Flag One"),
                         		temp_props.image[0],
                         		gkb->propbox);
        
        gtk_box_pack_start (GTK_BOX (hbox6), table1, TRUE, TRUE, 0);
   
        hbox8 = gtk_vbox_new (FALSE, 0);
        gtk_box_pack_start (GTK_BOX (hbox6), hbox8, TRUE, TRUE, 0);
   
        combo1 = gtk_combo_new ();
        gtk_box_pack_start (GTK_BOX (hbox8), combo1, FALSE, FALSE, 30);
        gtk_container_set_border_width (GTK_CONTAINER (combo1), 2);


	combo1_items = g_list_append (combo1_items, temp_props.dmap[0]);
	
 	i=0;
	while ( basemaps[i] ){
	combo1_items = g_list_append (combo1_items, basemaps[i]);
	i++;
	}
        
	gtk_combo_set_popdown_strings (GTK_COMBO (combo1), combo1_items);
   
        gtk_signal_connect (GTK_OBJECT (combo1_items), "changed",
                 	    (GtkSignalFunc) apply_cb, gkb);

	g_list_free (combo1_items);
   
        table2 = gtk_table_new(2,2,FALSE);
        gtk_container_set_border_width(GTK_CONTAINER (table2), 3);
   
        entry_2 = create_icon_entry(table2,"tile_file2",1,
                                	        _("Flag Two"),
                     			temp_props.image[1],
                         		gkb->propbox);
        
        gtk_box_pack_start (GTK_BOX (hbox8), table2, TRUE, TRUE, 0);
   
        combo2 = gtk_combo_new ();
        gtk_box_pack_start (GTK_BOX (hbox8), combo2, FALSE, FALSE, 30);
        gtk_container_set_border_width (GTK_CONTAINER (combo2), 2);
        GTK_WIDGET_SET_FLAGS (combo2, GTK_CAN_FOCUS);
   
	combo2_items = g_list_append (combo2_items, temp_props.dmap[1]);
 	
	i=0;
	while ( basemaps[i] ){
	combo2_items = g_list_append (combo2_items, basemaps[i]);
	i++;
	}
   
        gtk_combo_set_popdown_strings (GTK_COMBO (combo2), combo2_items);
        gtk_signal_connect (GTK_OBJECT (combo2_items), "changed",
                 	    (GtkSignalFunc) apply_cb, gkb);
        g_list_free (combo2_items);
   
        hbox5 = gtk_vbox_new (FALSE, 0);
        gtk_box_pack_start (GTK_BOX (vbox3), hbox5, TRUE, TRUE, 0);
   
        frame2 = gtk_frame_new ("Program");
        gtk_box_pack_start (GTK_BOX (hbox5), frame2, FALSE, FALSE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (frame2), 8);
   
        option1 = gtk_option_menu_new ();
        gtk_container_add (GTK_CONTAINER (frame2), option1);
        gtk_widget_set_usize (option1, 95, 20);
        GTK_WIDGET_SET_FLAGS (option1, GTK_CAN_DEFAULT);
   
        option1_menu = gtk_menu_new ();
        g2_menuitem = gtk_menu_item_new_with_label ("Xkb");
        gtk_menu_append (GTK_MENU (option1_menu), g2_menuitem);
        gtk_signal_connect (GTK_OBJECT (g2_menuitem), "activate",
				(GtkSignalFunc) ch_xkb_cb, gkb);
        g2_menuitem = gtk_menu_item_new_with_label ("Xmodmap");
        gtk_menu_append (GTK_MENU (option1_menu), g2_menuitem);
        gtk_signal_connect (GTK_OBJECT (g2_menuitem), "activate",
				(GtkSignalFunc) ch_xmodmap_cb, gkb);
        gtk_option_menu_set_menu (GTK_OPTION_MENU (option1),
				 option1_menu);
        gtk_option_menu_set_history (GTK_OPTION_MENU (option1), 1);                                                                                                                                              
   
        gtk_notebook_append_page (GTK_NOTEBOOK(
		    		  GNOME_PROPERTY_BOX(gkb->propbox)->notebook),
                         	  vbox3, gtk_label_new (
                         	  _("Menu")
                         	  ));
   
        gtk_signal_connect( GTK_OBJECT(gkb->propbox),
             "apply", GTK_SIGNAL_FUNC(apply_callback), gkb );
        gtk_signal_connect( GTK_OBJECT(gkb->propbox),
             "destroy", GTK_SIGNAL_FUNC(destroy_cb), gkb );
        gtk_signal_connect( GTK_OBJECT(gkb->propbox),
             "help", GTK_SIGNAL_FUNC(gnome_help_pbox_display), help_entry );
                         
        gtk_widget_show_all(gkb->propbox);
}

static void
gkb_draw(GtkWidget *darea,
	GKB *gkb)
{
	if(!GTK_WIDGET_REALIZED(gkb->darea))
		return;
	gdk_draw_pixmap(gkb->darea->window,
		gkb->darea->style->fg_gc[GTK_WIDGET_STATE(gkb->darea)],
		gkb->pix[gkb->properties.curpix]->pixmap,
		0, 0, 0, 0, -1, -1);
}

static void
do_that_command(GKB *gkb)
{
       char comm[100];
       int len; 

       len=(strlen(gkb->properties.command)+(strcmp(gkb->properties.command,"xmodmap")?11:                                    
           strlen(gnome_datadir_file(g_strconcat ("xmodmap/",
           "xmodmap.", gkb->properties.dmap[gkb->properties.curpix], NULL)) )+7));
       g_snprintf(comm, len, "%s %s%c", gkb->properties.command,
       (strcmp(gkb->properties.command,"xmodmap")?gkb->properties.dmap[gkb->properties.curpix]:
       gnome_datadir_file(g_strconcat ("xmodmap/",
           "xmodmap.", gkb->properties.dmap[gkb->properties.curpix], NULL)) ),0);
       system(comm);
}

static void 
gkb_cb(GtkWidget * widget,
	GdkEventButton * e, 
	GKB * gkb)
{
        if (e->button != 1) {
		/* Ignore buttons 2 and 3 */
		return; 
	}
	gkb->properties.curpix++;
	if(gkb->properties.curpix>=2) gkb->properties.curpix=0;
	gkb_draw(GTK_WIDGET(gkb->darea),gkb);

	do_that_command(gkb);
}

static int
gkb_expose(GtkWidget *darea,
	GdkEventExpose *event,
	GKB *gkb)
{
	gdk_draw_pixmap(gkb->darea->window,
			gkb->darea->style->fg_gc[GTK_WIDGET_STATE(
						 gkb->darea)],
			gkb->pix[gkb->properties.curpix]->pixmap,
			event->area.x, event->area.y,
			event->area.x, event->area.y,
			event->area.width, event->area.height);
        return FALSE;
}

static void
create_gkb_widget(GKB *gkb)
{
	GtkStyle *style;

	gtk_widget_push_visual (gdk_imlib_get_visual ());
	gtk_widget_push_colormap (gdk_imlib_get_colormap ());
	style = gtk_widget_get_style(gkb->applet);
	
	gkb->darea = gtk_drawing_area_new();
	gtk_drawing_area_size(GTK_DRAWING_AREA(gkb->darea),
			      gkb->pix[gkb->properties.curpix]->rgb_width,
			      gkb->pix[gkb->properties.curpix]->rgb_height);
	gtk_widget_set_events(gkb->darea, 
			      gtk_widget_get_events(gkb->darea) |
			      GDK_BUTTON_PRESS_MASK);
	gtk_signal_connect(GTK_OBJECT(gkb->darea), "button_press_event",
			   GTK_SIGNAL_FUNC(gkb_cb), gkb);
	gtk_signal_connect_after(GTK_OBJECT(gkb->darea), "realize",
			   GTK_SIGNAL_FUNC(gkb_draw), gkb);
	gtk_signal_connect(GTK_OBJECT(gkb->darea), "expose_event",
			   GTK_SIGNAL_FUNC(gkb_expose), gkb);
        gtk_widget_show(gkb->darea);

        gkb->properties.curpix = 0;

        gkb->frame = gtk_frame_new(NULL);
        gtk_frame_set_shadow_type(GTK_FRAME(gkb->frame),GTK_SHADOW_IN);
        gtk_container_add(GTK_CONTAINER(gkb->frame),gkb->darea);

	gtk_widget_pop_colormap ();
	gtk_widget_pop_visual ();
}

void
about_cb (AppletWidget *widget,
	GKB * gkb)
{
        static const char *authors[] = 
			{ "Szabolcs Ban (Shooby) <bansz@szif.hu>", NULL };

        if(gkb->aboutbox) {
		gtk_widget_show(gkb->aboutbox);
		gdk_window_raise(gkb->aboutbox->window);
		return;
	}

	gkb->aboutbox = gnome_about_new (_("The GNOME KB Applet"),
			_("0.99.1"),
                        _("(C) 1998 LSC - Linux Supporting Center"),
                        (const char **)authors,
                        _("This applet used to switch between "   
                          "keyboard maps. No more. It uses "
                          "setxkbmap, or xmodmap. "
                          "The main site of this app moved "
                          "tempolary to URL http://lsc.kva.hu/gkb."
                          "Mail me your flag, please (60x40 size),"
                          "I will put it to CVS."
                          "So long, and thanks for all the fish."
                          ),
                        _("gkb.xpm"));
        gtk_widget_show (gkb->aboutbox);

        return;
}

static int
applet_save_session(GtkWidget *w,
        const char *privcfgpath,
	const char *globcfgpath,
	GKB *gkb)
{
	gnome_config_push_prefix(privcfgpath);
	gnome_config_set_string("gkb/command",gkb->properties.command);
	gnome_config_set_string("gkb/image0",gkb->properties.image[0]);
	gnome_config_set_string("gkb/image1",gkb->properties.image[1]);
	gnome_config_set_string("gkb/map0",gkb->properties.dmap[0]);
	gnome_config_set_string("gkb/map1",gkb->properties.dmap[1]);
	gnome_config_pop_prefix();

	gnome_config_sync();
	gnome_config_drop_all();

	return FALSE;
}

static CORBA_Object
gkb_activator(PortableServer_POA poa,
	const char *goad_id,
	const char **params,
	gpointer *impl_ptr,
	CORBA_Environment *ev)
{
        GKB *gkb;
        
        gkb = g_new0(GKB,1);
      
        gkb->applet=applet_widget_new(goad_id);
        
        load_properties(gkb);
      
        /*gtk_widget_realize(applet);*/
        create_gkb_widget(gkb);
        gtk_widget_show(gkb->frame);
        applet_widget_add(APPLET_WIDGET(gkb->applet), gkb->frame);
        gtk_widget_show(gkb->applet);
      
        gtk_signal_connect(GTK_OBJECT(gkb->applet),"save_session",
                               	     GTK_SIGNAL_FUNC(applet_save_session),
                             	     gkb);
      
        applet_widget_register_stock_callback(APPLET_WIDGET(gkb->applet),
                      				"about",
                    				GNOME_STOCK_MENU_ABOUT,
                     				_("About..."),
                       				about_cb,
                     				gkb);
      
      
        applet_widget_register_stock_callback(APPLET_WIDGET(gkb->applet),
                      				"properties",
                     				GNOME_STOCK_MENU_PROP,
                      				_("Properties..."),
                   				properties_dialog,
                       				gkb);
      
        return applet_widget_corba_activate(gkb->applet,
						poa, goad_id, params,
				      		impl_ptr, ev);
}

static void
gkb_deactivator(PortableServer_POA poa,
	const char *goad_id,
	gpointer impl_ptr,
	CORBA_Environment *ev)
{
        applet_widget_corba_deactivate(poa, goad_id, impl_ptr, ev);
}

#if 1 || defined(SHLIB_APPLETS)
static const char *repo_id[]={"IDL:GNOME/Applet:1.0", NULL};
static GnomePluginObject applets_list[] = {
  {
       repo_id, "gkb_applet", NULL, "The GNOME keyboard switcher",
       &gkb_activator, &gkb_deactivator},
  {NULL}
};

GnomePlugin GNOME_Plugin_info = {
       applets_list, NULL
};
#else
int
main(int argc, char *argv[])
{
       gpointer gkb_impl;

       /* Initialize the i18n stuff */
       bindtextdomain (PACKAGE, GNOMELOCALEDIR);
       textdomain (PACKAGE);

       applet_widget_init("gkb_applet", VERSION, argc, argv, NULL, 0, NULL);

       APPLET_ACTIVATE(gkb_activator, "gkb_applet", &gkb_impl);
	
       applet_widget_gtk_main();

       APPLET_DEACTIVATE(gkb_deactivator, "gkb_applet", gkb_impl);

       return 0;
}
#endif
