#include "asclock.h"

static GdkImage *act_img;

static int
match_color (int red, int green, int blue)
{
  GdkColor *colors;
  guint sum, max;
  gint rdiff, gdiff, bdiff;
  gint i, index;

  colors = cmap->colors;
  max = 3 * (65536);
  index = -1;

  for (i = 0; i < cmap->size; i++)
    {
      rdiff = (red - colors[i].red);
      gdiff = (green - colors[i].green);
      bdiff = (blue - colors[i].blue);
      
      sum = ABS (rdiff) + ABS (gdiff) + ABS (bdiff);
      
      if (sum < max)
	{
	  index = i;
	  max = sum;
	}
    }

  return index;
}



static void get_pixel(int x, int y, int *r, int *g, int *b)
{
  guint32 pixel;

  pixel = gdk_image_get_pixel(act_img, x, y);

  if(visual_depth>=15) {
    *r = (pixel & visual->red_mask) >> visual->red_shift;
    *g = (pixel & visual->green_mask) >> visual->green_shift;
    *b = (pixel & visual->blue_mask) >> visual->blue_shift;
  }
  else {
    *r = cmap->colors[pixel].red / 256;
    *g = cmap->colors[pixel].green / 256;
    *b = cmap->colors[pixel].blue / 256;
  }
  
    
}

static void set_pixel(int x, int y, double weight, int r, int g, int b)
{
  guint32 old, pixel;
  int or, og, ob;
  if(weight<0.99) 
    {
      old = gdk_image_get_pixel(clock_img, x, y);

      if(visual_depth>=15) 
	{
	  or = (old & visual->red_mask) >> visual->red_shift;
	  og = (old & visual->green_mask) >> visual->green_shift;
	  ob = (old & visual->blue_mask) >> visual->blue_shift;
	}
      else 
	{
	  or = cmap->colors[old].red / 256;
	  og = cmap->colors[old].green / 256;
	  ob = cmap->colors[old].blue / 256;
	}
       	  
      r = (int) or * (1-weight) + r*weight;
      if(r>0xff) r=0xff;
      
      g = (int) og * (1-weight) + g*weight;
      if(g>0xff) g=0xff;
      
      b = (int) ob * (1-weight) + b*weight;
      if(b>0xff) b=0xff;
    }

  if(visual_depth>=15)
    pixel = (r << visual->red_shift) + (g << visual->green_shift) + (b << visual->blue_shift);
  else
    pixel = match_color(r*256, g*256, b*256);
  gdk_image_put_pixel(clock_img, x, y, pixel);

}

void rotate(GdkImage *img, char *map, int center_x, int center_y, int rot_x, int rot_y, double alpha)
{
  int left, right, bottom, top;
  int done, miss, missed=0;
  int goup;
  int goleft;

  int cols, rows;
  int x, y;
  double xoff, yoff;
  int rad;

  act_img = img;

  alpha = - alpha *3.14159/180.0;
  
  cols = img->width;
  rows = img->height;

  rad = (int) ceil(sqrt(2)* ((cols>rows) ? cols : rows));

  xoff = center_x;
  yoff = center_y;

  left = center_x - rad;
  if(left < 0) left = 0;
  right = center_x + rad;
  if(right>clock_img->width) right = clock_img->width;
  bottom = center_y - rad;
  if(bottom<0) bottom = 0;
  top = center_y + rad;
  if(top>clock_img->height) top = clock_img->height;

  done = FALSE;
  miss=FALSE;

  goup=TRUE;
  goleft=TRUE;
  x = center_x;
  y = center_y;
  while(!done) 
    {
      double ex, ey;
      double weight;
      int oldx, oldy;
      
      /* transform into old coordinate system */
      ex = rot_x-0.25 + ( (x-xoff) * cos(alpha) - (y-yoff) * sin(alpha) );
      ey = rot_y-0.25 + ( (x-xoff) * sin(alpha) + (y-yoff) * cos(alpha)  );
      oldx = floor(ex);
      oldy = floor(ey);
      
      if( (oldx>=-1) && (oldx<cols) && (oldy>=-1) && (oldy<rows) )
	{
	  float tr, tg, tb;
	  float af, bf, cf, df;
	  
	  /* calculate the fractile part of the according pixel */
	  df = (ex-oldx)*(ey-oldy);
	  cf = (oldx+1-ex)*(ey-oldy);
	  bf = (ex-oldx)*(oldy+1-ey);
	  af = (oldx+1-ex)*(oldy+1-ey);
	  
	  tr = 0.0; tg = 0.0; tb = 0.0;
	  weight = 0.0;
	  if( (oldx>=0) && (oldy>=0))
	    if(map[oldy*cols+oldx] ) {
	      int srcr, srcg, srcb;
	      get_pixel(oldx, oldy, &srcr, &srcg, &srcb);
	      if(!((srcr==255) && (srcg==0) && (srcb==255)))
		{
		  tr += af * srcr;
		  tg += af * srcg;
		  tb += af * srcb;
		  weight += af;
		}
	    }
	  if(  (oldx+1 < cols) && (oldy>=0))
	    if( map[oldy*cols+oldx+1] ) {
	      int srcr, srcg, srcb;
	      get_pixel(oldx+1, oldy, &srcr, &srcg, &srcb);
	      if(!((srcr==255) && (srcg==0) && (srcb==255)) )
		{
		  tr += bf * srcr;
		  tg += bf * srcg;
		  tb += bf * srcb;
		  weight += bf;
		}
	    }
	  if( (oldy+1 < rows) && (oldx >=0))
	    if( map[(oldy+1)*cols+oldx] ) {
	      int srcr, srcg, srcb;
	      get_pixel(oldx, oldy+1, &srcr, &srcg, &srcb);
	      
	      if(!((srcr==255) && (srcg==0) && (srcb==255)) )
		{
		  tr += cf * srcr;
		  tg += cf * srcg;
		  tb += cf * srcb;
		  weight += cf;
		}
	    }
	  if( (oldx+1 < cols) && (oldy+1 < rows) )
	    if( map[(oldy+1)*cols+oldx+1] ) {
	      int srcr, srcg, srcb;
	      get_pixel(oldx+1, oldy+1, &srcr, &srcg, &srcb);
	      
	      if(!((srcr==255) && (srcg==0) && (srcb==255)) )
		{
		  tr += df * srcr;
		  tg += df * srcg;
		  tb += df * srcb;
		  weight += df;
		}
	    }
          
	  if((x>=0) && (y>=0) && (x<clock_img->width) && (y<clock_img->height) )
	    set_pixel(x, y, weight, (int)tr, (int)tg, (int)tb);
	  missed = 0;
	  miss = FALSE;
	}
      else
	{
	  miss = TRUE;
	  missed++;
	  if(missed==15)
	    {
	      if(goup) {
		goup=FALSE;
		goleft=FALSE;
		x = center_x;
		y = center_y;
	      }
	      else
		done = TRUE;
	    }
	}

      if(miss && (missed==1)) {
	goleft = !goleft;
	if(goup)
	  y--;
	else
	  y++;

      }
      else {
	if(goleft)
	  x--;
	else
	  x++;
      }	
    }
}






