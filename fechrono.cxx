/********************************************************************\

  Name:         fechrono.cxx
  Created by:   AC
  Modified: Thomas Lindner for UCN (Nov 2018)

  Contents:     Frontend for the UCN Chronobox

\********************************************************************/

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "midas.h"
#include "cb.h"

  
/* make frontend functions callable from the C framework */
#ifdef __cplusplus
extern "C" {
#endif

  /*-- Globals -------------------------------------------------------*/

  /* The frontend name (client name) as seen by other MIDAS clients   */
  const char *frontend_name = "fechrono";
  

  /* The frontend file name, don't change it */
  const char *frontend_file_name = __FILE__;

  /* frontend_loop is called periodically if this variable is TRUE    */
  BOOL frontend_call_loop = FALSE;

  /* a frontend status page is displayed with this frequency in ms */
  INT display_period = 000;

  /* maximum event size produced by this frontend */
  INT max_event_size = 2*1024*1024;

  /* maximum event size for fragmented events (EQ_FRAGMENTED) */
  INT max_event_size_frag = 1024*1024;

  /* buffer size to hold events */
  INT event_buffer_size = 300*1024*1024;
  extern HNDLE hDB;


  /*-- Function declarations -----------------------------------------*/
  INT frontend_init();
  INT frontend_exit();
  INT begin_of_run(INT run_number, char *error);
  INT end_of_run(INT run_number, char *error);
  INT pause_run(INT run_number, char *error);
  INT resume_run(INT run_number, char *error);
  INT frontend_loop();
  
  INT read_cbhist(char *pevent, INT off);
  INT read_cbms(char *pevent, INT off);
  INT read_cbms_fifo(char *pevent, INT off);
  INT read_flow(char *pevent, INT off);

  /*-- Equipment list ------------------------------------------------*/
  
  EQUIPMENT equipment[] = {
#if 1
    {"chronobox%02d",               /* equipment name */
     { 10,                     /* event ID */
       (1<<10),                /* trigger mask */
       "SYSTEM",               /* event buffer */
       //EQ_MULTITHREAD,        /* equipment type */
       EQ_POLLED,              /* equipment type */
       0,                      /* event source */
       "MIDAS",                /* format */
       TRUE,                   /* enabled */
       RO_ALWAYS,              /* when to read this event */
       0,                     /* poll time in milliseconds */
       0,                      /* stop run after this event limit */
       0,                      /* number of sub events */
       0,                      /* whether to log history */
       "", "", "",},
     read_cbms_fifo,                /* readout routine */
     NULL,
     NULL,
     NULL,                     /* bank list */
    },
#endif
    {""}
  };
#ifdef __cplusplus
}
#endif
/********************************************************************\
              Callback routines for system transitions

  These routines are called whenever a system transition like start/
  stop of a run occurs. The routines are called on the following
  occations:

  frontend_init:  When the frontend program is started. This routine
                  should initialize the hardware.

  frontend_exit:  When the frontend program is shut down. Can be used
                  to releas any locked resources like memory, commu-
                  nications ports etc.

  begin_of_run:   When a new run is started. Clear scalers, open
                  rungates, etc.

  end_of_run:     Called on a request to stop a run. Can send
                  end-of-run event and close run gates.

  pause_run:      When a run is paused. Should disable trigger events.

  resume_run:     When a run is resumed. Should enable trigger events.
\********************************************************************/

Chronobox* gcb = NULL;

/*-- Global variables ----------------------------------------------*/

static int gHaveRun        = 0;
static int gRunNumber      = 0;
static int gCountEvents    = 0;

static int    gMcsClockChan = 59;  // channel number where the clock is
//static double gMcsClockFreq = 50000000.0; // clock frequency = 50 MHz
//static double gMcsClockFreq = 10000000.0; // clock frequency = 10 MHz
static double gMcsClockFreq = 100000000.0; // clock frequency = 100 MHz
const double gClock_period = 1./gMcsClockFreq;


// number of channels: 32+8+18+1
//static const int gCbChans = 60; // number of channels
static int gCbChans = 0; // number of channels, read from the chronobox itself
static const int gMcsChans = 32; // number of scaler channels 
static const int gFlowChan = 40; // first flowmeter channel
static const int gNflowChans = 8; // number of flowmeter channel

static uint32_t  gMaxChrono[60];  // max value of channels
static uint32_t  gSumChrono[60];  // sum channels
static uint32_t  gSaveChrono[60]; // sampled data
static uint32_t  gLastChrono[60]; // sampled data

uint32_t gFifoClock[60]={0};
uint32_t gPrevFifoClock[60]={0};
int NFifoChans=4;

uint32_t gPrevClock=0;
uint32_t gPrevClock2=0;
uint32_t gClock=0;

std::vector<std::string> ChannelNames;

extern INT frontend_index;
#include "utils.cxx"
/*-- Frontend Init -------------------------------------------------*/

INT frontend_init()
{

  printf("chronobox frontend_init start!\n");
  int status;

  setbuf(stdout,NULL);
  setbuf(stderr,NULL);


  Chronobox* cb = new Chronobox();

  cm_msg(MINFO, frontend_name, "Configuring Chronobox\n");
  status = cb->cb_open();

  if( status )
    {
      printf("status : %d\n",status);
      cm_msg(MERROR, frontend_name, "Chronobox Configuration FAILED");
      return status;
    }
  
  // assign to global pointer
  gcb=cb;
  gCbChans = gcb->cb_read_input_num(); 


  uint32_t fwrev = gcb->cb_read32(0);
  printf("cb fw rev: 0x%08x\n", fwrev);

  // Set the channels to report leading and trailing edges
  printf("Set the chronobox to report leading and trailing edges...\n");
  gcb->cb_write32(0x14,0xffffffff);
  gcb->cb_write32(0x15,0xffffffff);
  gcb->cb_write32(0x16,0xffffffff);
  gcb->cb_write32(0x17,0xffffffff);


  printf("chronobox %02d frontend_init done!\n",frontend_index);


  for(int j=0; j<gCbChans; ++j) gMaxChrono[j]=gSaveChrono[j]=gSumChrono[j]=uint32_t(0);
  char nchan[64];
  sprintf(nchan,"chronobox %02d, reading from %d inputs\n",frontend_index,gCbChans);
  cm_msg(MINFO, frontend_name, nchan);

  return SUCCESS;
}

/*-- Frontend Exit -------------------------------------------------*/

INT frontend_exit()
{
   // reset the counters
   //gcb->cb_write32bis(0, 2, 0);
   gHaveRun = 0;
   delete gcb;
   return SUCCESS;
}

/*-- Begin of Run --------------------------------------------------*/

INT begin_of_run(INT run_number, char *error)
{
  gRunNumber = run_number;
  gHaveRun = 1;
  gCountEvents = 0;


  for (int i=0; i<59; i++)
    gLastChrono[i]=0;
  
   uint32_t fwrev = gcb->cb_read32(0);
   printf("cb fw rev: 0x%08x\n", fwrev);
    
   // reset the counters
   gcb->cb_write32bis(0, 2, 0);


  printf("cb %02d begin run: %d\n",frontend_index,run_number);

  if( gcb ) return SUCCESS;
  return 0;
}


/*-- End of Run ----------------------------------------------------*/
INT end_of_run(INT run_number, char *error)
{
  gRunNumber = run_number;

  static bool gInsideEndRun = false;

  if (gInsideEndRun)
    {
      printf("breaking recursive end_of_run()\n");
      return SUCCESS;
    }
  
  gInsideEndRun = true;
  
  gHaveRun = 0;
  printf("cb %02d end run %d\n",frontend_index,run_number);
  cm_msg(MINFO, frontend_name, "Run %d finished, read %d events", 
	 run_number, gCountEvents);

  return SUCCESS;
}

/*-- Pause Run -----------------------------------------------------*/
INT pause_run(INT run_number, char *error)
{
  gHaveRun = 0;
  gRunNumber = run_number;
  return SUCCESS; 
}

/*-- Resume Run ----------------------------------------------------*/
INT resume_run(INT run_number, char *error)
{
  gHaveRun = 1;
  gRunNumber = run_number;
  return SUCCESS; 
}

/*-- Frontend Loop -------------------------------------------------*/
INT frontend_loop()
{
  printf("chronobox frontend_loop!\n");
  return SUCCESS;
}

/*-- Interrupt configuration ---------------------------------------*/
extern "C" INT interrupt_configure(INT cmd, INT source, PTYPE adr)
{
  assert(!"Not implemented");
}

extern "C" INT poll_event(INT source, INT count, BOOL test)
/* Polling routine for events. Returns TRUE if event
   is available. If test equals TRUE, don't return. The test
   flag is used to time the polling */
{
  //printf("poll_event %d %d %d!\n",source,count,test);

  for (int i = 0; i < count; i++)
    {
      if (test)
        usleep(1);
      else
        return TRUE;
    }
  return 0;
}

/*-- Event readout -------------------------------------------------*/

INT read_cbhist(char *pevent, INT off)
{
   if( gcb ) {
      if(0) cm_msg(MINFO, frontend_name, "Chronobox Read");
   } else {
      cm_msg(MERROR, frontend_name, "Chronobox Read FAILED");
      return 0;
   }
   /* init bank structure */
   bk_init32(pevent);
   double *p;
   char bankname[4];
   sprintf(bankname,"CBH%d",frontend_index);
   bk_create(pevent, bankname, TID_DOUBLE, (void**)&p);

   for (int i=0; i<gCbChans; i++)
      p[i] = 0;

   uint64_t numClocks = gClock-gPrevClock;
   gPrevClock=gClock;
   //  double dt = numClocks/gMcsClockFreq;
   double dt = numClocks*gClock_period;
   double dt1 = 0.;
   if (dt > 0.) 
      dt1 = 1.0/dt; 
   for (int i=0; i<gMcsChans; i++) { //Fifo could be asnycronus to other chans
      uint32_t counts=gSumChrono[i];
      p[i] = (counts)*dt1;  // sample RaTe in Hz
      if (counts>0)
         printf("ch: %d\tcnts: %d\tdelta: %1.6f s\trate: %1.3f Hz\n",
                i,counts,dt,p[i]);
   }
   p[gMcsClockChan] = (numClocks)*dt1;

   bk_close(pevent, p+gCbChans+1);

   for (int i=0; i<gMcsChans; i++)
      gSumChrono[i]=0;

   return bk_size(pevent);
}

bool FirstEvent=true;

struct ChronoChannelEvent {
//Time stamp events are stored as Channel + 100!
  uint8_t Channel;
  uint32_t Counts;
};


int ChansWithCounts=0;

std::vector<ChronoChannelEvent> buffer;
//[1200];
//Time stamp events:
uint32_t prev_ts = 0;
int prev_ch = 0;
static int num_scalers = 0;
static int count_scalers = 0;
static bool scalers_packet = false;



INT read_cbms_fifo(char *pevent, INT off)
{
   bool verbose = true;

   if( gcb ) {
      if(0) cm_msg(MINFO, frontend_name, "Chronobox Read");
   } else {
      cm_msg(MERROR, frontend_name, "Chronobox Read FAILED");
      return 0;
   }



   std::vector<uint32_t> fifo_data;

   gcb->cb_read_fifo(&fifo_data);

   int nread = fifo_data.size();

   if (verbose || 1) {
      //printf("read %d words\n", nread);
   }

   if (nread == 0) {
      return 0;
   }

   bool EVENT_GOOD=true;

   char bankname[4];
   //sprintf(bankname,"CBS0",frontend_index);
   /* init bank structure */
   bk_init32(pevent);
   uint32_t *pdata;
   /* create data bank */
   bk_create(pevent, "CBS0", TID_DWORD, (void**)&pdata);

   //   printf("Start loop...\n");
   for (int i=0; i<nread; i++) {
      //printf("Loop %i\n",i);
      uint32_t v = fifo_data[i];
      //printf("FFF %i\n",v);
      if (verbose) {
         //printf("read %3d: 0x%08x", i, v);
         printf("read %3d: %x\n", i, v);
      }
      
      // printf("pdata\n", i, v);
      *pdata++ = v;
      //   printf("pdata++\n", i, v);
   }
   
   fifo_data.clear();
   bk_close(pevent, pdata);
   return bk_size(pevent);

}

INT read_flow(char *pevent, INT off)
{
  if( frontend_index != 1 )
    return 0;

  if( gcb ) 
    {
      if(0) cm_msg(MINFO, frontend_name, "Chronobox Read Flow");
    }
  else
    {
      cm_msg(MERROR, frontend_name, "Chronobox Read FAILED");
      return 0;
    }

  /* init bank structure */
  bk_init32(pevent);
  double *p;
  bk_create(pevent, "CBFL", TID_DOUBLE, (void**)&p);

  uint64_t numClocks = gClock-gPrevClock2;
  gPrevClock2=gClock;
  double dt = numClocks*gClock_period, dt1 = 0.;
  if (dt > 0.) dt1 = 1.0/dt; 

  double TotFlow=0.;
  for( int i=0; i<gNflowChans; i++ )
    {
      uint32_t counts=gSumChrono[gFlowChan+i];
      double rate = (counts)*dt1;  // sample RaTe in Hz
      p[i] = rate * 1.e-2 + 0.065; // https://daq.triumf.ca/elog-alphag/alphag/1797

      if(0)
	printf("ch: %d\tcnts: %d\tdelta: %1.6f s\trate: %1.3f Hz\tflow: %1.2f l/min\n",
	       i,counts,dt,rate,p[i]);
      TotFlow += p[i];
    }
  //  p[gMcsClockChan] = (numClocks)*dt1;

  //Force bank to its historic size
  bk_close(pevent, p+gNflowChans);

  for (int i=0; i<gNflowChans; i++)
    gSumChrono[gFlowChan+i]=0;

  char stat[64];
  sprintf(stat,"Tot H20 Flow: %1.1f[l/min]",TotFlow);
  char col[64];
  if( TotFlow < 1. )
    sprintf(col,"yellow");
  else
    sprintf(col,"#00FF00");
  set_equipment_status("cbflow01",stat,col);

  return bk_size(pevent);
}


/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
