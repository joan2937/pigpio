#!/usr/bin/env python

import time
import struct

import pigpio

GPIO=4

def CHECK(t, st, got, expect, pc, desc):
   if got >= (((1E2-pc)*expect)/1E2) and got <= (((1E2+pc)*expect)/1E2):
      print("TEST {:2d}.{:<2d} PASS ({}: {:d})".format(t, st, desc, expect))
   else:
      print("TEST {:2d}.{:<2d} FAILED got {:d} ({}: {:d})".
         format(t, st, got, desc, expect))

def t0():
   print("Version.")

   print("pigpio version {}.".format(pigpio.get_pigpio_version()))

   print("Hardware revision {}.".format(pigpio.get_hardware_revision()))

def t1():

   print("Mode/PUD/read/write tests.")

   pigpio.set_mode(GPIO, pigpio.INPUT)
   v = pigpio.get_mode(GPIO)
   CHECK(1, 1, v, 0, 0, "set mode, get mode")

   pigpio.set_pull_up_down(GPIO, pigpio.PUD_UP)
   v = pigpio.read(GPIO)
   CHECK(1, 2, v, 1, 0, "set pull up down, read")

   pigpio.set_pull_up_down(GPIO, pigpio.PUD_DOWN)
   v = pigpio.read(GPIO)
   CHECK(1, 3, v, 0, 0, "set pull up down, read")

   pigpio.write(GPIO, pigpio.LOW)
   v = pigpio.get_mode(GPIO)
   CHECK(1, 4, v, 1, 0, "write, get mode")

   v = pigpio.read(GPIO)
   CHECK(1, 5, v, 0, 0, "read")

   pigpio.write(GPIO, pigpio.HIGH)
   v = pigpio.read(GPIO)
   CHECK(1, 6, v, 1, 0, "write, read")

t2_count=0

def t2cbf(gpio, level, tick):
   global t2_count
   t2_count += 1

def t2():
   global t2_count

   print("PWM dutycycle/range/frequency tests.")

   pigpio.set_PWM_range(GPIO, 255)
   pigpio.set_PWM_frequency(GPIO,0)
   f = pigpio.get_PWM_frequency(GPIO)
   CHECK(2, 1, f, 10, 0, "set PWM range, set/get PWM frequency")

   t2cb = pigpio.callback(GPIO, pigpio.EITHER_EDGE, t2cbf)

   pigpio.set_PWM_dutycycle(GPIO, 0)
   time.sleep(0.5) # allow old notifications to flush
   oc = t2_count
   time.sleep(2)
   f = t2_count - oc
   CHECK(2, 2, f, 0, 0, "set PWM dutycycle, callback")

   pigpio.set_PWM_dutycycle(GPIO, 128)
   time.sleep(1)
   oc = t2_count
   time.sleep(2)
   f = t2_count - oc
   CHECK(2, 3, f, 40, 5, "set PWM dutycycle, callback")

   pigpio.set_PWM_frequency(GPIO,100)
   f = pigpio.get_PWM_frequency(GPIO)
   CHECK(2, 4, f, 100, 0, "set/get PWM frequency")

   time.sleep(1)
   oc = t2_count
   time.sleep(2)
   f = t2_count - oc
   CHECK(2, 5, f, 400, 1, "callback")

   pigpio.set_PWM_frequency(GPIO,1000)
   f = pigpio.get_PWM_frequency(GPIO)
   CHECK(2, 6, f, 1000, 0, "set/get PWM frequency")

   time.sleep(1)
   oc = t2_count
   time.sleep(2)
   f = t2_count - oc
   CHECK(2, 7, f, 4000, 1, "callback")

   r = pigpio.get_PWM_range(GPIO)
   CHECK(2, 8, r, 255, 0, "get PWM range")

   rr = pigpio.get_PWM_real_range(GPIO)
   CHECK(2, 9, rr, 200, 0, "get PWM real range")

   pigpio.set_PWM_range(GPIO, 2000)
   r = pigpio.get_PWM_range(GPIO)
   CHECK(2, 10, r, 2000, 0, "set/get PWM range")

   rr = pigpio.get_PWM_real_range(GPIO)
   CHECK(2, 11, rr, 200, 0, "get PWM real range")

   pigpio.set_PWM_dutycycle(GPIO, 0)

t3_reset=True
t3_count=0
t3_tick=0
t3_on=0.0
t3_off=0.0

def t3cbf(gpio, level, tick):
   global t3_reset, t3_count, t3_tick, t3_on, t3_off

   if t3_reset:
      t3_count = 0
      t3_on = 0.0
      t3_off = 0.0
      t3_reset = False
   else:
      td = pigpio.tickDiff(t3_tick, tick)

      if level == 0:
         t3_on += td
      else:
         t3_off += td

   t3_count += 1
   t3_tick = tick

def t3():
   global t3_reset, t3_count, t3_on, t3_off

   pw=[500.0, 1500.0, 2500.0]
   dc=[0.2, 0.4, 0.6, 0.8]

   print("PWM/Servo pulse accuracy tests.")

   t3cb = pigpio.callback(GPIO, pigpio.EITHER_EDGE, t3cbf)

   t = 0
   for x in pw:
      t += 1
      pigpio.set_servo_pulsewidth(GPIO, x)
      time.sleep(1)
      t3_reset = True
      time.sleep(4)
      c = t3_count
      on = t3_on
      off = t3_off
      CHECK(3, t, int((1E3*(on+off))/on), int(2E7/x), 1, "set servo pulsewidth")


   pigpio.set_servo_pulsewidth(GPIO, 0)
   pigpio.set_PWM_frequency(GPIO, 1000)
   f = pigpio.get_PWM_frequency(GPIO)
   CHECK(3, 4, f, 1000, 0, "set/get PWM frequency")

   rr = pigpio.set_PWM_range(GPIO, 100)
   CHECK(3, 5, rr, 200, 0, "set PWM range")

   t = 5
   for x in dc:
      t += 1
      pigpio.set_PWM_dutycycle(GPIO, x*100)
      time.sleep(1)
      t3_reset = True
      time.sleep(2)
      c = t3_count
      on = t3_on
      off = t3_off
      CHECK(3, t, int((1E3*on)/(on+off)), int(1E3*x), 1, "set PWM dutycycle")

   pigpio.set_PWM_dutycycle(GPIO, 0)

def t4():

   print("Pipe notification tests.")

   pigpio.set_PWM_frequency(GPIO, 0)
   pigpio.set_PWM_dutycycle(GPIO, 0)
   pigpio.set_PWM_range(GPIO, 100)

   h = pigpio.notify_open()
   e = pigpio.notify_begin(h, (1<<4))
   CHECK(4, 1, e, 0, 0, "notify open/begin")

   time.sleep(1)

   with open("/dev/pigpio"+ str(h), "rb") as f:

      pigpio.set_PWM_dutycycle(GPIO, 50)
      time.sleep(4)
      pigpio.set_PWM_dutycycle(GPIO, 0)

      e = pigpio.notify_pause(h)
      CHECK(4, 2, e, 0, 0, "notify pause")

      e = pigpio.notify_close(h)
      CHECK(4, 3, e, 0, 0, "notify close")

      n = 0
      s = 0

      seq_ok = 1
      toggle_ok = 1

      while True:

         chunk = f.read(12)

         if len(chunk) == 12:

            S, fl, t, v = struct.unpack('HHII', chunk)
            if s != S:
               seq_ok = 0


            L = v & (1<<4)

            if n:
               if l != L:
                  toggle_ok = 0

            if L:
               l = 0
            else:
               l = (1<<4)
           
            s += 1
            n += 1

            # print(S, fl, t, hex(v))

         else:
            break

      f.close()

      CHECK(4, 4, seq_ok, 1, 0, "sequence numbers ok")

      CHECK(4, 5, toggle_ok, 1, 0, "gpio toggled ok")

      CHECK(4, 6, n, 80, 10, "number of notifications")

t5_count = 0

def t5cbf(gpio, level, tick):
   global t5_count
   t5_count += 1

def t5():
   global t5_count

   BAUD=4800

   TEXT="""
Now is the winter of our discontent
Made glorious summer by this sun of York;
And all the clouds that lour'd upon our house
In the deep bosom of the ocean buried.
Now are our brows bound with victorious wreaths;
Our bruised arms hung up for monuments;
Our stern alarums changed to merry meetings,
Our dreadful marches to delightful measures.
Grim-visaged war hath smooth'd his wrinkled front;
And now, instead of mounting barded steeds
To fright the souls of fearful adversaries,
He capers nimbly in a lady's chamber
To the lascivious pleasing of a lute.
"""

   print("Waveforms & serial read/write tests.")

   t5cb = pigpio.callback(GPIO, pigpio.FALLING_EDGE, t5cbf)

   pigpio.set_mode(GPIO, pigpio.OUTPUT)

   e = pigpio.wave_clear()
   CHECK(5, 1, e, 0, 0, "callback, set mode, wave clear")

   wf = []

   wf.append(pigpio.pulse(1<<GPIO, 0,  10000))
   wf.append(pigpio.pulse(0, 1<<GPIO,  30000))
   wf.append(pigpio.pulse(1<<GPIO, 0,  60000))
   wf.append(pigpio.pulse(0, 1<<GPIO, 100000))

   e = pigpio.wave_add_generic(wf)
   CHECK(5, 2, e, 4, 0, "pulse, wave add generic")

   e = pigpio.wave_tx_repeat()
   CHECK(5, 3, e, 9, 0, "wave tx repeat")

   oc = t5_count
   time.sleep(5)
   c = t5_count - oc
   CHECK(5, 4, c, 50, 1, "callback")

   e = pigpio.wave_tx_stop()
   CHECK(5, 5, e, 0, 0, "wave tx stop")

   e = pigpio.serial_read_open(GPIO, BAUD)
   CHECK(5, 6, e, 0, 0, "serial read open")

   pigpio.wave_clear()
   e = pigpio.wave_add_serial(GPIO, BAUD, 5000000, TEXT)
   CHECK(5, 7, e, 3405, 0, "wave clear, wave add serial")

   e = pigpio.wave_tx_start()
   CHECK(5, 8, e, 6811, 0, "wave tx start")

   oc = t5_count
   time.sleep(3)
   c = t5_count - oc
   CHECK(5, 9, c, 0, 0, "callback")

   oc = t5_count
   while pigpio.wave_tx_busy():
      time.sleep(0.1)
   time.sleep(0.1)
   c = t5_count - oc
   CHECK(5, 10, c, 1702, 0, "wave tx busy, callback")

   c, text = pigpio.serial_read(GPIO)
   CHECK(5, 11, TEXT == text, True, 0, "wave tx busy, serial read");

   e = pigpio.serial_read_close(GPIO)
   CHECK(5, 12, e, 0, 0, "serial read close")

   c = pigpio.wave_get_micros()
   CHECK(5, 13, c, 6158704, 0, "wave get micros")

   CHECK(5, 14, 0, 0, 0, "NOT APPLICABLE")

   c = pigpio.wave_get_max_micros()
   CHECK(5, 15, c, 1800000000, 0, "wave get max micros")

   c = pigpio.wave_get_pulses()
   CHECK(5, 16, c, 3405, 0, "wave get pulses")

   CHECK(5, 17, 0, 0, 0, "NOT APPLICABLE")

   c = pigpio.wave_get_max_pulses()
   CHECK(5, 18, c, 12000, 0, "wave get max pulses")

   c = pigpio.wave_get_cbs()
   CHECK(5, 19, c, 6810, 0, "wave get cbs")

   CHECK(5, 20, 0, 0, 0, "NOT APPLICABLE")

   c = pigpio.wave_get_max_cbs()
   CHECK(5, 21, c, 25016, 0, "wave get max cbs")

t6_count=0
t6_on=0
t6_on_tick=None

def t6cbf(gpio, level, tick):
   global t6_count, t6_on, t6_on_tick
   if level == 1:
      t6_on_tick = tick
      t6_count += 1
   else:
      if t6_on_tick is not None:
         t6_on += pigpio.tickDiff(t6_on_tick, tick)

def t6():
   global t6_count, t6_on

   print("Trigger tests.")

   pigpio.write(GPIO, pigpio.LOW)

   tp = 0

   t6cb = pigpio.callback(GPIO, pigpio.EITHER_EDGE, t6cbf)

   for t in range(10):
      time.sleep(0.1)
      p = 10 + (t*10)
      tp += p;
      pigpio.gpio_trigger(4, p, 1)

   time.sleep(0.5)

   CHECK(6, 1, t6_count, 10, 0, "gpio trigger count")

   CHECK(6, 2, t6_on, tp, 25, "gpio trigger pulse length")

t7_count=0

def t7cbf(gpio, level, tick):
   global t7_count
   if level == pigpio.TIMEOUT:
      t7_count += 1

def t7():
   global t7_count

   print("Watchdog tests.")

   # type of edge shouldn't matter for watchdogs
   t7cb = pigpio.callback(GPIO, pigpio.FALLING_EDGE, t7cbf)

   pigpio.set_watchdog(GPIO, 10) # 10 ms, 100 per second
   time.sleep(0.5)
   oc = t7_count
   time.sleep(2)
   c = t7_count - oc
   CHECK(7, 1, c, 200, 1, "set watchdog on count")

   pigpio.set_watchdog(GPIO, 0) # 0 switches watchdog off
   time.sleep(0.5)
   oc = t7_count
   time.sleep(2)
   c = t7_count - oc
   CHECK(7, 2, c, 0, 1, "set watchdog off count")

def t8():
   print("Bank read/write tests.")

   pigpio.write(GPIO, 0)
   v = pigpio.read_bank_1() & (1<<GPIO)
   CHECK(8, 1, v, 0, 0, "read bank 1")

   pigpio.write(GPIO, 1)
   v = pigpio.read_bank_1() & (1<<GPIO)
   CHECK(8, 2, v, (1<<GPIO), 0, "read bank 1")

   pigpio.clear_bank_1(1<<GPIO)
   v = pigpio.read(GPIO)
   CHECK(8, 3, v, 0, 0, "clear bank 1")

   pigpio.set_bank_1(1<<GPIO)
   v = pigpio.read(GPIO)
   CHECK(8, 4, v, 1, 0, "set bank 1")

   t = 0
   v = (1<<16)
   for i in range(100):
      if pigpio.read_bank_2() & v:
         t += 1
   CHECK(8, 5, t, 60, 75, "read bank 2")

   v = pigpio.clear_bank_2(0)
   CHECK(8, 6, v, 0, 0, "clear bank 2")

   pigpio.exceptions = False
   v = pigpio.clear_bank_2(0xffffff)
   pigpio.exceptions = True
   CHECK(8, 7, v, pigpio.PI_SOME_PERMITTED, 0, "clear bank 2")

   v = pigpio.set_bank_2(0)
   CHECK(8, 8, v, 0, 0, "set bank 2")

   pigpio.exceptions = False
   v = pigpio.set_bank_2(0xffffff)
   pigpio.exceptions = True
   CHECK(8, 9, v, pigpio.PI_SOME_PERMITTED, 0, "set bank 2")

def t9():
   print("Script store/run/status/stop/delete tests.")

   pigpio.write(GPIO, 0) # need known state

   # 100 loops per second
   # p0 number of loops
   # p1 GPIO
   script="""
   ldap 0
   ldva 0
   label 0
   w p1 1
   milli 5
   w p1 0
   milli 5
   dcrv 0
   ldav 0
   ldpa 9
   jp 0"""

   t9cb = pigpio.callback(GPIO)

   s = pigpio.store_script(script)
   oc = t9cb.tally()
   pigpio.run_script(s, [99, GPIO])
   time.sleep(2)
   c = t9cb.tally() - oc
   CHECK(9, 1, c, 100, 0, "store/run script")

   oc = t9cb.tally()
   pigpio.run_script(s, [200, GPIO])
   while True:
      e, p = pigpio.script_status(s)
      if e != pigpio.PI_SCRIPT_RUNNING:
         break
      time.sleep(0.5)
   c = t9cb.tally() - oc
   time.sleep(0.1)
   CHECK(9, 2, c, 201, 0, "run script/script status")

   oc = t9cb.tally()
   pigpio.run_script(s, [2000, GPIO])
   while True:
      e, p = pigpio.script_status(s)
      if e != pigpio.PI_SCRIPT_RUNNING:
         break
      if p[9] < 1900:
         pigpio.stop_script(s)
      time.sleep(0.1)
   c = t9cb.tally() - oc
   time.sleep(0.1)
   CHECK(9, 3, c, 110, 10, "run/stop script/script status")

   e = pigpio.delete_script(s)
   CHECK(9, 4, e, 0, 0, "delete script")

if pigpio.start(''): # must run notification test on localhost
   print("Connected to pigpio daemon.")

   test = [t0, t1, t2, t3, t4, t5, t6, t7, t8, t9]

   for t in test:
      t();

   pigpio.stop()

