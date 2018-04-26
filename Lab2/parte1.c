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