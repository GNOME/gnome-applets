/* GNOME sound-monitor applet
 * (C) 2000 John Ellis
 *
 * Author: John Ellis
 *
 */

#ifndef SCOPE_H
#define SCOPE_H


ScopeData *scope_new_from_data(gchar **data, gint x, gint y, gint horizontal);
ScopeData *scope_new_from_file(gchar *file, gint x, gint y, gint horizontal);
void scope_free(ScopeData *scope);
void scope_scale_set(ScopeData *scope, gint scale);
void scope_type_set(ScopeData *scope, ScopeType type);
void scope_draw(ScopeData *scope, GdkPixbuf *pb, gint flat, short *buffer, gint buffer_length);
void scope_back_set(ScopeData *scope, GdkPixbuf *pb);


#endif

