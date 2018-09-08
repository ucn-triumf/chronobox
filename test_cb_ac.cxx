//
// chronobox test
// August 2018
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cb.h"

int main(int argc, char* argv[])
{
   setbuf(stdout, NULL);
   setbuf(stderr, NULL);

   int zzz = 100000; // time in us between readouts
   if( argc == 2 ) zzz = atoi(argv[1]); 

   Chronobox* cb = new Chronobox();
   cb->cb_open();

   const int scalers_space = 32+8+18+1;
   const int counters_space = 32;

   int clock_channel=58;
   double clock_frequency = 50.e6;
   double clock_period = 1./clock_frequency;

   uint32_t clock,clock_curr;
   clock=clock_curr=0;
   double time_diff=0.;

   uint32_t counts[scalers_space];      
   for(int j=0; j<scalers_space; ++j) counts[j]=uint32_t(0);
   uint32_t counts_curr,counts_diff;
   counts_curr=counts_diff=0;

   // double rate=0.;
   //   FILE* nof = fopen("/dev/null", "w");

   int ich=0; // readout channel
   // reset the counters
   cb->cb_write32bis(0, 2, 0);

   while(1) 
      {
         // latch the counters
         cb->cb_write32bis(0, 1, 0);
         // ich = 7;
         // fprintf(nof,"A cb reg %2d: 0x%08x\n", ich, cb->cb_read32(ich));
         // fprintf(nof,"B cb reg %2d: 0x%08x\n", ich, cb->cb_read32(ich));
         cb->cb_read_scaler_begin();

         // read the time
         clock_curr=cb->cb_read_scaler(clock_channel);
         time_diff = double(clock_curr-clock)*clock_period;

         for(ich=0; ich<counters_space; ich++) 
            {
               counts_curr = cb->cb_read_scaler(ich);
               counts_diff = counts_curr - counts[ich];
               if( counts_diff )
                  {
                     // printf("C cb sc %3d: 0x%08x\t0x%08x\t%d\t%1.6fs\n",
                     //        ich, counts_curr, counts[ich], diff, time);
                     printf("ch: %d\tcnts: %d\tdelta: %1.6f s\trate: %1.3f Hz\n",
                            ich,counts_diff,time_diff,double(counts_diff)/time_diff);
                     // rate = double(counts_curr)/(double(clock_curr)*clock_period );
                     // printf("ch: %d\trate: %1.3f Hz\n",
                     //        ich,rate);
                  }
               counts[ich]=counts_curr;
               clock=clock_curr;
            }
         usleep(zzz);
      }
   //   fclose(nof);
   delete cb;
   return 0;
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
