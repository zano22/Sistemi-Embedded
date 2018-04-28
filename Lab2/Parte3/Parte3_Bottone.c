#include <c8051f020.h> // SFR definitions

/*
Cambio lo stato del led quando schiacchio il bottone.
Quando lo premo viene generato un interrupt che cambia lo stato.
*/

#define OFF 0	//off
#define ON 1	//on


sbit Button = P3^7;
sbit Led = P1^6;


void init (void) {
	EA = 0; //disabilita interrupt
	
	OSCICN = 0x04; //2Mhz
	
	//disabilita watchdog timer
	WDTCN = 0xDE;
	WDTCN = 0xAD;
	
	XBR0 = 0x00;
	XBR1 = 0x00;
	XBR2 = 0x40;
	
	EIE1 |= 0x02;
	
	P1MDOUT = 0x040; //setta pin come out
	
	Led = OFF;
	
	EA = 1;
}

void init_button(void){
	P3IF = 0x00;
	EIE2 |= 0x20;
}

void interrupt_button(void) interrupt 19{ //interrupt sul bottone
	P3IF &= 0x7F;
	if(Led==ON){
		Led=OFF;
	} else {
		Led=ON;
	}
}

void parte3(){
	init_button();
}

void main (void) {
	init();
	
	parte3();
	
	while(1);
}