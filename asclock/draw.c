#include "asclock.h"

extern GdkPixmap *month_pixmap;
extern GdkPixmap *led_pixmap;
extern GdkPixmap *weekday_pixmap;
extern GdkPixmap *month_pixmap;
extern GdkPixmap *date_pixmap;
extern GdkPixmap *clock_pixmap;

void analog(asclock *my, GdkPixmap *p, GdkGC *gc, struct tm *clk)
{
  double t;
  if(clock_img)
    gdk_image_destroy(clock_img); 
  
  clock_img = gdk_image_get(p, 0, 0,
			    ((GdkWindowPrivate *)clock_pixmap)->width, 
			    ((GdkWindowPrivate *)clock_pixmap)->height);

  if(hour_visible) {
    t = ((double) (clk->tm_hour)*60 + clk->tm_min)/2 + ((double)clk->tm_sec)/120;
    rotate(hour_img, hour_map, hour_center_x, hour_center_y, hour_rot_x, hour_rot_y, t);
  }
  
  if(min_visible) {
    t = (clk->tm_min*60 + (double)clk->tm_sec)/10;
    rotate(min_img, min_map, min_center_x, min_center_y, min_rot_x, min_rot_y, t);
  }
  
  if(sec_visible) {
    t = clk->tm_sec * 6;
    rotate(sec_img, sec_map, sec_center_x, sec_center_y, sec_rot_x, sec_rot_y, t);
  }
  
  gdk_draw_image(p, gc, clock_img, 0, 0, 0, 0, clock_img->width, clock_img->height);
  return;
}

void swatch_beats(asclock *my, GdkPixmap *p, GdkGC *gc, struct tm *clk, int beats)
{
  int pos1, pos2, pos3;

  pos1 = (beats/100) % 10;
  pos2 = (beats /10 ) % 10;
  pos3 = beats % 10;

   if (!my->itblinks || ((clk->tm_sec) % 2) )
    gdk_draw_pixmap(p, gc, beats_pixmap,
			10*beats_elem_width, 0,
			beats_at_x, beats_y,
			beats_at_width, beats_elem_height);
  else
    gdk_draw_pixmap(p, gc, clock_pixmap,
			beats_at_x, beats_y,
                        beats_at_x, beats_y,
                        beats_at_width, beats_elem_height);

  gdk_draw_pixmap(p, gc, beats_pixmap,
			pos1*beats_elem_width, 0,
			beats1_x, beats_y,
			beats_elem_width, beats_elem_height);

  gdk_draw_pixmap(p, gc, beats_pixmap,
			pos2*beats_elem_width, 0,
			beats2_x, beats_y,
			beats_elem_width, beats_elem_height);

  gdk_draw_pixmap(p, gc, beats_pixmap,
			pos3*beats_elem_width, 0,
			beats3_x, beats_y,
			beats_elem_width, beats_elem_height);

}

void Twelve(asclock *my, GdkPixmap *p, GdkGC *gc , struct tm *clk)
{
  int thishour;
  /* Stunde ohne am/pm */
  thishour = clk->tm_hour % 12;
  if (thishour == 0 )
    thishour = 12;

  if(led_visible) {
    if (clk->tm_hour >= 12)
      /* PM */
      gdk_draw_pixmap(p, gc, led_pixmap, 
		      13*led_elem_width, 0, 
		      led_ampm_x, led_ampm_y,
		      led_ampm_width, led_elem_height);
    else
      /* AM */
      gdk_draw_pixmap(p, gc, led_pixmap,
		      11*led_elem_width, 0, 
		      led_ampm_x, led_ampm_y,
		      led_ampm_width, led_elem_height);
    
    if (thishour>9)
      gdk_draw_pixmap(p, gc, led_pixmap,
		      led_elem_width, 0, 
		      led_12h_hour1_x, led_12h_y,
		      led_elem_width, led_elem_height);
    
    gdk_draw_pixmap(p, gc, led_pixmap,
		    led_elem_width * (thishour % 10), 0, 
		    led_12h_hour2_x, led_12h_y,
		    led_elem_width, led_elem_height);
    
    /* Minute, drawn first, so am/pm won't be overwritten */
    gdk_draw_pixmap(p, gc, led_pixmap,
		    led_elem_width * (clk->tm_min / 10),  0,  
		    led_12h_min1_x, led_12h_y,
		    led_elem_width, led_elem_height);
    
    gdk_draw_pixmap(p, gc, led_pixmap, 
		    led_elem_width * (clk->tm_min % 10), 0, 
		    led_12h_min2_x, led_12h_y,
		    led_elem_width, led_elem_height);
  }

  /* Date */
  if(day_visible) {
    if (clk->tm_mday>9)
      {
	gdk_draw_pixmap(p, gc, date_pixmap, 
			day_elem_width * (clk->tm_mday / 10), 0, 
			day1_x, day_y,
			day_elem_width, day_elem_height);
	
	gdk_draw_pixmap(p, gc, date_pixmap, 
			day_elem_width * (clk->tm_mday % 10), 0, 
			day2_x, day_y,
			day_elem_width, day_elem_height);
      }
    else
      gdk_draw_pixmap(p, gc, date_pixmap,
		      day_elem_width * (clk->tm_mday ), 0, 
		      day_x, day_y,
		      day_elem_width, day_elem_height);
  }

  /* weekday */
  if(week_visible) {
    gdk_draw_pixmap(p, gc, weekday_pixmap, 
		    0, week_elem_height * ((clk->tm_wday +6) % 7), 
		    week_x, week_y,
		    week_elem_width, week_elem_height);
  }

    /* month */
  if(month_visible) {
    gdk_draw_pixmap(p, gc, month_pixmap,
		    0, month_elem_height*(clk->tm_mon ), 
		    month_x, month_y,
		    month_elem_width, month_elem_height); 
  }
 
  /* blinking second colon */
  if(led_visible) {
    if (!my->itblinks || ((clk->tm_sec) % 2) ) 
      gdk_draw_pixmap(p, gc, led_pixmap, 
		      10*led_elem_width, 0,
		      led_12h_colon_x, led_12h_y,
		      (led_elem_width)/2, led_elem_height);
    else
      gdk_draw_pixmap(p, gc, clock_pixmap, 
		      led_12h_colon_x, led_12h_y,
		      led_12h_colon_x, led_12h_y,
		      (led_elem_width)/2, led_elem_height);
  }
}

void TwentyFour(asclock *my, GdkPixmap *p, GdkGC *gc , struct tm *clk)
{
  /* create the image to get data from and draw to */

  if(led_visible) {
    gdk_draw_pixmap(p, gc, led_pixmap,
		    led_elem_width * (clk->tm_hour / 10), 0, 
		    led_24h_hour1_x, led_24h_y,
		    led_elem_width, led_elem_height);
    
    gdk_draw_pixmap(p, gc, led_pixmap,
		    led_elem_width * (clk->tm_hour % 10), 0, 
		    led_24h_hour2_x, led_24h_y,
		    led_elem_width, led_elem_height);
    
    /* Minute */
    gdk_draw_pixmap(p, gc, led_pixmap, 
		    led_elem_width * (clk->tm_min / 10), 0, 
		    led_24h_min1_x, led_24h_y,
		    led_elem_width, led_elem_height);
    
    gdk_draw_pixmap(p, gc, led_pixmap, 
		    led_elem_width * (clk->tm_min % 10), 0, 
		    led_24h_min2_x, led_24h_y,
		    led_elem_width, led_elem_height);
  }

  /* Date */
  if(day_visible)
    {
      if (clk->tm_mday>9)
	{
	  gdk_draw_pixmap(p, gc, date_pixmap, 
			  day_elem_width * (clk->tm_mday / 10 ), 0, 
			  day1_x, day_y,
			  day_elem_width, day_elem_height); 
	  
	  gdk_draw_pixmap(p, gc, date_pixmap, 
			  day_elem_width * (clk->tm_mday % 10), 0, 
			  day2_x, day_y,
			  day_elem_width, day_elem_height);
	}
      else
	gdk_draw_pixmap(p, gc, date_pixmap,
			day_elem_width * clk->tm_mday, 0, 
			day_x, day_y,
			day_elem_width, day_elem_height);
    }
    
  /* weekday */
  if(week_visible) {
    gdk_draw_pixmap(p, gc, weekday_pixmap, 
		    0, week_elem_height * ((clk->tm_wday +6) % 7),
		    week_x, week_y,
		    week_elem_width, week_elem_height);
  }

  /* month */
  if(month_visible) {
    gdk_draw_pixmap(p, gc, month_pixmap,
		    0, month_elem_height*(clk->tm_mon ), 
		    month_x, month_y,
		    month_elem_width, month_elem_height);
  }

    /* blinking second colon */
  if(led_visible) {
    if (!my->itblinks || ((clk->tm_sec) % 2) ) 
      gdk_draw_pixmap(p, gc, led_pixmap, 
		      10*led_elem_width, 0,
		      led_24h_colon_x, led_24h_y,
		      (led_elem_width)/2, led_elem_height);
    else
      gdk_draw_pixmap(p, gc, clock_pixmap, 
		      led_24h_colon_x, led_24h_y,
		      led_24h_colon_x, led_24h_y,
		      (led_elem_width)/2, led_elem_height);
  }

}

