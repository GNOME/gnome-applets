The gnome diskusage applet.

Takes the output of df, and diskplays it in the panel.

003 is latest version

Usage:

Untar into gnome-core/applets

you will get gnome-core/applets/diskusage

edit gnome-core/applets/Makefile.am, and include diskusage applet there
edit gnome-core/configure.in and include applets/diskusage there

run ./autogen.sh

You should have a Makefile in applets/diskusage now

run make there

run the applet, with ./diskusage_applets

(of course, you can also 'make install' it, if you like)

have i forgotten something?

mail comments to the gnome-list, whith "diskusage" in 
the subject, and cc them to me.

bwidmann@tks.fh-sbg.ac.at
