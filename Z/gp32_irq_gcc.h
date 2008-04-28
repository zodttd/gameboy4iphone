
#ifdef __GCC3x__
void timer4Int(void) __attribute__ ((interrupt ("IRQ")));
#endif

#ifdef __GCC2x__
void timer4Int(void) __attribute__ ((naked));
#endif


void timer4Int(void) {
#ifdef __GCC2x__
	asm volatile (
		"stmdb    r13!,{r0-r12,lr}"
	);
#endif
	hwpal[0] = 0xffff;		// just show that interrupt handler got called

	// Line synchronization!

	waitline(_line);

	// Reload & restart the timer4

	//*tcntb4 = _pclk;			// timer4 counter
	*tcon = (*tcon & 0x0fffff) | 0x200000;			// manual update timer4 counter
	*tcon = (*tcon & 0x0fffff) | 0x100000;			// start timer4

	// Place you code here... DO NOT overflow vblank time!



	// Your code ends..

	hwpal[0] = 0xfc00;		// Just show that interrupt handler is done..

#ifdef __GCC2x__
	asm volatile (
		"ldmia    r13!,{r0-r12,lr}\n"
		"subs     pc,lr,#4"
		);
#endif
}



void ARMEnableInterrupt(void);
void ARMDisableInterrupt(void);

void ARMEnableInterrupt(void) {
        asm volatile ("
                mrs      r0,CPSR
                bic      r0,r0,#0x80
                msr      CPSR,r0
                " :  :  : "r0" );
}

void ARMDisableInterrupt(void) {
        asm volatile ("
                mrs      r0,CPSR
                orr      r0,r0,#0xc0
                msr      CPSR,r0
                " :  :  : "r0" );
}

void swi_install_irq(unsigned long nr,void *ptr) __attribute__ ((naked));
void swi_uninstall_irq(unsigned long nr) __attribute__ ((naked));

void swi_install_irq(unsigned long nr,void *ptr) {
        asm volatile ("
                mov      r0,%0
                mov      r1,%1
                swi      0x9
                " :  : "r" (nr),"r" (ptr) : "r0", "r1", "r2" );
}

void swi_uninstall_irq(unsigned long nr) {
        asm volatile ("
                mov      r0,%0
                swi      0xa
                " :  : "r" (nr) : "r0", "r1", "r2" );
}

extern void soundtimer(void);

#ifdef GP32_GCC3x

// (gcc 3.x and up) 
void myDMA2_ISR(void) __attribute__ ((interrupt ("IRQ")));
void myDMA2_ISR(void) {

        soundtimer();

}

#endif

#ifdef GP32_GCC2x
// (gcc 2.x and up) 

void myDMA2_ISR(void) __attribute__ ((naked));
void myDMA2_ISR(void) {

        asm volatile ("
                stmdb    r13!,{r0-r12,lr}
                ");

        soundtimer();

        asm volatile ("
                ldmia    r13!,{r0-r12,lr}
                subs     pc,lr,#4
                ");

}

#endif
