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
	int curpix;
};

        GdkImlibImage *pix[2]={ NULL, NULL };
static  gchar *wpFileName=NULL;   
static  GtkWidget *propbox = NULL;                                               
static  GtkWidget *fileSel = NULL;                                               
	int curpix;

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

	if(pix[0])
		gdk_imlib_destroy_image(pix[0]);
	if(pix[1])
		gdk_imlib_destroy_image(pix[1]);
	
	pix[0] = gdk_imlib_load_image(gkb->properties.image[0]);
	pix[1] = gdk_imlib_load_image(gkb->properties.image[1]);
	gdk_imlib_render (pix[0], pix[0]->rgb_width, pix[0]->rgb_height);
	gdk_imlib_render (pix[1], pix[1]->rgb_width, pix[1]->rgb_height);
}

/* Old, but maybe works */

gint
delete_browse (GtkWidget *w, GdkEvent *e, GtkWidget **f)
{	
	if (temp_props.image[curpix])
		g_free (temp_props.image[curpix]);
        temp_props.image[curpix] = g_strdup (gtk_file_selection_get_filename
				  (GTK_FILE_SELECTION (*f)));
/*	gnome_pixmap_load_file (GNOME_PIXMAP (cur==0?pix1:pix2), temp_props.image[cur]);
*/
	gnome_property_box_changed(GNOME_PROPERTY_BOX(propbox));
  	*f = NULL;
	return TRUE;
}

static void
icon_selection_ok (GtkWidget *w, GtkWidget **f)
{
	GtkWidget *cf = *f;
	
	if (w)
	 delete_browse (w, NULL, f);
	gtk_widget_destroy (cf);      
}

static gint
icon_selection_cancel (GtkWidget *w, GtkWidget **f)
{
	GtkWidget *cf = *f;
	delete_browse (w, NULL, f);
	gtk_widget_destroy (cf);
        return FALSE;
}

static void
browse_icons (GtkWidget *w, gpointer p)
{
	if (!fileSel) {
		
		fileSel = gtk_file_selection_new (
		_("Icon Selection")
		);
		if (temp_props.image[curpix])
			gtk_file_selection_set_filename (GTK_FILE_SELECTION (fileSel),
							 temp_props.image[curpix]);

		gtk_signal_connect (GTK_OBJECT (fileSel), "delete_event",
				    (GtkSignalFunc) delete_browse,
				    &fileSel);

		gtk_signal_connect (GTK_OBJECT
				    (GTK_FILE_SELECTION (fileSel)->ok_button),
				    "clicked", (GtkSignalFunc) icon_selection_ok,
				    &fileSel);

		gtk_signal_connect (GTK_OBJECT
				    (GTK_FILE_SELECTION (fileSel)->cancel_button),
				    "clicked",
				    (GtkSignalFunc) icon_selection_cancel,
				    &fileSel);

		gtk_widget_show (fileSel);
	}
}

static void
browse_icons1 (GtkWidget *w, gpointer p)
{
 curpix=0;
 browse_icons (w,p);
}

static void
browse_icons2 (GtkWidget *w, gpointer p)
{
 curpix=1;
 browse_icons (w,p);
}

static void
apply_cb(GnomePropertyBox * pb, gint page, gpointer data)
{
 gnome_property_box_changed(GNOME_PROPERTY_BOX(propbox));
}

static void
apply_callback(GtkWidget * pb, gint page, gpointer data, GKB *gkb)
{

  GtkWidget * entry1 = gtk_object_get_data(GTK_OBJECT(pb),"e1");
  GtkWidget * entry2 = gtk_object_get_data(GTK_OBJECT(pb),"e2");
  gchar * ent1, * ent2;

  ent1 = gtk_entry_get_text(GTK_ENTRY(entry1));
  ent2 = gtk_entry_get_text(GTK_ENTRY(entry2));

  g_free(temp_props.dmap[0]);
  g_free(temp_props.dmap[1]);
  temp_props.dmap[0] = g_strdup(ent1);
  temp_props.dmap[1] = g_strdup(ent2);

  memcpy( &gkb->properties, &temp_props, sizeof(gkb_properties) );
  /* cnange the pixmap? */
  
}

static void ch_xkb_cb(GtkWidget *widget, gpointer data)
{
       temp_props.command = g_strdup("setxkbmap");
       gnome_property_box_changed(GNOME_PROPERTY_BOX(propbox)); 
}

static void ch_xmodmap_cb(GtkWidget *widget, gpointer data)
{
        temp_props.command = g_strdup("xmodmap");
        gnome_property_box_changed(GNOME_PROPERTY_BOX(propbox));
}

gint
destroy_cb( GtkWidget *widget, void *data )
{

	propbox = NULL;
	return FALSE;
}

void
properties_dialog(AppletWidget *applet, gpointer data)
{
        GtkWidget * nbox;
        GtkWidget * box;
        GtkWidget * but1, * but2;
        GtkWidget * ent1, * ent2;
        GtkWidget * vbx1, * vbx2;
        GtkWidget *omenu;
        GtkWidget *menu;
        GtkWidget *item1, *item2;
	GtkWidget * pix1, * pix2;

        GKB * gkb = data;
	
        nbox= gtk_vbox_new (TRUE,0);
    	box= gtk_hbox_new  (TRUE,0);
	vbx1= gtk_vbox_new (FALSE,0);
    	vbx2= gtk_vbox_new (FALSE,0);

	if( propbox ) {
		gdk_window_raise(propbox->window);
		return;
	}

	memcpy(&temp_props,&gkb->properties,sizeof(gkb_properties));

        propbox = gnome_property_box_new();
	gtk_window_set_title(GTK_WINDOW(propbox), 
	_("GKB settings")
	);
	gtk_window_set_policy(GTK_WINDOW(propbox), FALSE, FALSE, TRUE);

	gtk_container_set_border_width( GTK_CONTAINER(nbox), 1 );

        ent1 = gtk_entry_new_with_max_length (5);
        gtk_object_set_data(GTK_OBJECT(propbox),"e1",ent1);
        gtk_entry_set_text (GTK_ENTRY(ent1),temp_props.dmap[0]);
	gtk_signal_connect (GTK_OBJECT (ent1), "changed",
			    (GtkSignalFunc) apply_cb, propbox);
	gtk_box_pack_start( GTK_BOX(vbx1), ent1, FALSE,FALSE,1);

        ent2 = gtk_entry_new_with_max_length (5);
        gtk_object_set_data(GTK_OBJECT(propbox),"e2",ent2);
        gtk_entry_set_text (GTK_ENTRY(ent2),temp_props.dmap[1]);
	gtk_signal_connect (GTK_OBJECT (ent2), "changed",
			    (GtkSignalFunc) apply_cb, propbox);
	gtk_box_pack_start( GTK_BOX(vbx2), ent2 ,FALSE,FALSE,1);

	but1 = gtk_button_new();
	gtk_signal_connect (GTK_OBJECT (but1), "clicked",
			    (GtkSignalFunc) browse_icons1, NULL);
	gtk_box_pack_end( GTK_BOX(vbx1), but1 ,FALSE,FALSE,1);

         if (g_file_exists(temp_props.image[0])) 
         {
          pix1 = gnome_pixmap_new_from_file(temp_props.image[0]);
         }
         if (pix1 == NULL) {
          pix1 = gtk_label_new(
	  _("Couldn't\nload\nicon")
	  );
         }

        if ( pix1 ) {
        gtk_container_add(GTK_CONTAINER(but1), pix1 );
	}

	but2 = gtk_button_new();
	gtk_signal_connect (GTK_OBJECT (but2), "clicked",
			    (GtkSignalFunc) browse_icons2, propbox);
	gtk_box_pack_end( GTK_BOX(vbx2), but2, FALSE,FALSE,1);

         if (g_file_exists(temp_props.image[1])) 
         {
          pix2 = gnome_pixmap_new_from_file(temp_props.image[1]);
         }
         if (pix2 == NULL) {
          pix2 = gtk_label_new(
	   _("Couldn't\nload\nicon")
	   );
         }
        if ( pix2 ) {
         gtk_container_add(GTK_CONTAINER(but2), pix2 );
        }
	
	gtk_box_pack_start( GTK_BOX(box), vbx1,FALSE,FALSE,1 );
	gtk_box_pack_start( GTK_BOX(box), vbx2,FALSE,FALSE,1 );

        omenu = gtk_option_menu_new ();
        menu = gtk_menu_new ();

        item1 = gtk_menu_item_new_with_label(
	 _("Xkb")
	 );
        gtk_signal_connect (GTK_OBJECT (item1), "activate", (GtkSignalFunc) ch_xkb_cb, NULL);
        gtk_menu_append (GTK_MENU (menu), item1);

        item2 = gtk_menu_item_new_with_label(
	 _("Xmodmap")
	);
        gtk_signal_connect (GTK_OBJECT (item2), "activate", (GtkSignalFunc) ch_xmodmap_cb, NULL);
        gtk_menu_append (GTK_MENU (menu), item2);

        gtk_option_menu_set_menu (GTK_OPTION_MENU (omenu), (strcmp("xmodmap",temp_props.command)?item1:item2));
        gtk_option_menu_set_menu (GTK_OPTION_MENU (omenu), menu);

        gtk_box_pack_start( GTK_BOX(nbox), box, TRUE, TRUE, 5);
	gtk_box_pack_start( GTK_BOX(nbox), omenu,FALSE,FALSE,1 );

        gtk_widget_show (omenu);

	gtk_notebook_append_page (GTK_NOTEBOOK(GNOME_PROPERTY_BOX (propbox)->notebook),
				  nbox, gtk_label_new (
				  _("Menu")
				  ));

        gtk_signal_connect( GTK_OBJECT(propbox),
		"apply", GTK_SIGNAL_FUNC(apply_callback), gkb );

        gtk_signal_connect( GTK_OBJECT(propbox),
		"destroy", GTK_SIGNAL_FUNC(destroy_cb), gkb );

	gtk_widget_show_all(propbox);
}

/* --- */

static void
gkb_draw(GtkWidget *darea,GKB *gkb)
{
	if(!GTK_WIDGET_REALIZED(gkb->darea))
		return;
	gdk_draw_pixmap(gkb->darea->window,
			gkb->darea->style->fg_gc[GTK_WIDGET_STATE(gkb->darea)],
			pix[gkb->properties.curpix]->pixmap,
			0, 0,
			0, 0,
			-1, -1);
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

static int 
gkb_cb(GtkWidget * widget, GdkEventButton * e, 
		gpointer data)
{
 
       GKB * gkb = data;
       
       if (e->button != 1) {
		/* Ignore buttons 2 and 3 */
		return FALSE; 
	}
	gkb->properties.curpix++;
	if(gkb->properties.curpix>=2) gkb->properties.curpix=0;
	gkb_draw(GTK_WIDGET(gkb->darea),gkb);

	do_that_command(gkb);

	return TRUE;
}

static int
gkb_expose(GtkWidget *darea, GdkEventExpose *event,GKB *gkb)
{
	gdk_draw_pixmap(gkb->darea->window,
			gkb->darea->style->fg_gc[GTK_WIDGET_STATE(gkb->darea)],
			pix[gkb->properties.curpix]->pixmap,
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
			      pix[gkb->properties.curpix]->rgb_width,
			      pix[gkb->properties.curpix]->rgb_height);
	gtk_widget_set_events(gkb->darea, gtk_widget_get_events(gkb->darea) |
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
about_cb (AppletWidget *widget, gpointer data)
{
	GKB *gkb = data;
	
        static const char *authors[] = { "Szabolcs Ban (Shooby) <bansz@szif.hu>", NULL };

        if(gkb->aboutbox) {
		gtk_widget_show(gkb->aboutbox);
		gdk_window_raise(gkb->aboutbox->window);
		return;
	}

	gkb->aboutbox = gnome_about_new (_("The GNOME KB Applet"), _("0.99"),
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

  return applet_widget_corba_activate(gkb->applet, poa, goad_id, params,
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
  {repo_id, "gkb_applet", NULL, "The GNOME keyboard switcher",
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
