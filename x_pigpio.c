/*
gcc -o x_pigpio x_pigpio.c -lpigpio -lrt -lpthread
sudo ./x_pigpio
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "pigpio.h"

#define USERDATA 18249013

#define GPIO 4

void CHECK(int t, int st, int got, int expect, int pc, char *desc)
{
   if ((got >= (((1E2-pc)*expect)/1E2)) && (got <= (((1E2+pc)*expect)/1E2)))
   {
      printf("TEST %2d.%-2d PASS (%s: %d)\n", t, st, desc, expect);
   }
   else
   {
      fprintf(stderr,
              "TEST %2d.%-2d FAILED got %d (%s: %d)\n",
              t, st, got, desc, expect);
   }
}

void t0()
{
   printf("Version.\n");

   printf("pigpio version %d.\n", gpioVersion());

   printf("Hardware revision %d.\n", gpioHardwareRevision());
}

void t1()
{
   int v;

   printf("Mode/PUD/read/write tests.\n");

   gpioSetMode(GPIO, PI_INPUT);
   v = gpioGetMode(GPIO);
   CHECK(1, 1, v, 0, 0, "set mode, get mode");

   gpioSetPullUpDown(GPIO, PI_PUD_UP);
   v = gpioRead(GPIO);
   CHECK(1, 2, v, 1, 0, "set pull up down, read");

   gpioSetPullUpDown(GPIO, PI_PUD_DOWN);
   v = gpioRead(GPIO);
   CHECK(1, 3, v, 0, 0, "set pull up down, read");

   gpioWrite(GPIO, PI_LOW);
   v = gpioGetMode(GPIO);
   CHECK(1, 4, v, 1, 0, "write, get mode");

   v = gpioRead(GPIO);
   CHECK(1, 5, v, 0, 0, "read");

   gpioWrite(GPIO, PI_HIGH);
   v = gpioRead(GPIO);
   CHECK(1, 6, v, 1, 0, "write, read");
}

int t2_count=0;

void t2cb(int gpio, int level, uint32_t tick)
{
   t2_count++;
}

void t2()
{
   int f, r, rr, oc;

   printf("PWM dutycycle/range/frequency tests.\n");

   gpioSetPWMrange(GPIO, 255);
   gpioSetPWMfrequency(GPIO, 0);
   f = gpioGetPWMfrequency(GPIO);
   CHECK(2, 1, f, 10, 0, "set PWM range, set/get PWM frequency");

   gpioSetAlertFunc(GPIO, t2cb);

   gpioPWM(GPIO, 0);
   time_sleep(0.5); /* allow old notifications to flush */
   oc = t2_count;
   time_sleep(2);
   f = t2_count - oc;
   CHECK(2, 2, f, 0, 0, "set PWM dutycycle, callback");

   gpioPWM(GPIO, 128);
   oc = t2_count;
   time_sleep(2);
   f = t2_count - oc;
   CHECK(2, 3, f, 40, 5, "set PWM dutycycle, callback");

   gpioSetPWMfrequency(GPIO, 100);
   f = gpioGetPWMfrequency(GPIO);
   CHECK(2, 4, f, 100, 0, "set/get PWM frequency");

   oc = t2_count;
   time_sleep(2);
   f = t2_count - oc;
   CHECK(2, 5, f, 400, 1, "callback");

   gpioSetPWMfrequency(GPIO, 1000);
   f = gpioGetPWMfrequency(GPIO);
   CHECK(2, 6, f, 1000, 0, "set/get PWM frequency");

   oc = t2_count;
   time_sleep(2);
   f = t2_count - oc;
   CHECK(2, 7, f, 4000, 1, "callback");

   r = gpioGetPWMrange(GPIO);
   CHECK(2, 8, r, 255, 0, "get PWM range");

   rr = gpioGetPWMrealRange(GPIO);
   CHECK(2, 9, rr, 200, 0, "get PWM real range");

   gpioSetPWMrange(GPIO, 2000);
   r = gpioGetPWMrange(GPIO);
   CHECK(2, 10, r, 2000, 0, "set/get PWM range");

   rr = gpioGetPWMrealRange(GPIO);
   CHECK(2, 11, rr, 200, 0, "get PWM real range");

   gpioPWM(GPIO, 0);
}

int t3_val = USERDATA;
int t3_reset=1;
int t3_count=0;
uint32_t t3_tick=0;
float t3_on=0.0;
float t3_off=0.0;

void t3cbf(int gpio, int level, uint32_t tick, void *userdata)
{
   static int unreported = 1;

   uint32_t td;
   int *val;

   val = userdata;

   if (*val != USERDATA)
   {
      if (unreported)
      {
         fprintf
         (
            stderr,
            "unexpected userdata %d (expected %d)\n",
            *val, USERDATA
         );
      }
      unreported = 0;
   }

   if (t3_reset)
   {
      t3_count = 0;
      t3_on = 0.0;
      t3_off = 0.0;
      t3_reset = 0;
   }
   else
   {
      td = tick - t3_tick;

      if (level == 0) t3_on += td;
      else            t3_off += td;
   }

   t3_count ++;
   t3_tick = tick;
}

void t3()
{
   int f, rr;

   float on, off;

   int t;

   int pw[3]={500, 1500, 2500};
   int dc[4]={20, 40, 60, 80};

   printf("PWM/Servo pulse accuracy tests.\n");

   gpioSetAlertFuncEx(GPIO, t3cbf, &t3_val); /* test extended alert */

   for (t=0; t<3; t++)
   {
      gpioServo(GPIO, pw[t]);
      time_sleep(1);
      t3_reset = 1;
      time_sleep(4);
      on = t3_on;
      off = t3_off;
      CHECK(3, 1+t, (1E3*(on+off))/on, 2E7/pw[t], 1,
          "set servo pulsewidth");
   }

   gpioServo(GPIO, 0);
   gpioSetPWMfrequency(GPIO, 1000);
   f = gpioGetPWMfrequency(GPIO);
   CHECK(3, 4, f, 1000, 0, "set/get PWM frequency");

   rr = gpioSetPWMrange(GPIO, 100);
   CHECK(3, 5, rr, 200, 0, "set PWM range");

   for (t=0; t<4; t++)
   {
      gpioPWM(GPIO, dc[t]);
      time_sleep(1);
      t3_reset = 1;
      time_sleep(2);
      on = t3_on;
      off = t3_off;
      CHECK(3, 6+t, (1E3*on)/(on+off), 1E1*dc[t], 1,
         "set PWM dutycycle");
   }

   gpioPWM(GPIO, 0);
}

void t4()
{
   int h, e, f, n, s, b, l, seq_ok, toggle_ok;
   gpioReport_t r;
   char p[32];

   printf("Pipe notification tests.\n");

   gpioSetPWMfrequency(GPIO, 0);
   gpioPWM(GPIO, 0);
   gpioSetPWMrange(GPIO, 100);

   h = gpioNotifyOpen();
   e = gpioNotifyBegin(h, (1<<4));
   CHECK(4, 1, e, 0, 0, "notify open/begin");

   time_sleep(1);

   sprintf(p, "/dev/pigpio%d", h);

   f = open(p, O_RDONLY);

   gpioPWM(GPIO, 50);
   time_sleep(4);
   gpioPWM(GPIO, 0);

   e = gpioNotifyPause(h);
   CHECK(4, 2, e, 0, 0, "notify pause");

   e = gpioNotifyClose(h);
   CHECK(4, 3, e, 0, 0, "notify close");

   n = 0;
   s = 0;
   l = 0;
   seq_ok = 1;
   toggle_ok = 1;

   while (1)
   {
      b = read(f, &r, 12);
      if (b == 12)
      {
         if (s != r.seqno) seq_ok = 0;

         if (n) if (l != (r.level&(1<<4))) toggle_ok = 0;

         if (r.level&(1<<4)) l = 0;
         else                l = (1<<4);
           
         s++;
         n++;

         // printf("%d %d %d %X\n", r.seqno, r.flags, r.tick, r.level);
      }
      else break;
   }

   close(f);

   CHECK(4, 4, seq_ok, 1, 0, "sequence numbers ok");

   CHECK(4, 5, toggle_ok, 1, 0, "gpio toggled ok");

   CHECK(4, 6, n, 80, 10, "number of notifications");
}

int t5_count = 0;

void t5cbf(int gpio, int level, uint32_t tick)
{
   if (level == 0) t5_count++; /* falling edges */
}

void t5()
{
   int BAUD=4800;

   char *TEXT=
"\n\
Now is the winter of our discontent\n\
Made glorious summer by this sun of York;\n\
And all the clouds that lour'd upon our house\n\
In the deep bosom of the ocean buried.\n\
Now are our brows bound with victorious wreaths;\n\
Our bruised arms hung up for monuments;\n\
Our stern alarums changed to merry meetings,\n\
Our dreadful marches to delightful measures.\n\
Grim-visaged war hath smooth'd his wrinkled front;\n\
And now, instead of mounting barded steeds\n\
To fright the souls of fearful adversaries,\n\
He capers nimbly in a lady's chamber\n\
To the lascivious pleasing of a lute.\n\
";

   gpioPulse_t wf[] =
   {
      {1<<GPIO, 0,  10000},
      {0, 1<<GPIO,  30000},
      {1<<GPIO, 0,  60000},
      {0, 1<<GPIO, 100000},
   };

   int e, oc, c;

   char text[2048];

   printf("Waveforms & serial read/write tests.\n");

   gpioSetAlertFunc(GPIO, t5cbf);

   gpioSetMode(GPIO, PI_OUTPUT);

   e = gpioWaveClear();
   CHECK(5, 1, e, 0, 0, "callback, set mode, wave clear");

   e = gpioWaveAddGeneric(4, wf);
   CHECK(5, 2, e, 4, 0, "pulse, wave add generic");

   e = gpioWaveTxStart(PI_WAVE_MODE_REPEAT);
   CHECK(5, 3, e, 9, 0, "wave tx repeat");

   oc = t5_count;
   time_sleep(5);
   c = t5_count - oc;
   CHECK(5, 4, c, 50, 1, "callback");

   e = gpioWaveTxStop();
   CHECK(5, 5, e, 0, 0, "wave tx stop");

   /* gpioSerialReadOpen changes the alert function */

   e = gpioSerialReadOpen(GPIO, BAUD);
   CHECK(5, 6, e, 0, 0, "serial read open");

   gpioWaveClear();
   e = gpioWaveAddSerial(GPIO, BAUD, 5000000, strlen(TEXT), TEXT);
   CHECK(5, 7, e, 3405, 0, "wave clear, wave add serial");

   e = gpioWaveTxStart(PI_WAVE_MODE_ONE_SHOT);
   CHECK(5, 8, e, 6811, 0, "wave tx start");

   CHECK(5, 9, 0, 0, 0, "NOT APPLICABLE");

   CHECK(5, 10, 0, 0, 0, "NOT APPLICABLE");

   while (gpioWaveTxBusy()) time_sleep(0.1);
   time_sleep(0.1);
   c = gpioSerialRead(GPIO, text, sizeof(text)-1);
   if (c > 0) text[c] = 0;
   CHECK(5, 11, strcmp(TEXT, text), 0, 0, "wave tx busy, serial read");

   e = gpioSerialReadClose(GPIO);
   CHECK(5, 12, e, 0, 0, "serial read close");

   c = gpioWaveGetMicros();
   CHECK(5, 13, c, 6158704, 0, "wave get micros");

   c = gpioWaveGetHighMicros();
   CHECK(5, 14, c, 6158704, 0, "wave get high micros");

   c = gpioWaveGetMaxMicros();
   CHECK(5, 15, c, 1800000000, 0, "wave get max micros");

   c = gpioWaveGetPulses();
   CHECK(5, 16, c, 3405, 0, "wave get pulses");

   c = gpioWaveGetHighPulses();
   CHECK(5, 17, c, 3405, 0, "wave get high pulses");

   c = gpioWaveGetMaxPulses();
   CHECK(5, 18, c, 12000, 0, "wave get max pulses");

   c = gpioWaveGetCbs();
   CHECK(5, 19, c, 6810, 0, "wave get cbs");

   c = gpioWaveGetHighCbs();
   CHECK(5, 20, c, 6810, 0, "wave get high cbs");

   c = gpioWaveGetMaxCbs();
   CHECK(5, 21, c, 25016, 0, "wave get max cbs");
}

int t6_count=0;
int t6_on=0;
uint32_t t6_on_tick=0;

void t6cbf(int gpio, int level, uint32_t tick)
{
   if (level == 1)
   {
      t6_on_tick = tick;
      t6_count++;
   }
   else
   {
      if (t6_on_tick) t6_on += (tick - t6_on_tick);
   }
}

void t6()
{
   int tp, t, p;

   printf("Trigger tests\n");

   gpioWrite(GPIO, PI_LOW);

   tp = 0;

   gpioSetAlertFunc(GPIO, t6cbf);

   for (t=0; t<10; t++)
   {
      time_sleep(0.1);
      p = 10 + (t*10);
      tp += p;
      gpioTrigger(4, p, 1);
   }

   time_sleep(0.2);

   CHECK(6, 1, t6_count, 10, 0, "gpio trigger count");

   CHECK(6, 2, t6_on, tp, 25, "gpio trigger pulse length");
}

int t7_count=0;

void t7cbf(int gpio, int level, uint32_t tick)
{
   if (level == PI_TIMEOUT) t7_count++;
}

void t7()
{
   int c, oc;

   printf("Watchdog tests.\n");

   /* type of edge shouldn't matter for watchdogs */
   gpioSetAlertFunc(GPIO, t7cbf);

   gpioSetWatchdog(GPIO, 10); /* 10 ms, 100 per second */
   time_sleep(0.5);
   oc = t7_count;
   time_sleep(2);
   c = t7_count - oc;
   CHECK(7, 1, c, 200, 1, "set watchdog on count");

   gpioSetWatchdog(GPIO, 0); /* 0 switches watchdog off */
   time_sleep(0.5);
   oc = t7_count;
   time_sleep(2);
   c = t7_count - oc;
   CHECK(7, 2, c, 0, 1, "set watchdog off count");
}

void t8()
{
   int v, t, i;

   printf("Bank read/write tests.\n");

   gpioWrite(GPIO, 0);
   v = gpioRead_Bits_0_31() & (1<<GPIO);
   CHECK(8, 1, v, 0, 0, "read bank 1");

   gpioWrite(GPIO, 1);
   v = gpioRead_Bits_0_31() & (1<<GPIO);
   CHECK(8, 2, v, (1<<GPIO), 0, "read bank 1");

   gpioWrite_Bits_0_31_Clear(1<<GPIO);
   v = gpioRead(GPIO);
   CHECK(8, 3, v, 0, 0, "clear bank 1");

   gpioWrite_Bits_0_31_Set(1<<GPIO);
   v = gpioRead(GPIO);
   CHECK(8, 4, v, 1, 0, "set bank 1");

   t = 0;
   v = (1<<16);
   for (i=0; i<100; i++)
   {
      if (gpioRead_Bits_32_53() & v) t++;
   };
   CHECK(8, 5, t, 60, 75, "read bank 2");

   v = gpioWrite_Bits_32_53_Clear(0);
   CHECK(8, 6, v, 0, 0, "clear bank 2");

   CHECK(8, 7, 0, 0, 0, "NOT APPLICABLE");

   v = gpioWrite_Bits_32_53_Set(0);
   CHECK(8, 8, v, 0, 0, "set bank 2");

   CHECK(8, 9, 0, 0, 0, "NOT APPLICABLE");
}

int t9_count = 0;

void t9cbf(int gpio, int level, uint32_t tick)
{
   if (level == 1) t9_count++;
}

void t9()
{
   int s, oc, c, e;
   uint32_t p[10];

   /*
   100 loops per second
   p0 number of loops
   p1 GPIO
   */
   char *script="\
   ldap 0\
   ldva 0\
   label 0\
   w p1 1\
   milli 5\
   w p1 0\
   milli 5\
   dcrv 0\
   ldav 0\
   ldpa 9\
   jp 0";

   printf("Script store/run/status/stop/delete tests.\n");

   gpioWrite(GPIO, 0); /* need known state */

   gpioSetAlertFunc(GPIO, t9cbf);

   s = gpioStoreScript(script);
   time_sleep(0.1);
   oc = t9_count;
   p[0] = 99;
   p[1] = GPIO;
   gpioRunScript(s, 2, p);
   time_sleep(2);
   c = t9_count - oc;
   CHECK(9, 1, c, 100, 0, "store/run script");

   oc = t9_count;
   p[0] = 200;
   p[1] = GPIO;
   gpioRunScript(s, 2, p);
   time_sleep(0.1);
   while (1)
   {
      e = gpioScriptStatus(s, p);
      if (e != PI_SCRIPT_RUNNING) break;
      time_sleep(0.5);
   }
   c = t9_count - oc;
   time_sleep(0.1);
   CHECK(9, 2, c, 201, 0, "run script/script status");

   oc = t9_count;
   p[0] = 2000;
   p[1] = GPIO;
   gpioRunScript(s, 2, p);
   time_sleep(0.1);
   while (1)
   {
      e = gpioScriptStatus(s, p);
      if (e != PI_SCRIPT_RUNNING) break;
      if (p[9] < 1900) gpioStopScript(s);
      time_sleep(0.1);
   }
   c = t9_count - oc;
   time_sleep(0.1);
   CHECK(9, 3, c, 110, 10, "run/stop script/script status");

   e = gpioDeleteScript(s);
   CHECK(9, 4, e, 0, 0, "delete script");
}

int main(int argc, char *argv[])
{
   int t, status;
   void (*test[])(void) = {t0, t1, t2, t3, t4, t5, t6, t7, t8, t9};

   status = gpioInitialise();

   if (status < 0)
   {
      fprintf(stderr, "pigpio initialisation failed.\n");
      return 1;
   }

   for (t=0; t<10; t++) test[t]();

   gpioTerminate();

   return 0;
}

