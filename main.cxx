/*
This program demonstrate how to use hps communicate with FPGA through light AXI Bridge.
uses should program the FPGA by GHRD project before executing the program
refer to user manual chapter 7 for details about the demo
*/


#include <stdio.h>
#include <stdlib.h>
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

uint32_t read32(void* virtual_base, int addr)
{
  void* laddr = virtual_base + ( ( unsigned long  )( ALT_LWFPGASLVS_OFST + addr ) & ( unsigned long)( HW_REGS_MASK ) );
  // write *(uint32_t *)h2p_lw_led_addr = ~led_mask; 

  return *(uint32_t *)laddr;
}

void write32(void* virtual_base, int addr, uint32_t data)
{
  void* laddr = virtual_base + ( ( unsigned long  )( ALT_LWFPGASLVS_OFST + addr ) & ( unsigned long)( HW_REGS_MASK ) );
  *(uint32_t *)laddr = data;
}

uint32_t cb_read32(void* virtual_base, int ireg)
{
  write32(virtual_base, 0x20+0, 0x40000000 + ireg);
  return read32(virtual_base, 0x00+0);
}

void cb_write32(void* virtual_base, int ireg, uint32_t data)
{
  write32(virtual_base, 0x10+0, data);
  write32(virtual_base, 0x20+0, 0x80000000 + ireg);
  write32(virtual_base, 0x20+0, 0);
}

void cb_write32bis(void* virtual_base, int ireg, uint32_t data1, uint32_t data2)
{
  write32(virtual_base, 0x10+0, data1);
  write32(virtual_base, 0x20+0, 0x80000000 + ireg);
  write32(virtual_base, 0x10+0, data2);
  write32(virtual_base, 0x20+0, 0);
}

void cb_read_scaler_begin(void* virtual_base)
{
  // write the address register
  write32(virtual_base, 0x20+0, 0x40000000 + 8);
}

uint32_t cb_read_scaler(void* virtual_base, int iscaler)
{
  //write32(virtual_base, 0x20+0, 0x80000000 + 8); // should not be needed
  // write data into reg8 sets scaler_addr_out
  write32(virtual_base, 0x10+0, iscaler);
  //write32(virtual_base, 0x20+0, 0x40000000 + 8); // should not be needed
  // read reg8_scaler_data_in
  return read32(virtual_base, 0x00+0);
}

int main() {

	void *virtual_base;
	int fd;
	int loop_count;
	int led_direction;
	int led_mask;
	void *h2p_lw_led_addr;

	setbuf(stdout, NULL);
	setbuf(stderr, NULL);

	// map the address space for the LED registers into user space so we can interact with them.
	// we'll actually map in the entire CSR span of the HPS since we want to access various registers within that span

	if( ( fd = open( "/dev/mem", ( O_RDWR | O_SYNC ) ) ) == -1 ) {
		printf( "ERROR: could not open \"/dev/mem\"...\n" );
		return( 1 );
	}

	virtual_base = mmap( NULL, HW_REGS_SPAN, ( PROT_READ | PROT_WRITE ), MAP_SHARED, fd, HW_REGS_BASE );

	if( virtual_base == MAP_FAILED ) {
		printf( "ERROR: mmap() failed...\n" );
		close( fd );
		return( 1 );
	}

	printf("SYSID_QSYS reg0: 0x%08x\n", read32(virtual_base, SYSID_QSYS_BASE));
	//printf("SYSID_QSYS reg1: 0x%08x\n", read32(virtual_base, SYSID_QSYS_BASE+1));
	printf("SYSID_QSYS reg1: 0x%08x\n", read32(virtual_base, SYSID_QSYS_BASE+4));

	if (0) {
	printf("read_data:  0x%08x\n", read32(virtual_base, 0x00+0));
	printf("write_data: 0x%08x\n", read32(virtual_base, 0x10+0));
	printf("addr:       0x%08x\n", read32(virtual_base, 0x20+0));

	write32(virtual_base, 0x10+0, 0x12345678);
	write32(virtual_base, 0x20+0, 0xbaadf00d);
	
	printf("read_data:  0x%08x\n", read32(virtual_base, 0x00+0));
	printf("write_data: 0x%08x\n", read32(virtual_base, 0x10+0));
	printf("addr:       0x%08x\n", read32(virtual_base, 0x20+0));

	write32(virtual_base, 0x10+0, 0xFFFFFFFF);
	write32(virtual_base, 0x20+0, 0x80000001);
	}
	
	printf("read_data:  0x%08x\n", read32(virtual_base, 0x00+0));
	printf("write_data: 0x%08x\n", read32(virtual_base, 0x10+0));
	printf("addr:       0x%08x\n", read32(virtual_base, 0x20+0));

	cb_write32(virtual_base, 1, 0xFFFFFFFF);
	sleep(1);
	cb_write32(virtual_base, 1, 0);
	cb_write32(virtual_base, 4, 0xfeedf00d);

	int i;
	for (i=0; i<10; i++) {
	  printf("cb reg %2d: 0x%08x\n", i, cb_read32(virtual_base, i));
	}

	if (1) {
	  uint32_t fwrev = cb_read32(virtual_base, 0);
	  printf("cb fw rev: 0x%08x\n", fwrev);
	  printf("fpga reconfigure!\n");
	  sleep(1);
	  cb_write32(virtual_base, 0xE, ~fwrev);
	  //printf("cb regE: 0x%08x\n", cb_read32(virtual_base, 0xE));
	  sleep(1);
	  printf("cb fw rev: 0x%08x\n", cb_read32(virtual_base, 0));
	  sleep(1);
	  printf("cb fw rev: 0x%08x\n", cb_read32(virtual_base, 0));
	  sleep(1);
	  printf("cb fw rev: 0x%08x\n", cb_read32(virtual_base, 0));
	  printf("done.\n");
	  exit(1);
	}

	// reset the counters
	cb_write32bis(virtual_base, 0, 2, 0);

	int j;
	for (j=0; j<5; j++) {
	  // latch the counters
	  cb_write32bis(virtual_base, 0, 1, 0);
	  i = 7;
	  printf("A cb reg %2d: 0x%08x\n", i, cb_read32(virtual_base, i));
	  printf("B cb reg %2d: 0x%08x\n", i, cb_read32(virtual_base, i));
	  cb_read_scaler_begin(virtual_base);
	  i = 0;
	  printf("C cb sc %3d: 0x%08x\n", i, cb_read_scaler(virtual_base, i));
	  i = 1;
	  printf("C cb sc %3d: 0x%08x\n", i, cb_read_scaler(virtual_base, i));
	  i = 15;
	  printf("C cb sc %3d: 0x%08x\n", i, cb_read_scaler(virtual_base, i));
	  i = 16;
	  printf("C cb sc %3d: 0x%08x\n", i, cb_read_scaler(virtual_base, i));
	  i = 17;
	  printf("C cb sc %3d: 0x%08x\n", i, cb_read_scaler(virtual_base, i));
	  sleep(1);
	}

	if (1) {
	  while (1) {
	    // latch the counters
	    cb_write32bis(virtual_base, 0, 1, 0);
	    int i = 7;
	    printf("A cb reg %2d: 0x%08x\n", i, cb_read32(virtual_base, i));
	    printf("B cb reg %2d: 0x%08x\n", i, cb_read32(virtual_base, i));
	    cb_read_scaler_begin(virtual_base);
	    for (i=0; i<32+8+18+1; i++) {
	      printf("C cb sc %3d: 0x%08x\n", i, cb_read_scaler(virtual_base, i));
	    }
	    sleep(1);
	  }
	}

	if (1) {
	  for (i=0; i<8; i++) {
	    printf("turning on LED%d\n", i+1);
	    cb_write32(virtual_base, 1, 1<<(i+8));
	    sleep(1);
	  }
	  
	  cb_write32(virtual_base, 1, 0);
	}

	if (1) {
	  printf("enable LEMO outputs:\n");

	  cb_write32(virtual_base, 0xB, 0xffffffff); // lemo out
	  cb_write32(virtual_base, 0xC, 0xffffffff); // gpio out
	  cb_write32(virtual_base, 0xD, 0xffffffff); // out enable

	  while (1) {
	    printf("on!\n");
	    cb_write32(virtual_base, 0xB, 0xffffffff); // lemo out
	    cb_write32(virtual_base, 0xC, 0xffffffff); // gpio out
	    sleep(2);
	    printf("off!\n");
	    cb_write32(virtual_base, 0xB, 0x00000000); // lemo out
	    cb_write32(virtual_base, 0xC, 0x00000000); // gpio out
	    sleep(2);
	  }
	}

	if (1) {
	  printf("print ECL, LEMO and GPIO inputs:\n");
	
	  cb_write32(virtual_base, 0xD, 0x000000000); // out enable

	  while (1) {
	    printf("ECL: 0x%08x, LEMO 0x%04x, GPIO: 0x%08x\n",
		   cb_read32(virtual_base, 0x6),
		   cb_read32(virtual_base, 0x9),
		   cb_read32(virtual_base, 0xA)
		   );
	    sleep(1);
	  }
	}
	
	h2p_lw_led_addr=virtual_base + ( ( unsigned long  )( ALT_LWFPGASLVS_OFST + LED_PIO_BASE ) & ( unsigned long)( HW_REGS_MASK ) );
	

	// toggle the LEDs a bit

	loop_count = 0;
	led_mask = 0x01;
	led_direction = 0; // 0: left to right direction
	while( loop_count < 60 ) {
		
		// control led
		*(uint32_t *)h2p_lw_led_addr = ~led_mask; 

		// wait 100ms
		usleep( 100*1000 );
		
		// update led mask
		if (led_direction == 0){
			led_mask <<= 1;
			if (led_mask == (0x01 << (LED_PIO_DATA_WIDTH-1)))
				 led_direction = 1;
		}else{
			led_mask >>= 1;
			if (led_mask == 0x01){ 
				led_direction = 0;
				loop_count++;
			}
		}
		
	} // while
	

	// clean up our memory mapping and exit
	
	if( munmap( virtual_base, HW_REGS_SPAN ) != 0 ) {
		printf( "ERROR: munmap() failed...\n" );
		close( fd );
		return( 1 );
	}

	close( fd );

	return( 0 );
}
