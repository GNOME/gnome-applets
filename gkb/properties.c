/* properties dialog from GKB applet
 */

#include <stdio.h>
#include <string.h>
#include <config.h>
#include <gnome.h>

#include "properties.h"

GtkWidget *propbox=NULL;                                                        

extern gint curpix;

extern GtkWidget *pixmap;


static gkb_properties temp_props;                                               
                                                                                
static GtkWidget *fileSel = NULL;                                               
static gchar *wpFileName=NULL;                                                  
                                                                                
static gint cur;                                                                
                                                                                
extern GtkWidget *disp;                                                         
extern gkb_properties pr;                                                       

static GtkWidget * pix1, * pix2;

void
load_properties( char *path, gkb_properties *prop )
{

	gnome_config_push_prefix (path);
	prop->dmap[0]	= gnome_config_get_string ("gkb/dmap0=us");
	prop->dmap[1]	= gnome_config_get_string ("gkb/dmap1=hu");
	prop->dfile[0]	= gnome_config_get_string ("gkb/dfile0=/usr/share/pixmaps/gkb/us.xpm");
	prop->dfile[1]	= gnome_config_get_string ("gkb/dfile1=/usr/share/pixmaps/gkb/hu.xpm");
	gnome_config_pop_prefix ();
}


void
save_properties( char *path, gkb_properties *prop )
{
	gnome_config_push_prefix (path);
	gnome_config_set_string( "gkb/dmap0",  prop->dmap[0] );
	gnome_config_set_string( "gkb/dmap1",  prop->dmap[1] );
	gnome_config_set_string( "gkb/dfile0", prop->dfile[0] );
	gnome_config_set_string( "gkb/dfile1", prop->dfile[1] );
        gnome_config_pop_prefix ();
        gnome_config_sync();
	gnome_config_drop_all();
}

gint
delete_browse (GtkWidget *w, GdkEvent *e, GtkWidget **f)
{	
	if (temp_props.dfile[cur])
		g_free (temp_props.dfile[cur]);
        temp_props.dfile[cur] = g_strdup (gtk_file_selection_get_filename
				  (GTK_FILE_SELECTION (*f)));
	gnome_pixmap_load_file (GNOME_PIXMAP (cur==0?pix1:pix2), temp_props.dfile[cur]);
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

static void
icon_selection_cancel (GtkWidget *w, GtkWidget **f)
{
	GtkWidget *cf = *f;
	delete_browse (w, NULL, f);
	gtk_widget_destroy (cf);
}

static void
browse_icons (GtkWidget *w, gpointer p)
{
	if (!fileSel) {
		
		fileSel = gtk_file_selection_new (_("Icon Selection"));
		if (temp_props.dfile[cur])
			gtk_file_selection_set_filename (GTK_FILE_SELECTION (fileSel),
							 temp_props.dfile[cur]);

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
 cur=0;
 browse_icons (w,p);
}

static void
browse_icons2 (GtkWidget *w, gpointer p)
{
 cur=1;
 browse_icons (w,p);
}

static void
apply_cb(GnomePropertyBox * pb, gint page, gpointer data)
{
 gnome_property_box_changed(GNOME_PROPERTY_BOX(propbox));
}

static void
apply_callback(GtkWidget * pb, gint page, gpointer data)
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

  
  memcpy( &pr, &temp_props, sizeof(gkb_properties) );
  gnome_pixmap_load_file (GNOME_PIXMAP (pixmap), pr.dfile[curpix]);

}

gint
destroy_cb( GtkWidget *widget, void *data )
{

	propbox = NULL;
	return FALSE;
}

void
properties(AppletWidget *applet, gpointer data)
{
	GtkWidget * frame;
        GtkWidget * box;
        GtkWidget * but1, * but2;
        GtkWidget * ent1, * ent2;
        GtkWidget * vbx1, * vbx2;

    	box= gtk_hbox_new  (TRUE,0);
	vbx1= gtk_vbox_new (FALSE,0);
    	vbx2= gtk_vbox_new (FALSE,0);

	if( propbox ) {
		gdk_window_raise(propbox->window);
		return;
	}

	memcpy(&temp_props,&pr,sizeof(gkb_properties));

        propbox = gnome_property_box_new();
	gtk_window_set_title(GTK_WINDOW(propbox), _("GKB settings"));
	gtk_window_set_policy(GTK_WINDOW(propbox), FALSE, FALSE, TRUE);

	gtk_container_border_width( GTK_CONTAINER(box), 1 );

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

         if (g_file_exists(pr.dfile[0])) 
         {
          pix1 = gnome_pixmap_new_from_file(pr.dfile[0]);
         }
         if (pix1 == NULL) {
          pix1 = gtk_label_new(_("Couldn't\nload\nicon"));
         }

        if ( pix1 ) {
        gtk_container_add(GTK_CONTAINER(but1), pix1 );
	}

	but2 = gtk_button_new();
	gtk_signal_connect (GTK_OBJECT (but2), "clicked",
			    (GtkSignalFunc) browse_icons2, propbox);
	gtk_box_pack_end( GTK_BOX(vbx2), but2, FALSE,FALSE,1);

         if (g_file_exists(pr.dfile[1])) 
         {
          pix2 = gnome_pixmap_new_from_file(pr.dfile[1]);
         }
         if (pix2 == NULL) {
          pix2 = gtk_label_new(_("Couldn't\nload\nicon"));
         }
        if ( pix2 ) {
         gtk_container_add(GTK_CONTAINER(but2), pix2 );
        }
	
	gtk_box_pack_start( GTK_BOX(box), vbx1,FALSE,FALSE,1 );
	gtk_box_pack_start( GTK_BOX(box), vbx2,FALSE,FALSE,1 );

	gtk_notebook_append_page (GTK_NOTEBOOK(GNOME_PROPERTY_BOX (propbox)->notebook),
				  box, gtk_label_new (_("Menu")));

        gtk_signal_connect( GTK_OBJECT(propbox),
		"apply", GTK_SIGNAL_FUNC(apply_callback), NULL );

        gtk_signal_connect( GTK_OBJECT(propbox),
		"destroy", GTK_SIGNAL_FUNC(destroy_cb), NULL );

	gtk_widget_show_all(propbox);
}
