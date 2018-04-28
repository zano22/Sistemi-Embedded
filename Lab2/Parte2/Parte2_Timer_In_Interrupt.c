#include <c8051f020.h> // SFR definitions

/*
Faccio partire un timer e ogni 500ms mi viene genrato un interrupt che mi cambia lo stato del led.
*/

#define OFF 0	//off
#define ON 1	//on


sbit Button = P3^7;
sbit Led = P1^6;
unsigned long int timer;
sfr16 RCAP2  = 0xCA;	// Timer2 capture/reload
sfr16 T2     = 0xCC;	// Timer2
sbit Over = T2CON^7;


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

void init_timer(unsigned int counts){
	CKCON = 0x00; // Define clock (T2M). Timer 2 uses system clock DIV BY 12
	T2CON = 0x00;
	RCAP2 = counts; // Init reload values in the Capture registers
	T2 = 0xFFFF; // count register set to reload immediately when the first clock occurs
	IE |= 0x20; // IE.5, Enable Timer 2 interrupts (ET2)
	T2CON |= 0x04; 
}

void timer_ISR(void) interrupt 5{ //interrupt timer 2
	T2CON &= ~(0x80);
	if(timer==1){
		timer=0;
		if(Led==ON){
			Led=OFF;
		} else {
			Led=ON;
		}
	} else {
		timer=1;
	}
}

void parte2(){
	init_timer(23334);
	timer=0;
}

void main (void) {
	init();
	
	parte2();
	
	while(1);
}