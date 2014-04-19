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
This version is for pigpio version 14+
*/

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>

#include "pigpio.h"
#include "command.h"

cmdInfo_t cmdInfo[]=
{
   /* num         str   vfyt retv */

   {PI_CMD_BC1,   "BC1",   111, 1},
   {PI_CMD_BC2,   "BC2",   111, 1},
   {PI_CMD_BR1,   "BR1",   101, 3},
   {PI_CMD_BR2,   "BR2",   101, 3},
   {PI_CMD_BS1,   "BS1",   111, 1},
   {PI_CMD_BS2,   "BS2",   111, 1},
   {PI_CMD_HELP,  "H",     101, 5},
   {PI_CMD_HELP,  "HELP",  101, 5},
   {PI_CMD_HWVER, "HWVER", 101, 4},
   {PI_CMD_MICS,  "MICS",  112, 0},
   {PI_CMD_MILS,  "MILS",  112, 0},
   {PI_CMD_MODEG, "MG"   , 112, 2},
   {PI_CMD_MODEG, "MODEG", 112, 2},
   {PI_CMD_MODES, "M",     125, 0},
   {PI_CMD_MODES, "MODES", 125, 0},
   {PI_CMD_NB,    "NB",    122, 0},
   {PI_CMD_NC,    "NC",    112, 0},
   {PI_CMD_NO,    "NO",    101, 2},
   {PI_CMD_NP,    "NP",    112, 0},
   {PI_CMD_PARSE, "PARSE", 115, 2},
   {PI_CMD_PFG,   "PFG",   112, 2},
   {PI_CMD_PFS,   "PFS",   121, 2},
   {PI_CMD_PIGPV, "PIGPV", 101, 4},
   {PI_CMD_PRG,   "PRG",   112, 2},
   {PI_CMD_PROC,  "PROC",  115, 2},
   {PI_CMD_PROCD, "PROCD", 112, 2},
   {PI_CMD_PROCP, "PROCP", 112, 7},
   {PI_CMD_PROCR, "PROCR", 191, 2},
   {PI_CMD_PROCS, "PROCS", 112, 2},
   {PI_CMD_PRRG,  "PRRG",  112, 2},
   {PI_CMD_PRS,   "PRS",   121, 2},
   {PI_CMD_PUD,   "PUD",   126, 0},
   {PI_CMD_PWM,   "P",     121, 0},
   {PI_CMD_PWM,   "PWM",   121, 0},
   {PI_CMD_READ,  "R",     112, 2},
   {PI_CMD_READ,  "READ",  112, 2},
   {PI_CMD_SERVO, "S",     121, 0},
   {PI_CMD_SERVO, "SERVO", 121, 0},
   {PI_CMD_SLR,   "SLR",   121, 6},
   {PI_CMD_SLRC,  "SLRC",  112, 2},
   {PI_CMD_SLRO,  "SLRO",  121, 2},
   {PI_CMD_TICK,  "T",     101, 4},
   {PI_CMD_TICK,  "TICK",  101, 4},
   {PI_CMD_TRIG,  "TRIG",  131, 0},
   {PI_CMD_WDOG,  "WDOG",  121, 0},
   {PI_CMD_WRITE, "W",     121, 0},
   {PI_CMD_WRITE, "WRITE", 121, 0},
   {PI_CMD_WVAG,  "WVAG",  192, 2},
   {PI_CMD_WVAS,  "WVAS",  141, 2},
   {PI_CMD_WVBSY, "WVBSY", 101, 2},
   {PI_CMD_WVCLR, "WVCLR", 101, 2},
   {PI_CMD_WVCRE, "WVCRE", 101, 2},
   {PI_CMD_WVDEL, "WVDEL", 112, 2},
   {PI_CMD_WVGO,  "WVGO" , 101, 2},
   {PI_CMD_WVGOR, "WVGOR", 101, 2},
   {PI_CMD_WVHLT, "WVHLT", 101, 2},
   {PI_CMD_WVNEW, "WVNEW", 101, 2},
   {PI_CMD_WVSC,  "WVSC",  112, 2},
   {PI_CMD_WVSM,  "WVSM",  112, 2},
   {PI_CMD_WVSP,  "WVSP",  112, 2},
   {PI_CMD_WVTX,  "WVTX",  112, 2},
   {PI_CMD_WVTXR, "WVTXR", 112, 2},

   {PI_CMD_ADD  , "ADD"  , 111, 0},
   {PI_CMD_AND  , "AND"  , 111, 0},
   {PI_CMD_CALL , "CALL" , 114, 0},
   {PI_CMD_CMP  , "CMP"  , 111, 0},
   {PI_CMD_DCR  , "DCR"  , 113, 0},
   {PI_CMD_DCRA , "DCRA" , 101, 0},
   {PI_CMD_DIV  , "DIV"  , 111, 0},
   {PI_CMD_HALT , "HALT" , 101, 0},
   {PI_CMD_INR  , "INR"  , 113, 0},
   {PI_CMD_INRA , "INRA" , 101, 0},
   {PI_CMD_JM   , "JM"   , 114, 0},
   {PI_CMD_JMP  , "JMP"  , 114, 0},
   {PI_CMD_JNZ  , "JNZ"  , 114, 0},
   {PI_CMD_JP   , "JP"   , 114, 0},
   {PI_CMD_JZ   , "JZ"   , 114, 0},
   {PI_CMD_LD   , "LD"   , 123, 0},
   {PI_CMD_LDA  , "LDA"  , 111, 0},
   {PI_CMD_MLT  , "MLT"  , 111, 0},
   {PI_CMD_MOD  , "MOD"  , 111, 0},
   {PI_CMD_OR   , "OR"   , 111, 0},
   {PI_CMD_POP  , "POP"  , 113, 0},
   {PI_CMD_POPA , "POPA" , 101, 0},
   {PI_CMD_PUSH , "PUSH" , 113, 0},
   {PI_CMD_PUSHA, "PUSHA", 101, 0},
   {PI_CMD_RET  , "RET"  , 101, 0},
   {PI_CMD_RL   , "RL"   , 123, 0},
   {PI_CMD_RLA  , "RLA"  , 111, 0},
   {PI_CMD_RR   , "RR"   , 123, 0},
   {PI_CMD_RRA  , "RRA"  , 111, 0},
   {PI_CMD_STA  , "STA"  , 113, 0},
   {PI_CMD_SUB  , "SUB"  , 111, 0},
   {PI_CMD_SYS  , "SYS"  , 116, 0},
   {PI_CMD_TAG  , "TAG"  , 114, 0},
   {PI_CMD_WAIT , "WAIT" , 111, 0},
   {PI_CMD_X    , "X"    , 124, 0},
   {PI_CMD_XA   , "XA"   , 113, 0},
   {PI_CMD_XOR  , "XOR"  , 111, 0},

};

char * cmdUsage = "\
BC1 v         Clear gpios specified by mask v in bank 1.\n\
BC2 v         Clear gpios specified by mask v in bank 2.\n\
BR1           Read gpios bank 1.\n\
BR2           Read gpios bank 2.\n\
BS1 v         Set gpios specified by mask v in bank 1.\n\
BS2 v         Set gpios specified by mask v in bank 2.\n\
H             Displays command help.\n\
HELP          Displays command help.\n\
HWVER         Return hardware version.\n\
M g m         Set gpio g to mode m.\n\
MG g          Get gpio g mode.\n\
MICS v        Delay for v microseconds.\n\
MILS v        Delay for v milliseconds.\n\
MODEG g       Get gpio g mode.\n\
MODES g m     Set gpio g to mode m.\n\
NB h v        Start notifications on handle h for gpios specified by mask v.\n\
NC h          Close notification handle h.\n\
NO            Request notification handle.\n\
NP h          Pause notifications on handle h.\n\
P u v         Set PWM value for user gpio u to v.\n\
PARSE t       Validate script text t without storing.\n\
PFG u         Get PWM frequency for user gpio u.\n\
PFS u v       Set PWM frequency for user gpio u to v.\n\
PIGPV         Return pigpio version.\n\
PRG u         Get PWM range for user gpio u.\n\
PROC t        Store text t of script.\n\
PROCD s       Delete script s.\n\
PROCP s       Get current status and parameter values for script s.\n\
PROCR s pars  Run script s with up to 10 optional parameters.\n\
PROCS s       Stop script s.\n\
PRRG u        Get PWM real range for user gpio u.\n\
PRS u v       Set PWM range for user gpio u to v.\n\
PUD g p       Set gpio pull up/down for gpio g to p.\n\
PWM u v       Set PWM value for user gpio u to v.\n\
R g           Read gpio g.\n\
READ g        Read gpio g.\n\
S u v         Set servo value for user gpio u to v microseconds.\n\
SERVO u v     Set servo value for user gpio u to v microseconds.\n\
SLR u v       Read up to d bytes of serial data from user gpio u.\n\
SLRC u        Close user gpio u for serial data.\n\
SLRO u b      Open user gpio u for serial data at b baud.\n\
T             Return current tick.\n\
TICK          Return current tick.\n\
TRIG u pl L   Trigger level L for pl micros on user gpio u.\n\
W g L         Write level L to gpio g.\n\
WDOG u v      Set watchdog of v milliseconds on user gpio u.\n\
WRITE g L     Write level L to gpio g.\n\
WVAG pulses   Wave add generic pulses.\n\
WVAS u b o t  Wave add serial data t to user gpio u at b baud.\n\
WVBSY         Check if wave busy.\n\
WVCLR         Wave clear.\n\
WVCRE         Create wave from added pulses.\n\
WVDEL w       Delete waves w and higher.\n\
WVGO          Wave transmit (DEPRECATED).\n\
WVGOR         Wave transmit repeatedly (DEPRECATED).\n\
WVHLT         Wave stop.\n\
WVNEW         Start a new empty wave.\n\
WVSC ws       Wave get DMA control block stats.\n\
WVSM ws       Wave get micros stats.\n\
WVSP ws       Wave get pulses stats.\n\
WVTX w        Transmit wave w as one-shot.\n\
WVTXR w       Transmit wave w repeatedly.\n\
\n\
b = baud rate.\n\
g = any gpio (0-53).\n\
h = handle (>=0).\n\
L = level (0-1).\n\
m = mode (RW540123).\n\
mask = a bit mask where (1<<g) is set for each gpio g of interest.\n\
o = offset (>=0).\n\
p = pud (ODU).\n\
pars = 0 to 10 parameters for script.\n\
pl = pulse length (1-50).\n\
pulses = 1 or more triplets of gpios on, gpios off, delay.\n\
s = script id (>=0).\n\
t = text.\n\
u = user gpio (0-31).\n\
v = value.\n\
w = wave id (>=0).\n\
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
   {PI_BAD_PULSELEN     , "trigger pulse > 50 microseconds"},
   {PI_BAD_SCRIPT       , "invalid script"},
   {PI_BAD_SCRIPT_ID    , "unknown script id"},
   {PI_BAD_SER_OFFSET   , "add serial data offset > 30 minute"},
   {PI_GPIO_IN_USE      , "gpio already in use"},
   {PI_BAD_SERIAL_COUNT , "must read at least a byte at a time"},
   {PI_BAD_PARAM_NUM    , "script parameter must be 0-9"},
   {PI_DUP_TAG          , "script has duplicate tag"},
   {PI_TOO_MANY_TAGS    , "script has too many tags"},
   {PI_BAD_SCRIPT_CMD   , "illegal script command"},
   {PI_BAD_VAR_NUM      , "script variable must be 0-149"},
   {PI_NO_SCRIPT_ROOM   , "no more room for scripts"},
   {PI_NO_MEMORY        , "can't allocate temporary memory"},
   {PI_SOCK_READ_FAILED , "socket read failed"},
   {PI_SOCK_WRIT_FAILED , "socket write failed"},
   {PI_TOO_MANY_PARAM   , "too many script parameters > 10"},
   {PI_NOT_HALTED       , "script already running or failed"},
   {PI_BAD_TAG          , "script has unresolved tag"},
   {PI_BAD_MICS_DELAY   , "bad MICS delay (too large)"},
   {PI_BAD_MILS_DELAY   , "bad MILS delay (too large)"},
   {PI_BAD_WAVE_ID      , "non existent wave id"},
   {PI_TOO_MANY_CBS     , "No more CBs for waveform"},
   {PI_TOO_MANY_OOL     , "No more OOL for waveform"},
   {PI_EMPTY_WAVEFORM   , "attempt to create an empty waveform"},
   {PI_NO_WAVEFORM_ID   , "no more waveform ids"},

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
   return CMD_UNKNOWN_CMD;
}

static int getNum(char *str, unsigned *val, int8_t *opt)
{
   int f, n;
   unsigned v;

   *opt = 0;

   f = sscanf(str, " %i %n", &v, &n);

   if (f == 1)
   {
      *val = v;
      *opt = CMD_NUMERIC;
      return n;
   }

   f = sscanf(str, " v%i %n", &v, &n);

   if (f == 1)
   {
      *val = v;
      if (v < PI_MAX_SCRIPT_VARS) *opt = CMD_VAR;
      else *opt = -CMD_VAR;
      return n;
   }

   f = sscanf(str, " p%i %n", &v, &n);

   if (f == 1)
   {
      *val = v;
      if (v < PI_MAX_SCRIPT_PARAMS) *opt = CMD_PAR;
      else *opt = -CMD_PAR;
      return n;
   }

   return 0;
}

static char intCmdStr[32];

char *cmdStr(void)
{
   return intCmdStr;
}

int cmdParse(char *buf, uint32_t *p, void **v, cmdCtlParse_t *ctl)
{
   int f, valid, idx, val, pp, pars, n, n2, i;
   char *ptr;
   char c;
   int32_t param[MAX_PARAM];

   bzero(&ctl->opt, sizeof(ctl->opt));

   sscanf(buf+ctl->eaten, " %31s %n", intCmdStr, &pp);

   ctl->eaten += pp;

   p[0] = -1;

   idx = cmdMatch(intCmdStr);

   if (idx < 0) return idx;

   valid = 0;

   p[0] = cmdInfo[idx].cmd;
   p[1]  = 0;
   p[2]  = 0;

   switch (cmdInfo[idx].vt)
   {
      case 101: /* BR1  BR2  DCRA  H  HALT  HELP  HWVER  INRA  NO
                   PIGPV  POPA  PUSHA  RET  T  TICK  WVBSY  WVCLR
                   WVCRE  WVGO  WVGOR  WVHLT  WVNEW

                   No parameters, always valid.
                */
         valid = 1;

         break;

      case 111: /* ADD  AND  BC1  BC2  BS1  BS2  CMP  DIV  LDA  MLT
                   MOD  OR  RLA  RRA  SUB  WAIT  XOR

                   One parameter, any value.
                */
         ctl->eaten += getNum(buf+ctl->eaten, &p[1], &ctl->opt[0]);

         if (ctl->opt[0] > 0) valid = 1;

         break;

      case 112: /* MG  MICS  MILS  MODEG  NC  NP  PFG  PRG
                   PROCD  PROCP  PROCS  PRRG  R  READ  SLRC
                   WVDEL  WVSC  WVSM  WVSP  WVTX  WVTXR

                   One positive parameter.
                */
         ctl->eaten += getNum(buf+ctl->eaten, &p[1], &ctl->opt[0]);

         if ((ctl->opt[0] > 0) && ((int)p[1] >= 0)) valid = 1;

         break;

      case 113: /* DCR  INR  POP  PUSH  STA  XA

                   One register parameter.
                */
         ctl->eaten += getNum(buf+ctl->eaten, &p[1], &ctl->opt[0]);

         if ((ctl->opt[0] > 0) && (p[1] < PI_MAX_SCRIPT_VARS)) valid = 1;

         break;

      case 114: /* CALL  JM  JMP  JNZ  JP  JZ  TAG

                   One numeric parameter, any value.
                */
         ctl->eaten += getNum(buf+ctl->eaten, &p[1], &ctl->opt[0]);
         if (ctl->opt[0] == CMD_NUMERIC) valid = 1;

         break;

      case 115: /* PARSE  PROC

                   One parameter, string (rest of input).
                */
         p[1] = strlen(buf+ctl->eaten);
         v[1] = buf+ctl->eaten;
         ctl->eaten += p[1];

         valid = 1;

         break;

      case 116: /* SYS

                   One parameter, a string of letters, digits, '-' and '_'.
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

      case 121: /* P  PFS  PRS  PWM  S  SERVO  SLR  SLRO  W  WDOG  WRITE

                   Two positive parameters.
                */
         ctl->eaten += getNum(buf+ctl->eaten, &p[1], &ctl->opt[0]);
         ctl->eaten += getNum(buf+ctl->eaten, &p[2], &ctl->opt[1]);

         if ((ctl->opt[0] > 0) && ((int)p[1] >= 0) &&
             (ctl->opt[1] > 0) && ((int)p[2] >= 0)) valid = 1;

         break;

      case 122: /* NB

                   Two parameters, first positive, second any value.
                */
         ctl->eaten += getNum(buf+ctl->eaten, &p[1], &ctl->opt[0]);
         ctl->eaten += getNum(buf+ctl->eaten, &p[2], &ctl->opt[1]);

         if ((ctl->opt[0] > 0) && ((int)p[1] >= 0) &&
             (ctl->opt[1] > 0)) valid = 1;

         break;

      case 123: /* LD  RL  RR

                   Two parameters, first register, second any value.
                */
         ctl->eaten += getNum(buf+ctl->eaten, &p[1], &ctl->opt[0]);
         ctl->eaten += getNum(buf+ctl->eaten, &p[2], &ctl->opt[1]);

         if ((ctl->opt[0] > 0) &&
             (p[1] < PI_MAX_SCRIPT_VARS) &&
             (ctl->opt[1] > 0)) valid = 1;

         break;

      case 124: /* X

                   Two register parameters.
                */
         ctl->eaten += getNum(buf+ctl->eaten, &p[1], &ctl->opt[0]);
         ctl->eaten += getNum(buf+ctl->eaten, &p[2], &ctl->opt[1]);

         if ((ctl->opt[0] > 0) && (p[1] < PI_MAX_SCRIPT_VARS) &&
             (ctl->opt[1] > 0) && (p[2] < PI_MAX_SCRIPT_VARS)) valid = 1;

         break;

      case 125: /* M MODES

                   Two parameters, first positive, second in 'RW540123'.
                */
         ctl->eaten += getNum(buf+ctl->eaten, &p[1], &ctl->opt[0]);

         f = sscanf(buf+ctl->eaten, " %c %n", &c, &n);

         if ((ctl->opt[0] > 0) && ((int)p[1] >= 0) && (f >= 1))
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

      case 126: /* PUD

                   Two parameters, first positive, second in 'ODU'.
                */
         ctl->eaten += getNum(buf+ctl->eaten, &p[1], &ctl->opt[0]);

         f = sscanf(buf+ctl->eaten, " %c %n", &c, &n);

         if ((ctl->opt[0] > 0) && ((int)p[1] >= 0)  && (f >= 1))
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

      case 131: /* TRIG

                   Three positive parameters.
                */
         ctl->eaten += getNum(buf+ctl->eaten, &p[1], &ctl->opt[0]);
         ctl->eaten += getNum(buf+ctl->eaten, &p[2], &ctl->opt[1]);
         ctl->eaten += getNum(buf+ctl->eaten, &p[3], &ctl->opt[2]);

         if ((ctl->opt[0] > 0) && ((int)p[1] >= 0) &&
             (ctl->opt[1] > 0) && ((int)p[2] >= 0) &&
             (ctl->opt[2] == CMD_NUMERIC) && ((int)p[3] >= 0))
            valid = 1;

         break;

      case 141: /* WVAS

                  Four parameters, first two positive, third any value,
                  last string (rest of input).
                */
         ctl->eaten += getNum(buf+ctl->eaten, &p[1], &ctl->opt[0]);
         ctl->eaten += getNum(buf+ctl->eaten, &p[2], &ctl->opt[1]);
         ctl->eaten += getNum(buf+ctl->eaten, &p[3], &ctl->opt[2]);

         if ((ctl->opt[0] == CMD_NUMERIC) && ((int)p[1] >= 0) &&
             (ctl->opt[1] == CMD_NUMERIC) && ((int)p[2] >= 0) &&
             (ctl->opt[2] == CMD_NUMERIC))
         {
            p[4] = strlen(buf+ctl->eaten);
            v[1] = buf+ctl->eaten;
            ctl->eaten += p[4];
            valid = 1;
         }

         break;

      case 191: /* PROCR

                   One to 11 parameters, first positive,
                   optional remainder, any value.
                */
         ctl->eaten += getNum(buf+ctl->eaten, &p[1], &ctl->opt[0]);

         if ((ctl->opt[0] == CMD_NUMERIC) && ((int)p[1] >= 0))
         {
            pars = 0;

            while (pars < PI_MAX_SCRIPT_PARAMS)
            {
               ctl->eaten += getNum(buf+ctl->eaten, &p[2], &ctl->opt[1]);
               if (ctl->opt[1] == CMD_NUMERIC) param[pars++] = p[2];
               else break;
            }

            p[2] = pars;

            v[1] = param;

            valid = 1;
         }

         break;

      case 192: /* WVAG

                   One or more triplets (gpios on, gpios off, delay),
                   any value.
                */
         pars = 0;

         while (pars < MAX_PARAM)
         {
            ctl->eaten += getNum(buf+ctl->eaten, &p[1], &ctl->opt[0]);
            if (ctl->opt[0] == CMD_NUMERIC) param[pars++] = p[1];
            else break;
         }

         p[1] = pars / 3;

         v[1] = param;

         if (pars && ((pars % 3) == 0)) valid = 1;

         break;


   }

   if (valid) return idx; else return CMD_BAD_PARAMETER;
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

int cmdParseScript(char *script, cmdScript_t *s, int diags)
{
   int idx, len, b, i, j, tags, resolved;
   int status;
   uint32_t p[10];
   void *v[10];
   cmdInstr_t instr;
   cmdCtlParse_t ctl;

   ctl.eaten = 0;

   status = 0;

   cmdTagStep_t tag_step[PI_MAX_SCRIPT_TAGS];

   len = strlen(script);

   /* calloc space for PARAMS, VARS, CMDS, and STRINGS */

   b = (sizeof(int) * (PI_MAX_SCRIPT_PARAMS + PI_MAX_SCRIPT_VARS)) +
       (sizeof(cmdInstr_t) * (len + 2) / 2) + len;

   s->par = calloc(b, 1);

   if (s->par == NULL) return -1;

   s->var = s->par + PI_MAX_SCRIPT_PARAMS;

   s->instr = (cmdInstr_t *)(s->var + PI_MAX_SCRIPT_VARS);

   s->str_area = (char *)(s->instr + ((len + 2) / 2));

   s->str_area_len = len;
   s->str_area_pos = 0;

   s->instrs = 0;

   tags = 0;

   idx = 0;

   while (ctl.eaten<len)
   {
      idx = cmdParse(script, p, v, &ctl);

      memcpy(&instr.p, p, sizeof(instr.p));

      if (idx >= 0)
      {
         switch (instr.p[0])
         {
            case PI_CMD_HELP:
            case PI_CMD_PARSE:
            case PI_CMD_PROC:
            case PI_CMD_PROCD:
            case PI_CMD_PROCP:
            case PI_CMD_PROCR:
            case PI_CMD_PROCS:
            case PI_CMD_SLR:
            case PI_CMD_SLRC:
            case PI_CMD_SLRO:
            case PI_CMD_WVAG:
            case PI_CMD_WVAS:

               if (diags)
               {
                  fprintf(stderr, "Illegal command: %s\n", cmdStr());
               }

               if (!status) status = PI_BAD_SCRIPT_CMD;
               idx = -1;

               break;

            case PI_CMD_TAG:

               if (tags < PI_MAX_SCRIPT_TAGS)
               {
                  /* check tag not already used */
                  for (j=0; j<tags; j++)
                  {
                     if (tag_step[j].tag == instr.p[1])
                     {
                        if (diags)
                        {
                           fprintf(stderr, "Duplicate tag: %d\n", instr.p[1]);
                        }

                        if (!status) status = PI_DUP_TAG;
                        idx = -1;
                     }
                  }

                  tag_step[tags].tag = instr.p[1];
                  tag_step[tags].step = s->instrs;
                  tags++;
               }
               else
               {
                  if (diags)
                  {
                      fprintf(stderr, "Too many tags: %d\n", instr.p[1]);
                  }
                  if (!status) status = PI_TOO_MANY_TAGS;
                  idx = -1;
               }

               break;

            case PI_CMD_SYS:

               strncpy(s->str_area+s->str_area_pos, v[1], p[1]);
               s->str_area[s->str_area_pos+p[1]] = 0;
               instr.p[1] = (intptr_t) s->str_area+s->str_area_pos;
               s->str_area_pos += (p[1] + 1);

               break;

         }
      }
      else
      {
         if (diags)
         {
            if (idx == CMD_UNKNOWN_CMD)
               fprintf(stderr, "Unknown command: %s\n", cmdStr());
            else
               fprintf(stderr, "Bad parameter to %s\n", cmdStr());
         }
         if (!status) status = PI_BAD_SCRIPT_CMD;
      }

      if (idx >= 0)
      {
         if (instr.p[0] != PI_CMD_TAG)
         {
            memcpy(instr.opt, &ctl.opt, sizeof(instr.opt));
            s->instr[s->instrs++] = instr;
         }
      }
   }

   for (i=0; i<s->instrs; i++)
   {
      instr = s->instr[i];

      /* resolve jumps */

      if ((instr.p[0] == PI_CMD_JMP) || (instr.p[0] == PI_CMD_CALL) ||
          (instr.p[0] == PI_CMD_JZ)  || (instr.p[0] == PI_CMD_JNZ)  ||
          (instr.p[0] == PI_CMD_JM)  || (instr.p[0] == PI_CMD_JP))
      {
         resolved = 0;

         for (j=0; j<tags; j++)
         {
            if (instr.p[1] == tag_step[j].tag)
            {
               s->instr[i].p[1] = tag_step[j].step;
               resolved = 1;
               break;
            }
         }

         if (!resolved)
         {
            if (diags)
            {
               fprintf(stderr, "Can't resolve tag %d\n", instr.p[1]);
            }
            if (!status) status = PI_BAD_TAG;
         }
      }
   }
   return status;
}

