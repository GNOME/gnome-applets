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

#include <config.h>
#include <string.h>
#include <gnome.h>
#include <applet-widget.h>

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <libart_lgpl/art_alphagamma.h>
#include <libart_lgpl/art_filterlevel.h>
#include <libart_lgpl/art_pixbuf.h>
#include <libart_lgpl/art_rgb_rgba_affine.h>
#include <libart_lgpl/art_affine.h>
 
#include <sys/types.h>
#include <dirent.h>		/* for opendir() et al. */
#include <string.h>		/* for strncmp() */

#define debug(section,str) /*if (debug_turned_on) */ g_print ("%s:%d (%s) %s\n", __FILE__, __LINE__, __FUNCTION__, str); 

typedef struct _GKB GKB;
typedef struct _GkbKeymap          GkbKeymap;
typedef struct _GkbKeymapWg        GkbKeymapWg;
typedef struct _GkbPropertyBoxInfo GkbPropertyBoxInfo;

GKB * gkb;

struct _GkbPropertyBoxInfo
{
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
  GtkList *list;               /* The widget displaying the keymaps */
  GkbKeymap *selected_keymap;  /* A pointer to the selected keymap */
  
  /* Other properties */
  gint is_small;
  gint size;
};

struct _GkbKeymap
{
  gint i;
  GdkPixmap *pix;

  gchar *name;
  gchar *command;
  gchar *flag;
  gchar *country;
  gchar *lang;
  gchar *label;
  gchar *codepage;
  gchar *arch;
  gchar *type;
};

struct _GKB
{
  GtkWidget *applet;
  GtkWidget *frame;
  GtkWidget *darea;
  GtkWidget *mapedit;
  GtkWidget *addwindow;
  GtkWidget *propbox;

  gint n;
  gint tn;
  gint cur;
  gint size;
  gint w;
  gint h;
  gint is_small;
  gint mode;

  gchar *key;
  guint keysym, state;

  GList *maps;
  GkbKeymap *dact;
  PanelOrientType orient;
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

void gkb_update (GKB *gkb, gboolean set_command);

void properties_dialog (AppletWidget * applet);
GkbKeymap * loadprop (int i);
gboolean convert_string_to_keysym_state(const char *string,
					guint *keysym,
					guint *state);
char * convert_keysym_state_to_string(guint keysym,
					guint state);

GList * find_presets ();
GList * gkb_preset_load (GList * list);

/* presets.c */
GList *     find_presets (void);

/* prop-list.c */
GtkWidget * gkb_prop_create_buttons_vbox (GkbPropertyBoxInfo *pbi);
GtkWidget * gkb_prop_create_scrolled_window (GkbPropertyBoxInfo *pbi);

void gkb_prop_list_free_keymaps (GkbPropertyBoxInfo *pbi);

/* keygrab.c */
void grab_button_pressed (GtkButton *button, gpointer data);

gchar * prefixdir;
