/* 
 * GNOME diskusage panel applet - properties dialog
 *
 * (C) 1997 The Free Software Foundation
 *
 */

#include <config.h>
#include <gnome.h>

#include "properties.h"
#include "diskusage.h"

#define max_rgb_str_len  7
#define max_rgb_str_size 8

static GtkWidget *gfp = NULL;

GtkWidget *propbox = NULL;
static diskusage_properties temp_props;

/* static const unsigned int max_rgb_str_len = 7; */
/* static const unsigned int max_rgb_str_size = 8; */

extern DiskusageInfo   summary_info;
	
typedef struct	_RadioButtonCbData		RadioButtonCbData;
struct _RadioButtonCbData
{
	GtkWidget *button;
	GtkWidget *size;
	diskusage_properties *props;
};

gboolean font_changed = FALSE;

static void size_cb (GtkWidget *widget, GtkWidget *spin);
static void freq_cb (GtkWidget *widget, GtkWidget *spin);
void best_size_cb (GtkWidget *widget, RadioButtonCbData *cb_data);

static GtkWidget *create_frame (diskusage_properties *props);
static void apply_cb   (GtkWidget *widget, int page_num, AppletWidget *applet);
static gint destroy_cb (GtkWidget *widget, void *data);


void
load_properties (const char *path, diskusage_properties *prop)
{
	gchar *p;

	gnome_config_push_prefix (path);
	prop->startfs   = gnome_config_get_int    ("disk/startfs=0");
	prop->ucolor	= gnome_config_get_string ("disk/ucolor=#cf5f5f");
        if (prop->ucolor && (strlen(prop->ucolor) < max_rgb_str_len))
		prop->ucolor = g_realloc(prop->ucolor, max_rgb_str_size);

	prop->fcolor	= gnome_config_get_string ("disk/fcolor=#008f00");
        if (prop->fcolor && (strlen(prop->fcolor) < max_rgb_str_len))
		prop->fcolor = g_realloc(prop->fcolor, max_rgb_str_size);

	prop->tcolor	= gnome_config_get_string ("disk/tcolor=#bbbbbb");
        if (prop->tcolor && (strlen(prop->tcolor) < max_rgb_str_len))
		prop->tcolor = g_realloc(prop->tcolor, max_rgb_str_size);

	prop->bcolor	= gnome_config_get_string ("disk/bcolor=#000000");
        if (prop->bcolor && (strlen(prop->bcolor) < max_rgb_str_len))
		prop->bcolor = g_realloc(prop->bcolor, max_rgb_str_size);

	prop->speed	= gnome_config_get_int    ("disk/speed=5000");
	prop->size 	= gnome_config_get_int	  ("disk/size=120");
	prop->look	= gnome_config_get_bool   ("disk/look=false");
	prop->best_size	= gnome_config_get_bool   ("disk/best_size=true");
	p = gnome_config_get_string_with_default ("disk/font", NULL);
	if (p)
		prop->font = p;
	else
		prop->font = NULL;
	gnome_config_pop_prefix ();
}

void
save_properties (const char *path, diskusage_properties *prop)
{
	gnome_config_push_prefix (path);
	gnome_config_set_int    ( "disk/startfs", prop->startfs );
	gnome_config_set_string ( "disk/ucolor", prop->ucolor );
	gnome_config_set_string ( "disk/fcolor", prop->fcolor );
	gnome_config_set_string ( "disk/tcolor", prop->tcolor );
	gnome_config_set_string ( "disk/bcolor", prop->bcolor );

	gnome_config_set_string ( "disk/font", prop->font );

	gnome_config_set_int    ( "disk/speed", prop->speed );
	gnome_config_set_int    ( "disk/size", prop->size );
	gnome_config_set_bool   ( "disk/look", prop->look );
	gnome_config_set_bool   ( "disk/best_size", prop->best_size );
	gnome_config_pop_prefix ();
        gnome_config_sync();
	gnome_config_drop_all();
}

static void
ucolor_set_cb (GnomeColorPicker *cp)
{
        guint8 r, g, b;
        gnome_color_picker_get_i8 (cp, &r, &g, &b, NULL);
        g_snprintf (temp_props.ucolor, max_rgb_str_size, 
                    "#%02x%02x%02x", r, g, b);

        gnome_property_box_changed (GNOME_PROPERTY_BOX (propbox));
}

static void
fcolor_set_cb (GnomeColorPicker *cp)
{
        guint8 r, g, b;
        gnome_color_picker_get_i8 (cp, &r, &g, &b, NULL);
        g_snprintf (temp_props.fcolor, max_rgb_str_size, 
		    "#%02x%02x%02x", r, g, b);

        gnome_property_box_changed (GNOME_PROPERTY_BOX (propbox));
}

static void
tcolor_set_cb (GnomeColorPicker *cp)
{
        guint8 r, g, b;
        gnome_color_picker_get_i8 (cp, &r, &g, &b, NULL);
        g_snprintf( temp_props.tcolor, max_rgb_str_size,
                    "#%02x%02x%02x", r, g, b );

        gnome_property_box_changed (GNOME_PROPERTY_BOX (propbox));
}

static void
bcolor_set_cb (GnomeColorPicker *cp)
{
        guint8 r, g, b;
        gnome_color_picker_get_i8 (cp, &r, &g, &b, NULL);
        g_snprintf (temp_props.bcolor, max_rgb_str_size,
                    "#%02x%02x%02x", r, g, b);

        gnome_property_box_changed (GNOME_PROPERTY_BOX (propbox));
}

static void
font_set_cb (void)
{
	font_changed = TRUE;

	gnome_property_box_changed (GNOME_PROPERTY_BOX (propbox));
}

static void
size_cb (GtkWidget *widget, GtkWidget *spin)
{
	temp_props.size = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin));

        gnome_property_box_changed (GNOME_PROPERTY_BOX (propbox));
}

static void
freq_cb (GtkWidget *widget, GtkWidget *spin)
{
	temp_props.speed = gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(spin))*1000;

        gnome_property_box_changed (GNOME_PROPERTY_BOX (propbox));

        return;
}	

void
best_size_cb (GtkWidget *widget, RadioButtonCbData *cb_data)
{
	cb_data->props->best_size = GTK_TOGGLE_BUTTON (cb_data->button)->active;

	if (cb_data->props->best_size)
		gtk_widget_set_sensitive (cb_data->size, FALSE);
	else
		gtk_widget_set_sensitive (cb_data->size, TRUE);

        gnome_property_box_changed (GNOME_PROPERTY_BOX(propbox));
}

static GtkWidget *
create_frame (diskusage_properties *props)
{
	GtkWidget *label;
	GtkWidget *best_size_button, *best_size;
	GtkWidget *box, *color, *size, *speed;
	GtkWidget *color1, *color2;
	GtkWidget *applet_size, *freq;
	GtkObject *applet_size_a, *freq_a;
	GtkWidget *ucolor_gcp, *fcolor_gcp, *tcolor_gcp, *bcolor_gcp;
        int ur,ug,ub, fr,fg,fb, tr,tg,tb, br, bg, bb;
	RadioButtonCbData *cb_data;

	GtkWidget *frame;
	GtkWidget *size_box;
	GtkWidget *font_vbox;
	GtkWidget *font_hbox;
	GtkWidget *table;

	sscanf ( temp_props.ucolor, "#%02x%02x%02x", &ur, &ug, &ub );
        sscanf ( temp_props.fcolor, "#%02x%02x%02x", &fr, &fg, &fb );
        sscanf ( temp_props.tcolor, "#%02x%02x%02x", &tr, &tg, &tb );
        sscanf ( temp_props.bcolor, "#%02x%02x%02x", &br, &bg, &bb );
        
	box   = gtk_vbox_new (FALSE, 0);
	color = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
	size  = gtk_hbox_new (FALSE, 0);
	speed = gtk_hbox_new (FALSE, 0);
	best_size = gtk_hbox_new (FALSE, 0);
/*	gtk_container_set_border_width (GTK_CONTAINER(box), GNOME_PAD_SMALL); */

	table = gtk_table_new (2, 4, TRUE);

	color1 = gtk_hbox_new (FALSE, GNOME_PAD_SMALL);
	color2 = gtk_hbox_new (FALSE, GNOME_PAD_SMALL);

	ucolor_gcp = gnome_color_picker_new();
	fcolor_gcp = gnome_color_picker_new();
	tcolor_gcp = gnome_color_picker_new();
	bcolor_gcp = gnome_color_picker_new();
	gnome_color_picker_set_i8 (GNOME_COLOR_PICKER (ucolor_gcp), 
				   ur, ug, ub, 255);
	gnome_color_picker_set_i8 (GNOME_COLOR_PICKER (fcolor_gcp), 
				   fr, fg, fb, 255);
	gnome_color_picker_set_i8 (GNOME_COLOR_PICKER (tcolor_gcp), 
				   tr, tg, tb, 255);
	gnome_color_picker_set_i8 (GNOME_COLOR_PICKER (bcolor_gcp), 
				   br, bg, bb, 255);
	gtk_signal_connect (GTK_OBJECT (ucolor_gcp), "color_set",
			    GTK_SIGNAL_FUNC (ucolor_set_cb), NULL);
	gtk_signal_connect (GTK_OBJECT (fcolor_gcp), "color_set",
			    GTK_SIGNAL_FUNC (fcolor_set_cb), NULL);
	gtk_signal_connect (GTK_OBJECT (tcolor_gcp), "color_set",
			    GTK_SIGNAL_FUNC (tcolor_set_cb), NULL);
	gtk_signal_connect (GTK_OBJECT (bcolor_gcp), "color_set",
			    GTK_SIGNAL_FUNC (bcolor_set_cb), NULL);

	label = gtk_label_new (_("Used Diskspace:"));
	gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
/*	gtk_box_pack_start (GTK_BOX (color1), label, TRUE, TRUE, 0); */
/*	gtk_box_pack_start (GTK_BOX (color1), ucolor_gcp, FALSE, FALSE, 0); */

	gtk_table_attach (GTK_TABLE (table), label,
			  0, 1, 0, 1,
			  GTK_FILL | GTK_EXPAND, GTK_FILL,
			  4, 0);

	gtk_table_attach (GTK_TABLE (table), ucolor_gcp,
			  1, 2, 0, 1,
			  GTK_FILL, GTK_FILL,
			  4, 0);

	label = gtk_label_new (_("Text:"));
	gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
/*	gtk_box_pack_start (GTK_BOX (color1), label, TRUE, TRUE, 0); */
/*	gtk_box_pack_start (GTK_BOX (color1), tcolor_gcp, FALSE, FALSE, 0); */

	gtk_table_attach (GTK_TABLE (table), label,
			  2, 3, 0, 1,
			  GTK_FILL | GTK_EXPAND, GTK_FILL,
			  4, 0);

	gtk_table_attach (GTK_TABLE (table), tcolor_gcp,
			  3, 4, 0, 1,
			  GTK_FILL, GTK_FILL,
			  4, 0);

	label = gtk_label_new (_("Free Diskspace:"));
	gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
/*	gtk_box_pack_start (GTK_BOX (color2), label, TRUE, TRUE, 0); */
/*	gtk_box_pack_start (GTK_BOX (color2), fcolor_gcp, FALSE, FALSE, 0); */

	gtk_table_attach (GTK_TABLE (table), label,
			  0, 1, 1, 2,
			  GTK_FILL | GTK_EXPAND, GTK_FILL,
			  4, 0);

	gtk_table_attach (GTK_TABLE (table), fcolor_gcp,
			  1, 2, 1, 2,
			  GTK_FILL, GTK_FILL,
			  4, 0);


	label = gtk_label_new (_("Background:"));
	gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
/*	gtk_box_pack_start (GTK_BOX (color2), label, TRUE, TRUE, 0); */
/*	gtk_box_pack_start (GTK_BOX (color2), bcolor_gcp, FALSE, FALSE, 0); */

	gtk_table_attach (GTK_TABLE (table), label,
			  2, 3, 1, 2,
			  GTK_FILL | GTK_EXPAND, GTK_FILL,
			  4, 0);

	gtk_table_attach (GTK_TABLE (table), bcolor_gcp,
			  3, 4, 1, 2,
			  GTK_FILL, GTK_FILL,
			  4, 0);


	label = gtk_label_new(_("Applet Width:"));
	applet_size_a = gtk_adjustment_new (temp_props.size, 0.5, 256, 1, 8, 8);
	applet_size  = gtk_spin_button_new (GTK_ADJUSTMENT(applet_size_a), 1, 0);
	gtk_box_pack_start (GTK_BOX (size), label, FALSE, FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start (GTK_BOX (size), applet_size, FALSE, FALSE, GNOME_PAD_SMALL);

	best_size_button = gtk_check_button_new_with_label(_("Automatically pick best applet size"));

	cb_data = g_new0 (RadioButtonCbData, 1);
	cb_data->button = best_size_button;
	cb_data->size = size;
	cb_data->props = props;

	if (props->best_size) {
		gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (best_size_button), TRUE);
		gtk_widget_set_sensitive (size, FALSE);
	}

	gtk_signal_connect (GTK_OBJECT (best_size_button), "toggled", best_size_cb, cb_data);

	gtk_box_pack_start_defaults (GTK_BOX(best_size), best_size_button);

        gtk_signal_connect (GTK_OBJECT(applet_size_a), "value_changed",
			    GTK_SIGNAL_FUNC(size_cb), applet_size);
        gtk_signal_connect (GTK_OBJECT(applet_size_a), "changed",
			    GTK_SIGNAL_FUNC(size_cb), applet_size);
        gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON(applet_size),
					   GTK_UPDATE_ALWAYS);

	label = gtk_label_new (_("Update Frequency (seconds):"));
	freq_a = gtk_adjustment_new ((float)temp_props.speed/1000, 0.1, 60, 0.1, 5, 5);
	freq  = gtk_spin_button_new (GTK_ADJUSTMENT(freq_a), 0.1, 1 );
	gtk_box_pack_start (GTK_BOX(speed), label,FALSE, FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start (GTK_BOX(speed), freq, FALSE, FALSE, GNOME_PAD_SMALL);
	
        gtk_signal_connect (GTK_OBJECT (freq_a), "value_changed",
			    GTK_SIGNAL_FUNC(freq_cb), freq);
        gtk_signal_connect (GTK_OBJECT (freq), "changed",
			    GTK_SIGNAL_FUNC(freq_cb), freq);
        gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (freq),
					   GTK_UPDATE_ALWAYS);

	gtk_box_pack_start (GTK_BOX (color), table, FALSE, FALSE, GNOME_PAD_SMALL);
/*        gtk_box_pack_start (GTK_BOX (color), color2, FALSE, FALSE, 0); */
        
/*        gtk_box_pack_start (GTK_BOX (box), color, FALSE, FALSE, 0); */

	frame = gtk_frame_new (_("Colors"));
	gtk_container_border_width (GTK_CONTAINER (frame), GNOME_PAD_SMALL);
	gtk_container_add (GTK_CONTAINER (frame), color);
	gtk_box_pack_start (GTK_BOX (box), frame, FALSE, FALSE, GNOME_PAD_SMALL);

/*	gtk_box_pack_start (GTK_BOX (box), best_size, FALSE, FALSE, 0); */

	size_box = gtk_vbox_new (FALSE, 0);
/*	gtk_widget_set (best_size, */
/*			"border_width", 2, */
/*			"spacing", 2, */
/*			NULL); */

	frame = gtk_frame_new (_("Size"));
	gtk_container_border_width (GTK_CONTAINER (frame), GNOME_PAD_SMALL);
	gtk_container_add (GTK_CONTAINER (frame), size_box);
	gtk_box_pack_start (GTK_BOX (box), frame, FALSE, FALSE, 0);

	gtk_box_pack_start (GTK_BOX (size_box), size, FALSE, FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start (GTK_BOX (size_box), best_size, FALSE, FALSE, 4);


	frame = gtk_frame_new (_("Fonts"));
	gtk_container_border_width (GTK_CONTAINER (frame), GNOME_PAD_SMALL);
	gtk_box_pack_start (GTK_BOX (box), frame, FALSE, FALSE, 0);

	label = gtk_label_new (_("Font:"));
	gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);

	font_hbox = gtk_hbox_new (FALSE, 0);
	font_vbox = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (font_hbox), label, FALSE, FALSE, GNOME_PAD_SMALL);

	gfp = gnome_font_picker_new ();

	gnome_font_picker_set_mode (GNOME_FONT_PICKER (gfp),
				    GNOME_FONT_PICKER_MODE_FONT_INFO);
	gnome_font_picker_fi_set_use_font_in_label (GNOME_FONT_PICKER (gfp),
						    TRUE, 12);
	gtk_signal_connect (GTK_OBJECT (gfp), "font_set",
			    GTK_SIGNAL_FUNC (font_set_cb), NULL);
	gtk_box_pack_start (GTK_BOX (font_hbox), gfp, TRUE, TRUE, GNOME_PAD_SMALL);

	gtk_box_pack_start (GTK_BOX (font_vbox), font_hbox, FALSE, FALSE, GNOME_PAD_SMALL);
	gtk_container_add (GTK_CONTAINER (frame), font_vbox);

/*	gtk_box_pack_start (GTK_BOX (box), size, FALSE, FALSE, 0); */
/*	gtk_box_pack_start (GTK_BOX (box), best_size, FALSE, FALSE, 0); */
	gtk_box_pack_start (GTK_BOX (box), speed, FALSE, FALSE, GNOME_PAD_SMALL);
	
	gtk_widget_show_all (box);

	return box;
}

static void
apply_cb (GtkWidget *widget, gint page_num, AppletWidget *applet)
{
	memcpy (&props, &temp_props, sizeof(diskusage_properties));

	if (font_changed)
	{
		gchar *p = gnome_font_picker_get_font_name (GNOME_FONT_PICKER (gfp));

		g_free (props.font);

		props.font = g_strdup (p);

		load_my_font ();

		font_changed = FALSE;
	}

        setup_colors ();
	start_timer ();
	diskusage_resize ();
	applet_widget_sync_config (applet);
}

static gint
destroy_cb (GtkWidget *widget, void *data)
{
	propbox = NULL;

	return FALSE;
}

static void
phelp_cb (GtkWidget *w, gint tab, gpointer data)
{
	GnomeHelpMenuEntry help_entry = { "diskusage_applet",
					  "index.html#DISKUSAGE-APPLET-PREFS" };
	gnome_help_display(NULL, &help_entry);
}

void
properties (AppletWidget *applet, gpointer data)
{
	/* only show one property box at a time */
	if (propbox) {
		gdk_window_raise (propbox->window);
		return;
	}

	memcpy (&temp_props, &props, sizeof(diskusage_properties));

        propbox = gnome_property_box_new ();
	gtk_window_set_title (GTK_WINDOW (GNOME_PROPERTY_BOX (propbox)),
			      _("Diskusage Settings"));

	gnome_property_box_append_page (GNOME_PROPERTY_BOX (propbox),
					create_frame (&temp_props),
					gtk_label_new (_("General")));

	/* connect property box signals */
        gtk_signal_connect (GTK_OBJECT (propbox), "apply",
			    GTK_SIGNAL_FUNC (apply_cb), applet);

        gtk_signal_connect (GTK_OBJECT (propbox), "destroy",
			    GTK_SIGNAL_FUNC (destroy_cb), NULL);

        gtk_signal_connect (GTK_OBJECT (propbox), "help",
			    GTK_SIGNAL_FUNC (phelp_cb),
			    NULL);

	gtk_widget_show_all (propbox);

        return;
}
