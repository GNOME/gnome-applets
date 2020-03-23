NULL =

app_icons = \
	stickynotes-stock-locked.png \
	stickynotes-stock-unlocked.png \
	stickynotes-stock-close.png \
	stickynotes-stock-resize-se.png \
	stickynotes-stock-resize-sw.png

EXTRA_DIST = \
	$(app_icons) \
	$(NULL)

uninstall-local:
	for icon in $(app_icons); do \
		rm -f $(DESTDIR)$(pkgdatadir)/icons/hicolor/48x48/apps/$$icon; \
	done

install-data-local:
	for icon in $(app_icons); do \
		mkdir -p $(DESTDIR)$(pkgdatadir)/icons/hicolor/48x48/apps/; \
		$(INSTALL_DATA) $(srcdir)/$$icon $(DESTDIR)$(pkgdatadir)/icons/hicolor/48x48/apps/$$icon; \
	done

-include $(top_srcdir)/git.mk
