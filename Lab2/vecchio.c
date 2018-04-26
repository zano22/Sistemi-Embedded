#include <c8051f020.h> // SFR definitions

#define OFF 0 //off
#define ON 1 //on


sbit Button = P3^7;
sbit Led = P1^6;

unsigned long int timer;
unsigned long int timer4;

sfr16 RCAP2  = 0xCA;            // Timer2 capture/reload
sfr16 T2     = 0xCC;            // Timer2
sbit Over = T2CON^7;

sfr16 RCAP4  = 0xCA;            // Timer4 capture/reload
sfr16 T4     = 0xCC;            // Timer4
//sbit Over4 = T4CON^7;

int stato;
int click;


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

//parte 2
void init_timer(unsigned int counts){
	CKCON = 0x00;	// Define clock (T2M). Timer 2			// uses system clock DIV BY 12
	T2CON = 0x00;
	RCAP2 = counts;	// Init reload values in the Capture registers
	T2 = 0xFFFF;      // count register set to reload immediately when the first clock occurs
	IE |= 0x20;     	// IE.5, Enable Timer 2 interrupts (ET2)
	T2CON |= 0x04; 
}

void timer_ISR(void) interrupt 5{//parte 2  
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

//parte 3
void init_button(void){
	P3IF = 0x00;
	EIE2 |= 0x20;
}

void interrupt_button(void) interrupt 19{
	P3IF &= 0x7F;
	if(Led==ON){
		Led=OFF;
	} else {
		Led=ON;
	}
}

//parte 4
void init_timer4(unsigned int counts){
	CKCON = 0x00;	// Define clock (T2M). Timer 2			// uses system clock DIV BY 12
	T4CON = 0x00;
	RCAP4 = counts;	// Init reload values in the Capture registers
	T4 = 0xFFFF;      // count register set to reload immediately when the first clock occurs
	IE |= 0x20;     	// IE.5, Enable Timer 2 interrupts (ET2)
	T4CON |= 0x04; 
}

void timer4_ISR(void) interrupt 5{//parte 2  
	T4CON &= ~(0x80);
	if(timer4==1){
		timer4=0;
		if(Led==ON){
			Led=OFF;
		} else {
			Led=ON;
		}
	} else {
		timer4=1;
	}
}

void init_timer2_200(unsigned int counts){
	CKCON = 0x00;	// Define clock (T2M). Timer 2			// uses system clock DIV BY 12
	T2CON = 0x00;
	RCAP2 = counts;	// Init reload values in the Capture registers
	T2 = 0xFFFF;      // count register set to reload immediately when the first clock occurs
	IE |= 0x20;     	// IE.5, Enable Timer 2 interrupts (ET2)
	T2CON |= 0x04; 
}

void timer2_200_ISR(void) interrupt 5{//parte 2  
	T2CON &= ~(0x80);
	T2CON &= ~(0x04);
	click = 0;
}

void init_double(){
	P3IF = 0x00;
	EIE2 |= 0x20;
}

void interrupt_double(void) interrupt 19{
	P3IF &= 0x7F;
	//primo click
	if(click==0){
		//aspetto 200ms
		click = 1;
		init_timer2_200(9333);
	} else { //se doppio click
		T2CON = 0x00;//stop timer
		if(stato==0){//cambio stato
			stato=1;
			T4CON = 0x00;
		}	else {
			stato=0;
			init_timer4(9333);
			timer4=0;
		}
		//altrimenti nulla
		click = 0;
	}
}

void parte1(){
	EA = 0;
	init_timer(23334);
	timer=0;
	while(1){
		if(Over==1){
			Over=0;
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
	}
}

void parte2(){
	init_timer(23334);
	timer=0;
}

void parte3(){
	init_button();
}

void parte4(){
	stato = 0;
	click = 0;
	//init_double();
	init_timer4(9333);
	timer=0;
}

void main (void) {
	init();
	
	//parte1();
	//parte2();
	//parte3();
	parte4();
	
	while(1);
}