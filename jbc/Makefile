# Makefile for jbc
#
# Jon Anhold (jon@snoopy.net)
#
#
# gcc -ojbc_applet jbc-applet.c `gnome-config --libs gtk gnome gnomeui applets` `gnome-config --cflags gtk gnome gnomeui applets`


CC = gcc
CFLAGS = -Wall `gnome-config --cflags gtk gnome gnomeui applets`
LIBS = `gnome-config --libs gtk gnome gnomeui applets`

all: jbc_applet

jbc_applet: jbc-applet.c
	gcc $(CFLAGS) $(LIBS) -ojbc_applet jbc-applet.c

clean:
	rm -f jbc_applet
	rm -f *~
