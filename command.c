/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
*/

/*
This version is for pigpio version 13+
*/

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>

#include "pigpio.h"
#include "command.h"

/* retv
  pigs          pipe
0 ""   <0 ERR   %d
1 ""   <0 ERR   %d
2 %d   <0 ERR   %d
3 %08X          %08X
4 %u            %u
5 HELP          HELP
6 %s   <0 ERR
*/

/* vfyt
 1 cmd
 2 cmd   %d
 3 cmd   %d %d
 4 cmd   %d %x
 6 HELP
 7 cmd   %x
 8 MODES %d %c
 9 PUD   %d %c
10 PROC  %s
11 WVAS  %d %d %d %s
12 PROCR %d [%d]*10
13 WVAG  %d %d %d [%d %d %d]*160


20 cmd m(0-150)
21 cmd v
22 cmd L
23 cmd
24 cmd m1(0-150) m2(0-150)
25 cmd p(0-9)
*/

cmdInfo_t cmdInfo[]=
{
   /* num         str   vfyt retv */

   {PI_CMD_BC1,   "BC1",   7, 1},
   {PI_CMD_BC2,   "BC2",   7, 1},
   {PI_CMD_BR1,   "BR1",   1, 3},
   {PI_CMD_BR2,   "BR2",   1, 3},
   {PI_CMD_BS1,   "BS1",   7, 1},
   {PI_CMD_BS2,   "BS2",   7, 1},
   {PI_CMD_HELP,  "H",     6, 5},
   {PI_CMD_HELP,  "HELP",  6, 5},
   {PI_CMD_HWVER, "HWVER", 1, 4},
   {PI_CMD_MICRO, "MICRO", 21, 0},
   {PI_CMD_MILLI, "MILLI", 21, 0},
   {PI_CMD_MODEG, "MG"   , 2, 2},
   {PI_CMD_MODEG, "MODEG", 2, 2},
   {PI_CMD_MODES, "M",     8, 0},
   {PI_CMD_MODES, "MODES", 8, 0},
   {PI_CMD_NB,    "NB",    4, 0},
   {PI_CMD_NC,    "NC",    2, 0},
   {PI_CMD_NO,    "NO",    1, 2},
   {PI_CMD_NP,    "NP",    2, 0},
   {PI_CMD_PFG,   "PFG",   2, 2},
   {PI_CMD_PFS,   "PFS",   3, 2},
   {PI_CMD_PIGPV, "PIGPV", 1, 4},
   {PI_CMD_PRG,   "PRG",   2, 2},
   {PI_CMD_PROC,  "PROC", 10, 2},
   {PI_CMD_PROCD, "PROCD", 2, 2},
   {PI_CMD_PROCP, "PROCP", 2, 7},
   {PI_CMD_PROCR, "PROCR",12, 2},
   {PI_CMD_PROCS, "PROCS", 2, 2},
   {PI_CMD_PRRG,  "PRRG",  2, 2},
   {PI_CMD_PRS,   "PRS",   3, 2},
   {PI_CMD_PUD,   "PUD",   9, 0},
   {PI_CMD_PWM,   "P",     3, 0},
   {PI_CMD_PWM,   "PWM",   3, 0},
   {PI_CMD_READ,  "R",     2, 2},
   {PI_CMD_READ,  "READ",  2, 2},
   {PI_CMD_SERVO, "S",     3, 0},
   {PI_CMD_SERVO, "SERVO", 3, 0},
   {PI_CMD_SLR,   "SLR",   3, 6},
   {PI_CMD_SLRC,  "SLRC",  2, 2},
   {PI_CMD_SLRO,  "SLRO",  3, 2},
   {PI_CMD_TICK,  "T",     1, 4},
   {PI_CMD_TICK,  "TICK",  1, 4},
   {PI_CMD_TRIG,  "TRIG",  5, 0},
   {PI_CMD_WDOG,  "WDOG",  3, 0},
   {PI_CMD_WRITE, "W",     3, 0},
   {PI_CMD_WRITE, "WRITE", 3, 0},
   {PI_CMD_WVAG,  "WVAG", 13, 2},
   {PI_CMD_WVAS,  "WVAS", 11, 2},
   {PI_CMD_WVBSY, "WVBSY", 1, 2},
   {PI_CMD_WVCLR, "WVCLR", 1, 2},
   {PI_CMD_WVGO,  "WVGO" , 1, 2},
   {PI_CMD_WVGOR, "WVGOR", 1, 2},
   {PI_CMD_WVHLT, "WVHLT", 1, 2},
   {PI_CMD_WVSC,  "WVSC",  2, 2},
   {PI_CMD_WVSM,  "WVSM",  2, 2},
   {PI_CMD_WVSP,  "WVSP",  2, 2},

   {PI_CMD_ADDI , "ADDI" , 21, 0},
   {PI_CMD_ADDV , "ADDV" , 20, 0},
   {PI_CMD_ANDI , "ANDI" , 21, 0},
   {PI_CMD_ANDV , "ANDV" , 20, 0},
   {PI_CMD_CALL , "CALL" , 22, 0},
   {PI_CMD_CMPI , "CMPI" , 21, 0},
   {PI_CMD_CMPV , "CMPV" , 20, 0},
   {PI_CMD_DCRA , "DCRA" , 23, 0},
   {PI_CMD_DCRV , "DCRV" , 20, 0},
   {PI_CMD_HALT , "HALT" , 23, 0},
   {PI_CMD_INRA , "INRA" , 23, 0},
   {PI_CMD_INRV , "INRV" , 20, 0},
   {PI_CMD_JM   , "JM"   , 22, 0},
   {PI_CMD_JMP  , "JMP"  , 22, 0},
   {PI_CMD_JNZ  , "JNZ"  , 22, 0},
   {PI_CMD_JP   , "JP"   , 22, 0},
   {PI_CMD_JZ   , "JZ"   , 22, 0},
   {PI_CMD_LABEL, "LABEL", 22, 0},
   {PI_CMD_LDAI , "LDAI" , 21, 0},
   {PI_CMD_LDAP , "LDAP" , 25, 0},
   {PI_CMD_LDAV , "LDAV" , 20, 0},
   {PI_CMD_LDPA , "LDPA" , 25, 0},
   {PI_CMD_LDVA , "LDVA" , 20, 0},
   {PI_CMD_LDVI , "LDVI" , 28, 0},
   {PI_CMD_LDVV , "LDVV" , 24, 0},
   {PI_CMD_ORI  , "ORI"  , 21, 0},
   {PI_CMD_ORV  , "ORV"  , 20, 0},
   {PI_CMD_POPA , "POPA" , 23, 0},
   {PI_CMD_POPV , "POPV" , 20, 0},
   {PI_CMD_PUSHA, "PUSHA", 23, 0},
   {PI_CMD_PUSHV, "PUSHV", 20, 0},
   {PI_CMD_RET  , "RET"  , 23, 0},
   {PI_CMD_RAL  , "RAL"  , 21, 0},
   {PI_CMD_RAR  , "RAR"  , 21, 0},
   {PI_CMD_SUBI , "SUBI" , 21, 0},
   {PI_CMD_SUBV , "SUBV" , 20, 0},
   {PI_CMD_SWAPA, "SWAPA", 20, 0},
   {PI_CMD_SWAPV, "SWAPV", 24, 0},
   {PI_CMD_SYS  , "SYS"  , 27, 0},
   {PI_CMD_WAITI, "WAITI", 21, 0},
   {PI_CMD_WAITV, "WAITV", 20, 0},
   {PI_CMD_XORI , "XORI" , 21, 0},
   {PI_CMD_XORV , "XORV" , 20, 0},

};

char * cmdUsage = "\
BC1 v         Clear gpios defined by mask v in bank 1.\n\
BC2 v         Clear gpios defined by mask v in bank 2.\n\
BR1           Read gpios bank 1.\n\
BR2           Read gpios bank 2.\n\
BS1 v         Set gpios defined by mask v in bank 1.\n\
BS2 v         Set gpios defined by mask v in bank 2.\n\
H             Displays command help.\n\
HELP          Displays command help.\n\
HWVER         Return hardware version.\n\
M g m         Set gpio g to mode m.\n\
MG g          Get gpio g mode.\n\
MICRO v       Delay for v microseconds.\n\
MILLI v       Delay for v milliseconds.\n\
MODEG g       Get gpio g mode.\n\
MODES g m     Set gpio g to mode m.\n\
NB h v        Start notifications on handle h for gpios defined by mask v.\n\
NC h          Close notification handle h.\n\
NO            Request notification handle.\n\
NP h          Pause notifications on handle h.\n\
P u v         Set PWM value for user gpio u to d.\n\
PFG u         Get PWM frequency for user gpio u.\n\
PFS u v       Set PWM frequency for user gpio u to d.\n\
PIGPV         Return pigpio version.\n\
PRG u         Get PWM range for user gpio u.\n\
PROC t        Store text t of script.\n\
PROCD s       Delete script s.\n\
PROCP s       Get current status and parameter values for script s.\n\
PROCR s pars  Run script s with up to 10 optional parameters.\n\
PROCS s       Stop script s.\n\
PRRG u        Get PWM real range for user gpio u.\n\
PRS u v       Set PWM range for user gpio u to d.\n\
PUD g p       Set gpio pull up/down for gpio g to p.\n\
PWM u v       Set PWM value for user gpio u to d.\n\
R g           Read gpio g.\n\
READ g        Read gpio g.\n\
S u v         Set servo value for user gpio u to d microseconds.\n\
SERVO u v     Set servo value for user gpio u to d microseconds.\n\
SLR u v       Read up to d bytes of serial data from user gpio u.\n\
SLRC u        Close user gpio u for serial data.\n\
SLRO u b      Open user gpio u for serial data at b baud.\n\
T             Return current tick.\n\
TICK          Return current tick.\n\
TRIG u pl L   Trigger level L for pl micros on user gpio u.\n\
W g L         Write level L to gpio g.\n\
WDOG u v      Set watchdog of d milliseconds on user gpio u.\n\
WRITE g L     Write level L to gpio g.\n\
WVAG pulses   Wave add generic pulses.\n\
WVAS u b o t  Wave add serial data t to user gpio u at b baud.\n\
WVBSY         Check if wave busy.\n\
WVCLR         Wave clear.\n\
WVGO          Wave transmit.\n\
WVGOR         Wave transmit repeatedly.\n\
WVHLT         Wave stop.\n\
WVSC ws       Wave get DMA control block stats.\n\
WVSM ws       Wave get micros stats.\n\
WVSP ws       Wave get pulses stats.\n\
\n\
b = baud rate.\n\
g = any gpio (0-53).\n\
h = handle (0-31).\n\
L = level (0-1).\n\
m = mode (RW540123).\n\
mask = a bit mask where (1<<g) is set for each gpio g of interest.\n\
o = offset (0-).\n\
p = pud (ODU).\n\
pars = 0 to 10 parameters for script.\n\
pl = pulse length (0-100).\n\
pulses = 1 or more triplets of gpios on, gpios off, delay.\n\
s = script id (0-31).\n\
t = text.\n\
u = user gpio (0-31).\n\
v = value.\n\
ws = 0=now, 1=high, 2=max.\n\
\n\
Numbers may be entered as hex (prefix 0x), octal (prefix 0),\n\
otherwise they are assumed to be decimal.\n\
";

typedef struct
{
   int error;
   char * str;
} errInfo_t;

static errInfo_t errInfo[]=
{
   {PI_INIT_FAILED      , "pigpio initialisation failed"},
   {PI_BAD_USER_GPIO    , "gpio not 0-31"},
   {PI_BAD_GPIO         , "gpio not 0-53"},
   {PI_BAD_MODE         , "mode not 0-7"},
   {PI_BAD_LEVEL        , "level not 0-1"},
   {PI_BAD_PUD          , "pud not 0-2"},
   {PI_BAD_PULSEWIDTH   , "pulsewidth not 0 or 500-2500"},
   {PI_BAD_DUTYCYCLE    , "dutycycle outside set range"},
   {PI_BAD_TIMER        , "timer not 0-9"},
   {PI_BAD_MS           , "ms not 10-60000"},
   {PI_BAD_TIMETYPE     , "timetype not 0-1"},
   {PI_BAD_SECONDS      , "seconds < 0"},
   {PI_BAD_MICROS       , "micros not 0-999999"},
   {PI_TIMER_FAILED     , "gpioSetTimerFunc failed"},
   {PI_BAD_WDOG_TIMEOUT , "timeout not 0-60000"},
   {PI_NO_ALERT_FUNC    , "DEPRECATED"},
   {PI_BAD_CLK_PERIPH   , "clock peripheral not 0-1"},
   {PI_BAD_CLK_SOURCE   , "clock source not 0-1"},
   {PI_BAD_CLK_MICROS   , "clock micros not 1, 2, 4, 5, 8, or 10"},
   {PI_BAD_BUF_MILLIS   , "buf millis not 100-10000"},
   {PI_BAD_DUTYRANGE    , "dutycycle range not 25-40000"},
   {PI_BAD_SIGNUM       , "signum not 0-63"},
   {PI_BAD_PATHNAME     , "can't open pathname"},
   {PI_NO_HANDLE        , "no handle available"},
   {PI_BAD_HANDLE       , "unknown notify handle"},
   {PI_BAD_IF_FLAGS     , "ifFlags > 3"},
   {PI_BAD_CHANNEL      , "DMA channel not 0-14"},
   {PI_BAD_SOCKET_PORT  , "socket port not 1024-30000"},
   {PI_BAD_FIFO_COMMAND , "unknown fifo command"},
   {PI_BAD_SECO_CHANNEL , "DMA secondary channel not 0-6"},
   {PI_NOT_INITIALISED  , "function called before gpioInitialise"},
   {PI_INITIALISED      , "function called after gpioInitialise"},
   {PI_BAD_WAVE_MODE    , "waveform mode not 0-1"},
   {PI_BAD_CFG_INTERNAL , "bad parameter in gpioCfgInternals call"},
   {PI_BAD_WAVE_BAUD    , "baud rate not 100-250000"},
   {PI_TOO_MANY_PULSES  , "waveform has too many pulses"},
   {PI_TOO_MANY_CHARS   , "waveform has too many chars"},
   {PI_NOT_SERIAL_GPIO  , "no serial read in progress on gpio"},
   {PI_BAD_SERIAL_STRUC , "bad (null) serial structure parameter"},
   {PI_BAD_SERIAL_BUF   , "bad (null) serial buf parameter"}, 
   {PI_NOT_PERMITTED    , "no permission to update gpio"},
   {PI_SOME_PERMITTED   , "no permission to update one or more gpios"},
   {PI_BAD_WVSC_COMMND  , "bad WVSC subcommand"},
   {PI_BAD_WVSM_COMMND  , "bad WVSM subcommand"},
   {PI_BAD_WVSP_COMMND  , "bad WVSP subcommand"},
   {PI_BAD_PULSELEN     , "trigger pulse > 100 microseconds"},
   {PI_BAD_SCRIPT       , "invalid script"},
   {PI_BAD_SCRIPT_ID    , "unknown script id"},
   {PI_BAD_SER_OFFSET   , "add serial data offset > 30 minute"},
   {PI_GPIO_IN_USE      , "gpio already in use"},
   {PI_BAD_SERIAL_COUNT , "must read at least a byte at a time"},
   {PI_BAD_PARAM_NUM    , "script parameter must be 0-9"},
   {PI_DUP_LABEL        , "script has duplicate label"},
   {PI_TOO_MANY_LABELS  , "script has too many labels"},
   {PI_BAD_SCRIPT_CMD   , "illegal script command"},
   {PI_BAD_VAR_NUM      , "script variable must be 0-149"},
   {PI_NO_SCRIPT_ROOM   , "no more room for scripts"},
   {PI_NO_MEMORY        , "can't allocate temporary memory"},
   {PI_SOCK_READ_FAILED , "socket read failed"},
   {PI_SOCK_WRIT_FAILED , "socket write failed"},
   {PI_TOO_MANY_PARAM   , "too many script parameters > 10"},
   {PI_NOT_HALTED       , "script already running or failed"},

};

static char * fmtMdeStr="RW540123";
static char * fmtPudStr="ODU";

static int cmdMatch(char *str)
{
   int i;

   for (i=0; i<(sizeof(cmdInfo)/sizeof(cmdInfo_t)); i++)
   {
      if (strcasecmp(str, cmdInfo[i].name) == 0) return i;
   }
   return -1;
}

static int getNum(char *str, unsigned *val, uint8_t *opt, int flags)
{
   int f, n, v;

   *opt = 0;

   f = sscanf(str, " %i %n", &v, &n);

   if (f == 1)
   {
      *val = v;
      *opt = CMD_NUMERIC;
      return n;
   }

   if (flags & PARSE_FLAGS_PARAMS)
   {
      f = sscanf(str, " p%i %n", &v, &n);

      if (f == 1)
      {
         *val = v;
         *opt = CMD_PARAM;
         return n;
      }
   }

   if (flags & PARSE_FLAGS_VARS)
   {
      f = sscanf(str, " v%i %n", &v, &n);

      if (f == 1)
      {
         *val = v;
         *opt = CMD_VAR;
         return n;
      }
   }

   return 0;
}

static char intCmdStr[32];

char *cmdStr(void)
{
   return intCmdStr;
}

int cmdParse(char *buf, uint32_t *p, void **v, gpioCtlParse_t *ctl)
{
   int f, valid, idx, val, pp, pars, n, n2, i;
   char *ptr;
   char c;
   int param[MAX_PARAM];

   bzero(&ctl->opt, sizeof(ctl->opt));

   sscanf(buf+ctl->eaten, " %31s", intCmdStr);

   p[0] = -1;

   idx = cmdMatch(intCmdStr);

   if (idx < 0) return idx;

   sscanf(buf+ctl->eaten, " %*31s %n", &pp);

   ctl->eaten += pp;

   valid = 0;

   p[0] = cmdInfo[idx].cmd;
   p[1]  = 0;
   p[2]  = 0;

   switch (cmdInfo[idx].vt)
   {
      case 1: /* BR1  BR2  HWVER  NO  PIGPV  TICK  WVBSY  WVCLR  WVGO
                 WVGOR  WVHLT
              */
         valid = 1;
         break;

      case 2: /* MODEG  NC  NP  PFG  PRG  PROCD  PROCS  PRRG
                 SLRC  READ  WVSC  WVSM  WVSP
              */
         ctl->eaten += getNum(buf+ctl->eaten, &p[1], &ctl->opt[0], ctl->flags);
         if (ctl->opt[0]) valid = 1;
         break;

      case 3: /* PFS  PRS  PWM  SERVO  SLR  SLRO  WDOG  WRITE
              */
         ctl->eaten += getNum(buf+ctl->eaten, &p[1], &ctl->opt[0], ctl->flags);
         ctl->eaten += getNum(buf+ctl->eaten, &p[2], &ctl->opt[1], ctl->flags);
         if (ctl->opt[0] && ctl->opt[1]) valid = 1;
         break;

      case 4: /* NB
              */
         ctl->eaten += getNum(buf+ctl->eaten, &p[1], &ctl->opt[0], ctl->flags);
         ctl->eaten += getNum(buf+ctl->eaten, &p[2], &ctl->opt[1], ctl->flags);
         if (ctl->opt[0] && ctl->opt[1]) valid = 1;
         break;

      case 5: /* TRIG
              */
         ctl->eaten += getNum(buf+ctl->eaten, &p[1], &ctl->opt[0], ctl->flags);
         ctl->eaten += getNum(buf+ctl->eaten, &p[2], &ctl->opt[1], ctl->flags);
         ctl->eaten += getNum(buf+ctl->eaten, &p[3], &ctl->opt[2], ctl->flags);
         if (ctl->opt[0] && ctl->opt[1] && ctl->opt[2]) valid = 1;
         break;

      case 6: /* HELP
              */
         valid = 1;
         break;

      case 7: /* BC1  BC2  BS1  BS2
              */
         ctl->eaten += getNum(buf+ctl->eaten, &p[1], &ctl->opt[0], ctl->flags);
         if (ctl->opt[0]) valid = 1;
         break;

      case 8: /* MODES
              */
         ctl->eaten += getNum(buf+ctl->eaten, &p[1], &ctl->opt[0], ctl->flags);
         f = sscanf(buf+ctl->eaten, " %c %n", &c, &n);
         if (ctl->opt[0] && (f >= 1))
         {
            ctl->eaten += n;
            val = toupper(c);
            ptr = strchr(fmtMdeStr, val);
            if (ptr != NULL)
            {
               val = ptr - fmtMdeStr;
               p[2] = val;
               valid = 1;
            }
         }
         break;

      case 9: /* PUD
              */
         ctl->eaten += getNum(buf+ctl->eaten, &p[1], &ctl->opt[0], ctl->flags);
         f = sscanf(buf+ctl->eaten, " %c %n", &c, &n);
         if (ctl->opt[0] && (f >= 1))
         {
            ctl->eaten += n;
            val = toupper(c);
            ptr = strchr(fmtPudStr, val);
            if (ptr != NULL)
            {
               val = ptr - fmtPudStr;
               p[2] = val;
               valid = 1;
            }
         }
         break;

      case 10: /* PROC
               */
         p[1] = strlen(buf+ctl->eaten);
         v[1] = buf+ctl->eaten;
         ctl->eaten += p[1];
         valid = 1;
         break;

      case 11: /* WVAS
               */
         ctl->eaten += getNum(buf+ctl->eaten, &p[1], &ctl->opt[0], ctl->flags);
         ctl->eaten += getNum(buf+ctl->eaten, &p[2], &ctl->opt[1], ctl->flags);
         ctl->eaten += getNum(buf+ctl->eaten, &p[3], &ctl->opt[2], ctl->flags);

         if (ctl->opt[0] && ctl->opt[1] && ctl->opt[2])
         {
            p[4] = strlen(buf+ctl->eaten);
            v[1] = buf+ctl->eaten;
            ctl->eaten += p[4];
            valid = 1;
         }
         break;

      case 12: /* PROCR
               */
         ctl->eaten += getNum(buf+ctl->eaten, &p[1], &ctl->opt[0], 0);

         pars = 0;

         while (pars < 10)
         {
            ctl->eaten += getNum(buf+ctl->eaten, &p[2], &ctl->opt[1], 0);
            if (ctl->opt[1]) param[pars++] = p[2];
            else break;
         }

         p[2] = pars;

         v[1] = param;

         if (ctl->opt[0]) valid = 1;
         break;

      case 13: /* WVAG
               */
         pars = 0;

         while (pars < MAX_PARAM)
         {
            ctl->eaten += getNum(buf+ctl->eaten, &p[1], &ctl->opt[0], 0);
            if (ctl->opt[0]) param[pars++] = p[1];
            else break;
         }

         p[1] = pars/3;

         v[1] = param;

         if (pars && ((pars%3)==0)) valid = 1;
         break;

      case 20: /*
              */
         f = sscanf(buf+ctl->eaten, " %d %n", &p[1], &n);
         if ((f >= 1) && (p[1] >= 0) && (p[1] < MAX_SCRIPT_VARS))
         {
            ctl->eaten += n;
            valid = 1;
         }
         break;

      case 21: /*
              */
         f = sscanf(buf+ctl->eaten, " %d %n", &p[1], &n);
         if (f == 1)
         {
            ctl->eaten += n;
            valid = 1;
         }
         break;

      case 22: /*
              */
         f = sscanf(buf+ctl->eaten, " %d %n", &p[1], &n);
         if (f == 1)
         {
            ctl->eaten += n;
            valid = 1;
         }
         break;

      case 23: /*
              */
         valid = 1;
         break;

      case 24: /*
              */
         f = sscanf(buf+ctl->eaten, " %d %d %n", &p[1], &p[2], &n);
         if ((f >= 2) &&
             (p[1] >= 0) && (p[1] < MAX_SCRIPT_VARS) &&
             (p[2] >= 0) && (p[2] < MAX_SCRIPT_VARS))
         {
            ctl->eaten += n;
            valid = 1;
         }
         break;

      case 25: /*
              */
         f = sscanf(buf+ctl->eaten, " %d %n", &p[1], &n);
         if ((f >= 1) && (p[1] >= 0) && (p[1] < MAX_SCRIPT_PARAMS))
         {
            ctl->eaten += n;
            valid = 1;
         }
         break;

      case 26: /*
              */
         f = sscanf(buf+ctl->eaten, " %d %n", &p[1], &n);
         if (f >= 1)
         {
            ctl->eaten += n;
            valid = 1;
         }
         break;

      case 27: /*
              */
         f = sscanf(buf+ctl->eaten, " %*s%n %n", &n, &n2);
         if ((f >= 0) && n)
         {
            valid = 1;

            for (i=0; i<n; i++)
            {
               c = buf[ctl->eaten+i];

               if ((!isalnum(c)) && (c != '_') && (c != '-'))
               {
                  valid = 0;
                  break;
               }
            }

            if (valid)
            {
               p[1] = n;
               ctl->opt[0] = CMD_NUMERIC;
               v[1]=buf+ctl->eaten;
               ctl->eaten += n2;
            }
         }
         break;

      case 28: /*
              */
         f = sscanf(buf+ctl->eaten, " %d %d %n", &p[1], &p[2], &n);
         if ((f >= 2) && (p[1] >= 0) && (p[1] < MAX_SCRIPT_VARS))
         {
            ctl->eaten += n;
            valid = 1;
         }
         break;
   }

   if (valid) return idx; else return -1;
}

char * cmdErrStr(int error)
{
   int i;

   for (i=0; i<(sizeof(errInfo)/sizeof(errInfo_t)); i++)
   {
      if (errInfo[i].error == error) return errInfo[i].str;
   }
   return "unknown error";
}

