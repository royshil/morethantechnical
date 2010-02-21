#define CRYSTAL_FREQUENCY FOSC
#define PLL_FACTOR        PLL_MUL
#define VPBDIV_FACTOR     PBSD

#define lcd_backlight_on() { IODIR0 |= (1<<30); IOPIN0 |= (1<<30); }
#define lcd_en_rs_out() { IODIR1 |= (1<<24) | (1<<25); }
#define lcd_rw_out() { IODIR0 |= (1<<22); }

#define lcd_data8_out() { IODIR1 |= (0xff << 16); }
#define lcd_data8_in() { IODIR1 &= 0xff00ffff; }
#define lcd_data8_set(c) { IOSET1 = (c << 16); IOCLR1 = (~c << 16) & 0x00ff0000; }

#define lcd_data4_out() { IODIR1 |= 0x00f00000; }
#define lcd_data4_in() { IODIR1 &= 0xff0fffff; }
#define lcd_data4_set(c) { IOSET1 = ((c & 0x0f) << 20); IOCLR1 = (~(c & 0x0f) << 20) & 0x00f00000; }

#define lcd_rs_set() { IOSET1 = (1<<24); }
#define lcd_rs_clear() { IOCLR1 = (1<<24); }
#define lcd_en_set() { IOSET1 = (1<<25); }
#define lcd_en_clear() { IOCLR1 = (1<<25); }
#define lcd_rw_set() { IOSET0 = (1<<22); }
#define lcd_rw_clear() { IOCLR0 = (1<<22); }

static void delayMs(tU16 delayInMs)
{
	tU16 i,k;
	for(i=0;i<delayInMs;i++){
		for(k=0;k<3000;k++){}
}  
  /*
   * setup timer #1 for delay
   *
  TIMER1_TCR = 0x02;          //stop and reset timer
  TIMER1_PR  = 0x00;          //set prescaler to zero
  TIMER1_MR0 = delayInMs * ((CRYSTAL_FREQUENCY * PLL_FACTOR)/ (1000 * VPBDIV_FACTOR));
  TIMER1_IR  = 0xff;          //reset all interrrupt flags
  TIMER1_MCR = 0x04;          //stop timer on match
  TIMER1_TCR = 0x01;          //start timer
  
  //wait until delay time has elapsed
  while (TIMER1_TCR & 0x01)
    ;*/
}
void lcdWriteOne(tU8 c) {
#ifndef LCD_USE_4BIT
	lcd_data8_set(c);
#else
	lcd_data4_set(c);
#endif	
	delayMs(1);

	lcd_en_set();
	delayMs(10);
		
	lcd_en_clear();
	delayMs(100);
	
	//lcd_data8_set(0);
}



void lcdWriteInstruction(tU8 c)
{
	lcd_rs_clear();
	lcd_rw_clear();
	delayMs(1);

	lcdWriteOne(c);
}

tU8 lcdReadInstruction() //
{
	tU8 ret;
	lcd_data8_in();
	
	lcd_rs_set();
	lcd_rw_set();
	delayMs(1);
	lcd_en_set();
	delayMs(1);
	ret=((IOPIN1 & 0x00ff0000)>>16);
	lcd_en_clear();
	delayMs(1);
	lcd_data8_out();
	return (ret);
		
}
	
void lcdPrintChar(tU8 c) {
	lcd_rs_set();
	lcd_rw_clear();
	delayMs(5);

#ifndef LCD_USE_4BIT	
	lcdWriteOne(c);
#else
	lcdWriteOne((c >> 4) & 0x0f);
	lcdWriteOne(c & 0x0f);
#endif
}

void lcdPrintString(tU8 *str) {
	int i=0;
	while((str[i]!=0)&&(i<32))
	{
		if(i%32==0)
		{
			lcdWriteInstruction(0x01); //clear display
			delayMs(5);
			lcdWriteInstruction(0x02); //go home
			delayMs(5);
			lcdWriteInstruction(0x06); //increment
			delayMs(5);
		}
		else if(i%16==0)  
			lcdWriteInstruction(0xC0);

		#ifndef LCD_USE_4BIT	
		lcdPrintChar(str[i]);
		#else
		lcdWriteOne((str[i] >> 4) & 0x0f);
		lcdWriteOne(str[i] & 0x0f);
		#endif
		i++;
	}
}

void lcdInit() {
	//delayMs(3000);
	
	lcd_backlight_on();
	lcd_en_rs_out();
	lcd_rw_out();
	
#ifndef LCD_USE_4BIT
	lcd_data8_out();
	lcdWriteInstruction(0x38); //function set
	delayMs(5);
	lcdWriteInstruction(0x0E); //display on, blink off
	delayMs(5);
	lcdWriteInstruction(0x01); //clear display
	delayMs(5);
	lcdWriteInstruction(0x02); //go home
	delayMs(5);
	lcdWriteInstruction(0x06); //increment
	delayMs(5);
#else
	lcd_data4_out();

	//function set
	lcdWriteInstruction(0x2); 
	delayMs(2);
	lcdWriteInstruction(0x2);
	delayMs(2);
	lcdWriteInstruction(0x8);
	delayMs(2);
	
	//display on, blink off
	lcdWriteInstruction(0x0); 
	delayMs(2);
	lcdWriteInstruction(0xE);
	delayMs(2);

	//clear display
	lcdWriteInstruction(0x0); 
	delayMs(2);
	lcdWriteInstruction(0x1);
	delayMs(2);

	//go home
	lcdWriteInstruction(0x0); 
	delayMs(2);
	lcdWriteInstruction(0x2);
	delayMs(2);
	
	//increment
	lcdWriteInstruction(0x0); 
	delayMs(2);
	lcdWriteInstruction(0x6);
	delayMs(2);
#endif
	
	//delayMs(1000);
}

void lcdGoto(tU8 rl) {
	if(rl > 0) {
		//move right
#ifndef LCD_USE_4BIT
		lcdWriteInstruction(0x14);
#else
		lcdWriteInstruction(0x1);
		delayMs(40);
		lcdWriteInstruction(0x4);
#endif
	} else {
		//move left
#ifndef LCD_USE_4BIT
		lcdWriteInstruction(0x10);
#else
		lcdWriteInstruction(0x1);
		delayMs(40);
		lcdWriteInstruction(0x0);
#endif
	}
}
