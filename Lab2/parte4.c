void parte4(){
	stato = 0; //stato A
	click = 0; //primo click
	init_timer2(23334); //lampeggia led - stato A
	timer=0;
	init_double();
}

//parte 4
void init_timer2(unsigned int counts){
	//CKCON = 0x00;	// Define clock (T2M). Timer 2			// uses system clock DIV BY 12
	T2CON = 0x00;
	RCAP2 = counts;	// Init reload values in the Capture registers
	T2 = 0xFFFF;      // count register set to reload immediately when the first clock occurs
	IE |= 0x20;     	// IE.5, Enable Timer 2 interrupts (ET2)
	T2CON |= 0x04; 
}

void timer2_ISR(void) interrupt 5{
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

void init_timer4_200(unsigned int counts){
	//CKCON = 0x00;	// Define clock (T2M). Timer 4			// uses system clock DIV BY 12
	T4CON = 0x00;
	RCAP4 = counts;	// Init reload values in the Capture registers
	T4 = 0xFFFF;      // count register set to reload immediately when the first clock occurs
	IE |= 0x20;     	// IE.5, Enable Timer 4 interrupts (ET2)
	T4CON |= 0x04; 
}

void timer4_200_ISR(void) interrupt 16{ 
	T4CON &= ~(0x80);
	click = 0; //timer finito
}

void init_double(){
	P3IF = 0x00;
	EIE2 |= 0x20;
}

void interrupt_double(void) interrupt 19{
	P3IF &= 0x7F;
	//primo click
	if(click==0){
		click = 1;
		//aspetto 200ms
		init_timer4_200(45000);
	} else {
		//se doppio click
		T4CON &= ~(0x04);//stop timer
		if(stato==0){//cambio stato
			stato = 1;
			T2CON &= ~(0x04);//stop timer
		} else {
			stato = 0;
			init_timer2(23334); //lampeggia led - stato A
			timer=0;
		}
		click=0;
	}
	//altrimenti nulla
}