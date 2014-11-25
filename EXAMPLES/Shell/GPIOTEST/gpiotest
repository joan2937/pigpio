#!/bin/bash

skipped=""
tested=""
failed=""

usage()
{
   cat <<EOF
This program checks the Pi's (user) gpios.

The program reads and writes all the gpios.  Make sure NOTHING
is connected to the gpios during this test.

The program uses the pigpio daemon which must be running.

To start the daemon use the command sudo pigpiod.

Press the ENTER key to continue or ctrl-C to abort...
EOF

   read a
}

restore_mode()
{
   # $1 gpio
   # $2 mode

   case "$2" in
        0) m="r" ;;
        1) m="w" ;;
        2) m="5" ;;
        3) m="4" ;;
        4) m="0" ;;
        5) m="1" ;;
        6) m="2" ;;
        7) m="3" ;;
        *)
           echo "invalid mode $2 for gpio $1"
           exit 1
   esac

   $(pigs m $1 $m)
}

check_gpio()
{
   # $1 gpio
   # $2 i2c

   m=$(pigs mg $1) # save mode
   L=$(pigs r $1)  # save level

   s=$(pigs m $1 w)

   if [[ $s  = "" ]]
   then
      f=0
      tested+="$1 "

      # write mode tests
      $(pigs w $1 0)
      r=$(pigs r $1)
      if [[ $r -ne 0 ]]; then f=1; echo "Write 0 to gpio $1 failed."; fi

      $(pigs w $1 1)
      r=$(pigs r $1)
      if [[ $r -ne 1 ]]; then f=1; echo "Write 1 to gpio $1 failed."; fi

      # read mode tests using pull-ups and pull-downs
      $(pigs m $1 r)

      if [[ $2 -eq 0 ]]
      then
         $(pigs pud $1 d)
         r=$(pigs r $1)
         if [[ $r -ne 0 ]]; then f=1; echo "Pull down on gpio $1 failed."; fi
      fi

      $(pigs pud $1 u)
      r=$(pigs r $1)
      if [[ $r -ne 1 ]]; then f=1; echo "Pull up on gpio $1 failed."; fi

      $(pigs pud $1 o)   # switch pull-ups/downs off
      $(pigs w $1 $L)    # restore original level
      restore_mode $1 $m # restore original mode

      if [[ $f -ne 0 ]]; then failed+="$1 "; fi
   else
      skipped+="$1 "
   fi
}  2>/dev/null

usage

v=$(pigs hwver)

if [[ $v < 0 ]]
then
   echo "The pigpio daemon wasn't found.  Did you sudo pigpiod?"
   exit
fi

echo "Testing..."

for ((i=0;  i<4;  i++)) do check_gpio $i 1; done
for ((i=4;  i<16; i++)) do check_gpio $i 0; done

if [[ $v -ge 16 ]];
then
   check_gpio 16 0
else
   skipped+="16 "
fi

for ((i=17;  i<28; i++)) do check_gpio $i 0; done
for ((i=28; i<30; i++)) do check_gpio $i 1; done
for ((i=30; i<32; i++)) do check_gpio $i 0; done

if [[ $failed = "" ]]; then failed="None"; fi

echo "Skipped non-user gpios: $skipped"
echo "Tested user gpios: $tested"
echo "Failed user gpios: $failed"

