/* File: preset.c
 * Purpose: GNOME Keyboard switcher property box
 *
 * Copyright (C) 1998-2000 Free Software Foundation
 * Authors: Szabolcs Ban  <shooby@gnome.hu>
 *          Chema Celorio <chema@celorio.com>
 *
 * Some of functions came from Helixcode's keyboard grabbing sections.
 * Other functions, ideas stolen from other applets, for example
 * from the fish applet, Wanda.
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

#include "gkb.h"

static char *prefixdir = NULL;

GList *
find_presets ()
{
  DIR *dir;
  struct dirent *actfile;
  GList *diritems = NULL;
  gchar *text;

  /* TODO: user's local presets */

  text = gnome_unconditional_datadir_file ("gnome/gkb/");
  if (!text)
  	return NULL;
  dir = opendir (text);

  if (prefixdir)
    g_free (prefixdir);
  prefixdir = g_strdup (text);
  
  g_free (text);

  if (dir == NULL)
    return NULL;

  while ((actfile = readdir (dir)) != NULL)
    {
      if (!strstr (actfile->d_name, ".keyprop"))
	continue;

      if (actfile->d_name[0] != '.')
	{
	  diritems = g_list_insert_sorted (diritems,
					   g_strdup (actfile->d_name),
					   (GCompareFunc) strcmp);
	}
    }
  closedir (dir);  
  return diritems;
}

static gint 
comparepreset(gconstpointer a, gconstpointer b)
{
 gint aa,bb;
 const GkbKeymap *k1 = a, *k2 = b;

 aa = strcoll (k1->lang, k2->lang);
 bb = strcoll (k2->lang, k1->lang);
 if (aa == bb) {
  aa = strcoll (k1->country, k2->country);
  bb = strcoll (k2->country, k1->country); 
  if (aa == bb) {
   return strcoll (k1->name, k2->name);
  } else {
   return aa;
  } 
 } else {
  return aa;
 } 
}

GList *
gkb_preset_load (GList * list)
{
  GkbKeymap *val;
  GList *retlist, *templist;
  gchar *prefix;
  gchar *tname, *tcodepage;
  gchar *ttype, *tarch, *tcommand;
  gchar *set, *filename;
  gint i, knum = 1;

  templist = list;
  retlist = NULL;

  for (templist = list; templist != NULL; templist = g_list_next (templist))
    {

      filename = templist->data;

      g_assert (filename != NULL);

      prefix =
	g_strconcat ("=", prefixdir, "/", filename, "=/Keymap Entry/", NULL);

      gnome_config_push_prefix (prefix);
      g_free (prefix);

      knum = gnome_config_get_int ("Countries");

      tname = gnome_config_get_translated_string ("Name");
      tcodepage = gnome_config_get_string ("Codepage");
      ttype = gnome_config_get_string ("Type");
      tarch = gnome_config_get_string ("Arch");
      tcommand = gnome_config_get_string ("Command");

      gnome_config_pop_prefix ();

      for (i = 0; i < knum; i++)
	{
	  val = g_new0 (GkbKeymap, 1);

	  set = g_strdup_printf ("=/Country %d/", i);
	  prefix = g_strconcat ("=", prefixdir, "/", filename, set, NULL);
	  gnome_config_push_prefix (prefix);
	  g_free (prefix);
	  g_free (set);

	  val->name = g_strdup (tname);
	  val->codepage = g_strdup (tcodepage);
	  val->type = g_strdup (ttype);
	  val->arch = g_strdup (tarch);
	  val->command = g_strdup (tcommand);

	  val->flag = gnome_config_get_string ("Flag");
	  val->label = gnome_config_get_translated_string ("Label");
	  if (val->label == NULL) val->label = g_strdup(_("Undefined"));

	  val->lang = gnome_config_get_translated_string ("Language");
	  if (val->lang == NULL) val->lang = g_strdup(_("Undefined"));

	  val->country = gnome_config_get_translated_string ("Country");
	  if (val->country == NULL) val->country = g_strdup(_("Undefined"));
	
	  retlist = g_list_append (retlist, val);

	  gnome_config_pop_prefix ();
	}

      g_free (tname);
      g_free (tcodepage);
      g_free (ttype);
      g_free (tarch);
      g_free (tcommand);

    }
  retlist = g_list_sort (retlist, comparepreset);

  return retlist;
}
