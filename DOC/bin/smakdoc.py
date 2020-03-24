#!/usr/bin/env python3

import sys

def get_file():
   try:
      fn = sys.argv[1]
      f = open(fn, "r")
   except:
      exit("aborting, can't open {}".format(fn))
   return f

def emit(s):
   sys.stdout.write(s)

def str2hex(s):
   return ":".join("{:02x}".format(ord(c)) for c in s)

def get_line(f):
   line = f.readline()
   if man:
      line = line.replace(" \n", "\n.br\n")
   else:
      line = line.replace("<", "&lt;")
      line = line.replace(">", "&gt;")
      line = line.replace(" \n", "<br>\n")
   return line

if len(sys.argv) > 2: # Are we creating a man page?
   man = True
else:
   man = False

NONE=0
INTRO=1
OVERVIEW=2
COMMANDS=3
PARAMETERS=4
SCRIPTS=5

param_used = []
param_defd = []

f = get_file()

at = NONE

in_table = False
in_code = False

funcdef ={}

if man:
   emit("""
.\" Process this file with
.\" groff -man -Tascii foo.1
.\"
.TH pigs 1 2012-2020 Linux "pigpio archive"
.SH NAME
pigs - command line socket access to the pigpio daemon.\n
/dev/pigpio - command line pipe access to the pigpio daemon.\n
.SH SYNOPSIS\n
.B sudo pigpiod\n
then\n
.B pigs {command}+\n
or\n
.B \"echo {command}+ >/dev/pigpio\"\n
.SH DESCRIPTION\n
.ad l\n
.nh\n
""")

while True:

   line = get_line(f)

   if line == "":
      for p in param_used:
         if p not in param_defd:
            sys.stderr.write("{} used but not defined.\n".format(p))
      for p in param_defd:
         if p not in param_used:
            sys.stderr.write("{} defined but not used.\n".format(p))
      break

   if line.startswith("INTRO"):
      line = get_line(f)

      if man:
         pass
      else:
         emit("<h2><a name=\"Introduction\">Introduction</a></h2>\n")

      at = INTRO

   elif line.startswith("OVERVIEW"):
      line = get_line(f)

      if man:
         emit("\n.SH OVERVIEW\n")
      else:
         emit("<h2><a name=\"Overview\">Overview</a></h2>\n")
         emit(
            "<table border=\"0\" cellpadding=\"2\" cellspacing=\"2\"><tbody>")
      at = OVERVIEW

   elif line.startswith("COMMANDS"):
      line = get_line(f)

      if man:
         emit("\n.SH COMMANDS\n")
      else:
         emit("</tbody></table>")
         emit("<h2><a name=\"Commands\">Commands</a></h2>\n")

      funcs = sorted(funcdef)
      at = COMMANDS

   elif line.startswith("PARAMETERS"):
      line = get_line(f)
      last_par = ""

      if man:
         emit("\n.SH PARAMETERS\n")
      else:
         emit("<h2><a name=\"Parameters\">Parameters</a></h2>\n")

      at = PARAMETERS

   elif line.startswith("SCRIPTS"):
      line = get_line(f)

      if man:
         emit("\n.SH SCRIPTS\n")
      else:
         emit("<h2><a name=\"Scripts\">Scripts</a></h2>\n")

      at = SCRIPTS

   if at != NONE:
      if line.find("@") != -1:
         if not in_table:
            in_table = True

            if man:
               emit("\n.EX\n")
            else:
               emit("<table border=\"1\" cellpadding=\"2\" cellspacing=\"2\"><tbody>")
         if man:
            pass
         else:
            emit("<tr>")

         if man:
            line = line.replace("@", " ")
            emit(line)
         else:
            cols = line.split("@")
            for col in cols:
               emit("<td>{}</td>".format(col.strip()))

         if man:
            pass
         else:
            emit("</tr>")
         continue
      else:
         if in_table:
            in_table = False

            if man:
               emit("\n.EE\n")
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

            if at == COMMANDS:

               if line == "...\n":
                  if man:
                     emit("\n\\fBExample\\fP\n.br\n")
                  else:
                     emit("<b><small>Example</small></b><br><br>")

            if man:
               emit("\n.EX\n")
            else:
               emit("<code>")

         continue

      if line == "\n" and at != OVERVIEW:
         if man:
            # emit("\n.br\n.br\n")
            emit("\n.br\n")
         else:
            emit("<br><br>")
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

      while line.find("[{") != -1 and line.find("}]") != -1:
         (b, s, e) = line.partition("[[")
         (l, s, e) = e.partition("]]")

         if man:
            line = "{}\\fB{}\\fP{}".format(b, l, e)
         else:
            line = "{}<a href=\"{}\">{}</a>{}".format(b, l, l, e)

   if at == INTRO or at == SCRIPTS:
      if line[0] == "*" and line[-2] == "*":
         if man:
            emit(".SS {}".format(line[1:-2]))
         else:
            emit("<h3>{}</h3>".format(line[1:-2]))
      else:
         emit(line)

   elif at == OVERVIEW:
      if line == "\n":

         if man:
            pass
         else:
            emit("<tr><td></td><td></td><td></td></tr>")

      elif line.find("::") == -1:

         if man:
            emit(".SS {}".format(line))
         else:
            emit("<tr><td>{}</td><td></td><td></td></tr>".format(line))
      else:
         (funcpar, sep, desc) = line.partition("::")
         funcpar = funcpar.strip()
         desc = desc.strip()
         if desc != "":
            (func, sep, parl) = funcpar.partition(" ")
            func = func.strip()
            parl = parl.strip()
            if func in funcdef:
               sys.stderr.write("{} has been redefined.\n".format(func))
            funcdef[func]=parl+"::"+desc

            if man:
               emit(".B {}\n".format(func+" "+parl+" "))
               pass
            else:
               emit("<tr><td><a href=\"#{}\">{}</a>".format(func, func))

            pars = parl.split()

            for p in pars:

               if p not in param_used:
                  param_used.append(p)

               if man:
                  pass
               else:
                  emit(" <a href=\"#{}\">{}</a>".format(p, p))

            (des, sep, c) = desc.partition("::")
            c = c.strip()

            if man:
               emit("{}\n.P\n".format(des))
               pass
            else:
               emit("</td><td>{}</td><td><small><a href=\"cif.html#{}\">{}</a></small></td></tr>".format(des, c, c))

         else:
            if man:
               pass
            else:
               emit("<tr><td><b>{}</b></td><td></td></tr>".format(funcpar))

            if man:
               # emit(".IP {} 9\n".format(func))
               pass
            else:
               emit("<tr><td><a href=\"#{}\">{}</a>".format(func, func))


   elif at == COMMANDS:
      if line.find("::") != -1:
         (func, sep, desc) = line.partition("::")
         func = func.strip()

         if func not in funcdef:
            parl="unknown"
            desc="unknown"
         else:
            (parl,sep,desc) = funcdef[func].partition("::")

         (des, sep, c) = desc.partition("::")

         if man:
            t = "\\fB" + func + " " + parl + "\\fP" + " - " + des.strip()
            emit("\n.IP \"{}\"\n.IP \"\" 4\n".format(t))
         else:
            emit("<h3><a name=\"{}\">{}</a>\n".format(func,func))

         pars = parl.split()
         for p in pars:

            if man:
               pass
            else:
               emit(" <a href=\"#{}\">{}</a>".format(p, p))

         if man:
            pass
         else:
            emit(" - {}</h3>".format(des.strip()))

      else:
         emit(line)

   elif at == PARAMETERS:
      if line.find("::") != -1:
         (par, sep, desc) = line.partition("::")
         par = par.strip()
         desc = desc.strip()

         if par.lower() < last_par.lower():
            sys.stderr.write("Out of order {} after {}.\n".format(par, last_par))
         last_par = par

         if par in param_defd:
            sys.stderr.write("Duplicate definition of {}.\n".format(par))
         else:
            param_defd.append(par)

         if man:
            emit("\n.IP \"\\fB{}\\fP - {}\" 0\n".format(par, desc))
         else:
            emit("<h3><a name=\"{}\">{}</a> - {}</h3>\n".format(par,par,desc))
      else:
         emit(line)

if man:
   emit("""
.SH SEE ALSO\n
pigpiod(1), pig2vcd(1), pigpio(3), pigpiod_if(3), pigpiod_if2(3)
.SH AUTHOR\n
joan@abyz.me.uk
""")

f.close()

