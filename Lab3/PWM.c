#include <c8051f020.h> // SFR definitions

#define OFF 0 //off
#define ON 1 //on

#define RISING (P3IF == 0x08)
#define FALLING (P3IF == 0x00)


sbit Button = P3^7;
sbit Led = P1^6;

unsigned long int timer;
sfr16 RCAP2  = 0xCA;            // Timer2 capture/reload
sfr16 T2     = 0xCC;            // Timer2
sbit Over = T2CON^7;

int modConfig;   // 0 non attivata
int direzione = 1; //1 avanti, -1 indietro
unsigned char Lumi;


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

void init_timer2(unsigned int counts){ //timer lampeggio led
	T2CON = 0x00;
	RCAP2 = counts;	// Init reload values in the Capture registers
	T2 = 0xFFFF;      // count register set to reload immediately when the first clock occurs
	IE |= 0x20;     	// IE.5, Enable Timer 2 interrupts (ET2)
	T2CON |= 0x04; 
}

void timer2_ISR(void) interrupt 5{ //lampeggio com periodo 1s
	T2CON &= ~(0x80);
	if(modConfig==0){
		timer++;
		if(timer==100){
			modConfig = 1;
			timer=0;
		}
	} else {
		Lumi=Lumi + direzione;
		if(Lumi>=255 || Lumi<=0){
			direzione = -direzione;
		}
	}
	//cambio luminosità
}

void init_click(){ //inizializzo bottone
	P3IF = 0x00;
	EIE2 |= 0x20;
}

void interrupt_click(void) interrupt 19{ //interrupt per click bottone
	//se schiaccio
	//parte timer 10ms x 100
	
	//controllo pulsante
	//if (FALLING) {
	if (P3IF==0x08) {
		//aspetto 1000ms
		init_timer2(63500);
		//Led = ON;
		timer=0;
		P3IF |= 0x08;
	} else { // (RISING)
		T2CON &= ~(0x80);
		T2CON &= ~(0x04);
		if(modConfig==0){
			//accendo / spengo
			if(Led==ON){
				Led=OFF;
			} else {
				Led=ON;
			}
		} else {
			Led = ON;
		}
		modConfig = 0;
		P3IF &= 0xF7;
	}
	P3IF &= 0x7F;
	//se premo
	//altrimenti nulla
}

void timer0() interrupt 1
{
	TF0 = 0;
	TR0 = 0;

	if(Led==ON){
		Led = OFF;
		TL0 = Lumi;
	} else {
		Led = ON;
		TL0 = 0xFF - Lumi;
	}

	TR0 = 1;
}

void setup_timer0(void)
{
	TMOD |= 0x02;
	CKCON |= 0x04;
	TL0 = 0xFF;
	TH0 = 0x00;

	ET0 = 1;
	TR0 = 1;
}

void pwm(){
	Lumi = 0;
	modConfig = 0;
	setup_timer0();
	init_click();
}

void main (void) {
	init();
	
	pwm();
	
	while(1);
}