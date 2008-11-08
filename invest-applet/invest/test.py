#!/usr/bin/env python

import unittest

import quotes
import invest

def null_function (*args):
    pass

class TestQuotes (unittest.TestCase):
    def testQuoteUpdater_populate (self):
        qu = quotes.QuoteUpdater (null_function, null_function)
        invest.STOCKS = {'GOGO': [{'amount' : 1, 'comission' : 0.0, 'bought': 0.0}], 'JAVA': [{'amount' : 1, 'comission' : 0.0, 'bought': 0.0}]}
        quote = { 'GOGO': { "ticker": 'GOGO', "trade": 386.91, "time": "10/3/2008", "date": "4.00pm", "variation": -3.58, "open": 397.14, "variation_pct": 10 }}
        qu.populate (quote)
        self.assertEqual (qu.quotes_valid, True)
        # In response to bug 554425, try a stock that isn't in our database
        quote = { "clearlyFake": { "ticker": "clearlyFake", "trade": 386.91, "time": "10/3/2008", "date": "4.00pm", "variation": -3.58, "open": 397.14, "variation_pct": 10 }}
        qu.populate (quote)
        self.assertEqual (qu.quotes_valid, False)

if __name__ == '__main__':
    unittest.main ()
