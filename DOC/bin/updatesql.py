#!/usr/bin/env python3

import sqlite3
import glob

db=sqlite3.connect("dbase/pigpio.sqlite")

c=db.cursor()

for fn in glob.glob("tmp/body/*.body"):
   f = open(fn, encoding='utf-8')
   body = f.read()
   f.close()
   c.execute("UPDATE pigpio SET body=? WHERE file_name=?", (body, fn[9:-5]))

c.close()

db.commit()

db.close()

