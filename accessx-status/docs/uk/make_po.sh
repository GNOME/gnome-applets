#!/bin/bash

xml2po -o acc_uk.po -r ../../../accessx-status/docs/uk/accessx-status.xml ../../../accessx-status/docs/C/accessx-status.xml
xml2po -o bat_uk.po -r ../../../battstat/docs/uk/battstat.xml ../../../battstat/docs/C/battstat.xml
xml2po -o cdp_uk.po -r ../../../cdplayer/docs/uk/cdplayer.xml ../../../cdplayer/docs/C/cdplayer.xml
xml2po -o chr_uk.po -r ../../../charpick/help/uk/char-palette.xml ../../../charpick/help/C/char-palette.xml
xml2po -o drv_uk.po -r ../../../drivemount/help/uk/drivemount.xml ../../../drivemount/help/C/drivemount.xml
xml2po -o eye_uk.po -r ../../../geyes/docs/uk/geyes.xml ../../../geyes/docs/C/geyes.xml
xml2po -o gkn_uk.po -r ../../../gkb-new/help/uk/gkb.xml ../../../gkb-new/help/C/gkb.xml
xml2po -o gti_uk.po -r ../../../gtik/help/uk/gtik2_applet2.xml ../../../gtik/help/C/gtik2_applet2.xml
xml2po -o gwe_uk.po -r ../../../gweather/docs/uk/gweather.xml ../../../gweather/docs/C/gweather.xml
xml2po -o mai_uk.po -r ../../../mailcheck/help/uk/mailcheck.xml ../../../mailcheck/help/C/mailcheck.xml
xml2po -o mcm_uk.po -r ../../../mini-commander/help/uk/command-line.xml ../../../mini-commander/help/C/command-line.xml
xml2po -o mix_uk.po -r ../../../mixer/docs/uk/mixer_applet2.xml ../../../mixer/docs/C/mixer_applet2.xml
xml2po -o mod_uk.po -r ../../../modemlights/docs/uk/modemlights.xml ../../../modemlights/docs/C/modemlights.xml
#multiload has no uk translation - skipped
xml2po -o sti_uk.po -r ../../../stickynotes/docs/uk/stickynotes_applet.xml ../../../stickynotes/docs/C/stickynotes_applet.xml
xml2po -o wir_uk.po -r ../../../wireless/docs/uk/wireless.xml ../../../wireless/docs/C/wireless.xml

cat header ???_uk.po >uk_.po
msguniq -o uk.po uk_.po
