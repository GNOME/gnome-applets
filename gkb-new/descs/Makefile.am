%.keyprop: %.keyprop.desktop.in 
		$(top_builddir)/intltool-merge -d -p $(top_srcdir)/po $< $(srcdir)/$*.keyprop

preset_data_predata = \
		AM_Armenian.keyprop.desktop.in AR_Arabic.keyprop.desktop.in \
		AR_Arabic_x.keyprop.desktop.in Armenian.keyprop.desktop.in \
		AZ_Azerbaidjani_Turkic.keyprop.desktop.in Basque.keyprop.desktop.in \
		BE_Dutch.keyprop.desktop.in Belgian.keyprop.desktop.in \
		BG_Bulgarian.keyprop.desktop.in BG_Bulgarian_x.keyprop.desktop.in \
		BR_Portuguese.keyprop.desktop.in BR_Portuguese_x.keyprop.desktop.in \
		BulgarianCyril.keyprop.desktop.in BY_Belarussian.keyprop.desktop.in \
		CA_English.keyprop.desktop.in CH_German_x.keyprop.desktop.in \
		CZ_Czech.keyprop.desktop.in CZ_Czech_Slovak.keyprop.desktop.in \
		CZ_Czech_x.keyprop.desktop.in Default.keyprop.desktop.in \
		DE_German.keyprop.desktop.in DE_German_x.keyprop.desktop.in \
		DK_Danish.keyprop.desktop.in DK_Danish_x.keyprop.desktop.in \
		Dvorak.keyprop.desktop.in EE_Estonian.keyprop.desktop.in \
		EE_Estonian_x.keyprop.desktop.in ES_Spanish.keyprop.desktop.in \
		ES_Spanish-x86.keyprop.desktop.in ES_Spanish_x.keyprop.desktop.in \
		FI_Finnish.keyprop.desktop.in FI_Finnish_x.keyprop.desktop.in \
		FrenchCanadian2.keyprop.desktop.in FrenchCanadian.keyprop.desktop.in \
		FrenchSwiss.keyprop.desktop.in FR_French.keyprop.desktop.in \
		FR_French-x86.keyprop.desktop.in FR_French_x.keyprop.desktop.in \
		GE_Georgian_x.keyprop.desktop.in GeorgianLatin.keyprop.desktop.in \
		GeorgianRussian.keyprop.desktop.in German.keyprop.desktop.in \
		GermanSwiss.keyprop.desktop.in GR_Greek.keyprop.desktop.in \
		GR_Greek_x.keyprop.desktop.in HR_Croatian.keyprop.desktop.in \
		HU_Hungarian.keyprop.desktop.in Hungarian101-1.keyprop.desktop.in \
		Hungarian-101.keyprop.desktop.in Hungarian101.keyprop.desktop.in \
		Hungarian.keyprop.desktop.in Hungarian-lat1.keyprop.desktop.in \
		Hungarian-sun.keyprop.desktop.in Hungarian-x86.keyprop.desktop.in \
		IL_Hebrew.keyprop.desktop.in IL_Hebrew_x.keyprop.desktop.in \
		IS_Icelandic.keyprop.desktop.in IS_Icelandic_x.keyprop.desktop.in \
		IT_Italian.keyprop.desktop.in IT_Italian_x.keyprop.desktop.in \
		JP_Japanese.keyprop.desktop.in JP_Japanese_x.keyprop.desktop.in \
		KR_Korean.keyprop.desktop.in LA_Lao_x.keyprop.desktop.in \
		LT_Lithuanian.keyprop.desktop.in LT_Lithuanian_x.keyprop.desktop.in \
		LT_Prog.keyprop.desktop.in LT_Standard.keyprop.desktop.in \
		LV_Apostrophe.keyprop.desktop.in  LV_Standart.keyprop.desktop.in  LV_Tilde.keyprop.desktop.in \
		Macedonian.keyprop.desktop.in MK_Macedonian.keyprop.desktop.in \
		MN_Mongolian.keyprop.desktop.in MN_Mongolian_x.keyprop.desktop.in \
		MN_Mongolian_x_rev.keyprop.desktop.in MN_Mongolian_x_uni.keyprop.desktop.in \
		NL_Dutch_x.keyprop.desktop.in NO_Norwegian.keyprop.desktop.in \
		Norwegian.keyprop.desktop.in PL_Polish.keyprop.desktop.in \
		PolishDeadkeys.keyprop.desktop.in Polish.keyprop.desktop.in \
		PortugalDeadkeys.keyprop.desktop.in Portugal.keyprop.desktop.in \
		PT_Portuguese.keyprop.desktop.in PT_Portuguese_x.keyprop.desktop.in \
		RO_Romanian.keyprop.desktop.in RU_Russian.keyprop.desktop.in \
		RU_Russian_x.keyprop.desktop.in RU_Russian_x_rev.keyprop.desktop.in \
		RussianCyril.keyprop.desktop.in SE_Swedish.keyprop.desktop.in \
		SE_Swedish_x.keyprop.desktop.in SI_Slovenian.keyprop.desktop.in \
		SI_Slovenian_x.keyprop.desktop.in SK__x.keyprop.desktop.in \
		Slovak.keyprop.desktop.in \
		Slovene.keyprop.desktop.in Slovenian.keyprop.desktop.in \
		SR_Dutch.keyprop.desktop.in Swedish.keyprop.desktop.in \
		Syriac.keyprop.desktop.in Syriac_Phonetic.keyprop.desktop.in \
		Thai2.keyprop.desktop.in Thai.keyprop.desktop.in TH_Thai.keyprop.desktop.in \
		TH_Thai_x.keyprop.desktop.in TR-f.keyprop.desktop.in TR-q.keyprop.desktop.in \
		TR_Turkish.keyprop.desktop.in TR_Turkish_x.keyprop.desktop.in \
		UA_Ukrainian.keyprop.desktop.in UK-102.keyprop.desktop.in \
		UK_English.keyprop.desktop.in UK_English_x.keyprop.desktop.in UK.keyprop.desktop.in \
		UK_United_Kingdom.keyprop.desktop.in UK-x86.keyprop.desktop.in \
		US-101A-x86.keyprop.desktop.in US-101.keyprop.desktop.in US-84.keyprop.desktop.in \
		US-dec.keyprop.desktop.in US_English.keyprop.desktop.in US-ibm.keyprop.desktop.in \
		US_international.keyprop.desktop.in US-int.keyprop.desktop.in US.keyprop.desktop.in \
		US-mac.keyprop.desktop.in US-sgi-101.keyprop.desktop.in US-sun.keyprop.desktop.in \
		VN_Vietnamese.keyprop.desktop.in Yugoslav.keyprop.desktop.in \
		YU_Serbo-Croatian_x.keyprop.desktop.in YU_Serb_x.keyprop.desktop.in

presetdir = $(datadir)/gnome/gkb
preset_in_files = $(preset_data_predata)
preset_DATA = $(preset_in_files:.keyprop.desktop.in=.keyprop)
EXTRA_DIST = $(preset_DATA) $(xml_DATA) $(preset_in_files) $(xml_in_files)

xml_in_files = presets.xml.in
XMLS = $(xml_in_files:.xml.in=.xml)

xmldir = $(datadir)/gnome/gkb
xml_DATA = $(XMLS)

%.xml: %.xml.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*.po)
		$(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -x -u \
                -c $(top_srcdir)/po/.intltool-merge-cache

clean-local:
	rm -f *~ *.keyprop
