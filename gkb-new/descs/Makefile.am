preset_data_predata = AM_Armenian.keyprop.in \
		AZ_Azerbaidjani_Turkic.keyprop.in Armenian.keyprop.in \
		BE_Dutch.keyprop.in BG_Bulgarian.keyprop.in \
		BG_Bulgarian_x.keyprop.in BR_Portuguese.keyprop.in \
		BR_Portuguese_x.keyprop.in BY_Belarussian.keyprop.in \
		Belgian.keyprop.in BulgarianCyril.keyprop.in \
		CA_English.keyprop.in CH_German_x.keyprop.in \
		CS_Czech.keyprop.in CZ_Czech_Slovak.keyprop.in \
		CZ_Czech_x.keyprop.in DE_German.keyprop.in \
		DE_German_x.keyprop.in DK_Danish.keyprop.in \
		DK_Danish_x.keyprop.in EE_Estonian.keyprop.in \
		EE_Estonian_x.keyprop.in ES_Spanish.keyprop.in \
		ES_Spanish_x.keyprop.in FI_Finnish.keyprop.in \
		FI_Finnish_x.keyprop.in FR_French.keyprop.in \
		FR_French_x.keyprop.in FrenchCanadian.keyprop.in \
		FrenchCanadian2.keyprop.in FrenchSwiss.keyprop.in \
		GB-102.keyprop.in GB.keyprop.in GB_English.keyprop.in \
		GB_English_x.keyprop.in GE_Georgian_x.keyprop.in \
		GR_Greek.keyprop.in GR_Greek_x.keyprop.in \
		GeorgianLatin.keyprop.in GeorgianRussian.keyprop.in \
		German.keyprop.in GermanSwiss.keyprop.in \
		HU_Hungarian.keyprop.in Hungarian-101.keyprop.in \
		Hungarian-lat1.keyprop.in Hungarian-sun.keyprop.in \
		Hungarian.keyprop.in Hungarian101-1.keyprop.in \
		Hungarian101.keyprop.in IL_Hebrew.keyprop.in \
		IL_Hebrew_x.keyprop.in IS_Icelandic.keyprop.in \
		IS_Icelandic_x.keyprop.in IT_Italian.keyprop.in \
		IT_Italian_x.keyprop.in JP_Japanese.keyprop.in \
		JP_Japanese_x.keyprop.in LA_Lao_x.keyprop.in \
		LT_Lithuanian.keyprop.in LT_Lithuanian_x.keyprop.in \
		LT_Prog.keyprop.in LT_Standard.keyprop.in \
		MK_Macedonian.keyprop.in NL_Dutch_x.keyprop.in \
		NO_Norwegian.keyprop.in Norwegian.keyprop.in \
		PL_Polish.keyprop.in PT_Portuguese.keyprop.in \
		PT_Portuguese_x.keyprop.in Polish.keyprop.in \
		PolishDeadkeys.keyprop.in Portugal.keyprop.in \
		PortugalDeadkeys.keyprop.in RO_Romanian.keyprop.in \
		RU_Russian.keyprop.in RU_Russian_x.keyprop.in \
		RussianCyril.keyprop.in SE_Swedish.keyprop.in \
		SI_Slovenian.keyprop.in SR_Dutch.keyprop.in \
		Slovak.keyprop.in Slovene.keyprop.in Swedish.keyprop.in \
		TH_Thai.keyprop.in TH_Thai_x.keyprop.in TR-f.keyprop.in \
		TR-q.keyprop.in TR_Turkish.keyprop.in \
		TR_Turkish_x.keyprop.in Thai.keyprop.in Thai2.keyprop.in \
		UA_Ukrainian.keyprop.in UK.keyprop.in US-101.keyprop.in \
		US-84.keyprop.in US-dec.keyprop.in US-ibm.keyprop.in \
		US-int.keyprop.in US-mac.keyprop.in US-sgi-101.keyprop.in \
		US-sun.keyprop.in US.keyprop.in US_English.keyprop.in \
		US_international.keyprop.in VN_Vietnamese.keyprop.in \
		YU_Serbo-Croatian_x.keyprop.in Yugoslav.keyprop.in \
		Macedonian.keyprop.in Basque.keyprop.in

preset_datadir = $(datadir)/gnome/gkb
preset_data_DATA = $(preset_data_predata:.keyprop.in=.keyprop)
EXTRA_DIST = $(preset_data_DATA)

%.keyprop: %.keyprop.in $(top_srcdir)/xml-i18n-merge $(top_srcdir)/po/*.po
		$(top_srcdir)/xml-i18n-merge -d $(top_srcdir)/po $< $(srcdir)/$*.keyprop
