#!/usr/bin/env python3

import glob

for fn in glob.glob("src/html/*.html"):
   f = open(fn)
   h = f.read()
   f.close()
   s1,d1,e1=h.partition("<body>")
   s2,d2,e2=e1.partition("</body>")
   f = open("tmp/body/" + fn[9:-5] + ".body", "w")
   f.write(s2)
   f.close()

