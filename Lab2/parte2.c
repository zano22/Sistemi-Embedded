void parte2(){
	init_timer(23334);
	timer=0;
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