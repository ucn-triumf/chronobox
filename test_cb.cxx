//
// reboot the chronobox fpga
//
// K.Olchanski TRIUMF August 2018
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cb.h"

void usage()
{
   printf("Usage:\n");
   printf("test_cb.exe 0 # read register 0\n");
   printf("test_cb.exe 4 0xabcd # write register 4 value 0xabcd\n");
   printf("test_cb.exe scalers # read and print all scalers\n");
   printf("test_cb.exe rates # read and print all the scales + rates\n");
   printf("test_cb.exe flows # read and print H2O flow rates\n");
   printf("test_cb.exe reboot # reboot the fpga\n");
   exit(1);
}

int main(int argc, char* argv[])
{
   setbuf(stdout, NULL);
   setbuf(stderr, NULL);
   Chronobox* cb = new Chronobox();
   cb->cb_open();

   for (int i=0; i<argc; i++) {
      printf("argv[%d]: %s\n", i, argv[i]);
   }

   if (argc <= 1) {
      usage(); // does not return
   }

   if (strcmp(argv[1], "reboot")==0) {
      cb->cb_reboot();
      exit(0);
   } else if (strcmp(argv[1], "mem")==0) {
      if (argc == 3) {
         uint32_t addr = strtoul(argv[2], NULL, 0);
         uint32_t v = cb->read32(addr);
         printf("addr 0x%08x: 0x%08x (%d)\n", addr, v, v);
         exit(0);
      } else if (argc == 4) {
         uint32_t addr = strtoul(argv[2], NULL, 0);
         uint32_t v = strtoul(argv[3], NULL, 0);
         cb->write32(addr, v);
         printf("addr 0x%08x write 0x%08x\n", addr, v);
         exit(0);
      }
   } else if (strcmp(argv[1], "fifo")==0) {
      uint32_t prev_ts = 0;
      int prev_ch = 0;
      int num_scalers = 0;
      int count_scalers = 0;
      while (1) {
         uint32_t fifo_status = cb->cb_read32(0x10);
         bool fifo_full = fifo_status & 0x80000000;
         bool fifo_empty = fifo_status & 0x40000000;
         int fifo_used = fifo_status & 0x00FFFFFF;

         printf("fifo status: 0x%08x, full %d, empty %d, used %d\n", fifo_status, fifo_full, fifo_empty, fifo_used);

         if (fifo_empty) {
            sleep(.5);
            if (1) {
               printf("latch scalers!\n");
               cb->cb_write32bis(0, 1, 0);
            }
            continue;
         }

         if (fifo_full && fifo_used == 0) {
            fifo_used = 0x10;
         }
         
         for (int i=0; i<fifo_used; i++) {
            cb->cb_write32(0, 4);
            cb->cb_write32(0, 0);
            uint32_t v = cb->cb_read32(0x11);
            printf("read %3d: 0x%08x", i, v);
            if ((v & 0xFF000000) == 0xFF000000) {
               printf(" overflow 0x%04x", v & 0xFFFF);
            } else if ((v & 0xFF000000) == 0xFE000000) {
               num_scalers = v & 0xFFFF;
               count_scalers = 0;
               printf(" packet of %d scalers", num_scalers);
            } else if (count_scalers < num_scalers) {
               printf(" scaler %d", count_scalers);
               count_scalers++;
            } else {
               uint32_t ts = v & 0x00FFFFFF;
               int ch = (v & 0x7F000000)>>24;
               if (ts == prev_ts) {
                  if (ch != prev_ch) {
                     printf(" coinc");
                  } else {
                     printf(" dupe");
                  }
               } else if (ts < prev_ts) {
                  printf(" wrap");
               }
               printf(" ts:%d %d %d",ts,v,ch);
               prev_ts = ts;
               prev_ch = ch;
            }
            printf("\n");
         }
      }
      exit(0); 
   } else if (strcmp(argv[1], "scalers")==0) {
      int num = 1+cb->cb_read_input_num();
      while (1) {
         // latch the counters
         cb->cb_write32bis(0, 1, 0);
         int i = 7;
         printf("A cb reg %2d: 0x%08x\n", i, cb->cb_read32(i));
         printf("B cb reg %2d: 0x%08x\n", i, cb->cb_read32(i));
         cb->cb_read_scaler_begin();
         for (i=0; i<num; i++) {
            printf("C cb sc %3d: 0x%08x\n", i, cb->cb_read_scaler(i));
         }
         sleep(1);
      }
      exit(0);
   } else if (strcmp(argv[1], "rates")==0) {
      int num = 1+cb->cb_read_input_num();
      int clk100 = num-1;
      uint32_t prevscalers[num];
      for (int i=0; i<num; i++) {
         prevscalers[i] = 0;
      }
      while (1) {
         // latch the counters
         cb->cb_write32bis(0, 1, 0);
         int i = 7;
         printf("A cb reg %2d: 0x%08x\n", i, cb->cb_read32(i));
         printf("B cb reg %2d: 0x%08x\n", i, cb->cb_read32(i));
         cb->cb_read_scaler_begin();
         uint32_t vclk = cb->cb_read_scaler(clk100);
         double delta_time = (double) vclk - (double)prevscalers[clk100];
         delta_time = delta_time / 1e8;
         for (i=0; i<num; i++) {
            uint32_t v = cb->cb_read_scaler(i);
            printf("dT:%f Idx:%3d Cnt: 0x%08x Diff:%d  Rate:%f\n"
                   , delta_time, i, v, v - prevscalers[i]
                   , (float) (v - prevscalers[i]) / delta_time);
            prevscalers[i] = v; 
         }
         prevscalers[clk100] = vclk; 
         sleep(1);
      }
   } else if (strcmp(argv[1], "flows")==0) {
      uint32_t prevscalers[58];
      while (1) {
         // latch the counters
         cb->cb_write32bis(0, 1, 0);
         int i = 7;
         printf("A cb reg %2d: 0x%08x\n", i, cb->cb_read32(i));
         printf("B cb reg %2d: 0x%08x\n", i, cb->cb_read32(i));
         cb->cb_read_scaler_begin();
         float delta_time = (float) cb->cb_read_scaler(58) - (float)prevscalers[58];
         delta_time = delta_time / 1e8;
         for (i=40; i<40+8; i++) {
            printf("dT:%f Idx:%3d Diff:%d  Rate:%f\n", delta_time, i, cb->cb_read_scaler(i) - prevscalers[i]
                   , (float) (cb->cb_read_scaler(i) - prevscalers[i]) / delta_time);
            prevscalers[i] = cb->cb_read_scaler(i); 
         }
         prevscalers[58] = cb->cb_read_scaler(58); 
         sleep(1);
      }
      exit(0);
   } else if (argc == 2) {
      int ireg = strtoul(argv[1], NULL, 0);
      uint32_t v = cb->cb_read32(ireg);
      printf("cb register 0x%x: 0x%08x (%d)\n", ireg, v, v);
      exit(0);
   } else if (argc == 3) {
      int ireg = strtoul(argv[1], NULL, 0);
      uint32_t v = strtoul(argv[2], NULL, 0);
      cb->cb_write32(ireg, v);
      printf("cb register 0x%x write 0x%08x\n", ireg, v);
      exit(0);
   }

   //printf("SYSID_QSYS reg0: 0x%08x\n", cb->read32(SYSID_QSYS_BASE));
   //printf("SYSID_QSYS reg1: 0x%08x\n", cb->read32(SYSID_QSYS_BASE+4));

   if (0) {
      printf("read_data:  0x%08x\n", cb->read32(0x00+0));
      printf("write_data: 0x%08x\n", cb->read32(0x10+0));
      printf("addr:       0x%08x\n", cb->read32(0x20+0));
      
      cb->write32(0x10+0, 0x12345678);
      cb->write32(0x20+0, 0xbaadf00d);
      
      printf("read_data:  0x%08x\n", cb->read32(0x00+0));
      printf("write_data: 0x%08x\n", cb->read32(0x10+0));
      printf("addr:       0x%08x\n", cb->read32(0x20+0));
      
      cb->write32(0x10+0, 0xFFFFFFFF);
      cb->write32(0x20+0, 0x80000001);
   }
   
   printf("read_data:  0x%08x\n", cb->read32(0x00+0));
   printf("write_data: 0x%08x\n", cb->read32(0x10+0));
   printf("addr:       0x%08x\n", cb->read32(0x20+0));
   
   cb->cb_write32(1, 0xFFFFFFFF);
   sleep(1);
   cb->cb_write32(1, 0);
   cb->cb_write32(4, 0xfeedf00d);
   
   int i;
   for (i=0; i<10; i++) {
      printf("cb reg %2d: 0x%08x\n", i, cb->cb_read32(i));
   }
   
   if (1) {
      uint32_t fwrev = cb->cb_read32(0);
      printf("cb fw rev: 0x%08x\n", fwrev);
      printf("fpga reconfigure!\n");
      sleep(1);
      cb->cb_write32(0xE, ~fwrev);
      //printf("cb regE: 0x%08x\n", cb->cb_read32(0xE));
      sleep(1);
      printf("cb fw rev: 0x%08x\n", cb->cb_read32(0));
      sleep(1);
      printf("cb fw rev: 0x%08x\n", cb->cb_read32(0));
      sleep(1);
      printf("cb fw rev: 0x%08x\n", cb->cb_read32(0));
      printf("done.\n");
      exit(1);
   }
   
   // reset the counters
   cb->cb_write32bis(0, 2, 0);
   
   int j;
   for (j=0; j<5; j++) {
      // latch the counters
      cb->cb_write32bis(0, 1, 0);
      i = 7;
      printf("A cb reg %2d: 0x%08x\n", i, cb->cb_read32(i));
      printf("B cb reg %2d: 0x%08x\n", i, cb->cb_read32(i));
      cb->cb_read_scaler_begin();
      i = 0;
      printf("C cb sc %3d: 0x%08x\n", i, cb->cb_read_scaler(i));
      i = 1;
      printf("C cb sc %3d: 0x%08x\n", i, cb->cb_read_scaler(i));
      i = 15;
      printf("C cb sc %3d: 0x%08x\n", i, cb->cb_read_scaler(i));
      i = 16;
      printf("C cb sc %3d: 0x%08x\n", i, cb->cb_read_scaler(i));
      i = 17;
      printf("C cb sc %3d: 0x%08x\n", i, cb->cb_read_scaler(i));
      sleep(1);
   }
   
   if (1) {
      while (1) {
         // latch the counters
         cb->cb_write32bis(0, 1, 0);
         int i = 7;
         printf("A cb reg %2d: 0x%08x\n", i, cb->cb_read32(i));
         printf("B cb reg %2d: 0x%08x\n", i, cb->cb_read32(i));
         cb->cb_read_scaler_begin();
         for (i=0; i<32+8+18+1; i++) {
            printf("C cb sc %3d: 0x%08x\n", i, cb->cb_read_scaler(i));
         }
         sleep(1);
      }
   }
   
   if (1) {
      for (i=0; i<8; i++) {
         printf("turning on LED%d\n", i+1);
         cb->cb_write32(1, 1<<(i+8));
         sleep(1);
      }
      
      cb->cb_write32(1, 0);
   }
   
   if (1) {
      printf("enable LEMO outputs:\n");
      
      cb->cb_write32(0xB, 0xffffffff); // lemo out
      cb->cb_write32(0xC, 0xffffffff); // gpio out
      cb->cb_write32(0xD, 0xffffffff); // out enable
      
      while (1) {
         printf("on!\n");
         cb->cb_write32(0xB, 0xffffffff); // lemo out
         cb->cb_write32(0xC, 0xffffffff); // gpio out
         sleep(2);
         printf("off!\n");
         cb->cb_write32(0xB, 0x00000000); // lemo out
         cb->cb_write32(0xC, 0x00000000); // gpio out
         sleep(2);
      }
   }
   
   if (1) {
      printf("print ECL, LEMO and GPIO inputs:\n");
      
      cb->cb_write32(0xD, 0x000000000); // out enable
      
      while (1) {
         printf("ECL: 0x%08x, LEMO 0x%04x, GPIO: 0x%08x\n",
                cb->cb_read32(0x6),
                cb->cb_read32(0x9),
                cb->cb_read32(0xA)
                );
         sleep(1);
      }
   }
   
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
