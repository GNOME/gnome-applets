/* File: gkb.h
 * Purpose: GNOME Keyboard switcher header file
 *
 * Copyright (C) 1999 Free Software Foundation
 * Author: Szabolcs BAN <shooby@gnome.hu>, 1998-2000
 *
 * Thanks for aid of Balazs Nagy <julian7@kva.hu>,
 * Charles Levert <charles@comm.polymtl.ca>,
 * George Lebl <jirka@5z.com> and solidarity
 * Emese Kovacs <emese@eik.bme.hu>.
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
#include <libart_lgpl/art_rgb_pixbuf_affine.h>
#include <libart_lgpl/art_affine.h>
 
#include <sys/types.h>
#include <dirent.h>		/* for opendir() et al. */
#include <string.h>		/* for strncmp() */

typedef struct _Prop Prop;
struct _Prop
{
  int i;
  GdkPixmap *pix;

  char *name;
  char *command;
  char *iconpath;

};

typedef struct _GKB GKB;
struct _GKB
{
  GtkWidget *applet;
  GtkWidget *frame;
  GtkWidget *darea;
  GtkWidget *propbox;
  GtkWidget *notebook;

  int n, tn, cur, size, w, h;

  GList *maps;
  GList *tempmaps;
  Prop *dact;
  PanelOrientType orient;

};

void properties_dialog (AppletWidget * applet, gpointer gkbx);
void sized_render (GKB * gkb);
void gkb_draw (GKB * gkb);
Prop * loadprop (GKB * gkb, int i);
int applet_save_session (GtkWidget * w, const char *privcfgpath,
                     const char *globcfgpath, GKB * gkb);
