fivemiles-ios
=============

Fivemiles app iOS edition


https://github.com/ndfred/xcode-tools/blob/master/update_strings.py

We use this script to generate the i18n Localizable.strings file.

Copy this script to your root directory of your project , then run python update_strings.py <The Localizable.strings file you want to keep>



2015.4.14
localhost:Pinnacle alex$ find . -name '*.m' -exec cat {} \; | wc -l
50545
localhost:Pinnacle alex$ find . -name '*.h' -exec cat {} \; | wc -l
12285
---------------------------

47161 lines *.m 2015.3.24
11741 lines *.h 2015.3.24

---------------------------
38264 lines *.m 2015.01.21
8118  lines *.h 2015.01.21
----------------------------

33912 lines *.m 2014.12.16
7727  lines *.h 2014.12.16
----------------------------

29064  lines *.m 2014.11.26
7045   lines *.h 2014.11.26
----------------------------
25828  lines *.m 2014.11.13
6278   lines *.h 2014.11.13
---------------------------
22707 lines *.m   2014 10-30
5884 lines *.h   2014 10-30


////////////////////////
##find . -name '*.m' -exec cat {} \; | wc -l
##find . -name '*.h' -exec cat {} \; | wc -l
##  this is designed to calculate the total lines of codes in your project
##  version 2 @May 24, 2009
##  \- add file filter ,count specific file types add 
##  version 1 @october 8th,2008 
