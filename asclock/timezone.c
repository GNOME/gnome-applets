#include "asclock.h"
#include <stdio.h>

static gint 
cmp (gconstpointer a, gconstpointer b)
{
  location one = *((location *)a);
  location two = *((location *)b);

  return (two.lon)-(one.lon);
}

void enum_timezones(asclock *my_asclock, GtkWidget *clist )
{
  FILE *tz;
  GSList * my;
  char line[1024], *pos;
  char *newelem[3] = { line, NULL, NULL };
  gpointer localtime_data = NULL;
  int inspos, len, row;

  tz = fopen("/usr/share/zoneinfo/zone.tab", "r");
  if(!tz) {
    fprintf(stderr, "/usr/share/zoneinfo/zone.tab not found...\n");
    return;
  }

  len = readlink("/etc/localtime", line, sizeof(line)-1);
  if(len>=0)
    {
      line [len] = 0;
 
      pos = strstr (line, "zoneinfo/");
      if (pos)
	{
	  pos += strlen ("zoneinfo/");
	  strncpy(my_asclock->timezone, g_strdup (pos), MAX_PATH_LEN);
	  my_asclock->timezone_changed = FALSE;
	}
    }
  my = NULL;

  while(1)
  {
    int i=0;
    int cnt=0;
    
    memset(line, 0, sizeof (line));
    fgets(line, 1024, tz);
    if(feof(tz)) break;
    if((line[0]=='#') || (line[0]==0))
      continue;

    while((i<1024) && (cnt < 3))
      {
	while((i< 1024) && (line[i]!='\t') && line[i]!='\n' && !(cnt==2 && line[i]==' '))
	  i++;
	if(i==1024) break;
	line[i++]=0;
	newelem[cnt] = &(line[i]);
	cnt++;
      }
    
    if((i<1024) && (strlen(newelem[1])>0))
      {
	gint lat, lon;
	location *loc = malloc(sizeof(location));

	sscanf(newelem[0], "%d%d", &lat, &lon);
	if(newelem[0][5]=='+' || newelem[0][5]=='-')
	  {
	    loc->lon = lon/100.0;
	    loc->lat = lat/100.0;
	  }
	else
	  {
	    loc->lon = lon/10000.0;
	    loc->lat = lat/10000.0;    
	  }
	strcpy((loc->name), newelem[1]);

	my = g_slist_insert_sorted(my, loc, cmp);
      }

  }

  inspos = 0;
  while(my)
    {
      char *elems[2] = { ((location *)my->data)->name, NULL };

      if ((strlen(my_asclock->timezone)>0) && !strcmp (elems [0], my_asclock->timezone))
	localtime_data = (location *) my->data;

      gtk_clist_append(GTK_CLIST(clist), elems );
      gtk_clist_set_row_data( GTK_CLIST(clist), inspos++, ((location *)my->data));

      my = g_slist_next(my);
    }
  fclose(tz);

  gtk_clist_sort(GTK_CLIST(clist));
  
  row = gtk_clist_find_row_from_data(GTK_CLIST(clist), localtime_data);
  if (row >= 0) 
    {
      gtk_clist_moveto(GTK_CLIST(clist), row, 0, 0.5, 0.0);
      gtk_clist_select_row(GTK_CLIST(clist), row, 0);
    }
}


