#include <c8051f020.h> // SFR definitions

#define WRITE 0x00 // SMBus WRITE command
#define READ 0x01 // SMBus READ command

#define SMB_START 0x08 // (MT & MR) START transmitted
#define SMB_MTADDACK 0x18 // (MT) Slave address + W transmitted
#define SMB_MTDBACK 0x28 // (MT) data byte transmitted
#define SMB_MRADDACK 0x40 // (MR) Slave address + R transmitted;
#define SMB_MRDBACK 0x50 // (MR) data byte rec’vd; ACK transmitted

#define LCD 0x7C

sbit BackLight = P0^6;
int Temp;

char COMMAND; // Holds the slave address + R/W bit for use in the SMBus ISR.
char WORD; // Holds data to be transmitted by the SMBus OR data that has just been received.
char BYTE_NUMBER; // Used by ISR to check what data has just been sent - High address byte, Low byte, or data byte
unsigned char HIGH_ADD, LOW_ADD; // High & Low byte for EEPROM memory address
bit SM_BUSY; // This bit is set when a send or receive is started. It is cleared by the ISR when the operation is finished.


int cmd_init_lcd[8] = {0x38, 0x39, 0x14, 0x74, 0x54, 0x6F, 0x0C, 0x01};
int array[16] = {
	0x40, 0x30, 0x30, 0x30, 0x30, 0x39, 0x32, 0x32,
	0x30, 0x39, 0x35, 0x30, 0x30, 0x30, 0x36, 0x31
};

//array 16 byte caricata da smsend con stringa che passo
//var elementi correnti
//interrupt legge array


void SM_Send (char chip_select, unsigned int byte_address, char out_byte){
	while (SM_BUSY); // Wait for SMBus to be free.
	SM_BUSY = 1; // Occupy SMBus (set to busy)
	SMB0CN = 0x44; // SMBus enabled, ACK on acknowledge cycle
	BYTE_NUMBER = 2; // 2 address bytes.
	COMMAND = (chip_select | WRITE); // Chip select + WRITE
	HIGH_ADD = ((byte_address >> 8) & 0x00FF);// Upper 8 address bits
	LOW_ADD = (byte_address & 0x00FF); // Lower 8 address bits
	WORD = out_byte; // Data to be writen

	STO = 0;
	STA = 1; // Start transfer
}

char SM_Receive (char chip_select, unsigned int byte_address){
	while (SM_BUSY); // Wait for bus to be free.
	SM_BUSY = 1; // Occupy SMBus (set to busy)
	SMB0CN = 0x44; // SMBus enabled, ACK on acknowledge cycle
	BYTE_NUMBER = 2; // 2 address bytes
	COMMAND = (chip_select | READ); // Chip select + READ
	HIGH_ADD = ((byte_address >> 8) & 0x00FF);// Upper 8 address bits
	LOW_ADD = (byte_address & 0x00FF); // Lower 8 address bits

	STO = 0;
	STA = 1; // Start transfer
	while (SM_BUSY); // Wait for transfer to finish
	return WORD;
}

void SMBUS_ISR (void) interrupt 7 {
	switch (SMB0STA){ // Status code for the SMBus (SMB0STA register)
		// Master Transmitter/Receiver: START condition transmitted.
		// The R/W bit of the COMMAND word sent after this state will
		// always be a zero (W) because for both read and write,
		// the memory address must be written first.
		case SMB_START:
			SMB0DAT = LCD;//(COMMAND & 0xFE); // Load address of the slave to be accessed.
			STA = 0; // Manually clear START bit
			break;
		// Master Transmitter: Slave address + WRITE transmitted. ACK received.
		case SMB_MTADDACK:
			SMB0DAT = HIGH_ADD; // Load high byte of memory address to be written.
			break;
		// Master Transmitter: Data byte transmitted. ACK received.
		// This state is used in both READ and WRITE operations. Check BYTE_NUMBER
		// for memory address status - if only HIGH_ADD has been sent, load LOW_ADD.
		// If LOW_ADD has been sent, check COMMAND for R/W value to determine
		// next state.
		case SMB_MTDBACK:
			switch (BYTE_NUMBER){
			 case 2: // If BYTE_NUMBER=2, only HIGH_ADD
				SMB0DAT = LOW_ADD; // has been sent.
				BYTE_NUMBER--; // Decrement for next time around.
				break;
			case 1: // If BYTE_NUMBER=1, LOW_ADD was just sent.
				if (COMMAND & 0x01){ // If R/W=READ, sent repeated START.
				STO = 0;
				STA = 1;
				} else {
				SMB0DAT = WORD; // If R/W=WRITE, load byte to write.
				BYTE_NUMBER--;
				}
				break;
			default: // If BYTE_NUMBER=0, transfer is finished.
				STO = 1;
				SM_BUSY = 0; // Free SMBus
			}
			break;
		// Master Receiver: Slave address + READ transmitted. ACK received.
		// Set to transmit NACK after next transfer since it will be the last (only)
		// byte.
		case SMB_MRADDACK:
			AA = 0; // NACK sent on acknowledge cycle.
			break;
		// Data byte received. ACK transmitted.
		// State should not occur because AA is set to zero in previous state.
		// Send STOP if state does occur.
		case SMB_MRDBACK:
			STO = 1;
			SM_BUSY = 0;
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

void init (void) {
	EA = 0; //disabilita interrupt
	
	OSCICN = 0x04;  //2Mhz
	
	//disabilita watchdog timer
	WDTCN = 0xDE;
	WDTCN = 0xAD;
	
	XBR0 = 0x05;
	XBR1 = 0x00;
	XBR2 = 0x40;
	
	EIE1 |= 0x02;
	
	P0MDOUT = 0x040;//setta pin come out
	
	BackLight = 1;
	
	EA = 1;
}

void main (void) {
	init();
	
	init_SMBus();
	
	SM_Send(LCD, cmd_init_lcd, 8);
	SM_Send(LCD, "@ciao", 5);
	//SM_Send(LCD, array, 16);
	
	//while(1);
}