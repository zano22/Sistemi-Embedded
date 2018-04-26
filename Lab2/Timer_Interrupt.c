#include <c8051f020.h> // SFR definitions

#define OFF 0 //off
#define ON 1 //on


sbit Button = P3^7;
sbit Led = P1^6;
unsigned long int timer;
sfr16 RCAP2  = 0xCA;            // Timer2 capture/reload
sfr16 T2     = 0xCC;            // Timer2
sfr16 RCAP4  = 0xE4;            // Timer4 capture/reload
sfr16 T4     = 0xF4;            // Timer4
sbit Over = T2CON^7;
int stato;	//stato = 0 -> stato A			stato = 1 -> stato B
int click;	//click = 0 -> primo click	click = 1 -> secondo click


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
	
	Led = OFF;
	
	EA = 1;
}

//parte 4
void init_timer2(unsigned int counts){ //timer lampeggio led
	T2CON = 0x00;
	RCAP2 = counts;	// Init reload values in the Capture registers
	T2 = 0xFFFF;      // count register set to reload immediately when the first clock occurs
	IE |= 0x20;     	// IE.5, Enable Timer 2 interrupts (ET2)
	T2CON |= 0x04; 
}

void timer2_ISR(void) interrupt 5{ //lampeggio com periodo 1s
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

void init_timer4_200(unsigned int counts){ //timer scadenza doppio click
	T4CON = 0x00;
	RCAP4 = counts;	// Init reload values in the Capture registers
	T4 = 0x0000;      // count register set to reload immediately when the first clock occurs
	EIE2 |= 0x04;     	// EIE2.2 Enable Timer 4 interrupts (ET4)
	T4CON |= 0x04; 
}

void timer4_200_ISR(void) interrupt 16{ //interrupt per il timer del doppio click
	T4CON &= ~(0x80);
	T4CON &= ~(0x04); //stop timer doppio click
	click = 0; //timer finito
}

void init_double(){ //inizializzo bottone
	P3IF = 0x00;
	EIE2 |= 0x20;
}

void interrupt_double(void) interrupt 19{ //interrupt per click bottone
	P3IF &= 0x7F;
	//primo click
	if(click==0){
		click = 1;
		//aspetto 200ms
		init_timer4_200(9500);
	} else {
		click=0;
		//se doppio click
		T4CON &= ~(0x04); //stop timer doppio click
		if(stato==0){ //cambio stato
			stato = 1;
			T2CON &= ~(0x04); //stop timer lampeggio
		} else {
			stato = 0;
			init_timer2(23334); //lampeggia led - stato A
			timer=0;
		}
	}
	//altrimenti nulla
}

void parte4(){
	stato = 0; //stato A
	click = 0; //primo click
	init_timer2(23334); //lampeggia led - stato A
	timer=0;
	init_double();
}

void main (void) {
	init();
	
	parte4();
	
	while(1);
}