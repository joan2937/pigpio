#!/usr/bin/env python3

import sys
import sqlite3
import time

i_file_name = 0
i_menu_title = 1
i_menu_pos = 2
i_menu_level = 3
i_page_title = 4
i_pic1 = 5
i_pic2 = 6
i_pic3 = 7
i_body = 8

print("""
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
   "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>
   <meta name="description" content="Raspberry Pi Reg. C GPIO library and Python GPIO module and shell command utilities to control the GPIO, including SPI, I2C, and serial links." />
   <meta name="keywords" content="raspberry, pi, C, Python, GPIO, library, shell, command, utilities, module, SPI, I2C, serial" />
   <meta http-equiv="Content-Type" content="text/html;charset=utf-8" />
   <title>pigpio library</title>
   <link rel="stylesheet" type="text/css" href="scripts/index.css">
   <link rel="icon" href="favicon.ico" type="image/x-icon">
   <link rel="shortcut icon" href="favicon.ico" type="image/x-icon">
</head>
<body>
""")

page   = sys.argv[1]

menuH = ""
menuV = ""
sitemap = ""

header = '<a href="index.html"><img src="images/pigpio-logo.gif" border="0" /></a>pigpio library'

footer1 = "<small>&copy; 2012-2020</small>";
footer2 = "e-mail: pigpio @ abyz.me.uk";
footer3 = "<small>Updated: " + time.strftime("%d/%m/%Y") + "</small>";

db=sqlite3.connect("dbase/pigpio.sqlite")

c=db.cursor()

def menu_titles():

   global menuV, menuH

   c.execute(
      "SELECT file_name, menu_title, menu_level FROM pigpio ORDER by menu_pos")

   recs = c.fetchall()

   menuV = ""
   menuH = ""
 
   for r in recs:
      if r[2] == 1:
         menuV += '<a class="l1" href="' + r[0] + '.html">' + r[1] + '</a>\n'
         menuH += '<a class="l2" href="' + r[0] + '.html">[' + r[1] + ']</a>\n'

def sitemap():

   c.execute(
      "SELECT file_name, menu_title, menu_level FROM pigpio ORDER by menu_pos")

   recs = c.fetchall()

   stemap = ""
 
   for r in recs:
      if r[2] > 0:
         s = "----" * (r[2]-1)
         stemap += s + '<a href="' + r[0] + '.html">' + r[1] + '</a><br>\n'

   return stemap

def check_image(d):

   img = "images/" + d

   try:
      with open("HTML/" + img) as f:
         print('<td><img src="' + img + '" width="250"></td>')
   except:
      pass

titles = menu_titles()

s_sidebar = 'style="background:#EAF2E6 url(\'images/sidebar.gif\') repeat-y; width:35px; height:100%"'

s_header = 'style="background:url(\'images/topbar.gif\') repeat-x; height: 70px; font-size:1.5em; vertical-align: top;"'

s_menuV = 'style="vertical-align: top; background-color: #98bf21;"'

c.execute("SELECT * FROM pigpio WHERE file_name=?", (page,))

rec = c.fetchone()

if page == "sitemap":
   body = sitemap()
else:
   body = rec[i_body]

c.close()
db.close()

print('<table style="padding:0px; border:0px; margin:0px; width:780px; background-color:#e0e0e0;">')
print('<td ' +  s_sidebar + '></td>')
print('<td>')
print('<table>')
print('<div ' + s_header + '>' + header + '</div>')
print('</table>')
print('<table><div>')
check_image(rec[i_pic1])
check_image(rec[i_pic2])
check_image(rec[i_pic3])
print('</div></table>')
print("<table>")
print('<td ' + s_menuV + '>' + menuV + '</td>')
print('<td><center><h2>' + rec[i_page_title] + '</h2></center>' + body + '</td>')
print('</table>')
print('<div style="vertical-align: center; text-align: center; background-color:#98bf21; font-size:0.8em; height:30px">' + menuH + '</div>')
print('<table><tr>')
print('<td style="width: 200px"><div style="text-align: left;">' + footer1 + '</div></td>')
print('<td style="width: 350px"><div style="text-align: center;">' + footer2 + '</div></td>')
print('<td style="width: 200px"><div style="text-align: right;">' + footer3 + '</div></td>')
print('</tr></table>')
print('</td>')
print('</table>')

print('</body>\n</html>')

