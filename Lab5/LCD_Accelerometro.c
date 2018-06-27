#include <c8051f020.h> // SFR definitions

#define WRITE 0xFE // SMBus WRITE command
#define READ 0x01 // SMBus READ command

#define SMB_START 0x08 // (MT & MR) START transmitted
#define SMB_RP_START 0x10 // (MT & MR) START transmitted
#define SMB_MTADDACK 0x18 // (MT) Slave address + W transmitted
#define SMB_MTDBACK 0x28 // (MT) data byte transmitted
#define SMB_MRADDACK 0x40 // (MR) Slave address + R transmitted;
#define SMB_MRDBACK 0x50 // (MR) data byte rec’vd; ACK transmitted

// indirizzi I2C_SMBus
#define LCD 0x3E
#define TERM 0x48
#define ACC 0x4C

sbit BackLight = P0^6;

bit SM_BUSY = 0; // This bit is set when a send or receive is started. It is cleared by the ISR when the operation is finished.

int Slave = 0;
int SM_Mode = 0;
int *DataWrite;
int *DataRead;
int DataLen = 0;

int cmd_init_lcd[8] = {0x38, 0x39, 0x14, 0x74, 0x54, 0x6F, 0x0C, 0x01}; // sequenza di inizializzazione lcd
int clean_LCD[]={0x00, 0x01}; // pulisce lcd
//int cmd_init_acc[2] = {0x07, 0x01};
int cmd_init_acc[4] = {0x06, 0x00, 0x01, 0x03}; // sequenza di inizializzazione accelerometro

// lookup table
// code inserisce nella memoria del codice
code int ANG[] = {
// Angoli positivi
	  0,   3,   5,   8,  11,  14,  16,  19,  22,  25,  28,
	 31,  34,  38,  41,  45,  49,  53,  58,  63,  70,  80,
// Maggiori di 80 / Shaken
	 90,  90,  90,  90,  90,  90,  90,  90,  90,  90,
// Minori di -80 / Shaken
	 90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,
// Angoli negativi (aggiungere - e controllare conversione)
	 80,  70,  63,  58,  53,  49,  45,  41,  38,  34,  31,
	 28,  25,  22,  19,  16,  14,  11,   8,   5,   3
};

void SM_Send (int chip_select, int *dati, int lunghezza, int mode){
	while (SM_BUSY); // Wait for SMBus to be free.
	SM_BUSY = 1; // Occupy SMBus (set to busy)
	SMB0CN = 0x44; // SMBus enabled, ACK on acknowledge cycle

	Slave = (chip_select << 1) & WRITE;
	SM_Mode = mode;
	DataWrite = dati;
	DataLen = lunghezza;

	STO = 0;
	STA = 1; // Start transfer
}

void SM_Read (int chip_select, int *dati, int lunghezza){
	while (SM_BUSY); // Wait for bus to be free.
	SM_BUSY = 1; // Occupy SMBus (set to busy)
	SMB0CN = 0x44; // SMBus enabled, ACK on acknowledge cycle
	
	Slave = (chip_select << 1) | READ;
	DataRead = dati;
	DataLen = lunghezza;

	STO = 0;
	STA = 1; // Start transfer
	while (SM_BUSY); // Wait for transfer to finish
}

void SMBUS_ISR (void) interrupt 7 {
	switch (SMB0STA){ // Status code for the SMBus (SMB0STA register)
		// Master Transmitter/Receiver: START condition transmitted.
		// The R/W bit of the COMMAND word sent after this state will
		// always be a zero (W) because for both read and write,
		// the memory address must be written first.
		case SMB_START:
			SMB0DAT = Slave; // Load address of the slave to be accessed.
			break;
		case SMB_RP_START:
			SMB0DAT = Slave; // Load address of the slave to be accessed.
			break;
		// Master Transmitter: Slave address + WRITE transmitted. ACK received.
		case SMB_MTADDACK:
			STA = 0;
			if (SM_Mode == 0x00 || SM_Mode == 0x40){
				SMB0DAT = SM_Mode;
			}else{
				SMB0DAT = *DataWrite;
				DataWrite++;
				DataLen--;
			}
			break;
		// Master Transmitter: Data byte transmitted. ACK received.
		// This state is used in both READ and WRITE operations. Check BYTE_NUMBER
		// for memory address status - if only HIGH_ADD has been sent, load LOW_ADD.
		// If LOW_ADD has been sent, check COMMAND for R/W value to determine
		// next state.
		case SMB_MTDBACK:
			if (SM_Mode == 0x01) {
				Slave |= READ;
				DataLen = 3;
				STA = 1;
			} else if (DataLen) {
				SMB0DAT = *DataWrite;
				DataWrite++;
				DataLen--;
			} else {
				STO = 1;
				SM_BUSY = 0;
			}
			break;
		// Master Receiver: Slave address + READ transmitted. ACK received.
		// Set to transmit NACK after next transfer since it will be the last (only)
		// byte.
		case SMB_MRADDACK:
			STA = 0;
			break;
		// Data byte received. ACK transmitted.
		// State should not occur because AA is set to zero in previous state.
		// Send STOP if state does occur.
		case SMB_MRDBACK:
			if (DataLen) {
				*DataRead = SMB0DAT;
				DataRead++;
				DataLen--;
			}
			else {
				AA = 0;
			}
			break;
		// All other status codes meaningless in this application. Reset communication.
		default:
			STO = 1; // Reset communication.
			SM_BUSY = 0;
			break;
	}
	SI=0; // clear interrupt flag
}

void init_SMBus (void) {
	SMB0CN = 0x44; // Enable SMBus with ACKs on acknowledge cycle
	SMB0CR = -80; 
	SM_BUSY = 0; // Free SMBus for first transfer.
	SCON0 = 0x00; //UART0
}

void init_LCD (void) {
	P0MDOUT = 0x040;//setta pin come out
	BackLight = 1;
	SM_Send(LCD, cmd_init_lcd, 8, 0x00);
}

void init (void) {
	EA = 0; //disabilita interrupt
	
	OSCICN |= 0x04;  //2Mhz
	
	//disabilita watchdog timer
	WDTCN = 0xDE;
	WDTCN = 0xAD;
	
	XBR0 = 0x05;
	XBR1 = 0x00;
	XBR2 = 0x40;
	
	EIE1 |= 0x02;
	
	EA = 1;
}

void init_acc(void){
	SM_Send(ACC, cmd_init_acc, 4, 0x41);
}

void leggi_inclinazione(void){
	int i[3];
	int a[10];
	int src = 0x00;
	DataRead = i;
	SM_Send (ACC, &src, 1, 0x01);
	while(SM_BUSY);
	
	i[0] = ANG[i[0] & 0x3F];
	i[1] = ANG[i[1] & 0x3F];
	//i[2] = 90 - ANG[i[2] & 0x3F];
	i[2] = ANG[i[2] & 0x3F];
	
	/*i[0] &= 0x3F;
	i[1] &= 0x3F;
	i[2] &= 0x3F;
	
	i[0] = ANG[i[0]];
	i[1] = ANG[i[1]];
	i[2] = 90 - ANG[i[2]];*/
	
	a[0] = 48 + (i[0]/10); //decine
	a[1] = 48 + (i[0]%10); //unità
	a[2] = 0;
	a[3] = 48 + (i[1]/10); //decine
	a[4] = 48 + (i[1]%10); //unità
	a[5] = 0;
	a[6] = 48 + (i[2]/10); //decine
	a[7] = 48 + (i[2]%10); //unità
	a[8] = 0;
	
	// lcd 00 0x12  per andare a capo
	SM_Send(LCD, clean_LCD, 2, 0x00); // indirizzo LCD 0x00 0x01 
	
	SM_Send(LCD, a, 8, 0x40);
}

void main (void) {
	init();
	
	init_SMBus();
	
	init_LCD();
	
	init_acc();
	
	leggi_inclinazione();
	leggi_inclinazione();
	
	while(1){
		leggi_inclinazione();
	}
}
