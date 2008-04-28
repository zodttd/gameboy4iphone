/*
void ARMEnableInterrupt(void);
void ARMDisableInterrupt(void);

void ARMEnableInterrupt(void) {
//r0,CPSR
        __asm {                      
                mrs      r0,CPSR       //E10F0000
                bic      r0,r0,#0x80   //E3C00080
                msr      CPSR,r0       //E12FF000
        }
}

void ARMDisableInterrupt(void) {
        __asm {
                mrs      r0,CPSR       //E10F0000
                orr      r0,r0,#0xc0
                msr      CPSR,r0       //E12FF000
        }
}


void swi_install_irq(unsigned long nr,void *ptr);
void swi_uninstall_irq(unsigned long nr);

void swi_install_irq(unsigned long nr,void *ptr) {
        __asm {
                mov      r0,nr
                mov      r1,ptr
                swi      0x9, {r0-r1}, {r14}
        }
}

void swi_uninstall_irq(unsigned long nr) {
        __asm {
                mov      r0,nr
                swi      0xa, {r0}, {r14}
        }
}

extern void soundtimer(void);

// (arm sdt/ads) 
void __irq myDMA2_ISR(void);
void myDMA2_ISR(void) {

        soundtimer();

}

*/

extern void soundtimer(void);

void __irq timer4Int(void);

void __irq timer4Int(void) {

	// Line synchronization!

        // Reload & restart the timer4

	// Place you code here... DO NOT overflow vblank time!

        *tcon = (*tcon & 0x0fffff) | 0x000000;  // stop timer4

        soundtimer();


//        waitline(_line);

	//*tcntb4 = _pclk;			// timer4 counter
        *tcntb4 = 60;                           // timer4 counter
        *tcon = (*tcon & 0x0fffff) | 0x200000;  // manual update timer4 counter
        *tcon = (*tcon & 0x0fffff) | 0x100000;  // start timer4



/*
//      *tcntb4 = _pclk;                                                                // timer0 counter
        *tcon = (*tcon & 0xfffff0ff) | (0x2<<8);                // manually update timer0 counter
        *tcon = (*tcon & 0xfffff0ff) | (0x1<<8);                // start timer0
*/

	// Your code ends..
}





