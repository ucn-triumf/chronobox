/********************************************************************\

  Name:         fechrono.cxx
  Created by:   AC

  Contents:     Frontend for the ALPHA Chronobox

\********************************************************************/


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

  extern INT run_state;
  extern HNDLE hDB;

  /*-- Function declarations -----------------------------------------*/
  INT frontend_init();
  INT frontend_exit();
  INT begin_of_run(INT run_number, char *error);
  INT end_of_run(INT run_number, char *error);
  INT pause_run(INT run_number, char *error);
  INT resume_run(INT run_number, char *error);
  INT frontend_loop();
  
  INT read_cb(char *pevent, INT off);

  /*-- Equipment list ------------------------------------------------*/
  
  EQUIPMENT equipment[] = {
    {"chronobox",              /* equipment name */
     { 10,                     /* event ID */
       0,                      /* trigger mask */
       "SYSTEM",               /* event buffer */
       EQ_PERIODIC,            /* equipment type */
       0,                      /* event source */
       "MIDAS",                /* format */
       TRUE,                   /* enabled */
       RO_ALWAYS,              /* when to read this event */
       //       1000,                   /* poll time in milliseconds */
       50,                   /* poll time in milliseconds */
       0,                      /* stop run after this event limit */
       0,                      /* number of sub events */
       1,                      /* whether to log history */
       "", "", "",},
     read_cb,                  /* readout routine */
     NULL,
     NULL,
     NULL,       /* bank list */
    },
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

Chronobox* gcb = 0;

/*-- Global variables ----------------------------------------------*/

static int gHaveRun        = 0;
static int gRunNumber      = 0;
//static int gIsPedestalsRun = 0;
static int gCountEvents    = 0;

static int    gMcsClockChan = 58;  // channel number where the clock is
static double gMcsClockFreq = 50000000.0; // clock frequency
//static double gMcsClockFreq = 10000000.0; // clock frequency
const double gClock_period = 1./gMcsClockFreq;
//static int    gMcsChans = 32; // number of scaler channels 
static const int    gMcsChans = 32+8+18+1; // number of scaler channels 

// static int    gMcsAdChan    = -1;  // channel of the AD pulse signal
// static char   gMcsAdTalk[256];

// static int    gMcsMixChan    = -1;  // channel of the Mixing gate pulse signal
// static char   gMcsMixTalk[256];

uint32_t gCounts[gMcsChans];
uint32_t gClock=0;


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

  printf("chronobox frontend_init done!\n");

  // reset the counter
  for(int j=0; j<gMcsChans; ++j) gCounts[j]=uint32_t(0);
  gClock=0;

  return SUCCESS;
}

/*-- Frontend Exit -------------------------------------------------*/

INT frontend_exit()
{
  // reset the counters
  gcb->cb_write32bis(0, 2, 0);
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

  printf("begin run: %d\n",run_number);

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
  
  // reset the counters
  gcb->cb_write32bis(0, 2, 0);
  gHaveRun = 0;
  printf("end run %d\n",run_number);
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
      //udelay(1);
      if (test)
        usleep(1000);
      else
        return TRUE;
    }
  return 0;
}

/*-- Event readout -------------------------------------------------*/

INT read_cb(char *pevent, INT off)
{
  if( gcb ) 
    {
      if(0) cm_msg(MINFO, frontend_name, "Chronobox Read");
    }
  else
    {
      cm_msg(MERROR, frontend_name, "Chronobox Read FAIELD");
      return 0;
    }

  uint32_t counts_curr,counts_diff;
  counts_curr=counts_diff=0;

  // // reset the counters
  // gcb->cb_write32bis(0, 2, 0);

  // latch the counters
  gcb->cb_write32bis(0, 1, 0);
  gcb->cb_read32(7);
  gcb->cb_read32(7);
  gcb->cb_read_scaler_begin();
  
  // read the clock
  uint32_t clock_curr=gcb->cb_read_scaler(gMcsClockChan);
  double time_diff = double(clock_curr-gClock)*gClock_period;
  if( !time_diff ) 
    {
      cm_msg(MERROR, frontend_name, "Time NOT moving forward");
      return 0;
    }
  
  /* init bank structure */
  bk_init32(pevent);
  double *p;
  bk_create(pevent, "CBMS", TID_DOUBLE, (void**)&p);

  for (int i=0; i<gMcsChans; i++)
    {
      // read the scaler
      counts_curr = gcb->cb_read_scaler(i);
      // compute the difference
      counts_diff = counts_curr - gCounts[i];

      // printf("ch: %d\tcnts: %d\tdelta: %1.6f s\trate: %1.3f Hz\n",
      // 	     i,counts_diff,time_diff,double(counts_diff)/time_diff);
      
      // TO DATA BANK
      p[i] = double(counts_diff)/time_diff;  // sample Rate in Hz 

      // save the variables
      gCounts[i]=counts_curr;
      gClock=clock_curr;
      // p[i] = double(gcb->cb_read_scaler(i));
      // printf("ch: %d\tcnt: %1.0f\t%d\n",i,p[i],gcb->cb_read_scaler(i));
    }

  bk_close(pevent, p+gMcsChans);
  ++gCountEvents;

  return bk_size(pevent);
}
