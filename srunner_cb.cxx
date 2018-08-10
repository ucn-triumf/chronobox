/****************************************************************************/
/*																			*/
/*	Module:			main.c	(SRunner)										*/	
/*																			*/
/*					Copyright (C) Altera Corporation 2004					*/
/*																			*/
/*	Descriptions:	Main source file that manages SRunner-User interface	*/
/*					to execute program or read routine in SRunner			*/
/*																			*/
/*	Revisions:		1.0	09/30/04 Khai Liang Aw								*/
/*					1.1	06/05/05 Khai Liang Aw - EPCS64 Support				*/
/*					1.2	06/11/08 Noor Hazlina Ramly - EPCS128 Support		*/
/*																			*/
/*																			*/
/*																			*/
/****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

//#include "user.h"

#define HANDLE int
#define USHORT unsigned short
#define ULONG  unsigned long
#define BOOL bool

//////////////////////////////// user.h ///////////////////////////////////
/* Error Code Start */
#define	CB_OK								0

#define CB_FS_OPEN_FILE_ERROR				-1
#define CB_FS_CLOSE_FILE_ERROR				-2
#define CB_FS_SIZE_EOF_NOT_FOUND			-3
#define CB_FS_READ_ERROR					-4

#define CB_BB_OPEN_ERROR_OPEN_PORT			-5
#define CB_BB_OPEN_VERSION_INCOMPATIBLE		-6
#define CB_BB_OPEN_DRIVER_INCOMPATIBLE		-7
#define CB_BB_OPEN_DEVICEIOCONTROL_FAIL		-8
#define CB_BB_CLOSE_BYTEBLASTER_NOT_OPEN	-9
#define CB_BB_FLUSH_ERROR					-10
#define CB_BB_VERIFY_BYTEBLASTER_NOT_FOUND	-11
#define CB_BB_LPTREAD_ERROR					-12
#define CB_BB_LPTWRITE_ERROR				-13

#define CB_PS_CONF_NSTATUS_LOW				-14
#define CB_PS_CONF_CONFDONE_LOW				-15
#define CB_PS_INIT_NSTATUS_LOW				-16
#define CB_PS_INIT_CONFDONE_LOW				-17

#define CB_AS_VERIFY_FAIL					-18		
#define CB_AS_UNSUPPORTED_DEVICE			-19 
#define CB_AS_WRONG_RPD_FILE				-20

#define CB_INVALID_NUMBER_OF_ARGUMENTS		-21
#define CB_INVALID_COMMAND					-22
#define CB_INVALID_EPCS_DENSITY				-23

/* Error Code END  */

//#include "as.h"

////////////////////// as.h ///////////////////////////////

/*////////////////////*/
/* Global Definitions */
/*////////////////////*/

#define CHECK_EVERY_X_BYTE	10240
#define INIT_CYCLE			200



/*///////////////////////*/
/* AS Instruction Set    */
/*///////////////////////*/
#define AS_WRITE_ENABLE				0x06
#define AS_WRITE_DISABLE			0x04
#define AS_READ_STATUS	    		0x05
#define AS_WRITE_STATUS	    		0x01
#define AS_READ_BYTES   			0x03
#define AS_FAST_READ_BYTES  		0x0B
#define AS_PAGE_PROGRAM				0x02
#define AS_ERASE_SECTOR				0xD8
#define AS_ERASE_BULK				0xC7
#define AS_READ_SILICON_ID			0xAB
#define AS_CHECK_SILICON_ID			0x9F


/*///////////////////////*/
/* Silicon ID for EPCS   */
/*///////////////////////*/
#define EPCS1_ID	0x10
#define EPCS4_ID	0x12
#define EPCS16_ID	0x14
#define EPCS64_ID	0x16
#define EPCS128_ID	0x18


/*///////////////////////*/
/* EPCS device			 */
/*///////////////////////*/
#define EPCS1		1	
#define EPCS4		4	
#define EPCS16		16	
#define EPCS64		64
#define EPCS128		128

#define DEV_READBACK   0xFF //Special bypass indicator during EPCS data readback	


/*///////////////////////*/
/* Functions Prototyping */
/*///////////////////////*/

int as_id(int epcsDensity);
int as_program( char*, FILE*);
int as_read( char*, FILE* );
int as_ver( char *, FILE* );
int as_open( char*, FILE**, long int* );
int as_close( FILE* );
int as_program_start( void );
int as_program_done(void);
int as_bulk_erase( void );
int as_prog( FILE*, int );
int as_silicon_id(int, int);
int as_program_byte_lsb( int );
int as_read_byte_lsb( int* );
int as_program_byte_msb( int );
int as_read_byte_msb( int* );
int as_readback(FILE*);
int as_verify( FILE*, int);
void as_lsb_to_msb( int *, int *);

///////////////////////// user.h ////////////////////////////////

/* Error Code Start */
#define	CB_OK								0

#define CB_FS_OPEN_FILE_ERROR				-1
#define CB_FS_CLOSE_FILE_ERROR				-2
#define CB_FS_SIZE_EOF_NOT_FOUND			-3
#define CB_FS_READ_ERROR					-4

#define CB_BB_OPEN_ERROR_OPEN_PORT			-5
#define CB_BB_OPEN_VERSION_INCOMPATIBLE		-6
#define CB_BB_OPEN_DRIVER_INCOMPATIBLE		-7
#define CB_BB_OPEN_DEVICEIOCONTROL_FAIL		-8
#define CB_BB_CLOSE_BYTEBLASTER_NOT_OPEN	-9
#define CB_BB_FLUSH_ERROR					-10
#define CB_BB_VERIFY_BYTEBLASTER_NOT_FOUND	-11
#define CB_BB_LPTREAD_ERROR					-12
#define CB_BB_LPTWRITE_ERROR				-13

#define CB_PS_CONF_NSTATUS_LOW				-14
#define CB_PS_CONF_CONFDONE_LOW				-15
#define CB_PS_INIT_NSTATUS_LOW				-16
#define CB_PS_INIT_CONFDONE_LOW				-17

#define CB_AS_VERIFY_FAIL					-18		
#define CB_AS_UNSUPPORTED_DEVICE			-19 
#define CB_AS_WRONG_RPD_FILE				-20

#define CB_INVALID_NUMBER_OF_ARGUMENTS		-21
#define CB_INVALID_COMMAND					-22
#define CB_INVALID_EPCS_DENSITY				-23

/* Error Code END  */

///////////////////////// bb.h ////////////////////////////////

/****************************************************************************/
/*																			*/
/*	Module:			mb_io.h	(MicroBlaster)									*/	
/*																			*/
/*					Copyright (C) Altera Corporation 2001					*/
/*																			*/
/*	Descriptions:	Defines all IO control functions. operating system		*/
/*					is defined here. Functions are operating system 		*/
/*					dependant.												*/
/*																			*/
/*	Revisions:		1.0	12/10/01 Sang Beng Ng								*/
/*					Supports Altera ByteBlaster hardware download cable		*/
/*					on Windows NT.											*/
/*																			*/
/****************************************************************************/


//#include <windows.h>


/*////////////////////*/
/* Global Definitions */
/*////////////////////*/

#define LPT_DATA	0
#define LPT_STATUS	1
#define LPT_CONTROL	2

#define DCLK		0x01
#define NCONFIG		0x02
#define DATA0		0x40
#define CONF_DONE	0x80
#define NSTATUS		0x10
#define NCS			0x04
#define NCE			0x08
#define ASDI		0x40
#define DATAOUT		0x10
#define TCK			0x01
#define TMS			0x02
#define TDI			0x40
#define TDO			0x80

#define BBNONE		0
#define BBMV		1
#define BBII		2

/* Port Mode for ByteBlaster II Cable */
#define BBII_CONFIG_MODE	1 /* Reset */
#define BBII_USER_MODE		0 /* User */

/* Port Mode for ByteBlasterMV Cable */
#define BBMV_CONFIG_MODE	0 /* Reset */
#define BBMV_USER_MODE		1 /* User */


/*///////////////////////*/
/* Functions Prototyping */
/*///////////////////////*/

int bb_open		( void );
int bb_close	( void );
int bb_flush	( void );
int bb_verify	( int* );
int bb_lptread	( int, int* );
int bb_lptwrite	( int, int, int );
int bb_read		( int, int* );
int bb_write	( int, int );
int bb_reset	( int );




///////////////////////// bb.c ////////////////////////////////

/****************************************************************************/
/*																			*/
/*	Module:			mb_io.c	(MicroBlaster)									*/	
/*																			*/
/*					Copyright (C) Altera Corporation 2001					*/
/*																			*/
/*	Descriptions:	Defines all IO control functions. operating system		*/
/*					is defined here. Functions are operating system 		*/
/*					dependant.												*/
/*																			*/
/*	Revisions:		1.0	12/10/01 Sang Beng Ng								*/
/*					Supports Altera ByteBlaster hardware download cable		*/
/*					on Windows NT.											*/
/*																			*/
/****************************************************************************/

#include <stdio.h>
//#include "user.h"
//#include "bb.h"


/*////////////////////*/
/* Global Definitions */
/*////////////////////*/
#define	PGDC_IOCTL_GET_DEVICE_INFO_PP	0x00166A00L
#define PGDC_IOCTL_READ_PORT_PP			0x00166A04L
#define PGDC_IOCTL_WRITE_PORT_PP		0x0016AA08L
#define PGDC_IOCTL_PROCESS_LIST_PP		0x0016AA1CL
#define PGDC_WRITE_PORT					0x0a82
#define PGDC_HDLC_NTDRIVER_VERSION		2
#define PORT_IO_BUFFER_SIZE				256


/*//////////////////*/
/* Global Variables */
/*//////////////////*/
//HANDLE	nt_device_handle		= INVALID_HANDLE_VALUE;
//int		port_io_buffer_count	= 0;




 //struct PORT_IO_LIST_STRUCT
 //{
 //	USHORT command;
 //	USHORT data;
 //} port_io_buffer[PORT_IO_BUFFER_SIZE];

int bb_type = 0;

/* port_data holds the current values of signals for every port. By default, they hold the values in */
/*   reset mode (PM_RESET_<ByteBlaster used>). */
/*   port_data[Z], where Z - port number, holds the value of the port. */
int	cur_data = 0x42;/* Initial value for Port 0, 1 and 2 */

#include <stdint.h>

static int debug = 0;

#include "cb.h"

Chronobox* gCb = NULL;

void gpmc_open()
{
   gCb = new Chronobox();
   gCb->cb_open();
}

void gpmc_write(uint16_t v)
{
   //printf("write 0x%02x\n", v);
   gCb->cb_write32(5, v);
   //sleep(2);
}

uint16_t gpmc_read()
{
   return gCb->cb_read32(5);
}

void gpmc_close()
{
   gpmc_write(0);
   delete gCb;
   gCb = NULL;
}

/********************************************************************************/
/*	Name:			InitNtDriver  												*/
/*																				*/
/*	Parameters:		None.          												*/
/*																				*/
/*	Return Value:	None.														*/
/*																				*/
/*	Descriptions:	Initiallize Windows NT Driver for ByteBlasterMV.			*/
/*																				*/
/********************************************************************************/
int bb_open( void )
{
   printf("bb_open!\n");

   setbuf(stdout, NULL);
   setbuf(stderr, NULL);

   int init_ok = 1;	/* Initialization OK */
   int status = CB_OK;


	if ( !init_ok )
	{
		fprintf( stderr, "Error: Driver initialization fail!\n" );
		return status;
	}
	else
	{
		status = bb_verify( &bb_type );
		if ( status != CB_OK )
		{
			return status;
		}
		else
		{
			if ( bb_type == 1 )
				status = bb_reset( BBMV_CONFIG_MODE );
			else if ( bb_type == 2)
				status = bb_reset( BBII_CONFIG_MODE );
			
			return status;
		}
	}
}


/********************************************************************************/
/*	Name:			CloseNtDriver 												*/
/*																				*/
/*	Parameters:		None.          												*/
/*																				*/
/*	Return Value:	None.														*/
/*																				*/
/*	Descriptions:	Close Windows NT Driver.									*/
/*																				*/
/********************************************************************************/
int bb_close( void )
{
   printf("bb_close!\n");

   if ( bb_type == BBNONE )
      {
         fprintf(stderr, "ByteBlaster not opened!");
         return CB_BB_CLOSE_BYTEBLASTER_NOT_OPEN;
      }
   else
      {
         int status = CB_OK;

         if ( bb_type == BBMV )
            status = bb_reset( BBMV_USER_MODE );
         else if ( bb_type == BBII)
            status = bb_reset( BBII_USER_MODE );
         
         if ( status == CB_OK )
            bb_type = BBNONE;
      }

   return CB_OK;
}


/********************************************************************************/
/*	Name:			flush_ports 												*/
/*																				*/
/*	Parameters:		None.          												*/
/*																				*/
/*	Return Value:	None.														*/
/*																				*/
/*	Descriptions:	Flush processes in [port_io_buffer]	and reset buffer		*/
/*					size to 0.													*/
/*																				*/
/********************************************************************************/
int bb_flush( void )
{
   if (debug)
      printf("bb_flush!\n");
   return CB_OK;
}


/******************************************************************/
/* Name:         VerifyBBII (ByteBlaster II)					  */
/*                                                                */
/* Parameters:   None.                                            */
/*                                                                */
/* Return Value: '0' if verification is successful;'1' if not.    */
/*               		                                          */
/* Descriptions: Verify if ByteBlaster II is properly attached to */
/*               the parallel port.                               */
/*                                                                */
/******************************************************************/
int bb_verify( int *types )
{
   //int v0 = gpmc_read();
   gpmc_write(0xAB00);
   int v1 = gpmc_read();

   if ((v1 & 0xFF) != 0xCD)
      {
         fprintf( stderr, "Error: Verifying hardware: ByteBlaster not found, GPMC read 0x%08x should be 0xXXXXXXcd!\n", v1);
         return CB_BB_VERIFY_BYTEBLASTER_NOT_FOUND;
      }

   fprintf( stdout, "Info: Found GPMC 0xABCD FlashProgrammer interface.\n" );
   *types = BBMV;
   return CB_OK;

	int status = 0;
	
	int type = 0;
	int test_count = 0;
	int read_data = 0;
	int error = 0;
	int i = 0;
	
	for ( type = 0; type < 2; type++ )
	{
		int vector = (type) ? 0x10 : 0xA0;
		int expect = (type) ? 0x40 : 0x60;
		int vtemp;
		
		for ( test_count = 0; test_count < 2; test_count++ )
		{
			/* Write '0' to Pin 6 (Data4) for the first test and '1' for the second test */
			vtemp = (test_count) ? (vector & 0xff) : 0x00;/* 0001 0000:0000 0000... drive to Port0 */

			status = bb_lptwrite( LPT_DATA, vtemp, 1 );
			if ( status != CB_OK )
				return status;

			//delay
			for (i=0;i<1500;i++);

			/* Expect '0' at Pin 10 (Ack) and Pin 15 (Error) for the first test */
			/* and '1' at Pin 10 (Ack) and '0' Pin 15 (Error) for the second test */
			status = bb_lptread( LPT_STATUS, &read_data );
			if ( status != CB_OK )
				return status;
			
			read_data = read_data & (expect & 0xff);

			/* If no ByteBlaster II detected, error = 1 */
			if (test_count==0)
			{
				if(read_data==0x00)
					error=0;
				else error=1;
			}

			if (test_count==1)
			{
				if(read_data == (expect & 0xff))
					error=error|0;
				else error=1;
			}
		}

		if ( !error )
			break;
	}

	if (!type)
	{
		fprintf( stdout, "Info: Verifying hardware: ByteBlasterMV found.\n" );
		*types = BBMV;
		return CB_OK;
	}
	else
	{
		if (!error)
		{
			fprintf( stdout, "Info: Verifying hardware: ByteBlaster II found.\n" );
			*types = BBII;
			return CB_OK;
		}
		else
		{
			fprintf( stderr, "Error: Verifying hardware: ByteBlaster not found or not installed properly!\n" );
			return CB_BB_VERIFY_BYTEBLASTER_NOT_FOUND;
		}
	}
}


/********************************************************************************/
/*	Name:			ReadByteBlaster												*/
/*																				*/
/*	Parameters:		int port       												*/
/*					- port number 0, 1, or 2. Index to parallel port base		*/
/*					  address.													*/
/*																				*/
/*	Return Value:	Integer, value of the port.									*/
/*																				*/
/*	Descriptions:	Read the value of the port registers.						*/
/*																				*/
/********************************************************************************/
int bb_lptread( int port, int *data )
{
   if (debug)
      printf("bb_lptread  port %d\n", port);

   assert(port==1);

   int temp = gpmc_read();

   //assert((temp & 0xFF) == 0xCD);

   *data = (temp>>8) & 0xff;

   if (debug)
      printf("bb_lptread  port %d, data 0x%02x, GPMC read 0x%08x\n", port, *data, temp);

   return CB_OK;
}


/********************************************************************************/
/*	Name:			WriteByteBlaster											*/
/*																				*/
/*	Parameters:		int port, int data, int test								*/
/*					- port number 0, 1, or 2. Index to parallel port base		*/
/*					  address.													*/
/*					- value to written to port registers.						*/
/*					- purpose of write.											*/ 
/*																				*/
/*	Return Value:	None                       									*/
/*																				*/
/*	Descriptions:	Write [data] to [port] registers. When dump to Port0, if	*/
/*					[test] = '0', processes in [port_io_buffer] are dumped		*/
/*					when [PORT_IO_BUFFER_SIZE] is reached. If [test] = '1',		*/
/*					[data] is dumped immediately to Port0.						*/
/*																				*/
/********************************************************************************/
int bb_lptwrite( int port, int data, int nbuffering )
{
   uint16_t w = 0xAB00 | (data & 0xFF);

   if (debug)
      printf("bb_lptwrite port %d, data 0x%02x, nbuffering %d, GPMC write 0x%08x\n", port, data, nbuffering, w);

   if (port==2)
      return CB_OK;

   assert(port == 0);
   
   gpmc_write(w);

   return CB_OK;
}


/********************************************************************************/
/*	Name:			CheckSignal													*/
/*																				*/
/*	Parameters:		int signal						 							*/
/*					- name of the signal (SIG_*).								*/
/*																				*/
/*	Return Value:	Integer, the value of the signal. '0' is returned if the	*/
/*					value of the signal is LOW, if not, the signal is HIGH.		*/
/*																				*/
/*	Descriptions:	Return the value of the signal.								*/
/*																				*/
/********************************************************************************/
int bb_read( int signal, int *data )
{
	int temp = 0;
	int	status = 0;

	status = bb_lptread( LPT_STATUS, &temp );
	if ( status == CB_OK )
		*data = (temp ^ 0x80) & signal;
	return status;
}

/********************************************************************************/
/*	Name:			Dump2Port													*/
/*																				*/
/*	Parameters:		int signal, int data, int clk	 							*/
/*					- name of the signal (SIG_*).								*/
/*					- value to be dumped to the signal.							*/
/*					- assert a LOW to HIGH transition to SIG_DCLK togther with	*/
/*					  [signal].													*/
/*																				*/
/*	Return Value:	None.														*/
/*																				*/
/*	Descriptions:	Dump [data] to [signal]. If [clk] is '1', a clock pulse is	*/
/*					generated after the [data] is dumped to [signal].			*/
/*																				*/
/********************************************************************************/
int bb_write( int signal, int data )
{
	int status = 0;

	/* AND signal bit with '0', then OR with [data] */
	int mask = ~signal;

	cur_data = ( cur_data & mask ) | ( data * signal );
	status = bb_lptwrite( LPT_DATA, cur_data, 0 );
	return status;
}

/********************************************************************************/
/*	Name:			SetPortMode													*/
/*																				*/
/*	Parameters:		int mode  						 							*/
/*					- The mode of the port (PM_*)								*/
/*																				*/
/*	Return Value:	None.														*/
/*																				*/
/*	Descriptions:	Set the parallel port registers to particular values.		*/
/*																				*/
/********************************************************************************/
int bb_reset( int mode )
{
	int status = 0;

	/* write to Port 0 and Port 2 with predefined values */
	int control = mode ? 0x0C : 0x0E; 
	cur_data = 0x42;	
	

	status = bb_lptwrite( LPT_DATA, cur_data, 1 );
	if ( status == CB_OK )
		status = bb_lptwrite( LPT_CONTROL, control, 1 );
	return status;
}


///////////////////////// fs.h ////////////////////////////////

/*////////////////////*/
/* Global Definitions */
/*////////////////////*/

#define S_CUR				1 /* SEEK_CUR */
#define S_END				2 /* SEEK_END */
#define S_SET				0 /* SEEK_SET */


/*///////////////////////*/
/* Functions Prototyping */
/*///////////////////////*/

int fs_open( char[], const char*, int* );
int	fs_close( FILE* );
int fs_size( FILE*, long int* );
int fs_read( FILE*, int* );

int fs_open_log(void);			//Srunner 4827
void fs_write(FILE*, int);		//Srunner 4827
void fs_rewind( FILE* file_id);	//Srunner 4827


///////////////////////// fs.c ////////////////////////////////

#include <stdio.h>
//#include "user.h"
//#include "fs.h"


/********************************************************************************/
/*	Name:			fs_open  													*/
/*																				*/
/*	Descriptions:	Open programming file										*/
/********************************************************************************/
int fs_open( char argv[], const char* mode, FILE** file_id )
{
	FILE* fid;
	fid = fopen( argv, mode );
	if ( fid == NULL )
	{
		fprintf( stderr, "Error: Could not open file: \"%s\"!\n", argv );
		return CB_FS_OPEN_FILE_ERROR;
	}
	else
	{
		*file_id = fid;
		fprintf( stdout, "Info: Programming file: \"%s\" opened.\n", argv );
		return CB_OK;
	}
}



/********************************************************************************/
/*	Name:			fs_close  													*/
/*																				*/
/*	Descriptions:	Close file													*/
/********************************************************************************/
int	fs_close( FILE* file_id )
{
	int status = 0;
	status = fclose( file_id);
	if ( status )
	{
		fprintf( stderr, "Error: Could not close file!\n");
		return CB_FS_CLOSE_FILE_ERROR;
	}
	else
	{
		return CB_OK;
	}
}



/********************************************************************************/
/*	Name:			fs_size		  												*/
/*																				*/
/*	Descriptions:	check file size												*/
/********************************************************************************/
int fs_size( FILE* file_id, long int *size )
{
	int status = 0;
	status = fseek( file_id, 0, S_END );
	if ( status )
	{
		fprintf( stderr, "Error: End of file could not be located!" );
		return CB_FS_SIZE_EOF_NOT_FOUND;
	}
	else
	{
		*size = ftell( file_id );
		fseek( file_id, 0, S_SET );
		fprintf( stdout, "Info: File size: %ld bytes.\n", *size );
		return CB_OK;
	}
}



/********************************************************************************/
/*	Name:			fs_read		  												*/
/*																				*/
/*	Descriptions:	read a byte from file										*/
/********************************************************************************/
int fs_read( FILE* file_id, int *data )
{
	int status = 0;
	status = fgetc( file_id );
	if ( status == EOF )
	{
		fprintf( stderr, "Error: Could not read data from file!" );
		return CB_FS_READ_ERROR;
	}
	else
	{
		*data = status;
		return CB_OK;
	}
}



/********************************************************************************/
/*	Name:			fs_write	  												*/
/*																				*/
/*	Descriptions:	write a byte to file										*/
/********************************************************************************/
void fs_write(FILE* file_id, int data)  
{
	fputc(data, file_id );
		
}



/********************************************************************************/
/*	Name:			fs_rewind	  												*/
/*																				*/
/*	Descriptions:	Repositions the file pointer to the beginning of a file		*/
/********************************************************************************/
void fs_rewind( FILE* file_id)  
{
	rewind( file_id );	
}



///////////////////////// as.c ////////////////////////////////


#include <stdio.h>
//#include "user.h"
//#include "as.h"
//#include "fs.h"
//#include "bb.h"

int EPCS_device = 0;
int RPD_file_size = 0;


/********************************************************************************/
/*	Name:			as_program  												*/
/*																				*/
/*	Parameters:		FILE* finputid												*/
/*					- programming file pointer.									*/
/*																				*/
/*	Return Value:	Error Code													*/
/*																				*/
/*	Descriptions:	Get programming file size, parse through every single byte	*/
/*					and dump to parallel port.									*/
/*																				*/
/*					FPGA access to the EPCS is disable when the programming 	*/
/*					starts.														*/
/*																				*/
/*																				*/
/********************************************************************************/
int as_program( char *file_path, int epcsDensity )
{
	int status = 0;
	FILE* file_id = 0;
	long int file_size = 0;


	/* Open RPD file for programming */
	status = as_open( file_path, &file_id, &file_size );
	if ( status != CB_OK )
		return status;

	
	/* Disable FPGA access to EPCS */
	status = as_program_start();
	if ( status != CB_OK )
		return status;


	/* Read EPCS silicon ID */
	status = as_silicon_id(file_size, epcsDensity);
	if ( status != CB_OK )
		return status;
	


	/* EPCS Bulk Erase */
	status = as_bulk_erase( );
	if ( status != CB_OK )
		return status;

	
	/* Start EPCS Programming */
	status = as_prog( file_id, file_size );
	if ( status != CB_OK )
		return status;
	

	/* Start EPCS Verifying */
	//status = as_verify( file_id, file_size );
	//if ( status != CB_OK )
	//	return status;
	


	/* Enable FPGA access to EPCS */
	status = as_program_done();
	if ( status != CB_OK )
		return status;	
	
	
	status = as_close( file_id );
	if ( status != CB_OK )
		return status;



	return CB_OK;
	
}

int as_id(int epcsDensity)
{
	int status = 0;
	long int file_size = 0;

	status = bb_open();
	if ( status != CB_OK )
		return status;


        /* Read EPCS silicon ID */
	status = as_silicon_id(file_size, epcsDensity);
	if ( status != CB_OK )
		return status;

	return CB_OK;
	
}

/********************************************************************************/
/*	Name:			as_ver  													*/
/*																				*/
/*	Parameters:		FILE* finputid												*/
/*					- programming file pointer.									*/
/*																				*/
/*	Return Value:	Error Code													*/
/*																				*/
/*	Descriptions:	Verify EPCS data											*/
/*																				*/		
/*																				*/
/*					FPGA access to the EPCS is disable when the programming 	*/
/*					starts.														*/
/*																				*/
/*																				*/
/********************************************************************************/
int as_ver( char *file_path, int epcsDensity)
{
	int status = 0;
	FILE* file_id = 0;
	long int file_size = 0;

	

	/* Open RPD file for verify */
	status = as_open( file_path, &file_id, &file_size );
	if ( status != CB_OK )
		return status;

	
	/* Disable FPGA access to EPCS */
	status = as_program_start();
	if ( status != CB_OK )
		return status;


	/* Read EPCS silicon ID */	
	status = as_silicon_id(file_size, epcsDensity);
	if ( status != CB_OK )
		return status;
	

	/* Start EPCS Verifying */
	status = as_verify( file_id, file_size );
	if ( status != CB_OK )
		return status;
	


	/* Enable FPGA access to EPCS */
	status = as_program_done();
	if ( status != CB_OK )
		return status;	
	
	
	status = as_close( file_id );
	if ( status != CB_OK )
		return status;



	return CB_OK;
	
}

/********************************************************************************/
/*	Name:			as_read     												*/
/*																				*/
/*	Parameters:		FILE* finputid												*/
/*					- programming file pointer.									*/
/*																				*/
/*	Return Value:	Error Code													*/
/*																				*/
/*	Descriptions:	Get EPCS data and save in a file							*/
/*																				*/
/*																				*/
/*					FPGA access to the EPCS is disable when the reading     	*/
/*					starts.														*/
/*																				*/
/*																				*/
/********************************************************************************/
int as_read( char *file_path, int epcsDensity )
{
	int status = 0;
	FILE* file_id = 0;
	//long int file_size = 0;

	
	status = bb_open();
	if ( status != CB_OK )
		return status;


	/* Open RPD file for to store EPCS data */
	status = fs_open( file_path, "w+b", &file_id );
	if ( status != CB_OK )
		return status;


	/* Disable FPGA access to EPCS */
	status = as_program_start();
	if ( status != CB_OK )
		return status;


	/* Read EPCS silicon ID */
	status = as_silicon_id(DEV_READBACK, epcsDensity);
	if ( status != CB_OK )
		return status;
	


	/* Start EPCS Readback */
	status = as_readback( file_id);
	if ( status != CB_OK )
		return status;
	

	
	/* Enable FPGA access to EPCS */
	status = as_program_done();
	if ( status != CB_OK )
		return status;	
	
	
	status = as_close( file_id );
	if ( status != CB_OK )
		return status;

	return CB_OK;
	
}


int as_program_start(void)
{
	int status = 0;
	
	// Drive NCONFIG to reset FPGA before programming EPCS
	status = bb_write( NCONFIG, 0 );
	if ( status != CB_OK )
		return status;

	// Drive NCE to disable FPGA from accessing EPCS
	status = bb_write( NCE, 1 );
	if ( status != CB_OK )
		return status;
	
	// Drive NCS to high when not acessing EPCS
	status = bb_write( NCS, 1 );
	if ( status != CB_OK )
		return status;

	status = bb_flush();
	if ( status != CB_OK )
		return status;	

	return CB_OK;
}



int as_program_done(void)
{
	int		status;

	// Drive NCE to enable FPGA
	status = bb_write( NCE, 0 );
	if ( status != CB_OK )
		return status;
	
	// Drive NCONFIG from low to high to reset FPGA
	status = bb_write( NCONFIG, 1 );
	if ( status != CB_OK )
		return status;

	// Drive NCS to high when not acessing EPCS
	status = bb_write( NCS, 1 );
	if ( status != CB_OK )
		return status;

	status = bb_flush();
	if ( status != CB_OK )
		return status;

	 	
	return CB_OK;
}


int as_open( char *file_path, FILE* *file_id, long int *file_size )
{
	int status = 0;

	status = fs_open( file_path, "rb", file_id );
	if ( status != CB_OK )
		return status;

	status = fs_size( *file_id, file_size );
	if ( status != CB_OK )
		return status;

        if (strstr(file_path, ".pof") != 0)
           {
              // skip POF file header
              int c;
              int i = 0;
              for (; ; i++)
                 {
                    fs_read(*file_id, &c);
                    if ((c & 0xFF) == 0xFF)
                       break;
                 }

              // unread the 0xFF byte
              fseek((FILE*)*file_id, -1, SEEK_CUR);
              i--;

              printf("Skipping POF file header: start of data at %d, file size %ld, remain %ld, modulo 1024 = %ld\n", i, *file_size, *file_size - i, (*file_size - i)%1024);

              *file_size -= i;

              *file_size -= (*file_size) % 1024;

              printf("Final file size %ld\n", *file_size);

              //exit(1);
           }

        if (strstr(file_path, ".jic") != 0)
           {
              // skip JIC file header
              int c;
              int i = 0;
              for (; ; i++)
                 {
                    fs_read(*file_id, &c);
                    if ((c & 0xFF) == 0xFF)
                       break;
                 }

              // unread the 0xFF byte
              fseek((FILE*)*file_id, -1, SEEK_CUR);
              i--;

              printf("Skipping JIC file header: start of data at %d, file size %ld, remain %ld, modulo 1024 = %ld\n", i, *file_size, *file_size - i, (*file_size - i)%1024);

              *file_size -= i;

              *file_size -= (*file_size) % 1024;

              printf("Final file size %ld\n", *file_size);

              //exit(1);
           }

	status = bb_open();
	if ( status != CB_OK )
		return status;

	return CB_OK;
}


int as_close( FILE* file_id )
{
	int	status = 0;

	status = bb_close();
	if ( status != CB_OK )
		return status;

	status = fs_close( file_id );
	if ( status != CB_OK )
		return status;

	return CB_OK;
}





/********************************************************************************/
/*	Name:			as_prog														*/
/*																				*/
/*	Parameters:		int file_size					 							*/
/*					- file size to check for the correct programming file.		*/
/*					int file_id					 								*/
/*					- to refer to the RPD file.									*/
/*																				*/
/*	Return Value:	status.														*/
/*																				*/
/*	Descriptions:	program the data in the EPCS								*/
/*																				*/
/********************************************************************************/
int as_prog( FILE* file_id, int file_size )
{
	int			page = 0;
	int         one_byte = 0;
	int         EPCS_Address =0;
	int         StatusReg =0;
	int			i,j;		
	int			status = 0;
	int         bal_byte = 0;
	int         byte_per_page = 256;

	fprintf( stdout, "\nInfo: Start programming process.\n" );
	page = file_size/256;
	
	bal_byte = file_size%256;
	
	if(bal_byte) //if there is balance after divide, program the balance in the next page
	{
		page++;
	}


	//=========== Page Program command Start=========//
	status = bb_write( NCS, 0 );
	if ( status != CB_OK )
		return status;
	
	status = as_program_byte_msb( AS_WRITE_ENABLE );
	if ( status != CB_OK )
		return status;

	status = bb_write( NCS, 1 );
	if ( status != CB_OK )
		return status;	
	
	status = bb_flush();
	if ( status != CB_OK )
		return status;


	// page program
	fprintf( stdout, "\nInfo: Programming %d pages...\n", page);	

	for(i=0; i<page; i++ )
	{
           if ((i%100)==0)
               printf("page %d (%.1f%%)...\r", i, i*100.0/page);

        again:

		status = bb_write( NCS, 0 );
		if ( status != CB_OK )
			return status;
	
		status = as_program_byte_msb( AS_WRITE_ENABLE );
		if ( status != CB_OK )
			return status;

		status = bb_write( NCS, 1 );
		if ( status != CB_OK )
			return status;	
	
		status = bb_flush();
		if ( status != CB_OK )
			return status;
		
		
		status = bb_write( NCS, 0 );
		if ( status != CB_OK )
			return status;
	
		status = as_program_byte_msb( AS_PAGE_PROGRAM );
		if ( status != CB_OK )
			return status;		
		
		EPCS_Address = i*256;
		
		status = as_program_byte_msb( ((EPCS_Address & 0xFF0000)>>16));
		status = as_program_byte_msb( ((EPCS_Address & 0x00FF00)>>8) );
		status = as_program_byte_msb( EPCS_Address & 0xFF);

		status = bb_flush();
		if ( status != CB_OK )
			return status;

		if((i == (page - 1)) && (bal_byte != 0))	//if the last page has has been truncated less than 256
			byte_per_page = bal_byte;
		
		for(j=0; j<byte_per_page; j++)
		{
			// read one byte
			status = fs_read( file_id, &one_byte );
			if ( status != CB_OK )
				return status;

                        //printf("byte %d, value 0x%02x\n", j, one_byte);
	
			// Progaram a byte 
			status = as_program_byte_lsb( one_byte );
			if ( status != CB_OK )
				return status;
		}
		
		status = bb_write( NCS, 1 );
		if ( status != CB_OK )
			return status;	

		status = bb_flush();
		if ( status != CB_OK )
			return status;

		//Program in proress
		status = bb_write( NCS, 0 );
		if ( status != CB_OK )
			return status;		
		
		status = as_program_byte_msb( AS_READ_STATUS );
		if ( status != CB_OK )
			return status;

		status = bb_flush();	
		if ( status != CB_OK )	
			return status;		
				
		
		status = as_read_byte_msb(&StatusReg);
		if ( status != CB_OK )
			return status;	
	
	
		while((StatusReg & 0x01))
		{
			status = as_read_byte_msb(&StatusReg);
                        //printf("status reg 0x%02x\n", StatusReg);
                        if (StatusReg == 0xFF) {
                           printf("After progamming page %d, unexpected status returned by EPCS AS_READ_STATUS: 0x%02x\n", i, StatusReg);

                           if (0)
                              {
                                 status = bb_write( NCS, 1 );
                                 if ( status != CB_OK )
                                    return status;
                                 
                                 status = bb_flush();
                                 if ( status != CB_OK )
                                    return status;
                                 
                                 i++;
                                 goto again;
                              }

                           return CB_BB_LPTREAD_ERROR;
                        }

			if ( status != CB_OK )
				return status;
		}
		
		
		status = bb_write( NCS, 1 );
		if ( status != CB_OK )
			return status;

		status = bb_flush();
		if ( status != CB_OK )
			return status;
		//Program End

	}

	fprintf( stdout, "Info: Programming successful\n" );

	//=========== Page Program command End==========//


	return CB_OK;
}





/********************************************************************************/
/*	Name:			as_bulk_erase												*/
/*																				*/
/*	Parameters:		int file_size					 							*/
/*					- file size to check for the correct programming file.		*/
/*					int file_id					 								*/
/*					- to refer to the RPD file.									*/
/*																				*/
/*	Return Value:	status.														*/
/*																				*/
/*	Descriptions:	program the data in the EPCS								*/
/*																				*/
/********************************************************************************/
int as_bulk_erase( void )
{
	int status =0;
	int StatusReg =0;

	//=========== Bulk erase command Start ===========//
	status = bb_write( NCS, 0 );
	if ( status != CB_OK )
		return status;
	
	status = as_program_byte_msb( AS_WRITE_ENABLE );
	if ( status != CB_OK )
		return status;

	status = bb_write( NCS, 1 );
	if ( status != CB_OK )
		return status;

	status = bb_flush();
	if ( status != CB_OK )
		return status;
	
	
	status = bb_write( NCS, 0 );
	if ( status != CB_OK )
		return status;
	
	status = as_program_byte_msb( AS_ERASE_BULK );
	if ( status != CB_OK )
		return status;

	status = bb_write( NCS, 1 );
	if ( status != CB_OK )
		return status;

	status = bb_flush();
	if ( status != CB_OK )
		return status;


	status = bb_write( NCS, 0 );
	if ( status != CB_OK )
		return status;

	status = as_program_byte_msb( AS_READ_STATUS );
	if ( status != CB_OK )
		return status;
	
	status = bb_flush();
	if ( status != CB_OK )
		return status;

	//Erase in proress
	fprintf( stdout, "Info: Erasing...\n" );	
	
	status = as_read_byte_msb(&StatusReg);
	if ( status != CB_OK )
		return status;	

	
	while((StatusReg & 0x01))	//Keep on polling if the WIP is high
	{
           printf(".");
		status = as_read_byte_msb(&StatusReg);
		if ( status != CB_OK )
			return status;
                sleep(1);
	}

	fprintf( stdout, "Info: Erase Done" );	
	//Erase End

	status = bb_write( NCS, 1 );
	if ( status != CB_OK )
		return status;

	status = bb_flush();
	if ( status != CB_OK )
		return status;

	//=========== Bulk erase command End ============//
	return CB_OK;

}


/********************************************************************************/
/*	Name:			as_readback													*/
/*																				*/
/*	Parameters:		none							 							*/
/*																				*/
/*																				*/
/*																				*/
/*																				*/
/*	Return Value:	status.														*/
/*																				*/
/*	Descriptions:	read the content of the EPCS devices and store in RPD file	*/
/*																				*/
/********************************************************************************/
int as_readback( FILE* file_id )
{
	//=========== Readback Program command Start=========//
	int		status;
	int     i;
	int     read_byte;
	
		
        fprintf( stdout, "Info: Reading %d bytes...\n", RPD_file_size);
		
		status = bb_write( NCS, 0 );
		if ( status != CB_OK )
			return status;
	
		status = as_program_byte_msb( AS_READ_BYTES );
		if ( status != CB_OK )
			return status;		
	
		
		status = as_program_byte_msb(0x00);
		status = as_program_byte_msb(0x00);
		status = as_program_byte_msb(0x00);


		status = bb_flush();	
		if ( status != CB_OK )	
			return status;		

		for(i=0; i<RPD_file_size; i++)
		{
                   if ((i % 0x10000) == 0)
                      printf(" %d (%.1f%%)\r", i, i*100.0/RPD_file_size);

			status = as_read_byte_lsb(&read_byte);
			if ( status != CB_OK )
				return status;

			fs_write(file_id, read_byte); 

		}

		status = bb_write( NCS, 0 );
		if ( status != CB_OK )
			return status;

		status = bb_flush();	
		if ( status != CB_OK )
			return status;
		
		fprintf( stdout, "Info: Read successful\n" );
	//=========== Readback Program command End==========//

	return CB_OK;
}



/********************************************************************************/
/*	Name:			as_verify													*/
/*																				*/
/*	Parameters:		int file_size					 							*/
/*					- file size to check for the correct programming file.		*/
/*					int file_id					 								*/
/*					- to refer to the RPD file.									*/
/*																				*/
/*	Return Value:	None.														*/
/*																				*/
/*	Descriptions:	verify the all the programmed data matach the data in the	*/
/*					data in RPD file.											*/
/*																				*/
/********************************************************************************/
int as_verify( FILE* file_id, int file_size )
{
	//=========== Readback Program command Start=========//
	int		status;
	int     i;
	int     read_byte =0;
	int     one_byte = 0;
	
	
		
		fprintf( stdout, "Info: Verifying...\n" );

		//fs_rewind(file_id);	//reposition the file pointer of the RPD file

		
		status = bb_write( NCS, 0 );
		if ( status != CB_OK )
			return status;
	
		status = as_program_byte_msb( AS_READ_BYTES );
		if ( status != CB_OK )
			return status;		
	
		status = as_program_byte_msb(0x00);
		status = as_program_byte_msb(0x00);
		status = as_program_byte_msb(0x00);

		status = bb_flush();	
		if ( status != CB_OK )	
			return status;		

                int errcount = 0;
		for(i=0; i<file_size; i++)
		{
                   if ((i % 0x10000) == 0)
                      printf(" %d (%.1f%%)\r", i, i*100.0/RPD_file_size);

			// read one byte from the EPCS
			status = as_read_byte_lsb(&read_byte);
			if ( status != CB_OK )
				return status;

		
			// read one byte from RPD file
			status = fs_read( file_id, &one_byte );
			if ( status != CB_OK )
				return status;

			if(one_byte != read_byte)
			{
                          errcount++;
                          printf("file position %3d, mismatch: EPCS has %3d (0x%02x), file has %3d (0x%02x)\n", i, read_byte, read_byte, one_byte, one_byte);
                          if (errcount>10) {
                             status = CB_AS_VERIFY_FAIL;
                             return status;
                          }
			}

		}

		fprintf( stdout, "Info: Verify completed\n" );

		status = bb_write( NCS, 1 );
		if ( status != CB_OK )
			return status;

		status = bb_flush();	
		if ( status != CB_OK )
			return status;
		
	//=========== Readback Program command End==========//

	return CB_OK;
}



/********************************************************************************/
/*	Name:			as_silicon_id												*/
/*																				*/
/*	Parameters:		int file_size					 							*/
/*					- file size to check for the correct programming file.		*/
/*																				*/
/*	Return Value:	None.														*/
/*																				*/
/*	Descriptions:	check silicon id to determine:								*/
/*					EPCS devices.												*/
/*					RPD file size.												*/
/*																				*/
/********************************************************************************/
int as_silicon_id(int file_size, int epcsDensity)
{
	//=========== Read silicon id command Start=========//
	int	status;
	int silicon_ID = 0;

	printf("epcsDensity: %d\n", epcsDensity);
				
		status = bb_write( NCS, 0 );
		if ( status != CB_OK )
			return status;
		
		if (epcsDensity != 128)		//for EPCS1, EPCS4, EPCS16, EPCS64
		{
		  printf("Command AS_READ_SILICON_ID!\n");

			status = as_program_byte_msb( AS_READ_SILICON_ID );
			if ( status != CB_OK )
				return status;		
		
			status = as_program_byte_msb(0x00);		//3 Dummy bytes
			status = as_program_byte_msb(0x00);
			status = as_program_byte_msb(0x00);
		}
		else						// for EPCS128
		{
		  printf("Command AS_CHECK_SILICON_ID!\n");

			status = as_program_byte_msb( AS_CHECK_SILICON_ID );
			if ( status != CB_OK )
				return status;	

			status = as_program_byte_msb(0x00);		//2 Dummy bytes
			status = as_program_byte_msb(0x00);
		}
		
		status = bb_flush();	
		if ( status != CB_OK )	
			return status;		

		// read silicon byte from the EPCS
		status = as_read_byte_msb(&silicon_ID);
		if ( status != CB_OK )
			return status;

		// determine the required RPD file size and EPCS devices
		if(silicon_ID == EPCS1_ID)
		{
			EPCS_device = EPCS1;
		}
		else if(silicon_ID == EPCS4_ID)
		{
			EPCS_device = EPCS4;
		}
		else if(silicon_ID == EPCS16_ID)		
		{
			EPCS_device = EPCS16;
		}
		else if(silicon_ID == EPCS64_ID)		
		{
			EPCS_device = EPCS64;
		}
		else if(silicon_ID == EPCS128_ID)		
		{
			EPCS_device = EPCS128;
		}
		else
		{
                   fprintf( stdout, "\nError: Unsupported Device 0x%x\n", silicon_ID);
			status = CB_AS_UNSUPPORTED_DEVICE;
			return status;
		}

		fprintf( stdout, "\nInfo: Silicon ID - 0x%x \n", silicon_ID);
		fprintf( stdout, "Info: Serial Configuration Device - EPCS%d\n", EPCS_device);
			
		RPD_file_size = EPCS_device * 131072;	//To calculate the maximum file size for the EPCS
		
		if(file_size > RPD_file_size && file_size != DEV_READBACK)
		{
			fprintf( stdout, "\nError: Wrong programming file");
			return CB_AS_WRONG_RPD_FILE;
		}

		status = bb_write( NCS, 1 );
		if ( status != CB_OK )
			return status;

		status = bb_flush();	
		if ( status != CB_OK )	
			return status;
		
	//=========== Readback Program command End==========//

	return CB_OK;
}



/********************************************************************************/
/*	Name:			as_program_byte_lsb											*/
/*																				*/
/*	Parameters:		int one_byte					 							*/
/*					- The byte to dump.											*/
/*																				*/
/*	Return Value:	None.														*/
/*																				*/
/*	Descriptions:	Dump to parallel port bit by bit, from least significant	*/
/*					bit to most significant bit. A positive edge clock pulse	*/
/*					is also asserted.											*/
/*																				*/
/********************************************************************************/
int as_program_byte_lsb( int one_byte )
{
	int	bit = 0;
	int i = 0;
	int status = 0;

	//printf("as_program_byte_lsb: 0x%02x\n", one_byte);
	
	// write from LSB to MSB 
	for ( i = 0; i < 8; i++ )
	{
		bit = one_byte >> i;
		bit = bit & 0x1;
		
		// Dump to DATA0 and insert a positive edge pulse at the same time 
		status = bb_write( DCLK, 0 );
		if ( status != CB_OK )
			return status;
		//status = bb_write( DATA0, bit );
		status = bb_write( ASDI, bit );
		if ( status != CB_OK )
			return status;
		status = bb_write( DCLK, 1 );
		if ( status != CB_OK )
			return status;

		//sleep(2);
	}


	return CB_OK;
}



/********************************************************************************/
/*	Name:			as_program_byte_msb											*/
/*																				*/
/*	Parameters:		int one_byte					 							*/
/*					- The byte to dump.											*/
/*																				*/
/*	Return Value:	None.														*/
/*																				*/
/*	Descriptions:	Convert MSB to LSB and Dump to parallel port bit by bit,	*/
/*					from most significant bit to least significant bit.			*/
/*					A positive edge clock pulse	is also asserted.				*/
/*																				*/
/********************************************************************************/
int as_program_byte_msb( int one_byte )
{
	int status = 0;
	int data_byte = 0;

	//printf("as_program_byte_msb: 0x%02x\n", one_byte);

	//Convert MSB to LSB before programming
	as_lsb_to_msb(&one_byte, &data_byte);
	
	//After conversion, MSB will goes out first
	status = as_program_byte_lsb(data_byte); 
	
	return CB_OK;
}


/********************************************************************************/
/*	Name:			as_read_byte_lsb											*/
/*																				*/
/*	Parameters:		int one_byte					 							*/
/*					- The byte to read.											*/
/*																				*/
/*	Return Value:	None.														*/
/*																				*/
/*	Descriptions:	read to parallel port bit by bit, from least significant    */
/*					bit to most significant bit. A positive edge clock pulse	*/
/*					is also asserted. (read during positive edge)				*/
/*																				*/
/********************************************************************************/
int as_read_byte_lsb( int *one_byte )
{
	int	bit = 0;
	int mask = 0x01;
	int i;
	int status = 0;

	*one_byte = 0;
	
	
	// Flush out the remaining data in Port0 before reading
	status = bb_flush();
	if ( status != CB_OK )
		return status;	
	
	// read from from LSB to MSB 
	for ( i = 0; i < 8; i++ )
	{
		// Dump to DATA0 and insert a positive edge pulse at the same time 
		status = bb_write( DCLK, 0 );
		if ( status != CB_OK )
			return status;

		status = bb_write( DCLK, 1 );
		if ( status != CB_OK )
			return status;

		// Flush the positive clk before reading
		status = bb_flush();
		if ( status != CB_OK )
			return status;

		status = bb_read( DATAOUT, &bit );
		if ( status != CB_OK )
			return status;

		if (bit!=0) //if bit is true
			*one_byte |= (mask << i); 
	}
		
	return CB_OK;	

}



/********************************************************************************/
/*	Name:			as_read_byte_msb											*/
/*																				*/
/*	Parameters:		int one_byte					 							*/
/*					- The byte to read.											*/
/*																				*/
/*	Return Value:	None.														*/
/*																				*/
/*	Descriptions:	read from parallel port bit by bit, from most significant	*/
/*					bit to least significant bit. A positive edge clock pulse	*/
/*					is also asserted. (read during positive edge)				*/
/*																				*/
/********************************************************************************/
int as_read_byte_msb( int *one_byte )
{
	int status = 0;
	int data_byte = 0;

	status = as_read_byte_lsb(&data_byte);
	if ( status != CB_OK )
			return status;


	//After conversion, MSB will come in first
	as_lsb_to_msb(&data_byte, one_byte);
	
	return CB_OK;	

}


/********************************************************************************/
/*	Name:			as_lsb_to_msb												*/
/*																				*/
/*	Parameters:		int *in_byte					 							*/
/*					- The byte to convert.										*/
/*					int *out_byte												*/
/*					- The converted byte										*/
/*																				*/
/*	Return Value:	None.														*/
/*																				*/
/*	Descriptions:	Convert LSB to MSB											*/
/*																				*/
/*																				*/
/*																				*/
/********************************************************************************/
void as_lsb_to_msb( int *in_byte, int *out_byte)
{
	int		mask;
	int		i;
	int     temp;

	*out_byte = 0x00;
	

	for ( i = 0; i < 8; i++ )
	{	
		temp = *in_byte >> i;
		mask = 0x80 >> i;
		
		if(temp & 0x01)	//if lsb is set inbyte, set msb for outbyte
		{
			*out_byte |= mask;
		}

	}
}

///////////////////////// main.c //////////////////////////////

/* Version Number */
const char VERSION[] = "1.2-de10-nano-chronobox-0xABCD-KO4 (POF, JIC support)";


/********************************************************************************/
/*	Name:			Main          												*/
/*																				*/
/*	Parameters:		int argc, char* argv[]										*/
/*					- number of argument.										*/
/*					- argument character pointer.								*/ 
/*																				*/
/*	Return Value:	None.														*/
/*																				*/
/*	Descriptions:	Open programming file and initialize driver if required		*/
/*					(WINDOWS NT).												*/
/*																				*/
/********************************************************************************/

void help()
{
   fprintf( stdout, " \n Command\t\t\t\t\t\tDescription\n");	
   fprintf( stdout, " ============\t\t\t\t\t\t===============\n");	
   fprintf( stdout, " srunner -program -<EPCS density in Mb> <file.rpd>\t=> Program EPCS\n");	
   fprintf( stdout, " srunner -read -<EPCS density in Mb> <file.rpd>\t\t=> Read EPCS data to file\n");
   fprintf( stdout, " srunner -verify -<EPCS density in Mb> <file.rpd>\t=> Verify EPCS data with file\n");
   fprintf( stdout, " srunner -id -<EPCS density in Mb> <file.rpd>\t=> Identify EPCS device\n");
   fprintf( stdout, "\nExample of command -> srunner -program -64 Mydesign.rpd\n");
   fprintf( stdout, "\nExample of command -> srunner -id -64 /dev/null\n");
}

int main( int argc, char** argv)
{
	int status = 0;
	int epcsDensity = 0;
	
	/* Introduction */
	fprintf( stdout, "\n================================================================\n" );
	fprintf( stdout, "SRunner Version %s", VERSION );
	fprintf( stdout, "\nAltera Corporation " );
	fprintf( stdout, "\nConfiguration Devices: EPCS1, EPCS4, EPCS16, EPCS64 and EPCS128" );
	fprintf( stdout, "\n================================================================\n" );

	if(argc != 4)	//Syntax check
	{
		fprintf( stdout, " \nError: Invalid Number of Arguments\n");	
                help();
		status = CB_INVALID_NUMBER_OF_ARGUMENTS;
	}
	else if ( argv[1][1] != 'p' && argv[1][1] != 'r' && argv[1][1] != 'v' && argv[1][1] != 'i')
	{
		fprintf( stdout, " \nError: Invalid Command\n");	
                help();
		status = CB_INVALID_COMMAND;
		
	}
	else if	((strcmp(&argv[2][1],"1")!=0) && (strcmp(&argv[2][1],"4")!=0) && (strcmp(&argv[2][1],"16")!=0) && (strcmp(&argv[2][1],"64")!=0) && (strcmp(&argv[2][1],"128")!=0))
	{
			fprintf( stdout, "\nError: Invalid EPCS density\n");	
			fprintf( stdout, "\nValid choices are 1, 4, 16, 64, or 128\n");
			fprintf( stdout, "\nExample of command -> srunner -program -64 Mydesign.rpd <-\n");
						
			status = CB_INVALID_EPCS_DENSITY;
	}
	else
	{
		epcsDensity = atoi(&argv[2][1]);

		printf("epcsDensity: %d\n", epcsDensity);
   
                gpmc_open();

		if	( argv[1][1] == 'p')
		{ 
		fprintf( stdout, "\nOperation: Programming EPCS\n");
		status = as_program( &argv[3][0], epcsDensity );	//Execute programming function
			
		}
		else if ( argv[1][1] == 'r')	
		{
		fprintf( stdout, "\nOperation: Reading EPCS Data\n");
		status = as_read( &argv[3][0], epcsDensity );		//Execute reading function
		}
		else if ( argv[1][1] == 'v')	
		{
		fprintf( stdout, "\nOperation: Verifying EPCS Data\n");
		status = as_ver( &argv[3][0], epcsDensity );		//Execute verify function
		}
		else if ( argv[1][1] == 'i')	
		{
		fprintf( stdout, "\nOperation: Identify EPCS Device\n");
		status = as_id(epcsDensity);		//Execute identify function
		}

                gpmc_close();
	}
	
	if(status != CB_OK)
	   fprintf( stdout, "\nError code: %d\n\n", status );
	else
	   fprintf( stdout, "\nOperation Completed!!!\n\n" );
	
	return status;

}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
