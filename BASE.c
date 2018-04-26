#include <c8051f020.h> // SFR definitions

void init (void) {
	EA = 0; //disabilita interrupt
	
	OSCICN = 0x04;  //2Mhz
	
	//disabilita watchdog timer
	WDTCN = 0xDE;
	WDTCN = 0xAD;
	
	XBR0 = 0x00;
	XBR1 = 0x00;
	XBR2 = 0x40;
	
	EIE1 |= 0x02;
	
	P1MDOUT = 0x040;//setta pin come out
	
	EA = 1;
}

void main (void) {
	init();
	
	while(1);
}