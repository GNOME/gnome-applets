/* GNOME sound-Monitor applet
 * (C) 2000 John Ellis
 *
 * Author: John Ellis
 *
 */

#ifndef ESDCALLS_H
#define ESDCALLS_H


void sound_get_volume(SoundData *sd, gint *l, gint *r);
void sound_get_buffer(SoundData *sd, short **buffer, gint *length);
SoundData *sound_init(const gchar *host, gint monitor_input);
void sound_free(SoundData *sd);

gint esd_control(ControlType function, const gchar *host);
StatusType esd_status(const gchar *host);


#endif
