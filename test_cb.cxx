//
// reboot the chronobox fpga
//
// K.Olchanski TRIUMF August 2018
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cb.h"

void usage()
{
   printf("Usage:\n");
   printf("test_cb.exe 0 # read register 0\n");
   printf("test_cb.exe 4 0xabcd # write register 4 value 0xabcd\n");
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
