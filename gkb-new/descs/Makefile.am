preset_datadir = $(datadir)/gnome/gkb
preset_data_predata = \
	Armenian.keyprop.in Belgian.keyprop.in BulgarianCyril.keyprop.in \
	FrenchCanadian.keyprop.in FrenchCanadian2.keyprop.in FrenchSwiss.keyprop.in \
	GB-102.keyprop.in GB.keyprop.in GeorgianLatin.keyprop.in \
	GeorgianRussian.keyprop.in German.keyprop.in GermanSwiss.keyprop.in \
	Hungarian-lat1.keyprop.in Hungarian.keyprop.in Hungarian101-1.keyprop.in \
	Hungarian101.keyprop.in Norwegian.keyprop.in Polish.keyprop.in \
	PolishDeadkeys.keyprop.in Portugal.keyprop.in PortugalDeadkeys.keyprop.in \
	RussianCyril.keyprop.in Slovak.keyprop.in Slovene.keyprop.in Swedish.keyprop.in \
	TR-f.keyprop.in TR-q.keyprop.in Thai.keyprop.in Thai2.keyprop.in UK.keyprop.in \
	US-101.keyprop.in US-84.keyprop.in US-dec.keyprop.in US-ibm.keyprop.in \
	US-int.keyprop.in US-mac.keyprop.in US-sgi-101.keyprop.in US-sun.keyprop.in \
	US.keyprop.in Yugoslav.keyprop.in Hungarian-sun.keyprop.in

EXTRA_DIST = $(preset_data_DATA)

preset_data_DATA = $(preset_data_predata:.keyprop.in=.keyprop)

%.keyprop : %.keyprop.in $(top_srcdir)/xml-i18n-merge $(top_srcdir)/po/*.po
		$(top_srcdir)/xml-i18n-merge -d $(top_srcdir)/po $< $*.keyprop
