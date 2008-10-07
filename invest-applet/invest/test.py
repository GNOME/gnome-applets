#!/usr/bin/env python

import unittest

import quotes
import invest

print """=====================================================================
Please note that these tests will fail if you do not have any stocks
already set up in the applet. This is an unfortunate state of affairs
that we hope to have fixed in the near future. For now, please run
the applet and enter at least one stock to be monitored. The default
is just fine.
====================================================================="""

def null_function (*args):
    pass

class TestQuotes (unittest.TestCase):
    def testQuoteUpdater_populate (self):
        qu = quotes.QuoteUpdater (null_function, null_function)
        # We require a pre-created invest.STOCKS for the moment (since
        # the quotes module will load the real database). Pick the first
        # stock as the one we test.
        goodname = invest.STOCKS.keys()[0]
        quote = { goodname: { "ticker": goodname, "trade": 386.91, "time": "10/3/2008", "date": "4.00pm", "variation": -3.58, "open": 397.14, "variation_pct": 10 }}
        qu.populate (quote)
        self.assertEqual (qu.quotes_valid, True)
        # In response to bug 554425, try a stock that isn't in our database
        quote = { "clearlyFake": { "ticker": "clearlyFake", "trade": 386.91, "time": "10/3/2008", "date": "4.00pm", "variation": -3.58, "open": 397.14, "variation_pct": 10 }}
        qu.populate (quote)
        self.assertEqual (qu.quotes_valid, False)

if __name__ == '__main__':
    unittest.main ()
