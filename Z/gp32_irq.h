//#define __GCC3x__               // select your GCC version..
//#define __GCC2x__

// timers

volatile long* tcfg0  = (long*)0x15100000;
volatile long* tcfg1  = (long*)0x15100004;
volatile long* tcon   = (long*)0x15100008;
volatile long* tcntb0 = (long*)0x1510000c;
volatile long* tcmpb0 = (long*)0x15100010;
volatile long* tcnto0 = (long*)0x15100014;
volatile long* tcntb1 = (long*)0x15100018;
volatile long* tcmpb1 = (long*)0x1510001c;
volatile long* tcnto1 = (long*)0x15100020;
volatile long* tcntb2 = (long*)0x15100024;
volatile long* tcmpb2 = (long*)0x15100028;
volatile long* tcnto2 = (long*)0x1510002c;
volatile long* tcntb3 = (long*)0x15100030;
volatile long* tcmpb3 = (long*)0x15100034;
volatile long* tcnto3 = (long*)0x15100038;
volatile long* tcntb4 = (long*)0x1510003c;
volatile long* tcnto4 = (long*)0x15100040;

//

#define MAGIC_VCLK 0x98aeb8

//
// Calculate correct CLOCKVAL to use with LCDCON1 register
//

int clockVal( void ) {

	unsigned long hclk = GpHClkGet();
	unsigned long vclk = MAGIC_VCLK;
	int clkval = 0;

	while (hclk >= vclk) {
		clkval++;
		hclk -= vclk;
	}
	
	vclk >>= 2;
	if (hclk < vclk) { clkval--; }
	
	return clkval;
}

//
// Calculate current FrameRate. This code expects that the
// screen has actually been set up correctly.
//

double frameRate( void ) {
	long reg;
	int vspw, vbpd, vfpd;
	int hspw, hbpd, hfpd;
	int lineval, hozval, clkval;
	double frate;
	long hclk = (long)GpHClkGet();

volatile long* lcdcon1 = (long*)0x14a00000;
volatile long* lcdcon2 = (long*)0x14a00004;
volatile long* lcdcon3 = (long*)0x14a00008;
volatile long* lcdcon4 = (long*)0x14a0000c;

	// Read values directly from LCDCONx registers

	clkval  = (*lcdcon1 >> 8) & 0x3ff;
	reg     = *lcdcon2;
	vbpd    = reg >> 24;
	vfpd    = (reg >> 6) & 0xff;
	vspw    = reg & 0x3f;
	lineval = (reg >> 14) & 0x3ff;

	reg     = *lcdcon3;
	hbpd    = (reg >> 19) & 0x7f;
	hozval  = (reg >> 8) & 0x7ff;
	hfpd    = reg & 0xff;

	hspw    = *lcdcon4 & 0xff;

	// and calculate..

	frate = (double)hclk / (double)(((vspw+1)+(vbpd+1)+(lineval+1)+(vfpd+1))*
                   ((hspw+1) +(hbpd+1)+(hozval+1) +(hfpd+1))*
                   (2*(clkval+1)));
	return frate;
}

//
// Wait for specified LCD line. Note that the LCD must be enabled
// when calling this function.
//

void waitline( int line ) {
        volatile long* lcdcon1 = (long*)0x14a00000;
	while (line != ((*lcdcon1 >> 18) & 0x1ff) );
}


//
// These two variables are used by the emulated vblank interrupt handler
//

static int _line;
static long _pclk;

//
// The timer4 interrupt handler code.. GCC version checking shamelessly
// taken from _ttl_'s sample player code -> and thus never tested on GCC 2.x something
//

#ifdef GP32_GCC
#include "gp32_irq_gcc.h"
#endif

#ifdef GP32_ADS
#include "gp32_irq_arm.h"
#endif

#ifdef GP32_ARM
#include "gp32_irq_arm.h"
#endif



//
// Installs timer4 interrupt and calculates correct timer values for it.
// This code SHOULD use SDK provided timers.. but I had to reinvent the wheel!
//
// Parameters:
//   int line - at what rasterline the emulated vblank interrupt should occur.
//              Do not place it into line 0. Lines 1 to 3 are ok.
//

void installVBlank( int line ) {
	double pclk;

	// Calculate timer frequencies etc..

	pclk = (double)GpPClkGet();
	pclk /= 16;	//   clock divider 1/16
	pclk /= 256; //   prescaler 256
	pclk /= frameRate();
	_pclk = (long)(pclk);
	_line = line;

	// Install timer4 interrupt..

	ARMDisableInterrupt();


	swi_install_irq(14,timer4Int);
        waitline(_line);

        *tcntb4 = _pclk;                                                                // timer4 counter
	*tcon = (*tcon & 0x0fffff) | 0x200000;			// manually update timer4 counter
	*tcfg0 |= 0xff00;								// prescaler for timers 2,3,4
	*tcfg1 = (*tcfg1 & 0x0ffff) | 0x030000;			// configure timer4, clock divider 1/16
	*tcon = (*tcon & 0x0fffff) | 0x100000;			// start timer4


/*
        swi_install_irq(10,timer4Int);
        waitline(_line);

        *tcntb4 = _pclk;                                                                // timer0 counter
        *tcon = (*tcon & 0xfffff0ff) | (0x2<<8);                // manually update timer0 counter
        *tcfg0 |= 0xff00;                                       // prescaler for timers 2,3,4
        *tcfg1 = (*tcfg1 & 0xfffffff8) | 0x03;                  // configure timer0, clock divider 1/16
        *tcon = (*tcon & 0xfffff0ff) | (0x1<<8);                // start timer0
*/

	ARMEnableInterrupt();


// pclk=(fclk/4)           //gpclock(fclk 13100000,xxx,3) -> 3 means pclk=fclk/4
// tick = ( pclck / (tcfg0+1) / 2^(tcon+1) ) * tcntb4 adjust
// in this case
// tick = 131000000/4) / 255+1 / 2^(2+1) * 1000
// tick = 16000 hz         * 1000
// tick = 1/16000 seconds  * 1000
// 1/16 -> irq every 0.0625 secs, or 16 irqs per second
// we want 60 irqs per second (60/16) -> decrease by 3.75
// so adjust now is 1000/3.75=266 -> that makes 60 irqs per second


}

//
// Remove timer4 interrupt -> remove emulated vblank.
//

void uninstallVBlank() {
	ARMDisableInterrupt();
	swi_uninstall_irq(14);
//        swi_uninstall_irq(10);
	ARMEnableInterrupt();
}

//
//
//
//
//

/*
	installVBlank( 200 );	// Install VBlank on rasterline 200

	uninstallVBlank();
*/
