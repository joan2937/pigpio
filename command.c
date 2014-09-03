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
This version is for pigpio version 21+
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

   {PI_CMD_BC1,   "BC1",   111, 1}, // gpioWrite_Bits_0_31_Clear
   {PI_CMD_BC2,   "BC2",   111, 1}, // gpioWrite_Bits_32_53_Clear

   {PI_CMD_BR1,   "BR1",   101, 3}, // gpioRead_Bits_0_31
   {PI_CMD_BR2,   "BR2",   101, 3}, // gpioRead_Bits_32_53

   {PI_CMD_BS1,   "BS1",   111, 1}, // gpioWrite_Bits_0_31_Set
   {PI_CMD_BS2,   "BS2",   111, 1}, // gpioWrite_Bits_32_53_Set

   {PI_CMD_HELP,  "H",     101, 5}, // cmdUsage
   {PI_CMD_HELP,  "HELP",  101, 5}, // cmdUsage

   {PI_CMD_HWVER, "HWVER", 101, 4}, // gpioHardwareRevision

   {PI_CMD_I2CC,  "I2CC",  112, 0}, // i2cClose
   {PI_CMD_I2CO,  "I2CO",  131, 2}, // i2cOpen

   {PI_CMD_I2CPC, "I2CPC", 131, 2}, // i2cProcessCall
   {PI_CMD_I2CPK, "I2CPK", 194, 6}, // i2cBlockProcessCall

   {PI_CMD_I2CRB, "I2CRB", 121, 2}, // i2cReadByteData
   {PI_CMD_I2CRD, "I2CRD", 121, 6}, // i2cReadDevice
   {PI_CMD_I2CRI, "I2CRI", 131, 6}, // i2cReadI2CBlockData
   {PI_CMD_I2CRK, "I2CRK", 121, 6}, // i2cReadBlockData
   {PI_CMD_I2CRS, "I2CRS", 112, 2}, // i2cReadByte
   {PI_CMD_I2CRW, "I2CRW", 121, 2}, // i2cReadWordData

   {PI_CMD_I2CWB, "I2CWB", 131, 0}, // i2cWriteByteData
   {PI_CMD_I2CWD, "I2CWD", 193, 0}, // i2cWriteDevice
   {PI_CMD_I2CWI, "I2CWI", 194, 0}, // i2cWriteI2CBlockData
   {PI_CMD_I2CWK, "I2CWK", 194, 0}, // i2cWriteBlockData
   {PI_CMD_I2CWQ, "I2CWQ", 121, 0}, // i2cWriteQuick
   {PI_CMD_I2CWS, "I2CWS", 121, 0}, // i2cWriteByte
   {PI_CMD_I2CWW, "I2CWW", 131, 0}, // i2cWriteWordData

   {PI_CMD_MICS,  "MICS",  112, 0}, // gpioDelay
   {PI_CMD_MILS,  "MILS",  112, 0}, // gpioDelay

   {PI_CMD_MODEG, "MG"   , 112, 2}, // gpioGetMode
   {PI_CMD_MODEG, "MODEG", 112, 2}, // gpioGetMode

   {PI_CMD_MODES, "M",     125, 0}, // gpioSetMode
   {PI_CMD_MODES, "MODES", 125, 0}, // gpioSetMode

   {PI_CMD_NB,    "NB",    122, 0}, // gpioNotifyBegin
   {PI_CMD_NC,    "NC",    112, 0}, // gpioNotifyClose
   {PI_CMD_NO,    "NO",    101, 2}, // gpioNotifyOpen
   {PI_CMD_NP,    "NP",    112, 0}, // gpioNotifyPause

   {PI_CMD_PARSE, "PARSE", 115, 0}, // cmdParseScript

   {PI_CMD_PFG,   "PFG",   112, 2}, // gpioGetPWMfrequency
   {PI_CMD_PFS,   "PFS",   121, 2}, // gpioSetPWMfrequency

   {PI_CMD_PIGPV, "PIGPV", 101, 4}, // gpioVersion

   {PI_CMD_PRG,   "PRG",   112, 2}, // gpioGetPWMrange

   {PI_CMD_PROC,  "PROC",  115, 2}, // gpioStoreScript
   {PI_CMD_PROCD, "PROCD", 112, 0}, // gpioDeleteScript
   {PI_CMD_PROCP, "PROCP", 112, 7}, // gpioScriptStatus
   {PI_CMD_PROCR, "PROCR", 191, 0}, // gpioRunScript
   {PI_CMD_PROCS, "PROCS", 112, 0}, // gpioStopScript

   {PI_CMD_PRRG,  "PRRG",  112, 2}, // gpioGetPWMrealRange
   {PI_CMD_PRS,   "PRS",   121, 2}, // gpioSetPWMrange

   {PI_CMD_PUD,   "PUD",   126, 0}, // gpioSetPullUpDown

   {PI_CMD_PWM,   "P",     121, 0}, // gpioPWM
   {PI_CMD_PWM,   "PWM",   121, 0}, // gpioPWM

   {PI_CMD_READ,  "R",     112, 2}, // gpioRead
   {PI_CMD_READ,  "READ",  112, 2}, // gpioRead

   {PI_CMD_SERRB, "SERRB", 112, 2}, // serReadByte
   {PI_CMD_SERWB, "SERWB", 121, 0}, // serWriteByte
   {PI_CMD_SERC,  "SERC",  112, 0}, // serClose
   {PI_CMD_SERDA, "SERDA", 112, 2}, // serDataAvailable
   {PI_CMD_SERO,  "SERO",  132, 2}, // serOpen
   {PI_CMD_SERR,  "SERR",  121, 6}, // serRead
   {PI_CMD_SERW,  "SERW",  193, 0}, // serWrite

   {PI_CMD_SERVO, "S",     121, 0}, // gpioServo
   {PI_CMD_SERVO, "SERVO", 121, 0}, // gpioServo

   {PI_CMD_SLR,   "SLR",   121, 6}, // gpioSerialRead
   {PI_CMD_SLRC,  "SLRC",  112, 0}, // gpioSerialReadClose
   {PI_CMD_SLRO,  "SLRO",  121, 0}, // gpioSerialReadOpen

   {PI_CMD_SPIC,  "SPIC",  112, 0}, // spiClose
   {PI_CMD_SPIO,  "SPIO",  131, 2}, // spiOpen
   {PI_CMD_SPIR,  "SPIR",  121, 6}, // spiRead
   {PI_CMD_SPIW,  "SPIW",  193, 0}, // spiWrite
   {PI_CMD_SPIX,  "SPIX",  193, 6}, // spiXfer

   {PI_CMD_TICK,  "T",     101, 4}, // gpioTick
   {PI_CMD_TICK,  "TICK",  101, 4}, // gpioTick

   {PI_CMD_TRIG,  "TRIG",  131, 0}, // gpioTrigger

   {PI_CMD_WDOG,  "WDOG",  121, 0}, // gpioSetWatchdog

   {PI_CMD_WRITE, "W",     121, 0}, // gpioWrite
   {PI_CMD_WRITE, "WRITE", 121, 0}, // gpioWrite

   {PI_CMD_WVAG,  "WVAG",  192, 2}, // gpioWaveAddGeneric
   {PI_CMD_WVAS,  "WVAS",  196, 2}, // gpioWaveAddSerial
   {PI_CMD_WVBSY, "WVBSY", 101, 2}, // gpioWaveTxBusy
   {PI_CMD_WVCLR, "WVCLR", 101, 0}, // gpioWaveClear
   {PI_CMD_WVCRE, "WVCRE", 101, 2}, // gpioWaveCreate
   {PI_CMD_WVDEL, "WVDEL", 112, 0}, // gpioWaveDelete
   {PI_CMD_WVGO,  "WVGO" , 101, 2}, // gpioWaveTxStart
   {PI_CMD_WVGOR, "WVGOR", 101, 2}, // gpioWaveTxStart
   {PI_CMD_WVHLT, "WVHLT", 101, 0}, // gpioWaveTxStop
   {PI_CMD_WVNEW, "WVNEW", 101, 0}, // gpioWaveAddNew
   {PI_CMD_WVSC,  "WVSC",  112, 2}, // gpioWaveGet*Cbs
   {PI_CMD_WVSM,  "WVSM",  112, 2}, // gpioWaveGet*Micros
   {PI_CMD_WVSP,  "WVSP",  112, 2}, // gpioWaveGet*Pulses
   {PI_CMD_WVTX,  "WVTX",  112, 2}, // gpioWaveTxSend
   {PI_CMD_WVTXR, "WVTXR", 112, 2}, // gpioWaveTxSend

   {PI_CMD_ADD  , "ADD"  , 111, 0},
   {PI_CMD_AND  , "AND"  , 111, 0},
   {PI_CMD_CALL , "CALL" , 114, 0},
   {PI_CMD_CMDR  ,"CMDR" , 111, 0},
   {PI_CMD_CMDW , "CMDW" , 111, 0},
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
   {PI_CMD_LDAB , "LDAB" , 111, 0},
   {PI_CMD_MLT  , "MLT"  , 111, 0},
   {PI_CMD_MOD  , "MOD"  , 111, 0},
   {PI_CMD_NOP  , "NOP"  , 101, 0},
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
   {PI_CMD_STAB , "STAB" , 111, 0},
   {PI_CMD_SUB  , "SUB"  , 111, 0},
   {PI_CMD_SYS  , "SYS"  , 116, 0},
   {PI_CMD_TAG  , "TAG"  , 114, 0},
   {PI_CMD_WAIT , "WAIT" , 111, 0},
   {PI_CMD_X    , "X"    , 124, 0},
   {PI_CMD_XA   , "XA"   , 113, 0},
   {PI_CMD_XOR  , "XOR"  , 111, 0},

};


char * cmdUsage = "\
BC1 bits         Clear specified gpios in bank 1.\n\
BC2 bits         Clear specified gpios in bank 2.\n\
BR1              Read bank 1 gpios.\n\
BR2              Read bank 2 gpios.\n\
BS1 bits         Set specified gpios in bank 2.\n\
BS2 bits         Set specified gpios in bank 2.\n\
\n\
H/HELP           Display command help.\n\
\n\
HWVER            Get hardware version.\n\
\n\
I2CC h           Close I2C handle.\n\
I2CO ib id if    Open I2C bus and device with flags.\n\
\n\
I2CPC h r wv     smb Process Call: exchange register with word.\n\
I2CPK h r bvs    smb Block Process Call: exchange data bytes with register.\n\
\n\
I2CRB h r        smb Read Byte Data: read byte from register.\n\
I2CRD h num      i2c Read bytes.\n\
I2CRI h r num    smb Read I2C Block Data: read bytes from register.\n\
I2CRK h r        smb Read Block Data: read data from register.\n\
I2CRS h          smb Read Byte: read byte.\n\
I2CRW h r        smb Read Word Data: read word from register.\n\
\n\
I2CWB h r bv     smb Write Byte Data: write byte to register.\n\
I2CWD h bvs      i2c Write data.\n\
I2CWI h r bvs    smb Write I2C Block Data.\n\
I2CWK h r bvs    smb Write Block Data: write data to register.\n\
I2CWQ h bit      smb Write Quick: write bit.\n\
I2CWS h bv       smb Write Byte: write byte.\n\
I2CWW h r wv     smb Write Word Data: write word to register.\n\
\n\
M/MODES g m      Set gpio mode.\n\
MG/MODEG g       Get gpio mode.\n\
\n\
MICS v           Delay for microseconds.\n\
MILS v           Delay for milliseconds.\n\
\n\
NB h bits        Start notification.\n\
NC h             Close notification.\n\
NO               Request a notification.\n\
NP h             Pause notification.\n\
\n\
P/PWM u v        Set gpio PWM value.\n\
\n\
PARSE t          Validate script.\n\
\n\
PFG u            Get gpio PWM frequency.\n\
PFS u v          Set gpio PWM frequency.\n\
\n\
PIGPV            Get pigpio library version.\n\
\n\
PRG u            Get gpio PWM range.\n\
\n\
PROC t           Store script.\n\
PROCD sid        Delete script.\n\
PROCP sid        Get script status and parameters.\n\
PROCR sid pars   Run script.\n\
PROCS sid        Stop script.\n\
\n\
PRRG u           Get gpio PWM real range.\n\
PRS u v          Set gpio PWM range.\n\
\n\
PUD g p          Set gpio pull up/down.\n\
\n\
R/READ g         Read gpio level.\n\
\n\
S/SERVO u v      Set gpio servo pulsewidth.\n\
\n\
SERC h           Close serial handle.\n\
SERDA h          Check for serial data ready to read.\n\
SERO srd srb srf Open serial device at baud with flags.\n\
\n\
SERR h num       Read bytes from serial handle.\n\
SERRB            Read byte from serial handle.\n\
SERW h bvs       Write bytes to serial handle.\n\
SERWB h bv       Write byte to serial handle.\n\
\n\
SLR u num        Read bit bang serial data from gpio.\n\
SLRC u           Close gpio for bit bang serial data.\n\
SLRO u b         Open gpio for bit bang serial data.\n\
\n\
SPIC h           SPI close handle.\n\
SPIO sc sb sf    SPI open channel at baud with flags.\n\
SPIR h num       SPI read bytes from handle.\n\
SPIW h bvs       SPI write bytes to handle.\n\
SPIX h bvs       SPI transfer bytes to handle.\n\
\n\
T/TICK           Get current tick.\n\
\n\
TRIG u pl L      Trigger level for micros on gpio.\n\
\n\
W/WRITE g L      Write level to gpio.\n\
\n\
WDOG u v         Set millisecond watchdog on gpio.\n\
\n\
WVAG trips       Wave add generic pulses.\n\
WVAS u b o bvs   Wave add serial data with offset for gpio at baud.\n\
WVBSY            Check if wave busy.\n\
WVCLR            Wave clear.\n\
WVCRE            Create wave from added pulses.\n\
WVDEL wid        Delete waves w and higher.\n\
WVGO             Wave transmit (DEPRECATED).\n\
WVGOR            Wave transmit repeatedly (DEPRECATED).\n\
WVHLT            Wave stop.\n\
WVNEW            Start a new empty wave.\n\
WVSC ws          Wave get DMA control block stats.\n\
WVSM ws          Wave get micros stats.\n\
WVSP ws          Wave get pulses stats.\n\
WVTX wid         Transmit wave as one-shot.\n\
WVTXR wid        Transmit wave repeatedly.\n\
\n\
bits  = a mask where (1<<g) is set for each gpio g of interest.\n\
bv    = byte value (0-255).\n\
bvs   = one or more byte values (0-255).\n\
g     = any gpio (0-53).\n\
h     = handle (>=0).\n\
ib    = I2C bus (0-1).\n\
id    = I2C device (0-127).\n\
if    = I2C flags (0).\n\
L     = level (0-1).\n\
m     = mode (RW540123).\n\
num   = number of bytes to read.\n\
o     = offset (>=0).\n\
p     = pud (ODU).\n\
pars  = 0 to 10 parameters for script.\n\
pl    = pulse length (1-50).\n\
r     = register.\n\
sid   = script id (>=0).\n\
sb    = SPI baud.\n\
sc    = SPI channel (0-1).\n\
sf    = SPI flags (0-3).\n\
srd   = serial device (/dev/tty*).\n\
srb   = serial baud rate.\n\
srf   = serial flags (0).\n\
t     = text.\n\
trips = 1 or more triplets of gpios on, gpios off, delay.\n\
u     = user gpio (0-31).\n\
v     = value.\n\
w     = wave id (>=0).\n\
ws    = 0=now, 1=high, 2=max.\n\
wv    = word value (0-65535).\n\
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
   {PI_BAD_DUTYCYCLE    , "dutycycle not 0-range (default 255)"},
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
   {PI_BAD_HANDLE       , "unknown handle"},
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
   {PI_I2C_OPEN_FAILED  , "can't open I2C device"},
   {PI_SER_OPEN_FAILED  , "can't open serial device"},
   {PI_SPI_OPEN_FAILED  , "can't open SPI device"},
   {PI_BAD_I2C_BUS      , "bad I2C bus"},
   {PI_BAD_I2C_ADDR     , "bad I2C address"},
   {PI_BAD_SPI_CHANNEL  , "bad SPI channel"},
   {PI_BAD_FLAGS        , "bad i2c/spi/ser open flags"},
   {PI_BAD_SPI_SPEED    , "bad SPI speed"},
   {PI_BAD_SER_DEVICE   , "bad serial device name"},
   {PI_BAD_SER_SPEED    , "bad serial baud rate"},
   {PI_BAD_PARAM        , "bad i2c/spi/ser parameter"},
   {PI_I2C_WRITE_FAILED , "I2C write failed"},
   {PI_I2C_READ_FAILED  , "I2C read failed"},
   {PI_BAD_SPI_COUNT    , "bad SPI count"},
   {PI_SER_WRITE_FAILED , "ser write failed"},
   {PI_SER_READ_FAILED  , "ser read failed"},
   {PI_SER_READ_NO_DATA , "ser read no data available"},
   {PI_UNKNOWN_COMMAND  , "unknown command"},
   {PI_SPI_XFER_FAILED  , "spi xfer/read/write failed"},
   {PI_BAD_POINTER      , "bad (NULL) pointer"},
   {PI_NO_AUX_SPI       , "need a B+ for auxiliary SPI"},

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

int cmdParse(
   char *buf, uint32_t *p, unsigned ext_len, char *ext, cmdCtlParse_t *ctl)
{
   int f, valid, idx, val, pp, pars, n, n2, i;
   char *p8;
   int32_t *p32;
   char c;
   uint32_t tp1;
   int8_t to1;

   /* Check that ext is big enough for the largest message. */
   if (ext_len < (4 * CMD_MAX_PARAM)) return CMD_EXT_TOO_SMALL;

   bzero(&ctl->opt, sizeof(ctl->opt));

   sscanf(buf+ctl->eaten, " %31s %n", intCmdStr, &pp);

   ctl->eaten += pp;

   p[0] = -1;

   idx = cmdMatch(intCmdStr);

   if (idx < 0) return idx;

   valid = 0;

   p[0] = cmdInfo[idx].cmd;
   p[1] = 0;
   p[2] = 0;
   p[3] = 0;

   switch (cmdInfo[idx].vt)
   {
      case 101: /* BR1  BR2  H  HELP  HWVER
                   DCRA  HALT  INRA  NO
                   PIGPV  POPA  PUSHA  RET  T  TICK  WVBSY  WVCLR
                   WVCRE  WVGO  WVGOR  WVHLT  WVNEW

                   No parameters, always valid.
                */
         valid = 1;

         break;

      case 111: /* BC1  BC2  BS1  BS2  
                   ADD  AND  CMP  DIV  LDA  LDAB  MLT
                   MOD  OR  RLA  RRA  STAB  SUB  WAIT  XOR

                   One parameter, any value.
                */
         ctl->eaten += getNum(buf+ctl->eaten, &p[1], &ctl->opt[1]);

         if (ctl->opt[1] > 0) valid = 1;

         break;

      case 112: /* I2CC
                   I2CRB MG  MICS  MILS  MODEG  NC  NP  PFG  PRG
                   PROCD  PROCP  PROCS  PRRG  R  READ  SLRC  SPIC
                   WVDEL  WVSC  WVSM  WVSP  WVTX  WVTXR

                   One positive parameter.
                */
         ctl->eaten += getNum(buf+ctl->eaten, &p[1], &ctl->opt[1]);

         if ((ctl->opt[1] > 0) && ((int)p[1] >= 0)) valid = 1;

         break;

      case 113: /* DCR  INR  POP  PUSH  STA  XA

                   One register parameter.
                */
         ctl->eaten += getNum(buf+ctl->eaten, &p[1], &ctl->opt[1]);

         if ((ctl->opt[1] > 0) && (p[1] < PI_MAX_SCRIPT_VARS)) valid = 1;

         break;

      case 114: /* CALL  JM  JMP  JNZ  JP  JZ  TAG

                   One numeric parameter, any value.
                */
         ctl->eaten += getNum(buf+ctl->eaten, &p[1], &ctl->opt[1]);
         if (ctl->opt[1] == CMD_NUMERIC) valid = 1;

         break;

      case 115: /* PARSE  PROC

                   One parameter, string (rest of input).
                */
         p[3] = strlen(buf+ctl->eaten);
         memcpy(ext, buf+ctl->eaten, p[3]);
         ctl->eaten += p[3];
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
               p[3] = n;
               ctl->opt[3] = CMD_NUMERIC;
               memcpy(ext, buf+ctl->eaten, n);
               ctl->eaten += n2;
            }
         }

         break;

      case 121: /* I2CRD  I2CRR  I2CRW  I2CWB I2CWQ  P  PFS  PRS
                   PWM  S  SERVO  SLR  SLRO  W  WDOG  WRITE

                   Two positive parameters.
                */
         ctl->eaten += getNum(buf+ctl->eaten, &p[1], &ctl->opt[1]);
         ctl->eaten += getNum(buf+ctl->eaten, &p[2], &ctl->opt[2]);

         if ((ctl->opt[1] > 0) && ((int)p[1] >= 0) &&
             (ctl->opt[2] > 0) && ((int)p[2] >= 0)) valid = 1;

         break;

      case 122: /* NB

                   Two parameters, first positive, second any value.
                */
         ctl->eaten += getNum(buf+ctl->eaten, &p[1], &ctl->opt[1]);
         ctl->eaten += getNum(buf+ctl->eaten, &p[2], &ctl->opt[2]);

         if ((ctl->opt[1] > 0) && ((int)p[1] >= 0) &&
             (ctl->opt[2] > 0)) valid = 1;

         break;

      case 123: /* LD  RL  RR

                   Two parameters, first register, second any value.
                */
         ctl->eaten += getNum(buf+ctl->eaten, &p[1], &ctl->opt[1]);
         ctl->eaten += getNum(buf+ctl->eaten, &p[2], &ctl->opt[2]);

         if ((ctl->opt[1] > 0) &&
             (p[1] < PI_MAX_SCRIPT_VARS) &&
             (ctl->opt[2] > 0)) valid = 1;

         break;

      case 124: /* X

                   Two register parameters.
                */
         ctl->eaten += getNum(buf+ctl->eaten, &p[1], &ctl->opt[1]);
         ctl->eaten += getNum(buf+ctl->eaten, &p[2], &ctl->opt[2]);

         if ((ctl->opt[1] > 0) && (p[1] < PI_MAX_SCRIPT_VARS) &&
             (ctl->opt[2] > 0) && (p[2] < PI_MAX_SCRIPT_VARS)) valid = 1;

         break;

      case 125: /* M MODES

                   Two parameters, first positive, second in 'RW540123'.
                */
         ctl->eaten += getNum(buf+ctl->eaten, &p[1], &ctl->opt[1]);

         f = sscanf(buf+ctl->eaten, " %c %n", &c, &n);

         if ((ctl->opt[1] > 0) && ((int)p[1] >= 0) && (f >= 1))
         {
            ctl->eaten += n;
            val = toupper(c);
            p8 = strchr(fmtMdeStr, val);

            if (p8 != NULL)
            {
               val = p8 - fmtMdeStr;
               p[2] = val;
               valid = 1;
            }
         }

         break;

      case 126: /* PUD

                   Two parameters, first positive, second in 'ODU'.
                */
         ctl->eaten += getNum(buf+ctl->eaten, &p[1], &ctl->opt[1]);

         f = sscanf(buf+ctl->eaten, " %c %n", &c, &n);

         if ((ctl->opt[1] > 0) && ((int)p[1] >= 0)  && (f >= 1))
         {
            ctl->eaten += n;
            val = toupper(c);
            p8 = strchr(fmtPudStr, val);
            if (p8 != NULL)
            {
               val = p8 - fmtPudStr;
               p[2] = val;
               valid = 1;
            }
         }

         break;

      case 131: /* I2CO  I2CPC  I2CRI  I2CWB  I2CWW  SPIO  TRIG

                   Three positive parameters.
                */
         ctl->eaten += getNum(buf+ctl->eaten, &p[1], &ctl->opt[1]);
         ctl->eaten += getNum(buf+ctl->eaten, &p[2], &ctl->opt[2]);
         ctl->eaten += getNum(buf+ctl->eaten, &tp1, &to1);

         if ((ctl->opt[1] > 0) && ((int)p[1] >= 0) &&
             (ctl->opt[2] > 0) && ((int)p[2] >= 0) &&
             (to1 == CMD_NUMERIC) && ((int)tp1 >= 0))
         {
            p[3] = 4;
            memcpy(ext, &tp1, 4);
            valid = 1;
         }

         break;

      case 132: /* SERO

                   Three parameters, first a string, rest >=0
                */
         f = sscanf(buf+ctl->eaten, " %*s%n %n", &n, &n2);
         if ((f >= 0) && n)
         {
            p[3] = n;
            ctl->opt[2] = CMD_NUMERIC;
            memcpy(ext, buf+ctl->eaten, n);
            ctl->eaten += n2;

            ctl->eaten += getNum(buf+ctl->eaten, &p[1], &ctl->opt[1]);
            ctl->eaten += getNum(buf+ctl->eaten, &p[2], &ctl->opt[2]);

            if ((ctl->opt[1] > 0) && ((int)p[1] >= 0) &&
                (ctl->opt[2] > 0) && ((int)p[2] >= 0))
               valid = 1;
         }

         break;

      case 191: /* PROCR

                   One to 11 parameters, first positive,
                   optional remainder, any value.
                */
         ctl->eaten += getNum(buf+ctl->eaten, &p[1], &ctl->opt[1]);

         if ((ctl->opt[1] == CMD_NUMERIC) && ((int)p[1] >= 0))
         {
            pars = 0;
            p32 = (int32_t *)ext;

            while (pars < PI_MAX_SCRIPT_PARAMS)
            {
               ctl->eaten += getNum(buf+ctl->eaten, &tp1, &to1);
               if (to1 == CMD_NUMERIC)
               {
                  pars++;
                  *p32++ = tp1;
               }
               else break;
            }

            p[3] = pars * 4;

            valid = 1;
         }

         break;

      case 192: /* WVAG

                   One or more triplets (gpios on, gpios off, delay),
                   any value.
                */

         pars = 0;
         p32 = (int32_t *)ext;

         while (pars < CMD_MAX_PARAM)
         {
            ctl->eaten += getNum(buf+ctl->eaten, &tp1, &to1);
            if (to1 == CMD_NUMERIC)
            {
               pars++;
               *p32++ = tp1;
            }
            else break;
         }

         p[3] = pars * 4;

         if (pars && ((pars % 3) == 0)) valid = 1;

         break;

      case 193: /* I2CWD  SERW

                   Two or more parameters, first >=0, rest 0-255.
                */
         ctl->eaten += getNum(buf+ctl->eaten, &p[1], &ctl->opt[1]);

         if ((ctl->opt[1] == CMD_NUMERIC) && ((int)p[1] >= 0))
         {
            pars = 0;
            p8 = ext;

            while (pars < CMD_MAX_PARAM)
            {
               ctl->eaten += getNum(buf+ctl->eaten, &tp1, &to1);
               if ((to1 == CMD_NUMERIC) &&
                   ((int)tp1>=0) && ((int)tp1<=255))
               {
                  pars++;
                  *p8++ = tp1;
               }
               else break;
            }

            p[3] = pars;

            if (pars) valid = 1;
         }

         break;

      case 194: /* I2CPK  I2CWI  I2CWK

                   Three to 34 parameters, all 0-255.
                */

         ctl->eaten += getNum(buf+ctl->eaten, &p[1], &ctl->opt[1]);
         ctl->eaten += getNum(buf+ctl->eaten, &p[2], &ctl->opt[2]);

         if ((ctl->opt[1] == CMD_NUMERIC) &&
             (ctl->opt[2] == CMD_NUMERIC) &&
             ((int)p[1]>=0) && ((int)p[2]>=0) && ((int)p[2]<=255))
         {
            pars = 0;
            p8 = ext;

            while (pars < 32)
            {
               ctl->eaten += getNum(buf+ctl->eaten, &tp1, &to1);
               if ((to1 == CMD_NUMERIC) &&
                  ((int)tp1>=0) &&
                  ((int)tp1<=255))
               {
                  pars++;
                  *p8++ = tp1;
               }
               else break;
            }

            p[3] = pars;

            if (pars > 0) valid = 1;
         }

         break;

      case 196: /* WVAS

                   gpio baud offset char...

                   p1 gpio
                   p2 baud
                   p3 len + 4
                   ---------
                   uint32_t offset
                   uint8_t[len]
                */
         ctl->eaten += getNum(buf+ctl->eaten, &p[1], &ctl->opt[1]);
         ctl->eaten += getNum(buf+ctl->eaten, &p[2], &ctl->opt[2]);
         ctl->eaten += getNum(buf+ctl->eaten, &tp1, &to1);

         if ((ctl->opt[1] == CMD_NUMERIC) && ((int)p[1] >= 0) &&
             (ctl->opt[2] == CMD_NUMERIC) && ((int)p[2] > 0) &&
             (to1 == CMD_NUMERIC))
         {
            pars = 0;

            memcpy(ext, &tp1, 4);
            p8 = ext + 4;
            while (pars < CMD_MAX_PARAM)
            {
               ctl->eaten += getNum(buf+ctl->eaten, &tp1, &to1);
               if ((to1 == CMD_NUMERIC) &&
                   ((int)tp1>=0) && ((int)tp1<=255))
               {
                  *p8++ = tp1;
                  pars++;
               }
               else break;
            }

            p[3] = pars + 4;

            if (pars > 0) valid = 1;
         }

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
   cmdInstr_t instr;
   cmdCtlParse_t ctl;
   char v[CMD_MAX_EXTENSION];

   ctl.eaten = 0;

   status = 0;

   cmdTagStep_t tag_step[PI_MAX_SCRIPT_TAGS];

   len = strlen(script);

   /* calloc space for PARAMS, VARS, CMDS, and STRINGS */

   b = (sizeof(int) * (PI_MAX_SCRIPT_PARAMS + PI_MAX_SCRIPT_VARS)) +
       (sizeof(cmdInstr_t) * (len + 2) / 2) + len;

   s->par = calloc(1, b);

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
      idx = cmdParse(script, p, CMD_MAX_EXTENSION, v, &ctl);

      if (idx >= 0)
      {
         if (p[3])
         {
            memcpy(s->str_area + s->str_area_pos, v, p[3]);
            s->str_area[s->str_area_pos + p[3]] = 0;
            p[4] = (intptr_t) s->str_area + s->str_area_pos;
            s->str_area_pos += (p[3] + 1);
         }

         memcpy(&instr.p, p, sizeof(instr.p));

         if (instr.p[0] == PI_CMD_TAG)
         {
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

