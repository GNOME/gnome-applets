#include "asclock.h"

symbol config_symbols[] = {
  { "led_visible",      &led_visible},
  { "led_elem_width",   &led_elem_width},
  { "led_elem_height",  &led_elem_height},
  { "led_12h_hour1_x",  &led_12h_hour1_x},
  { "led_12h_hour2_x",  &led_12h_hour2_x},
  { "led_12h_colon_x",  &led_12h_colon_x},
  { "led_12h_min1_x",   &led_12h_min1_x},
  { "led_12h_min2_x",   &led_12h_min2_x},
  { "led_ampm_x",       &led_ampm_x},
  { "led_ampm_y",       &led_ampm_y},
  { "led_ampm_width",   &led_ampm_width},
  { "led_12h_y",        &led_12h_y},
  { "led_24h_hour1_x",  &led_24h_hour1_x},
  { "led_24h_hour2_x",  &led_24h_hour2_x},
  { "led_24h_colon_x",  &led_24h_colon_x},
  { "led_24h_min1_x",   &led_24h_min1_x},
  { "led_24h_min2_x",   &led_24h_min2_x},
  { "led_24h_y",        &led_24h_y},
  { "week_visible",     &week_visible},
  { "week_elem_width",  &week_elem_width},
  { "week_elem_height", &week_elem_height},
  { "week_x",           &week_x},
  { "week_y",           &week_y},
  { "day_visible",      &day_visible},
  { "day_elem_width",   &day_elem_width},
  { "day_elem_height",  &day_elem_height},
  { "day1_x",           &day1_x},
  { "day2_x",           &day2_x},
  { "day_y",            &day_y},
  { "month_visible",    &month_visible},
  { "month_elem_width", &month_elem_width},
  { "month_elem_height",&month_elem_height},
  { "month_x",          &month_x},
  { "month_y",          &month_y},
  { "analog_visible",   &analog_visible},
  { "hour_visible",     &hour_visible},
  { "hour_center_x",    &hour_center_x},
  { "hour_center_y",    &hour_center_y},
  { "hour_rot_x",       &hour_rot_x},
  { "hour_rot_y",       &hour_rot_y},
  { "min_visible",      &min_visible},
  { "min_center_x",     &min_center_x},
  { "min_center_y",     &min_center_y},
  { "min_rot_x",        &min_rot_x},
  { "min_rot_y",        &min_rot_y},
  { "sec_visible",      &sec_visible},
  { "sec_center_x",     &sec_center_x},
  { "sec_center_y",     &sec_center_y},
  { "sec_rot_x",        &sec_rot_x},
  { "sec_rot_y",        &sec_rot_y},
  { "beats_visible",    &beats_visible},
  { "beats_at_x",       &beats_at_x},
  { "beats_at_width",   &beats_at_width},
  { "beats1_x",         &beats1_x},
  { "beats2_x",         &beats2_x},
  { "beats3_x",         &beats3_x},
  { "beats_y",          &beats_y},
  { "beats_elem_width", &beats_elem_width},
  { "beats_elem_height",&beats_elem_height},
  { NULL, NULL} };

int itdocks;

int led_visible;
int led_elem_width;
int led_elem_height;
int led_12h_hour1_x;
int led_12h_hour2_x;
int led_12h_colon_x;
int led_12h_min1_x;
int led_12h_min2_x;
int led_ampm_x;
int led_ampm_y;
int led_ampm_width;
int led_12h_y;
int led_24h_hour1_x;
int led_24h_hour2_x;
int led_24h_colon_x;
int led_24h_min1_x;
int led_24h_min2_x;
int led_24h_y;

int week_visible;
int week_elem_width;
int week_elem_height;
int week_x;
int week_y;

int day_visible;
int day_elem_width;
int day_elem_height;
int day_x;
int day1_x;
int day2_x;
int day_y;

int month_visible;
int month_elem_width;
int month_elem_height;
int month_x;
int month_y;

int analog_visible;
int hour_visible ;
int hour_center_x ;
int hour_center_y ;
int hour_rot_x ;
int hour_rot_y ;

int min_visible ;
int min_center_x ;
int min_center_y ;
int min_rot_x ;
int min_rot_y ;

int sec_visible ;
int sec_center_x ;
int sec_center_y ;
int sec_rot_x ;
int sec_rot_y ;

int beats_visible;
int beats_at_x;
int beats_at_width;
int beats1_x;
int beats2_x;
int beats3_x;
int beats_y;
int beats_elem_width;
int beats_elem_height;

void init_symbols(void) 
{

 led_visible=0;
 led_elem_width=UNDEFINED;
 led_elem_height=UNDEFINED;
 led_12h_hour1_x=UNDEFINED;
 led_12h_hour2_x=UNDEFINED;
 led_12h_colon_x=UNDEFINED;
 led_12h_min1_x=UNDEFINED;
 led_12h_min2_x=UNDEFINED;
 led_ampm_x=UNDEFINED;
 led_ampm_y=UNDEFINED;
 led_ampm_width=UNDEFINED;
 led_12h_y=UNDEFINED;
 led_24h_hour1_x=UNDEFINED;
 led_24h_hour2_x=UNDEFINED;
 led_24h_colon_x=UNDEFINED;
 led_24h_min1_x=UNDEFINED;
 led_24h_min2_x=UNDEFINED;
 led_24h_y=UNDEFINED;

 week_visible=0;
 week_elem_width=UNDEFINED;
 week_elem_height=UNDEFINED;
 week_x=UNDEFINED;
 week_y=UNDEFINED;

 day_visible=0;
 day_elem_width=UNDEFINED;
 day_elem_height=UNDEFINED;
 day_x=UNDEFINED;
 day1_x=UNDEFINED;
 day2_x=UNDEFINED;
 day_y=UNDEFINED;

 month_visible=0;
 month_elem_width=UNDEFINED;
 month_elem_height=UNDEFINED;
 month_x=UNDEFINED;
 month_y=UNDEFINED;

 analog_visible=0;
 hour_visible = 0;
 hour_center_x =UNDEFINED;
 hour_center_y =UNDEFINED;
 hour_rot_x =UNDEFINED;
 hour_rot_y =UNDEFINED;

 min_visible = 0;
 min_center_x =UNDEFINED;
 min_center_y =UNDEFINED;
 min_rot_x =UNDEFINED;
 min_rot_y =UNDEFINED;

 sec_visible = 0;
 sec_center_x =UNDEFINED;
 sec_center_y =UNDEFINED;
 sec_rot_x =UNDEFINED;
 sec_rot_y =UNDEFINED;

 beats_visible = 0;
 beats_at_x=UNDEFINED;
 beats_at_width=UNDEFINED;
 beats1_x=UNDEFINED;
 beats2_x=UNDEFINED;
 beats3_x=UNDEFINED;
 beats_y=UNDEFINED;
 beats_elem_width=UNDEFINED;
 beats_elem_height=UNDEFINED;

}

#define PROPAGATE(from,to,inc) if((to == UNDEFINED) && (from != UNDEFINED)) { to = from + inc; }

void postconfig(void)
{

    if(analog_visible==VISIBLE) {
      PROPAGATE(hour_center_x, min_center_x,0)
      PROPAGATE(min_center_x, sec_center_x,0)
      PROPAGATE(sec_center_x, hour_center_x,0)
      PROPAGATE(hour_center_y, min_center_y,0)
      PROPAGATE(min_center_y, sec_center_y,0)
      PROPAGATE(sec_center_y, hour_center_y,0)
    }

    if(led_visible==VISIBLE) {
      PROPAGATE(led_12h_hour1_x, led_24h_hour1_x, 0)
      PROPAGATE(led_24h_hour1_x, led_12h_hour1_x, 0)

      PROPAGATE(led_12h_hour1_x, led_12h_hour2_x, led_elem_width) 
      PROPAGATE(led_12h_hour2_x, led_12h_colon_x, led_elem_width)
      PROPAGATE(led_12h_colon_x, led_12h_min1_x, (led_elem_width+1)/2)
      PROPAGATE(led_12h_min1_x,  led_12h_min2_x,  led_elem_width)
      PROPAGATE(led_12h_min2_x,  led_ampm_x,      led_elem_width)
      PROPAGATE(led_12h_y, led_24h_y, 0)

      PROPAGATE(led_24h_hour1_x, led_24h_hour2_x, led_elem_width) 
      PROPAGATE(led_24h_hour2_x, led_24h_colon_x, led_elem_width)
      PROPAGATE(led_24h_colon_x, led_24h_min1_x, (led_elem_width+1)/2)
      PROPAGATE(led_24h_min1_x,  led_24h_min2_x,  led_elem_width)
      PROPAGATE(led_24h_y, led_12h_y, 0)

      PROPAGATE(led_12h_y, led_ampm_y, 0)
    }

    if(day_visible==VISIBLE) {
      PROPAGATE(day1_x, day2_x, day_elem_width)
      day_x = (day1_x + day2_x)/2;
    }

    if(beats_visible==VISIBLE) {
      PROPAGATE(beats_elem_width, beats_at_width, beats_elem_width)
    
      PROPAGATE(beats_at_x, beats1_x, beats_at_width)
      PROPAGATE(beats1_x, beats2_x, beats_elem_width)
      PROPAGATE(beats2_x, beats3_x, beats_elem_width)
    }
}

