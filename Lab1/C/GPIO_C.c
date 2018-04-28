#include <c8051f020.h> // SFR definitions

#define OFF 0	//off
#define ON 1	//on

sbit Button = P3^7;
sbit Led = P1^6;

void init (void) {
	EA = 0; //disabilita interrupt
	
	//disabilita watchdog timer
	WDTCN = 0xDE;
	WDTCN = 0xAD;
	
	XBR0 = 0x000;
	XBR1 = 0x000;
	XBR2 = 0x40;
	
	P1MDOUT = 0x040; //setta pin come out
	
	Led = OFF;
}

void main (void) {
	init();
	
	while(1){
		if(Button==0){ //se premo il bottone
			Led = ON; //accendo il led
		} else { //se il bottone non è premuto
			Led = OFF; //spengo il led
		}
	}
}