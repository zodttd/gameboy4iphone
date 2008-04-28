/*
    File:			JoyPad.c
	
    Description:	This code handles the Input from joypad connected to UART (tty.iap)
    				It also shows how to open, write to, read from, and close a serial port.
                
    Copyright:		© Copyright 2007-200? Freeware2. All rights reserved.
	
    Disclaimer:		This Software is provided by Freeware2 on an "AS IS" basis.  Freeware2 MAKES NO
					WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
					WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
					PURPOSE, REGARDING Freeware2 OR ITS USE AND OPERATION ALONE OR IN
					COMBINATION WITH YOUR PRODUCTS.

					IN NO EVENT SHALL Freeware2 BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
					CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
					GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
					ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
					OF THIS SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
					(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF Freeware2 HAS BEEN
					ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
				
	Change History (most recent first):
        
            <1>	 	07/14/07	New sample.
        
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <paths.h>
#include <termios.h>
#include <sysexits.h>
#include <sys/param.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>
#include <AvailabilityMacros.h>

#ifdef __MWERKS__
#define __CF_USE_FRAMEWORK_INCLUDES__
#endif

#include <CoreFoundation/CoreFoundation.h>

#include <IOKit/IOKitLib.h>
#include <IOKit/serial/IOSerialKeys.h>
#if defined(MAC_OS_X_VERSION_10_3) && (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_3)
#include <IOKit/serial/ioss.h>
#endif
#include <IOKit/IOBSD.h>

#include "app_iPhone.h"

#define INP_BUTTON_UP				(0)
#define INP_BUTTON_DOWN				(1)
#define INP_BUTTON_LEFT				(2)
#define INP_BUTTON_RIGHT			(3)
#define INP_BUTTON_HARDB			(4)
#define INP_BUTTON_HARDA			(5)
#define INP_BUTTON_HARDAB			(6)
#define INP_BUTTON_START			(7)
#define INP_BUTTON_SELECT			(8)


#define BIT_U			(1<<INP_BUTTON_UP)
#define BIT_D			(1<<INP_BUTTON_DOWN)
#define BIT_L 			(1<<INP_BUTTON_LEFT)
#define BIT_R		 	(1<<INP_BUTTON_RIGHT)
#define BIT_B			(1<<INP_BUTTON_HARDB)
#define BIT_A			(1<<INP_BUTTON_HARDA)
#define BIT_AB			(1<<INP_BUTTON_HARDAB)
#define BIT_ST			(1<<INP_BUTTON_START)
#define BIT_SEL			(1<<INP_BUTTON_SELECT)

#define BIT_LPAD1		(1<<29)
#define BIT_RPAD1		(1<<30)
#define BIT_MENU		(1<<31)


// Function prototypes
static int OpenSerialPort(const char *bsdPath);
static void CloseSerialPort(int fileDescriptor);

#define			PROTOCOL_SIZE	5
#define 		START_TOKEN		0x6F
#define 		END_TOKEN		0xFE

unsigned char	rx_ep = 0, rx_sp = 0;
unsigned char	Queue[PROTOCOL_SIZE*122];
unsigned short	key_data;
int			fileDescriptor;
int				ttyS2 = -1;
// Hold the original termios attributes so we can reset them
static struct termios gOriginalTTYAttrs;

void	ProtocolQ(unsigned char rdata)
{	
	rx_ep %= PROTOCOL_SIZE;	
	if(rx_ep == rx_sp)		
	{		
		rx_sp++;	
		rx_sp %= PROTOCOL_SIZE;	
	}	
	Queue[rx_ep++] = rdata;
}						    

int	ProtocolCheck(void)	
{	
	if(Queue[(rx_sp + PROTOCOL_SIZE-1) % PROTOCOL_SIZE] != END_TOKEN)	
		return	0; 	
	if(Queue[(rx_sp +            	0) % PROTOCOL_SIZE] != START_TOKEN)	
		return	0;	

	return	1;
}

int	ProtocolCatch(void)
{	
	unsigned char 	ck_sum;	
	unsigned char 	rd_ck_sum = Queue[(rx_ep + 3) % PROTOCOL_SIZE];
	unsigned short	kdata;		

	ck_sum = Queue[(rx_sp + 1) % PROTOCOL_SIZE] + Queue[(rx_sp + 2) % PROTOCOL_SIZE];	
	if (rd_ck_sum != ck_sum)	
	{		
		printf(" Protocol Check Sum Fail!!\n");			
		return 	-1;	
	}	
	kdata  = Queue[(rx_sp + 1) % PROTOCOL_SIZE] << 8; 	
	kdata |= Queue[(rx_sp + 2) % PROTOCOL_SIZE];	
	return	kdata;
}

int		KeyCheck(unsigned short *KeyData)	
{	
	unsigned char	rdata;	
	int				kdata;	
	int				len;
	while(1)
	{
		len=read(fileDescriptor, (char *)&rdata, 1);
		if(len>=1)
		{
			ProtocolQ(rdata);				
			if(ProtocolCheck())		
			{
				kdata = ProtocolCatch();	
				if(kdata < 0)	return	0;			
				else			*KeyData = (unsigned short)kdata;
				return	1;		
			}
		}
		else
			break;	
	}	
	return	0;
}

// Given the path to a serial device, open the device and configure it.
// Return the file descriptor associated with the device.
static int OpenSerialPort(const char *bsdPath)
{
	int				handshake;	
	struct termios	options;
    // Check exsitance of joypad dev-node
    // If not exists, make it 
    
 //   if(mknod("/dev/tty.joypad", S_IFCHR|S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH, const char *path, mode_t mode, dev_t dev)!=0) 

    // Check exsitance of iap dev-nodes (tty.iap, cu.iap, uart.iap)
    // If exist, remove all of them


    // Open the serial port read/write, with no controlling terminal, and don't wait for a connection.
    // The O_NONBLOCK flag also causes subsequent I/O on the device to be non-blocking.
    // See open(2) ("man 2 open") for details.
    
    fileDescriptor = open(bsdPath, O_RDWR | O_NOCTTY | O_NONBLOCK |O_NDELAY);
    if (fileDescriptor == -1)
    {
        printf("Error opening serial port %s - %s(%d).\n",
               bsdPath, strerror(errno), errno);
        goto error;
    }
#if 1
    // Note that open() follows POSIX semantics: multiple open() calls to the same file will succeed
    // unless the TIOCEXCL ioctl is issued. This will prevent additional opens except by root-owned
    // processes.
    // See tty(4) ("man 4 tty") and ioctl(2) ("man 2 ioctl") for details.
    
    if (ioctl(fileDescriptor, TIOCEXCL) == -1)
    {
        printf("Error setting TIOCEXCL on %s - %s(%d).\n",
            bsdPath, strerror(errno), errno);
        goto error;
    }
    
    // Now that the device is open, set the O_NONBLOCK flag so subsequent I/O will be okay.
    // See fcntl(2) ("man 2 fcntl") for details.
    
    if (fcntl(fileDescriptor, F_SETFL, O_NONBLOCK) == -1)
    {
        printf("Error clearing O_NONBLOCK %s - %s(%d).\n",
            bsdPath, strerror(errno), errno);
        goto error;
    }
    
    // Get the current options and save them so we can restore the default settings later.
    if (tcgetattr(fileDescriptor, &gOriginalTTYAttrs) == -1)
    {
        printf("Error getting tty attributes %s - %s(%d).\n",
            bsdPath, strerror(errno), errno);
        goto error;
    }

    // The serial port attributes such as timeouts and baud rate are set by modifying the termios
    // structure and then calling tcsetattr() to cause the changes to take effect. Note that the
    // changes will not become effective without the tcsetattr() call.
    // See tcsetattr(4) ("man 4 tcsetattr") for details.
    
    options = gOriginalTTYAttrs;
    
    // Print the current input and output baud rates.
    // See tcsetattr(4) ("man 4 tcsetattr") for details.
    
    printf("Current input baud rate is %d\n", (int) cfgetispeed(&options));
    printf("Current output baud rate is %d\n", (int) cfgetospeed(&options));
    
    // Set raw input (non-canonical) mode, with reads blocking until either a single character 
    // has been received or a one second timeout expires.
    // See tcsetattr(4) ("man 4 tcsetattr") and termios(4) ("man 4 termios") for details.
    
    cfmakeraw(&options);
    options.c_cc[VMIN] = 1;
    options.c_cc[VTIME] = 10;
        
    // The baud rate, word length, and handshake options can be set as follows:
    
    cfsetspeed(&options, B9600);		// Set 19200 baud    
    options.c_cflag |= (CS8);			// Use 8 bit words
/*     	 						   | 	
						PARENB	   | 	// Parity enable (even parity if PARODD not also set)
						CCTS_OFLOW | 	// CTS flow control of output
						CRTS_IFLOW);	// RTS flow control of input
*/			
#if defined(MAC_OS_X_VERSION_10_4) && (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_4)
	// Starting with Tiger, the IOSSIOSPEED ioctl can be used to set arbitrary baud rates
	// other than those specified by POSIX. The driver for the underlying serial hardware
	// ultimately determines which baud rates can be used. This ioctl sets both the input
	// and output speed. 
				   
	speed_t speed = 9600; // Set 19200 baud
    if (ioctl(fileDescriptor, IOSSIOSPEED, &speed) == -1)
    {
        printf("Error calling ioctl(..., IOSSIOSPEED, ...) %s - %s(%d).\n",
            bsdPath, strerror(errno), errno);
    }
#endif
    
    // Print the new input and output baud rates. Note that the IOSSIOSPEED ioctl interacts with the serial driver 
	// directly bypassing the termios struct. This means that the following two calls will not be able to read
	// the current baud rate if the IOSSIOSPEED ioctl was used but will instead return the speed set by the last call
	// to cfsetspeed.
    
    printf("Input baud rate changed to %d\n", (int) cfgetispeed(&options));
    printf("Output baud rate changed to %d\n", (int) cfgetospeed(&options));
    
    // Cause the new options to take effect immediately.
    if (tcsetattr(fileDescriptor, TCSANOW, &options) == -1)
    {
        printf("Error setting tty attributes %s - %s(%d).\n",
            bsdPath, strerror(errno), errno);
        goto error;
    }

    // To set the modem handshake lines, use the following ioctls.
    // See tty(4) ("man 4 tty") and ioctl(2) ("man 2 ioctl") for details.
    
    if (ioctl(fileDescriptor, TIOCSDTR) == -1) // Assert Data Terminal Ready (DTR)
    {
        printf("Error asserting DTR %s - %s(%d).\n",
            bsdPath, strerror(errno), errno);
    }
    
    if (ioctl(fileDescriptor, TIOCCDTR) == -1) // Clear Data Terminal Ready (DTR)
    {
        printf("Error clearing DTR %s - %s(%d).\n",
            bsdPath, strerror(errno), errno);
    }
    
    handshake = TIOCM_DTR | TIOCM_RTS | TIOCM_CTS | TIOCM_DSR;
    if (ioctl(fileDescriptor, TIOCMSET, &handshake) == -1)
    // Set the modem lines depending on the bits set in handshake
    {
        printf("Error setting handshake lines %s - %s(%d).\n",
            bsdPath, strerror(errno), errno);
    }
    
    // To read the state of the modem lines, use the following ioctl.
    // See tty(4) ("man 4 tty") and ioctl(2) ("man 2 ioctl") for details.
    
    if (ioctl(fileDescriptor, TIOCMGET, &handshake) == -1)
    // Store the state of the modem lines in handshake
    {
        printf("Error getting handshake lines %s - %s(%d).\n",
            bsdPath, strerror(errno), errno);
    }
    
    printf("Handshake lines currently set to %d\n", handshake);
	
#if defined(MAC_OS_X_VERSION_10_3) && (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_3)
	unsigned long mics = 1UL;

	// Set the receive latency in microseconds. Serial drivers use this value to determine how often to
	// dequeue characters received by the hardware. Most applications don't need to set this value: if an
	// app reads lines of characters, the app can't do anything until the line termination character has been
	// received anyway. The most common applications which are sensitive to read latency are MIDI and IrDA
	// applications.
	
	if (ioctl(fileDescriptor, IOSSDATALAT, &mics) == -1)
	{
		// set latency to 1 microsecond
        printf("Error setting read latency %s - %s(%d).\n",
            bsdPath, strerror(errno), errno);
        goto error;
	}
#endif

#endif    
    // Success
	rx_ep = 0, rx_sp = 0;
    	return fileDescriptor;
    
    // Failure path
error:
    if (fileDescriptor != -1)
    {
        close(fileDescriptor);
    }
    
    return -1;
}


// Given the file descriptor for a serial device, close that device.
void CloseSerialPort(int fileDescriptor)
{
#if 1
    // Block until all written output has been sent from the device.
    // Note that this call is simply passed on to the serial device driver. 
	// See tcsendbreak(3) ("man 3 tcsendbreak") for details.
    if (tcdrain(fileDescriptor) == -1)
    {
        printf("Error waiting for drain - %s(%d).\n",
            strerror(errno), errno);
    }
    
    // Traditionally it is good practice to reset a serial port back to
    // the state in which you found it. This is why the original termios struct
    // was saved.
    if (tcsetattr(fileDescriptor, TCSANOW, &gOriginalTTYAttrs) == -1)
    {
        printf("Error resetting tty attributes - %s(%d).\n",
            strerror(errno), errno);
    }
#endif
	KeyCheck(&key_data); // Clean up serial buffer;
	printf("Clena up uart buffer completed!\n");
    	close(fileDescriptor);
}

//#define BUFSIZE 100
//char readbuf[BUFSIZE];
int Init_joypad(void)
{
//    kern_return_t	kernResult; // on PowerPC this is an int (4 bytes)
/*
 *	error number layout as follows (see mach/error.h):
 *
 *	hi		 		       lo
 *	| system(6) | subsystem(12) | code(14) |
 */

//    io_iterator_t	serialPortIterator;
    char		bsdPath[MAXPATHLEN]="/dev/tty.iap";
/* 
    kernResult = FindModems(&serialPortIterator);
    
    kernResult = GetModemPath(serialPortIterator, bsdPath, sizeof(bsdPath));
    
    IOObjectRelease(serialPortIterator);	// Release the iterator.
*/    
    // Now open the modem port we found, initialize the modem, then close it
    if (!bsdPath[0])
    {
        printf("No modem port found.\n");
        return EX_UNAVAILABLE;
    }

    fileDescriptor = OpenSerialPort(bsdPath);
    if (-1 == fileDescriptor)
    {
        return EX_IOERR;
    }
/*
    if (InitializeModem(fileDescriptor))
    {
        printf("Modem initialized successfully.\n");
    }
    else {
        printf("Could not initialize modem.\n");
    }
*/        
	printf("Clena up uart buffer!\n");

	KeyCheck(&key_data); // Clean up serial buffer;
	key_data=0;
	printf("Joynux init OK.!\n");
        
    return EX_OK;
}

int End_joypad(void)
{
    CloseSerialPort(fileDescriptor);
    printf("Modem port closed.\n");
}

unsigned int old_button=0;
unsigned int Read_joypad(void)
{
	  unsigned int button=0;
	  if(KeyCheck(&key_data)) 
	  {
		  if(key_data & 0x2000) button  = BIT_U;  //UP
		  if(key_data & 0x1000) button |= BIT_D;  //DOWN
		  if(key_data & 0x8000) button |= BIT_L;  //LEFT	  
		  if(key_data & 0x4000) button |= BIT_R;  //RIGHT	  
		
		  if(key_data & 0x0010) button |= BIT_ST;	// START	  
		  if(key_data & 0x0020) button |= BIT_SEL;	// SELECT	

		  if(key_data & 0x0040) button |= BIT_RPAD1;	// TR	  
		  if(key_data & 0x0080) button |= BIT_LPAD1;	// TL	

		  if(key_data & 0x0200) button |= BIT_A;	// 1	- A	  
		  if(key_data & 0x0100) button |= BIT_B;	// 2	- B	
		  old_button=button;
		  //printf("%02x %02x\n",(key_data&0xff00)>>8, key_data&0xff);
	  }
	  return old_button;
}

