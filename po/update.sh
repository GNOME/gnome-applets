#!/bin/sh

xgettext --default-domain=gnome-applets --directory=.. \
  --add-comments --keyword=_ --keyword=N_ \
  --files-from=./POTFILES.in \
&& test ! -f gnome-applets.po \
   || ( rm -f ./gnome-applets.pot \
    && mv gnome-applets.po ./gnome-applets.pot )
