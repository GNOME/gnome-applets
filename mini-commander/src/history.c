/*
 * Mini-Commander Applet
 * Copyright (C) 1998 Oliver Maruhn <oliver@maruhn.com>
 *
 * Author: Oliver Maruhn <oliver@maruhn.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* Actually the command history is a simple list.  So, I guess this
   here could also be done with the list routines of glib. */

#include <config.h>
#include <string.h>
#include <gnome.h>
#include <gconf/gconf.h>
#include <panel-applet.h>
#include <panel-applet-gconf.h>

#include "history.h"
#include "preferences.h"
#include "message.h"

static void delete_history_entry(MCData *mcdata, int element_number);


int
exists_history_entry(MCData *mcdata, int pos)
{
    return(mcdata->history_command[pos] != NULL);
}

char *
get_history_entry(MCData *mcdata, int pos)
{
    return(mcdata->history_command[pos]);
}

void
set_history_entry(MCData *mcdata, int pos, char * entry)
{
    if(mcdata->history_command[pos] != NULL)
	free(mcdata->history_command[pos]);
    mcdata->history_command[pos] = (char *)malloc(sizeof(char) * (strlen(entry) + 1));
    strcpy(mcdata->history_command[pos], entry);
}

/* load_history indicates whether the history list is being loaded at startup.
** If true then don't save the new entries with gconf (since they are being read
** using gconf
*/
void
append_history_entry(MCData *mcdata, char * entry, gboolean load_history)
{
    PanelApplet *applet = mcdata->applet;
    GConfValue *history;
    GSList *list = NULL;
    int pos, i;

    /* remove older dupes */
    for(pos = 0; pos <= LENGTH_HISTORY_LIST - 1; pos++)
	{
	    if(exists_history_entry(mcdata, pos) && strcmp(entry, mcdata->history_command[pos]) == 0)
		/* dupe found */
		delete_history_entry(mcdata, pos);
	}

    /* delete oldest entry */
    if(mcdata->history_command[0] != NULL)
	free(mcdata->history_command[0]);

    /* move entries */
    for(pos = 0; pos < LENGTH_HISTORY_LIST - 1; pos++)
	{
	    mcdata->history_command[pos] = mcdata->history_command[pos+1];
	    /* printf("%s\n", mcdata->history_command[pos]); */
	}

    /* append entry */
    mcdata->history_command[LENGTH_HISTORY_LIST - 1] = (char *)malloc(sizeof(char) * (strlen(entry) + 1));
    strcpy(mcdata->history_command[LENGTH_HISTORY_LIST - 1], entry);
    
    if (load_history)
    	return;
    	
    /* Save history - this seems like a waste to do it every time it's updated 
    ** but it doesn't seem to work when called on the destroy signal of the applet 
    */
    for(i = 0; i < LENGTH_HISTORY_LIST; i++)
	{
	    GConfValue *entry;
	    
	    entry = gconf_value_new (GCONF_VALUE_STRING);
	    if(exists_history_entry(mcdata, i)) {
	    	gconf_value_set_string (entry, (gchar *) get_history_entry(mcdata, i));
	    	list = g_slist_append (list, entry);
	    }        
	    
	}

    history = gconf_value_new (GCONF_VALUE_LIST);
    if (list) {
    	gconf_value_set_list_type (history, GCONF_VALUE_STRING);
        gconf_value_set_list (history, list);
        panel_applet_gconf_set_value (applet, "history", history, NULL);
    }
   
    while (list) {
    	GConfValue *value = list->data;
    	gconf_value_free (value);
    	list = g_slist_next (list);
    }
   
    gconf_value_free (history);
    
}

void
delete_history_entry(MCData *mcdata, int element_number)
{
    int pos;

    for(pos = element_number; pos > 0; --pos)
	mcdata->history_command[pos] = mcdata->history_command[pos - 1];

    mcdata->history_command[0] = NULL;   
}
