%.keyprop: %.keyprop.in 
		$(top_srcdir)/intltool-merge -d -p $(top_srcdir)/po $< $(srcdir)/$*.keyprop

preset_data_predata = \
		AM_Armenian.keyprop.in AR_Arabic.keyprop.in \
		AR_Arabic_x.keyprop.in Armenian.keyprop.in \
		AZ_Azerbaidjani_Turkic.keyprop.in Basque.keyprop.in \
		BE_Dutch.keyprop.in Belgian.keyprop.in \
		BG_Bulgarian.keyprop.in BG_Bulgarian_x.keyprop.in \
		BR_Portuguese.keyprop.in BR_Portuguese_x.keyprop.in \
		BulgarianCyril.keyprop.in BY_Belarussian.keyprop.in \
		CA_English.keyprop.in CH_German_x.keyprop.in \
		CZ_Czech.keyprop.in CZ_Czech_Slovak.keyprop.in \
		CZ_Czech_x.keyprop.in Default.keyprop.in \
		DE_German.keyprop.in DE_German_x.keyprop.in \
		DK_Danish.keyprop.in DK_Danish_x.keyprop.in \
		Dvorak.keyprop.in EE_Estonian.keyprop.in \
		EE_Estonian_x.keyprop.in ES_Spanish.keyprop.in \
		ES_Spanish-x86.keyprop.in ES_Spanish_x.keyprop.in \
		FI_Finnish.keyprop.in FI_Finnish_x.keyprop.in \
		FrenchCanadian2.keyprop.in FrenchCanadian.keyprop.in \
		FrenchSwiss.keyprop.in FR_French.keyprop.in \
		FR_French-x86.keyprop.in FR_French_x.keyprop.in \
		GE_Georgian_x.keyprop.in GeorgianLatin.keyprop.in \
		GeorgianRussian.keyprop.in German.keyprop.in \
		GermanSwiss.keyprop.in GR_Greek.keyprop.in \
		GR_Greek_x.keyprop.in HR_Croatian.keyprop.in \
		HU_Hungarian.keyprop.in Hungarian101-1.keyprop.in \
		Hungarian-101.keyprop.in Hungarian101.keyprop.in \
		Hungarian.keyprop.in Hungarian-lat1.keyprop.in \
		Hungarian-sun.keyprop.in Hungarian-x86.keyprop.in \
		IL_Hebrew.keyprop.in IL_Hebrew_x.keyprop.in \
		IS_Icelandic.keyprop.in IS_Icelandic_x.keyprop.in \
		IT_Italian.keyprop.in IT_Italian_x.keyprop.in \
		JP_Japanese.keyprop.in JP_Japanese_x.keyprop.in \
		KR_Korean.keyprop.in LA_Lao_x.keyprop.in \
		LT_Lithuanian.keyprop.in LT_Lithuanian_x.keyprop.in \
		LT_Prog.keyprop.in LT_Standard.keyprop.in \
		Macedonian.keyprop.in MK_Macedonian.keyprop.in \
		MN_Mongolian.keyprop.in NL_Dutch_x.keyprop.in \
		NO_Norwegian.keyprop.in Norwegian.keyprop.in \
		PL_Polish.keyprop.in PolishDeadkeys.keyprop.in \
		Polish.keyprop.in PortugalDeadkeys.keyprop.in \
		Portugal.keyprop.in PT_Portuguese.keyprop.in \
		PT_Portuguese_x.keyprop.in RO_Romanian.keyprop.in \
		RU_Russian.keyprop.in RU_Russian_x.keyprop.in \
		RU_Russian_x_rev.keyprop.in RussianCyril.keyprop.in \
		SE_Swedish.keyprop.in SE_Swedish_x.keyprop.in \
		SI_Slovenian.keyprop.in SI_Slovenian_x.keyprop.in \
		SK__x.keyprop.in Slovak.keyprop.in Slovene.keyprop.in \
		Slovenian.keyprop.in SR_Dutch.keyprop.in Swedish.keyprop.in \
		Thai2.keyprop.in Thai.keyprop.in TH_Thai.keyprop.in \
		TH_Thai_x.keyprop.in TR-f.keyprop.in TR-q.keyprop.in \
		TR_Turkish.keyprop.in TR_Turkish_x.keyprop.in \
		UA_Ukrainian.keyprop.in UK-102.keyprop.in \
		UK_English.keyprop.in UK_English_x.keyprop.in UK.keyprop.in \
		UK_United_Kingdom.keyprop.in UK-x86.keyprop.in \
		US-101A-x86.keyprop.in US-101.keyprop.in US-84.keyprop.in \
		US-dec.keyprop.in US_English.keyprop.in US-ibm.keyprop.in \
		US_international.keyprop.in US-int.keyprop.in US.keyprop.in \
		US-mac.keyprop.in US-sgi-101.keyprop.in US-sun.keyprop.in \
		VN_Vietnamese.keyprop.in Yugoslav.keyprop.in \
		YU_Serbo-Croatian_x.keyprop.in YU_Serb_x.keyprop.in

presetdir = $(datadir)/gnome/gkb
preset_in_files = $(preset_data_predata)
preset_DATA = $(preset_in_files:.keyprop.in=.keyprop)
EXTRA_DIST = $(preset_DATA) $(xml_DATA)

xml_in_files = presets.xml.in
XMLS = $(xml_in_files:.xml.in=.xml)

xmldir = $(datadir)/gnome/gkb
xml_DATA = $(XMLS)

%.xml: %.xml.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*.po)
		$(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -x -u \
                -c $(top_srcdir)/po/.intltool-merge-cache

clean-local:
	rm -f *~ *.keyprop
