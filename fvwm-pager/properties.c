
/* fvwm-pager
 *
 * Copyright (C) 1998 Michael Lausch
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <gnome.h>
#include "gtkpager.h"
#include "properties.h"

static GtkWidget* propbox = 0;
static PagerProps temp_props;
extern PagerProps pager_props;

gchar confpath[1024];

void
load_fvwmpager_properties(gchar* path, PagerProps* prop)
{
  g_snprintf(confpath, sizeof(confpath), "/%s/gui/", path);
  gnome_config_push_prefix(confpath);
  prop->active_desk_color   = gnome_config_get_string("active_desk=red");
  prop->active_win_color    = gnome_config_get_string("active_win=MintCream");
  prop->inactive_desk_color = gnome_config_get_string("inactive_desk=NavajoWhite");
  prop->inactive_win_color  = gnome_config_get_string("inactive_win=lavender");
  prop->width               = gnome_config_get_int("width=170");
  prop->height              = gnome_config_get_int("height=70");
  prop->window_drag_cursor  = 0;
  prop->default_cursor      = 0;
  gnome_config_pop_prefix();
}


void
save_fvwmpager_properties(gchar* path, PagerProps* props)
{
  g_snprintf(confpath, sizeof(confpath), "/%s/gui/", path);
  gnome_config_push_prefix(confpath);
  gnome_config_set_string("active_desk", props->active_desk_color);
  gnome_config_set_string("active_win", props->active_win_color);
  gnome_config_set_string("inactive_desk", props->inactive_desk_color);
  gnome_config_set_string("inactive_win", props->inactive_win_color);
  gnome_config_set_int("width", props->width);
  gnome_config_set_int("height", props->height);
  gnome_config_pop_prefix();
  gnome_config_sync();
}

static void
color_changed_cb (GnomeColorPicker* widget, int r, int g, int b, int a, gchar** color)
{
  char *tmp;


  tmp = g_malloc (24);
  snprintf (tmp, 24, "#%04x%04x%04x", r, g, b);
  *color = tmp;
  gnome_property_box_changed (GNOME_PROPERTY_BOX(propbox));
}


static void
height_cb (GtkWidget *widget, GtkWidget *spin)
{
	temp_props.height =
		gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(spin));
        gnome_property_box_changed (GNOME_PROPERTY_BOX(propbox));
}

static void
width_cb (GtkWidget *widget, GtkWidget *spin)
{
	temp_props.width =
		gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(spin));
        gnome_property_box_changed (GNOME_PROPERTY_BOX(propbox));
}

static GtkWidget *
create_properties_frame (void)
{
  GtkWidget* label;
  GtkWidget* box;
  GtkWidget* wcolor;
  GtkWidget* dcolor;
  GtkWidget* color;
  GtkWidget* size;
  GtkWidget* height;
  GtkWidget* width;
  GtkWidget* height_a;
  GtkWidget* width_a;
  
  GtkWidget* actwin_gcs;
  GtkWidget* inactwin_gcs;
  GtkWidget* actdesk_gcs;
  GtkWidget* inactdesk_gcs;
  gint                adr, adg, adb;
  gint                idr, idg, idb;
  gint                awr, awg, awb;
  gint                iwr, iwg, iwb;
  
  sscanf (temp_props.active_desk_color,   "#%04x%04x%04x", &adr, &adg, &adb);
  sscanf (temp_props.inactive_desk_color, "#%04x%04x%04x", &idr, &idg, &idb);
  sscanf (temp_props.active_win_color,    "#%04x%04x%04x", &awr, &awg, &awb);
  sscanf (temp_props.inactive_win_color,  "#%04x%04x%04x", &iwr, &iwg, &iwb);
  
  box   = gtk_vbox_new (5, TRUE);
  wcolor = gtk_vbox_new (5, TRUE);
  dcolor = gtk_vbox_new (5, TRUE);
  color  = gtk_hbox_new (5, TRUE);
  size  = gtk_hbox_new (5, TRUE);
  gtk_container_set_border_width (GTK_CONTAINER(box), 5);

  gtk_box_pack_start_defaults(GTK_BOX(color), dcolor);
  gtk_box_pack_start_defaults(GTK_BOX(color), wcolor);
  
  
  actwin_gcs =  gnome_color_picker_new();
  gnome_color_picker_set_use_alpha(GNOME_COLOR_PICKER(actwin_gcs), 0);
  gnome_color_picker_set_title(GNOME_COLOR_PICKER(actwin_gcs), _("Active Window Color"));
  
  inactwin_gcs = gnome_color_picker_new();
  gnome_color_picker_set_use_alpha(GNOME_COLOR_PICKER(actwin_gcs), 0);
  gnome_color_picker_set_title(GNOME_COLOR_PICKER(actwin_gcs), _("Inactive Window Color"));
  
  actdesk_gcs = gnome_color_picker_new();
  gnome_color_picker_set_use_alpha(GNOME_COLOR_PICKER(actwin_gcs), 0);
  gnome_color_picker_set_title(GNOME_COLOR_PICKER(actwin_gcs), _("Active Desktop Color"));
  
  inactdesk_gcs = gnome_color_picker_new();
  gnome_color_picker_set_use_alpha(GNOME_COLOR_PICKER(actwin_gcs), 0);
  gnome_color_picker_set_title(GNOME_COLOR_PICKER(actwin_gcs), _("Inactive Desktop Color"));
  
  gnome_color_picker_set_i16 (GNOME_COLOR_PICKER(actwin_gcs),   awr, awg, awb, 255);
  gnome_color_picker_set_i16 (GNOME_COLOR_PICKER(inactwin_gcs), iwr, iwg, iwb, 255);
  gnome_color_picker_set_i16 (GNOME_COLOR_PICKER(actdesk_gcs),  adr, adg, adb, 255);
  gnome_color_picker_set_i16 (GNOME_COLOR_PICKER(inactdesk_gcs),idr, idg, idb, 255);
  gtk_signal_connect(GTK_OBJECT(actdesk_gcs), "color-set",
		     GTK_SIGNAL_FUNC(color_changed_cb), &temp_props.active_desk_color);
  gtk_signal_connect(GTK_OBJECT(inactdesk_gcs), "color-set",
		     GTK_SIGNAL_FUNC(color_changed_cb), &temp_props.inactive_desk_color);
  gtk_signal_connect(GTK_OBJECT(actwin_gcs), "color-set",
		     GTK_SIGNAL_FUNC(color_changed_cb), &temp_props.active_win_color);
  gtk_signal_connect(GTK_OBJECT(inactwin_gcs), "color-set",
		     GTK_SIGNAL_FUNC(color_changed_cb), &temp_props.inactive_win_color);
  
		     
  
  label = gtk_label_new (_("Active Window Color"));
  gtk_box_pack_start_defaults(GTK_BOX (wcolor), label);
  gtk_box_pack_start_defaults(GTK_BOX (wcolor), actwin_gcs);
  
  label = gtk_label_new (_("Inactive Window Color"));
  gtk_box_pack_start_defaults(GTK_BOX(wcolor), label);
  gtk_box_pack_start_defaults(GTK_BOX(wcolor), inactwin_gcs);

  label = gtk_label_new (_("Active Desktop Color"));
  gtk_box_pack_start_defaults(GTK_BOX (dcolor), label);
  gtk_box_pack_start_defaults(GTK_BOX (dcolor), actdesk_gcs);
  
  label = gtk_label_new (_("Inactive Desktop Color"));
  gtk_box_pack_start_defaults(GTK_BOX(dcolor), label);
  gtk_box_pack_start_defaults(GTK_BOX(dcolor), inactdesk_gcs);
  
  label = gtk_label_new (_("Applet Height"));
  height_a = gtk_adjustment_new(temp_props.height, 0.5, 128, 1, 8, 8);
  height  = gtk_spin_button_new(GTK_ADJUSTMENT (height_a), 1, 0);
  gtk_box_pack_start_defaults(GTK_BOX (size), label);
  gtk_box_pack_start_defaults(GTK_BOX (size), height);
  
  label = gtk_label_new (_("Width"));
  width_a = gtk_adjustment_new(temp_props.width, 0.5, 128, 1, 8, 8);
  width  = gtk_spin_button_new(GTK_ADJUSTMENT (width_a), 1, 0);
  gtk_box_pack_start_defaults(GTK_BOX (size), label);
  gtk_box_pack_start_defaults(GTK_BOX (size), width);
  
  gtk_signal_connect (GTK_OBJECT (height_a), "value_changed",
		      GTK_SIGNAL_FUNC (height_cb), height);
  gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (height),
				     GTK_UPDATE_ALWAYS);
  gtk_signal_connect (GTK_OBJECT (width_a), "value_changed",
		      GTK_SIGNAL_FUNC (width_cb), width);
  gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (width),
				     GTK_UPDATE_ALWAYS);
  
  
  gtk_box_pack_start_defaults (GTK_BOX (box), color);
  gtk_box_pack_start_defaults (GTK_BOX (box), size);
  
  gtk_widget_show_all (box);
  return box;
}

static void
apply_cb (GtkWidget *widget, gint dummy, gpointer data)
{

  GtkFvwmPager* pager = data;
  memcpy(&pager_props, &temp_props, sizeof(PagerProps));
  gtk_fvwmpager_prop_changed(pager);
}

static gint
destroy_cb (GtkWidget *widget, gpointer data)
{
  propbox = NULL;
  return FALSE;
}


void
pager_properties_dialog(AppletWidget *applet, gpointer data)
{
  GtkWidget* frame;
  GtkWidget* label;
  
  if (propbox)
    {
      gdk_window_raise (propbox->window);
      return;
    }

  memcpy (&temp_props, &pager_props, sizeof (PagerProps));

  propbox = gnome_property_box_new ();
  
  gtk_window_set_title
    (GTK_WINDOW(&GNOME_PROPERTY_BOX(propbox)->dialog.window),
     _("Fvwm Pager Settings"));
	
  frame = create_properties_frame();

  label = gtk_label_new (_("General"));
  gtk_widget_show (frame);
  gtk_notebook_append_page
    (GTK_NOTEBOOK(GNOME_PROPERTY_BOX(propbox)->notebook),
     frame, label);
  
  gtk_signal_connect (GTK_OBJECT (propbox), "apply",
		      GTK_SIGNAL_FUNC (apply_cb), data);
  
  gtk_signal_connect (GTK_OBJECT (propbox), "destroy",
		      GTK_SIGNAL_FUNC (destroy_cb), data);
  
  gtk_widget_show_all (propbox);
}
