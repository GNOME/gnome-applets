#ifndef __LOCATION_H__
#define __LOCATION_H__


void show_locations (GWeatherApplet *applet);
GtkWidget *create_countries_widget (GWeatherApplet *applet);
GtkWidget *create_cities_widget (GWeatherApplet *applet);
void fetch_countries (GWeatherApplet *applet);

#endif

