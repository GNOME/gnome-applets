#!/bin/sh

#simple test of tickastat news module output format
#it is up to the script to download any images to the local path
#command line arguments:
# test.sh <filename> <directory>
#    filename: where to place the text info.
#    directory: where to place downloaded images.
#
# output should print one of these before exiting:
#  pass      printed in response to a test request (filename = "test")
#  fail      printed in response to a test request
#            subsequent lines are considered the reason
#  done      script is finished processing, data file is ready
#  error     script encountered an error during processing
#            subsequent lines are considered the reason
# <anything else> considered the same as error, and the reason
#
#
# Note: counts for article and body must start at 0

if [ "$1" = "test" ] ; then
	echo "pass"
	exit
fi

echo "[article_0]" > $1
echo "date=1999-07-04 12:00:30 EDT" >> $1
echo "heading=We have news" >> $1
echo "topic=Things to see" >> $1
echo "url=file://" >> $1
echo "image=image.png" >> $1
echo "image_lib=test.png" >> $1
echo "body_0=blah blah" >> $1
echo "body_1=more blah blah" >> $1
echo "display_count=1"
echo "[article_1]" >> $1
echo "heading=We have more news" >> $1
echo "body_0=this line only has a summary and 1 detail." >> $1

echo "done"

#done.
