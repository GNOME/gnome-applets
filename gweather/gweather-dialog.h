#ifndef __GWEATHER_DIALOG_H_
#define __GWEATHER_DIALOG_H_

/* $Id$ */

/*
 *  Papadimitriou Spiros <spapadim+@cs.cmu.edu>
 *
 *  This code released under the GNU GPL.
 *  Read the file COPYING for more information.
 *
 *  Main status dialog
 *
 */

G_BEGIN_DECLS

extern void gweather_dialog_create (GWeatherApplet *gw_applet);
extern void gweather_dialog_open (GWeatherApplet *gw_applet);
extern void gweather_dialog_close (GWeatherApplet *gw_applet);
extern void gweather_dialog_display_toggle (GWeatherApplet *gw_applet);
extern void gweather_dialog_update (GWeatherApplet *gw_applet);

G_END_DECLS

#endif /* __GWEATHER_DIALOG_H_ */

