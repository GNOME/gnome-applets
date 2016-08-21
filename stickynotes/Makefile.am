NULL =

SUBDIRS = data help pixmaps src

ui_files = stickynotes-applet-menu.xml \
	stickynotes-note-menu.xml
builder_files = stickynotes-note.ui \
	stickynotes-delete.ui \
	stickynotes-delete-all.ui \
	stickynotes-preferences.ui \
	stickynotes-properties.ui

if BUILD_STICKYNOTES_APPLET

builderdir = $(pkgdatadir)/builder
builder_DATA = $(builder_files)

uidir = $(pkgdatadir)/ui
ui_DATA = $(ui_files)

endif

EXTRA_DIST = \
	$(builder_files) \
	$(ui_files) \
	$(NULL)

-include $(top_srcdir)/git.mk
