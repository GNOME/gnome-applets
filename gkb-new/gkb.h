/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 2 -*- */
/* File: gkb.h
 * Purpose: GNOME Keyboard switcher header file
 *
 * Copyright (C) 1998-2000 Free Software Foundation
 * Authors: Szabolcs Ban  <shooby@gnome.hu>
 *          Chema Celorio <chema@celorio.com>
 *
 * Thanks for aid of George Lebl <jirka@5z.com> and solidarity
 * Balazs Nagy <js@lsc.hu>, Charles Levert <charles@comm.polymtl.ca>
 * and Emese Kovacs <emese@gnome.hu> for her docs and ideas.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include <string.h>
#include <gnome.h>

#include <panel-applet.h>

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <libart_lgpl/art_alphagamma.h>
#include <libart_lgpl/art_filterlevel.h>
#include <libart_lgpl/art_pixbuf.h>
#include <libart_lgpl/art_rgb_rgba_affine.h>
#include <libart_lgpl/art_affine.h>

#include <gconf/gconf-client.h>
 
#include <sys/types.h>
#include <dirent.h>		/* for opendir() et al. */
#include <string.h>		/* for strncmp() */

typedef struct _GKB GKB;
typedef struct _GkbKeymap          GkbKeymap;
typedef struct _GkbWindow          GkbWindow;
typedef struct _GkbKeymapWg        GkbKeymapWg;
typedef struct _GkbPropertyBoxInfo GkbPropertyBoxInfo;

typedef enum {
  GKB_LABEL,
  GKB_FLAG,
  GKB_FLAG_AND_LABEL
} GkbMode;

struct _GkbPropertyBoxInfo
{
  GKB *gkb;
  
  GtkWidget *box;

  /* Buttons */
  GtkWidget *buttons_vbox;
  GtkWidget *add_button;
  GtkWidget *edit_button;
  GtkWidget *up_button;
  GtkWidget *down_button;
  GtkWidget *delete_button;

  /* Keymaps */
  GList *keymaps;              /* A list of GkbKemap pointers */
  GtkTreeView *list;           /* The widget displaying the keymaps */
  GkbKeymap *selected_keymap;  /* A pointer to the selected keymap */

  /* Hotkey Entry */
  GtkWidget *hotkey_entry;

  /* Selected keymap to add */
  GkbKeymap *keymap_for_add;

  GtkTreeStore *model;

  /* Other properties */
  gint is_small; 
  GkbMode mode;
};


struct _GkbKeymap
{
  gint i;
  GdkPixbuf *pixbuf;

  gchar *name;
  gchar *command;
  gchar *flag;
  gchar *country;
  gchar *lang;
  gchar *label;
  gchar *codepage;
  gchar *arch;
  gchar *type;
  GkbKeymap *parent; /* The temp keymaps that are copied have a reference to their parents */
};

struct _GKB
{
  /* Keymaps */
  GkbKeymap *keymap; /* This is the currently selected keymap */

  GList *maps;
  
  /* Properties */
  PanelAppletOrient orient;
  GkbMode mode;
  gint is_small;
  gint w;
  gint h;

  /* Widgets */
  GtkWidget *applet;
  GtkWidget *eventbox;
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *label1;
  GtkWidget *label_frame1;
  GtkWidget *label2;
  GtkWidget *label_frame2;
  GtkWidget *darea_frame;
  GtkWidget *image;
  GtkWidget *addwindow;
  GtkTooltips *tooltips;

  gint n;
  gint cur;

  gchar *key, *old_key;
  guint keysym, state;
  gint keycode;
  guint old_keysym, old_state;

  guint button_press_id;
};

struct _GkbKeymapWg
{
  GdkPixmap *pix;

  char *name;
  char *command;
  char *flag;

  GtkWidget *diff_ch;
  GtkWidget *propbox;
  GtkWidget *notebook;
  GtkWidget *label1;
  GtkWidget *iconentry;
  GtkWidget *keymapname;
  GtkWidget *commandinput;
  GtkWidget *iconpathinput;
  GtkWidget *scrolledwin, *scrolledwinl;
  GtkWidget *hidebox, *hfa, *hfn;
  GtkWidget *frame21, *frame22, *label25, *entry21;
  GtkWidget *vbox1, *hbox1, *vbox2, *hbox2, *hbox3, *hboxmap;
  GtkWidget *frame1, *frame2, *frame3, *frame4, *frame6;
  GtkWidget *vbox21, *hbox21;
  GtkWidget *list, *tree, *iconentry21;
  GtkWidget *newkeymap, *delkeymap;
};

#define GKB_SMALL_PANEL_SIZE 25 /* less than */

/* gkb.c */
void gkb_update (GKB *gkb, gboolean set_command);
void alert (const gchar * str);
void applet_save_session (GKB *gkb);

void add_atk_relation(GtkWidget *obj1, GtkWidget *obj2, AtkRelationType type);  
void add_atk_namedesc(GtkWidget *widget, const gchar *name, const gchar *desc); 

/* prop.c */
void gkb_apply (GkbPropertyBoxInfo * pbi);
void properties_dialog (BonoboUIComponent *uic,
                        GKB	 *gkb,
                        const gchar	 *verbname);
void gkb_sized_render (GKB * gkb);

/* presets.c */
GList * find_presets (void);
GList * gkb_preset_load (GList * list);
GkbKeymap * loadprop (GKB *gkb, int i);

/* prop-list.c */
GtkWidget * gkb_prop_create_buttons_vbox (GkbPropertyBoxInfo *pbi);
GtkWidget * gkb_prop_create_scrolled_window (GkbPropertyBoxInfo *pbi);
       void gkb_prop_list_reload (GkbPropertyBoxInfo *pbi);

/* system.c */
void gkb_system_set_keymap (GKB * gkb);

/* keymap.c */
GkbKeymap * gkb_keymap_copy (GkbKeymap *keymap);
    GList * gkb_keymap_copy_list (GList *list_in);
       void gkb_keymap_free_internals (GkbKeymap *keymap);
       void gkb_keymap_free (GkbKeymap *keymap);
       void gkb_keymap_free_list (GList *list_in);

/* prop-map.h */
void gkb_prop_map_edit (GkbPropertyBoxInfo *pbi);
void gkb_prop_map_add (GkbPropertyBoxInfo *pbi);


/* util.c */
const gchar *  gkb_util_get_text_from_mode (GkbMode mode);
gint  gkb_util_get_int_from_mode (GkbMode mode);
GkbMode gkb_util_get_mode_from_text (const gchar *text);

/* keygrab.c */
gboolean convert_string_to_keysym_state(const char *string,
					guint *keysym,
					guint *state);
char * convert_keysym_state_to_string(guint keysym,
					guint state);
void gkb_xgrab   (int        keycode,
		  int        modifiers,
		  GdkWindow *root_window);
void gkb_xungrab (int        keycode,
		  int        modifiers,
		  GdkWindow *root_window);

void grab_button_pressed (GtkButton *button, gpointer data);

/* gconf.c */

gboolean gconf_applet_set_string (PanelApplet *parent, 
                                  const char *gconf_key,
                                  gchar *value,
                                  gchar *blah);

gchar * gconf_applet_get_string  (PanelApplet *parent, 
                                  const char *gconf_key,
                                  gchar *blah);

gboolean gconf_applet_set_int (PanelApplet *parent, 
                                  const char *gconf_key,
                                  gint value,
                                  gchar *blah);

gint gconf_applet_get_int  (PanelApplet *parent, 
                                  const char *gconf_key,
                                  gchar *blah);

gboolean gconf_applet_set_bool (PanelApplet *parent, 
                                  const char *gconf_key,
                                  gboolean value,
                                  gchar *blah);

gboolean gconf_applet_get_bool  (PanelApplet *parent, 
                                  const char *gconf_key,
                                  gchar *blah);

/* Globals */
gchar * prefixdir;

void gkb_update_handlers (GKB *gkb, gboolean disconnect);

G_BEGIN_DECLS

gboolean fill_gkb_applet(PanelApplet *applet);

G_END_DECLS
