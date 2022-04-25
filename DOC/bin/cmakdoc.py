#!/usr/bin/env python3

import sys

pigpio_m1="""
.\" Process this file with
.\" groff -man -Tascii pigpio.3
.\"
.TH pigpio 3 2012-2020 Linux "pigpio archive"
.SH NAME
pigpio - A C library to manipulate the Pi's GPIO.\n
.SH SYNOPSIS\n
#include <pigpio.h>\n

gcc -Wall -pthread -o prog prog.c -lpigpio -lrt\n
sudo ./prog
.SH DESCRIPTION\n
"""

pigpiod_if_m1="""
.\" Process this file with
.\" groff -man -Tascii pigpiod_if.3
.\"
.TH pigpiod_if 3 2012-2020 Linux "pigpio archive"
.SH NAME
pigpiod_if - A C library to interface to the pigpio daemon.\n
.SH SYNOPSIS\n
#include <pigpiod_if.h>\n

gcc -Wall -pthread -o prog prog.c -lpigpiod_if -lrt\n
 ./prog
.SH DESCRIPTION\n
"""

pigpiod_if2_m1="""
.\" Process this file with
.\" groff -man -Tascii pigpiod_if2.3
.\"
.TH pigpiod_if2 3 2012-2020 Linux "pigpio archive"
.SH NAME
pigpiod_if2 - A C library to interface to the pigpio daemon.\n
.SH SYNOPSIS\n
#include <pigpiod_if2.h>\n

gcc -Wall -pthread -o prog prog.c -lpigpiod_if2 -lrt\n
 ./prog
.SH DESCRIPTION\n
"""

pigpiod_m1="""
.\" Process this file with
.\" groff -man -Tascii pigpiod.1
.\"
.TH pigpiod 1 2012-2020 Linux "pigpio archive"
.SH NAME
pigpiod - A utility to start the pigpio library as a daemon.\n
.SH SYNOPSIS\n
sudo pigpiod [OPTION]...
.SH DESCRIPTION\n
"""

pig2vcd_m1="""
.\" Process this file with
.\" groff -man -Tascii pig2vcd.1
.\"
.TH pig2vcd 1 2012-2020 Linux "pigpio archive"
.SH NAME
pig2vd - A utility to convert pigpio notifications to VCD.\n
.SH SYNOPSIS\n
pig2vcd </dev/pigpioXX >file.VCD
.SH DESCRIPTION\n
"""

pigpio_m2="""
.SH SEE ALSO\n
pigpiod(1), pig2vcd(1), pigs(1), pigpiod_if(3), pigpiod_if2(3)
.SH AUTHOR\n
joan@abyz.me.uk
"""

pigpiod_if_m2="""
.SH SEE ALSO\n
pigpiod(1), pig2vcd(1), pigs(1), pigpio(3), pigpiod_if2(3)
.SH AUTHOR\n
joan@abyz.me.uk
"""

pigpiod_if2_m2="""
.SH SEE ALSO\n
pigpiod(1), pig2vcd(1), pigs(1), pigpio(3), pigpiod_if(3)
.SH AUTHOR\n
joan@abyz.me.uk
"""

pigpiod_m2="""
.SH SEE ALSO\n
pig2vcd(1), pigs(1), pigpio(3), pigpiod_if(3), pigpiod_if2(3)
.SH AUTHOR\n
joan@abyz.me.uk
"""

pig2vcd_m2="""
.SH SEE ALSO\n
pigpiod(1), pigs(1), pigpio(3), pigpiod_if(3), pigpiod_if2(3)
.SH AUTHOR\n
joan@abyz.me.uk
"""


def emit(s):
   sys.stdout.write(s)

def get_line(f):
   line = f.readline()
   if man:
      line = line.replace(" \n", "\n.br\n")
   else:
      line = line.replace("<", "&lt;")
      line = line.replace(">", "&gt;")
      line = line.replace(" \n", "<br>\n")
   return line

def nostar(k):
   if k[0] == "*":
      return k[1:]
   else:
      return k

NONE    =0
MAN     =1
TEXT    =2
OVERVIEW=4
FUNC    =5
DESC    =6
OPT     =7
PARAMS  =8
DEFS    =9

param_used = []
param_defd = []
param_refd = []

at = NONE

in_code = False
in_pard = False
in_table = False

if len(sys.argv) > 2:
   obj = sys.argv[1]
   fn = sys.argv[2]
   if len(sys.argv) > 3:
      man = True
   else:
      man = False
else:
   exit("bad args, need page file [man]")
try:
   f = open(fn, "r")
except:
   exit("aborting, can't open {}".format(fn))

if man:
   if obj == "pigpio":
      emit(pigpio_m1)
   elif obj == "pigpiod_if":
      emit(pigpiod_if_m1)
   elif obj == "pigpiod_if2":
      emit(pigpiod_if2_m1)
   elif obj == "pigpiod":
      emit(pigpiod_m1)
   elif obj == "pig2vcd":
      emit(pig2vcd_m1)

   emit("\n.ad l\n")
   emit("\n.nh\n")

while True:

   line = get_line(f)

   if line == "":
      for p in param_used:
         if p not in param_defd:
            sys.stderr.write("{} used but not defined.\n".format(p))
      for p in param_defd:
         if p not in param_used and p not in param_refd:
            sys.stderr.write("{} defined but not used.\n".format(p))
      break

   if line == "/*MAN\n":
      at = MAN
      continue

   elif line == "MAN*/\n":
      at = NONE
      continue

   if line == "/*TEXT\n":
      at = TEXT
      continue

   elif line == "TEXT*/\n":
      at = NONE
      continue

   elif line == "/*OVERVIEW\n":

      if man:
         emit("\n.SH OVERVIEW\n")
      else:
         emit("<h2>OVERVIEW</h2>")
         emit(
            "<table border=\"0\" cellpadding=\"2\" cellspacing=\"2\"><tbody>")

      at = OVERVIEW
      continue

   elif line == "OVERVIEW*/\n":

      if man:
         emit(".SH FUNCTIONS\n")
      else:
         emit("</tbody></table>")
         emit("<h2>FUNCTIONS</h2>")

      at = NONE

   elif line == "/*F*/\n":
      in_code = False
      fdef=""
      line = get_line(f)
      at = FUNC

   elif line == "/*D\n":
      # Function definition should be complete.
      fdef = fdef.replace("\n", " ")

      while fdef.find("  ") != -1:
         fdef = fdef.replace("  ", " ")

      fdef = fdef.replace("( ", "(")

      (rf, sep1, end1) = fdef.partition("(")
      (parl, sep2, end2) = end1.partition(")")
      tps = parl.split(",")
      rf = rf.split(" ")
      ret = rf[0]
      func = rf[1]

      if ret not in param_used:
         param_used.append(ret)

      if man:
         t = "\\fB" + ret + " " + func + "(" + parl + ")\\fP"
         emit("\n.IP \"{}\"\n.IP \"\" 4\n".format(t))
      else:
         emit("<h3><a name=\"{}\"></a><a href=\"#{}\"><small>{}</small></a> {}".
            format(nostar(func), ret, ret,func))
         emit("<small>(")

      x = 0
      for tp in tps:
         tp = tp.strip()
         (t, sep3, p) = tp.partition(" ")
         t = t.strip()
         p = p.strip()
         if (p != ""):
            if p not in param_used:
               param_used.append(p)
            if t not in param_used:
               param_used.append(t)
            if x > 0:

               if man:
                  pass
               else:
                  emit(", ")

            x += 1

            if man:
               pass
            else:

               emit("<a href=\"#{}\">{}</a> <a href=\"#{}\">{}</a>".
                  format(t, t, p, p))

         else:

            if man:
               pass
            else:
               emit("{}".format(t))

      if man:
         pass
      else:
         emit(")</small></h3>\n")

      line = get_line(f)
      at = DESC

   elif line == "D*/\n":
      at = NONE

   elif line == "/*O\n":
      if man:
         emit(".SH OPTIONS\n")
      else:
         emit("<table border=\"1\" cellpadding=\"2\" cellspacing=\"2\"><tbody>")
      at = OPT
      continue

   elif line == "O*/\n":
      if man:
         pass
      else:
         emit("</tbody></table>")
      at = NONE
      continue

   elif line == "/*PARAMS\n":
      last_par = "*"

      if man:
         emit(".SH PARAMETERS\n")
      else:
         emit("<h2>PARAMETERS</h2>")

      at = PARAMS
      continue

   elif line == "PARAMS*/\n":
      at = NONE

   elif line.startswith("/*DEF_S "):
      title = line[8:-3]
      in_code = True
      if man:
         emit(".SH {}\n".format(title))
         emit("\n.EX\n")
      else:
         emit("<h2>{}</h2>".format(title))
         emit("<code>")
      at = DEFS
      continue

   elif line == "/*DEF_E*/\n":
      in_code = False
      if man:
        emit("\n.EE\n")
      else:
         emit("</code>")
      at = NONE
      continue


   if at != NONE and at != OVERVIEW:
      if line.find("@") != -1:
         if not in_table:
            in_table = True

            if man:
               pass
            else:
               emit("<table border=\"1\" cellpadding=\"2\" cellspacing=\"2\"><tbody>")

         if man:
            line = line.replace("@", " ")
            emit(line)
            # emit("\n.br\n")
            emit(".br\n")
         else:
            emit("<tr>")
            cols = line.split("@")
            for col in cols:
               emit("<td>{}</td>".format(col.strip()))
            emit("</tr>")

         continue

      else:
         if in_table:
            in_table = False

            if man:
               pass
            else:
               emit("</tbody></table>")

      if line == "...\n" or line == ". .\n":
         if in_code:
            in_code = False

            if man:
               emit("\n.EE\n")
            else:
               emit("</code>")

         else:
            in_code = True
            if line == "...\n":

               if man:
                  emit("\\fBExample\\fP\n.br\n")
               else:
                  emit("<b><small>Example</small></b><br><br>")

            if man:
               emit("\n.EX\n")
            else:
               emit("<code>")

         continue

      if line == "\n":

         if man:
            emit("\n.br\n")
         else:
            emit("<br>")

         if not in_code:
            if man:
              emit("\n.br\n")
            else:
               emit("<br>")

         continue

      if in_code:

         if man:
            line = line.replace("\n", "\n.br\n")
         else:
            line = line.replace(" ", "&nbsp;")
            line = line.replace("\n", "<br>")

      while line.find("[*") != -1 and line.find("*]") != -1:
         (b, s, e) = line.partition("[*")
         (l, s, e) = e.partition("*]")

         if man:
            line = "{}\\fB{}\\fP{}".format(b, l, e)
         else:
            line = "{}<a href=\"#{}\">{}</a>{}".format(b, l, l, e)

         if l not in param_refd:
            param_refd.append(l)

      while line.find("[[") != -1 and line.find("]]") != -1:
         (b, s, e) = line.partition("[[")
         (l, s, e) = e.partition("]]")

         if man:
            line = "{}\\fB{}\\fP{}".format(b, l, e)
         else:
            line = "{}<a href=\"{}\">{}</a>{}".format(b, l, l, e)

   if at == TEXT:
      if line[0] == '*' and line[-2] == '*':
         if man:
            emit(".SS {}".format(line[1:-2]))
         else:
            emit("<h3>{}</h3>".format(line[1:-2]))
      elif line[0] == '^' and line[-2] == '^':
         if man:
            emit(".SS {}".format(line[1:-2]))
         else:
            emit("<br><b>{}</b><br>".format(line[1:-2]))
      else:
         emit(line)

   elif at == OVERVIEW:
      if line == "\n":

         if man:
            emit("\n.br\n")
         else:
            emit("<tr><td></td><td></td></tr>")

      else:
         (func, sep, desc) = line.partition(" ")
         if desc != "":

            if man:
               emit("\n.br\n{}".format(line.strip()))
            else:
               emit("<tr><td><a href=\"#{}\">{}</a></td><td>{}</td></tr>".
                  format(func, func, desc))

         else:

            if man:
               emit(".SS {}".format(line.replace("_", " ").strip()))
            else:
               emit("<tr><td><b>{}</b></td><td></td></tr>".
                  format(func.replace("_", " ")))

   elif at == FUNC:
      fdef += line

   elif at == DESC:
      emit(line)

   elif at == PARAMS:
      if line.find("::") != -1:
         (par, sep, end) = line.partition("::")
         par = par.strip()
         end = end.strip()

         if nostar(par.lower()) < nostar(last_par.lower()):
            sys.stderr.write("Out of order {} after {}.\n".
               format(par, last_par))
         last_par = par

         if par in param_defd:
            sys.stderr.write("Duplicate definition of {}.\n".format(par))
         else:
            param_defd.append(par)

         if end != "":
            end = ": " + end
         if man:
            emit("\n.IP \"\\fB{}\\fP{}\" 0\n".format(par, end))
         else:
            emit("<h3><a name=\"{}\">{}</a>{}</h3>\n".format(par, par, end))

      else:
         emit(line)

   elif at == MAN:
      if man:
         emit(line)

   elif at == OPT:
      line = line.split("|")
      if man:
         emit("\n.IP \"\\fB{}\\fP\"\n{}.\n{}.\n{}.".format(
         line[0], line[1], line[2], line[3]))
      else:
         emit("<tr><td><b>{}</b></td><td>{}</td><td>{}</td><td>{}</td></tr>".
            format(line[0], line[1], line[2], line[3]))

   elif at == DEFS:
      emit(line)

if man:
   if obj == "pigpio":
      emit(pigpio_m2)
   elif obj == "pigpiod_if":
      emit(pigpiod_if_m2)
   elif obj == "pigpiod_if2":
      emit(pigpiod_if2_m2)
   elif obj == "pigpiod":
      emit(pigpiod_m2)
   elif obj == "pig2vcd":
      emit(pig2vcd_m2)


f.close()

