%.keyprop: %.keyprop.in 
		$(top_srcdir)/intltool-merge -d -p $(top_srcdir)/po $< $(srcdir)/$*.keyprop

preset_data_predata = \
		Sun_AM_Armenian.keyprop.in Sun_AZ_Azerbaidjani_Turkic.keyprop.in \
		Sun_BE_Dutch.keyprop.in Sun_BG_Bulgarian_x.keyprop.in \
		Sun_BR_Portuguese_x.keyprop.in Sun_BY_Belarussian.keyprop.in \
		Sun_CA_English.keyprop.in Sun_CH_German_x.keyprop.in \
		Sun_CZ_Czech_x.keyprop.in Sun_DE_German_x.keyprop.in \
		Sun_DK_Danish_x.keyprop.in Sun_EE_Estonian_x.keyprop.in \
		Sun_ES_Spanish_x.keyprop.in Sun_FI_Finnish_x.keyprop.in \
		Sun_FR_French_x.keyprop.in Sun_GB_English_x.keyprop.in \
		Sun_HU_Hungarian-lat1.keyprop.in Sun_HU_Hungarian.keyprop.in \
		Sun_IL_Hebrew_x.keyprop.in Sun_IS_Icelandic_x.keyprop.in \
		Sun_IT_Italian_x.keyprop.in Sun_JP_Japanese_x.keyprop.in \
		Sun_LT_Lithuanian_x.keyprop.in Sun_LV_Latvian_x.keyprop.in \
		Sun_MK_Macedonian.keyprop.in Sun_MN_Mongolian.keyprop.in \
		Sun_NO_Norwegian.keyprop.in Sun_PL_Polish.keyprop.in \
		Sun_PT_Portuguese_x.keyprop.in Sun_RO_Romanian.keyprop.in \
		Sun_RU_Russian_x.keyprop.in Sun_SE_Swedish_x.keyprop.in \
		Sun_SI_Slovenian.keyprop.in Sun_SI_Slovenian_x.keyprop.in \
		Sun_SK_Slovak.keyprop.in Sun_SR_Dutch.keyprop.in \
		Sun_TH_Thai_x.keyprop.in Sun_TR_Turkish_x.keyprop.in \
		Sun_UA_Ukrainian.keyprop.in Sun_US.keyprop.in \
		Sun_VN_Vietnamese.keyprop.in Sun_BR_Portuguese_USB_x.keyprop.in \
		Sun_DE_German_USB_x.keyprop.in Sun_DK_Danish_USB_x.keyprop.in \
		Sun_ES_Spanish_USB_x.keyprop.in Sun_FR_French_USB_x.keyprop.in \
		Sun_GB_English_USB_x.keyprop.in Sun_IT_Italian_USB_x.keyprop.in \
		Sun_SE_Swedish_USB_x.keyprop.in Sun_US_USB.keyprop.in Default.keyprop.in \
		Sun_DE_German_type4_x.keyprop.in Sun_DK_Danish_type4_x.keyprop.in \
		Sun_ES_Spanish_type4_x.keyprop.in Sun_GB_English_type4_x.keyprop.in \
		Sun_IT_Italian_type4_x.keyprop.in Sun_JP_Japanese_type4_x.keyprop.in \
		Sun_PT_Portuguese_type4_x.keyprop.in Sun_SE_Swedish_type4_x.keyprop.in \
		Sun_US_type4.keyprop.in Sun_US_type6.keyprop.in


presetdir = $(datadir)/gnome/gkb
preset_in_files = $(preset_data_predata)
preset_DATA = $(preset_in_files:.keyprop.in=.keyprop)
EXTRA_DIST = $(preset_DATA)
