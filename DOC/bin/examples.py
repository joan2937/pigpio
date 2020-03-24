#!/usr/bin/env python3

import sys
from collections import OrderedDict

def emit(s):
   sys.stdout.write(s)

def get_line(f):
   line = f.readline()
   return line

def get_file():
   try:
      fn = sys.argv[1]
      f = open(fn, "r")
   except:
      exit("aborting, can't open {}".format(fn))
   return f

def cancel_row():
   global in_row
   if in_row:
      in_row = False
      emit('</td></tr>')

def start_row():
   global in_row
   if not in_row:
      in_row = True
      emit('<tr><td style="width: 150px; vertical-align: top; font-size: 0.8em; font-weight: bold;">')

def cancel_table():
   global in_table
   if in_table:
      in_table = False
      cancel_row()
      emit('</tbody></table>')

def start_table():
   global in_table
   if not in_table:
      in_table = True
      emit('<table style="text-align: left; width: 90%;" border="0" cellpadding="4" cellspacing="4"><tbody>')
   else:
      cancel_row()
   start_row()

index = {}

in_code = False
in_samp = False
in_table = False
in_row = False

f = get_file()

while True:

   line = get_line(f)

   if line == "":
      cancel_table()

      emit('<table style="text-align: left; width: 90%;" border="0" cellpadding="4" cellspacing="4"><tbody>\n')
      last = None
      ordered = OrderedDict(sorted(index.items(), key=lambda t: t[1].lower()))
      for k,v in ordered.items():
         tag=k.split('_')[0]
         if last != v.lower():
            if last is not None:
               emit('</td></tr>')
            last = v.lower()
            anchor="index_"+last.replace(" ", "_")
            emit('<tr><td><span id="'+anchor+'"></span>'+v+'</td><td>')
         emit(' <a href="#'+k+'">'+tag+'</a>\n')
      emit('</td></tr></tbody></table>')
      break

   if line.startswith("?0|"):
      s = line.split("|")
      anchor=s[1].strip()
      emit('<a href="#'+anchor+'">'+anchor+'</a><br>')
      continue

   if line.startswith("?1|"):
      cancel_table()
      s = line.split("|")
      tag = s[1].strip()+"_"
      anchor=s[2].strip()
      emit('<h3><span id="'+anchor+'">'+anchor+'</span></h3>')
      continue

   elif line.startswith("?2|") or line.startswith("?3|") or line.startswith("?4|"):
      start_table()

      s = line.split("|")

      anchor=tag+s[1].strip()

      if line.startswith("?2|"):
         link=s[1].strip()+".html"
      elif line.startswith("?3|"):
         link="code/"+s[1].strip()+".zip"
      elif line.startswith("?4|"):
         link=s[1].strip()
      else:
         link="code/"+s[1].strip()

      date=s[2].strip()

      title=s[3].strip()
      emit('<span id="'+anchor+'"><a href="'+link+'">'+title+'</a><br>'+date+'</span></td><td>')

      index.update({anchor:title})

      continue

   else:
      emit(line.replace("\n", "<br>\n"))

f.close()

