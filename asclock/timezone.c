#include "asclock.h"
#include <stdio.h>

gint 
cmp (gconstpointer a, gconstpointer b)
{
  location one = *((location *)a);
  location two = *((location *)b);

  return (two.lon)-(one.lon);
}

void enum_timezones(GtkWidget *clist )
{
  FILE *tz;
  GSList * my;
  char line[1024];
  char *newelem[3] = { line, NULL, NULL };
  int inspos;
  tz = fopen("/usr/share/zoneinfo/zone.tab", "r");

  my = NULL;

  while(!feof(tz))
  {
    int i=0;
    int cnt=0;

    fgets(line, 1024, tz);
    if(line[0]=='#')
      continue;

    while((i<1024) && (cnt < 3))
      {
	while((i< 1024) && (line[i]!='\t') && !(cnt==2 && line[i]==' '))
	  i++;
	if(i==1024) break;
	line[i++]=0;
	newelem[cnt] = &(line[i]);
	cnt++;
      }
    
    if(i<1024)
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

      gtk_clist_append(GTK_CLIST(clist), elems );
      gtk_clist_set_row_data( GTK_CLIST(clist), inspos++, ((location *)my->data));

      my = g_slist_next(my);
    }
  fclose(tz);
}


