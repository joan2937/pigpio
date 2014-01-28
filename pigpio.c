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

/* pigpio version 12 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <ctype.h>
#include <syslog.h>
#include <poll.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <limits.h>
#include <pthread.h> 
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>

#include "pigpio.h"
#include "command.h"

/* --------------------------------------------------------------- */

/*
 0 GPFSEL0   GPIO Function Select 0
 1 GPFSEL1   GPIO Function Select 1
 2 GPFSEL2   GPIO Function Select 2
 3 GPFSEL3   GPIO Function Select 3
 4 GPFSEL4   GPIO Function Select 4
 5 GPFSEL5   GPIO Function Select 5
 6 -         Reserved
 7 GPSET0    GPIO Pin Output Set 0
 8 GPSET1    GPIO Pin Output Set 1
 9 -         Reserved
10 GPCLR0    GPIO Pin Output Clear 0
11 GPCLR1    GPIO Pin Output Clear 1
12 -         Reserved
13 GPLEV0    GPIO Pin Level 0
14 GPLEV1    GPIO Pin Level 1
15 -         Reserved
16 GPEDS0    GPIO Pin Event Detect Status 0
17 GPEDS1    GPIO Pin Event Detect Status 1
18 -         Reserved
19 GPREN0    GPIO Pin Rising Edge Detect Enable 0
20 GPREN1    GPIO Pin Rising Edge Detect Enable 1
21 -         Reserved
22 GPFEN0    GPIO Pin Falling Edge Detect Enable 0
23 GPFEN1    GPIO Pin Falling Edge Detect Enable 1
24 -         Reserved
25 GPHEN0    GPIO Pin High Detect Enable 0
26 GPHEN1    GPIO Pin High Detect Enable 1
27 -         Reserved
28 GPLEN0    GPIO Pin Low Detect Enable 0
29 GPLEN1    GPIO Pin Low Detect Enable 1
30 -         Reserved
31 GPAREN0   GPIO Pin Async. Rising Edge Detect 0
32 GPAREN1   GPIO Pin Async. Rising Edge Detect 1
33 -         Reserved
34 GPAFEN0   GPIO Pin Async. Falling Edge Detect 0
35 GPAFEN1   GPIO Pin Async. Falling Edge Detect 1
36 -         Reserved
37 GPPUD     GPIO Pin Pull-up/down Enable
38 GPPUDCLK0 GPIO Pin Pull-up/down Enable Clock 0
39 GPPUDCLK1 GPIO Pin Pull-up/down Enable Clock 1
40 -         Reserved
41 -         Test
*/

/*
0 CS           DMA Channel 0 Control and Status
1 CPI_ONBLK_AD DMA Channel 0 Control Block Address
2 TI           DMA Channel 0 CB Word 0 (Transfer Information)
3 SOURCE_AD    DMA Channel 0 CB Word 1 (Source Address)
4 DEST_AD      DMA Channel 0 CB Word 2 (Destination Address)
5 TXFR_LEN     DMA Channel 0 CB Word 3 (Transfer Length)
6 STRIDE       DMA Channel 0 CB Word 4 (2D Stride)
7 NEXTCPI_ONBK DMA Channel 0 CB Word 5 (Next CB Address)
8 DEBUG        DMA Channel 0 Debug 
*/

/*
DEBUG register bits

bit 2 READ_ERROR

   Slave Read Response Error RW 0x0

   Set if the read operation returned an error value on 
   the read response bus. It can be cleared by writing 
   a 1.

bit 1 FIFO_ERROR

   Fifo Error RW 0x0

   Set if the optional read Fifo records an error 
   condition. It can be cleared by writing a 1.

bit 0 READ_LAST_NOT_SET_ERROR

   Read Last Not Set Error RW 0x0

   If the AXI read last signal was not set when 
   expected, then this error bit will be set. It can be 
   cleared by writing a 1. 
*/

/*
0 CTL        PWM Control
1 STA        PWM Status
2 DMAC       PWM DMA Configuration
4 RNG1       PWM Channel 1 Range
5 DAT1       PWM Channel 1 Data
6 FIF1       PWM FIFO Input
8 RNG2       PWM Channel 2 Range
9 DAT2       PWM Channel 2 Data
*/

/*
0 PCM_CS     PCM Control and Status
1 PCM_FIFO   PCM FIFO Data
2 PCM_MODE   PCM Mode
3 PCM_RXC    PCM Receive Configuration
4 PCM_TXC    PCM Transmit Configuration
5 PCM_DREQ   PCM DMA Request Level
6 PCM_INTEN  PCM Interrupt Enables
7 PCM_INTSTC PCM Interrupt Status & Clear
8 PCM_GRAY   PCM Gray Mode Control
*/

/*
0 CS  System Timer Control/Status
1 CLO System Timer Counter Lower 32 bits
2 CHI System Timer Counter Higher 32 bits
3 C0  System Timer Compare 0
4 C1  System Timer Compare 1
5 C2  System Timer Compare 2
6 C3  System Timer Compare 3
*/

/* --------------------------------------------------------------- */

#define THOUSAND 1000
#define MILLION  1000000
#define BILLION  1000000000

#define BANK (gpio>>5)

#define BIT  (1<<(gpio&0x1F))


#define CHECK_INITED                                               \
   do                                                              \
   {                                                               \
      if (!libInitialised)                                         \
      {                                                            \
         fprintf(stderr,                                           \
            "%s %s: pigpio uninitialised, call gpioInitialise()\n",\
            myTimeStamp(), __FUNCTION__);                          \
         return PI_NOT_INITIALISED;                                \
      }                                                            \
   }                                                               \
   while (0)

#define CHECK_INITED_RET_NULL_PTR                                  \
   do                                                              \
   {                                                               \
      if (!libInitialised)                                         \
      {                                                            \
         fprintf(stderr,                                           \
            "%s %s: pigpio uninitialised, call gpioInitialise()\n",\
            myTimeStamp(), __FUNCTION__);                          \
         return (NULL);                                            \
      }                                                            \
   }                                                               \
   while (0)

#define CHECK_INITED_RET_NIL                                       \
   do                                                              \
   {                                                               \
      if (!libInitialised)                                         \
      {                                                            \
         fprintf(stderr,                                           \
            "%s %s: pigpio uninitialised, call gpioInitialise()\n",\
            myTimeStamp(), __FUNCTION__);                          \
      }                                                            \
   }                                                               \
   while (0)

#define CHECK_NOT_INITED                                           \
   do                                                              \
   {                                                               \
      if (libInitialised)                                          \
      {                                                            \
         fprintf(stderr,                                           \
            "%s %s: pigpio initialised, call gpioTerminate()\n",   \
            myTimeStamp(), __FUNCTION__);                          \
         return PI_INITIALISED;                                    \
      }                                                            \
   }                                                               \
   while (0)

#define DBG(level, format, arg...)                                 \
   do                                                              \
   {                                                               \
      if (gpioCfg.dbgLevel >= level)                               \
         fprintf(stderr, "%s %s: " format "\n" ,                   \
            myTimeStamp(), __FUNCTION__ , ## arg);                 \
   }                                                               \
   while (0)

#define SOFT_ERROR(x, format, arg...)                              \
   do                                                              \
   {                                                               \
      fprintf(stderr, "%s %s: " format "\n",                       \
         myTimeStamp(), __FUNCTION__ , ## arg);                    \
      return x;                                                    \
   }                                                               \
   while (0)

#define PERM_ERROR(format, arg...)                                 \
   do                                                              \
   {                                                               \
      fprintf(stderr, "%s " format "\n", myTimeStamp(), ## arg);   \
   }                                                               \
   while (0)

#define TIMER_ADD(a, b, result)                                    \
   do                                                              \
   {                                                               \
      (result)->tv_sec =  (a)->tv_sec  + (b)->tv_sec;              \
      (result)->tv_nsec = (a)->tv_nsec + (b)->tv_nsec;             \
      if ((result)->tv_nsec >= BILLION)                            \
      {                                                            \
        ++(result)->tv_sec;                                        \
        (result)->tv_nsec -= BILLION;                              \
      }                                                            \
   }                                                               \
   while (0)

#define TIMER_SUB(a, b, result)                                    \
   do                                                              \
   {                                                               \
      (result)->tv_sec =  (a)->tv_sec  - (b)->tv_sec;              \
      (result)->tv_nsec = (a)->tv_nsec - (b)->tv_nsec;             \
      if ((result)->tv_nsec < 0)                                   \
      {                                                            \
         --(result)->tv_sec;                                       \
         (result)->tv_nsec += BILLION;                             \
      }                                                            \
   }                                                               \
   while (0)

#define DMA_BUS_ADR 0x40000000

#define CLK_BASE   0x20101000
#define DMA_BASE   0x20007000
#define DMA15_BASE 0x20E05000
#define GPIO_BASE  0x20200000
#define PCM_BASE   0x20203000
#define PWM_BASE   0x2020C000
#define SPI0_BASE  0x20204000
#define SYST_BASE  0x20003000
#define UART0_BASE 0x20201000
#define UART1_BASE 0x20215000

#define DMA_LEN   0x1000 /* allow access to all channels */
#define CLK_LEN   0xA8
#define GPIO_LEN  0xB4
#define SYST_LEN  0x1C
#define PCM_LEN   0x24
#define PWM_LEN   0x28

#define DMA_ENABLE (0xFF0/4)

#define GPFSEL0    0

#define GPSET0     7
#define GPSET1     8

#define GPCLR0    10
#define GPCLR1    11

#define GPLEV0    13
#define GPLEV1    14

#define GPEDS0    16
#define GPEDS1    17

#define GPREN0    19
#define GPREN1    20
#define GPFEN0    22
#define GPFEN1    23
#define GPHEN0    25
#define GPHEN1    26
#define GPLEN0    28
#define GPLEN1    29
#define GPAREN0   31
#define GPAREN1   32
#define GPAFEN0   34
#define GPAFEN1   35

#define GPPUD     37
#define GPPUDCLK0 38
#define GPPUDCLK1 39

#define DMA_CS        0
#define DMA_CONBLK_AD 1
#define DMA_DEBUG     8

/* DMA CS Control and Status bits */
#define DMA_CHANNEL_RESET       (1<<31)
#define DMA_WAIT_ON_WRITES      (1<<28)
#define DMA_PANIC_PRIORITY(x) ((x)<<20)
#define DMA_PRIORITY(x)       ((x)<<16)
#define DMA_INTERRUPT_STATUS    (1<< 2)
#define DMA_END_FLAG            (1<< 1)
#define DMA_ACTIVATE            (1<< 0)

/* DMA control block "info" field bits */
#define DMA_NO_WIDE_BURSTS          (1<<26)
#define DMA_PERIPHERAL_MAPPING(x) ((x)<<16)
#define DMA_BURST_LENGTH(x)       ((x)<<12)
#define DMA_SRC_IGNORE              (1<<11)
#define DMA_SRC_DREQ                (1<<10)
#define DMA_SRC_INC                 (1<< 8)
#define DMA_DEST_IGNORE             (1<< 7)
#define DMA_DEST_DREQ               (1<< 6)
#define DMA_DEST_INC                (1<< 4)
#define DMA_WAIT_RESP               (1<< 3)

#define DMA_DEBUG_READ_ERR           (1<<2)
#define DMA_DEBUG_FIFO_ERR           (1<<1)
#define DMA_DEBUG_RD_LST_NOT_SET_ERR (1<<0)

#define PWM_CTL      0
#define PWM_STA      1
#define PWM_DMAC     2
#define PWM_RNG1     4
#define PWM_DAT1     5
#define PWM_FIFO     6
#define PWM_RNG2     8
#define PWM_DAT2     9

#define PWM_CTL_CLRF1 (1<<6)
#define PWM_CTL_USEF1 (1<<5)
#define PWM_CTL_MODE1 (1<<1)
#define PWM_CTL_PWEN1 (1<<0)

#define PWM_DMAC_ENAB      (1 <<31)
#define PWM_DMAC_PANIC(x) ((x)<< 8)
#define PWM_DMAC_DREQ(x)   (x)

#define PCM_CS     0
#define PCM_FIFO   1
#define PCM_MODE   2
#define PCM_RXC    3
#define PCM_TXC    4
#define PCM_DREQ   5
#define PCM_INTEN  6
#define PCM_INTSTC 7
#define PCM_GRAY   8

#define PCM_CS_STBY     (1 <<25)
#define PCM_CS_SYNC     (1 <<24)
#define PCM_CS_RXSEX    (1 <<23)
#define PCM_CS_RXERR    (1 <<16)
#define PCM_CS_TXERR    (1 <<15)
#define PCM_CS_DMAEN    (1  <<9)
#define PCM_CS_RXTHR(x) ((x)<<7)
#define PCM_CS_TXTHR(x) ((x)<<5)
#define PCM_CS_RXCLR    (1  <<4)
#define PCM_CS_TXCLR    (1  <<3)
#define PCM_CS_TXON     (1  <<2)
#define PCM_CS_RXON     (1  <<1)
#define PCM_CS_EN       (1  <<0)

#define PCM_MODE_CLK_DIS  (1  <<28)
#define PCM_MODE_PDMN     (1  <<27)
#define PCM_MODE_PDME     (1  <<26)
#define PCM_MODE_FRXP     (1  <<25)
#define PCM_MODE_FTXP     (1  <<24)
#define PCM_MODE_CLKM     (1  <<23)
#define PCM_MODE_CLKI     (1  <<22)
#define PCM_MODE_FSM      (1  <<21)
#define PCM_MODE_FSI      (1  <<20)
#define PCM_MODE_FLEN(x)  ((x)<<10)
#define PCM_MODE_FSLEN(x) ((x)<< 0)

#define PCM_RXC_CH1WEX    (1  <<31)
#define PCM_RXC_CH1EN     (1  <<30)
#define PCM_RXC_CH1POS(x) ((x)<<20)
#define PCM_RXC_CH1WID(x) ((x)<<16)
#define PCM_RXC_CH2WEX    (1  <<15)
#define PCM_RXC_CH2EN     (1  <<14)
#define PCM_RXC_CH2POS(x) ((x)<< 4)
#define PCM_RXC_CH2WID(x) ((x)<< 0)

#define PCM_TXC_CH1WEX    (1  <<31)
#define PCM_TXC_CH1EN     (1  <<30)
#define PCM_TXC_CH1POS(x) ((x)<<20)
#define PCM_TXC_CH1WID(x) ((x)<<16)
#define PCM_TXC_CH2WEX    (1  <<15)
#define PCM_TXC_CH2EN     (1  <<14)
#define PCM_TXC_CH2POS(x) ((x)<< 4)
#define PCM_TXC_CH2WID(x) ((x)<< 0)

#define PCM_DREQ_TX_PANIC(x) ((x)<<24)
#define PCM_DREQ_RX_PANIC(x) ((x)<<16)
#define PCM_DREQ_TX_REQ_L(x) ((x)<< 8)
#define PCM_DREQ_RX_REQ_L(x) ((x)<< 0)

#define PCM_INTEN_RXERR (1<<3)
#define PCM_INTEN_TXERR (1<<2)
#define PCM_INTEN_RXR   (1<<1)
#define PCM_INTEN_TXW   (1<<0)

#define PCM_INTSTC_RXERR (1<<3)
#define PCM_INTSTC_TXERR (1<<2)
#define PCM_INTSTC_RXR   (1<<1)
#define PCM_INTSTC_TXW   (1<<0)

#define PCM_GRAY_FLUSH (1<<2)
#define PCM_GRAY_CLR   (1<<1)
#define PCM_GRAY_EN    (1<<0)

#define CLK_PASSWD  (0x5A<<24)

#define CLK_CTL_MASH(x)((x)<<9)
#define CLK_CTL_BUSY    (1 <<7)
#define CLK_CTL_KILL    (1 <<5)
#define CLK_CTL_ENAB    (1 <<4)
#define CLK_CTL_SRC(x) ((x)<<0)

#define CLK_CTL_SRC_OSC  1  /*  19.2 MHz */
#define CLK_CTL_SRC_PLLD 6  /* 500.0 MHz */

#define CLK_DIV_DIVI(x) ((x)<<12)
#define CLK_DIV_DIVF(x) ((x)<< 0)

#define CLK_PCMCTL 38
#define CLK_PCMDIV 39

#define CLK_PWMCTL 40
#define CLK_PWMDIV 41

#define SYST_CS      0
#define SYST_CLO     1
#define SYST_CHI     2

/* --------------------------------------------------------------- */

#define NORMAL_DMA (DMA_NO_WIDE_BURSTS | DMA_WAIT_RESP)

#define TIMED_DMA(x)  (DMA_DEST_DREQ | DMA_PERIPHERAL_MAPPING(x))

#define DBG_MIN_LEVEL 0
#define DBG_STARTUP   1
#define DBG_DMACBS    2
#define DBG_USER      3
#define DBG_INTERNAL  4
#define DBG_SLOW_TICK 5
#define DBG_FAST_TICK 6
#define DBG_MAX_LEVEL 6

#define GPIO_UNDEFINED 0
#define GPIO_INPUT     1
#define GPIO_OUTPUT    2
#define GPIO_PWM       3
#define GPIO_SERVO     4
#define GPIO_ALTERNATE 5

#define STACK_SIZE (256*1024)

#define PAGE_SIZE 4096

#define PWM_FREQS 18

#define CYCLES_PER_BLOCK 80
#define PULSE_PER_CYCLE  25

#define PAGES_PER_BLOCK   53

#define CBS_PER_IPAGE    117
#define LVS_PER_IPAGE     38
#define OFF_PER_IPAGE     38
#define TCK_PER_IPAGE      2
#define ON_PER_IPAGE       2

#define CBS_PER_OPAGE    118
#define ONOFF_PER_OPAGE   79

#define CBS_PER_CYCLE ((PULSE_PER_CYCLE*3)+2)

#define NUM_CBS (CBS_PER_CYCLE * bufferCycles)

#define SUPERCYCLE 800
#define SUPERLEVEL 20000

#define BLOCK_SIZE (PAGES_PER_BLOCK*PAGE_SIZE)

#define DMA_PAGES (PAGES_PER_BLOCK * bufferBlocks)

#define TICKSLOTS 50

#define PI_NOTIFY_SLOTS  32

#define PI_NOTIFY_CLOSED  0
#define PI_NOTIFY_CLOSING 1
#define PI_NOTIFY_OPENED  2
#define PI_NOTIFY_RUNNING 3
#define PI_NOTIFY_PAUSED  4

#define PI_WFRX_NONE   0
#define PI_WFRX_SERIAL 1
#define PI_WF_MICROS   2

#define DATUMS 2000

#define DEFAULT_PWM_IDX 5

#define MAX_EMITS (PIPE_BUF / sizeof(gpioReport_t))

#define SRX_BUF_SIZE 8192
#define CMD_BUF_SIZE 2048

/* --------------------------------------------------------------- */

typedef void (*callbk_t) ();

typedef struct { /* linux/arch/arm/mach-bcm2708/include/mach/dma.h */
   unsigned long info;
   unsigned long src;
   unsigned long dst;
   unsigned long length;
   unsigned long stride;
   unsigned long next;
   unsigned long pad[2];
} dmaCbs_t;

typedef struct
{
   dmaCbs_t cb           [128];
} dmaPage_t;

typedef struct
{
   dmaCbs_t cb           [CBS_PER_IPAGE];
   uint32_t level        [LVS_PER_IPAGE];
   uint32_t gpioOff      [OFF_PER_IPAGE];
   uint32_t tick         [TCK_PER_IPAGE];
   uint32_t gpioOn       [ON_PER_IPAGE];
   uint32_t periphData;
   uint32_t pad[7];
} dmaIPage_t;

typedef struct
{
   dmaCbs_t cb           [CBS_PER_OPAGE];
   uint32_t gpioOnOff    [ONOFF_PER_OPAGE];
   uint32_t periphData;
} dmaOPage_t;

typedef struct
{
   uint8_t  is;
   uint8_t  pad;
   uint16_t width;
   uint16_t range; /* duty cycles specified by 0 .. range */
   uint16_t freqIdx;
} gpioInfo_t;

typedef struct
{
   callbk_t func;
   unsigned ex;
   void *   userdata;
   int      timeout;
   uint32_t tick;
} gpioAlert_t;

typedef struct
{
   callbk_t func;
   unsigned ex;
   void *   userdata;
} gpioSignal_t;

typedef struct
{
   callbk_t func;
   unsigned ex;
   void *   userdata;
   uint32_t bits;
} gpioGetSamples_t;

typedef struct
{
   callbk_t        func;
   unsigned        ex;
   void *          userdata;
   unsigned        id;
   unsigned        running;
   unsigned        millis;
   struct timespec nextTick;
   pthread_t       pthId;
} gpioTimer_t;

typedef struct
{
   uint16_t valid;
   uint16_t bits;
   uint16_t divi;
   uint16_t divf;
   uint16_t mash;
   uint16_t servoIdx;
   uint16_t pwmIdx;
} clkCfg_t;

typedef struct
{
   uint16_t seqno;
   uint16_t state;
   uint32_t bits;
   int      fd;
   int      pipe;
} gpioNotify_t;

typedef struct
{
   uint32_t startTick;
   uint32_t alertTicks;
   uint32_t diffTick[TICKSLOTS];
   uint32_t cbTicks;
   uint32_t cbCalls;
   uint32_t maxEmit;
   uint32_t emitFrags;
   uint32_t maxSamples;
   uint32_t numSamples;
} gpioStats_t;

typedef struct
{
   unsigned bufferMilliseconds;
   unsigned clockMicros;
   unsigned clockPeriph;
   unsigned clockSource;
   unsigned DMAprimaryChannel;
   unsigned DMAsecondaryChannel;
   unsigned socketPort;
   unsigned ifFlags;
   int      dbgLevel;
   unsigned showStats;
} gpioCfg_t;

typedef struct
{
   uint32_t micros;
   uint32_t highMicros;
   uint32_t maxMicros;
   uint32_t pulses;
   uint32_t highPulses;
   uint32_t maxPulses;
   uint32_t cbs;
   uint32_t highCbs;
   uint32_t maxCbs;
} wfStats_t;

typedef struct
{
   int      gpio;
   char   * buf;
   uint32_t bufSize;
   int      readPos;
   int      writePos;
   uint32_t baud;
   uint32_t fullBit;
   uint32_t halfBit;
   int      timeout;
   uint32_t startBitTick;
   uint32_t nextBitDiff;
   int      bit;
   int      byte;
   int      level;
   int      mode;
} wfRx_t;


/* --------------------------------------------------------------- */

/* initialise once then preserve */

static volatile gpioCfg_t gpioCfg =
{
   PI_DEFAULT_BUFFER_MILLIS,
   PI_DEFAULT_CLK_MICROS,
   PI_DEFAULT_CLK_PERIPHERAL,
   PI_DEFAULT_CLK_SOURCE,
   PI_DEFAULT_DMA_PRIMARY_CHANNEL,
   PI_DEFAULT_DMA_SECONDARY_CHANNEL,
   PI_DEFAULT_SOCKET_PORT,
   PI_DEFAULT_IF_FLAGS,
   0,
   0,
};

static volatile gpioStats_t gpioStats;

static int gpioMaskSet = 0;

/* initialise every gpioInitialise */

static struct timespec libStarted;

/* initialse if not libInitialised */

static uint64_t gpioMask;

static gpioPulse_t wf[3][PI_WAVE_MAX_PULSES];

static int wfc[3]={0, 0, 0};

static int wfcur=0;

static wfStats_t wfStats=
{
   0, 0, PI_WAVE_MAX_MICROS,
   0, 0, PI_WAVE_MAX_PULSES,
   0, 0, (PAGES_PER_BLOCK * CBS_PER_OPAGE)
};

static volatile wfRx_t wfRx[PI_MAX_USER_GPIO+1];

static volatile uint32_t alertBits    = 0;
static volatile uint32_t monitorBits  = 0;
static volatile uint32_t notifyBits   = 0;

static volatile int DMAstarted = 0;

static int      libInitialised = 0;

static int pthAlertRunning  = 0;
static int pthFifoRunning   = 0;
static int pthSocketRunning = 0;

static gpioAlert_t      gpioAlert  [PI_MAX_USER_GPIO+1];

static gpioGetSamples_t gpioGetSamples;

static gpioInfo_t       gpioInfo   [PI_MAX_USER_GPIO+1];

static gpioNotify_t     gpioNotify [PI_NOTIFY_SLOTS];

static gpioSignal_t     gpioSignal [PI_MAX_SIGNUM+1];

static gpioTimer_t      gpioTimer  [PI_MAX_TIMER+1];

static int pwmFreq[PWM_FREQS];

/* no initialisation required */

static unsigned bufferBlocks; /* number of blocks in buffer */
static unsigned bufferCycles; /* number of cycles */

static pthread_attr_t pthAttr;

static pthread_t pthAlert;
static pthread_t pthFifo;
static pthread_t pthSocket;

static gpioSample_t gpioSample[DATUMS];
static gpioReport_t gpioReport[DATUMS];

/* resources which must be released on gpioTerminate */

static FILE * inpFifo = NULL;
static FILE * outFifo = NULL;

static int fdLock = -1;
static int fdMem  = -1;
static int fdSock = -1;

static dmaPage_t * * dmaBloc = MAP_FAILED;
static dmaPage_t * * dmaVirt = MAP_FAILED;
static dmaPage_t * * dmaPhys = MAP_FAILED;

static dmaIPage_t * * dmaIVirt = MAP_FAILED;
static dmaIPage_t * * dmaIPhys = MAP_FAILED;

static dmaOPage_t * * dmaOVirt = MAP_FAILED;
static dmaOPage_t * * dmaOPhys = MAP_FAILED;

static volatile uint32_t  * clkReg  = MAP_FAILED;
static volatile uint32_t  * dmaReg  = MAP_FAILED;
static volatile uint32_t  * gpioReg = MAP_FAILED;
static volatile uint32_t  * pcmReg  = MAP_FAILED;
static volatile uint32_t  * pwmReg  = MAP_FAILED;
static volatile uint32_t  * systReg = MAP_FAILED;

static volatile uint32_t  * dmaIn   = MAP_FAILED;
static volatile uint32_t  * dmaOut  = MAP_FAILED;

/* constant data */

static const clkCfg_t clkCfg[]=
{
   /* valid bits  divi  divf mash servo                pwm */
      {   0,   0,    0,    0,   0,    0,                  0}, /*  0 */
      {   1,   9,    2,  546,   1,   17,    DEFAULT_PWM_IDX}, /*  1 */
      {   1,  19,    2,   86,   1,   16,    DEFAULT_PWM_IDX}, /*  2 */
      {   0,  19,    3,  129,   1,    0,                  0}, /*  3 */
      {   1,  11,    6, 4021,   1,   15,    DEFAULT_PWM_IDX}, /*  4 */
      {   1,   8,   12,    0,   0,   14,    DEFAULT_PWM_IDX}, /*  5 */
      {   0,  23,    5,   35,   1,    0,                  0}, /*  6 */
      {   0,  27, 4004,    0,   1,    0,                  0}, /*  7 */
      {   1,  51,    3,   48,   1,   13,    DEFAULT_PWM_IDX}, /*  8 */
      {   0,  43,    4,   76,   1,    0,                  0}, /*  9 */
      {   1,   8,   24,    0,   0,   12,    DEFAULT_PWM_IDX}, /* 10 */
};

static const uint16_t pwmCycles[PWM_FREQS]=
   {  1,    2,    4,    5,    8,   10,   16,    20,    25,
     32,   40,   50,   80,  100,  160,  200,   400,   800};
 
static const uint16_t pwmRealRange[PWM_FREQS]=
   { 25,   50,  100,  125,  200,  250,  400,   500,   625,
    800, 1000, 1250, 2000, 2500, 4000, 5000, 10000, 20000};
 
/* ======================================================================= */

static void intNotifyBits(void);

static int  gpioNotifyOpenInBand(int fd);

static void myGpioSleep(int seconds, int micros)
{
   struct timespec ts, rem;

   ts.tv_sec  = seconds;
   ts.tv_nsec = micros * 1000;

   while (clock_nanosleep(CLOCK_REALTIME, 0, &ts, &rem))
   {
      /* copy remaining time to ts */
      ts.tv_sec  = rem.tv_sec;
      ts.tv_nsec = rem.tv_nsec;
   }
}

static uint32_t myGpioDelay(uint32_t micros)
{
   uint32_t start;

   start = systReg[SYST_CLO];

   if (micros < 101) while ((systReg[SYST_CLO] - start) <= micros) ;

   else myGpioSleep(micros/MILLION, micros%MILLION);

   return (systReg[SYST_CLO] - start);
}

static char * myTimeStamp()
{
   static struct timeval last;
   static char buf[32];
   struct timeval now;

   struct tm tmp;

   gettimeofday(&now, NULL);

   if (now.tv_sec != last.tv_sec)
   {
      localtime_r(&now.tv_sec, &tmp);
      strftime(buf, sizeof(buf), "%F %T", &tmp);
      last.tv_sec = now.tv_sec;
   }

   return buf;
}

/* ----------------------------------------------------------------------- */

static void myCreatePipe(char * name, int perm)
{
   unlink(name);

   mkfifo(name, perm);

   if (chmod(name, perm) < 0)
   {
      DBG(DBG_MIN_LEVEL, "Can't set permissions (%d) for %s, %m", perm, name);
      return;
   }
}

/* ----------------------------------------------------------------------- */

static void myOffPageSlot(int pos, int * page, int * slot)
{
   *page = pos/OFF_PER_IPAGE;
   *slot = pos%OFF_PER_IPAGE;
}

/* ----------------------------------------------------------------------- */

static void myLvsPageSlot(int pos, int * page, int * slot)
{
   *page = pos/LVS_PER_IPAGE;
   *slot = pos%LVS_PER_IPAGE;
}

/* ----------------------------------------------------------------------- */

static void myTckPageSlot(int pos, int * page, int * slot)
{
   *page = pos/TCK_PER_IPAGE;
   *slot = pos%TCK_PER_IPAGE;
}

/* ----------------------------------------------------------------------- */

static uint32_t myGetLevel(int pos)
{
   uint32_t level;
   int page, slot;

   myLvsPageSlot(pos, &page, &slot);

   level = dmaIVirt[page]->level[slot];

   return level;
}

/* ----------------------------------------------------------------------- */

static uint32_t myGetTick(int pos)
{
   uint32_t tick;
   int page, slot;

   myTckPageSlot(pos, &page, &slot);

   tick = dmaIVirt[page]->tick[slot];

   return tick;
}

/* ----------------------------------------------------------------------- */

static void myDoCommand
   (cmdCmd_t *cmd, gpioExtent_t *iExt, gpioExtent_t *oExt)
{
   int p1, p2, res, i;
   uint32_t mask, tmp;
   gpioPulse_t *pulse;
   int masked;

   p1  = cmd->p1;
   p2  = cmd->p2;
   res = cmd->res;

   switch (cmd->cmd)
   {
      case PI_CMD_BC1:
         mask = gpioMask;

         res = gpioWrite_Bits_0_31_Clear(p1&mask);

         if ((mask | p1) != mask)
         {
            PERM_ERROR(
               "gpioWrite_Bits_0_31_Clear: bad levels %08X (permissions %08X)",
               p1, mask);
            res = PI_SOME_PERMITTED;
         }
         break;

      case PI_CMD_BC2:
         mask = gpioMask>>32;

         res = gpioWrite_Bits_32_53_Clear(p1&mask);

         if ((mask | p1) != mask)
         {
            PERM_ERROR(
               "gpioWrite_Bits_32_53_Clear: bad levels %08X (permissions %08X)",
               p1, mask);
            res = PI_SOME_PERMITTED;
         }
         break;

      case PI_CMD_BR1: res = gpioRead_Bits_0_31(); break;

      case PI_CMD_BR2: res = gpioRead_Bits_32_53(); break;

      case PI_CMD_BS1:
         mask = gpioMask;

         res = gpioWrite_Bits_0_31_Set(p1&mask);

         if ((mask | p1) != mask)
         {
            PERM_ERROR(
               "gpioWrite_Bits_0_31_Set: bad levels %08X (permissions %08X)",
               p1, mask);
            res = PI_SOME_PERMITTED;
         }
         break;

      case PI_CMD_BS2:
         mask = gpioMask>>32;

         res = gpioWrite_Bits_32_53_Set(p1&mask);

         if ((mask | p1) != mask)
         {
            PERM_ERROR(
               "gpioWrite_Bits_32_53_Set: bad levels %08X (permissions %08X)",
               p1, mask);
            res = PI_SOME_PERMITTED;
         }
         break;

      case PI_CMD_HELP: break;

      case PI_CMD_HWVER: res = gpioHardwareRevision(); break;

      case PI_CMD_MODEG: res = gpioGetMode(p1); break;

      case PI_CMD_MODES:
         if (gpioMask & (uint64_t)(1<<p1)) res = gpioSetMode(p1, p2);
         else
         {
            PERM_ERROR("gpioSetMode: gpio %d, no permission to update", p1);
            res = PI_NOT_PERMITTED;
         }
         break;

      case PI_CMD_NB: res = gpioNotifyBegin(p1, p2); break;

      case PI_CMD_NC: res = gpioNotifyClose(p1); break;

      case PI_CMD_NO: res = gpioNotifyOpen();  break;

      case PI_CMD_NP: res = gpioNotifyPause(p1); break;

      case PI_CMD_PFG: res = gpioGetPWMfrequency(p1); break;

      case PI_CMD_PFS:
         if (gpioMask & (uint64_t)(1<<p1)) res = gpioSetPWMfrequency(p1, p2);
         else
         {
            PERM_ERROR(
               "gpioSetPWMfrequency: gpio %d, no permission to update", p1);
            res = PI_NOT_PERMITTED;
         }
         break;

      case PI_CMD_PIGPV: res = gpioVersion(); break;

      case PI_CMD_PRG: res = gpioGetPWMrange(p1); break;

      case PI_CMD_PROC:
         res = gpioStoreScript(iExt[0].ptr);
         break;

      case PI_CMD_PROCD: res = gpioDeleteScript(p1); break;

      case PI_CMD_PROCR: res = gpioRunScript(p1); break;

      case PI_CMD_PROCS: res = gpioStopScript(p1); break;

      case PI_CMD_PRRG: res = gpioGetPWMrealRange(p1); break;

      case PI_CMD_PRS:
         if (gpioMask & (uint64_t)(1<<p1)) res = gpioSetPWMrange(p1, p2);
         else
         {
            PERM_ERROR(
               "gpioSetPWMrange: gpio %d, no permission to update", p1);
            res = PI_NOT_PERMITTED;
         }
         break;

      case PI_CMD_PUD:
         if (gpioMask & (uint64_t)(1<<p1)) res = gpioSetPullUpDown(p1, p2);
         else
         {
            PERM_ERROR(
               "gpioSetPullUpDown: gpio %d, no permission to update", p1);
            res = PI_NOT_PERMITTED;
         }
         break;

      case PI_CMD_PWM:
         if (gpioMask & (uint64_t)(1<<p1)) res = gpioPWM(p1, p2);
         else
         {
            PERM_ERROR("gpioPWM: gpio %d, no permission to update", p1);
            res = PI_NOT_PERMITTED;
         }
         break;

      case PI_CMD_READ: res = gpioRead(p1); break;

      case PI_CMD_SERVO:
         if (gpioMask & (uint64_t)(1<<p1)) res = gpioServo(p1, p2);
         else
         {
            PERM_ERROR("gpioServo: gpio %d, no permission to update", p1);
            res = PI_NOT_PERMITTED;
         }
         break;

      case PI_CMD_SLRO: res = gpioSerialReadOpen(p1, p2); break;

      case PI_CMD_SLR:
         if (p2 < oExt[0].size) oExt[0].size = p2;
         res = gpioSerialRead(p1, oExt[0].ptr, oExt[0].size);
         break;

      case PI_CMD_SLRC: res = gpioSerialReadClose(p1); break;

      case PI_CMD_TICK: res = gpioTick(); break;

      case PI_CMD_TRIG:
         if (gpioMask & (uint64_t)(1<<p1))
            res = gpioTrigger(p1, p2, *(int *) (iExt[0].ptr));
         else
         {
            PERM_ERROR("gpioTrigger: gpio %d, no permission to update", p1);
            res = PI_NOT_PERMITTED;
         }
         break;

      case PI_CMD_WDOG: res = gpioSetWatchdog(p1, p2); break;

      case PI_CMD_WRITE:
         if (gpioMask & (uint64_t)(1<<p1)) res = gpioWrite(p1, p2);
         else
         {
            PERM_ERROR("gpioWrite: gpio %d, no permission to update", p1);
            res = PI_NOT_PERMITTED;
         }
         break;

      case PI_CMD_WVAG:

         /* need to mask off any non permitted gpios */

         mask = gpioMask;
         pulse = iExt[0].ptr;
         masked = 0;

         for (i=0; i<p1; i++)
         {
            tmp = pulse[i].gpioOn & mask;
            if (tmp != pulse[i].gpioOn)
            {
               pulse[i].gpioOn = tmp;
               masked = 1;
            }

            tmp = pulse[i].gpioOff & mask;
            if (tmp != pulse[i].gpioOff)
            {
               pulse[i].gpioOff = tmp;
               masked = 1;
            }
         }

         res = gpioWaveAddGeneric(p1, pulse);

         /* report permission error unless another error occurred */
         if (masked && (res >= 0)) res = PI_SOME_PERMITTED;

         break;

      case PI_CMD_WVAS:
         if (gpioMask & (uint64_t)(1<<p1))
            res = gpioWaveAddSerial
               (p1,
                *(int *)(iExt[0].ptr),
                *(int *)(iExt[1].ptr),
                p2,
                iExt[2].ptr);
         else
         {
            PERM_ERROR
               ("gpioWaveAddSerial: gpio %d, no permission to update", p1);
            res = PI_NOT_PERMITTED;
         }
         break;

      case PI_CMD_WVBSY: res = gpioWaveTxBusy(); break;

      case PI_CMD_WVCLR: res = gpioWaveClear(); break;

      case PI_CMD_WVGO:  res = gpioWaveTxStart(PI_WAVE_MODE_ONE_SHOT); break;

      case PI_CMD_WVGOR: res = gpioWaveTxStart(PI_WAVE_MODE_REPEAT); break;

      case PI_CMD_WVHLT: res = gpioWaveTxStop(); break;

      case PI_CMD_WVSC:
         switch(p1)
         {
            case 0: res = gpioWaveGetCbs();     break;
            case 1: res = gpioWaveGetHighCbs(); break;
            case 2: res = gpioWaveGetMaxCbs();  break;
            default: res = -9999;
         }
         break;

      case PI_CMD_WVSM:
         switch(p1)
         {
            case 0: res = gpioWaveGetMicros();     break;
            case 1: res = gpioWaveGetHighMicros(); break;
            case 2: res = gpioWaveGetMaxMicros();  break;
            default: res = -9999;
         }
         break;

      case PI_CMD_WVSP:
         switch(p1)
         {
            case 0: res = gpioWaveGetPulses();     break;
            case 1: res = gpioWaveGetHighPulses(); break;
            case 2: res = gpioWaveGetMaxPulses();  break;
            default: res = -9999;
         }
         break;
   }
   cmd->res = res;
}

/* ----------------------------------------------------------------------- */

static void mySetGpioOff(unsigned gpio, int pos)
{
   int page, slot;

   myOffPageSlot(pos, &page, &slot);

   dmaIVirt[page]->gpioOff[slot] |= (1<<gpio);
}

/* ----------------------------------------------------------------------- */

static void myClearGpioOff(unsigned gpio, int pos)
{
   int page, slot;

   myOffPageSlot(pos, &page, &slot);

   dmaIVirt[page]->gpioOff[slot] &= ~(1<<gpio);
}

/* ----------------------------------------------------------------------- */

static void mySetGpioOn(unsigned gpio, int pos)
{
   int page, slot;

   page = pos/ON_PER_IPAGE;
   slot = pos%ON_PER_IPAGE;

   dmaIVirt[page]->gpioOn[slot] |= (1<<gpio);
}

/* ----------------------------------------------------------------------- */

static void myClearGpioOn(unsigned gpio, int pos)
{
   int page, slot;

   page = pos/ON_PER_IPAGE;
   slot = pos%ON_PER_IPAGE;

   dmaIVirt[page]->gpioOn[slot] &= ~(1<<gpio);
}

/* ----------------------------------------------------------------------- */

static void myGpioSetPwm(unsigned gpio, int oldVal, int newVal)
{
   int switchGpioOff;
   int newOff, oldOff, realRange, cycles, i;

   DBG(DBG_INTERNAL,
      "myGpioSetPwm %d from %d to %d", gpio, oldVal, newVal);

   switchGpioOff = 0;

   realRange = pwmRealRange[gpioInfo[gpio].freqIdx];

   cycles    = pwmCycles   [gpioInfo[gpio].freqIdx];

   newOff = (newVal * realRange)/gpioInfo[gpio].range;
   oldOff = (oldVal * realRange)/gpioInfo[gpio].range;

   if (newOff != oldOff)
   {
      if (newOff && oldOff)                      /* PWM CHANGE */
      {
         for (i=0; i<SUPERLEVEL; i+=realRange)
            mySetGpioOff(gpio, i+newOff);

         for (i=0; i<SUPERLEVEL; i+=realRange)
            myClearGpioOff(gpio, i+oldOff);
      }
      else if (newOff)                           /* PWM START */
      {
         for (i=0; i<SUPERLEVEL; i+=realRange)
            mySetGpioOff(gpio, i+newOff);

         /* schedule new gpio on */

         for (i=0; i<SUPERCYCLE; i+=cycles) mySetGpioOn(gpio, i);
      }
      else                                       /* PWM STOP */
      {
         /* deschedule gpio on */

         for (i=0; i<SUPERCYCLE; i+=cycles)
            myClearGpioOn(gpio, i);

         for (i=0; i<SUPERLEVEL; i+=realRange)
            myClearGpioOff(gpio, i+oldOff);

         switchGpioOff = 1;
      }

      if (switchGpioOff)
      {
         *(gpioReg + GPCLR0) = (1<<gpio);
         *(gpioReg + GPCLR0) = (1<<gpio);
      }
   }
}

/* ----------------------------------------------------------------------- */

static void myGpioSetServo(unsigned gpio, int oldVal, int newVal)
{
   int switchGpioOff;
   int newOff, oldOff, realRange, cycles, i;

   DBG(DBG_INTERNAL,
      "myGpioSetServo %d from %d to %d", gpio, oldVal, newVal);

   switchGpioOff = 0;

   realRange = pwmRealRange[gpioInfo[gpio].freqIdx];
   cycles    = pwmCycles   [gpioInfo[gpio].freqIdx];

   newOff = (newVal * realRange)/20000;
   oldOff = (oldVal * realRange)/20000;

   if (newOff != oldOff)
   {
      if (newOff && oldOff)                       /* SERVO CHANGE */
      {
         for (i=0; i<SUPERLEVEL; i+=realRange)
            mySetGpioOff(gpio, i+newOff);

         for (i=0; i<SUPERLEVEL; i+=realRange)
            myClearGpioOff(gpio, i+oldOff);
      }
      else if (newOff)                            /* SERVO START */
      {
         for (i=0; i<SUPERLEVEL; i+=realRange)
            mySetGpioOff(gpio, i+newOff);

         /* schedule new gpio on */

         for (i=0; i<SUPERCYCLE; i+=cycles)
            mySetGpioOn(gpio, i);
      }
      else                                        /* SERVO STOP */
      {
         /* deschedule gpio on */

         for (i=0; i<SUPERCYCLE; i+=cycles)
            myClearGpioOn(gpio, i);

         for (i=0; i<SUPERLEVEL; i+=realRange)
            myClearGpioOff(gpio, i+oldOff);

         switchGpioOff = 1;
      }

      if (switchGpioOff)
      {
         *(gpioReg + GPCLR0) = (1<<gpio);
         *(gpioReg + GPCLR0) = (1<<gpio);
      }
   }
}

/* ======================================================================= */

static dmaCbs_t * waveCbVOadr(int pos)
{
   int page, slot;

   page = pos/CBS_PER_OPAGE;
   slot = pos%CBS_PER_OPAGE;

   return &dmaOVirt[page]->cb[slot];
}


/* ----------------------------------------------------------------------- */

static uint32_t waveCbPOadr(int pos)
{
   int page, slot;

   page = pos/CBS_PER_OPAGE;
   slot = pos%CBS_PER_OPAGE;

   return (uint32_t) &dmaOPhys[page]->cb[slot];
}


/* ----------------------------------------------------------------------- */

static uint32_t waveOnOffPOadr(int pos)
{
   int page, slot;

   page = pos/ONOFF_PER_OPAGE;
   slot = pos%ONOFF_PER_OPAGE;

   return (uint32_t) &dmaOPhys[page]->gpioOnOff[slot];
}


/* ----------------------------------------------------------------------- */

static void waveCbOPrint(int pos)
{
   dmaCbs_t * p;

   p = waveCbVOadr(pos);

   fprintf(stderr, "i=%lx s=%lx d=%lx len=%lx s=%lx nxt=%lx\n",
      p->info, p->src, p->dst, p->length, p->stride, p->next);
}


/* ----------------------------------------------------------------------- */

void waveBitDelay(unsigned baud, unsigned * bitDelay)
{
   unsigned fullBit, halfBit, s, e, d, m, i, err;

   fullBit = 100000000 / baud;
   halfBit =  50000000 / baud;

   d = (fullBit/200)*200;

   s = 0;

   e = d;

   bitDelay[0] = d/100;

   err = d / 3;

   for (i=0; i<8; i++)
   {
      s = e;

      m = halfBit + (i+1)*fullBit;

      e = s + d;

      if ((e-m) < err) e+=200;

      bitDelay[i+1] = (e-s)/100;
   }

   s = e;

   e = ((1000000000 / baud)+100)/200*200;

   bitDelay[9] = (e-s)/100;
}

/* ----------------------------------------------------------------------- */

void gpioWaveDump(void)
{
   int i;

   unsigned numPulses, t;

   gpioPulse_t * pulses;

   numPulses = wfc[wfcur];
   pulses    = wf [wfcur];

   t = 0;

   for (i=0; i<numPulses; i++)
   {
      printf("%10u %08X %08X %10u\n",
         t, pulses[i].gpioOn, pulses[i].gpioOff, pulses[i].usDelay);
      t += pulses[i].usDelay;
   }
}



/* ----------------------------------------------------------------------- */

static int wave2Cbs(unsigned mode)
{
   int cb=0, onoff=0;

   dmaCbs_t * p=NULL;

   unsigned i, half, repeatCb;

   unsigned numPulses;

   gpioPulse_t * pulses;

   numPulses = wfc[wfcur];
   pulses    = wf [wfcur];

   half = PI_WF_MICROS/2;

   /* add delay cb at start of DMA */

   p = waveCbVOadr(cb++);

   /* use the secondary clock */

   if (gpioCfg.clockPeriph != PI_CLOCK_PCM)
   {
      p->info   = NORMAL_DMA |
                  DMA_DEST_DREQ |
                  DMA_PERIPHERAL_MAPPING(2);

      p->dst    = ((PCM_BASE + PCM_FIFO*4) & 0x00ffffff) | 0x7e000000;
   }
   else
   {
      p->info   = NORMAL_DMA |
                  DMA_DEST_DREQ |
                  DMA_PERIPHERAL_MAPPING(5);

      p->dst    = ((PWM_BASE + PWM_FIFO*4) & 0x00ffffff) | 0x7e000000;
   }

   p->src    = (uint32_t) (&dmaOPhys[0]->periphData) | DMA_BUS_ADR;
   p->length = 4 * 50 / PI_WF_MICROS; /* 50 micros delay */
   p->next   = waveCbPOadr(cb) | DMA_BUS_ADR;

   repeatCb = cb;

   for (i=0; i<numPulses; i++)
   {
      if (pulses[i].gpioOn)
      {
         dmaOVirt[onoff/ONOFF_PER_OPAGE]->gpioOnOff[onoff%ONOFF_PER_OPAGE] =
            pulses[i].gpioOn;

         p = waveCbVOadr(cb++);

         p->info   = NORMAL_DMA;
         p->src    = waveOnOffPOadr(onoff++) | DMA_BUS_ADR;
         p->dst    = ((GPIO_BASE + (GPSET0*4)) & 0x00ffffff) | 0x7e000000;
         p->length = 4;
         p->next   = waveCbPOadr(cb) | DMA_BUS_ADR;
      }

      if (pulses[i].gpioOff)
      {
         dmaOVirt[onoff/ONOFF_PER_OPAGE]->gpioOnOff[onoff%ONOFF_PER_OPAGE] =
            pulses[i].gpioOff;

         p = waveCbVOadr(cb++);

         p->info   = NORMAL_DMA;
         p->src    = waveOnOffPOadr(onoff++) | DMA_BUS_ADR;
         p->dst    = ((GPIO_BASE + (GPCLR0*4)) & 0x00ffffff) | 0x7e000000;
         p->length = 4;
         p->next   = waveCbPOadr(cb) | DMA_BUS_ADR;
      }

      if (pulses[i].usDelay)
      {
         p = waveCbVOadr(cb++);

         /* use the secondary clock */

         if (gpioCfg.clockPeriph != PI_CLOCK_PCM)
         {
            p->info   = NORMAL_DMA |
                        DMA_DEST_DREQ |
                        DMA_PERIPHERAL_MAPPING(2);

            p->dst    = ((PCM_BASE + PCM_FIFO*4) & 0x00ffffff) | 0x7e000000;
         }
         else
         {
            p->info   = NORMAL_DMA |
                        DMA_DEST_DREQ |
                        DMA_PERIPHERAL_MAPPING(5);

            p->dst    = ((PWM_BASE + PWM_FIFO*4) & 0x00ffffff) | 0x7e000000;
         }

         p->src    = (uint32_t) (&dmaOPhys[0]->periphData) | DMA_BUS_ADR;
         p->length = 4 * ((pulses[i].usDelay+half)/PI_WF_MICROS);
         p->next   = waveCbPOadr(cb) | DMA_BUS_ADR;
      }
   }

   if (p != NULL)
   {
      if (mode == PI_WAVE_MODE_ONE_SHOT)
           p->next = 0;
      else p->next = waveCbPOadr(repeatCb) | DMA_BUS_ADR;
   }

   return cb;
}


/* ----------------------------------------------------------------------- */

static void waveRxSerial(volatile wfRx_t *s, int level, uint32_t tick)
{
   int diffTicks;
   int newWritePos;

   if (s->bit >= 0)
   {
      diffTicks = tick - s->startBitTick;

      if (level != PI_TIMEOUT) s->level = level;

      while ((s->bit < 9) && (diffTicks > s->nextBitDiff))
      {
         if (s->bit)
         {
            if (!(s->level)) s->byte |= (1<<(s->bit-1));
         }
         else s->byte = 0;

         ++(s->bit);

         s->nextBitDiff += s->fullBit;
      }

      if (s->bit == 9)
      {
         s->buf[s->writePos] = s->byte;

         /* don't let writePos catch readPos */

         newWritePos = s->writePos;

         if (++newWritePos >= s->bufSize) newWritePos = 0;

         if (newWritePos != s->readPos) s->writePos = newWritePos;

         if (level == 0) /* true transition high->low, not a timeout */
         {
            s->bit          = 0;
            s->startBitTick = tick;
            s->nextBitDiff  = s->halfBit;
         }
         else
         {
            s->bit = -1;
            gpioSetWatchdog(s->gpio, 0);
         }
      }
   }
   else
   {
      /* start bit if high->low */

      if (level == 0)
      {
         gpioSetWatchdog(s->gpio, s->timeout);
         s->level        = 0;
         s->bit          = 0;
         s->startBitTick = tick;
         s->nextBitDiff  = s->halfBit;
      }
   }
}


/* ----------------------------------------------------------------------- */

static void waveRxBit(int gpio, int level, uint32_t tick)
{
   switch (wfRx[gpio].mode)
   {
      case PI_WFRX_NONE:
         break;

      case PI_WFRX_SERIAL:
         waveRxSerial(&wfRx[gpio], level, tick);
   }
}


/* ----------------------------------------------------------------------- */

static int waveMerge(unsigned numIn1, gpioPulse_t * in1)
{
   unsigned inPos1=0, inPos2=0, outPos=0;

   unsigned cbs=0;

   unsigned numIn2, numOut;

   uint32_t tNow, tNext1, tNext2, tDelay;

   gpioPulse_t * in2, * out;

   numIn2 = wfc[wfcur];
   in2    = wf[wfcur];

   numOut = PI_WAVE_MAX_PULSES;
   out   = wf[1-wfcur];

   tNow = 0;
  
   if (!numIn1) tNext1 = -1; else tNext1 = 0;
   if (!numIn2) tNext2 = -1; else tNext2 = 0;

   while (((inPos1<numIn1) || (inPos2<numIn2)) && (outPos<numOut))
   {
      if (tNext1 < tNext2)
      {
         /* pulse 1 due */

         if (tNow < tNext1)
         {
            /* extend previous delay */
            out[outPos-1].usDelay += (tNext1 - tNow);
            tNow = tNext1;
         }

         out[outPos].gpioOn  = in1[inPos1].gpioOn;
         out[outPos].gpioOff = in1[inPos1].gpioOff;

         tNext1 = tNow + in1[inPos1].usDelay; ++inPos1;
      }
      else if (tNext2 < tNext1)
      {
         /* pulse 2 due */

         if (tNow < tNext2)
         {
            /* extend previous delay */
            out[outPos-1].usDelay += (tNext2 - tNow);
            tNow = tNext2;
         }

         out[outPos].gpioOn  = in2[inPos2].gpioOn;
         out[outPos].gpioOff = in2[inPos2].gpioOff;

         tNext2 = tNow + in2[inPos2].usDelay; ++inPos2;
      }
      else
      {
         /* pulse 1 and 2 both due */

         if (tNow < tNext1)
         {
            /* extend previous delay */
            out[outPos-1].usDelay += (tNext1 - tNow);
            tNow = tNext1;
         }

         out[outPos].gpioOn  = in1[inPos1].gpioOn  | in2[inPos2].gpioOn;
         out[outPos].gpioOff = in1[inPos1].gpioOff | in2[inPos2].gpioOff;

         tNext1 = tNow + in1[inPos1].usDelay; ++inPos1;
         tNext2 = tNow + in2[inPos2].usDelay; ++inPos2;
      }

      if (tNext1 <= tNext2) { tDelay = tNext1 - tNow; tNow = tNext1; }
      else                  { tDelay = tNext2 - tNow; tNow = tNext2; }

      out[outPos].usDelay = tDelay;

      cbs++; /* one cb for delay */

      if (out[outPos].gpioOn) cbs++; /* one cb if gpio on */

      if (out[outPos].gpioOff) cbs++; /* one cb if gpio off */

      outPos++;

      if (inPos1 >= numIn1) tNext1 = -1;
      if (inPos2 >= numIn2) tNext2 = -1;

   }

   if (outPos < numOut)
   {
      wfStats.micros = tNow;

      if (tNow > wfStats.highMicros) wfStats.highMicros = tNow;

      wfStats.pulses = outPos;

      if (outPos > wfStats.highPulses) wfStats.highPulses = outPos;

      wfStats.cbs    = cbs;

      if (cbs > wfStats.highCbs) wfStats.highCbs = cbs;

      wfc[1-wfcur] = outPos;
      wfcur = 1 - wfcur;

      return outPos;
   }
   else return PI_TOO_MANY_PULSES;
}


/* ======================================================================= */

static dmaCbs_t * dmaCB2adr(int pos)
{
   int page, slot;

   page = pos/CBS_PER_IPAGE;
   slot = pos%CBS_PER_IPAGE;

   return &dmaIVirt[page]->cb[slot];
}

/* ----------------------------------------------------------------------- */

static void dmaCbPrint(int pos)
{
   dmaCbs_t * p;

   p = dmaCB2adr(pos);

   fprintf(stderr, "i=%lx s=%lx d=%lx len=%lx s=%lx nxt=%lx\n",
      p->info, p->src, p->dst, p->length, p->stride, p->next);
}

/* ----------------------------------------------------------------------- */

static unsigned dmaCurrentCb(void)
{
   unsigned cb;
   static unsigned lastPage=0;
   unsigned page;
   uint32_t cbAddr;
   uint32_t startTick, endTick;

   startTick = systReg[SYST_CLO];

   cbAddr = dmaIn[DMA_CONBLK_AD];

   page = lastPage;

   /* which page are we dma'ing? */

   while (1)
   {
      cb = (cbAddr - ((int)dmaIPhys[page] | DMA_BUS_ADR)) / 32;

      if (cb < CBS_PER_IPAGE)
      {
         endTick = systReg[SYST_CLO];

         if (endTick != startTick)
              gpioStats.cbTicks += (endTick - startTick);
         else gpioStats.cbTicks ++;

         gpioStats.cbCalls++;

         lastPage = page;

         return (page*CBS_PER_IPAGE) + cb;
      }

      if (page++ >= DMA_PAGES) page=0;

      if (page == lastPage) break;
   }

   return 0;
}

/* ----------------------------------------------------------------------- */

static unsigned dmaCurrentSlot(unsigned pos)
{
   unsigned cycle=0, slot=0, tmp;

   cycle = (pos/CBS_PER_CYCLE);
   tmp   = (pos%CBS_PER_CYCLE);

   if (tmp > 2) slot = ((tmp-2)/3);

   return (cycle*PULSE_PER_CYCLE)+slot;
}

/* ----------------------------------------------------------------------- */

static uint32_t dmaPwmDataAdr(int pos)
{
   return (uint32_t) &dmaIPhys[pos]->periphData;
}

static uint32_t dmaGpioOnAdr(int pos)
{
   int page, slot;

   page = pos/ON_PER_IPAGE;
   slot = pos%ON_PER_IPAGE;

   return (uint32_t) &dmaIPhys[page]->gpioOn[slot];
}

/* ----------------------------------------------------------------------- */

static uint32_t dmaGpioOffAdr(int pos)
{
   int page, slot;

   myOffPageSlot(pos, &page, &slot);

   return (uint32_t) &dmaIPhys[page]->gpioOff[slot];
}

/* ----------------------------------------------------------------------- */

static uint32_t dmaTickAdr(int pos)
{
   int page, slot;

   myTckPageSlot(pos, &page, &slot);

   return (uint32_t) &dmaIPhys[page]->tick[slot];
}

/* ----------------------------------------------------------------------- */

static uint32_t dmaReadLevelsAdr(int pos)
{
   int page, slot;

   myLvsPageSlot(pos, &page, &slot);

   return (uint32_t) &dmaIPhys[page]->level[slot];
}

/* ----------------------------------------------------------------------- */

static uint32_t dmaCbAdr(int pos)
{
   int page, slot;

   page = (pos/CBS_PER_IPAGE);
   slot = (pos%CBS_PER_IPAGE);

   return (uint32_t) &dmaIPhys[page]->cb[slot];
}

/* ----------------------------------------------------------------------- */

static void dmaGpioOnCb(int b, int pos)
{   
   dmaCbs_t * p;

   p = dmaCB2adr(b);

   p->info   = NORMAL_DMA;
   p->src    = dmaGpioOnAdr(pos) | DMA_BUS_ADR;
   p->dst    = ((GPIO_BASE + (GPSET0*4)) & 0x00ffffff) | 0x7e000000;
   p->length = 4;
   p->next   = dmaCbAdr(b+1) | DMA_BUS_ADR;
}

/* ----------------------------------------------------------------------- */

static void dmaTickCb(int b, int pos)
{   
   dmaCbs_t * p;

   p = dmaCB2adr(b);

   p->info   = NORMAL_DMA;
   p->src    = ((SYST_BASE + (SYST_CLO*4)) & 0x00ffffff) | 0x7e000000;
   p->dst    = dmaTickAdr(pos) | DMA_BUS_ADR;
   p->length = 4;
   p->next   = dmaCbAdr(b+1) | DMA_BUS_ADR;
}

/* ----------------------------------------------------------------------- */

static void dmaGpioOffCb(int b, int pos)
{
   dmaCbs_t * p;

   p = dmaCB2adr(b);

   p->info   = NORMAL_DMA;
   p->src    = dmaGpioOffAdr(pos) | DMA_BUS_ADR;
   p->dst    = ((GPIO_BASE + (GPCLR0*4)) & 0x00ffffff) | 0x7e000000;
   p->length = 4;
   p->next   = dmaCbAdr(b+1) | DMA_BUS_ADR;
}

/* ----------------------------------------------------------------------- */

static void dmaReadLevelsCb(int b, int pos)
{
   dmaCbs_t * p;

   p = dmaCB2adr(b);

   p->info   = NORMAL_DMA;
   p->src    = ((GPIO_BASE + (GPLEV0*4)) & 0x00ffffff) | 0x7e000000;
   p->dst    = dmaReadLevelsAdr(pos) | DMA_BUS_ADR;
   p->length = 4;
   p->next   = dmaCbAdr(b+1) | DMA_BUS_ADR;
}

/* ----------------------------------------------------------------------- */

static void dmaDelayCb(int b)
{
   dmaCbs_t * p;

   p = dmaCB2adr(b);

   if (gpioCfg.clockPeriph == PI_CLOCK_PCM)
   {
      p->info   = NORMAL_DMA | TIMED_DMA(2);
      p->dst    = ((PCM_BASE + PCM_FIFO*4) & 0x00ffffff) | 0x7e000000;
   }
   else
   {
      p->info   = NORMAL_DMA | TIMED_DMA(5);
      p->dst    = ((PWM_BASE + PWM_FIFO*4) & 0x00ffffff) | 0x7e000000;
   }

   p->src    = dmaPwmDataAdr(b%DMA_PAGES) | DMA_BUS_ADR;
   p->length = 4;
   p->next   = dmaCbAdr(b+1) | DMA_BUS_ADR;
}

/* ----------------------------------------------------------------------- */

static void dmaInitCbs(void)
{
   int b, pulse, level, cycle;

   dmaCbs_t * p;

   /* set up the DMA control blocks */

   DBG(DBG_STARTUP, "");

   b = -1;
   level = 0;

   for (cycle=0; cycle<bufferCycles; cycle++)
   {
      b++; dmaGpioOnCb(b, cycle%SUPERCYCLE); /* gpio on slot */

      b++; dmaTickCb(b, cycle);              /* tick slot */

      for (pulse=0; pulse<PULSE_PER_CYCLE; pulse++)
      {
         b++; dmaReadLevelsCb(b, level);               /* read levels slot */

         b++; dmaDelayCb(b);                           /* delay slot */

         b++; dmaGpioOffCb(b, (level%SUPERLEVEL)+1);   /* gpio off slot */

         ++level;
      }
   }

   /* point last cb back to first for continuous loop */

   p = dmaCB2adr(b);

   p->next = dmaCbAdr(0) | DMA_BUS_ADR;

   DBG(DBG_STARTUP, "DMA page type size = %d", sizeof(dmaIPage_t));

   DBG(DBG_STARTUP, "%d control blocks (exp=%d)", b+1, NUM_CBS);
}

/* ======================================================================= */


static void sigHandler(int signum)
{
   if ((signum >= PI_MIN_SIGNUM) && (signum <= PI_MAX_SIGNUM))
   {
      if (gpioSignal[signum].func)
      {
         if (gpioSignal[signum].ex)
         {
            (gpioSignal[signum].func)(signum, gpioSignal[signum].userdata);
         }
         else
         {
            (gpioSignal[signum].func)(signum);
         }
      }
      else
      {
         if (signum == SIGUSR1)
         {
            if (gpioCfg.dbgLevel > DBG_MIN_LEVEL)
            {
               --gpioCfg.dbgLevel;
            }
            else gpioCfg.dbgLevel = DBG_MIN_LEVEL;

            DBG(DBG_USER, "Debug level %d\n", gpioCfg.dbgLevel);
         }
         else if (signum == SIGUSR2)
         {
            if (gpioCfg.dbgLevel < DBG_MAX_LEVEL)
            {
               ++gpioCfg.dbgLevel;
            }
            else gpioCfg.dbgLevel = DBG_MAX_LEVEL;

            DBG(DBG_USER, "Debug level %d\n", gpioCfg.dbgLevel);
         }
         else if (signum == SIGPIPE)
         {
            DBG(DBG_USER, "SIGPIPE received");
         }
         else
         {
            /* exit */

            DBG(DBG_MIN_LEVEL, "Unhandled signal %d, terminating\n", signum);

            exit(-1);
         }
      }
   }
   else
   {
      /* exit */

      DBG(DBG_MIN_LEVEL, "Unhandled signal %d, terminating\n", signum);

      exit(-1);
   }
}

/* ----------------------------------------------------------------------- */

static void sigSetHandler(void)
{
   int i;
   struct sigaction new;

   for (i=PI_MIN_SIGNUM; i<=PI_MAX_SIGNUM; i++)
   {

      memset(&new, 0, sizeof(new));
      new.sa_handler = sigHandler;

      sigaction(i, &new, NULL);
   }
}

/* ======================================================================= */

static void * pthAlertThread(void *x)
{
   struct timespec req, rem;
   uint32_t oldLevel, newLevel, level, reportedLevel;
   uint32_t oldSlot,  newSlot;
   uint32_t tick, expected;
   int32_t diff;
   int cycle, pulse;
   int emit, seqno, emitted;
   uint32_t changes, bits, changedBits, timeoutBits;
   int numSamples, d;
   int b, n, v;
   int err;
   char fifo[32];

   req.tv_sec = 0;

   /* don't start until DMA started */

   while (!DMAstarted) myGpioDelay(1000);

   myGpioDelay(20000); /* let DMA run for a while */

   reportedLevel = gpioReg[GPLEV0];

   tick = systReg[SYST_CLO];

   gpioStats.startTick = tick;

   oldSlot = dmaCurrentSlot(dmaCurrentCb());

   cycle = (oldSlot/PULSE_PER_CYCLE);
   pulse = (oldSlot%PULSE_PER_CYCLE);

   while (1)
   {
      gpioStats.alertTicks++;

      req.tv_nsec = 850000;

      while (nanosleep(&req, &rem))
      {
         req.tv_sec  = rem.tv_sec;
         req.tv_nsec = rem.tv_nsec;
      }

      newSlot = dmaCurrentSlot(dmaCurrentCb());

      numSamples = 0;

      changedBits = 0;

      oldLevel = reportedLevel & monitorBits;

      while ((oldSlot != newSlot) && (numSamples < DATUMS))
      {
         level = myGetLevel(oldSlot++);

         newLevel = (level & monitorBits);

         if (newLevel != oldLevel)
         {
            gpioSample[numSamples].tick  = tick;
            gpioSample[numSamples].level = level;

            changedBits |= (newLevel ^ oldLevel);

            oldLevel = newLevel;

            numSamples++;
         }
         
         tick += gpioCfg.clockMicros;

         if (++pulse >= PULSE_PER_CYCLE)
         {
            pulse = 0;

            if (++cycle >= bufferCycles)
            {
               cycle = 0;
               oldSlot = 0;
            }
            
            expected = tick;

            tick = myGetTick(cycle);

            diff = tick - expected;

            diff += (TICKSLOTS/2);

            if (diff < 0) gpioStats.diffTick[0]++;

            else if (diff >= TICKSLOTS)
               gpioStats.diffTick[TICKSLOTS-1]++;

            else gpioStats.diffTick[diff]++;
         }
      }

      /* should gpioGetSamples be called */

      if (changedBits)
      {
         if (gpioGetSamples.func)
         {
            if (gpioGetSamples.ex)
            {
               (gpioGetSamples.func)
                  (gpioSample, numSamples, gpioGetSamples.userdata);
            }
            else
            {
               (gpioGetSamples.func)
                  (gpioSample, numSamples);
            }
         }
      }

      /* reset timeouts for any changed bits */

      if (changedBits)
      {
         for (b=0; b<=PI_MAX_USER_GPIO; b++)
         {
            if (changedBits & (1<<b)) gpioAlert[b].tick = tick;
         }
      }

      /* call alert callbacks for each bit transition */

      if (changedBits & alertBits)
      {
         oldLevel = reportedLevel & alertBits;

         for (d=0; d<numSamples; d++)
         {
            newLevel = gpioSample[d].level & alertBits;

            if (newLevel != oldLevel)
            {
               changes = newLevel ^ oldLevel;

               for (b=0; b<=PI_MAX_USER_GPIO; b++)
               {
                  if (changes & (1<<b))
                  {
                     if (newLevel & (1<<b)) v = 1; else v = 0;

                     if (gpioAlert[b].func)
                     {
                        if (gpioAlert[b].ex)
                        {
                           (gpioAlert[b].func)
                              (b, v, gpioSample[d].tick,
                               gpioAlert[b].userdata);
                        }
                        else
                        {
                           (gpioAlert[b].func)
                              (b, v, gpioSample[d].tick);
                        }
                     }
                  }
               }
               oldLevel = newLevel;
            }
         }
      }

      /* check for timeout watchdogs */

      timeoutBits = 0;

      for (b=0; b<=PI_MAX_USER_GPIO; b++)
      {
         if (gpioAlert[b].timeout)
         {
            diff = tick - gpioAlert[b].tick;

            if (diff > (gpioAlert[b].timeout*1000))
            {
               timeoutBits |= (1<<b);

               gpioAlert[b].tick += (gpioAlert[b].timeout*1000);

               if (gpioAlert[b].func)
               {
                  if (gpioAlert[b].ex)
                  {
                     (gpioAlert[b].func)(b, 2, tick, gpioAlert[b].userdata);
                  }
                  else
                  {
                     (gpioAlert[b].func)(b, 2, tick);
                  }
               }
            }
         }
      }

      for (n=0; n<PI_NOTIFY_SLOTS; n++)
      {
         if (gpioNotify[n].state == PI_NOTIFY_CLOSING)
         {
            if (gpioNotify[n].pipe)
            {
               close(gpioNotify[n].fd);

               sprintf(fifo, "/dev/pigpio%d", n);

               unlink(fifo);
            }

            gpioNotify[n].state = PI_NOTIFY_CLOSED;
         }
         else if (gpioNotify[n].state == PI_NOTIFY_RUNNING)
         {
            bits = gpioNotify[n].bits;

            emit = 0;

            seqno = gpioNotify[n].seqno;

            /* check to see if any bits have changed for this
               notification.

               bits        is the set of notification bits
               changedBits is the set of changed bits
            */

            if (changedBits & bits)
            {
               oldLevel = reportedLevel & bits;

               for (d=0; d<numSamples; d++)
               {
                  newLevel = gpioSample[d].level & bits;

                  if (newLevel != oldLevel)
                  {
                     gpioReport[emit].seqno = seqno;
                     gpioReport[emit].flags = 0;
                     gpioReport[emit].tick  = gpioSample[d].tick;
                     gpioReport[emit].level = gpioSample[d].level;

                     oldLevel = newLevel;

                     emit++;
                     seqno++;
                  }
               }
            }

            /* check to see if any watchdogs are due for this
               notification.

               bits        is the set of notification bits
               timeoutBits is the set of timed out bits
            */

            if (timeoutBits & bits)
            {
               /* at least one watchdog has fired for this
                  notification.
               */

               for (b=0; b<=PI_MAX_USER_GPIO; b++)
               {
                  if (timeoutBits & bits & (1<<b))
                  {
                     if (numSamples)
                        newLevel = gpioSample[numSamples-1].level;
                     else
                        newLevel = reportedLevel;

                     gpioReport[emit].seqno = seqno;
                     gpioReport[emit].flags = PI_NTFY_FLAGS_WDOG |
                                          PI_NTFY_FLAGS_BIT(b);
                     gpioReport[emit].tick  = tick;
                     gpioReport[emit].level = newLevel;

                     emit++;
                     seqno++;
                  }
               }
            }

            if (emit)
            {
               if (emit > gpioStats.maxEmit) gpioStats.maxEmit = emit;

               emitted = 0;

               while (emit > 0)
               {
                  if (emit > MAX_EMITS)
                  {
                     gpioStats.emitFrags++;

                     err = write(gpioNotify[n].fd,
                              gpioReport+emitted,
                              MAX_EMITS*sizeof(gpioReport_t));

                     if (err != (MAX_EMITS*sizeof(gpioReport_t)))
                     {
                        DBG(0, "fd=%d err=%d errno=%d",
                          gpioNotify[n].fd, err, errno);
                        if (err < 0) DBG(0, "%s", strerror(errno));
                        if ((err != EAGAIN) && (err != EWOULDBLOCK))
                        {
                           /* serious error, no point continuing */
                           gpioNotify[n].bits  = 0;
                           gpioNotify[n].state = PI_NOTIFY_CLOSING;
                           intNotifyBits();
                           break;
                        }
                     }

                     emitted += MAX_EMITS;
                     emit    -= MAX_EMITS;
                  }
                  else
                  {
                     err = write(gpioNotify[n].fd,
                              gpioReport+emitted,
                              emit*sizeof(gpioReport_t));

                     if (err != (emit*sizeof(gpioReport_t)))
                     {
                        DBG(0, "fd=%d err=%d errno=%d",
                          gpioNotify[n].fd, err, errno);
                        if (err < 0) DBG(0, "%s", strerror(errno));
                        if ((err != EAGAIN) && (err != EWOULDBLOCK))
                        {
                           /* serious error, no point continuing */
                           gpioNotify[n].bits  = 0;
                           gpioNotify[n].state = PI_NOTIFY_CLOSING;
                           intNotifyBits();
                           break;
                        }
                     }

                     emitted += emit;
                     emit = 0;
                  }
               }

               gpioNotify[n].seqno = seqno;
            }
         }
      }

      /* once all outputs have been emitted set reported level */

      if (numSamples) reportedLevel = gpioSample[numSamples-1].level;

      if (numSamples > gpioStats.maxSamples)
         gpioStats.maxSamples = numSamples;
      
      gpioStats.numSamples += numSamples;
   }

   return 0;
}

/* ----------------------------------------------------------------------- */

static void * pthTimerTick(void *x)
{
   gpioTimer_t *     tp;
   struct timespec   req, rem, period;
   char              buf[256];

   tp = x;

   clock_gettime(CLOCK_REALTIME, &tp->nextTick);

   while (1)
   {
      clock_gettime(CLOCK_REALTIME, &rem);

      period.tv_sec  = tp->millis / THOUSAND;
      period.tv_nsec = (tp->millis % THOUSAND) * THOUSAND * THOUSAND;

      do
      {
         TIMER_ADD(&tp->nextTick, &period, &tp->nextTick);

         TIMER_SUB(&tp->nextTick, &rem, &req);
      }
      while (req.tv_sec < 0);

      while (nanosleep(&req, &rem))
      {
         req.tv_sec  = rem.tv_sec;
         req.tv_nsec = rem.tv_nsec;
      }

      if (gpioCfg.dbgLevel >= DBG_SLOW_TICK)
      {
         if ((tp->millis > 50) || (gpioCfg.dbgLevel >= DBG_FAST_TICK))
         {
            sprintf(buf, "pigpio: TIMER=%d @ %u %u\n",
               tp->id,
              (unsigned)tp->nextTick.tv_sec,
              (unsigned)tp->nextTick.tv_nsec);
            fprintf(stderr, buf);
         }
      }

      if (tp->ex) (tp->func)(tp->userdata);
      else        (tp->func)();
   }

   return 0;
}

/* ----------------------------------------------------------------------- */


static void * pthFifoThread(void *x)
{
   char buf[CMD_BUF_SIZE];
   int idx, flags, len;
   cmdCmd_t cmd;
   gpioExtent_t iExt[3];
   gpioExtent_t oExt[3];
   char *p;

   myCreatePipe(PI_INPFIFO, 0662);

   if ((inpFifo = fopen(PI_INPFIFO, "r+")) == NULL)
      SOFT_ERROR((void*)PI_INIT_FAILED, "fopen %s failed(%m)", PI_INPFIFO);

   myCreatePipe(PI_OUTFIFO, 0664);

   if ((outFifo = fopen(PI_OUTFIFO, "w+")) == NULL)
      SOFT_ERROR((void*)PI_INIT_FAILED, "fopen %s failed (%m)", PI_OUTFIFO);

   /* set outFifo non-blocking */

   flags = fcntl(fileno(outFifo), F_GETFL, 0);
   fcntl(fileno(outFifo), F_SETFL, flags | O_NONBLOCK);

   while (1)
   {
      if (fgets(buf, sizeof(buf), inpFifo) == NULL)
         SOFT_ERROR((void*)PI_INIT_FAILED, "fifo fgets failed (%m)");

      len = strlen(buf);

      if (len)
      {
        --len;
        buf[len] = 0; /* replace terminating */
      }

      if ((idx=cmdParse(buf, &cmd, 0, NULL, iExt)) >= 0)
      {
         oExt[0].ptr = buf;
         oExt[0].size = CMD_BUF_SIZE-1;

         myDoCommand(&cmd, iExt, oExt);

         switch (cmdInfo[idx].rv)
         {
            case 0:
               fprintf(outFifo, "%d\n", cmd.res);
               break;

            case 1:
               fprintf(outFifo, "%d\n", cmd.res);
               break;

            case 2:
               fprintf(outFifo, "%d\n", cmd.res);
               break;

            case 3:
               fprintf(outFifo, "%08X\n", cmd.res);
               break;

            case 4:
               fprintf(outFifo, "%u\n", cmd.res);
               break;

            case 5:
               fprintf(outFifo, cmdUsage);
               break;

            case 6:
               if (cmd.res < 0) fprintf(outFifo, "%d\n", cmd.res);
               else if (cmd.res > 0)
               {
                  p = oExt[0].ptr;
                  p[cmd.res] = 0;
                  fprintf(outFifo, "%s", (char *)oExt[0].ptr);
               }
               break;

         }

      }
      else fprintf(outFifo, "%d\n", PI_BAD_FIFO_COMMAND);

      fflush(outFifo);
   }

   return 0;
}

/* ----------------------------------------------------------------------- */

static void *pthSocketThreadHandler(void *fdC)
{
   int sock = *(int*)fdC;
   cmdCmd_t cmd;
   unsigned bytes;
   char *memPtr;
   gpioExtent_t iExt[3];
   gpioExtent_t oExt[3];
   unsigned tmp;
   char buf[CMD_BUF_SIZE];

   oExt[0].size = CMD_BUF_SIZE-1;
   oExt[0].ptr = buf;

   free(fdC);

   while(1)
   {
      if (recv(sock, &cmd, sizeof(cmdCmd_t), MSG_WAITALL) == sizeof(cmdCmd_t))
      {
         if (cmd.cmd == PI_CMD_NOIB)
         {
            cmd.res = gpioNotifyOpenInBand(sock);
         }
         else if (cmd.cmd == PI_CMD_WVAG)
         {
            /*
            p1=numPulses
            p2=0
            ## extension ##
            gpioPulse_t[] pulses
            */

            bytes = cmd.p1 * sizeof(gpioPulse_t);

            memPtr = malloc(bytes);

            if (memPtr)
            {
               if (recv(sock, memPtr, bytes, MSG_WAITALL) == bytes)
               {
                  iExt[0].size = bytes;
                  iExt[0].ptr = memPtr;
                  myDoCommand(&cmd, iExt, oExt);
                  free(memPtr);
               }
               else
               {
                  free(memPtr);
                  break;
               }
            }
            else break;

         }
         else if (cmd.cmd == PI_CMD_WVAS)
         {
            /*
            p1=user_gpio
            p2=numChar
            ## extension ##
            unsigned baud
            unsigned offset
            char[] str
            */

            bytes = sizeof(unsigned) + sizeof(unsigned) + cmd.p2;

            memPtr = malloc(bytes+1); /* add 1 for a nul terminator */

            if (memPtr)
            {
               if (recv(sock, memPtr, bytes, MSG_WAITALL) == bytes)
               {
                  iExt[0].size = sizeof(unsigned);
                  iExt[0].ptr = memPtr;
                  iExt[1].size = sizeof(unsigned);
                  iExt[1].ptr = memPtr + sizeof(unsigned);
                  iExt[2].size = cmd.p2;
                  iExt[2].ptr = memPtr + sizeof(unsigned) + sizeof(unsigned);
                  memPtr[bytes] = 0; /* may be duplicate terminator */
                  myDoCommand(&cmd, iExt, oExt);
                  free(memPtr);
               }
               else
               {
                  free(memPtr);
                  break;
               }
            }
            else break;

         }
         else if (cmd.cmd == PI_CMD_PROC)
         {
            /*
            p1=script length
            p2=0
            ## extension ##
            char[] script
            */

            bytes = cmd.p1;

            memPtr = malloc(bytes+1); /* add 1 for a nul terminator */

            if (memPtr)
            {
               if (bytes) /* script appended */
               {
                  if (recv(sock, memPtr, bytes, MSG_WAITALL) != bytes)
                  {
                     free(memPtr);
                     break;
                  }
               }
               iExt[0].size = bytes;
               iExt[0].ptr = memPtr;
               memPtr[bytes] = 0; /* may be duplicate terminator */
               myDoCommand(&cmd, iExt, oExt);
               free(memPtr);
            }
            else break;
         }
         else
         {
            switch (cmd.cmd)
            {
               case PI_CMD_TRIG:
                  /*
                  p1=user_gpio
                  p2=pulseLen
                  ## extension ##
                  unsigned level
                  */
                  iExt[0].size = 4;
                  iExt[0].ptr = &tmp;

                  if (recv(sock, &tmp, sizeof(unsigned), MSG_WAITALL) !=
                     sizeof(unsigned))
                  {
                     close(sock);
                     return 0;
                  }
                  break;

               default:
                  break;
            }
            myDoCommand(&cmd, iExt, oExt);
         }

         write(sock, &cmd, sizeof(cmdCmd_t));

         switch (cmd.cmd)
         {
            case PI_CMD_SLR: /* extension */

               if (cmd.res > 0)
               {
                  write(sock, oExt[0].ptr, cmd.res);
               }
               break;

            default:
               break;
         }


      }
      else break;
   }

   close(sock);

   return 0;
}

/* ----------------------------------------------------------------------- */

static void * pthSocketThread(void *x)
{
   int fdC, c, *sock;
   struct sockaddr_in client;
   pthread_attr_t attr;
 
   if (pthread_attr_init(&attr))
      SOFT_ERROR((void*)PI_INIT_FAILED,
         "pthread_attr_init failed (%m)");

   if (pthread_attr_setstacksize(&attr, STACK_SIZE))
      SOFT_ERROR((void*)PI_INIT_FAILED,
         "pthread_attr_setstacksize failed (%m)");

   if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED))
      SOFT_ERROR((void*)PI_INIT_FAILED,
         "pthread_attr_setdetachstate failed (%m)");

   /* fdSock opened in gpioInitialise so that we can treat
      failure to bind as fatal. */

   listen(fdSock, 100);
   
   c = sizeof(struct sockaddr_in);

   while ((fdC =
      accept(fdSock, (struct sockaddr *)&client, (socklen_t*)&c)))
   {
      pthread_t thr;

      sock = malloc(sizeof(int));

      *sock = fdC;
      
      if (pthread_create
         (&thr, &attr, pthSocketThreadHandler, (void*) sock) < 0)
         SOFT_ERROR((void*)PI_INIT_FAILED,
            "socket pthread_create failed (%m)");
   }
   
   if (fdC < 0)
      SOFT_ERROR((void*)PI_INIT_FAILED, "accept failed (%m)");

   return 0;
}

/* ======================================================================= */

static int initGrabLockFile(void)
{
   int  fd;
   int  lockResult;
   char pidStr[20];

   /* try to grab the lock file */

   fd = open(PI_LOCKFILE, O_WRONLY|O_CREAT|O_EXCL|O_TRUNC, 0644);
   
   if (fd != -1)
   {
      lockResult = flock(fd, LOCK_EX|LOCK_NB);
   
      if(lockResult == 0)
      {
         sprintf(pidStr, "%d\n", (int)getpid());
   
         write(fd, pidStr, strlen(pidStr));
      }
      else
      {
         close(fd);
         return -1;
      }
   }

   return fd;
}

/* ----------------------------------------------------------------------- */

static int initZaps
(
   int       pmapFd,
   dmaPage_t *dmaV1,
   dmaPage_t *dmaV2[],
   dmaPage_t *dmaP[],
   int       pages)
{
   int n;
   long index;
   off_t offset;
   ssize_t t;
   int status;
   uint32_t pageAdr2;
   unsigned long long pa;

   DBG(DBG_STARTUP, "");

   status = 0;

   pageAdr2 = (uint32_t) dmaV2[0];

   index  = ((uint32_t)dmaV1 / PAGE_SIZE) * 8;

   offset = lseek(pmapFd, index, SEEK_SET);

   if (offset != index)
      SOFT_ERROR(PI_INIT_FAILED, "lseek pagemap failed (%m)");

   for (n=0; n<pages; n++)
   {
      t = read(pmapFd, &pa, sizeof(pa));

      if (t != sizeof(pa))
         SOFT_ERROR(PI_INIT_FAILED, "read pagemap failed (%m)");

      DBG(DBG_STARTUP, "pf%d=%016llX", n, pa);

      dmaP[n] = (dmaPage_t *) (uint32_t) (PAGE_SIZE * (pa & 0xFFFFFFFF));

      dmaV2[n] = mmap
      (
         (void *)pageAdr2,
         PAGE_SIZE,
         PROT_READ|PROT_WRITE,
         MAP_SHARED|MAP_FIXED|MAP_LOCKED|MAP_NORESERVE,
         fdMem,
         (uint32_t)dmaP[n] | 0x40000000
      );

      pageAdr2 += PAGE_SIZE;

      if (dmaP[n] == 0) status = 1;
   }

   return status;
}

/* ----------------------------------------------------------------------- */

static uint32_t * initMapMem(int fd, uint32_t addr, uint32_t len)
{
    return (uint32_t *) mmap(0, len,
       PROT_READ|PROT_WRITE|PROT_EXEC,
       MAP_SHARED|MAP_LOCKED,
       fd, addr);
}

/* ----------------------------------------------------------------------- */

static int initCheckPermitted(void)
{
   DBG(DBG_STARTUP, "");

   if ((fdMem = open("/dev/mem", O_RDWR | O_SYNC) ) < 0)
   {
      fprintf(stderr,
         "\n" \
         "+---------------------------------------------------------+\n" \
         "|Sorry, you don't have permission to run this program.    |\n" \
         "|Try running as root, e.g. precede the command with sudo. |\n" \
         "+---------------------------------------------------------+\n\n");
      exit(-1);
   }
   return 0;
}

/* ----------------------------------------------------------------------- */

static int initPeripherals(void)
{
   uint32_t dmaBase;

   DBG(DBG_STARTUP, "");

   gpioReg = initMapMem(fdMem, GPIO_BASE, GPIO_LEN);

   if (gpioReg == MAP_FAILED)
      SOFT_ERROR(PI_INIT_FAILED, "mmap gpio failed (%m)");

   /* dma channels 0-14 share one page, 15 has another */

   if (gpioCfg.DMAprimaryChannel < 15)
   {
      dmaBase = DMA_BASE;
   }
   else dmaBase = DMA15_BASE;

   dmaReg = initMapMem(fdMem, dmaBase,  DMA_LEN);

   if (dmaReg == MAP_FAILED)
      SOFT_ERROR(PI_INIT_FAILED, "mmap dma failed (%m)");

   if (gpioCfg.DMAprimaryChannel < 15)
   {
      dmaIn =  dmaReg + (gpioCfg.DMAprimaryChannel   * 0x40);
      dmaOut = dmaReg + (gpioCfg.DMAsecondaryChannel * 0x40);
   }

   DBG(DBG_STARTUP, "DMA #%d @ %08X @ %08X",
      gpioCfg.DMAprimaryChannel, dmaBase, (uint32_t)dmaIn);

   DBG(DBG_STARTUP, "debug reg is %08X", dmaIn[DMA_DEBUG]);

   clkReg  = initMapMem(fdMem, CLK_BASE,  CLK_LEN);

   if (clkReg == MAP_FAILED)
      SOFT_ERROR(PI_INIT_FAILED, "mmap clk failed (%m)");

   systReg  = initMapMem(fdMem, SYST_BASE,  SYST_LEN);

   if (systReg == MAP_FAILED)
      SOFT_ERROR(PI_INIT_FAILED, "mmap syst failed (%m)");

   pwmReg  = initMapMem(fdMem, PWM_BASE,  PWM_LEN);

   if (pwmReg == MAP_FAILED)
      SOFT_ERROR(PI_INIT_FAILED, "mmap pwm failed (%m)");

   pcmReg  = initMapMem(fdMem, PCM_BASE,  PCM_LEN);

   if (pcmReg == MAP_FAILED)
      SOFT_ERROR(PI_INIT_FAILED, "mmap pcm failed (%m)");

   return 0;
}

/* ----------------------------------------------------------------------- */

static int initDMAblock(int pagemapFd, int block)
{
   int trys, ok;
   unsigned pageNum;

   DBG(DBG_STARTUP, "");

   dmaBloc[block] = mmap(
       0, (PAGES_PER_BLOCK*PAGE_SIZE),
       PROT_READ|PROT_WRITE,
       MAP_SHARED|MAP_ANONYMOUS|MAP_NORESERVE|MAP_LOCKED,
       -1, 0);

   if (dmaBloc[block] == MAP_FAILED)
      SOFT_ERROR(PI_INIT_FAILED, "mmap dma block %d failed (%m)", block);

   /* force allocation of physical memory */

   memset((void *)dmaBloc[block], 0, (PAGES_PER_BLOCK*PAGE_SIZE));

   pageNum = block * PAGES_PER_BLOCK;

   dmaVirt[pageNum] = mmap(
       0, (PAGES_PER_BLOCK*PAGE_SIZE),
       PROT_READ|PROT_WRITE,
       MAP_SHARED|MAP_ANONYMOUS|MAP_NORESERVE|MAP_LOCKED,
       -1, 0);

   if (dmaVirt[pageNum] == MAP_FAILED)
      SOFT_ERROR(PI_INIT_FAILED, "mmap dma block %d failed (%m)", block);

   munmap(dmaVirt[pageNum], PAGES_PER_BLOCK*PAGE_SIZE);

   trys = 0;
   ok = 0;

   while ((trys < 10) && !ok)
   {
      if (initZaps(pagemapFd,
                    dmaBloc[block],
                    &dmaVirt[pageNum],
                    &dmaPhys[pageNum],
                    PAGES_PER_BLOCK) == 0) ok = 1;
      else myGpioDelay(50000);

      ++trys;
   }

   if (!ok) SOFT_ERROR(PI_INIT_FAILED, "initZaps failed");

   return 0;
}


/* ----------------------------------------------------------------------- */

static int initDMAcbs(void)
{
   int pid;
   char str[64];
   int pagemapFd;
   int i, servoCycles, superCycles;

   DBG(DBG_STARTUP, "");

   /* Calculate the number of blocks needed for buffers.  The number
      of blocks must be a multiple of the 20ms servo cycle.
   */

   servoCycles = gpioCfg.bufferMilliseconds / 20;
   if           (gpioCfg.bufferMilliseconds % 20) servoCycles++;

   bufferCycles = (SUPERCYCLE * servoCycles) / gpioCfg.clockMicros;

   superCycles = bufferCycles / SUPERCYCLE;
   if           (bufferCycles % SUPERCYCLE) superCycles++;

   bufferCycles = SUPERCYCLE * superCycles;

   bufferBlocks = bufferCycles / CYCLES_PER_BLOCK;

   DBG(DBG_STARTUP, "bmillis=%d mics=%d bblk=%d bcyc=%d",
      gpioCfg.bufferMilliseconds, gpioCfg.clockMicros,
      bufferBlocks, bufferCycles);

   /* allocate memory for pointers to virtual and physical pages */

   dmaBloc = mmap(
       0, (bufferBlocks+PI_WAVE_BLOCKS)*sizeof(dmaPage_t *),
       PROT_READ|PROT_WRITE,
       MAP_PRIVATE|MAP_ANONYMOUS|MAP_LOCKED,
       -1, 0);

   if (dmaBloc == MAP_FAILED)
      SOFT_ERROR(PI_INIT_FAILED, "mmap dma virtual failed (%m)");

   dmaVirt = mmap(
       0, PAGES_PER_BLOCK*(bufferBlocks+PI_WAVE_BLOCKS)*sizeof(dmaPage_t *),
       PROT_READ|PROT_WRITE,
       MAP_PRIVATE|MAP_ANONYMOUS|MAP_LOCKED,
       -1, 0);

   if (dmaVirt == MAP_FAILED)
      SOFT_ERROR(PI_INIT_FAILED, "mmap dma virtual failed (%m)");

   dmaPhys = mmap(
       0, PAGES_PER_BLOCK*(bufferBlocks+PI_WAVE_BLOCKS)*sizeof(dmaPage_t *),
       PROT_READ|PROT_WRITE,
       MAP_PRIVATE|MAP_ANONYMOUS|MAP_LOCKED,
       -1, 0);

   if (dmaPhys == MAP_FAILED)
      SOFT_ERROR(PI_INIT_FAILED, "mmap dma physical failed (%m)");

   dmaIPhys = (dmaIPage_t **) dmaPhys;
   dmaIVirt = (dmaIPage_t **) dmaVirt;

   dmaOPhys = (dmaOPage_t **)(dmaPhys + (PAGES_PER_BLOCK*bufferBlocks));
   dmaOVirt = (dmaOPage_t **)(dmaVirt + (PAGES_PER_BLOCK*bufferBlocks));

   pid = getpid();

   sprintf(str, "/proc/%d/pagemap", pid);

   pagemapFd = open(str, O_RDONLY);
  
   if (pagemapFd < 0)
      SOFT_ERROR(PI_INIT_FAILED, "open pagemap failed(%m)");

   for (i=0; i<(bufferBlocks+PI_WAVE_BLOCKS); i++) initDMAblock(pagemapFd, i);

   close(pagemapFd);

   DBG(DBG_STARTUP, "dmaBloc=%08X dmaIn=%08X",
      (uint32_t)dmaBloc, (uint32_t)dmaIn);

   DBG(DBG_STARTUP, "gpioReg=%08X pwmReg=%08X pcmReg=%08X clkReg=%08X",
      (uint32_t)gpioReg, (uint32_t)pwmReg,
      (uint32_t)pcmReg,  (uint32_t)clkReg); 

   for (i=0; i<DMA_PAGES; i++)
      DBG(DBG_STARTUP, "dmaIPhys[%d]=%08X", i, (uint32_t)dmaIPhys[i]);

   dmaInitCbs();

   if (gpioCfg.dbgLevel >= DBG_DMACBS)
   {
      fprintf(stderr, "*** INPUT DMA CONTROL BLOCKS ***\n");
      for (i=0; i<NUM_CBS; i++) dmaCbPrint(i);
   }

   return 0;
}

/* ----------------------------------------------------------------------- */

static void initPWM(unsigned bits)
{
   DBG(DBG_STARTUP, "");

   /* reset PWM */

   pwmReg[PWM_CTL] = 0;

   myGpioDelay(10);

   pwmReg[PWM_STA] = -1;

   myGpioDelay(10);

   /* set number of bits to transmit */

   pwmReg[PWM_RNG1] = bits;

   myGpioDelay(10);

   dmaIVirt[0]->periphData = 1;
   
   /* enable PWM DMA, raise panic and dreq thresholds to 15 */

   pwmReg[PWM_DMAC] = PWM_DMAC_ENAB      |
                      PWM_DMAC_PANIC(15) |
                      PWM_DMAC_DREQ(15);

   myGpioDelay(10);

   /* clear PWM fifo */

   pwmReg[PWM_CTL] = PWM_CTL_CLRF1;

   myGpioDelay(10);

   /* enable PWM channel 1 and use fifo */

   pwmReg[PWM_CTL] = PWM_CTL_USEF1 | PWM_CTL_MODE1 | PWM_CTL_PWEN1;
}

/* ----------------------------------------------------------------------- */

static void initPCM(unsigned bits)
{
   DBG(DBG_STARTUP, "");

   /* disable PCM so we can modify the regs */

   pcmReg[PCM_CS] = 0;

   myGpioDelay(1000);

   pcmReg[PCM_FIFO]   = 0;
   pcmReg[PCM_MODE]   = 0;
   pcmReg[PCM_RXC]    = 0;
   pcmReg[PCM_TXC]    = 0;
   pcmReg[PCM_DREQ]   = 0;
   pcmReg[PCM_INTEN]  = 0;
   pcmReg[PCM_INTSTC] = 0;
   pcmReg[PCM_GRAY]   = 0;

   myGpioDelay(1000);
  
   pcmReg[PCM_MODE] = PCM_MODE_FLEN(bits-1); /* # bits in frame */

   /* enable channel 1 with # bits width */

   pcmReg[PCM_TXC] = PCM_TXC_CH1EN | PCM_TXC_CH1WID(bits-8);

   pcmReg[PCM_CS] |= PCM_CS_STBY; /* clear standby */

   myGpioDelay(1000);

   pcmReg[PCM_CS] |= PCM_CS_TXCLR; /* clear TX FIFO */

   pcmReg[PCM_CS] |= PCM_CS_DMAEN; /* enable DREQ */

   pcmReg[PCM_DREQ] = PCM_DREQ_TX_PANIC(16) | PCM_DREQ_TX_REQ_L(30);

   pcmReg[PCM_INTSTC] = 0b1111; /* clear status bits */

   /* enable PCM */

   pcmReg[PCM_CS] |= PCM_CS_EN ;

   /* enable tx */

   pcmReg[PCM_CS] |= PCM_CS_TXON;

   dmaIVirt[0]->periphData = 0x0F;
}

/* ----------------------------------------------------------------------- */

static void initClock(int mainClock)
{
   unsigned clkCtl, clkDiv, clkSrc, clkDivI, clkDivF, clkMash, clkBits;
   char * per, * src;
   unsigned micros;

   DBG(DBG_STARTUP, "");

   if (mainClock) micros = gpioCfg.clockMicros;
   else           micros = PI_WF_MICROS;

   if ((gpioCfg.clockPeriph == PI_CLOCK_PCM) && mainClock)
   {
      clkCtl = CLK_PCMCTL;
      clkDiv = CLK_PCMDIV;
      per = "PCM";
   }
   else
   {
      clkCtl = CLK_PWMCTL;
      clkDiv = CLK_PWMDIV;
      per = "PWM";
   }

   if (gpioCfg.clockSource == PI_CLOCK_PLLD)
   {
      clkSrc  = CLK_CTL_SRC_PLLD;
      clkDivI = 50 * micros;
      clkDivF = 0;
      clkMash = 0;
      clkBits = 10;
      src = "PLLD";
   }
   else
   {
      clkSrc  = CLK_CTL_SRC_OSC;
      clkDivI = clkCfg[micros].divi;
      clkDivF = clkCfg[micros].divf;
      clkMash = clkCfg[micros].mash;
      clkBits = clkCfg[micros].bits;
      src = "OSC";
   }

   DBG(DBG_STARTUP, "%s %s divi=%d divf=%d mash=%d bits=%d",
      per, src, clkDivI, clkDivF, clkMash, clkBits);

   clkReg[clkCtl] = CLK_PASSWD | CLK_CTL_KILL;

   myGpioDelay(10);

   clkReg[clkDiv] =
      (CLK_PASSWD | CLK_DIV_DIVI(clkDivI) | CLK_DIV_DIVF(clkDivF));

   myGpioDelay(10);

   clkReg[clkCtl] =
      (CLK_PASSWD | CLK_CTL_MASH(clkMash) | CLK_CTL_SRC(clkSrc));

   myGpioDelay(10);

   clkReg[clkCtl] |= (CLK_PASSWD | CLK_CTL_ENAB);

   myGpioDelay(10);

   if ((gpioCfg.clockPeriph == PI_CLOCK_PCM) && mainClock)
        initPCM(clkBits);
   else initPWM(clkBits);

   myGpioDelay(2000);
}

/* ----------------------------------------------------------------------- */

static void initDMAgo(volatile uint32_t  * dmaAddr, uint32_t cbAddr)
{
   DBG(DBG_STARTUP, "");

   dmaAddr[DMA_CS] = DMA_CHANNEL_RESET;

   dmaAddr[DMA_CS] = DMA_INTERRUPT_STATUS | DMA_END_FLAG;

   dmaAddr[DMA_CONBLK_AD] = cbAddr | DMA_BUS_ADR;

   /* clear READ/FIFO/READ_LAST_NOT_SET error bits */

   dmaAddr[DMA_DEBUG] = DMA_DEBUG_READ_ERR            |          
                        DMA_DEBUG_FIFO_ERR            |
                        DMA_DEBUG_RD_LST_NOT_SET_ERR;


   dmaAddr[DMA_CS] = DMA_WAIT_ON_WRITES    |
                     DMA_PANIC_PRIORITY(8) |
                     DMA_PRIORITY(8)       |
                     DMA_ACTIVATE;

   DMAstarted = 1;
}

/* ----------------------------------------------------------------------- */

static void initClearGlobals(void)
{
   int i;

   DBG(DBG_STARTUP, "");

   alertBits    = 0;
   monitorBits  = 0;
   notifyBits   = 0;

   libInitialised   = 0;
   DMAstarted       = 0;

   pthAlertRunning  = 0;
   pthFifoRunning   = 0;
   pthSocketRunning = 0;

   wfc[0] = 0;
   wfc[1] = 0;
   wfc[2] = 0;

   wfcur=0;

   wfStats.micros     = 0;
   wfStats.highMicros = 0;
   wfStats.maxMicros  = PI_WAVE_MAX_MICROS;

   wfStats.pulses     = 0;
   wfStats.highPulses = 0;
   wfStats.maxPulses  = PI_WAVE_MAX_PULSES;

   wfStats.cbs        = 0;
   wfStats.highCbs    = 0;
   wfStats.maxCbs     = (PAGES_PER_BLOCK * CBS_PER_OPAGE);

   gpioGetSamples.func     = NULL;
   gpioGetSamples.ex       = 0;
   gpioGetSamples.userdata = NULL;
   gpioGetSamples.bits     = 0;

   for (i=0; i<=PI_MAX_USER_GPIO; i++)
   {
      wfRx[i].mode         = PI_WFRX_NONE;

      gpioAlert[i].func    = NULL;

      gpioInfo [i].is      = GPIO_UNDEFINED;
      gpioInfo [i].width   = 0;
      gpioInfo [i].range   = PI_DEFAULT_DUTYCYCLE_RANGE;
      gpioInfo [i].freqIdx = DEFAULT_PWM_IDX;
   }

   for (i=0; i<PI_NOTIFY_SLOTS; i++)
   {
      gpioNotify[i].seqno = 0;
      gpioNotify[i].state = PI_NOTIFY_CLOSED;
   }

   for (i=0; i<=PI_MAX_SIGNUM; i++)
   {
      gpioSignal[i].func     = NULL;
      gpioSignal[i].ex       = 0;
      gpioSignal[i].userdata = NULL;
   }

   for (i=0; i<=PI_MAX_TIMER; i++)
   {
      gpioTimer[i].running = 0;
      gpioTimer[i].func    = NULL;
   }

   /* calculate the usable PWM frequencies */

   for (i=0; i<PWM_FREQS; i++)
   {
      pwmFreq[i]=
         (1000000.0/
            ((float)PULSE_PER_CYCLE*gpioCfg.clockMicros*pwmCycles[i]))+0.5;

      DBG(DBG_STARTUP, "f%d is %d", i, pwmFreq[i]);
   }

   inpFifo = NULL;
   outFifo = NULL;

   fdLock = -1;
   fdMem  = -1;
   fdSock = -1;

   dmaBloc = MAP_FAILED;
   dmaVirt = MAP_FAILED;
   dmaPhys = MAP_FAILED;

   clkReg  = MAP_FAILED;
   dmaReg  = MAP_FAILED;
   gpioReg = MAP_FAILED;
   pcmReg  = MAP_FAILED;
   pwmReg  = MAP_FAILED;
   systReg = MAP_FAILED;
}

/* ----------------------------------------------------------------------- */

static void initReleaseResources(void)
{
   int i;

   DBG(DBG_STARTUP, "");

   /* shut down running threads */

   for (i=0; i<=PI_MAX_TIMER; i++)
   {
      if (gpioTimer[i].running)
      {
         /* destroy thread */

         pthread_cancel(gpioTimer[i].pthId);
         pthread_join(gpioTimer[i].pthId, NULL);
         gpioTimer[i].running = 0;
      }
   }

   if (pthAlertRunning)
   {
      pthread_cancel(pthAlert);
      pthread_join(pthAlert, NULL);
      pthAlertRunning = 0;
   }

   if (pthFifoRunning)
   {
      pthread_cancel(pthFifo);
      pthread_join(pthFifo, NULL);
      pthFifoRunning = 0;
   }

   if (pthSocketRunning)
   {
      pthread_cancel(pthSocket);
      pthread_join(pthSocket, NULL);
      pthSocketRunning = 0;
   }

   /* release mmap'd memory */

   if (clkReg  != MAP_FAILED) munmap((void *)clkReg,  CLK_LEN);
   if (dmaReg  != MAP_FAILED) munmap((void *)dmaReg,  DMA_LEN);
   if (gpioReg != MAP_FAILED) munmap((void *)gpioReg, GPIO_LEN);
   if (pcmReg  != MAP_FAILED) munmap((void *)pcmReg,  PCM_LEN);
   if (pwmReg  != MAP_FAILED) munmap((void *)pwmReg,  PWM_LEN);
   if (systReg != MAP_FAILED) munmap((void *)systReg, SYST_LEN);

   clkReg  = MAP_FAILED;
   dmaReg  = MAP_FAILED;
   gpioReg = MAP_FAILED;
   pcmReg  = MAP_FAILED;
   pwmReg  = MAP_FAILED;
   systReg = MAP_FAILED;

   if (dmaVirt != MAP_FAILED)
   {
      for (i=0; i<PAGES_PER_BLOCK*(bufferBlocks+PI_WAVE_BLOCKS); i++)
      {
         munmap(dmaVirt[i], PAGE_SIZE);
      }

      munmap(dmaVirt,
         PAGES_PER_BLOCK*(bufferBlocks+PI_WAVE_BLOCKS)*sizeof(dmaPage_t *));
   }

   dmaVirt = MAP_FAILED;

   if (dmaPhys != MAP_FAILED)
   {
      for (i=0; i<PAGES_PER_BLOCK*(bufferBlocks+PI_WAVE_BLOCKS); i++)
      {
         munmap(dmaPhys[i], PAGE_SIZE);
      }

      munmap(dmaPhys,
         PAGES_PER_BLOCK*(bufferBlocks+PI_WAVE_BLOCKS)*sizeof(dmaPage_t *));
   }

   dmaPhys = MAP_FAILED;

   if (dmaBloc != MAP_FAILED)
   {
      for (i=0; i<(bufferBlocks+PI_WAVE_BLOCKS); i++)
      {
         munmap(dmaBloc[i], PAGES_PER_BLOCK*PAGE_SIZE);
      }

      munmap(dmaBloc, (bufferBlocks+PI_WAVE_BLOCKS)*sizeof(dmaPage_t *));
   }

   dmaBloc = MAP_FAILED;

   if (inpFifo != NULL)
   {
      fclose(inpFifo);
      unlink(PI_INPFIFO);
      inpFifo = NULL;
   }

   if (outFifo != NULL)
   {
      fclose(outFifo);
      unlink(PI_OUTFIFO);
      outFifo = NULL;
   }

   if (fdMem != -1)
   {
      close(fdMem);
      fdMem = -1;
   }

   if (fdLock != -1)
   {
      close(fdLock);
      unlink(PI_LOCKFILE);
      fdLock = -1;
   }

   if (fdSock != -1)
   {
      close(fdSock);
      fdSock = -1;
   }
}

/* ======================================================================= */

int gpioInitialise(void)
{
   int i;
   struct sockaddr_in server;
   char * portStr;
   unsigned port;

   clock_gettime(CLOCK_REALTIME, &libStarted);

   DBG(DBG_STARTUP, "");

   if (libInitialised) return PIGPIO_VERSION;

   initClearGlobals();

   if (initCheckPermitted() < 0) return PI_INIT_FAILED;

   fdLock = initGrabLockFile();

   if (fdLock < 0)
      SOFT_ERROR(PI_INIT_FAILED, "Can't lock %s", PI_LOCKFILE);

   if (!gpioMaskSet)
   {
      i = gpioHardwareRevision();

      if      (i == 0) gpioMask = PI_DEFAULT_UPDATE_MASK_R0;
      else if (i <  4) gpioMask = PI_DEFAULT_UPDATE_MASK_R1;
      else             gpioMask = PI_DEFAULT_UPDATE_MASK_R2;

      gpioMaskSet = 1;
   }

   sigSetHandler();

   if (initPeripherals() < 0) return PI_INIT_FAILED;

   if (initDMAcbs() < 0) return PI_INIT_FAILED;

   /* done with /dev/mem */

   if (fdMem != -1)
   {
      close(fdMem);
      fdMem = -1;
   }

   initClock(1); /* initialise main clock */

   libInitialised = 1;

   atexit(gpioTerminate);

   if (pthread_attr_init(&pthAttr))
      SOFT_ERROR(PI_INIT_FAILED, "pthread_attr_init failed (%m)");

   if (pthread_attr_setstacksize(&pthAttr, STACK_SIZE))
      SOFT_ERROR(PI_INIT_FAILED, "pthread_attr_setstacksize failed (%m)");

   if (pthread_create(&pthAlert, &pthAttr, pthAlertThread, &i))
      SOFT_ERROR(PI_INIT_FAILED, "pthread_create alert failed (%m)");

   pthAlertRunning = 1;

   if (!(gpioCfg.ifFlags & PI_DISABLE_FIFO_IF))
   {
      if (pthread_create(&pthFifo, &pthAttr, pthFifoThread, &i))
         SOFT_ERROR(PI_INIT_FAILED, "pthread_create fifo failed (%m)");

      pthFifoRunning = 1;
   }

   if (!(gpioCfg.ifFlags & PI_DISABLE_SOCK_IF))
   {
      fdSock = socket(AF_INET , SOCK_STREAM , 0);

      if (fdSock == -1)
         SOFT_ERROR(PI_INIT_FAILED, "socket failed (%m)");
   
      portStr = getenv(PI_ENVPORT);

      if (portStr) port = atoi(portStr); else port = gpioCfg.socketPort;

      server.sin_family = AF_INET;
      server.sin_addr.s_addr = INADDR_ANY;
      server.sin_port = htons(port);
   
      if (bind(fdSock,(struct sockaddr *)&server , sizeof(server)) < 0)
         SOFT_ERROR(PI_INIT_FAILED, "bind to port %d failed (%m)", port);

      if (pthread_create(&pthSocket, &pthAttr, pthSocketThread, &i))
         SOFT_ERROR(PI_INIT_FAILED, "pthread_create socket failed (%m)");

      pthSocketRunning = 1;
   }

   initDMAgo((uint32_t *)dmaIn, (uint32_t)dmaIPhys[0]);

   return PIGPIO_VERSION;
 }

/* ----------------------------------------------------------------------- */

void gpioTerminate(void)
{
   int i;

   DBG(DBG_USER, "");

   gpioMaskSet = 0;

   if (libInitialised)
   {
      /* reset DMA */

      dmaIn[DMA_CS] = DMA_CHANNEL_RESET;
      dmaOut[DMA_CS] = DMA_CHANNEL_RESET;

      /* reset PWM */

      pwmReg[PWM_CTL] = 0;

      libInitialised = 0;

      DMAstarted = 0;

      if (gpioCfg.showStats)
      {
         fprintf(stderr, "micros=%d\n", gpioCfg.clockMicros);

         fprintf(stderr, "samples %u maxSamples %u maxEmit %u emitFrags %u\n",
            gpioStats.numSamples, gpioStats.maxSamples,
            gpioStats.maxEmit, gpioStats.emitFrags);

         fprintf(stderr, "cb time %d, calls %u alert ticks %u\n",
            gpioStats.cbTicks, gpioStats.cbCalls, gpioStats.alertTicks);

         for (i=0; i< TICKSLOTS; i++)
            fprintf(stderr, "%9u ", gpioStats.diffTick[i]);

         fprintf(stderr, "\n");
      }
   }

   initReleaseResources();

   fflush(NULL);
}

/* ----------------------------------------------------------------------- */

int gpioSetMode(unsigned gpio, unsigned mode)
{
   int reg, shift;

   DBG(DBG_USER, "gpio=%d mode=%d", gpio, mode);

   CHECK_INITED;

   if (gpio > PI_MAX_GPIO)
      SOFT_ERROR(PI_BAD_GPIO, "bad gpio (%d)", gpio);

   if (mode > PI_ALT3)
      SOFT_ERROR(PI_BAD_MODE, "gpio %d, bad mode (%d)", gpio, mode);

   reg   =  gpio/10;
   shift = (gpio%10) * 3;

   if (gpio <= PI_MAX_USER_GPIO)
   {
      if (mode != PI_OUTPUT)
      {
         switch (gpioInfo[gpio].is)
         {
            case GPIO_SERVO:
               /* switch servo off */
               myGpioSetServo(gpio,
                  gpioInfo[gpio].width/gpioCfg.clockMicros, 0);
               break;

            case GPIO_PWM:
               /* switch pwm off */
               myGpioSetPwm(gpio, gpioInfo[gpio].width, 0);
               break;

         }

         gpioInfo[gpio].is = GPIO_UNDEFINED;
      }
   }

   gpioReg[reg] = (gpioReg[reg] & ~(7<<shift)) | (mode<<shift);

   return 0;
}


/* ----------------------------------------------------------------------- */

int gpioGetMode(unsigned gpio)
{
   int reg, shift;

   DBG(DBG_USER, "gpio=%d", gpio);

   CHECK_INITED;

   if (gpio > PI_MAX_GPIO)
      SOFT_ERROR(PI_BAD_GPIO, "bad gpio (%d)", gpio);

   reg   =  gpio/10;
   shift = (gpio%10) * 3;

   return (*(gpioReg + reg) >> shift) & 7;
}


/* ----------------------------------------------------------------------- */

int gpioSetPullUpDown(unsigned gpio, unsigned pud)
{
   DBG(DBG_USER, "gpio=%d pud=%d", gpio, pud);

   CHECK_INITED;

   if (gpio > PI_MAX_GPIO)
      SOFT_ERROR(PI_BAD_GPIO, "bad gpio (%d)", gpio);

   if (pud > PI_PUD_UP)
      SOFT_ERROR(PI_BAD_PUD, "gpio %d, bad pud (%d)", gpio, pud);

   *(gpioReg + GPPUD) = pud;

   myGpioDelay(20);

   *(gpioReg + GPPUDCLK0 + BANK) = BIT;

   myGpioDelay(20);
  
   *(gpioReg + GPPUD) = 0;

   *(gpioReg + GPPUDCLK0 + BANK) = 0;

   return 0;
}


/* ----------------------------------------------------------------------- */

int gpioRead(unsigned gpio)
{
   DBG(DBG_USER, "gpio=%d", gpio);

   CHECK_INITED;

   if (gpio > PI_MAX_GPIO)
      SOFT_ERROR(PI_BAD_GPIO, "bad gpio (%d)", gpio);

   if ((*(gpioReg + GPLEV0 + BANK) & BIT) != 0) return PI_ON;
   else                                         return PI_OFF;
}


/* ----------------------------------------------------------------------- */

int gpioWrite(unsigned gpio, unsigned level)
{
   DBG(DBG_USER, "gpio=%d level=%d", gpio, level);

   CHECK_INITED;

   if (gpio > PI_MAX_GPIO)
      SOFT_ERROR(PI_BAD_GPIO, "bad gpio (%d)", gpio);

   if (level > PI_ON)
      SOFT_ERROR(PI_BAD_LEVEL, "gpio %d, bad level (%d)", gpio, level);

   if (gpio <= PI_MAX_USER_GPIO)
   {
      if (gpioInfo[gpio].is != GPIO_OUTPUT)
      {
         if (gpioInfo[gpio].is == GPIO_UNDEFINED)
         {
            gpioSetMode(gpio, PI_OUTPUT);
         }
         else if (gpioInfo[gpio].is == GPIO_PWM)
         {
            /* switch pwm off */
            myGpioSetPwm(gpio, gpioInfo[gpio].width, 0);
         }
         else if (gpioInfo[gpio].is == GPIO_SERVO)
         {
            /* switch servo off */
            myGpioSetServo(
               gpio, gpioInfo[gpio].width/gpioCfg.clockMicros, 0);
         }

         gpioInfo[gpio].is=GPIO_OUTPUT;
         gpioInfo[gpio].width=0;
      }
   }

   if (level == PI_OFF) *(gpioReg + GPCLR0 + BANK) = BIT;
   else                 *(gpioReg + GPSET0 + BANK) = BIT;

   return 0;
}


/* ----------------------------------------------------------------------- */

int gpioPWM(unsigned gpio, unsigned val)
{
   DBG(DBG_USER, "gpio=%d dutycycle=%d", gpio, val);

   CHECK_INITED;

   if (gpio > PI_MAX_USER_GPIO)
      SOFT_ERROR(PI_BAD_USER_GPIO, "bad gpio (%d)", gpio);

   if (val > gpioInfo[gpio].range) 
      SOFT_ERROR(PI_BAD_DUTYCYCLE, "gpio %d, bad dutycycle (%d)", gpio, val);

   if (gpioInfo[gpio].is != GPIO_PWM)
   {
      if (gpioInfo[gpio].is == GPIO_UNDEFINED)
      {
         gpioSetMode(gpio, PI_OUTPUT);
      }
      else if (gpioInfo[gpio].is == GPIO_SERVO)
      {
         /* switch servo off */
         myGpioSetServo(gpio, gpioInfo[gpio].width, 0);
         gpioInfo[gpio].width = 0;
         gpioInfo[gpio].freqIdx = DEFAULT_PWM_IDX; /* default frequency */
      }
      gpioInfo[gpio].is = GPIO_PWM;
   }

   myGpioSetPwm(gpio, gpioInfo[gpio].width, val);

   gpioInfo[gpio].width=val;

   return 0;
}


/* ----------------------------------------------------------------------- */

int gpioSetPWMrange(unsigned gpio, unsigned range)
{
   int oldWidth, newWidth;

   DBG(DBG_USER, "gpio=%d range=%d", gpio, range);

   CHECK_INITED;

   if (gpio > PI_MAX_USER_GPIO)
      SOFT_ERROR(PI_BAD_USER_GPIO, "bad gpio (%d)", gpio);

   if ((range < PI_MIN_DUTYCYCLE_RANGE)  || (range > PI_MAX_DUTYCYCLE_RANGE))
      SOFT_ERROR(PI_BAD_DUTYRANGE, "gpio %d, bad range (%d)", gpio, range);

   oldWidth = gpioInfo[gpio].width;

   if (oldWidth)
   {
      if (gpioInfo[gpio].is == GPIO_PWM)
      {
         newWidth = (range * oldWidth) / gpioInfo[gpio].range;

         myGpioSetPwm(gpio, oldWidth, 0);
         gpioInfo[gpio].range = range;
         gpioInfo[gpio].width = newWidth;
         myGpioSetPwm(gpio, 0, newWidth);
      }
   }

   gpioInfo[gpio].range = range;

   /* return the actual range for the current gpio frequency */

   return pwmRealRange[gpioInfo[gpio].freqIdx];
}


/* ----------------------------------------------------------------------- */

int gpioGetPWMrange(unsigned gpio)
{
   DBG(DBG_USER, "gpio=%d", gpio);

   CHECK_INITED;

   if (gpio > PI_MAX_USER_GPIO)
      SOFT_ERROR(PI_BAD_USER_GPIO, "bad gpio (%d)", gpio);

   return (gpioInfo[gpio].range);
}


/* ----------------------------------------------------------------------- */

int gpioGetPWMrealRange(unsigned gpio)
{
   DBG(DBG_USER, "gpio=%d", gpio);

   CHECK_INITED;

   if (gpio > PI_MAX_USER_GPIO)
      SOFT_ERROR(PI_BAD_USER_GPIO, "bad gpio (%d)", gpio);

   return pwmRealRange[gpioInfo[gpio].freqIdx];
}


/* ----------------------------------------------------------------------- */

int gpioSetPWMfrequency(unsigned gpio, unsigned frequency)
{
   int i, width;
   unsigned diff, best, idx;

   DBG(DBG_USER, "gpio=%d frequency=%d", gpio, frequency);

   CHECK_INITED;

   if (gpio > PI_MAX_USER_GPIO)
      SOFT_ERROR(PI_BAD_USER_GPIO, "bad gpio (%d)", gpio);

   if      (frequency > pwmFreq[0])           idx = 0;
   else if (frequency < pwmFreq[PWM_FREQS-1]) idx = PWM_FREQS-1;
   else
   {
      best = 100000; /* impossibly high frequency difference */
      idx = 0;

      for (i=0; i<PWM_FREQS; i++)
      {
         if (frequency > pwmFreq[i]) diff = frequency - pwmFreq[i];
         else                        diff = pwmFreq[i] - frequency;

         if (diff < best)
         {
            best = diff;
            idx = i;
         }
      }
   }

   width = gpioInfo[gpio].width;

   if (width)
   {
      if (gpioInfo[gpio].is == GPIO_PWM)
      {
         myGpioSetPwm(gpio, width, 0);
         gpioInfo[gpio].freqIdx = idx;
         myGpioSetPwm(gpio, 0, width);
      }
   }

   gpioInfo[gpio].freqIdx = idx;

   return pwmFreq[idx];
}


/* ----------------------------------------------------------------------- */

int gpioGetPWMfrequency(unsigned gpio)
{
   DBG(DBG_USER, "gpio=%d", gpio);

   CHECK_INITED;

   if (gpio > PI_MAX_USER_GPIO)
      SOFT_ERROR(PI_BAD_USER_GPIO, "bad gpio (%d)", gpio);

   return (pwmFreq[gpioInfo[gpio].freqIdx]);
}


/* ----------------------------------------------------------------------- */

int gpioServo(unsigned gpio, unsigned val)
{
   DBG(DBG_USER, "gpio=%d pulsewidth=%d", gpio, val);

   CHECK_INITED;

   if (gpio > PI_MAX_USER_GPIO)
      SOFT_ERROR(PI_BAD_USER_GPIO, "bad gpio (%d)", gpio);

   if ((val!=PI_SERVO_OFF) && (val<PI_MIN_SERVO_PULSEWIDTH))
      SOFT_ERROR(PI_BAD_PULSEWIDTH,
         "gpio %d, bad pulsewidth (%d)", gpio, val);

   if (val>PI_MAX_SERVO_PULSEWIDTH)
      SOFT_ERROR(PI_BAD_PULSEWIDTH,
         "gpio %d, bad pulsewidth (%d)", gpio, val);

   if (gpioInfo[gpio].is != GPIO_SERVO)
   {
      if (gpioInfo[gpio].is == GPIO_UNDEFINED)
      {
         gpioSetMode(gpio, PI_OUTPUT);
      }
      else if (gpioInfo[gpio].is == GPIO_PWM)
      {
         /* switch pwm off */
         myGpioSetPwm(gpio, gpioInfo[gpio].width, 0);
         gpioInfo[gpio].width=0;
      }
      gpioInfo[gpio].is = GPIO_SERVO;
      gpioInfo[gpio].freqIdx = clkCfg[gpioCfg.clockMicros].servoIdx;
   }

   myGpioSetServo(gpio, gpioInfo[gpio].width, val);

   gpioInfo[gpio].width=val;

   return 0;
}


/* ----------------------------------------------------------------------- */

int gpioWaveGetMicros(void)
{
   DBG(DBG_USER, "");

   CHECK_INITED;

   return wfStats.micros;
}


/* ----------------------------------------------------------------------- */

int gpioWaveGetHighMicros(void)
{
   DBG(DBG_USER, "");

   CHECK_INITED;

   return wfStats.highMicros;
}


/* ----------------------------------------------------------------------- */

int gpioWaveGetMaxMicros(void)
{
   DBG(DBG_USER, "");

   CHECK_INITED;

   return wfStats.maxMicros;
}


/* ----------------------------------------------------------------------- */

int gpioWaveGetPulses(void)
{
   DBG(DBG_USER, "");

   CHECK_INITED;

   return wfStats.pulses;
}


/* ----------------------------------------------------------------------- */

int gpioWaveGetHighPulses(void)
{
   DBG(DBG_USER, "");

   CHECK_INITED;

   return wfStats.highPulses;
}


/* ----------------------------------------------------------------------- */

int gpioWaveGetMaxPulses(void)
{
   DBG(DBG_USER, "");

   CHECK_INITED;

   return wfStats.maxPulses;
}


/* ----------------------------------------------------------------------- */

int gpioWaveGetCbs(void)
{
   DBG(DBG_USER, "");

   CHECK_INITED;

   return wfStats.cbs;
}


/* ----------------------------------------------------------------------- */

int gpioWaveGetHighCbs(void)
{
   DBG(DBG_USER, "");

   CHECK_INITED;

   return wfStats.highCbs;
}


/* ----------------------------------------------------------------------- */

int gpioWaveGetMaxCbs(void)
{
   DBG(DBG_USER, "");

   CHECK_INITED;

   return wfStats.maxCbs;
}


/* ----------------------------------------------------------------------- */

int gpioWaveClear(void)
{
   DBG(DBG_USER, "");

   CHECK_INITED;

   wfc[0] = 0;
   wfc[1] = 0;
   wfc[2] = 0;

   wfcur = 0;

   wfStats.micros = 0;
   wfStats.pulses = 0;
   wfStats.cbs    = 0;

   return 0;
}


/* ----------------------------------------------------------------------- */

int gpioWaveAddGeneric(unsigned numPulses, gpioPulse_t * pulses)
{
   DBG(DBG_USER, "numPulses=%u pulses=%08X", numPulses, (uint32_t)pulses);

   CHECK_INITED;

   if (numPulses > PI_WAVE_MAX_PULSES)
      SOFT_ERROR(PI_TOO_MANY_PULSES, "bad number of pulses (%d)", numPulses);

   return waveMerge(numPulses, pulses);
}


/* ----------------------------------------------------------------------- */

int gpioWaveAddSerial(unsigned gpio,
                      unsigned baud,
                      unsigned offset,
                      unsigned numChar,
                      char     *str)
{
   int i, b, p, lev, c, v;

   unsigned bitDelay[10];

   DBG(DBG_USER, "gpio=%d baud=%d offset=%d numChar=%d str=%s",
      gpio, baud, offset, numChar, str);

   DBG(DBG_USER, "l=%d s=%X e=%X",
      strlen(str), str[0], str[strlen(str)-1]);

   CHECK_INITED;

   if (gpio > PI_MAX_USER_GPIO)
      SOFT_ERROR(PI_BAD_USER_GPIO, "bad gpio (%d)", gpio);

   if ((baud < PI_WAVE_MIN_BAUD) || (baud > PI_WAVE_MAX_BAUD))
      SOFT_ERROR(PI_BAD_WAVE_BAUD,
         "gpio %d, bad baud rate (%d)", gpio, baud);

   if (numChar > PI_WAVE_MAX_CHARS)
      SOFT_ERROR(PI_TOO_MANY_CHARS, "too many chars (%d)", numChar);

   if (offset > PI_WAVE_MAX_MICROS)
      SOFT_ERROR(PI_BAD_SER_OFFSET, "offset too large (%d)", offset);

   if (!numChar) return 0;

   waveBitDelay(baud, bitDelay);

   p = 0;

   wf[2][p].gpioOn  = (1<<gpio);
   wf[2][p].gpioOff = 0;

   if (offset > bitDelay[0]) wf[2][p].usDelay = offset;
   else                      wf[2][p].usDelay = bitDelay[0];

   for (i=0; i<numChar; i++)
   {
      p++;
   
      /* start bit */

      wf[2][p].gpioOn = 0;
      wf[2][p].gpioOff = (1<<gpio);
      wf[2][p].usDelay = bitDelay[0];

      lev = 0;

      c = str[i];

      for (b=0; b<8; b++)
      {
         if (c & (1<<b)) v=1; else v=0;

         if (v == lev) wf[2][p].usDelay += bitDelay[b+1];
         else
         {
            p++;

            lev = v;

            if (lev)
            {
               wf[2][p].gpioOn = (1<<gpio);
               wf[2][p].gpioOff = 0;
            }
            else
            {
               wf[2][p].gpioOn = 0;
               wf[2][p].gpioOff = (1<<gpio);
            }

            wf[2][p].usDelay = bitDelay[b+1];
         }
      }

      /* stop bit */

      if (lev) wf[2][p].usDelay += bitDelay[9];
      else
      {
         p++;

         wf[2][p].gpioOn = (1<<gpio);
         wf[2][p].gpioOff = 0;
         wf[2][p].usDelay = bitDelay[9];
      }
   }

   p++;

   wf[2][p].gpioOn  = (1<<gpio);
   wf[2][p].gpioOff = 0;
   wf[2][p].usDelay = bitDelay[0];

   return waveMerge(p, wf[2]);
}


/*-------------------------------------------------------------------------*/

int gpioSerialReadOpen(unsigned gpio, unsigned baud)
{
   int bitTime, timeout;

   DBG(DBG_USER, "gpio=%d baud=%d", gpio, baud);

   CHECK_INITED;

   if (gpio > PI_MAX_USER_GPIO)
      SOFT_ERROR(PI_BAD_USER_GPIO, "bad gpio (%d)", gpio);

   if ((baud < PI_WAVE_MIN_BAUD) || (baud > PI_WAVE_MAX_BAUD))
      SOFT_ERROR(PI_BAD_WAVE_BAUD,
         "gpio %d, bad baud rate (%d)", gpio, baud);

   if (wfRx[gpio].mode != PI_WFRX_NONE)
      SOFT_ERROR(PI_GPIO_IN_USE, "gpio %d is already being used", gpio);

   bitTime = MILLION / baud;

   timeout  = (10 * bitTime)/1000;
   if (timeout < 1) timeout = 1;

   wfRx[gpio].gpio     = gpio;
   wfRx[gpio].buf      = malloc(SRX_BUF_SIZE);
   wfRx[gpio].bufSize  = SRX_BUF_SIZE;
   wfRx[gpio].mode     = PI_WFRX_SERIAL;
   wfRx[gpio].baud     = baud;
   wfRx[gpio].timeout  = timeout;
   wfRx[gpio].fullBit  = bitTime;
   wfRx[gpio].halfBit  = bitTime/2;
   wfRx[gpio].readPos  = 0;
   wfRx[gpio].writePos = 0;
   wfRx[gpio].bit      = -1;

   gpioSetAlertFunc(gpio, waveRxBit);

   return 0;
}

/*-------------------------------------------------------------------------*/

int gpioSerialRead(unsigned gpio, void *buf, size_t bufSize)
{
   unsigned bytes=0, wpos;
   volatile wfRx_t *p;

   DBG(DBG_USER, "gpio=%d buf=%08X bufSize=%d", gpio, (int)buf, bufSize);

   CHECK_INITED;

   if (gpio > PI_MAX_USER_GPIO)
      SOFT_ERROR(PI_BAD_USER_GPIO, "bad gpio (%d)", gpio);

   if (bufSize == 0)
      SOFT_ERROR(PI_BAD_SERIAL_COUNT, "buffer size can't be zero");

   if (wfRx[gpio].mode != PI_WFRX_SERIAL)
      SOFT_ERROR(PI_NOT_SERIAL_GPIO, "no serial read on gpio (%d)", gpio);

   p = &wfRx[gpio];

   if (p->readPos != p->writePos)
   {
      wpos = p->writePos;

      if (wpos > p->readPos) bytes = wpos - p->readPos;
      else                   bytes = p->bufSize - p->readPos;

      if (bytes > bufSize) bytes = bufSize;

      memcpy(buf, p->buf+p->readPos, bytes);

      p->readPos += bytes;

      if (p->readPos >= p->bufSize) p->readPos = 0;
   }
   return bytes;
}


/*-------------------------------------------------------------------------*/

int gpioSerialReadClose(unsigned gpio)
{
   DBG(DBG_USER, "gpio=%d", gpio);

   CHECK_INITED;

   if (gpio > PI_MAX_USER_GPIO)
      SOFT_ERROR(PI_BAD_USER_GPIO, "bad gpio (%d)", gpio);

   switch(wfRx[gpio].mode)
   {
      case PI_WFRX_NONE:

         SOFT_ERROR(PI_NOT_SERIAL_GPIO, "no serial read on gpio (%d)", gpio);

         break;

      case PI_WFRX_SERIAL:

         free(wfRx[gpio].buf);

         gpioSetWatchdog(gpio, 0); /* switch off timeouts */

         gpioSetAlertFunc(gpio, NULL); /* cancel alert */

         wfRx[gpio].mode = PI_WFRX_NONE;

         break;
   }

   return 0;
}


/*-------------------------------------------------------------------------*/

int gpioWaveTxBusy(void)
{
   DBG(DBG_USER, "");

   CHECK_INITED;

   if (dmaOut[DMA_CONBLK_AD])
      return 1;
   else
      return 0;
}


/* ----------------------------------------------------------------------- */

int gpioWaveTxStart(unsigned mode)
{
   static int secondaryClockInited = 0;

   int cb, i;

   DBG(DBG_USER, "mode=%d", mode);

   CHECK_INITED;

   if (mode > PI_WAVE_MODE_REPEAT)
      SOFT_ERROR(PI_BAD_WAVE_MODE, "bad wave mode (%d)", mode);

   if (wfc[wfcur] == 0) return 0;
   
   if (!secondaryClockInited)
   {
      initClock(0); /* initialise secondary clock */
      secondaryClockInited = 1;
   }

   cb = wave2Cbs(mode);

   if (gpioCfg.dbgLevel >= DBG_SLOW_TICK)
   {
      fprintf(stderr, "*** OUTPUT DMA CONTROL BLOCKS ***\n");
      for (i=0; i<cb; i++) waveCbOPrint(i);
   }

   initDMAgo((uint32_t *)dmaOut, (uint32_t)dmaOPhys[0]);

   return cb;
}


/* ----------------------------------------------------------------------- */

int gpioWaveTxStop(void)
{
   DBG(DBG_USER, "");

   CHECK_INITED;

   dmaOut[DMA_CS] = DMA_CHANNEL_RESET;

   dmaOut[DMA_CONBLK_AD] = 0;

   return 0;
}


/* ----------------------------------------------------------------------- */

static int intGpioSetAlertFunc(
   unsigned gpio,
   void *   f,
   int      user,
   void *   userdata)
{
   DBG(DBG_INTERNAL, "gpio=%d function=%08X, user=%d, userdata=%08X",
      gpio, (uint32_t)f, user, (uint32_t)userdata);

   gpioAlert[gpio].ex = user;
   gpioAlert[gpio].userdata = userdata;

   gpioAlert[gpio].func = f;

   if (f)
   {
      alertBits |= BIT;
   }
   else
   {
      alertBits &= ~BIT;
   }

   monitorBits = alertBits | notifyBits | gpioGetSamples.bits;

   return 0;
}

/* ----------------------------------------------------------------------- */

int gpioSetAlertFunc(unsigned gpio, gpioAlertFunc_t f)
{
   DBG(DBG_USER, "gpio=%d function=%08X", gpio, (uint32_t)f);

   CHECK_INITED;

   if (gpio > PI_MAX_USER_GPIO)
      SOFT_ERROR(PI_BAD_USER_GPIO, "bad gpio (%d)", gpio);

   intGpioSetAlertFunc(gpio, f, 0, NULL);

   return 0;
}


/* ----------------------------------------------------------------------- */

int gpioSetAlertFuncEx(unsigned gpio, gpioAlertFuncEx_t f, void * userdata)
{
   DBG(DBG_USER, "gpio=%d function=%08X userdata=%08X",
      gpio, (uint32_t)f, (uint32_t)userdata);

   CHECK_INITED;

   if (gpio > PI_MAX_USER_GPIO)
      SOFT_ERROR(PI_BAD_USER_GPIO, "bad gpio (%d)", gpio);

   intGpioSetAlertFunc(gpio, f, 1, userdata);

   return 0;
}


/* ----------------------------------------------------------------------- */

int gpioNotifyOpen(void)
{
   int i, slot, fd;
   char name[32];

   DBG(DBG_USER, "");

   CHECK_INITED;

   slot = -1;

   for (i=0; i<PI_NOTIFY_SLOTS; i++)
   {
      if (gpioNotify[i].state == PI_NOTIFY_CLOSED)
      {
         slot = i;
         break;
      }
   }

   if (slot < 0)
      SOFT_ERROR(PI_NO_HANDLE, "no handle");

   sprintf(name, "/dev/pigpio%d", slot);

   myCreatePipe(name, 0664);

   fd = open(name, O_RDWR|O_NONBLOCK);

   if (fd < 0)
      SOFT_ERROR(PI_BAD_PATHNAME, "open %s failed (%m)", name);

   gpioNotify[slot].state = PI_NOTIFY_OPENED;
   gpioNotify[slot].seqno = 0;
   gpioNotify[slot].bits  = 0;
   gpioNotify[slot].fd    = fd;
   gpioNotify[slot].pipe  = 1;

   return slot;
}


/* ----------------------------------------------------------------------- */

static int gpioNotifyOpenInBand(int fd)
{
   int i, slot;

   DBG(DBG_USER, "");

   CHECK_INITED;

   slot = -1;

   for (i=0; i<PI_NOTIFY_SLOTS; i++)
   {
      if (gpioNotify[i].state == PI_NOTIFY_CLOSED)
      {
         slot = i;
         break;
      }
   }

   if (slot < 0) SOFT_ERROR(PI_NO_HANDLE, "no handle");

   gpioNotify[slot].state = PI_NOTIFY_OPENED;
   gpioNotify[slot].seqno = 0;
   gpioNotify[slot].bits  = 0;
   gpioNotify[slot].fd    = fd;
   gpioNotify[slot].pipe  = 0;

   return slot;
}


/* ----------------------------------------------------------------------- */

static void intNotifyBits(void)
{
   int i;
   uint32_t bits;

   bits = 0;

   for (i=0; i<PI_NOTIFY_SLOTS; i++)
   {
      if (gpioNotify[i].state == PI_NOTIFY_RUNNING)
      {
         bits |= gpioNotify[i].bits;
      }
   }

   notifyBits = bits;

   monitorBits = alertBits | notifyBits | gpioGetSamples.bits;
}


/* ----------------------------------------------------------------------- */

int gpioNotifyBegin(unsigned handle, uint32_t bits)
{
   DBG(DBG_USER, "handle=%d bits=%08X", handle, bits);

   CHECK_INITED;

   if (handle > PI_NOTIFY_SLOTS)
      SOFT_ERROR(PI_BAD_HANDLE, "bad handle (%d)", handle);

   if (gpioNotify[handle].state <= PI_NOTIFY_CLOSING)
      SOFT_ERROR(PI_BAD_HANDLE, "bad handle (%d)", handle);

   gpioNotify[handle].bits  = bits;

   gpioNotify[handle].state = PI_NOTIFY_RUNNING;

   intNotifyBits();

   return 0;
}


/* ----------------------------------------------------------------------- */

int gpioNotifyPause (unsigned handle)
{
   DBG(DBG_USER, "handle=%d", handle);

   CHECK_INITED;

   if (handle > PI_NOTIFY_SLOTS)
      SOFT_ERROR(PI_BAD_HANDLE, "bad handle (%d)", handle);

   if (gpioNotify[handle].state <= PI_NOTIFY_CLOSING)
      SOFT_ERROR(PI_BAD_HANDLE, "bad handle (%d)", handle);

   gpioNotify[handle].bits  = 0;

   gpioNotify[handle].state = PI_NOTIFY_PAUSED;

   intNotifyBits();

   return 0;
}


/* ----------------------------------------------------------------------- */

int gpioNotifyClose(unsigned handle)
{
   DBG(DBG_USER, "handle=%d", handle);

   CHECK_INITED;

   if (handle > PI_NOTIFY_SLOTS)
      SOFT_ERROR(PI_BAD_HANDLE, "bad handle (%d)", handle);

   if (gpioNotify[handle].state <= PI_NOTIFY_CLOSING)
      SOFT_ERROR(PI_BAD_HANDLE, "bad handle (%d)", handle);

   gpioNotify[handle].bits  = 0;

   gpioNotify[handle].state = PI_NOTIFY_CLOSING;

   intNotifyBits();

   /* actual close done in alert thread */

   return 0;
}

/* ----------------------------------------------------------------------- */

int gpioTrigger(unsigned gpio, unsigned pulseLen, unsigned level)
{
   DBG(DBG_USER, "gpio=%d pulseLen=%d level=%d", gpio, pulseLen, level);

   CHECK_INITED;

   if (gpio > PI_MAX_USER_GPIO)
      SOFT_ERROR(PI_BAD_USER_GPIO, "bad gpio (%d)", gpio);

   if (level > PI_ON)
      SOFT_ERROR(PI_BAD_LEVEL, "gpio %d, bad level (%d)", gpio, level);

   if (pulseLen > PI_MAX_PULSELEN)
      SOFT_ERROR(PI_BAD_PULSELEN,
         "gpio %d, bad pulseLen (%d)", gpio, pulseLen);

   if (level == PI_OFF) *(gpioReg + GPCLR0 + BANK) = BIT;
   else                 *(gpioReg + GPSET0 + BANK) = BIT;

   myGpioDelay(pulseLen);

   if (level != PI_OFF) *(gpioReg + GPCLR0 + BANK) = BIT;
   else                 *(gpioReg + GPSET0 + BANK) = BIT;

   return 0;
}


/* ----------------------------------------------------------------------- */

int gpioSetWatchdog(unsigned gpio, unsigned timeout)
{
   DBG(DBG_USER, "gpio=%d timeout=%d", gpio, timeout);

   CHECK_INITED;

   if (gpio > PI_MAX_USER_GPIO)
      SOFT_ERROR(PI_BAD_USER_GPIO, "bad gpio (%d)", gpio);

   if (timeout > PI_MAX_WDOG_TIMEOUT)
      SOFT_ERROR(PI_BAD_WDOG_TIMEOUT,
         "gpio %d, bad timeout (%d)", gpio, timeout);

   gpioAlert[gpio].timeout = timeout;
   gpioAlert[gpio].tick    = systReg[SYST_CLO];

   return 0;
}

/* ----------------------------------------------------------------------- */

int gpioSetGetSamplesFunc(gpioGetSamplesFunc_t f, uint32_t bits)
{
   DBG(DBG_USER, "function=%08X bits=%08X", (uint32_t)f, bits);

   CHECK_INITED;

   gpioGetSamples.ex       = 0;
   gpioGetSamples.userdata = NULL;
   gpioGetSamples.func     = f;

   if (f) gpioGetSamples.bits = bits;
   else   gpioGetSamples.bits = 0;

   monitorBits = alertBits | notifyBits | gpioGetSamples.bits;

   return 0;
}


/* ----------------------------------------------------------------------- */

int gpioSetGetSamplesFuncEx(gpioGetSamplesFuncEx_t f,
                            uint32_t bits,
                            void * userdata)
{
   DBG(DBG_USER, "function=%08X bits=%08X", (uint32_t)f, bits);

   CHECK_INITED;

   gpioGetSamples.ex       = 1;
   gpioGetSamples.userdata = userdata;
   gpioGetSamples.func     = f;

   if (f) gpioGetSamples.bits = bits;
   else   gpioGetSamples.bits = 0;

   monitorBits = alertBits | notifyBits | gpioGetSamples.bits;

   return 0;
}


/* ----------------------------------------------------------------------- */

static int intGpioSetTimerFunc(unsigned id,
                               unsigned ms,
                               void * f,
                               int user,
                               void * userdata)
{
   DBG(DBG_INTERNAL, "id=%d ms=%d function=%08X user=%d userdata=%08X",
      id, ms, (uint32_t)f, user, (uint32_t)userdata);

   gpioTimer[id].id   = id;

   if (f)
   {
      gpioTimer[id].func     = f;
      gpioTimer[id].ex       = user;
      gpioTimer[id].userdata = userdata;
      gpioTimer[id].millis   = ms;

      if (!gpioTimer[id].running)
      {
         if (pthread_create(
            &gpioTimer[id].pthId, &pthAttr, pthTimerTick, &gpioTimer[id]))
               SOFT_ERROR(PI_TIMER_FAILED,
                  "timer %d, create failed (%m)", id);

         gpioTimer[id].running = 1;
      }
   }
   else
   {
      if (gpioTimer[id].running)
      {
         /* destroy thread */

         if (pthread_cancel(gpioTimer[id].pthId))
            SOFT_ERROR(PI_TIMER_FAILED, "timer %d, cancel failed (%m)", id);

         if (pthread_join(gpioTimer[id].pthId, NULL))
            SOFT_ERROR(PI_TIMER_FAILED, "timer %d, join failed (%m)", id);

         gpioTimer[id].running = 0;
         gpioTimer[id].func    = f;
      }
   }

   return 0;
}


/* ----------------------------------------------------------------------- */

int gpioSetTimerFunc(unsigned id, unsigned ms, gpioTimerFunc_t f)
{
   DBG(DBG_USER, "id=%d ms=%d function=%08X", id, ms, (uint32_t)f);

   CHECK_INITED;

   if (id > PI_MAX_TIMER)
      SOFT_ERROR(PI_BAD_TIMER, "bad timer id (%d)", id);

   if ((ms < PI_MIN_MS) || (ms > PI_MAX_MS))
      SOFT_ERROR(PI_BAD_MS, "timer %d, bad ms (%d)", id, ms);

   intGpioSetTimerFunc(id, ms, f, 0, NULL);

   return 0;
}


/* ----------------------------------------------------------------------- */

int gpioSetTimerFuncEx(unsigned id, unsigned ms, gpioTimerFuncEx_t f,
                       void * userdata)
{
   DBG(DBG_USER, "id=%d ms=%d function=%08X, userdata=%08X",
      id, ms, (uint32_t)f, (uint32_t)userdata);

   CHECK_INITED;

   if (id > PI_MAX_TIMER)
      SOFT_ERROR(PI_BAD_TIMER, "bad timer id (%d)", id);

   if ((ms < PI_MIN_MS) || (ms > PI_MAX_MS))
      SOFT_ERROR(PI_BAD_MS, "timer %d, bad ms (%d)", id, ms);

   intGpioSetTimerFunc(id, ms, f, 1, userdata);

   return 0;
}

/* ----------------------------------------------------------------------- */

pthread_t *gpioStartThread(gpioThreadFunc_t func, void *arg)
{
   pthread_t *pth;
   pthread_attr_t pthAttr;

   DBG(DBG_USER, "func=%08X, arg=%08X", (uint32_t)func, (uint32_t)arg);

   CHECK_INITED_RET_NULL_PTR;

   pth = malloc(sizeof(pthread_t));

   if (pth)
   {
      if (pthread_attr_init(&pthAttr))
      {
         free(pth);
         SOFT_ERROR(NULL, "pthread_attr_init failed");
      }

      if (pthread_attr_setstacksize(&pthAttr, STACK_SIZE))
      {
         free(pth);
         SOFT_ERROR(NULL, "pthread_attr_setstacksize failed");
      }

      if (pthread_create(pth, &pthAttr, func, arg))
      {
         free(pth);
         SOFT_ERROR(NULL, "pthread_create failed");
      }
   }
   return pth;
}

/* ----------------------------------------------------------------------- */

void gpioStopThread(pthread_t *pth)
{
   DBG(DBG_USER, "pth=%08X", (uint32_t)pth);

   CHECK_INITED_RET_NIL;

   if (pth)
   {
      pthread_cancel(*pth);
      pthread_join(*pth, NULL);
   }
}

/* ----------------------------------------------------------------------- */

int gpioStoreScript(char *script)
{
   DBG(DBG_USER, "script=%s", script);
   DBG(DBG_USER, "l=%d s=%X e=%X",
      strlen(script), script[0], script[strlen(script)-1]);

   CHECK_INITED;

   return PI_BAD_SCRIPT;
}



/* ----------------------------------------------------------------------- */

int gpioRunScript(int script_id)
{
   DBG(DBG_USER, "script_id=%d", script_id);

   CHECK_INITED;

   return PI_BAD_SCRIPT_ID;
}



/* ----------------------------------------------------------------------- */

int gpioStopScript(int script_id)
{
   DBG(DBG_USER, "script_id=%d", script_id);

   CHECK_INITED;

   return PI_BAD_SCRIPT_ID;
}



/* ----------------------------------------------------------------------- */

int gpioDeleteScript(int script_id)
{
   DBG(DBG_USER, "script_id=%d", script_id);

   CHECK_INITED;

   return PI_BAD_SCRIPT_ID;
}



/* ----------------------------------------------------------------------- */

int gpioSetSignalFunc(unsigned signum, gpioSignalFunc_t f)
{
   DBG(DBG_USER, "signum=%d function=%08X", signum, (uint32_t)f);

   CHECK_INITED;

   if (signum > PI_MAX_SIGNUM)
      SOFT_ERROR(PI_BAD_SIGNUM, "bad signum (%d)", signum);

   gpioSignal[signum].ex = 0;
   gpioSignal[signum].userdata = NULL;

   gpioSignal[signum].func = f;

   return 0;
}


/* ----------------------------------------------------------------------- */

int gpioSetSignalFuncEx(unsigned signum, gpioSignalFuncEx_t f,
                        void * userdata)
{
   DBG(DBG_USER, "signum=%d function=%08X userdata=%08X",
      signum, (uint32_t)f, (uint32_t)userdata);

   CHECK_INITED;

   if (signum > PI_MAX_SIGNUM)
      SOFT_ERROR(PI_BAD_SIGNUM, "bad signum (%d)", signum);

   gpioSignal[signum].ex = 1;
   gpioSignal[signum].userdata = userdata;

   gpioSignal[signum].func = f;

   return 0;
}


/* ----------------------------------------------------------------------- */

uint32_t gpioRead_Bits_0_31(void)
{
   DBG(DBG_USER, "");

   CHECK_INITED;

   return (*(gpioReg + GPLEV0));
}


/* ----------------------------------------------------------------------- */

uint32_t gpioRead_Bits_32_53(void)
{
   DBG(DBG_USER, "");

   CHECK_INITED;

   return (*(gpioReg + GPLEV1));
}


/* ----------------------------------------------------------------------- */

int gpioWrite_Bits_0_31_Clear(uint32_t levels)
{
   DBG(DBG_USER, "levels=%08X", levels);

   CHECK_INITED;

   *(gpioReg + GPCLR0) = levels;

   return 0;
}


/* ----------------------------------------------------------------------- */

int gpioWrite_Bits_32_53_Clear(uint32_t levels)
{
   DBG(DBG_USER, "levels=%08X", levels);

   CHECK_INITED;

   *(gpioReg + GPCLR1) = levels;

   return 0;
}


/* ----------------------------------------------------------------------- */

int gpioWrite_Bits_0_31_Set(uint32_t levels)
{
   DBG(DBG_USER, "levels=%08X", levels);

   CHECK_INITED;

   *(gpioReg + GPSET0) = levels;

   return 0;
}


/* ----------------------------------------------------------------------- */

int gpioWrite_Bits_32_53_Set(uint32_t levels)
{
   DBG(DBG_USER, "levels=%08X", levels);

   CHECK_INITED;

   *(gpioReg + GPSET1) = levels;

   return 0;
}


/* ----------------------------------------------------------------------- */

int gpioTime(unsigned timetype, int * seconds, int * micros)
{
   struct timespec ts;

   DBG(DBG_USER, "timetype=%d &seconds=%08X &micros=%08X",
      timetype, (uint32_t)seconds, (uint32_t)micros);

   CHECK_INITED;

   if (timetype > PI_TIME_ABSOLUTE)
      SOFT_ERROR(PI_BAD_TIMETYPE, "bad timetype (%d)", timetype);

   if (timetype == PI_TIME_ABSOLUTE)
   {
      clock_gettime(CLOCK_REALTIME, &ts);
      *seconds = ts.tv_sec;
      *micros  = ts.tv_nsec/1000;
   }
   else
   {
      clock_gettime(CLOCK_REALTIME, &ts);

      TIMER_SUB(&ts, &libStarted, &ts);

      *seconds = ts.tv_sec;
      *micros  = ts.tv_nsec/1000;
   }

   return 0;
}


/* ----------------------------------------------------------------------- */

int gpioSleep(unsigned timetype, int seconds, int micros)
{
   struct timespec ts, rem;

   DBG(DBG_USER, "timetype=%d seconds=%d micros=%d",
      timetype, seconds, micros);

   CHECK_INITED;

   if (timetype > PI_TIME_ABSOLUTE)
      SOFT_ERROR(PI_BAD_TIMETYPE, "bad timetype (%d)", timetype);

   if (seconds < 0)
      SOFT_ERROR(PI_BAD_SECONDS, "bad seconds (%d)", seconds);

   if ((micros < 0) || (micros > 999999))
      SOFT_ERROR(PI_BAD_MICROS, "bad micros (%d)", micros);

   ts.tv_sec  = seconds;
   ts.tv_nsec = micros * 1000;

   if (timetype == PI_TIME_ABSOLUTE)
   {
      while (clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &ts, &rem));
   }
   else
   {
      while (clock_nanosleep(CLOCK_REALTIME, 0, &ts, &rem))
      {
         /* copy remaining time to ts */
         ts.tv_sec  = rem.tv_sec;
         ts.tv_nsec = rem.tv_nsec;
      }
   }

   return 0;
}


/* ----------------------------------------------------------------------- */

uint32_t gpioDelay(uint32_t micros)
{
   uint32_t start;

   DBG(DBG_USER, "microseconds=%u", micros);

   CHECK_INITED;

   start = systReg[SYST_CLO];

   if (micros < 100) while ((systReg[SYST_CLO] - start) <= micros) ;

   else gpioSleep(PI_TIME_RELATIVE, (micros/MILLION), (micros%MILLION));

   return (systReg[SYST_CLO] - start);
}


/* ----------------------------------------------------------------------- */

uint32_t gpioTick(void)
{
   CHECK_INITED;

   return systReg[SYST_CLO];
}


/* ----------------------------------------------------------------------- */

unsigned gpioVersion(void)
{
   DBG(DBG_USER, "");

   return PIGPIO_VERSION;
}


/* ----------------------------------------------------------------------- */

unsigned gpioHardwareRevision(void)
{
   static unsigned rev = 0;

   FILE * filp;
   char buf[512];
   char term;

   DBG(DBG_USER, "");

   if (rev) return rev;

   filp = fopen ("/proc/cpuinfo", "r");

   if (filp != NULL)
   {
      while (fgets(buf, sizeof(buf), filp) != NULL)
      {
         if (!strncasecmp("revision\t", buf, 9))
         {
            if (sscanf(buf+strlen(buf)-5, "%x%c", &rev, &term) == 2)
            {
               if (term == '\n') break;
               rev = 0;
            }
         }
      }
      fclose(filp);
   }
   return rev;
}


/* ----------------------------------------------------------------------- */

int gpioCfgBufferSize(unsigned millis)
{
   DBG(DBG_USER, "millis=%d", millis);

   CHECK_NOT_INITED;

   if ((millis < PI_BUF_MILLIS_MIN) || (millis > PI_BUF_MILLIS_MAX))
      SOFT_ERROR(PI_BAD_BUF_MILLIS, "bad millis (%d)", millis);

   gpioCfg.bufferMilliseconds = millis;

   return 0;
}


/* ----------------------------------------------------------------------- */

int gpioCfgClock(unsigned micros, unsigned peripheral, unsigned source)
{
   DBG(DBG_USER, "micros=%d peripheral=%d source=%d",
      micros, peripheral, source);

   CHECK_NOT_INITED;

   if ((micros < 1) || (micros > 10))
      SOFT_ERROR(PI_BAD_CLK_MICROS, "bad micros (%d)", micros);

   if (!clkCfg[micros].valid)
      SOFT_ERROR(PI_BAD_CLK_MICROS, "bad micros (%d)", micros);

   if (peripheral > PI_CLOCK_PCM)
      SOFT_ERROR(PI_BAD_CLK_PERIPH, "bad peripheral (%d)", peripheral);

   if (source > PI_CLOCK_PLLD)
      SOFT_ERROR(PI_BAD_CLK_SOURCE, "bad clock (%d)", source);

   gpioCfg.clockMicros = micros;
   gpioCfg.clockPeriph = peripheral;
   gpioCfg.clockSource = source;

   return 0;
}


/* ----------------------------------------------------------------------- */

int gpioCfgDMAchannel(unsigned channel)
{
   DBG(DBG_USER, "channel=%d", channel);

   CHECK_NOT_INITED;

   if ((channel < PI_MIN_DMA_CHANNEL) || (channel > PI_MAX_DMA_CHANNEL))
      SOFT_ERROR(PI_BAD_CHANNEL, "bad channel (%d)", channel);

   gpioCfg.DMAprimaryChannel = channel;

   return 0;
}


/* ----------------------------------------------------------------------- */

int gpioCfgDMAchannels(unsigned primaryChannel, unsigned secondaryChannel)
{
   DBG(DBG_USER, "primary channel=%d, secondary channel=%d",
      primaryChannel, secondaryChannel);

   CHECK_NOT_INITED;

   if (primaryChannel > PI_MAX_PRIMARY_CHANNEL)
      SOFT_ERROR(PI_BAD_PRIM_CHANNEL, "bad primary channel (%d)",
         primaryChannel);

   if (secondaryChannel > PI_MAX_SECONDARY_CHANNEL)
      SOFT_ERROR(PI_BAD_SECO_CHANNEL, "bad secondary channel (%d)",
         secondaryChannel);

   gpioCfg.DMAprimaryChannel   = primaryChannel;
   gpioCfg.DMAsecondaryChannel = secondaryChannel;

   return 0;
}


/*-------------------------------------------------------------------------*/

int gpioCfgPermissions(uint64_t updateMask)
{
   DBG(DBG_USER, "gpio update mask=%llX", updateMask);

   CHECK_NOT_INITED;

   gpioMask = updateMask;

   gpioMaskSet = 1;

   return 0;
}


/* ----------------------------------------------------------------------- */

int gpioCfgInterfaces(unsigned ifFlags)
{
   DBG(DBG_USER, "ifFlags=%X", ifFlags);

   CHECK_NOT_INITED;

   if (ifFlags > 3)
      SOFT_ERROR(PI_BAD_IF_FLAGS, "bad ifFlags (%X)", ifFlags);

   gpioCfg.ifFlags = ifFlags;

   return 0;
}


/* ----------------------------------------------------------------------- */

int gpioCfgSocketPort(unsigned port)
{
   DBG(DBG_USER, "port=%d", port);

   CHECK_NOT_INITED;

   if ((port < PI_MIN_SOCKET_PORT) || (port > PI_MAX_SOCKET_PORT))
      SOFT_ERROR(PI_BAD_SOCKET_PORT, "bad port (%d)", port);

   gpioCfg.socketPort = port;

   return 0;
}


/* ----------------------------------------------------------------------- */

int gpioCfgInternals(unsigned what, int value)
{
   int retVal = PI_BAD_CFG_INTERNAL;

   DBG(DBG_USER, "what=%u, value=%d", what, value);

   CHECK_NOT_INITED;

   /* 
   133084774
   207081315 
   293640712
   394342930
   472769257
   430873902
   635370313
   684442696
   786301093
   816051706
   858202631
   997413601
   */

   switch(what)
   {
      case 562484977:

         gpioCfg.showStats = value;

         DBG(DBG_MIN_LEVEL, "showStats is %u", value);

         retVal = 0;

         break;

      case 984762879:

         if (value < DBG_MIN_LEVEL) value = DBG_MIN_LEVEL;

         if (value > DBG_MAX_LEVEL) value = DBG_MAX_LEVEL;

         gpioCfg.dbgLevel = value;

         DBG(DBG_MIN_LEVEL, "Debug level is %u", value);

         retVal = 0;

         break;
   }

   return retVal;
}

