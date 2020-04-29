#!/usr/bin/env python3

import glob

def snafu(t, s, r):
   l = t
   while l.find(s) != -1:
      l = l.replace(s, r)
   return l

for n in glob.glob("tmp/body/*.body"):
   
   f = open(n, "r");
   t = f.read()
   f.close()
   
   t = snafu(t, "<br><h2>", "<h2>")
   t = snafu(t, "<br><h3>", "<h3>")
   t = snafu(t, "</h2><br>", "</h2>")
   t = snafu(t, "</h2>\n<br>", "</h2>\n")
   t = snafu(t, "</h3><br>", "</h3>")
   t = snafu(t, "</h3>\n<br>", "</h3>\n")
   t = snafu(t, "<br><br><br>", "<br><br>")

   f = open(n, "w");
   f.write(t)
   f.close()

