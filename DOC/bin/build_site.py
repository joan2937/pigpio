#!/usr/bin/env python3

import os
import sqlite3

db=sqlite3.connect("dbase/pigpio.sqlite")

c=db.cursor()

c.execute("select file_name from pigpio")

names = c.fetchall()

for n in names:
   os.system("bin/html.py {0} >HTML/{0}.html".format(n[0]))
   print(n[0])

c.close()

db.close()

