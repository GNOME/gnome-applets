/*
 * Mini-Commander Applet
 * Copyright (C) 1998, 1999 Oliver Maruhn <oliver@maruhn.com>
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

/*
  If you expect tons of C code for command completion then you will
  probably be astonished...

  These routines have probably to be rewritten in future. But they
  should work on every system with a bash-alike shell.  
*/

#include <string.h>
#include <config.h>
#include <gnome.h>
#include <applet-widget.h>

#include <sys/stat.h>
#include <dirent.h>

#include "cmd_completion.h"
#include "preferences.h"
#include "macro.h"
#include "message.h"


static GList*    cmdc( char* );
static void      process_dir( const gchar* );
static void      cleanup( void );
static gint      g_list_str_cmp( gconstpointer, gconstpointer );

/* global declaration so g_atexit() can reference it to free it.
 * (not strictly necessary).
 */
static GList *path_elements = NULL;

void
cmd_completion(char *cmd)
{
    char buffer[MAX_COMMAND_LENGTH] = "";
    char largest_possible_completion[MAX_COMMAND_LENGTH] = "";
    int completion_not_unique = FALSE;   
    int num_whitespaces, i, pos;

    GList *possible_completions_list = NULL;
    GList *completion_element;

  
    if(strlen(cmd) == 0)
	{
	    show_message((gchar *) _("not unique"));
	    return;
	}

    show_message((gchar *) _("completing..."));
    num_whitespaces = prefix_length_Including_whithespaces(cmd) - prefix_length(cmd);
    possible_completions_list = cmdc(cmd + prefix_length_Including_whithespaces(cmd));

    /* get first possible completion */
    completion_element = g_list_first(possible_completions_list);
    if(completion_element)
	strcpy(largest_possible_completion, (char *) completion_element->data);
    else
	strcpy(largest_possible_completion, "");

    /* get the rest */
    while((completion_element = g_list_next(completion_element)))
	{
	    strcpy(buffer, (char *) completion_element->data);
	    completion_not_unique = TRUE;
	    pos = 0;
	    while(largest_possible_completion[pos] != '\000' 
		  && buffer[pos] != '\000'
		  && strncmp(largest_possible_completion, buffer, pos + 1) == 0)
		pos++;
	    strncpy(largest_possible_completion, buffer, pos);
	    /* strncpy does not add \000 to the end */
	    largest_possible_completion[pos] = '\000';
	}
      
    if(strlen(largest_possible_completion) > 0)
	{
	    if(get_prefix(cmd) != NULL)
		strcpy(cmd, get_prefix(cmd));
	    else
		strcpy(cmd, "");

	    /* fill up the whitespaces */
	    for(i = 0; i < num_whitespaces; i++)
		strcat(cmd, " ");	

	    strcat(cmd, largest_possible_completion);

	    if(!completion_not_unique)
		show_message((gchar *) _("completed"));
	    else
		show_message((gchar *) _("not unique"));		
	}
    else
	show_message((gchar *) _("not found"));
}



#if 0
static void
cmd_completion_old(char *cmd)
{
    FILE *pipe_fp;
    char buffer[MAX_COMMAND_LENGTH] = "";
    char shell_command[2048];
    char largest_possible_completion[MAX_COMMAND_LENGTH] = "";
    int completion_not_unique = FALSE;   
    int num_whitespaces, i, pos;

    static char shell_script[] = 
"\n\
for dir in `echo $PATH|sed \"s/^:/. /; s/:$/ ./; s/::/ . /g; s/:/ /g\"`\n\
do\n\
   for file in $dir/$cmd*\n\
   do\n\
      if test -x $file -a ! -d $file\n\
      then\n\
         echo `basename $file`\n\
      fi\n\
   done\n\
done\n\
";

  
    if(strlen(cmd) == 0)
	{
	    show_message((gchar *) _("not unique"));
	    return;
	}

    show_message((gchar *) _("completing..."));

    num_whitespaces = prefix_length_Including_whithespaces(cmd) - prefix_length(cmd);

    strcpy(shell_command, "/bin/sh -c '");
    strcat(shell_command, "cmd=\"");
    strcat(shell_command, cmd + prefix_length_Including_whithespaces(cmd));
    strcat(shell_command, "\"\n");
    strcat(shell_command, shell_script);
    strcat(shell_command, "'");
    
    if((pipe_fp = popen(shell_command, "r")) == NULL)
	show_message((gchar *) _("no /bin/sh"));

    /* get first line from shell script answer */
    fscanf(pipe_fp, "%s\n", largest_possible_completion);

    /* get the rest */
    while(fscanf(pipe_fp, "%s\n", buffer) == 1){
	completion_not_unique = TRUE;
	pos = 0;
	while(largest_possible_completion[pos] != '\000' 
	      && buffer[pos] != '\000'
	      && strncmp(largest_possible_completion, buffer, pos + 1) == 0)
	    pos++;
	strncpy(largest_possible_completion, buffer, pos);
	/* strncpy does not add \000 to the end */
	largest_possible_completion[pos] = '\000';
    }
    pclose(pipe_fp);
      
    if(strlen(largest_possible_completion) > 0)
	{
	    if(get_prefix(cmd) != NULL)
		strcpy(cmd, get_prefix(cmd));
	    else
		strcpy(cmd, "");

	    /* fill up the whitespaces */
	    for(i = 0; i < num_whitespaces; i++)
		strcat(cmd, " ");	

	    strcat(cmd, largest_possible_completion);

	    if(!completion_not_unique)
		show_message((gchar *) _("completed"));
	    else
		show_message((gchar *) _("not unique"));		
	}
    else
	show_message((gchar *) _("not found"));
}
#endif




/*
 * cmdc() -- command completion function.
 *
 * cmdc takes a char* and returns a GList* of possible completions.
 *
 * Initial version by Travis Hume <travishume@acm.org>.
 *
 */
static GList *
cmdc( char *s )
{
   GCompletion        *completion  = NULL;
   GList              *ret_list     = NULL;
   static GHashTable  *path_hash    = NULL;
   static char        *path        = NULL;
   gchar              *path_elem;
   struct stat         buf;
   static gboolean     inited      = FALSE;
   gpointer            hash_key     = NULL;


   /*
    * Only want to build the GCompletion once.  At some point I'd like to add
    * code to refresh the GCompletion, either at a regular interval, or when
    * there is a completion failure, ...
    *
    */
   if(!inited)
   {
      /* Make a local copy of the path variable. Otherwise the path
         environment variable would be modified. */
      path = (char *) malloc(sizeof(char) * (strlen(getenv("PATH")) + 1));
      strcpy(path, getenv("PATH"));
      
      path_hash = g_hash_table_new( g_str_hash, g_str_equal );

      for( path_elem = strtok( path, ":" ); path_elem;
            path_elem = strtok( NULL, ":" ))
      {
         if( stat( path_elem, &buf ))
            continue;

         if( buf.st_mode & S_IFDIR )
         {
            /* keep a hash of processed paths, to avoid reprocessing
             * dupped path entries.
             */
            hash_key = g_hash_table_lookup( path_hash, path_elem );
            if( hash_key )
               continue;   /* duplicate $PATH entry */
            else
            {
               g_hash_table_insert(
                     path_hash, (gpointer)path_elem, (gpointer)path_elem );

               process_dir( path_elem );
            }
         }
      }

      /* atexit() we want to free the completion. */
      g_atexit( cleanup );

      inited = TRUE;
   }

   completion = g_completion_new( NULL );
   g_completion_add_items( completion, path_elements );
   ret_list = g_list_copy( g_completion_complete( completion, s, NULL ));
   g_completion_free( completion );

   return g_list_sort( ret_list, (GCompareFunc)g_list_str_cmp );
}


/*
 * Compare function to return a sorted completion.
 */
static gint
g_list_str_cmp( gconstpointer a, gconstpointer b )
{
   return( strcmp( (char *)a, (char *)b ));
}



/*
 * Reads directory entries and adds non-dir, executable files
 * to the GCompletion.
 * Initial version by Travis Hume <travishume@acm.org>.
 */
static void
process_dir( const char *d )
{
   DIR            *dir;
   struct dirent  *de;
   struct stat     buf;
   gpointer        data;
   gchar          *path_str;


   if( (dir = opendir( d )) == NULL )
      return;

   while( (de = readdir( dir )) != NULL )
   {
      if( strcmp( de->d_name, "." ) == 0 || strcmp( de->d_name, "..") == 0)
         continue;

      path_str = (gchar *)g_malloc( strlen( d ) + 1 + strlen( de->d_name ) + 1);
      strcpy( path_str, d );
      strcat( path_str, "/" );
      strcat( path_str, de->d_name );
      if( stat( path_str, &buf ) != 0)
         continue;
      g_free( (gpointer)path_str );

      if( S_ISDIR( buf.st_mode ) )
         continue;

      data = g_malloc( strlen( (gchar *)de->d_name ) + 1 );
      strcpy( (gchar *)data, (gchar *)(de->d_name) );
      if( buf.st_mode & S_IXUSR )
         path_elements = g_list_append( path_elements, data );
   }
   closedir( dir );
}


/*
 * cleanup() -- free the memory used by the GCompletion.
 */
static void
cleanup( void )
{
   g_list_free( path_elements );
}
