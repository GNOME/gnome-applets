#include <gtk/gtk.h>
#include <time.h>

extern GdkPixmap *month_pixmap;
extern GdkPixmap *led_pixmap;
extern GdkPixmap *weekday_pixmap;
extern GdkPixmap *month_pixmap;
extern GdkPixmap *date_pixmap;
extern GdkPixmap *clock_pixmap;

int ampm_posx[11] = {4, 8, 17, 22, 31, 45, 21, 21, 26, 31, 19};
int mil_posx[11] = {5+5, 5+14, 5+24, 5+28, 5+37, 45, 21, 21, 26, 31, 19};
int posy[4]  = {7, 25, 34, 49};

void Twelve(GdkPixmap *p, GdkGC *gc , struct tm *clk)
{
  int thishour;
  /* Stunde ohne am/pm */
  thishour = clk->tm_hour % 12;
  if (thishour == 0 )
    thishour = 12;

  gdk_draw_pixmap(p, gc, clock_pixmap, 0, 0, 0, 0, 64, 64);

  if (clk->tm_hour >= 12)
    /* PM */
    gdk_draw_pixmap(p, gc, led_pixmap, 107, 5,ampm_posx[5],posy[0]+5, 11, 6);
  else
    /* AM */
    gdk_draw_pixmap(p, gc, led_pixmap, 94, 5, ampm_posx[5],posy[0]+5, 12, 6);
  
  if (thishour>9)
    gdk_draw_pixmap(p, gc, led_pixmap, 13,0,ampm_posx[0], posy[0], 5, 11);

  gdk_draw_pixmap(p, gc, led_pixmap, 9*(thishour % 10),0,ampm_posx[1], posy[0], 9, 11);
  
  /* Minute, drawn first, so am/pm won't be overwritten */
  gdk_draw_pixmap(p, gc, led_pixmap, 9*(clk->tm_min / 10),0,ampm_posx[3],posy[0], 9, 11);
  gdk_draw_pixmap(p, gc, led_pixmap, 9*(clk->tm_min % 10),0,ampm_posx[4],posy[0], 9, 11);

  /* Date */
  if (clk->tm_mday>9)
    {
      gdk_draw_pixmap(p, gc, date_pixmap, 9*((clk->tm_mday / 10 +9) % 10),0,ampm_posx[7],posy[2], 9, 13);
      gdk_draw_pixmap(p, gc, date_pixmap, 9*((clk->tm_mday % 10 +9) % 10),0,ampm_posx[9],posy[2], 9, 13);
    }
  else
      gdk_draw_pixmap(p, gc, date_pixmap, 9*(clk->tm_mday -1),0, ampm_posx[8], posy[2], 9, 13);
    
  /* weekday */
  gdk_draw_pixmap(p, gc, weekday_pixmap, 0,6*((clk->tm_wday +6) % 7),ampm_posx[6], posy[1], 21, 7); 

  /* month */
  gdk_draw_pixmap(p, gc, month_pixmap, 0,6*(clk->tm_mon ),ampm_posx[10], posy[3], 22, 6);

  /* blinking second colon */
  if ((clk->tm_sec) % 2 ) 
    gdk_draw_pixmap(p, gc, led_pixmap, 90,0, ampm_posx[2], posy[0], 3,11);
  else
    gdk_draw_pixmap(p, gc, clock_pixmap, 27,6, ampm_posx[2], posy[0], 3,11);

}

void TwentyFour(GdkPixmap *p, GdkGC *gc , struct tm *clk)
{
  gdk_draw_pixmap(p, gc, clock_pixmap, 0, 0, 0, 0, 64, 64);

  gdk_draw_pixmap(p, gc, led_pixmap, 9*(clk->tm_hour / 10),0,mil_posx[0],posy[0], 9, 11);
  gdk_draw_pixmap(p, gc, led_pixmap, 9*(clk->tm_hour % 10),0,mil_posx[1],posy[0], 9, 11);

  /* Minute */
  gdk_draw_pixmap(p, gc, led_pixmap, 9*(clk->tm_min / 10),0,mil_posx[3],posy[0], 9, 11);
  gdk_draw_pixmap(p, gc, led_pixmap, 9*(clk->tm_min % 10),0,mil_posx[4],posy[0], 9, 11);
  
  /* Date */
  if (clk->tm_mday>9)
    {
      gdk_draw_pixmap(p, gc, date_pixmap, 9*((clk->tm_mday / 10 +9) % 10),0,mil_posx[7],posy[2], 9, 13);
      gdk_draw_pixmap(p, gc, date_pixmap, 9*((clk->tm_mday % 10 +9) % 10),0,mil_posx[9],posy[2], 9, 13);
    }
  else
      gdk_draw_pixmap(p, gc, date_pixmap, 9*(clk->tm_mday -1),0,mil_posx[8], posy[2], 9, 13);
    
  /* weekday */
  gdk_draw_pixmap(p, gc, weekday_pixmap, 0,6*((clk->tm_wday +6) % 7),mil_posx[6], posy[1], 21, 7); 

  /* month */
  gdk_draw_pixmap(p, gc, month_pixmap, 0,6*(clk->tm_mon ),mil_posx[10],posy[3], 22, 6);

  /* blinking second colon */
  if ( (clk->tm_sec) % 2 ) 
    gdk_draw_pixmap(p, gc, led_pixmap, 90,0, mil_posx[2], posy[0], 3,11);
  else
    gdk_draw_pixmap(p, gc, clock_pixmap, 27,6, mil_posx[2], posy[0], 3,11);

}

