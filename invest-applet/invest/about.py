from os.path import join
from gettext import gettext as _
from invest.defs import VERSION
import invest
import gtk, gtk.gdk, gnomevfs, gobject


def on_email(about, mail):
	gnomevfs.url_show("mailto:%s" % mail)

def on_url(about, link):
	gnomevfs.url_show(link)

gtk.about_dialog_set_email_hook(on_email)
gtk.about_dialog_set_url_hook(on_url)

invest_logo = None
try:
	invest_logo = gtk.gdk.pixbuf_new_from_file_at_size(join(invest.ART_DATA_DIR, "invest.svg"), 96,96)
except Exception, msg:
	pass
	
def show_about():
	about = gtk.AboutDialog()
	infos = {
		"name" : _("Invest"),
		"logo" : invest_logo,
		"version" : VERSION,
		"comments" : _("Track your invested money."),
		"copyright" : "Copyright © 2004-2005 Raphael Slinckx.",
		"website" : "http://raphael.slinckx.net/invest",
		"website-label" : _("Invest Website"),
	}

	about.set_authors(["Raphael Slinckx <raphael@slinckx.net>"])
#	about.set_artists([])
#	about.set_documenters([])
	
	#translators: These appear in the About dialog, usual format applies.
	about.set_translator_credits( _("translator-credits") )
	
	for prop, val in infos.items():
		about.set_property(prop, val)
	
	about.connect ("response", lambda self, *args: self.destroy ())
	about.show_all()
