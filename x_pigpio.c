/*
gcc -Wall -pthread -o x_pigpio x_pigpio.c -lpigpio
sudo ./x_pigpio

*** WARNING ************************************************
*                                                          *
* All the tests make extensive use of gpio 25 (pin 22).    *
* Ensure that either nothing or just a LED is connected to *
* gpio 25 before running any of the tests.                 *
*                                                          *
* Some tests are statistical in nature and so may on       *
* occasion fail.  Repeated failures on the same test or    *
* many failures in a group of tests indicate a problem.    *
************************************************************
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include "pigpio.h"

#define USERDATA 18249013

#define GPIO 25

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
   printf("\nTesting pigpio C I/F\n");

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
   gpioDelay(1); /* 1 micro delay to let GPIO reach level reliably */
   v = gpioRead(GPIO);
   CHECK(1, 2, v, 1, 0, "set pull up down, read");

   gpioSetPullUpDown(GPIO, PI_PUD_DOWN);
   gpioDelay(1); /* 1 micro delay to let GPIO reach level reliably */
   v = gpioRead(GPIO);
   CHECK(1, 3, v, 0, 0, "set pull up down, read");

   gpioWrite(GPIO, PI_LOW);
   v = gpioGetMode(GPIO);
   CHECK(1, 4, v, 1, 0, "write, get mode");

   v = gpioRead(GPIO);
   CHECK(1, 5, v, 0, 0, "read");

   gpioWrite(GPIO, PI_HIGH);
   gpioDelay(1); /* 1 micro delay to let GPIO reach level reliably */
   v = gpioRead(GPIO);
   CHECK(1, 6, v, 1, 0, "write, read");
}

int t2_count;

void t2cb(int gpio, int level, uint32_t tick)
{
   t2_count++;
}

void t2()
{
   int dc, f, r, rr, oc;

   printf("PWM dutycycle/range/frequency tests.\n");

   gpioSetPWMrange(GPIO, 255);
   gpioSetPWMfrequency(GPIO, 0);
   f = gpioGetPWMfrequency(GPIO);
   CHECK(2, 1, f, 10, 0, "set PWM range, set/get PWM frequency");

   t2_count=0;

   gpioSetAlertFunc(GPIO, t2cb);

   gpioPWM(GPIO, 0);
   dc = gpioGetPWMdutycycle(GPIO);
   CHECK(2, 2, dc, 0, 0, "get PWM dutycycle");

   time_sleep(0.5); /* allow old notifications to flush */
   oc = t2_count;
   time_sleep(2);
   f = t2_count - oc;
   CHECK(2, 3, f, 0, 0, "set PWM dutycycle, callback");

   gpioPWM(GPIO, 128);
   dc = gpioGetPWMdutycycle(GPIO);
   CHECK(2, 4, dc, 128, 0, "get PWM dutycycle");

   oc = t2_count;
   time_sleep(2);
   f = t2_count - oc;
   CHECK(2, 5, f, 40, 5, "set PWM dutycycle, callback");

   gpioSetPWMfrequency(GPIO, 100);
   f = gpioGetPWMfrequency(GPIO);
   CHECK(2, 6, f, 100, 0, "set/get PWM frequency");

   oc = t2_count;
   time_sleep(2);
   f = t2_count - oc;
   CHECK(2, 7, f, 400, 1, "callback");

   gpioSetPWMfrequency(GPIO, 1000);
   f = gpioGetPWMfrequency(GPIO);
   CHECK(2, 8, f, 1000, 0, "set/get PWM frequency");

   oc = t2_count;
   time_sleep(2);
   f = t2_count - oc;
   CHECK(2, 9, f, 4000, 1, "callback");

   r = gpioGetPWMrange(GPIO);
   CHECK(2, 10, r, 255, 0, "get PWM range");

   rr = gpioGetPWMrealRange(GPIO);
   CHECK(2, 11, rr, 200, 0, "get PWM real range");

   gpioSetPWMrange(GPIO, 2000);
   r = gpioGetPWMrange(GPIO);
   CHECK(2, 12, r, 2000, 0, "set/get PWM range");

   rr = gpioGetPWMrealRange(GPIO);
   CHECK(2, 13, rr, 200, 0, "get PWM real range");

   gpioPWM(GPIO, 0);
}

int t3_val;
int t3_reset;
int t3_count;
uint32_t t3_tick;
float t3_on;
float t3_off;

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

   int t, v;

   int pw[3]={500, 1500, 2500};
   int dc[4]={20, 40, 60, 80};

   printf("PWM/Servo pulse accuracy tests.\n");

   t3_val = USERDATA;
   t3_reset=1;
   t3_count=0;
   t3_tick=0;
   t3_on=0.0;
   t3_off=0.0;

   gpioSetAlertFuncEx(GPIO, t3cbf, &t3_val); /* test extended alert */

   for (t=0; t<3; t++)
   {
      gpioServo(GPIO, pw[t]);
      v = gpioGetServoPulsewidth(GPIO);
      CHECK(3, t+t+1, v, pw[t], 0, "get servo pulsewidth");

      time_sleep(1);
      t3_reset = 1;
      time_sleep(4);
      on = t3_on;
      off = t3_off;
      CHECK(3, t+t+2, (1E3*(on+off))/on, 2E7/pw[t], 1,
          "set servo pulsewidth");
   }

   gpioServo(GPIO, 0);
   gpioSetPWMfrequency(GPIO, 1000);
   f = gpioGetPWMfrequency(GPIO);
   CHECK(3, 7, f, 1000, 0, "set/get PWM frequency");

   rr = gpioSetPWMrange(GPIO, 100);
   CHECK(3, 8, rr, 200, 0, "set PWM range");

   for (t=0; t<4; t++)
   {
      gpioPWM(GPIO, dc[t]);
      v = gpioGetPWMdutycycle(GPIO);
      CHECK(3, t+t+9, v, dc[t], 0, "get PWM dutycycle");

      time_sleep(1);
      t3_reset = 1;
      time_sleep(2);
      on = t3_on;
      off = t3_off;
      CHECK(3, t+t+10, (1E3*on)/(on+off), 1E1*dc[t], 1,
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

   sprintf(p, "/dev/pigpio%d", h);
   f = open(p, O_RDONLY);

   e = gpioNotifyBegin(h, (1<<GPIO));
   CHECK(4, 1, e, 0, 0, "notify open/begin");

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

         if (n) if (l != (r.level&(1<<GPIO))) toggle_ok = 0;

         if (r.level&(1<<GPIO)) l = 0;
         else                   l = (1<<GPIO);
           
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

int t5_count;

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

   int e, oc, c, wid;

   char text[2048];

   printf("Waveforms & serial read/write tests.\n");

   t5_count = 0;

   gpioSetAlertFunc(GPIO, t5cbf);

   gpioSetMode(GPIO, PI_OUTPUT);

   e = gpioWaveClear();
   CHECK(5, 1, e, 0, 0, "callback, set mode, wave clear");

   e = gpioWaveAddGeneric(4, wf);
   CHECK(5, 2, e, 4, 0, "pulse, wave add generic");

   wid = gpioWaveCreate();
   e = gpioWaveTxSend(wid, PI_WAVE_MODE_REPEAT);
   if (e < 14) CHECK(5, 3, e,  9, 0, "wave tx repeat");
   else        CHECK(5, 3, e, 19, 0, "wave tx repeat");

   oc = t5_count;
   time_sleep(5);
   c = t5_count - oc;
   CHECK(5, 4, c, 50, 1, "callback");

   e = gpioWaveTxStop();
   CHECK(5, 5, e, 0, 0, "wave tx stop");

   /* gpioSerialReadOpen changes the alert function */

   e = gpioSerialReadOpen(GPIO, BAUD, 8);
   CHECK(5, 6, e, 0, 0, "serial read open");

   gpioWaveClear();
   e = gpioWaveAddSerial(GPIO, BAUD, 8, 2, 5000000, strlen(TEXT), TEXT);
   CHECK(5, 7, e, 3405, 0, "wave clear, wave add serial");

   wid = gpioWaveCreate();
   e = gpioWaveTxSend(wid, PI_WAVE_MODE_ONE_SHOT);
   if (e < 6964) CHECK(5, 8, e, 6811, 0, "wave tx start");
   else          CHECK(5, 8, e, 7116, 0, "wave tx start");

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
   CHECK(5, 13, c, 6158148, 0, "wave get micros");

   c = gpioWaveGetHighMicros();
   CHECK(5, 14, c, 6158148, 0, "wave get high micros");

   c = gpioWaveGetMaxMicros();
   CHECK(5, 15, c, 1800000000, 0, "wave get max micros");

   c = gpioWaveGetPulses();
   CHECK(5, 16, c, 3405, 0, "wave get pulses");

   c = gpioWaveGetHighPulses();
   CHECK(5, 17, c, 3405, 0, "wave get high pulses");

   c = gpioWaveGetMaxPulses();
   CHECK(5, 18, c, 12000, 0, "wave get max pulses");

   c = gpioWaveGetCbs();
   if (e < 6963) CHECK(5, 19, c, 6810, 0, "wave get cbs");
   else          CHECK(5, 19, c, 7115, 0, "wave get cbs");

   c = gpioWaveGetHighCbs();
   if (e < 6963) CHECK(5, 20, c, 6810, 0, "wave get high cbs");
   else          CHECK(5, 20, c, 7115, 0, "wave get high cbs");

   c = gpioWaveGetMaxCbs();
   CHECK(5, 21, c, 25016, 0, "wave get max cbs");

   /* waveCreatePad tests */
   gpioWaveTxStop();
   gpioWaveClear();
   gpioSetAlertFunc(GPIO, t5cbf);

   e = gpioWaveAddGeneric(2, (gpioPulse_t[])
         {  {1<<GPIO, 0,  10000},
            {0, 1<<GPIO,  30000}
         });
   wid = gpioWaveCreatePad(50, 50, 0);
   CHECK(5, 22, wid, 0, 0, "wave create pad, count==1, wid==");

   e = gpioWaveAddGeneric(4, (gpioPulse_t[])
         {  {1<<GPIO, 0,  10000},
            {0, 1<<GPIO,  30000},
            {1<<GPIO, 0,  60000},
            {0, 1<<GPIO, 100000}
         });
   wid = gpioWaveCreatePad(50, 50, 0);
   CHECK(5, 23, wid, 1, 0, "wave create pad, count==2, wid==");

   c = gpioWaveDelete(0);
   CHECK(5, 24, c, 0, 0, "delete wid==0 success");

   e = gpioWaveAddGeneric(6, (gpioPulse_t[])
         {  {1<<GPIO, 0,  10000},
            {0, 1<<GPIO,  30000},
            {1<<GPIO, 0,  60000},
            {0, 1<<GPIO, 100000},
            {1<<GPIO, 0,  60000},
            {0, 1<<GPIO, 100000}
         });
   c = gpioWaveCreate();
   CHECK(5, 25, c, -67, 0, "No more CBs using wave create");
   wid = gpioWaveCreatePad(50, 50, 0);
   CHECK(5, 26, wid, 0, 0, "wave create pad, count==3, wid==");

   t5_count = 0;
   e = gpioWaveChain((char[]) {1,0}, 2);
   CHECK(5, 27, e,  0, 0, "wave chain [1,0]");
   while (gpioWaveTxBusy()) time_sleep(0.1);
   CHECK(5, 28, t5_count, 5, 1, "callback count==");

   gpioSetAlertFunc(GPIO, NULL);
}

int t6_count;
int t6_on;
uint32_t t6_on_tick;

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

   t6_count=0;
   t6_on=0;
   t6_on_tick=0;

   gpioSetAlertFunc(GPIO, t6cbf);

   for (t=0; t<5; t++)
   {
      time_sleep(0.1);
      p = 10 + (t*10);
      tp += p;
      gpioTrigger(GPIO, p, 1);
   }

   time_sleep(0.2);

   CHECK(6, 1, t6_count, 5, 0, "gpio trigger count");

   CHECK(6, 2, t6_on, tp, 25, "gpio trigger pulse length");
}

int t7_count;

void t7cbf(int gpio, int level, uint32_t tick)
{
   if (level == PI_TIMEOUT) t7_count++;
}

void t7()
{
   int c, oc;

   printf("Watchdog tests.\n");

   t7_count=0;

   /* type of edge shouldn't matter for watchdogs */
   gpioSetAlertFunc(GPIO, t7cbf);

   gpioSetWatchdog(GPIO, 50); /* 50 ms, 20 per second */
   time_sleep(0.5);
   oc = t7_count;
   time_sleep(2);
   c = t7_count - oc;
   CHECK(7, 1, c, 39, 5, "set watchdog on count");

   gpioSetWatchdog(GPIO, 0); /* 0 switches watchdog off */
   time_sleep(0.5);
   oc = t7_count;
   time_sleep(2);
   c = t7_count - oc;
   CHECK(7, 2, c, 0, 1, "set watchdog off count");
}

void t8()
{
   int v;

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

   v = gpioRead_Bits_32_53();

   if (v) v = 0; else v = 1;

   CHECK(8, 5, v, 0, 0, "read bank 2");

   v = gpioWrite_Bits_32_53_Clear(0);
   CHECK(8, 6, v, 0, 0, "clear bank 2");

   CHECK(8, 7, 0, 0, 0, "NOT APPLICABLE");

   v = gpioWrite_Bits_32_53_Set(0);
   CHECK(8, 8, v, 0, 0, "set bank 2");

   CHECK(8, 9, 0, 0, 0, "NOT APPLICABLE");
}

int t9_count;

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
   ld p9 p0\
   tag 0\
   w p1 1\
   mils 5\
   w p1 0\
   mils 5\
   dcr p9\
   jp 0";

   printf("Script store/run/status/stop/delete tests.\n");

   gpioWrite(GPIO, 0); /* need known state */

   t9_count = 0;

   gpioSetAlertFunc(GPIO, t9cbf);

   s = gpioStoreScript(script);

   while (1)
   {
      /* loop until script initialised */
      time_sleep(0.1);
      e = gpioScriptStatus(s, p);
      if (e != PI_SCRIPT_INITING) break;
   }

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

void ta()
{
   int h, b, e;
   char *TEXT;
   char text[2048];

   printf("Serial link tests.\n");

   /* this test needs RXD and TXD to be connected */

   h = serOpen("/dev/ttyAMA0", 57600, 0);

   CHECK(10, 1, h, 0, 0, "serial open");

   b = serRead(h, text, sizeof(text)); /* flush buffer */

   b = serDataAvailable(h);
   CHECK(10, 2, b, 0, 0, "serial data available");

   TEXT = "\
To be, or not to be, that is the question-\
Whether 'tis Nobler in the mind to suffer\
The Slings and Arrows of outrageous Fortune,\
Or to take Arms against a Sea of troubles,\
";
   e = serWrite(h, TEXT, strlen(TEXT));
   CHECK(10, 3, e, 0, 0, "serial write");

   e = serWriteByte(h, 0xAA);
   e = serWriteByte(h, 0x55);
   e = serWriteByte(h, 0x00);
   e = serWriteByte(h, 0xFF);

   CHECK(10, 4, e, 0, 0, "serial write byte");

   time_sleep(0.1); /* allow time for transmission */

   b = serDataAvailable(h);
   CHECK(10, 5, b, strlen(TEXT)+4, 0, "serial data available");

   b = serRead(h, text, strlen(TEXT));
   CHECK(10, 6, b, strlen(TEXT), 0, "serial read");
   if (b >= 0) text[b] = 0;
   CHECK(10, 7, strcmp(TEXT, text), 0, 0, "serial read");

   b = serReadByte(h);
   CHECK(10, 8, b, 0xAA, 0, "serial read byte");

   b = serReadByte(h);
   CHECK(10, 9, b, 0x55, 0, "serial read byte");

   b = serReadByte(h);
   CHECK(10, 10, b, 0x00, 0, "serial read byte");

   b = serReadByte(h);
   CHECK(10, 11, b, 0xFF, 0, "serial read byte");

   b = serDataAvailable(h);
   CHECK(10, 12, b, 0, 0, "serial data availabe");

   e = serClose(h);
   CHECK(10, 13, e, 0, 0, "serial close");
}

void tb()
{
   int h, e, b, len;
   char *exp;
   char buf[128];

   printf("SMBus / I2C tests.");

   /* this test requires an ADXL345 on I2C bus 1 addr 0x53 */

   h = i2cOpen(1, 0x53, 0);
   CHECK(11, 1, h, 0, 0, "i2cOpen");

   e = i2cWriteDevice(h, "\x00", 1); /* move to known register */
   CHECK(11, 2, e, 0, 0, "i2cWriteDevice");

   b = i2cReadDevice(h, buf, 1);
   CHECK(11, 3, b, 1, 0, "i2cReadDevice");
   CHECK(11, 4, buf[0], 0xE5, 0, "i2cReadDevice");

   b = i2cReadByte(h);
   CHECK(11, 5, b, 0xE5, 0, "i2cReadByte");

   b = i2cReadByteData(h, 0);
   CHECK(11, 6, b, 0xE5, 0, "i2cReadByteData");

   b = i2cReadByteData(h, 48);
   CHECK(11, 7, b, 2, 0, "i2cReadByteData");

   exp = "\x1D[aBcDeFgHjKM]";
   len = strlen(exp);

   e = i2cWriteDevice(h, exp, len);
   CHECK(11, 8, e, 0, 0, "i2cWriteDevice");

   e = i2cWriteDevice(h, "\x1D", 1);
   b = i2cReadDevice(h, buf, len-1);
   CHECK(11, 9, b, len-1, 0, "i2cReadDevice");
   CHECK(11, 10, strncmp(buf, exp+1, len-1), 0, 0, "i2cReadDevice");

   if (strncmp(buf, exp+1, len-1))
      printf("got [%.*s] expected [%.*s]\n", len-1, buf, len-1, exp+1);

   e = i2cWriteByteData(h, 0x1d, 0xAA);
   CHECK(11, 11, e, 0, 0, "i2cWriteByteData");

   b = i2cReadByteData(h, 0x1d);
   CHECK(11, 12, b, 0xAA, 0, "i2cReadByteData");

   e = i2cWriteByteData(h, 0x1d, 0x55);
   CHECK(11, 13, e, 0, 0, "i2cWriteByteData");

   b = i2cReadByteData(h, 0x1d);
   CHECK(11, 14, b, 0x55, 0, "i2cReadByteData");

   exp = "[1234567890#]";
   len = strlen(exp);

   e = i2cWriteBlockData(h, 0x1C, exp, len);
   CHECK(11, 15, e, 0, 0, "i2c writeBlockData");

   e = i2cWriteDevice(h, "\x1D", 1);
   b = i2cReadDevice(h, buf, len);
   CHECK(11, 16, b, len, 0, "i2cReadDevice");
   CHECK(11, 17, strncmp(buf, exp, len), 0, 0, "i2cReadDevice");

   if (strncmp(buf, exp, len))
      printf("got [%.*s] expected [%.*s]\n", len, buf, len, exp);

   b = i2cReadI2CBlockData(h, 0x1D, buf, len);
   CHECK(11, 18, b, len, 0, "i2cReadI2CBlockData");
   CHECK(11, 19, strncmp(buf, exp, len), 0, 0, "i2cReadI2CBlockData");

   if (strncmp(buf, exp, len))
      printf("got [%.*s] expected [%.*s]\n", len, buf, len, exp);

   exp = "(-+=;:,<>!%)";
   len = strlen(exp);

   e = i2cWriteI2CBlockData(h, 0x1D, exp, len);
   CHECK(11, 20, e, 0, 0, "i2cWriteI2CBlockData");

   b = i2cReadI2CBlockData(h, 0x1D, buf, len);
   CHECK(11, 21, b, len, 0, "i2cReadI2CBlockData");
   CHECK(11, 22, strncmp(buf, exp, len), 0, 0, "i2cReadI2CBlockData");

   if (strncmp(buf, exp, len))
      printf("got [%.*s] expected [%.*s]\n", len, buf, len, exp);

   e = i2cClose(h);
   CHECK(11, 23, e, 0, 0, "i2cClose");
}

void tc()
{
   int h, x, b, e;
   char txBuf[8], rxBuf[8];

   printf("SPI tests.");

   /* this test requires a MCP3202 on SPI channel 1 */

   h = spiOpen(1, 50000, 0);
   CHECK(12, 1, h, 0, 0, "spiOpen");

   sprintf(txBuf, "\x01\x80");

   for (x=0; x<5; x++)
   {
      b = spiXfer(h, txBuf, rxBuf, 3);
      CHECK(12, 2, b, 3, 0, "spiXfer");
      if (b == 3)
      {
         time_sleep(1.0);
         printf("%d ", ((rxBuf[1]&0x0F)*256)|rxBuf[2]);
      }
   }

   e = spiClose(h);
   CHECK(12, 99, e, 0, 0, "spiClose");
}

int main(int argc, char *argv[])
{
   int i, t, c, status;

   char test[64]={0,};

   if (argc > 1)
   {
      t = 0;

      for (i=0; i<strlen(argv[1]); i++)
      {
         c = tolower(argv[1][i]);

         if (!strchr(test, c))
         {
            test[t++] = c;
            test[t] = 0;
         }
      }
   }
   else strcat(test, "0123456789");

   status = gpioInitialise();

   if (status < 0)
   {
      fprintf(stderr, "pigpio initialisation failed.\n");
      return 1;
   }

   if (strchr(test, '0')) t0();
   if (strchr(test, '1')) t1();
   if (strchr(test, '2')) t2();
   if (strchr(test, '3')) t3();
   if (strchr(test, '4')) t4();
   if (strchr(test, '5')) t5();
   if (strchr(test, '6')) t6();
   if (strchr(test, '7')) t7();
   if (strchr(test, '8')) t8();
   if (strchr(test, '9')) t9();
   if (strchr(test, 'a')) ta();
   if (strchr(test, 'b')) tb();
   if (strchr(test, 'c')) tc();

   gpioTerminate();

   return 0;
}

