#include <c8051f020.h> // SFR definitions

/*
Led controllato da un PWM con luminosità sia impostabile.
Il pulsante ha 2 funzioni:

	- Se premuto e rilasciato prima di 1s 
		Cambia lo stato del led da acceso a spento e viceversa.

	- Se premuto per più di 1s
		Fa entrare nella modalità configurazione dove il Led deve essere acceso.
		Finché il pulsante viene tenuto premuto la luminosità del Led varia in modo continuo.
		Il rilascio fa uscire dalla modalità di configurazione.
		Alla fine il Led è acceso al nuovo valore di luminosità selezionato.
*/

#define OFF 0	// off
#define ON 1	// on

#define RISING 0x08		// interrupt quando rilascio
#define FALLING 0x00	// interrupt quando premo


sbit Button = P3^7;
sbit Led = P1^6;

unsigned long int timer;
sfr16 RCAP2  = 0xCA; // Timer2 capture/reload
sfr16 T2     = 0xCC; // Timer2
sbit Over = T2CON^7;

bit modConfig; // 0 non attivata, 1 attivata
char direzione; // 1 avanti, -1 indietro
bit statoLed; // stato indipendente da come cambia Led in PWM
unsigned char Lumi; // intensità luminosa del led


void init (void) {
	EA = 0; // disabilita interrupt
	
	OSCICN = 0x04; // 2Mhz
	
	// disabilita watchdog timer
	WDTCN = 0xDE;
	WDTCN = 0xAD;
	
	XBR0 = 0x00;
	XBR1 = 0x00;
	XBR2 = 0x40;
	
	EIE1 |= 0x02;
	
	P1MDOUT = 0x040; // setta pin come out
	
	EA = 1;
}

void init_timer2(unsigned int counts){ // inizializzo timer conteggio 1s
	T2CON = 0x00;
	RCAP2 = counts; // Init reload values in the Capture registers
	T2 = 0xFFFF; // count register set to reload immediately when the first clock occurs
	IE |= 0x20; // IE.5, Enable Timer 2 interrupts (ET2)
	T2CON |= 0x04; 
}

void interrupt_timer2(void) interrupt 5{ // lanciato ogni 10ms
	T2CON &= ~(0x80);
	if(modConfig==0){ // se non sono in modalità configurazione
		timer++; // incremento contatore
		if(timer==100){ // se sono arrivato a 1s
			modConfig = 1; // entro in modalità configurazione
			TR0 = 1; // faccio ripartire timer 0
			timer=0; // azzero contatore
		}
	} else { // se sono in modalità configurazione
		Lumi=Lumi + direzione; // modifico luminosità
		if(Lumi>=255 || Lumi<=0){ // se la luminosità è fuori range
			direzione = -direzione; // cambio direzione
		}
	}
}

void init_click(){ // inizializzo bottone
	P3IF = 0x00;
	EIE2 |= 0x20;
	Led=OFF; // setto led spento
	statoLed=OFF; // setto stato led spento
}

void interrupt_click(void) interrupt 19{ // interrupt click bottone
	P3IF &= 0x7F; // resetto richiesta di interrupt per poterne ricevere altre
	if (P3IF==FALLING){ // se il pulsante è premuto
		P3IF=RISING; // setto per  pulsante rilasciato
		init_timer2(63500); // faccio partire il timer entro il quale rilasciarlo per cambiare stato
		timer=0;
	} else { // se il pulsante è rilasciato
		P3IF=FALLING; // setto per pulsante premuto
		TF0 = 0;
		TR0 = 0; // stoppo timer 0
		T2CON &= ~(0x80); 
		T2CON &= ~(0x04); // stoppo timer 2
		if(modConfig==0){ // se sono in modalità di configurazione
			if(statoLed==ON){ // se lo stato del led è acceso
				Led=OFF; // spengo led
				statoLed = OFF; // setto stato a spento
			} else { // se lo stato del led è acceso
				TR0 = 1; // faccio ripartire timer 0
				Led=ON; // accendo led
				statoLed = ON; // setto stato a acceso
			}
		} else { // se sono in modalità configurazione
			TR0 = 1; // faccio ripartire timer 0
			Led = ON; // accendo led
			statoLed = ON; // setto stato a acceso
			modConfig = 0; // esco dalla modalità configurazione
		}
	}
}

void interrupt_timer0() interrupt 1{ // creo onda per duty-cycle tramite timer
	TF0 = 0;
	TR0 = 0; // stoppo timer 0

	if(Led==ON){ // se il led è acceso
		Led = OFF; // spengo led
		TL0 = Lumi; // imposto duty-cycle
	} else { // se il led è spento
		Led = ON; // accendo led
		TL0 = 0xFF - Lumi; // imposto duty-cycle
	}

	TR0 = 1; // faccio ripartire timer 0
}

void init_timer0(void){ // inizializzo timer duty-cycle
	TMOD |= 0x02;
	CKCON |= 0x04;
	TL0 = 0xFF; // valore iniziale
	TH0 = 0x00; // valore da cui ripartire dopo overflow
	ET0 = 1;
	TR0 = 1; // faccio partire timer 0
}

void pwm(){ // inizializzo dati per PWM
	init_click(); // inizializzo bottone
	modConfig = 0; // modalità configurazione non attivata
	direzione = 1; // scelgo direzione crescente per la luminosità
	statoLed = ON; // stato led acceso
	Lumi = 128; // luminosità di partenza
	init_timer0(); // inizializzo timer 0
}

void main (void) {
	init(); // inizializzazione generale
	
	pwm(); // inizializzo per PWM
	
	while(1); // ciclo infinito
}