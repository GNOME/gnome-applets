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
static void      processDir( const gchar* );
static GVoidFunc cleanup( void );
static gint      g_list_str_cmp( gconstpointer, gconstpointer );

/* global declaration so g_atexit() can reference it to free it.
 * (not strictly necessary).
 */
static GList *pathElements = NULL;



void
cmdCompletion(char *cmd)
{
    FILE *pipe_fp;
    char buffer[MAX_COMMAND_LENGTH] = "";
    char dummyBuffer[MAX_COMMAND_LENGTH] = "";
    char shellCommand[2048];
    char largestPossibleCompletion[MAX_COMMAND_LENGTH] = "";
    int completionNotUnique = FALSE;   
    int numWhitespaces, i, pos;

    GList *possible_completions_list = NULL;
    GList *completion_element;

  
    if(strlen(cmd) == 0)
	{
	    showMessage((gchar *) _("not unique"));
	    return;
	}

    showMessage((gchar *) _("completing..."));
    numWhitespaces = prefixLength_IncludingWhithespaces(cmd) - prefixLength(cmd);
    possible_completions_list = cmdc(cmd + prefixLength_IncludingWhithespaces(cmd));

    /* get first possible completion */
    completion_element = g_list_first(possible_completions_list);
    if(completion_element)
	strcpy(largestPossibleCompletion, (char *) completion_element->data);
    else
	strcpy(largestPossibleCompletion, "");

    /* get the rest */
    while((completion_element = g_list_next(completion_element)))
	{
	    strcpy(buffer, (char *) completion_element->data);
	    completionNotUnique = TRUE;
	    pos = 0;
	    while(largestPossibleCompletion[pos] != '\000' 
		  && buffer[pos] != '\000'
		  && strncmp(largestPossibleCompletion, buffer, pos + 1) == 0)
		pos++;
	    strncpy(largestPossibleCompletion, buffer, pos);
	    /* strncpy does not add \000 to the end */
	    largestPossibleCompletion[pos] = '\000';
	}
      
    if(strlen(largestPossibleCompletion) > 0)
	{
	    if(getPrefix(cmd) != NULL)
		strcpy(cmd, getPrefix(cmd));
	    else
		strcpy(cmd, "");

	    /* fill up the whitespaces */
	    for(i = 0; i < numWhitespaces; i++)
		strcat(cmd, " ");	

	    strcat(cmd, largestPossibleCompletion);

	    if(!completionNotUnique)
		showMessage((gchar *) _("completed"));
	    else
		showMessage((gchar *) _("not unique"));		
	}
    else
	showMessage((gchar *) _("not found"));
}



static void
cmdCompletion_old(char *cmd)
{
    FILE *pipe_fp;
    char buffer[MAX_COMMAND_LENGTH] = "";
    char dummyBuffer[MAX_COMMAND_LENGTH] = "";
    char shellCommand[2048];
    char largestPossibleCompletion[MAX_COMMAND_LENGTH] = "";
    int completionNotUnique = FALSE;   
    int numWhitespaces, i, pos;

    static char shellScript[] = 
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
	    showMessage((gchar *) _("not unique"));
	    return;
	}

    showMessage((gchar *) _("completing..."));

    numWhitespaces = prefixLength_IncludingWhithespaces(cmd) - prefixLength(cmd);

    strcpy(shellCommand, "/bin/sh -c '");
    strcat(shellCommand, "cmd=\"");
    strcat(shellCommand, cmd + prefixLength_IncludingWhithespaces(cmd));
    strcat(shellCommand, "\"\n");
    strcat(shellCommand, shellScript);
    strcat(shellCommand, "'");
    
    if((pipe_fp = popen(shellCommand, "r")) == NULL)
	showMessage((gchar *) _("no /bin/sh"));

    /* get first line from shell script answer */
    fscanf(pipe_fp, "%s\n", largestPossibleCompletion);

    /* get the rest */
    while(fscanf(pipe_fp, "%s\n", buffer) == 1){
	completionNotUnique = TRUE;
	pos = 0;
	while(largestPossibleCompletion[pos] != '\000' 
	      && buffer[pos] != '\000'
	      && strncmp(largestPossibleCompletion, buffer, pos + 1) == 0)
	    pos++;
	strncpy(largestPossibleCompletion, buffer, pos);
	/* strncpy does not add \000 to the end */
	largestPossibleCompletion[pos] = '\000';
    }
    pclose(pipe_fp);
      
    if(strlen(largestPossibleCompletion) > 0)
	{
	    if(getPrefix(cmd) != NULL)
		strcpy(cmd, getPrefix(cmd));
	    else
		strcpy(cmd, "");

	    /* fill up the whitespaces */
	    for(i = 0; i < numWhitespaces; i++)
		strcat(cmd, " ");	

	    strcat(cmd, largestPossibleCompletion);

	    if(!completionNotUnique)
		showMessage((gchar *) _("completed"));
	    else
		showMessage((gchar *) _("not unique"));		
	}
    else
	showMessage((gchar *) _("not found"));
}




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
   GList              *retList     = NULL;
   static GHashTable  *pathHash    = NULL;
   static char        *path        = NULL;
   gchar              *pathElem;
   struct stat         buf;
   static gboolean     inited      = FALSE;
   gpointer            hashKey     = NULL;


   /*
    * Only want to build the GCompletion once.  At some point I'd like to add
    * code to refresh the GCompletion, either at a regular interval, or when
    * there is a completion failure, ...
    *
    */
   if( !inited )
   {
      path = getenv( "PATH" );
      pathHash = g_hash_table_new( g_str_hash, g_str_equal );

      for( pathElem = strtok( path, ":" ); pathElem;
            pathElem = strtok( NULL, ":" ))
      {
         if( stat( pathElem, &buf ))
            continue;

         if( buf.st_mode & S_IFDIR )
         {
            /* keep a hash of processed paths, to avoid reprocessing
             * dupped path entries.
             */
            hashKey = g_hash_table_lookup( pathHash, pathElem );
            if( hashKey )
               continue;   /* duplicate $PATH entry */
            else
            {
               g_hash_table_insert(
                     pathHash, (gpointer)pathElem, (gpointer)pathElem );

               processDir( pathElem );
            }
         }
      }

      /* atexit() we want to free the completion. */
      g_atexit( (GVoidFunc)cleanup );

      inited = TRUE;
   }

   completion = g_completion_new( NULL );
   g_completion_add_items( completion, pathElements );
   retList = g_list_copy( g_completion_complete( completion, s, NULL ));
   g_completion_free( completion );

   return g_list_sort( retList, (GCompareFunc)g_list_str_cmp );
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
processDir( const char *d )
{
   DIR            *dir;
   struct dirent  *de;
   struct stat     buf;
   GList          *list     = NULL;
   gpointer        data;
   gchar          *pathStr;


   if( (dir = opendir( d )) == NULL )
      return;

   while( (de = readdir( dir )) != NULL )
   {
      if( strcmp( de->d_name, "." ) == 0 || strcmp( de->d_name, "..") == 0)
         continue;

      pathStr = (gchar *)g_malloc( strlen( d ) + 1 + strlen( de->d_name ) + 1);
      strcpy( pathStr, d );
      strcat( pathStr, "/" );
      strcat( pathStr, de->d_name );
      if( stat( pathStr, &buf ) != 0)
         continue;
      g_free( (gpointer)pathStr );

      if( S_ISDIR( buf.st_mode ) )
         continue;

      data = g_malloc( strlen( (gchar *)de->d_name ) + 1 );
      strcpy( (gchar *)data, (gchar *)(de->d_name) );
      if( buf.st_mode & S_IXUSR )
         pathElements = g_list_append( pathElements, data );
   }
   closedir( dir );
}


/*
 * cleanup() -- free the memory used by the GCompletion.
 */
static GVoidFunc
cleanup( void )
{
   g_list_free( pathElements );
}
