#include <applet-widget.h>

#ifndef _PROPERTIES_H
#define _PROPERTIES_H

void battery_properties_window(AppletWidget * applet, gpointer data);
void col_value_changed_cb( GtkObject * ignored, guint arg1, guint arg2,
			   guint arg3, guint arg4, gpointer data );
void toggle_value_changed_cb( GtkToggleButton * ignored, gpointer data );
void adj_value_changed_cb( GtkAdjustment * ignored, gpointer data );

#endif /* _PROPERTIES_H */

