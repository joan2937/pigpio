#!/bin/bash

# if backup same as new delete it
if cmp -s dbase/pigpio.sqlite dbase/pigpio.sqlite.bak
then
   rm dbase/pigpio.sqlite.bak
else
   d=$(date "+%F-%H-%M-%S")
   mv -f dbase/pigpio.sqlite.bak dbase/pigpio.sqlite.$d
fi

# delete backups older than a week
find dbase/pigpio.sqlite.2* -mtime +7 -delete &>/dev/null

