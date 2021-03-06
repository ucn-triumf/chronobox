// cb.cxx --- chronobox driver K.Olchanski TRIUMF Aug 2018

#include "cb.h"

#include <stdio.h>

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "hwlib.h"
#include "socal/socal.h"
#include "socal/hps.h"
#include "socal/alt_gpio.h"
#include "hps_0.h"

#define HW_REGS_BASE ( ALT_STM_OFST )
#define HW_REGS_SPAN ( 0x04000000 )
#define HW_REGS_MASK ( HW_REGS_SPAN - 1 )

Chronobox::Chronobox() // ctor
{

}

Chronobox::~Chronobox() // dtor
{
   fBase = NULL;
}

int Chronobox::cb_open()
{
   char *virtual_base;
   int fd;
   
   // map the address space for the LED registers into user space so we can interact with them.
   // we'll actually map in the entire CSR span of the HPS since we want to access various registers within that span
   
   if( ( fd = open( "/dev/mem", ( O_RDWR | O_SYNC ) ) ) == -1 ) {
      printf( "ERROR: could not open \"/dev/mem\"...\n" );
      return( 1 );
   }
   
   virtual_base = (char*)mmap( NULL, HW_REGS_SPAN, ( PROT_READ | PROT_WRITE ), MAP_SHARED, fd, HW_REGS_BASE );
   
   if( virtual_base == MAP_FAILED ) {
      printf( "ERROR: mmap() failed...\n" );
      close( fd );
      return( 1 );
   }

   // file descriptor not needed after mmap()
   close(fd);
   
   fBase = virtual_base;
   
   printf("\n");
   printf("Chronobox SYSID_QSYS reg0: 0x%08x\n", read32(SYSID_QSYS_BASE));
   printf("Chronobox SYSID_QSYS reg1: 0x%08x\n", read32(SYSID_QSYS_BASE+4));
   printf("\n");
   printf("Chronobox read_data:  0x%08x\n", read32(0x00+0));
   printf("Chronobox write_data: 0x%08x\n", read32(0x10+0));
   printf("Chronobox addr:       0x%08x\n", read32(0x20+0));
   printf("\n");
   printf("Chronobox firmware revision: 0x%08x\n", cb_read32(0));
   printf("\n");

   return 0;
}

uint32_t Chronobox::read32(int addr)
{
   void* laddr = fBase + ( ( unsigned long  )( ALT_LWFPGASLVS_OFST + addr ) & ( unsigned long)( HW_REGS_MASK ) );
  // write *(uint32_t *)h2p_lw_led_addr = ~led_mask; 

   return *(uint32_t *)laddr;
}

void Chronobox::write32(int addr, uint32_t data)
{
   void* laddr = fBase + ( ( unsigned long  )( ALT_LWFPGASLVS_OFST + addr ) & ( unsigned long)( HW_REGS_MASK ) );
   *(uint32_t *)laddr = data;
}

uint32_t Chronobox::cb_read32(int ireg)
{
   write32(0x20+0, 0x40000000 + ireg);
   return read32(0x00+0);
}

void Chronobox::cb_write32(int ireg, uint32_t data)
{
   write32(0x10+0, data);
   write32(0x20+0, 0x80000000 + ireg);
   write32(0x20+0, 0);
}

void Chronobox::cb_write32bis(int ireg, uint32_t data1, uint32_t data2)
{
   write32(0x10+0, data1);
   write32(0x20+0, 0x80000000 + ireg);
   write32(0x10+0, data2);
   write32(0x20+0, 0);
}

void Chronobox::cb_reboot()
{
   uint32_t fwrev = cb_read32(0);
   printf("cb fw rev: 0x%08x\n", fwrev);
   printf("fpga reconfigure!\n");
   sleep(1);
   cb_write32(0xE, ~fwrev);
   //printf("cb regE: 0x%08x\n", cb_read32(0xE));
   sleep(1);
   printf("cb fw rev: 0x%08x\n", cb_read32(0));
   sleep(1);
   printf("cb fw rev: 0x%08x\n", cb_read32(0));
   sleep(1);
   printf("cb fw rev: 0x%08x\n", cb_read32(0));
   printf("done.\n");
}

int Chronobox::cb_read_input_num()
{
   uint32_t rev = cb_read32(0);
   if (rev >= 0x5b89e4b4) {
      return cb_read32(0xF);
   } else {
      return 58;
   }
}

void Chronobox::cb_read_scaler_begin()
{
   // write the address register
   write32(0x20+0, 0x40000000 + 8);
}

uint32_t Chronobox::cb_read_scaler(int iscaler)
{
   //write32(virtual_base, 0x20+0, 0x80000000 + 8); // should not be needed
   // write data into reg8 sets scaler_addr_out
   write32(0x10+0, iscaler);
   //write32(virtual_base, 0x20+0, 0x40000000 + 8); // should not be needed
   // read reg8_scaler_data_in
   return read32(0x00+0);
}

void Chronobox::cb_read_fifo(std::vector<uint32_t> *data)
{
   uint32_t fifo_status = cb_read32(0x10);
   bool fifo_full = fifo_status & 0x80000000;
   bool fifo_empty = fifo_status & 0x40000000;
   int fifo_used = fifo_status & 0x00FFFFFF;
   
   //printf("fifo status: 0x%08x, full %d, empty %d, used %d\n", fifo_status, fifo_full, fifo_empty, fifo_used);

   if (fifo_empty) {
      return;
   }
   
   if (fifo_full && fifo_used == 0) {
      fifo_used = 0x10;
   }
   
   for (int i=0; i<fifo_used; i++) {
      cb_write32(0, 4);
      cb_write32(0, 0);
      uint32_t v = cb_read32(0x11);
      //printf("read %3d: 0x%08x\n", i, v);
      data->push_back(v);
   }
}

void Chronobox::cb_latch_scalers()
{
   cb_write32bis(0, 1, 0);
}

void Chronobox::cb_reset_scalers()
{
   cb_write32bis(0, 2, 0);
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */

