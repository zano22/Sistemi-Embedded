void parte3(){
	init_button();
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