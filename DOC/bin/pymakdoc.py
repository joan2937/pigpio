#!/usr/bin/env python3

import sys

def emit(s):
   sys.stdout.write(s)

def str2hex(s):
   return ":".join("{:02x}".format(ord(c)) for c in s)

def funchdr(line):
   if len(line) > 1 and (not line.startswith("  ")) and  line.find("(") != -1:
      return True
   else:
      return False

def get_line(f):
   line = f.readline()
   line = line.replace("<", "&lt;")
   line = line.replace(">", "&gt;")
   return line

def get_file():
   try:
      fn = sys.argv[1]
      f = open(fn, "r")
   except:
      exit("aborting, can't open {}".format(fn))
   return f


NONE        = 0
DESCRIPTION = 1
OVERVIEW    = 2
CLASSES     = 3
CLASS       = 4
FUNC        = 5
XREF        = 6
DATA        = 7

f = get_file()

param_used = []
param_defd = []
param_refd = []

at = NONE

in_code = False
in_samp = False
in_table = False
left = 0

while True:

   line = get_line(f)

   if line == "":
      for p in param_used:
         if p not in param_defd:
            sys.stderr.write("{} is used but not defined.\n".format(p))
      for p in param_defd:
         if p not in param_used and p not in param_refd:
            sys.stderr.write("{} is defined but not used.\n".format(p))
      break

   if line.startswith("DESCRIPTION"):
      left = 4
      at = DESCRIPTION
      continue

   elif line.startswith("    OVERVIEW"):
      left = 4
      emit("<h2>OVERVIEW</h2>")
      emit(
         "<table border=\"0\" cellpadding=\"2\" cellspacing=\"2\"><tbody>")
      at = OVERVIEW
      continue

   elif line.startswith("CLASSES"):
      left = 4
      emit("</tbody></table>")
      at = NONE
      continue

   elif line.startswith("    class "):
      in_func = False
      if line.startswith("    class error"):
         at = NONE
      else:
         left = 8
         emit("<h2>{}</h2>".format(line))
         _class = line[10:-1]
         at = CLASS
      continue

   elif line.startswith("FUNCTIONS"):
      left = 4
      emit("<h2>FUNCTIONS</h2>")
      in_func = False
      at = FUNC
      continue

   elif line.startswith("    xref()"):
      left = 8
      emit("<h2>PARAMETERS</h2>")
      in_func = False
      last_par = ""
      at = XREF
      continue

   elif line.startswith("DATA"):
      at = DATA
      continue

   line = line[left:]

   at_funchdr = funchdr(line)

   if at != NONE and at != OVERVIEW:

      if at == CLASS or at == FUNC:

         if not at_funchdr:
            line = line[4:]

      line = line.replace(' \n', '<br>')

      if line.find('@') != -1:
         if not in_table:
            in_table = True
            emit("<table border=\"1\" cellpadding=\"2\" cellspacing=\"2\"><tbody>")
         emit("<tr>")
         cols = line.split('@')
         for col in cols:
            emit("<td>{}</td>".format(col.strip()))
         emit("</tr>")
         continue
      else:
         if in_table:
            in_table = False
            emit("</tbody></table>")

      if line.find(":=") != -1:
         if not in_samp:
            emit("<br><b><small>Parameters</small></b><br><br><samp>")
            in_samp = True

      if line == '...\n' or line == '. .\n':
         if in_code:
            in_code = False
            emit("</code>")
         else:
            in_code = True
            if line == '...\n':
               emit("<b><small>Example</small></b><br><br>")
            emit("<code>")
         continue

      if line == '\n' or line == '':
         if in_samp:
            emit("</samp>")
            in_samp = False
         emit('<br>')
         if not in_code:
            emit('<br>')
         continue

      if in_code or in_samp:
         line = line.replace(" ", "&nbsp;")
         line = line.replace("\n", "<br>")

      while line.find('[*') != -1 and line.find('*]') != -1:
         (b, s, e) = line.partition('[*')
         (l, s, e) = e.partition('*]')
         line = '{}<a href="#{}">{}</a>{}'.format(b, l, l, e)
         if l not in param_refd:
            param_refd.append(l)

      while line.find('[[') != -1 and line.find(']]') != -1:
         (b, s, e) = line.partition('[[')
         (l, s, e) = e.partition(']]')
         line = '{}<a href="{}">{}</a>{}'.format(b, l, l, e)

   if at == DESCRIPTION:
      if line[0] == '*' and line[-2] == '*':
         emit("<h3>{}</h3>".format(line[1:-2]))
      elif line[0] == '[' and line[-2] == ']':
         pass
      else:
         emit(line)

   elif at == OVERVIEW:
      if line == "\n":
         emit("<tr><td></td><td></td></tr>")
      else:
         (func, sep, desc) = line.partition(' ')
         if desc != '':
            emit("<tr><td><a href=\"#{}\">{}</a></td><td>{}</td></tr>".
               format(func, func, desc))
         else:
            emit("<tr><td><b>{}</b></td><td></td></tr>".
               format(func).replace("_", " "))

   elif at == CLASS or at == FUNC:
      if at_funchdr:
         in_func = True

         if in_code:
            emit("<br>***C***</code>")
            in_code = False

         if in_samp:
            emit("<br>***S***</samp>")
            in_samp = False

         (func, sep1, end1) = line.partition("(")
         (parl, sep2, end2) = end1.partition(")")
         pars = parl.split(",")

         func = func.strip()

         if at == FUNC:
            func = "pigpio." + func
         else:
            if func == "__init__":
               func = "pigpio." + _class

         emit("<h3><a name=\"{}\">{}".format(func,func))

         emit('<small>(')

         x = 0
         for p in pars:
            (p, sep3, end3) = p.partition('=')
            p = p.strip()
            if (p != 'self') and (p != '...') and (p != ''):
               if p not in param_used:
                  param_used.append(p)
               if x > 0:
                  emit(", ")
               x += 1
               emit("<a href=\"#{}\">{}</a>".format(p, p))
         emit(')</small></h3>\n')

      else:
         if in_func:
            emit(line)

   elif at == XREF:
      if line.find(':') != -1:
         (par, sep, end) = line.partition(':')
         par = par.strip()
         end = end.strip()
         emit("<h3><a name=\"{}\"></a>{}: {}</h3>".format(par, par, end))

         if par.lower() < last_par.lower():
            sys.stderr.write("Out of order {} after {}.\n".
               format(par, last_par))
         last_par = par

         if par in param_defd:
            sys.stderr.write("Duplicate definition of {}.\n".format(par))
         else:
            param_defd.append(par)
      else:
         emit(line)

f.close()

