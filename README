== WHAT IS THIS ===
This is a port of the initial window-picker-applet made by cannonical
to Gnome 3 and GTK+ 3. It give you a task list that only contains
the icons of each open window, but not the title. This saves a lot of
space, especially if you have many open windows. You can use the mouse
to reorder icons by drag and drop. Also on there are preferences to
set additional options, such as greying out inactive icons.

Currently this applet is not packaged anywhere, so if you want to use
it, clone this repository or download the archive and compile it on
your computer. The explanations below apply to most recent Ubuntu
version, but should work on any other Debian based distribution. If
you happen to be on Fedora or some other distribution, then the names
of the packages that you need might be different. For example Fedora's
packages often end with '-devel' instead of '-dev'.

=== HOW TO START HACKING ===
You have cloned the repository, so whats next:

1. Make sure you have the following packages installed or you will get 
   a warning like:
   ./autogen.sh: 2: autoreconf: not found
   or other warnings that are often not very helpful.
   
   On Ubuntu those would be:
    * automake
    * build-essential
    * libglib2.0-dev (for AM_GLIB_GNU_GETTEXT makro)
    * libtool (for AC_PROG_LIBTOOL makro)
    * intltool (otherwise you get a syntax error in .configure)
    * gettext (pulled by intltool automatically)
    * libgtk-3-dev
    * gsettings-desktop-schemas-dev
    * libwnck-3-dev
    * libpanel-applet-4-dev

3. When you have installed all above packages, run autogen.sh
   You must pass the --prefix and --libexecdir options or the gnome-panel will not find your applet
    $ ./autogen.sh --prefix=/usr --libexecdir=/usr/lib
    If autogen.sh says you are missing 'foo' then most of the time it means you are
    missing 'foo-dev' or 'libfoo-dev'.

3. If there are no other errors then run make and make install (no need to run configure,
   autogen.sh has done that already):
    $ make                 #compile the window-picker-applet into ./src/
    $ sudo make install    #the install it in /usr, (run with sudo!)

4. You now have the window-picker-applet installed

=== HOWTO START IT ===
ALT+RIGHT CLICK on the gnome-panel (find a spot which is not occupied by another widget) and choose
'Add to Panel...', select the Window Picker and click 'Add'.

I hope someone finds this as useful as I do.
Lanoxx

Thanks to Canonical for creating the initial version.

